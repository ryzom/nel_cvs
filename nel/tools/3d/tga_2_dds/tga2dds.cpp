/** \file tga2dds.cpp
 * TGA to DDS converter
 *
 * $Id: tga2dds.cpp,v 1.5 2001/04/18 09:17:16 berenguier Exp $
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

#include "s3_intrf.h"
#include "ddraw.h"


using namespace NLMISC;
using namespace std;


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








bool sameType(const char *sFileNameDest, uint8 algo, bool wantMipMap)
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

	bool	algoOk= false;
	switch(algo)
	{
		case DXT1:
			if(h.ddpfPixelFormat.dwFourCC==MAKEFOURCC('D','X', 'T', '1')
				&& h.ddpfPixelFormat.dwRGBBitCount==0)
				algoOk=true;
			break;
					
		case DXT1A:
			if(h.ddpfPixelFormat.dwFourCC==MAKEFOURCC('D','X', 'T', '1')
				&& h.ddpfPixelFormat.dwRGBBitCount>0)
				algoOk=true;
			break;
		
		case DXT3:
			if(h.ddpfPixelFormat.dwFourCC==MAKEFOURCC('D','X', 'T', '3'))
				algoOk=true;
			break;

		case DXT5:
			if(h.ddpfPixelFormat.dwFourCC==MAKEFOURCC('D','X', 'T', '5'))
				algoOk=true;
			break;
	}
	if(!algoOk)
		return false;
	
	// Test Mipmap.
	bool	fileHasMipMap= (h.dwFlags&DDSD_MIPMAPCOUNT) && (h.dwMipMapCount>1);
	if(fileHasMipMap==wantMipMap)
		return true;

	return false;
}



bool dataCheck(const char *sFileNameSrc, const char *sFileNameDest, uint8 algo, bool wantMipMap)
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
		if(!sameType(sFileNameDest, algo, wantMipMap))
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
	cout<<"syntax : tga2dds <input.tga> [-o <output.dds>] [-a <algo>] [-m]"<<endl;
	cout<<endl;
	cout<<"with"<<endl;
	cout<<"algo : 1  for DXTC1 (no alpha)"<<endl;
	cout<<"       1A for DXTC1 with alpha"<<endl;
	cout<<"       3  for DXTC3"<<endl;
	cout<<"       5  for DXTC5"<<endl;
	cout<<"-m   : Create MipMap"<<endl;
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


// ***************************************************************************
void		compressMipMap(uint8 *pixSrc, sint width, sint height, vector<uint8>	&compdata, DDSURFACEDESC &dest, sint algo)
{
	// Filling DDSURFACEDESC structure for input
	DDSURFACEDESC src;
	memset(&src, 0, sizeof(src));
	src.dwSize = sizeof(src);
	src.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_LPSURFACE | 
				  DDSD_PITCH | DDSD_PIXELFORMAT;
	src.dwHeight = height;
	src.dwWidth = width;
	src.lPitch = width * 4;
	src.lpSurface = pixSrc;
	src.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
	src.ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
	src.ddpfPixelFormat.dwRGBBitCount = 32;
	src.ddpfPixelFormat.dwRBitMask = 0x0000ff;
	src.ddpfPixelFormat.dwGBitMask = 0x00ff00;
	src.ddpfPixelFormat.dwBBitMask = 0xff0000;
	src.ddpfPixelFormat.dwRGBAlphaBitMask = 0xff000000;

	// Filling DDSURFACEDESC structure for output
	//===========================================
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

	

	// Encoding
	//===========
	// resize dest.
	uint32 encodeSz = S3TCgetEncodeSize(&src,encodeType);
	compdata.resize(encodeSz);
	// Go!
	float weight[3] = {0.3086f, 0.6094f, 0.0820f};
	S3TCencode(&src, NULL, &dest, &(*compdata.begin()), encodeType, weight);
	
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

}


// ***************************************************************************
string		OptOutputFileName;
sint		OptAlgo= -1;
bool		OptMipMap= false;
bool	parseOptions(int argc, char **argv)
{
	for(sint i=2;i<argc;i++)
	{
		// OutputFileName.
		if(!strcmp(argv[i], "-o"))
		{
			i++;
			if(i>=argc) return false;
			OptOutputFileName= argv[i];
		}
		// Algo.
		else if(!strcmp(argv[i], "-a"))
		{
			i++;
			if(i>=argc) return false;
			if(!strcmp(argv[i],"1"))	OptAlgo = DXT1;
			else
			if(!strcmp(argv[i],"1A")) 	OptAlgo = DXT1A;
			else
			if(!strcmp(argv[i],"1a")) 	OptAlgo = DXT1A;
			else
			if(!strcmp(argv[i],"3"))	OptAlgo = DXT3;
			else
			if(!strcmp(argv[i],"5"))	OptAlgo = DXT5;
			else
			{
				cerr<<"Algorithm unknown : "<<argv[i]<<endl;
				exit(1);
			}
		}
		// MipMap.
		else if(!strcmp(argv[i], "-m"))
		{
			OptMipMap= true;
		}
		// What is this option?
		else
		{
			return false;
		}
	}

	return true;
}


// ***************************************************************************
void main(int argc, char **argv)
{
	uint8 algo;
	/*
	if(!CheckPluginVersion ("tga_2_dds\\tga2dds.exe"))
	{
		cerr<<" (!) A newer version of this program can be found on the server"<<endl;
	}
	*/

	
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
	if(!parseOptions(argc, argv))
	{
		writeInstructions();
		return;
	}


	// Reading TGA and converting to RGBA
	//====================================
	CBitmap picTga;
	CBitmap picTga2;
	CBitmap picSrc;

	
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
	if(imageDepth!=24 && imageDepth!=32 && imageDepth!=8)
	{
		cerr<<"Image not supported : "<<imageDepth<<endl;
		exit(1);
	}
	input.close();
	uint32 height = picTga.getHeight();
	uint32 width= picTga.getWidth();
	picTga.convertToType (CBitmap::RGBA);


	// Output file name and algo.
	//===========================
	std::string outputFileName;
	if(OptOutputFileName!="")
		outputFileName= OptOutputFileName;
	else
		outputFileName= getOutputFileName(inputFileName);
	// Choose Algo.
	if(OptAlgo!=-1)
	{
		algo= OptAlgo;
	}
	else
	{
		if(imageDepth==24)
			algo = DXT1;
		else
			algo = DXT5;
	}


	// Data check
	//===========
	if(dataCheck(inputFileName.c_str(),outputFileName.c_str(),algo, OptMipMap))
	{
		cout<<outputFileName<<" : a recent dds file already exists"<<endl;
		return;
	}


	

	// Vectors for RGBA data
	std::vector<uint8> RGBASrc = picTga.getPixels();
	std::vector<uint8> RGBASrc2;
	std::vector<uint8> RGBADest;
	RGBADest.reserve(height*width*4);
	



	// UserColor
	//===========
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
		picTga2.convertToType (CBitmap::RGBA);



		RGBASrc2 = picTga2.getPixels();

		NLMISC::CRGBA *pRGBASrc = (NLMISC::CRGBA*)&RGBASrc[0];
		NLMISC::CRGBA *pRGBASrc2 = (NLMISC::CRGBA*)&RGBASrc2[0];
	
		for(uint32 i = 0; i<width*height; i++)
		{
			// If no UserColor, must take same RGB, and keep same Alpha from src1 !!! So texture can have both alpha
			// userColor and other alpha usage.
			if(pRGBASrc2[i].A==255) 
			{
				RGBADest.push_back(pRGBASrc[i].R);
				RGBADest.push_back(pRGBASrc[i].G);
				RGBADest.push_back(pRGBASrc[i].B);
				RGBADest.push_back(pRGBASrc[i].A);
			}
			else
			{
				// Old code.
				/*uint8 F = (uint8) ((float)pRGBASrc[i].R*0.3 + (float)pRGBASrc[i].G*0.56 + (float)pRGBASrc[i].B*0.14);
				uint8 Frgb;
				if((F*pRGBASrc2[i].A/255)==255)
					Frgb = 0;
				else
					Frgb = (255-pRGBASrc2[i].A)/(255-F*pRGBASrc2[i].A/255);
				RGBADest.push_back(Frgb*pRGBASrc[i].R/255);
				RGBADest.push_back(Frgb*pRGBASrc[i].G/255);
				RGBADest.push_back(Frgb*pRGBASrc[i].B/255);
				RGBADest.push_back(F*pRGBASrc[i].A/255);*/

				// New code: use new restrictions from IDriver.
				float	Rt, Gt, Bt, At;
				float	Lt;
				float	Rtm, Gtm, Btm, Atm;

				// read 0-1 RGB pixel.
				Rt= (float)pRGBASrc[i].R/255;
				Gt= (float)pRGBASrc[i].G/255;
				Bt= (float)pRGBASrc[i].B/255;
				Lt= Rt*0.3f + Gt*0.56f + Bt*0.14f;

				// take Alpha from userColor src.
				At= (float)pRGBASrc2[i].A/255;
				Atm= 1-Lt*(1-At);

				// If normal case.
				if(Atm>0)
				{
					Rtm= Rt*At / Atm;
					Gtm= Gt*At / Atm;
					Btm= Bt*At / Atm;
				}
				// Else special case: At==0, and Lt==1.
				else
				{
					Rtm= Gtm= Btm= 0;
				}

				// copy to buffer.
				sint	r,g,b,a;
				r= (sint)floor(Rtm*255+0.5f);
				g= (sint)floor(Gtm*255+0.5f);
				b= (sint)floor(Btm*255+0.5f);
				a= (sint)floor(Atm*255+0.5f);
				clamp(r, 0,255);
				clamp(g, 0,255);
				clamp(b, 0,255);
				clamp(a, 0,255);
				RGBADest.push_back(r);
				RGBADest.push_back(g);
				RGBADest.push_back(b);
				RGBADest.push_back(a);
			}
		}
	}
	else
		RGBADest = RGBASrc;

	// Copy to the dest bitmap.
	picSrc.resize(width, height, CBitmap::RGBA);
	picSrc.getPixels(0)= RGBADest;
	

	// Compress
	//===========
	vector<uint8>		CompressedMipMaps;
	DDSURFACEDESC		dest;

	// log.
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


	// For all mipmaps, compress.
	if(OptMipMap)
	{
		// Build the mipmaps.
		picSrc.buildMipMaps();
	}
	for(sint mp= 0;mp<(sint)picSrc.getMipMapCount();mp++)
	{
		uint8	*pixDest;
		uint8	*pixSrc= &(*picSrc.getPixels(mp).begin());
		sint	w= picSrc.getWidth(mp);
		sint	h= picSrc.getWidth(mp);
		vector<uint8>	compdata;
		DDSURFACEDESC	temp;
		compressMipMap(pixSrc, w, h, compdata, temp, algo);
		// Copy the result of the base dds in the dest.
		if(mp==0)
			dest= temp;

		// Append this data to the global data.
		sint	delta= CompressedMipMaps.size();
		CompressedMipMaps.resize(CompressedMipMaps.size()+compdata.size());
		pixDest= &(*CompressedMipMaps.begin())+ delta;
		memcpy( pixDest, &(*compdata.begin()), compdata.size());
	}



	// Replace DDSURFACEDESC destination header by a DDSURFACEDESC2 header
	//====================================================================
	DDSURFACEDESC2 dest2;
	memset(&dest2, 0, sizeof(dest2));
	dest2.dwSize = sizeof(dest2);
	dest2.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_LPSURFACE | 
					DDSD_LINEARSIZE | DDSD_PIXELFORMAT; 
	dest2.dwHeight = dest.dwHeight;
	dest2.dwWidth = dest.dwWidth;
	dest2.dwLinearSize = dest.dwLinearSize;
	dest2.dwMipMapCount = dest.dwMipMapCount;
	dest2.dwAlphaBitDepth = dest.dwAlphaBitDepth;
	dest2.dwReserved = dest.dwReserved;
	dest2.lpSurface = dest.lpSurface;
	dest2.ddpfPixelFormat = dest.ddpfPixelFormat;
	// Setting Nb MipMap.
	dest2.dwFlags|= DDSD_MIPMAPCOUNT;
	dest2.dwMipMapCount= picSrc.getMipMapCount();


	// Saving DDS file
	//=================
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
	
	output.serialBuffer(&(*CompressedMipMaps.begin()), CompressedMipMaps.size());

	
	output.close();

}	










	
