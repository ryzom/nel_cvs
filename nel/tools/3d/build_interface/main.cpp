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
using namespace NLMISC;

// ***************************************************************************
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

// ***************************************************************************
// test every 4 pixels for 2 reason: DXTC and speed
const	uint32	posStep= 4;

// ***************************************************************************
// Try all position to put pSrc in pDst
bool tryAllPos (NLMISC::CBitmap *pSrc, NLMISC::CBitmap *pDst, sint32 &x, sint32 &y)
{
	uint32 i, j;
	CObjectVector<uint8> &rSrcPix = pSrc->getPixels();
	CObjectVector<uint8> &rDstPix = pDst->getPixels();

	// Recalculate real size of the source (without padding to power of 2)
	uint32 nSrcWidth = pSrc->getWidth(), nSrcHeight = pSrc->getHeight();

	if (nSrcWidth > pDst->getWidth() ) return false;
	if (nSrcHeight > pDst->getHeight() ) return false;

	// For all position test if the Src plane can be put in
	for (j = 0; j <= (pDst->getHeight() - nSrcHeight); j+= posStep)
	for (i = 0; i <= (pDst->getWidth() - nSrcWidth); i+= posStep)
	{
		x = i; y = j;
		
		uint32 a, b;
		bool bCanPut = true;
		for (b = 0; b < nSrcHeight; ++b)
		{
			for (a = 0; a < nSrcWidth; ++a)
			{
				if (rDstPix[4*((x+a)+(y+b)*pDst->getWidth())+3] != 0)
				{
					bCanPut = false;
					break;
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

// ***************************************************************************
void	putPixel(uint8 *dst, uint8 *src, bool alphaTransfert)
{
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
	if (alphaTransfert)
		dst[3] = src[3];
	else
		dst[3] = 255;
}

// ***************************************************************************
bool putIn (NLMISC::CBitmap *pSrc, NLMISC::CBitmap *pDst, sint32 x, sint32 y, bool alphaTransfert=true)
{
	uint32 a, b;

	uint8 *rSrcPix = &pSrc->getPixels()[0];
	uint8 *rDstPix = &pDst->getPixels()[0];

	uint	wSrc= pSrc->getWidth();
	uint	hSrc= pSrc->getHeight();
	for (b = 0; b < hSrc; ++b)
	for (a = 0; a < wSrc; ++a)
	{
		if (rDstPix[4*((x+a)+(y+b)*pDst->getWidth())+3] != 0)
			return false;
		// write
		putPixel(rDstPix + 4*((x+a)+(y+b)*pDst->getWidth()), rSrcPix+ 4*(a+b*pSrc->getWidth()), alphaTransfert);
	}

	// DXTC compression optim: fill last column block and last row block of 4 pixels with block color (don't let black or undefined)
	uint	wSrc4= 4*((wSrc+3)/4);
	uint	hSrc4= 4*((hSrc+3)/4);
	// expand on W
	if(wSrc<wSrc4)
	{
		for(a=wSrc;a<wSrc4;a++)
		{
			for(b=0;b<hSrc4;b++)
			{
				putPixel(rDstPix + 4*((x+a)+(y+b)*pDst->getWidth()), rDstPix + 4*((x+wSrc-1)+(y+b)*pDst->getWidth()), alphaTransfert);
			}
		}
	}
	// expand on H
	if(hSrc<hSrc4)
	{
		for(b=hSrc;b<hSrc4;b++)
		{
			for(a=0;a<wSrc4;a++)
			{
				putPixel(rDstPix + 4*((x+a)+(y+b)*pDst->getWidth()), rDstPix + 4*((x+a)+(y+hSrc-1)*pDst->getWidth()), alphaTransfert);
			}
		}
	}

	return true;
}

// ***************************************************************************
string getBaseName (const string &fullname)
{
	string sTmp2 = "";
	int pos = fullname.rfind('_');
	for (int j = 0; j <= pos; ++j)
		sTmp2 += fullname[j];
	return sTmp2;
}

// ***************************************************************************
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
	
	CObjectVector<uint8> &rPixelBitmap = b2.getPixels(0);
	for (sint32 i = 0; i < nNewWidth*nNewHeight*4; ++i)
		rPixelBitmap[i] = 0;
	
	putIn (&b, &b2, 0, 0);
	b = b2;
}


// ***************************************************************************
// main
// ***************************************************************************
int main(int nNbArg, char **ppArgs)
{
	GetCurrentDirectory (MAX_PATH, sExeDir);

	if (nNbArg < 3)
	{
		outString ("ERROR : Wrong number of arguments\n");
		outString ("USAGE : build_interface <out_tga_name> <path_maps1> [path_maps2] [path_maps3] ....\n");
		return -1;
	}
	
	vector<string> AllMapNames;
	for(sint iPath=0;iPath<nNbArg-2;iPath++)
	{
		if( !CFile::isDirectory(ppArgs[2+iPath]) )
		{
			outString (string("ERROR : directory ") + ppArgs[2+iPath] + " do not exists\n");
			return -1;
		}
		CPath::getPathContent(ppArgs[2+iPath], false, false, true, AllMapNames);
	}

	vector<NLMISC::CBitmap*> AllMaps;
	sint32 i, j;

	// Load all maps
	AllMaps.resize (AllMapNames.size());
	for (i = 0; i < (sint32)AllMapNames.size(); ++i)
	{
		try
		{
			NLMISC::CBitmap *pBtmp = new NLMISC::CBitmap;
			NLMISC::CIFile inFile;
			inFile.open( AllMapNames[i] );
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
	CObjectVector<uint8> &rPixelBitmap = GlobalTexture.getPixels(0);
	rPixelBitmap[0] = rPixelBitmap[1] = rPixelBitmap[2] = rPixelBitmap[3] = 0;
	CObjectVector<uint8> &rPixelMask = GlobalMask.getPixels(0);
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
		UVMin[i].U = (float)x;
		UVMin[i].V = (float)y;
		UVMax[i].U = (float)x+AllMaps[i]->getWidth();
		UVMax[i].V = (float)y+AllMaps[i]->getHeight();

		/* // Do not remove this is usefull for debugging
		{
			NLMISC::COFile outTga;
			string fmtName = ppArgs[1];
			if (fmtName.rfind('.') == string::npos)
				fmtName += ".tga";
			if (outTga.open(fmtName))
			{
				GlobalTexture.writeTGA (outTga, 32);
				outTga.close();
			}
		}
		{
			NLMISC::COFile outTga;
			string fmtName = ppArgs[1];
			if (fmtName.rfind('.') == string::npos)
				fmtName += "_msk.tga";
			else
				fmtName = fmtName.substr(0,fmtName.rfind('.')) + "_msk.tga";
			if (outTga.open(fmtName))
			{
				GlobalMask.writeTGA (outTga, 32);
				outTga.close();
			}
		}*/


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
	string fmtName = ppArgs[1];
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
			// get the string whitout path
			string fileName= CFile::getFilename(AllMapNames[i]);
			fprintf (f, "%s %.12f %.12f %.12f %.12f\n", fileName.c_str(), UVMin[i].U, UVMin[i].V, 
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
