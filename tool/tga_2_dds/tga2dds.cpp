/** \file tga2dds.cpp
 * TGA to DDS converter
 *
 * $Id: tga2dds.cpp,v 1.2 2000/11/06 15:12:43 coutelas Exp $
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
#include <iostream.h>

#include "nel/misc/file.h"
#include "nel/3d/bitmap.h"
#include "nel/misc/file.h"
#include "nel/misc/debug.h"

#include "s3_intrf.h"
#include "ddraw.h"



#define	DXT1	1
#define	DXT1A	11
#define	DXT3	3
#define	DXT5	5



extern BOOL CheckPluginVersion (const char* sServerPluginFileName);
bool sameType(const char *sFileNameDest, uint8 algo);
bool dataCheck(const char *sFileNameSrc, const char *sFileNameDest, uint8 algo);
std::string getOutputFileName(std::string inputFileName);
void writeInstructions();
void main(int argc, char **argv);








bool sameType(const char *sFileNameDest, uint8 algo)
{
	uint32 dds;
	FILE *f = fopen(sFileNameDest,"rb");
	if(f==NULL)
	{
		return false;
	}
	DDSURFACEDESC2 h;
	fread(&dds,1,4,f);
	fread(&h,sizeof(DDSURFACEDESC2),1,f);
	if(fclose(f))
	{
		cerr<<sFileNameDest<< "is not closed"<<endl;
	}

	switch(algo)
	{
		case DXT1:
			if(h.ddpfPixelFormat.dwFourCC==MAKEFOURCC('D','X', 'T', '1')
				&& h.ddpfPixelFormat.dwRGBBitCount==0)
				return true;
			break;
					
		case DXT1A:
			if(h.ddpfPixelFormat.dwFourCC==MAKEFOURCC('D','X', 'T', '1')
				&& h.ddpfPixelFormat.dwRGBBitCount>0)
				return true;
			break;
		
		case DXT3:
			if(h.ddpfPixelFormat.dwFourCC==MAKEFOURCC('D','X', 'T', '3'))
				return true;
			break;

		case DXT5:
			if(h.ddpfPixelFormat.dwFourCC==MAKEFOURCC('D','X', 'T', '5'))
				return true;
			break;
	}
	
	return false;
}



bool dataCheck(const char *sFileNameSrc, const char *sFileNameDest, uint8 algo)
{
	
	HANDLE h1 = CreateFile( sFileNameSrc, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(h1 == INVALID_HANDLE_VALUE)
	{
		cerr<<"Can't open file "<<sFileNameSrc<<endl;
		exit(1);
	}
	
	HANDLE h2 = CreateFile( sFileNameDest, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h2==INVALID_HANDLE_VALUE)
	{
		return false; // destination file doesn't exist yet
	}
	
	uint8 res;
	FILETIME Tmp;
	FILETIME lpLastWriteTime1;
	FILETIME lpLastWriteTime2;
	
	res = GetFileTime(h1, &Tmp, &Tmp, &lpLastWriteTime1);
	nlassert(res);
	
	res = GetFileTime(h2, &Tmp, &Tmp, &lpLastWriteTime2);
	nlassert(res);
	
	uint32 nComp = CompareFileTime(&lpLastWriteTime1, &lpLastWriteTime2);

	CloseHandle(h1);
	CloseHandle(h2);
	
	if(nComp==+1)
	{
		return false;
	}
	if(nComp==-1)
	{
		if(!sameType(sFileNameDest, algo))
		{
			return false; // file exists but a new compression type is required
		}
		return true;
	}
	return true;
}


void writeInstructions()
{
	cout<<endl;
	cout<<"TGA2DDS"<<endl;
	cout<<"  Convert TGA image file (24bits or 32 bits) to DDS compressed file using"<<endl;
	cout<<"S3TC compression (DXTC1, DXTC1 with alpha, DXTC3, or DXTC5). "<<endl;
	cout<<"  The program looks for possible user color files and load them automatically,"<<endl;
	cout<<"a user color file must have the same name that the original tga file, plus the"<<endl;
	cout<<"extension \"_usercolor\""<<endl;
	cout<<"ex : pic.tga, the associated user color file must be : pic_usercolor.tga"<<endl; 
	cout<<endl;
	cout<<"syntax : tga2dds <input.tga> [<output.dds>] [<algo>] "<<endl;
	cout<<endl;
	cout<<"with"<<endl;
	cout<<"algo : 1  for DXTC1 (no alpha)"<<endl;
	cout<<"       1A for DXTC1 with alpha"<<endl;
	cout<<"       3  for DXTC3"<<endl;
	cout<<"       5  for DXTC3"<<endl;
	cout<<endl;
	cout<<"default : DXTC1 if Tga 24b, DXTC5 if Tga 32b."<<endl;
	cout<<endl;
	cout<<"/? for this help"<<endl;
	cout<<endl; 
}


std::string getOutputFileName(std::string inputFileName)
{
	uint8 pos = inputFileName.rfind(".tga");
	if(pos>=inputFileName.length()) // name whithout extension ".tga"
		return (inputFileName + ".dds");
	else
		return (inputFileName.substr(0,pos) + ".dds");
}


void main(int argc, char **argv)
{
	uint8 algo;
	/*
	if(!CheckPluginVersion ("tga_2_dds\\tga2dds.exe"))
	{
		cerr<<" (!) A newer version of this program can be found on the server"<<endl;
	}
	*/

	
	if(argc<2)
	{
		writeInstructions();
		return;
	}
	if(strcmp(argv[1],"/?")==0)
	{
		writeInstructions();
		return;
	}

	NL3D::CBitmap picTga;
	NL3D::CBitmap picTga2;

	
	// Reading TGA and converting to RGBA
	std::string inputFileName(argv[1]);
	if(inputFileName.find("_usercolor")<inputFileName.length())
	{
		return;
	}
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
	if(imageDepth!=24 && imageDepth!=32)
	{
		cerr<<"Image not supported : "<<imageDepth<<endl;
		exit(1);
	}
	input.close();
	uint32 height = picTga.getHeight();
	uint32 width= picTga.getWidth();
	picTga.convertToRGBA();



	// Output file name & algo
	std::string outputFileName;
	if(argc==2)
	{
		outputFileName = getOutputFileName(inputFileName);
		
		if(imageDepth==24)
			algo = DXT1;
		else
			algo = DXT5;
	}
	if(argc==3)
	{
		if(strcmp(argv[2],"1") == 0) 
		{
			algo = DXT1;
			outputFileName = getOutputFileName(inputFileName);
		}
		else
		if(strcmp(argv[2],"1A") == 0) 
		{
			algo = DXT1A;
			outputFileName = getOutputFileName(inputFileName);
		}
		else
		if(strcmp(argv[2],"3") == 0)
		{
			algo = DXT3;
			outputFileName = getOutputFileName(inputFileName);
		}
		else
		if(strcmp(argv[2],"5") == 0) 
		{
			algo = DXT5;
			outputFileName = getOutputFileName(inputFileName);
		}
		else
		{
			outputFileName = argv[2];
			if(imageDepth==24)
				algo = DXT1;
			else
				algo = DXT5;
		}
	}
	if(argc==4)
	{
		if(strcmp(argv[3],"1") == 0)	algo = DXT1;
		else
		if(strcmp(argv[3],"1A") == 0) 	algo = DXT1A;
		else
		if(strcmp(argv[3],"3") == 0)	algo = DXT3;
		else
		if(strcmp(argv[3],"5") == 0)	algo = DXT5;
		else
		{
			cerr<<"Algorithm unknown : "<<argv[3]<<endl;
			exit(1);
		}
		outputFileName = argv[3];
	}
			
	

	// Data check
	if(dataCheck(inputFileName.c_str(),outputFileName.c_str(),algo))
	{
		cout<<outputFileName<<" : a recent dds file already exists"<<endl;
		return;
	}


	

	// Vectors for RGBA data
	std::vector<uint8> RGBASrc = picTga.getPixels();
	std::vector<uint8> RGBASrc2;
	std::vector<uint8> RGBADest;
	RGBADest.reserve(height*width*4);
	



	/*
	// Checking if option "usercolor" has been used
	std::string userColorFileName;
	if(argc>4)
	{
		if(strcmp("-usercolor",argv[4])==0)
		{
			if(argc!=6)
			{
				writeInstructions();
				return;
			}
			userColorFileName = argv[5];
		}
		else
		{
			writeInstructions();
			return;
		}
	}
	*/
	// Checking if associate usercolor file  exists
	std::string userColorFileName;
	uint8 pos = inputFileName.rfind(".tga");
	if(pos>=inputFileName.length()) // name whithout extension ".tga"
	{
		pos = inputFileName.length();
		userColorFileName = inputFileName.substr(0,pos) + "_usercolor"; 
	}
	else
		userColorFileName = inputFileName.substr(0,pos) + "_usercolor.tga";
	
	
	// Reading second Tga for user color
	NLMISC::CIFile input2;
	if(input2.open(userColorFileName))
	{
		picTga2.load(input2);
		uint32 height2 = picTga2.getHeight();
		uint32 width2 = picTga2.getWidth();
		nlassert(width2==width);
		nlassert(height2==height);
		picTga2.convertToRGBA();
		RGBASrc2 = picTga2.getPixels();

		NLMISC::CRGBA *pRGBASrc = (NLMISC::CRGBA*)&RGBASrc[0];
		NLMISC::CRGBA *pRGBASrc2 = (NLMISC::CRGBA*)&RGBASrc2[0];
	
		for(uint32 i = 0; i<width*height; i++)
		{
			if(pRGBASrc2[i].A==0) 
			{
				RGBADest.push_back(pRGBASrc[i].R);
				RGBADest.push_back(pRGBASrc[i].G);
				RGBADest.push_back(pRGBASrc[i].B);
				RGBADest.push_back(pRGBASrc[i].A);
			}
			else
			{
				uint8 F = (uint8) ((float)pRGBASrc[i].R*0.3 + (float)pRGBASrc[i].G*0.56 + (float)pRGBASrc[i].B*0.14);
				uint8 Frgb;
				if((F*pRGBASrc2[i].A/255)==255)
					Frgb = 0;
				else
					Frgb = (255-pRGBASrc2[i].A)/(255-F*pRGBASrc2[i].A/255);
				RGBADest.push_back(Frgb*pRGBASrc[i].R/255);
				RGBADest.push_back(Frgb*pRGBASrc[i].G/255);
				RGBADest.push_back(Frgb*pRGBASrc[i].B/255);
				RGBADest.push_back(F*pRGBASrc[i].A/255);
			}
		}
	}
	else
		RGBADest = RGBASrc;
	

	// Filling DDSURFACEDESC structure for input
	DDSURFACEDESC src;
	memset(&src, 0, sizeof(src));
	src.dwSize = sizeof(src);
	src.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_LPSURFACE | 
				  DDSD_PITCH | DDSD_PIXELFORMAT;
	src.dwHeight = height;
	src.dwWidth = width;
	src.lPitch = width * 4;
	src.lpSurface = &(*RGBADest.begin());
	src.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
	src.ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
	src.ddpfPixelFormat.dwRGBBitCount = 32;
	src.ddpfPixelFormat.dwRBitMask = 0x0000ff;
	src.ddpfPixelFormat.dwGBitMask = 0x00ff00;
	src.ddpfPixelFormat.dwBBitMask = 0xff0000;
	src.ddpfPixelFormat.dwRGBAlphaBitMask = 0xff000000;


	// DDSURFACEDESC structure for output
	DDSURFACEDESC dest;
	memset(&dest, 0, sizeof(dest));
	dest.dwSize = sizeof(dest);
	
	
	// Setting encode type
	uint32 encodeType = 0;
	switch(algo)
	{
		case DXT1:
			encodeType = S3TC_ENCODE_RGB_FULL | S3TC_ENCODE_ALPHA_NONE;
			dest.dwLinearSize = width * height / 2; // required by S3TCTool
			break;
		case DXT1A:
			encodeType = S3TC_ENCODE_RGB_FULL | S3TC_ENCODE_RGB_ALPHA_COMPARE;
			S3TCsetAlphaReference(127); // set the threshold to 0.5
			dest.dwLinearSize = width * height / 2; // required by S3TCTool
			break;
		case DXT3:
			encodeType = S3TC_ENCODE_RGB_FULL | S3TC_ENCODE_ALPHA_EXPLICIT;
			dest.dwLinearSize = width * height; // required by S3TCTool
			break;
		case DXT5:
			encodeType = S3TC_ENCODE_RGB_FULL | S3TC_ENCODE_ALPHA_INTERPOLATED;
			dest.dwLinearSize = width * height; // required by S3TCTool
			break;
	}

	std::string algostr;
	switch(algo)
	{
		case DXT1:
			algostr = "DXTC1";
			break;
		case DXT1A:
			algostr = "DXTC1A";
			break;
		case DXT3:
			algostr = "DXTC3";
			break;
		case DXT5:
			algostr = "DXTC5";
			break;
	}
	cout<<"compressing ("<<algostr<<") "<<inputFileName<<" to "<<outputFileName<<endl;
	uint32 encodeSz = S3TCgetEncodeSize(&src,encodeType);
	uint8 * pDestBuf = new uint8[encodeSz];
	float weight[3] = {0.3086f, 0.6094f, 0.0820f};
	

	// Encoding
	S3TCencode(&src, NULL, &dest, pDestBuf, encodeType, weight);
	
	switch(algo)
	{
		case DXT1:
			dest.ddpfPixelFormat.dwFourCC = MAKEFOURCC('D','X', 'T', '1');
			break;
		case DXT1A:
			dest.ddpfPixelFormat.dwFourCC = MAKEFOURCC('D','X', 'T', '1');
			break;
		case DXT3:
			dest.ddpfPixelFormat.dwFourCC = MAKEFOURCC('D','X', 'T', '3');
			break;
		case DXT5:
			dest.ddpfPixelFormat.dwFourCC = MAKEFOURCC('D','X', 'T', '5');
			break;
	}


	// Replace DDSURFACEDESC destination header by a DDSURFACEDESC2 header
	DDSURFACEDESC2 dest2;
	memset(&dest2, 0, sizeof(dest2));
	dest2.dwSize = sizeof(dest2);
	dest2.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_LPSURFACE | 
					DDSD_LINEARSIZE | DDSD_PIXELFORMAT; 
	dest2.dwHeight = dest.dwHeight;
	dest2.dwWidth = dest.dwWidth;
	dest2.lPitch = dest.lPitch;
	dest2.dwLinearSize = dest.dwLinearSize;
	dest2.dwBackBufferCount = dest.dwBackBufferCount;
	dest2.dwMipMapCount = dest.dwMipMapCount;
	dest2.dwRefreshRate = dest.dwRefreshRate;
	dest2.dwAlphaBitDepth = dest.dwAlphaBitDepth;
	dest2.dwReserved = dest.dwReserved;
	dest2.lpSurface = dest.lpSurface;
	dest2.ddpfPixelFormat = dest.ddpfPixelFormat;


	// Saving DDS file
	NLMISC::COFile output;
	if(!output.open(outputFileName))
	{
		cerr<<"Can't open output file "<<outputFileName<<endl;
		exit(1);
	}
	output.serialBuffer((uint8*)std::string("DDS ").c_str(),4);
	try 
	{
		uint8 * pDest2 = (uint8*) &dest2;
		output.serialBuffer(pDest2, sizeof(dest2));
	}
	catch(NLMISC::EWriteError &e)
	{
		cerr<<e.what()<<endl;
		exit(1);
	}
	
	for(uint32 i=0; i<encodeSz; i++)
	{
		output.serial(pDestBuf[i]);
	}
	
	output.close();

}	










	
