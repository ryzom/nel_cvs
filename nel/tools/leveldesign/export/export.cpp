/** \file export.cpp
 * Implementation of export from leveldesign data to client data
 *
 * $Id: export.cpp,v 1.4 2002/02/13 16:55:54 besson Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "export.h"
#include "formVegetable.h"
#include "formPlant.h"

#include <windows.h>

#include "../../3d/ligo/lib/zone_region.h"
#include "../../3d/ligo/lib/primitive.h"

#include "../georges_lib/loader.h"
#include "../georges_lib/item.h"

#include "../georges_lib/FormBodyElt.h"
#include "../georges_lib/FormBodyEltAtom.h"
#include "../georges_lib/FormBodyEltList.h"
#include "../georges_lib/FormBodyEltStruct.h"

#include "3d/zone.h"
#include "3d/landscape.h"
#include "3d/scene_group.h"
#include "3d/visual_collision_manager.h"
#include "3d/visual_collision_entity.h"

#include "nel/misc/o_xml.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/path.h"


using namespace std;
using namespace NLMISC;
using namespace NL3D;
using namespace NLLIGO;

// ---------------------------------------------------------------------------
// SExportOptions
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
SExportOptions::SExportOptions ()
{
	GenerateLandscape = true;
	GenerateVegetable = true;
	CellSize = 160.0f;
}

// ---------------------------------------------------------------------------
void SExportOptions::serial (NLMISC::IStream& f)
{
	sint version = f.serialVersion (2);

	f.serial (GenerateLandscape);
	f.serial (GenerateVegetable);
	f.serial (OutLandscapeDir);
	f.serial (OutVegetableDir);

	if (version > 0)
	{
		f.serial (LandBankFile);
		f.serial (LandFarBankFile);
	}

	if (version > 1)
	{
		f.serial (LandTileNoiseDir);
	}
}

// ---------------------------------------------------------------------------
// CExport
// ---------------------------------------------------------------------------

bool CExport::_GeorgesLibInitialized = false;

// ---------------------------------------------------------------------------
CExport::CExport()
{
	if (!_GeorgesLibInitialized)
	{
		NLMISC_REGISTER_CLASS( CFormBodyElt );
		NLMISC_REGISTER_CLASS( CFormBodyEltAtom );
		NLMISC_REGISTER_CLASS( CFormBodyEltList );
		NLMISC_REGISTER_CLASS( CFormBodyEltStruct );
		_GeorgesLibInitialized = true;
	}
	_Landscape = NULL;
	_VCM = NULL;
	_VCE = NULL;
}

// ---------------------------------------------------------------------------
CExport::~CExport()
{
	if (_Landscape != NULL)
		delete _Landscape;
	if (_VCM != NULL)
	{
		_VCM->deleteEntity (_VCE);
		delete _VCM;
	}
}	


// ---------------------------------------------------------------------------
bool CExport::export (SExportOptions &opt, IExportCB *expCB)
{
	char sTmp[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, sTmp);

	_Options = &opt;
	_ExportCB = expCB;

	if (_Options->GenerateLandscape)
	{
		if (_ExportCB)
			_ExportCB->dispPass ("Generate Landscape");
		// \todo trap : GenerateLandscape with hulud
		//generateLandscape ();
	}

	// If we want to generate flora then we have to load the landscape
	vector<string>	allLandFiles;
	uint32 i;

	SetCurrentDirectory (_Options->SourceDir.c_str());
	getAllFiles (".land", allLandFiles);

	if (_Options->GenerateVegetable)
	{
		if (_ExportCB)
			_ExportCB->dispPass ("Load Landscape");
		if (_Landscape == NULL)
		{
			_Landscape = new CLandscape;
			_Landscape->init ();
			_VCM = new CVisualCollisionManager;
			_VCE = _VCM->createEntity ();
			_VCM->setLandscape (_Landscape);
			_VCE->setSnapToRenderedTesselation (false);
			try 
			{
				CIFile bankFile (_Options->LandBankFile);
				_Landscape->TileBank.serial (bankFile);
				CIFile farbankFile (_Options->LandFarBankFile);
				_Landscape->TileFarBank.serial (farbankFile);
				_Landscape->TileBank.makeAllPathRelative ();
				_Landscape->TileBank.setAbsPath ("");
				_Landscape->TileBank.makeAllExtensionDDS ();
				CPath::removeAllAlternativeSearchPath ();
				CPath::addSearchPath (_Options->LandTileNoiseDir, true, true);
				_Landscape->initTileBanks ();
				for (i = 0; i < allLandFiles.size(); ++i)
				{
					loadLandscape (allLandFiles[i]);
				}
			}
			catch (Exception &/*e*/)
			{
				if (_ExportCB)
					_ExportCB->dispError ("Cannot load banks files");
			}
		}
	}

	if (_Options->GenerateVegetable)
	{
		_VegInsts.clear ();
		if (_ExportCB)
			_ExportCB->dispPass ("Generate Vegetable");
		vector<string>	allVegetableFiles;
		SetCurrentDirectory (_Options->SourceDir.c_str());
		getAllFiles (".vegetable", allVegetableFiles);
		for (i = 0; i < allVegetableFiles.size(); ++i)
		{
			generateVegetable (allVegetableFiles[i]);
		}
		for (i = 0; i < allLandFiles.size(); ++i)
		{
			writeVegetable (allLandFiles[i]);
		}
	}

	if (_ExportCB)
		_ExportCB->dispPass ("Cleaning");

	SetCurrentDirectory (sTmp);
	if (_ExportCB)
		_ExportCB->dispPass ("Finished");

	return true;
}

