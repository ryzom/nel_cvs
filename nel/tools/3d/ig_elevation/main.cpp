// ---------------------------------------------------------------------------

#include <vector>
#include <string>

#include "nel/misc/config_file.h"
#include "nel/misc/file.h"
#include "nel/misc/bitmap.h"

#include "ligo/zone_region.h"

#include "3d/scene_group.h"

#include <windows.h>

// ---------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NLLIGO;
using namespace NL3D;

// ---------------------------------------------------------------------------
// Out a string to the stdout and log.log
void outString (string &sText)
{
	createDebug ();
	InfoLog->displayRaw(sText.c_str());
	//printf ("%s", sText.c_str());
}

// ---------------------------------------------------------------------------
struct SExportOptions
{
	string	InputIGDir;
	string	OutputIGDir;
	float	CellSize;
	string	HeightMapFile1;
	float	ZFactor1;
	string	HeightMapFile2;
	float	ZFactor2;
	string	LandFile;

	// -----------------------------------------------------------------------
	bool load (const string &sFilename)
	{
		FILE * f = fopen (sFilename.c_str(), "rt");
		if (f == NULL)
			return false;
		else 
			fclose (f);
		
		try
		{			
			CConfigFile cf;
		
			cf.load (sFilename);

			// Out
			CConfigFile::CVar &cvOutputIGDir = cf.getVar("OutputIGDir");
			OutputIGDir = cvOutputIGDir.asString();

			// In
			CConfigFile::CVar &cvInputIGDir = cf.getVar("InputIGDir");
			InputIGDir = cvInputIGDir.asString();

			CConfigFile::CVar &cvCellSize = cf.getVar("CellSize");
			CellSize = cvCellSize.asFloat();

			CConfigFile::CVar &cvHeightMapFile1 = cf.getVar("HeightMapFile1");
			HeightMapFile1 = cvHeightMapFile1.asString();

			CConfigFile::CVar &cvZFactor1 = cf.getVar("ZFactor1");
			ZFactor1 = cvZFactor1.asFloat();

			CConfigFile::CVar &cvHeightMapFile2 = cf.getVar("HeightMapFile2");
			HeightMapFile2 = cvHeightMapFile2.asString();

			CConfigFile::CVar &cvZFactor2 = cf.getVar("ZFactor2");
			ZFactor2 = cvZFactor2.asFloat();

			CConfigFile::CVar &cvLandFile = cf.getVar("LandFile");
			LandFile = cvLandFile.asString();
		}
		catch (EConfigFile &e)
		{
			string sTmp = string("ERROR : Error in config file : ") + e.what() + "\n";
			outString (sTmp);
			return false;
		}
		return true;
	}
};

// ---------------------------------------------------------------------------
void dir (const string &sFilter, vector<string> &sAllFiles, bool bFullPath)
{
	WIN32_FIND_DATA findData;
	HANDLE hFind;
	char sCurDir[MAX_PATH];
	sAllFiles.clear ();
	GetCurrentDirectory (MAX_PATH, sCurDir);
	hFind = FindFirstFile (sFilter.c_str(), &findData);	
	while (hFind != INVALID_HANDLE_VALUE)
	{
		if (!(GetFileAttributes(findData.cFileName)&FILE_ATTRIBUTE_DIRECTORY))
		{
			if (bFullPath)
				sAllFiles.push_back(string(sCurDir) + "\\" + findData.cFileName);
			else
				sAllFiles.push_back(findData.cFileName);
		}
		if (FindNextFile (hFind, &findData) == 0)
			break;
	}
	FindClose (hFind);
}


// ---------------------------------------------------------------------------
CZoneRegion *loadLand (const string &filename)
{
	CZoneRegion *ZoneRegion;
	try
	{
		CIFile fileIn;
		fileIn.open (filename);
		ZoneRegion = new CZoneRegion;
		ZoneRegion->serial (fileIn);
	}
	catch (Exception& e)
	{
		string sTmp = string("Error in land file : ") + e.what();
		outString (sTmp);
		return NULL;
	}
	return ZoneRegion;
}


// ***************************************************************************
CInstanceGroup* LoadInstanceGroup (const char* sFilename)
{
	CIFile file;
	CInstanceGroup *newIG = new CInstanceGroup;

	if( file.open( sFilename ) )
	{
		try
		{
			newIG->serial (file);
		}
		catch (Exception &)
		{
			// Cannot save the file
			delete newIG;
			return NULL;
		}
	}
	else
	{
		delete newIG;
		return NULL;
	}
	return newIG;
}

// ***************************************************************************
bool SaveInstanceGroup (const char* sFilename, CInstanceGroup *pIG)
{
	COFile file;

	if( file.open( sFilename ) )
	{
		try
		{
			pIG->serial (file);
		}
		catch (Exception &)
		{
			return false;
		}
	}
	else
	{
		return false;
	}
	return true;
}

