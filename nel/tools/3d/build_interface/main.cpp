// lightmap_optimizer
// ------------------
// the goal is to regroup lightmap of a level into lightmap with a higher level

#include "nel/misc/common.h"
#include "nel/misc/file.h"
#include "nel/misc/bitmap.h"
#include "nel/misc/log.h"
#include "nel/misc/path.h"
#include "nel/misc/uv.h"

#include "windows.h"

#include <vector>
#include <string>

// ---------------------------------------------------------------------------

using namespace std;

// ---------------------------------------------------------------------------
char sExeDir[MAX_PATH];

void outString (const string &sText)
{
	char sCurDir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, sCurDir);
	SetCurrentDirectory (sExeDir);
	NLMISC::createDebug ();
	NLMISC::InfoLog->displayRaw(sText.c_str());
	SetCurrentDirectory (sCurDir);
}

// ---------------------------------------------------------------------------
void dir (const std::string &sFilter, std::vector<std::string> &sAllFiles, bool bFullPath)
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
bool fileExist (const std::string &sFileName)
{
	HANDLE hFile = CreateFile (sFileName.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, 
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;
	CloseHandle (hFile);
	return true;
}

// -----------------------------------------------------------------------------------------------
// Try all position to put pSrc in pDst
bool tryAllPos (NLMISC::CBitmap *pSrc, NLMISC::CBitmap *pDst, sint32 &x, sint32 &y)
{
	uint32 i, j;
	vector<uint8> &rSrcPix = pSrc->getPixels();
	vector<uint8> &rDstPix = pDst->getPixels();

	// Recalculate real size of the source (without padding to power of 2)
	uint32 nSrcWidth = pSrc->getWidth(), nSrcHeight = pSrc->getHeight();

	if (nSrcWidth > pDst->getWidth() ) return false;
	if (nSrcHeight > pDst->getHeight() ) return false;

	// For all position test if the Src plane can be put in
	for (j = 0; j <= (pDst->getHeight() - nSrcHeight); ++j)
	for (i = 0; i <= (pDst->getWidth() - nSrcWidth); ++i)
	{
		x = i; y = j;
		
		uint32 a, b;
		bool bCanPut = true;
		for (b = 0; b < nSrcHeight; ++b)
		{
			for (a = 0; a < nSrcWidth; ++a)
			{
				if (rSrcPix[4*(a+b*pSrc->getWidth())+3] != 0)
				{
					if (rDstPix[4*((x+a)+(y+b)*pDst->getWidth())+3] != 0 )
					{
						bCanPut = false;
						break;
					}
				}
			}
			if (bCanPut == false)
				break;
		}
		if (bCanPut)
			return true;
	}
	return false;
}

// -----------------------------------------------------------------------------------------------
bool putIn (NLMISC::CBitmap *pSrc, NLMISC::CBitmap *pDst, sint32 x, sint32 y, bool alphaTransfert=true)
{
	uint32 a, b;

	vector<uint8> &rSrcPix = pSrc->getPixels();
	vector<uint8> &rDstPix = pDst->getPixels();

	for (b = 0; b < pSrc->getHeight(); ++b)
	for (a = 0; a < pSrc->getWidth(); ++a)
		if (rSrcPix[4*(a+b*pSrc->getWidth())+3] != 0)
		{
			if (rDstPix[4*((x+a)+(y+b)*pDst->getWidth())+3] != 0)
				return false;
			rDstPix[4*((x+a)+(y+b)*pDst->getWidth())+0] = rSrcPix[4*(a+b*pSrc->getWidth())+0];
			rDstPix[4*((x+a)+(y+b)*pDst->getWidth())+1] = rSrcPix[4*(a+b*pSrc->getWidth())+1];
			rDstPix[4*((x+a)+(y+b)*pDst->getWidth())+2] = rSrcPix[4*(a+b*pSrc->getWidth())+2];
			if (alphaTransfert)
				rDstPix[4*((x+a)+(y+b)*pDst->getWidth())+3] = rSrcPix[4*(a+b*pSrc->getWidth())+3];
			else
				rDstPix[4*((x+a)+(y+b)*pDst->getWidth())+3] = 255;
		}
	return true;
}

// ---------------------------------------------------------------------------
string getBaseName (const string &fullname)
{
	string sTmp2 = "";
	int pos = fullname.rfind('_');
	for (int j = 0; j <= pos; ++j)
		sTmp2 += fullname[j];
	return sTmp2;
}

// ---------------------------------------------------------------------------
// resize the bitmap to the next power of 2 and preserve content
void enlargeCanvas (NLMISC::CBitmap &b)
{
	sint32 nNewWidth = b.getWidth(), nNewHeight = b.getHeight();
	if (nNewWidth > nNewHeight)
		nNewHeight *= 2;
	else
		nNewWidth *= 2;

	NLMISC::CBitmap b2;
	b2.resize (nNewWidth, nNewHeight, NLMISC::CBitmap::RGBA);
	
	std::vector<uint8> &rPixelBitmap = b2.getPixels(0);
	for (sint32 i = 0; i < nNewWidth*nNewHeight*4; ++i)
		rPixelBitmap[i] = 0;
	
	putIn (&b, &b2, 0, 0);
	b = b2;
}


// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main(int nNbArg, char **ppArgs)
{
	
	if (nNbArg != 3)
	{
		outString ("ERROR : Wrong number of arguments\n");
		outString ("USAGE : build_interface <path_maps> <out_tga_name>\n");
		return -1;
	}
	
	char sMapsDir[MAX_PATH];

	GetCurrentDirectory (MAX_PATH, sExeDir);
	// Get absolute directory for lightmaps
	if (!SetCurrentDirectory(ppArgs[1]))
	{
		outString (string("ERROR : directory ") + ppArgs[1] + " do not exists\n");
		return -1;
	}
	GetCurrentDirectory (MAX_PATH, sMapsDir);
	SetCurrentDirectory (sExeDir);

	vector<string> AllMapNames;
	vector<NLMISC::CBitmap*> AllMaps;
	sint32 i, j;

	// Load all maps
	SetCurrentDirectory (sMapsDir);
	dir ("*.tga", AllMapNames, false);
	AllMaps.resize (AllMapNames.size());
	for (i = 0; i < (sint32)AllMapNames.size(); ++i)
	{
		try
		{
			NLMISC::CBitmap *pBtmp = new NLMISC::CBitmap;
			NLMISC::CIFile inFile;
			inFile.open(AllMapNames[i]);
			pBtmp->load(inFile);
			AllMaps[i] = pBtmp;
		}
		catch (NLMISC::Exception &e)
		{
			outString (string("ERROR :") + e.what());
			return -1;
		}
	}

	// Sort all maps by decreasing size
	for (i = 0; i < (sint32)(AllMaps.size()-1); ++i)
	for (j = i+1; j < (sint32)AllMaps.size(); ++j)
	{
		NLMISC::CBitmap *pBI = AllMaps[i];
		NLMISC::CBitmap *pBJ = AllMaps[j];
		if ((pBI->getWidth()*pBI->getHeight()) < (pBJ->getWidth()*pBJ->getHeight()))
		{
			NLMISC::CBitmap *pBTmp = AllMaps[i];
			AllMaps[i] = AllMaps[j];
			AllMaps[j] = pBTmp;

			string sTmp = AllMapNames[i];
			AllMapNames[i] = AllMapNames[j];
			AllMapNames[j] = sTmp;
		}
	}

	// Place all maps into the global texture
	NLMISC::CBitmap GlobalTexture, GlobalMask;
	GlobalTexture.resize (1, 1, NLMISC::CBitmap::RGBA);
	GlobalMask.resize (1, 1, NLMISC::CBitmap::RGBA);
	std::vector<uint8> &rPixelBitmap = GlobalTexture.getPixels(0);
	rPixelBitmap[0] = rPixelBitmap[1] = rPixelBitmap[2] = rPixelBitmap[3] = 0;
	std::vector<uint8> &rPixelMask = GlobalMask.getPixels(0);
	rPixelMask[0] = rPixelMask[1] = rPixelMask[2] = rPixelMask[3] = 0;
	vector<NLMISC::CUV> UVMin, UVMax;
	UVMin.resize (AllMapNames.size(), NLMISC::CUV(0.0f, 0.0f));
	UVMax.resize (AllMapNames.size(), NLMISC::CUV(0.0f, 0.0f));
	for (i = 0; i < (sint32)AllMapNames.size(); ++i)
	{
		sint32 x, y;
		while (!tryAllPos(AllMaps[i], &GlobalMask, x, y))
		{
			// Enlarge global texture
			enlargeCanvas (GlobalTexture);
			enlargeCanvas (GlobalMask);
		}
		putIn (AllMaps[i], &GlobalTexture, x, y);
		putIn (AllMaps[i], &GlobalMask, x, y, false);
		UVMin[i].U = x+0.5f;
		UVMin[i].V = y+0.5f;
		UVMax[i].U = x+AllMaps[i]->getWidth()-0.5f;
		UVMax[i].V = y+AllMaps[i]->getHeight()-0.5f;
	}

	// Convert UV from pixel to ratio
	for (i = 0; i < (sint32)AllMapNames.size(); ++i)
	{
		UVMin[i].U = UVMin[i].U / (float)GlobalTexture.getWidth();
		UVMin[i].V = UVMin[i].V / (float)GlobalTexture.getHeight();
		UVMax[i].U = UVMax[i].U / (float)GlobalTexture.getWidth();
		UVMax[i].V = UVMax[i].V / (float)GlobalTexture.getHeight();
	}

	// Write global texture file
	SetCurrentDirectory (sExeDir);

	NLMISC::COFile outTga;
	string fmtName = ppArgs[2];
	if (fmtName.rfind('.') == string::npos)
		fmtName += ".tga";
	if (outTga.open(fmtName))
	{
		GlobalTexture.writeTGA (outTga, 32);
		outTga.close();
		outString (string("Writing tga file : ") + fmtName + "\n");
	}
	else
	{
		outString (string("ERROR: Cannot write tga file : ") + fmtName + "\n");
	}

	// Write UV text file
	fmtName = fmtName.substr(0, fmtName.rfind('.'));
	fmtName += ".txt";
	FILE *f = fopen (fmtName.c_str(), "wt");
	if (f != NULL)
	{
		for (i = 0; i < (sint32)AllMapNames.size(); ++i)
		{
			fprintf (f, "%s %f %f %f %f\n", AllMapNames[i].c_str(), UVMin[i].U, UVMin[i].V, 
											UVMax[i].U, UVMax[i].V);
		}
		fclose (f);
		outString (string("Writing UV file : ") + fmtName + "\n");
	}
	else
	{
		outString (string("ERROR: Cannot write UV file : ") + fmtName + "\n");
	}
	return 0;
}