// ---------------------------------------------------------------------------
void CExport::getAllFiles (const string &ext, vector<string> &files)
{
	char sCurDir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, sCurDir);

	WIN32_FIND_DATA findData;
	HANDLE hFind = FindFirstFile ("*.*", &findData);	
	while (hFind != INVALID_HANDLE_VALUE)
	{
		if (!((strcmp (findData.cFileName, ".") == 0) || (strcmp (findData.cFileName, "..") == 0)))
		{
			if (findData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			{
				string sNewDir = sCurDir;
				sNewDir += string("\\") + findData.cFileName;
				SetCurrentDirectory (sNewDir.c_str());
				getAllFiles (ext, files);
				SetCurrentDirectory (sCurDir);
			}
			else
			{
				if (strlen(findData.cFileName) > strlen(ext.c_str()))
					if (stricmp(&findData.cFileName[strlen(findData.cFileName)-strlen(ext.c_str())], ext.c_str()) == 0)
					{
						string fullName = sCurDir;
						fullName += string("\\") + findData.cFileName;
						files.push_back (fullName);
					}
			}
		}

		if (FindNextFile (hFind, &findData) == 0)
			break;
	}
	FindClose (hFind);

	SetCurrentDirectory (sCurDir);
}

// ---------------------------------------------------------------------------
bool CExport::searchFile (const std::string &plantName, std::string &dir)
{
	char sCurDir[MAX_PATH];
	bool bFound = false;
	GetCurrentDirectory (MAX_PATH, sCurDir);

	WIN32_FIND_DATA findData;
	HANDLE hFind = FindFirstFile ("*.*", &findData);	
	while (hFind != INVALID_HANDLE_VALUE)
	{
		if (!((strcmp (findData.cFileName, ".") == 0) || (strcmp (findData.cFileName, "..") == 0)))
		{
			if (findData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			{
				string sNewDir = sCurDir;
				sNewDir += string("\\") + findData.cFileName;
				SetCurrentDirectory (sNewDir.c_str());
				if (searchFile (plantName, dir))
				{
					bFound = true;
					break;
				}
				SetCurrentDirectory (sCurDir);
			}
			else
			{
				if (plantName == findData.cFileName)
				{
					dir = sCurDir;
					bFound = true;
					break;
				}
			}
		}

		if (FindNextFile (hFind, &findData) == 0)
			break;
	}
	FindClose (hFind);
	SetCurrentDirectory (sCurDir);
	return bFound;
}

// ---------------------------------------------------------------------------
bool CExport::generateVegetable (const std::string &SrcFile)
{
	uint32 i, j, k, l, m;

	if (_ExportCB)
		_ExportCB->dispPass ("Generating From " + SrcFile);

	// Load all .prim
	vector<CPrimRegion> allPrimRegion;
	{
		vector<string> allPrimFiles;
		getAllFiles (".prim", allPrimFiles);
		for (i = 0; i < allPrimFiles.size(); ++i)
		{
			try 
			{
				CPrimRegion tmpPrimRegion;
				CIFile fileIn;
				fileIn.open (allPrimFiles[i]);
				CIXml input;
				input.init (fileIn);
				tmpPrimRegion.serial (input);
				allPrimRegion.push_back (tmpPrimRegion);
			}
			catch (Exception &/*e*/)
			{
				if (_ExportCB != NULL)
					_ExportCB->dispWarning (string("Cant load ") + allPrimFiles[i]);
			}
		}
	}

	// Load the .vegetable file (georges file) and load all associated .plant
	SFormVegetable formVegetable;
	map<string, SFormPlant> Plants;
	{
		CLoader loader;
		loader.SetRootDirectory (_Options->RootDir);
		loader.SetWorkDirectory (_Options->SourceDir);
		CPath::addSearchPath (_Options->LandTileNoiseDir, true, true);
		
		CItem	item;
		item.SetLoader (&loader);

		try
		{
			item.Load (SrcFile);
		}
		catch (Exception &e)
		{
			if (_ExportCB != NULL)
				_ExportCB->dispError (string("Cant load ") + SrcFile + "(" + e.what() + ")" );
		}

		// Read .vegetable
		formVegetable.build (item);

		// Load the .plant associated	
		for (i = 0; i < formVegetable.PlantInstances.size(); ++i)
		{
			const string &plantName = formVegetable.PlantInstances[i].Name;
			map<string, SFormPlant>::iterator it = Plants.find (plantName);
			if (it != Plants.end()) // Already here ?!
				continue; // Zap it
			CItem	itemTmp;
			itemTmp.SetLoader (&loader);

			try
			{
				char sCurDir[MAX_PATH];
				GetCurrentDirectory (MAX_PATH, sCurDir);
				SetCurrentDirectory (_Options->RootDir.c_str());
				string dir;

				if (searchFile (plantName, dir))
				{
					string tmpName = dir + string("\\") + plantName;
					itemTmp.Load (tmpName);
				}
				SetCurrentDirectory (sCurDir);
			}
			catch (Exception &e)
			{
				if (_ExportCB != NULL)
					_ExportCB->dispError (string("Cant load ") + plantName + "(" + e.what() + ")" );
			}

			SFormPlant plantTmp;
			plantTmp.build (itemTmp);
			Plants.insert (map<string, SFormPlant>::value_type(plantName, plantTmp));
		}
	}

	// Sort PlantInstances by biggest radius first
	if (formVegetable.PlantInstances.size() > 1)
	for (i = 0; i < (formVegetable.PlantInstances.size()-1); ++i)
	for (j = i+1; j < formVegetable.PlantInstances.size(); ++j)
	{
		SPlantInstance &rPlantI = formVegetable.PlantInstances[i];
		SPlantInstance &rPlantJ = formVegetable.PlantInstances[j];
		map<string, SFormPlant>::iterator it = Plants.find (rPlantI.Name);
		if (it == Plants.end())
			continue;
		SFormPlant &rFormPlantI = it->second;

		it = Plants.find (rPlantJ.Name);
		if (it == Plants.end())
			continue;
		SFormPlant &rFormPlantJ = it->second;
		if (rFormPlantI.BundingRadius < rFormPlantJ.BundingRadius)
		{
			SPlantInstance pi = formVegetable.PlantInstances[i];
			formVegetable.PlantInstances[i] = formVegetable.PlantInstances[j];
			formVegetable.PlantInstances[j] = pi;
		}
	}

	// Generating
	float jitter = formVegetable.JitterPos;
	srand (formVegetable.RandomSeed);
	for (i = 0; i < formVegetable.IncludePatats.size(); ++i)
	{
		uint32 nCurPlant = 0;
		CVector vMin, vMax;

		// Get the patat
		CPrimZone *pPatat = NULL;
		for (j = 0; j < allPrimRegion.size(); ++j)
			for (k = 0; k < allPrimRegion[j].VZones.size(); ++k)
			if (allPrimRegion[j].VZones[k].Name == formVegetable.IncludePatats[i])
				pPatat = &allPrimRegion[j].VZones[k];
		if ((pPatat == NULL) || (pPatat->VPoints.size() <= 2))
		{
			if (pPatat == NULL)
			{
				if (_ExportCB)
					_ExportCB->dispWarning ("Cannot find " + formVegetable.IncludePatats[i]);	
			}
			else
			if (pPatat->VPoints.size() <= 2)
			{
				if (_ExportCB)
					_ExportCB->dispWarning ("Patat " + pPatat->Name + " has less than 2 points");	
			}
			continue;
		}

		vMin = vMax = pPatat->VPoints[0];
		for (j = 0; j < pPatat->VPoints.size(); ++j)
		{
			if (vMin.x > pPatat->VPoints[j].x) vMin.x = pPatat->VPoints[j].x;
			if (vMin.y > pPatat->VPoints[j].y) vMin.y = pPatat->VPoints[j].y;
			if (vMin.z > pPatat->VPoints[j].z) vMin.z = pPatat->VPoints[j].z;
			if (vMax.x < pPatat->VPoints[j].x) vMax.x = pPatat->VPoints[j].x;
			if (vMax.y < pPatat->VPoints[j].y) vMax.y = pPatat->VPoints[j].y;
			if (vMax.z < pPatat->VPoints[j].z) vMax.z = pPatat->VPoints[j].z;
		}

		for (j = 0; j < formVegetable.PlantInstances.size(); ++j)
		{
			SPlantInstance &rPlant = formVegetable.PlantInstances[j];
			map<string, SFormPlant>::iterator it = Plants.find (rPlant.Name);
			if (it == Plants.end())
			{
				if (_ExportCB)
					_ExportCB->dispWarning ("Cannot find " + rPlant.Name);
				continue;
			}
			SFormPlant &rFormPlant = it->second;

			float squareLength = (float)sqrt (Pi*rFormPlant.BundingRadius*rFormPlant.BundingRadius / rPlant.Density);
			uint32 nNbPlantX = 1+(int)floor ((vMax.x-vMin.x) / squareLength);
			uint32 nNbPlantY = 1+(int)floor ((vMax.y-vMin.y) / squareLength);
			for (l = 0; l < nNbPlantY; ++l)
			for (k = 0; k < nNbPlantX; ++k)
			{
				if (_ExportCB)
					_ExportCB->dispPassProgress (((float)(k+l*nNbPlantX))/((float)(nNbPlantX*nNbPlantY)));

				bool bExists = false;
				CVector pos;
				for (m = 0; m < 32; ++m)
				{
					pos.x = vMin.x + squareLength * k + (frand(2.0f)-1.0f) * jitter * 0.5f * squareLength;
					pos.y = vMin.y + squareLength * l + (frand(2.0f)-1.0f) * jitter * 0.5f * squareLength;
					pos.z = 0.0f;
					if (pPatat->contains(pos))
					{
						if (isWellPlaced(pos, rPlant, rFormPlant))
						{
							bExists	= true;
							break;
						}
					}
				}

				if (!bExists)
					continue;

				SVegInst vi;
				vi.ShapeName = rFormPlant.Shape;
				vi.PlantName = rPlant.Name;
				vi.Scale = (formVegetable.ScaleMax-formVegetable.ScaleMin)*frand(1.0)+formVegetable.ScaleMin;
				vi.Radius = rFormPlant.BundingRadius * vi.Scale;
				vi.Rot = (float)Pi * frand (1.0);
				
				if (formVegetable.PutOnWater)
				{
					if (pos.z < formVegetable.WaterHeight)
						pos.z = formVegetable.WaterHeight;
				}
				vi.Pos = pos;
				_VegInsts.push_back (vi);
			}
		}
	}

	// temptemptemp
	/*if (_ExportCB != NULL)
	{
		map<string,SFormPlant>::iterator it = Plants.begin();
		while (it != Plants.end())
		{
			SFormPlant &rP = it->second;
			_ExportCB->dispInfo (it->first + " -> " + rP.Name + "\r\n" + rP.Shape + "\r\n" + 
								rP.Shadow + "\r\n" + toString(rP.BundingRadius) + "\r\n" + toString(rP.CollisionRadius));
			++it;
		}
	}*/


	/*	for (i = 0; i < formVegetable.PlantInstances.size(); ++i)
	{
		if (_ExportCB != NULL)
			_ExportCB->dispInfo (formVegetable.PlantInstances[i].Name + "(" + toString(formVegetable.PlantInstances[i].Density)
								+ ")(" + toString(formVegetable.PlantInstances[i].Falloff) + ")" );
	}*/
	// temptemptemp

	return true;
}

// ---------------------------------------------------------------------------
float CExport::getZFromXY (float x, float y)
{
	CVector pos = CVector(x, y, 0);
	CVector normal;
	float z, zmin, zmax;

	// Approximate the z with patch bounding boxes
	sint32 zoneX = (sint32)floor (x/_Options->CellSize);
	sint32 zoneY = (sint32)floor (-y/_Options->CellSize);
	sint32 zoneId = zoneY * 256 +  zoneX;
	CZone *pZone = _Landscape->getZone (zoneId);
	if (pZone == NULL)
		return -100000.0f;

	CAABBoxExt bb = pZone->getZoneBB();
	zmin = bb.getMin().z;
	zmax = bb.getMax().z;
	pos.z = zmin;
	z = zmin;
	while (z < zmax)
	{
		if (_VCE->snapToGround(pos, normal))
			break;
		z += CVisualCollisionEntity::BBoxRadiusZ / 2.0f; // Super sampling due to max frequency on radiosity
		pos.z = z;
	}

	if (z >= zmax)
		return -100000.0f;

	return pos.z;
}

// ---------------------------------------------------------------------------
bool CExport::isWellPlaced (CVector &pos, SPlantInstance &rPI, SFormPlant &rFP)
{
	uint32 i;

	// Look if this vegetable intersect with one of the current ones
	for (i = 0; i < _VegInsts.size(); ++i)
	{
		CVector temp = _VegInsts[i].Pos - pos;
		if (temp.norm() < (_VegInsts[i].Radius+rFP.BundingRadius))
			return false;
	}

	// Get the real Z
	pos.z = getZFromXY (pos.x, pos.y);
	if (pos.z < -90000.0f)
		return false;

	// Get some Z around to see if we can put the vegetable on the ground
	uint32 nNbSamples = 8; // Const to be put somewhere
	vector<CVector> base;
	base.resize (nNbSamples);
	for (i = 0; i < nNbSamples; ++i)
	{
		base[i] = pos;
		base[i].x += rFP.CollisionRadius * cosf((2.0f*(float)Pi*i)/(float)nNbSamples);
		base[i].y += rFP.CollisionRadius * sinf((2.0f*(float)Pi*i)/(float)nNbSamples);
		base[i].z = getZFromXY (base[i].x, base[i].y);

		if (fabs(base[i].z-pos.z) > 0.8f)
			return false;
	}

	return true;
}

// ---------------------------------------------------------------------------
void CExport::writeVegetable (const string &LandFile)
{
	sint32 i, j, k;

	if (_VegInsts.size() == 0)
		return;

	CZoneRegion zoneRegion;
	CIFile inFile;
	inFile.open (LandFile);
	zoneRegion.serial (inFile);
	inFile.close();
	// Load all zone

	for (j = zoneRegion.getMinY(); j <= zoneRegion.getMaxY(); ++j)
	for (i = zoneRegion.getMinX(); i <= zoneRegion.getMaxX(); ++i)
	{
		if ((zoneRegion.getName(i,j) == STRING_OUT_OF_BOUND) ||
			(zoneRegion.getName(i,j) == STRING_UNUSED))
			continue;

		vector<int> vegZone;
		// Take all vegetable instances in the zone (i,j)
		for (k = 0; k < (sint32)_VegInsts.size(); ++k)
		{
			if (((i*_Options->CellSize) < _VegInsts[k].Pos.x) && (_VegInsts[k].Pos.x < ((i+1)*_Options->CellSize)) && 
				((j*_Options->CellSize) < _VegInsts[k].Pos.y) && (_VegInsts[k].Pos.y < ((j+1)*_Options->CellSize)))
			{
				vegZone.push_back (k);
			}
		}


		// Make the .IG
		string ZoneName = "";
		ZoneName += NLMISC::toString(-j) + "_";
		ZoneName += 'a' + (i/26);
		ZoneName += 'a' + (i%26);

		CVector vGlobalPos = CVector (0.0f, 0.0f, 0.0f);
		CInstanceGroup::TInstanceArray Instances;
		vector<CCluster> Portals;
		vector<CPortal> Clusters;
		Instances.resize (vegZone.size());

		for (k = 0; k < (sint32)vegZone.size(); ++k)
		{
//vGlobalPos += _VegInsts[vegZone[k]].Pos;
			Instances[k].Pos = _VegInsts[vegZone[k]].Pos;
			Instances[k].Rot = CQuat(CVector::K, _VegInsts[vegZone[k]].Rot);
			Instances[k].Scale = CVector(_VegInsts[vegZone[k]].Scale, _VegInsts[vegZone[k]].Scale, _VegInsts[vegZone[k]].Scale);
			Instances[k].nParent = -1;
			Instances[k].Name = _VegInsts[vegZone[k]].ShapeName;
			Instances[k].InstanceName = _VegInsts[vegZone[k]].PlantName;
			/*Instances[k].InstanceName = "vegetable_"; // see if it works
			Instances[k].InstanceName += ZoneName + "_";
			Instances[k].InstanceName += '0' + ((k/1000)%10);
			Instances[k].InstanceName += '0' + ((k/100) %10);
			Instances[k].InstanceName += '0' + ((k/10)  %10);
			Instances[k].InstanceName += '0' + ( k      %10);*/
		}

// \todo trap -> look why it dont seems to work with a global positionning
//vGlobalPos /= (float)vegZone.size();
//for (k = 0; k < (sint32)vegZone.size(); ++k)
//	Instances[k].Pos -= vGlobalPos;

		CInstanceGroup IG;
		IG.build (vGlobalPos, Instances, Portals, Clusters);

		ZoneName = _Options->OutVegetableDir + "\\" + ZoneName;

		try 
		{
			ZoneName += ".ig";
			COFile outFile (ZoneName);
			IG.serial (outFile);
		}
		catch (Exception &e)
		{
			if (_ExportCB != NULL)
				_ExportCB->dispWarning ("Cant write " + ZoneName + "(" + e.what() + ")");
		}
	}
}

// ---------------------------------------------------------------------------
void CExport::loadLandscape (const string &LandFile)
{
	CZoneRegion zoneRegion;
	
	CIFile inFile;
	try 
	{
		inFile.open (LandFile);
		zoneRegion.serial (inFile);
		inFile.close();
	}
	catch (Exception &)
	{
		if (_ExportCB != NULL)
			_ExportCB->dispWarning (string("Cant load ") + LandFile);
	}
	// Load all zone

	sint32 nTotalFile = (zoneRegion.getMaxY() - zoneRegion.getMinY()) * (zoneRegion.getMaxX() - zoneRegion.getMinX());
	sint32 nCurrentFile = 0;
	for (sint32 j = zoneRegion.getMinY(); j <= zoneRegion.getMaxY(); ++j)
	for (sint32 i = zoneRegion.getMinX(); i <= zoneRegion.getMaxX(); ++i)
	{
		++nCurrentFile;
		if (_ExportCB != NULL)
			_ExportCB->dispPassProgress(((float)nCurrentFile)/((float)nTotalFile));

		if ((zoneRegion.getName(i,j) == STRING_OUT_OF_BOUND) ||
			(zoneRegion.getName(i,j) == STRING_UNUSED))
			continue;

		// Generate zone name
		string ZoneName = NLMISC::toString(-j) + "_";
		ZoneName += ('a' + (i/26));
		ZoneName += ('a' + (i%26));

		ZoneName = _Options->OutLandscapeDir + string("\\") + ZoneName + string(".zonew");

		//if (_ExportCB != NULL)
		//	_ExportCB->dispInfo (string("Loading ") + ZoneName);

		try 
		{
			CZone zone;
			inFile.open (ZoneName);
			zone.serial (inFile);
			inFile.close ();
			_Landscape->addZone (zone);
		}
		catch(Exception &/*e*/)
		{
			if (_ExportCB != NULL)
				_ExportCB->dispWarning (string("Cant load ") + ZoneName);
		}
		if ((_ExportCB != NULL) && (_ExportCB->isCanceled()))
			return;
	}

}