// ---------------------------------------------------------------------------
int main(int nNbArg, char**ppArgs)
{
	char sCurDir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, sCurDir);
	
	if (nNbArg != 2)
	{
		printf ("Use : ig_elevation configfile.cfg\n");
		printf ("\nExample of config.cfg\n\n");
		printf ("InputIGDir = \"ig_land_max\";\n");
		printf ("OutputIGDir = \"ig_land_max_elev\";\n");
		printf ("CellSize = 160.0;\n");
		printf ("HeightMapFile1 = \"w:/database/landscape/ligo/jungle/big.tga\";\n");
		printf ("ZFactor1 = 1.0;\n");
		printf ("HeightMapFile2 = \"w:/database/landscape/ligo/jungle/noise.tga\";\n");
		printf ("ZFactor2 = 0.5;\n");
		printf ("LandFile = \"w:/matis.land\";\n");

		return -1;
	}

	SExportOptions options;
	if (!options.load(ppArgs[1]))
	{
		return -1;
	}

	// Get all ig files in the input directory and elevate to the z of the double heightmap

	// Load the land
	CZoneRegion *ZoneRegion = loadLand(options.LandFile);
	sint32 nZoneMinX = ZoneRegion->getMinX() < 0	? 0		: ZoneRegion->getMinX();
	sint32 nZoneMaxX = ZoneRegion->getMaxX() > 255	? 255	: ZoneRegion->getMaxX();
	sint32 nZoneMinY = ZoneRegion->getMinY() > 0	? 0		: ZoneRegion->getMinY();
	sint32 nZoneMaxY = ZoneRegion->getMaxY() < -255 ? -255	: ZoneRegion->getMaxY();

	// Load the 2 height maps
	CBitmap *HeightMap1 = NULL;
	if (options.HeightMapFile1 != "")
	{
		HeightMap1 = new CBitmap;
		try 
		{
			CIFile inFile (options.HeightMapFile1);
			HeightMap1->load (inFile);
		}
		catch (Exception &)
		{
			string sTmp = string("Cant load height map : ") + options.HeightMapFile1;
			outString (sTmp);
			delete HeightMap1;
			HeightMap1 = NULL;
		}
	}
	CBitmap *HeightMap2 = NULL;
	if (options.HeightMapFile2 != "")
	{
		HeightMap2 = new CBitmap;
		try 
		{
			CIFile inFile (options.HeightMapFile2);
			HeightMap2->load (inFile);
		}
		catch (Exception &)
		{
			string sTmp = string("Cant load height map : ") + options.HeightMapFile2;
			outString (sTmp);
			delete HeightMap2;
			HeightMap2 = NULL;
		}
	}

	// Get all files
	vector<string> vAllFiles;
	SetCurrentDirectory (options.InputIGDir.c_str());
	dir ("*.ig", vAllFiles, false);
	SetCurrentDirectory (sCurDir);

	for (uint32 i = 0; i < vAllFiles.size(); ++i)
	{
		SetCurrentDirectory (options.InputIGDir.c_str());
		CInstanceGroup *pIG = LoadInstanceGroup (vAllFiles[i].c_str());
		if (pIG != NULL)
		{
			// For all instances !!!
			uint32 j;
			CVector vGlobalPos;
			CInstanceGroup::TInstanceArray IA;
			vector<CCluster> Clusters;
			vector<CPortal> Portals;
			vector<CPointLightNamed> PLN;
			pIG->retrieve (vGlobalPos, IA, Clusters, Portals, PLN);

			if (Clusters.size() != 0) continue;
			if (IA.size() == 0) continue;

			// Get average position of the center of the system
			CVector vCenter = CVector(0,0,0);
			for (j = 0; j < IA.size(); ++j)
				vCenter += IA[j].Pos;
			vCenter = vCenter / (float)IA.size();

			// Get Elevation at this point
			float x = vCenter.x;
			float y = vCenter.y;
			float deltaZ = 0.0f, deltaZ2 = 0.0f;
			CRGBAF color;
			sint32 SizeX = nZoneMaxX - nZoneMinX + 1;
			sint32 SizeY = nZoneMaxY - nZoneMinY + 1;
	
			clamp (x, options.CellSize*nZoneMinX, options.CellSize*(nZoneMaxX+1));
			clamp (y, options.CellSize*nZoneMinY, options.CellSize*(nZoneMaxY+1));

			if (HeightMap1 != NULL)
			{
				color = HeightMap1->getColor (	(x-options.CellSize*nZoneMinX)/(options.CellSize*SizeX), 
												1.0f - ((y-options.CellSize*nZoneMinY)/(options.CellSize*SizeY)));
				deltaZ = color.A;
				deltaZ = deltaZ - 127.0f; // Median intensity is 127
				deltaZ *= options.ZFactor1;
			}
		
			if (HeightMap2 != NULL)
			{
				color = HeightMap2->getColor (	(x-options.CellSize*nZoneMinX)/(options.CellSize*SizeX), 
												1.0f - ((y-options.CellSize*nZoneMinY)/(options.CellSize*SizeY)));
				deltaZ2 = color.A;
				deltaZ2 = deltaZ2 - 127.0f; // Median intensity is 127
				deltaZ2 *= options.ZFactor2;
			}
		
			deltaZ = (deltaZ + deltaZ2);


			// Modify all stuff of the instance group
			for (j = 0; j < IA.size(); ++j)
			{
				IA[j].Pos.z += deltaZ;
			}


			CInstanceGroup *pIGout = new CInstanceGroup;
			pIGout->build (vGlobalPos, IA, Clusters, Portals, PLN);

			SetCurrentDirectory (options.OutputIGDir.c_str());
			SaveInstanceGroup (vAllFiles[i].c_str(), pIGout);
			delete pIG;
		}
	}

	return 1;
}