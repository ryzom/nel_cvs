/** \file tga_cut.cpp
 * TGA to DDS converter
 *
 * $Id: tga_cut.cpp,v 1.1 2003/07/04 15:24:43 meyrignac Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
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
#include <iostream>

#include "nel/misc/file.h"
#include "nel/misc/bitmap.h"
#include "nel/misc/file.h"
#include "nel/misc/debug.h"
#include <math.h>
#include "ddraw.h"

using namespace NLMISC;
using namespace std;

#define	TGA16	16
#define NOT_DEFINED 0xff

const int CutSize = 160;

void writeInstructions();
void main(int argc, char **argv);

// ***************************************************************************

bool getZoneNameFromXY (sint32 x, sint32 y, std::string &zoneName)
{
	if ((y>0) || (y<-255) || (x<0) || (x>255))
		return false;
	zoneName = toString(-y) + "_";
	zoneName += ('A' + (x/26));
	zoneName += ('A' + (x%26));
	return true;
}

void writeInstructions()
{
	cout<<endl;
	cout<<"tga_cut"<<endl;
	cout<<"  Cut TGA image file (24bits or 32 bits) into smaller TGA"<<endl;
	cout<<"syntax : tga_cut <input.tga>"<<endl;
	cout<<endl;
	cout<<"/? for this help"<<endl;
	cout<<endl; 
}

// ***************************************************************************
void dividSize (CBitmap &bitmap)
{
	// Must be RGBA
	nlassert (bitmap.getPixelFormat () == CBitmap::RGBA);

	// Copy the bitmap
	CBitmap temp = bitmap;

	// Resize the destination
	const uint width = temp.getWidth ();
	const uint height = temp.getHeight ();
	const uint newWidth = temp.getWidth ()/2;
	const uint newHeight = temp.getHeight ()/2;
	bitmap.resize (newWidth, newHeight, CBitmap::RGBA);

	// Pointers
	uint8 *pixelSrc = &(temp.getPixels ()[0]);
	uint8 *pixelDest = &(bitmap.getPixels ()[0]);

	// Resample
	uint x, y;
	for (y=0; y<newHeight; y++)
	for (x=0; x<newWidth; x++)
	{
		const uint offsetSrc = ((y*2)*width+x*2)*4;
		const uint offsetDest = (y*newWidth+x)*4;
		uint i;
		for (i=0; i<4; i++)
		{
			pixelDest[offsetDest+i] = ((uint)pixelSrc[offsetSrc+i] + (uint)pixelSrc[offsetSrc+4+i] + 
				(uint)pixelSrc[offsetSrc+4*width+i] + (uint)pixelSrc[offsetSrc+4*width+4+i])>>2;
		}
	}
}

// ***************************************************************************
void main(int argc, char **argv)
{
	// Parse Command Line.
	//====================
	if(argc<2)
	{
		writeInstructions();
		return;
	}
	if(!strcmp(argv[1],"/?"))
	{
		writeInstructions();
		return;
	}
	if(!strcmp(argv[1],"-?"))
	{
		writeInstructions();
		return;
	}
	if(argc != 2)
	{
		writeInstructions();
		return;
	}

	// Reading TGA and converting to RGBA
	//====================================
	CBitmap picTga;
	CBitmap picSrc;

	std::string inputFileName(argv[1]);
	NLMISC::CIFile input;
	if(!input.open(inputFileName))
	{
		cerr<<"Can't open input file "<<inputFileName<<endl;
		exit(1);
	}
	uint8 imageDepth = picTga.load(input);
	if(imageDepth==0)
	{
		cerr<<"Can't load file : "<<inputFileName<<endl;
		exit(1);
	}
	if(imageDepth!=16 && imageDepth!=24 && imageDepth!=32 && imageDepth!=8)
	{
		cerr<<"Image not supported : "<<imageDepth<<endl;
		exit(1);
	}
	input.close();
	sint32 height = picTga.getHeight();
	sint32 width= picTga.getWidth();
	picTga.convertToType (CBitmap::RGBA);


	// Vectors for RGBA data
	CObjectVector<uint8> RGBASrc = picTga.getPixels();
	CObjectVector<uint8> RGBASrc2;
	CObjectVector<uint8> RGBADest;
	RGBADest.resize(CutSize*CutSize*4);
	uint	dstRGBADestId= 0;

	// Copy to the dest bitmap.
	picSrc.resize(CutSize, CutSize, CBitmap::RGBA);
	picSrc.getPixels(0) = RGBADest;

	// Must be RGBA
	nlassert (picSrc.getPixelFormat () == CBitmap::RGBA);

	// Pointers
	uint8 *pixelSrc = &(picTga.getPixels ()[0]);
	uint8 *pixelDest = &(picSrc.getPixels ()[0]);
	
	// Resample
	sint xzone, yzone;
	for (yzone = 0; yzone < height; yzone += CutSize)
	{
		for (xzone = 0; xzone < width; xzone += CutSize)
		{
			sint x, y;
			for (y=0; y<CutSize; y++)
			{
				for (x=0; x<CutSize; x++)
				{
					const uint offsetSrc = ((y+yzone)*width+x+xzone)*4;
					const uint offsetDest = (y*CutSize+x)*4;
					uint i;
					if (x+xzone>= width || y+yzone>=height)
					{
						// black outside the bitmap
						for (i=0; i<4; i++)
						{
							pixelDest[offsetDest+i] = 0;
						}
					}
					else
					{
						for (i=0; i<4; i++)
						{
							pixelDest[offsetDest+i] = pixelSrc[offsetSrc+i];
						}
					}
				}
			}
			int empty = 1;
			for (x=0;x<CutSize*CutSize*4;x+=4)
			{
				// tests R,G,B (omit A)
				if (pixelDest[x] || pixelDest[x+1] || pixelDest[x+2])
				{
					empty = 0;
					break;
				}
			}
			if (empty) continue;

			// if the picture is empty, we don't save it !!!
			
			NLMISC::COFile output;

			string ZoneName;
			if (!getZoneNameFromXY(xzone/CutSize, -yzone/CutSize, ZoneName))
			{
				cerr<<"Too large image"<<endl;
				exit(1);
			}

			ZoneName += ".tga";

			if(!output.open(ZoneName))
			{
				cerr<<"Can't open output file "<<ZoneName<<endl;
				exit(1);
			}

			// Saving TGA file
			try 
			{
				picSrc.writeTGA (output, 16);
			}
			catch(NLMISC::EWriteError &e)
			{
				cerr<<e.what()<<endl;
				exit(1);
			}
			
			output.close();
		}
	}
}	
