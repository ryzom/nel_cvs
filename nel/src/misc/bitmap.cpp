/** \file bitmap.cpp
 * Class managing bitmaps
 *
 * \todo yoyo: readDDS and decompressDXTC* must wirk in BigEndifan and LittleEndian.
 *
 * $Id: bitmap.cpp,v 1.29 2002/10/10 12:44:29 berenguier Exp $
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

#include "stdmisc.h"

#include <memory>
#include <algorithm>

#include "nel/misc/bitmap.h"
#include "nel/misc/stream.h"
#include "nel/misc/file.h"


using namespace std;

namespace NLMISC
{


struct EDDSBadHeader : public NLMISC::EStream
{
	EDDSBadHeader() : EStream( "Bad or unrecognized DDS file header" ) {}
};

struct ESeekFailed : public NLMISC::EStream
{
	ESeekFailed() : EStream( "Seek failed" ) {}
};

struct EAllocationFailure : public Exception
{
	EAllocationFailure() : Exception( "Can't allocate memory" ) {}
};

void blendFromui(NLMISC::CRGBA &c0, NLMISC::CRGBA &c1, uint coef);
uint32 blend(uint32 &n0, uint32 &n1, uint32 coef0);

const uint32 CBitmap::bitPerPixels[ModeCount]=
{
	32,		// RGBA
	8,		// Luminance
	8,		// Alpha
	16,		// AlphaLuminance
	4,		// DXTC1
	4,		// DXTC1Alpha
	8,		// DXTC3
	8,		// DXTC5
	16		// DsDt
};

const uint32 CBitmap::DXTC1HEADER = NL_MAKEFOURCC('D','X', 'T', '1');
const uint32 CBitmap::DXTC3HEADER = NL_MAKEFOURCC('D','X', 'T', '3');
const uint32 CBitmap::DXTC5HEADER = NL_MAKEFOURCC('D','X', 'T', '5');

/*-------------------------------------------------------------------*\
								load		
\*-------------------------------------------------------------------*/
uint8 CBitmap::load(NLMISC::IStream &f, uint mipMapSkip) 
{
	nlassert(f.isReading()); 
	
	// testing if DDS
	uint32 fileType = 0;;
	f.serial(fileType);
	if(fileType == DDS)
	{
		return readDDS(f, mipMapSkip);
	}
	// assuming it's TGA
	else 
	{
		NLMISC::IStream::TSeekOrigin origin= f.begin;
		if(!f.seek (0, origin))
		{
			throw ESeekFailed();
		}

		// Reading header, 
		// To make sure that the bitmap is TGA, we check imageType and imageDepth.
		uint8	lengthID;
		uint8	cMapType;
		uint8	imageType;
		uint16	tgaOrigin;
		uint16	length;
		uint8	depth;
		uint16	xOrg;
		uint16	yOrg;
		uint16	width;
		uint16	height;
		uint8	imageDepth;
		uint8	desc;
		
		f.serial(lengthID);
		f.serial(cMapType);
		f.serial(imageType);
		if(imageType!=2 && imageType!=3 && imageType!=10 && imageType!=11) return 0;
		f.serial(tgaOrigin);
		f.serial(length);
		f.serial(depth);
		f.serial(xOrg);
		f.serial(yOrg);
		f.serial(width);
		f.serial(height);
		f.serial(imageDepth);
		if(imageDepth!=8 && imageDepth!=16 && imageDepth!=24 && imageDepth!=32) return 0;
		f.serial(desc);

		if(!f.seek (0, origin))
		{
			throw ESeekFailed();
		}
		return readTGA(f);
	}	
}


/*-------------------------------------------------------------------*\
								makeDummy		
\*-------------------------------------------------------------------*/
void	CBitmap::makeDummy()
{
	static	const uint8	bitmap[1024]= {  
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,
		0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,
		0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,
		0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,
		0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,
		0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,
		0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,
		0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,
		0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,
		0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,
		0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	}; 

	PixelFormat = RGBA;
	_MipMapCount = 1;
	_Width= 32;
	_Height= 32;
	_Data[0].resize(_Width*_Height*sizeof(NLMISC::CRGBA));
	NLMISC::CRGBA	*pix= (NLMISC::CRGBA*)(&(*_Data[0].begin()));

	for(sint i=0;i<(sint)(_Width*_Height);i++)
	{
		if(bitmap[i])
			pix[i].set(255,255,255,255);
		else
			pix[i].set(0,0,0,255);
	}

}




/*-------------------------------------------------------------------*\
								readDDS		
\*-------------------------------------------------------------------*/
uint8 CBitmap::readDDS(NLMISC::IStream &f, uint mipMapSkip)
{
	uint32 i;

	//------------------ Reading Header ------------------------

	//-------------- reading entire header
	
	uint32 size = 0;
	f.serial(size); // size in Bytes of header(without "DDS")
	 uint32 * _DDSSurfaceDesc = new uint32[size]; 
	 std::auto_ptr<uint32> _DDSSurfaceDescAuto(_DDSSurfaceDesc);
	_DDSSurfaceDesc[0]= size;

	for(i= 0; i<size/4 - 1; i++)
	{
		f.serial(_DDSSurfaceDesc[i+1]);
	}
	
	// flags determines which members of the header structure contain valid data
	uint32 flags = _DDSSurfaceDesc[1];

	//verify if file have linearsize set
	if(!(flags & DDSD_LINEARSIZE)) 
    {
		throw EDDSBadHeader();
	}
	
	//-------------- extracting and testing useful info

	_Height = _DDSSurfaceDesc[2];
	_Width  = _DDSSurfaceDesc[3];
	_MipMapCount= (uint8) _DDSSurfaceDesc[6];
	// If no mipmap.
	if(_MipMapCount==0)
		_MipMapCount=1;
	switch (_DDSSurfaceDesc[20])
	{
	case DXTC1HEADER:
		PixelFormat=DXTC1;
		break;
	case DXTC3HEADER:
		PixelFormat=DXTC3;
		break;
	case DXTC5HEADER:
		PixelFormat=DXTC5;
		break;
	}
	
	flags = _DDSSurfaceDesc[19]; //PixelFormat flags
	
	if(PixelFormat==DXTC1 && _DDSSurfaceDesc[21]>0) //AlphaBitDepth
	{
		PixelFormat = DXTC1Alpha;
	}
	
	if(PixelFormat!= DXTC1 && PixelFormat!= DXTC1Alpha && PixelFormat!= DXTC3 && PixelFormat!= DXTC5)
	{
		throw EDDSBadHeader();
	}
	
	if((_Width%4!=0) || (_Height%4!=0)) return 0;
	

	//------------- manage mipMapSkip 
	if(_MipMapCount>1 && mipMapSkip>0)
	{
		// Keep at least the level where width and height are at leat 4.
		uint	minLevel= min(_Width, _Height);
		minLevel= getPowerOf2(minLevel);
		mipMapSkip= min(mipMapSkip, minLevel-2);
		// skip any mipmap
		uint	seekSize= 0;
		while(mipMapSkip>0)
		{
			uint32 mipMapSz;
			if(PixelFormat==DXTC1 || PixelFormat==DXTC1Alpha)
				mipMapSz = _Width*_Height/2;
			else
				mipMapSz = _Width*_Height;

			// add to how many to skip
			seekSize+= mipMapSz;

			// Size of final bitmap is reduced.
			_Width>>=1;
			_Height>>=1;
			_MipMapCount--;
			mipMapSkip--;
		}
		// skip data in file
		if(seekSize>0)
		{
			if(!f.seek(seekSize, IStream::current))
			{
				throw ESeekFailed();
			}
		}

	}

	//------------- reading mipmap levels compressed data
	
	uint32 w = _Width;
	uint32 h = _Height;

	for(uint8 m= 0; m<_MipMapCount; m++)
	{
		uint32 wtmp, htmp;
		if(w<4)
			wtmp = 4;
		else
			wtmp = w;
		if(h < 4)
			htmp = 4;
		else
			htmp = h;
		
		uint32 mipMapSz;
		if(PixelFormat==DXTC1 || PixelFormat==DXTC1Alpha)
			mipMapSz = wtmp*htmp/2;
		else
			mipMapSz = wtmp*htmp;


		_Data[m].resize(mipMapSz);
		f.serialBuffer(&(*_Data[m].begin()), mipMapSz);

	  	w = (w+1)/2;
		h = (h+1)/2;
	}

	switch(PixelFormat)
	{
		case DXTC1  : return 24;
		case DXTC1Alpha : return 32;
		case DXTC3  : return 32;
		case DXTC5  : return 32;
		default  : break;
	}

	return 0;
}




/*-------------------------------------------------------------------*\
							convertToDXTC5
\*-------------------------------------------------------------------*/
bool CBitmap::convertToDXTC5()
{
	/* Yoyo: RGB encoding for DXTC1 and DXTC5/3 are actually different!!
		DXTC3/5 don't rely on sign of color0>color1 to setup special encoding (ie use a special compression for Black)
		Since this can arise if the src is DXTC1 , we can't simply compress it into DXTC5 without doing a 
		heavy compression...
		(the inverse is false: DXTC5 to DXTC1 is possible, with maybe swap color0/color1 and bits).
	*/

	return false;

/*	uint32 i,j;

	if(PixelFormat!=DXTC1) return false;

	for(uint8 m= 0; m<_MipMapCount; m++)
	{
		std::vector<uint8> dataTmp;
		dataTmp.reserve(2*_Data[m].size());

		for(i=0; i<_Data[m].size(); i+=8)
		{
			//64 bits alpha
			for(j=0; j<8; j++)
			{
				dataTmp.push_back(255);
			}

			//64 bits RGB
			for(j=0; j<8; j++)
			{
				dataTmp.push_back(_Data[m][i+j]);
			}
		}
		_Data[m] = dataTmp;
	}
	PixelFormat = DXTC5;
	return true;
*/
}



/*-------------------------------------------------------------------*\
							luminanceToRGBA()
\*-------------------------------------------------------------------*/
bool CBitmap::luminanceToRGBA()
{
	uint32 i;

	if(_Width*_Height == 0)  return false;
	
	for(uint8 m= 0; m<_MipMapCount; m++)
	{
		std::vector<uint8> dataTmp;
		dataTmp.reserve(_Data[m].size()*4);

		for(i=0; i<_Data[m].size(); i++)
		{
			dataTmp.push_back(_Data[m][i]);
			dataTmp.push_back(_Data[m][i]);
			dataTmp.push_back(_Data[m][i]);
			dataTmp.push_back(255);
		}
		_Data[m] = dataTmp;
	}
	PixelFormat = RGBA;
	return true;
}

/*-------------------------------------------------------------------*\
							alphaToRGBA()
\*-------------------------------------------------------------------*/
bool CBitmap::alphaToRGBA()
{
	uint32 i;

	if(_Width*_Height == 0)  return false;
	
	for(uint8 m= 0; m<_MipMapCount; m++)
	{
		std::vector<uint8> dataTmp;
		dataTmp.reserve(_Data[m].size()*4);

		for(i=0; i<_Data[m].size(); i++)
		{
			dataTmp.push_back(255);
			dataTmp.push_back(255);
			dataTmp.push_back(255);
			dataTmp.push_back(_Data[m][i]);
		}
		_Data[m] = dataTmp;
	}
	PixelFormat = RGBA;
	return true;
}


/*-------------------------------------------------------------------*\
							alphaLuminanceToRGBA()
\*-------------------------------------------------------------------*/
bool CBitmap::alphaLuminanceToRGBA()
{
	uint32 i;

	if(_Width*_Height == 0)  return false;
	
	for(uint8 m= 0; m<_MipMapCount; m++)
	{
		std::vector<uint8> dataTmp;
		dataTmp.reserve(_Data[m].size()*2);

		for(i=0; i<_Data[m].size(); i+=2)
		{
			dataTmp.push_back(_Data[m][i]);
			dataTmp.push_back(_Data[m][i]);
			dataTmp.push_back(_Data[m][i]);
			dataTmp.push_back(_Data[m][i+1]);
		}
		_Data[m] = dataTmp;
	}
	PixelFormat = RGBA;
	return true;
}




/*-------------------------------------------------------------------*\
							rgbaToAlphaLuminance
\*-------------------------------------------------------------------*/
bool CBitmap::rgbaToAlphaLuminance()
{
	uint32 i;

	if(_Width*_Height == 0)  return false;
	
	for(uint8 m= 0; m<_MipMapCount; m++)
	{
		std::vector<uint8> dataTmp;
		dataTmp.reserve(_Data[m].size()/2);

		for(i=0; i<_Data[m].size(); i+=4)
		{
			dataTmp.push_back((_Data[m][i]*77 + _Data[m][i+1]*150 + _Data[m][i+2]*28)/255);
			dataTmp.push_back(_Data[m][i+3]);
		}
		NLMISC::contReset(_Data[m]); 
		_Data[m].resize(0);
		_Data[m] = dataTmp;
	}
	PixelFormat = AlphaLuminance;
	return true;
}


/*-------------------------------------------------------------------*\
							luminanceToAlphaLuminance
\*-------------------------------------------------------------------*/
bool CBitmap::luminanceToAlphaLuminance()
{
	uint32 i;

	if(_Width*_Height == 0)  return false;
		
	for(uint8 m= 0; m<_MipMapCount; m++)
	{
		std::vector<uint8> dataTmp;
		dataTmp.reserve(_Data[m].size()*2);

		for(i=0; i<_Data[m].size(); i++)
		{
			dataTmp.push_back(_Data[m][i]);
			dataTmp.push_back(255);
		}
		_Data[m] = dataTmp;
	}
	PixelFormat = AlphaLuminance;
	return true;
}



/*-------------------------------------------------------------------*\
							alphaToAlphaLuminance
\*-------------------------------------------------------------------*/
bool CBitmap::alphaToAlphaLuminance()
{
	uint32 i;

	if(_Width*_Height == 0)  return false;
		
	for(uint8 m= 0; m<_MipMapCount; m++)
	{
		std::vector<uint8> dataTmp;
		dataTmp.reserve(_Data[m].size()*2);

		for(i=0; i<_Data[m].size(); i++)
		{
			dataTmp.push_back(0);
			dataTmp.push_back(_Data[m][i]);
		}
		_Data[m] = dataTmp;
	}
	PixelFormat = AlphaLuminance;
	return true;
}



/*-------------------------------------------------------------------*\
							rgbaToLuminance
\*-------------------------------------------------------------------*/
bool CBitmap::rgbaToLuminance()
{
	uint32 i;

	if(_Width*_Height == 0)  return false;
		
	for(uint8 m= 0; m<_MipMapCount; m++)
	{
		std::vector<uint8> dataTmp;
		dataTmp.reserve(_Data[m].size()/4);

		for(i=0; i<_Data[m].size(); i+=4)
		{
			dataTmp.push_back((_Data[m][i]*77 + _Data[m][i+1]*150 + _Data[m][i+2]*28)/255);
		}
		NLMISC::contReset(_Data[m]); 
		_Data[m].resize(0);
		_Data[m] = dataTmp;
	}
	PixelFormat = Luminance;
	return true;
}



/*-------------------------------------------------------------------*\
							alphaToLuminance
\*-------------------------------------------------------------------*/
bool CBitmap::alphaToLuminance()
{
	if(_Width*_Height == 0)  return false;
		
	PixelFormat = Luminance;
	return true;
}



/*-------------------------------------------------------------------*\
							alphaLuminanceToLuminance
\*-------------------------------------------------------------------*/
bool CBitmap::alphaLuminanceToLuminance()
{
	uint32 i;

	if(_Width*_Height == 0)  return false;
		
	for(uint8 m= 0; m<_MipMapCount; m++)
	{
		std::vector<uint8> dataTmp;
		dataTmp.reserve(_Data[m].size()/2);

		for(i=0; i<_Data[m].size(); i+=2)
		{
			dataTmp.push_back(0);
			dataTmp.push_back(0);
			dataTmp.push_back(0);
			dataTmp.push_back(_Data[m][i]);
		}
		NLMISC::contReset(_Data[m]); 
		_Data[m].resize(0);
		_Data[m] = dataTmp;
	}
	PixelFormat = Luminance;
	return true;
}


/*-------------------------------------------------------------------*\
							rgbaToAlpha
\*-------------------------------------------------------------------*/
bool CBitmap::rgbaToAlpha()
{
	uint32 i;

	if(_Width*_Height == 0)  return false;
		
	for(uint8 m= 0; m<_MipMapCount; m++)
	{
		std::vector<uint8> dataTmp;
		dataTmp.reserve(_Data[m].size()/4);

		for(i=0; i<_Data[m].size(); i+=4)
		{
			dataTmp.push_back(0);
			dataTmp.push_back(0);
			dataTmp.push_back(0);
			dataTmp.push_back(_Data[m][i+3]);
		}
		NLMISC::contReset(_Data[m]); 
		_Data[m].resize(0);
		_Data[m] = dataTmp;
	}
	PixelFormat = Alpha;
	return true;
}


/*-------------------------------------------------------------------*\
							luminanceToAlpha
\*-------------------------------------------------------------------*/
bool CBitmap::luminanceToAlpha()
{
	uint32 i;

	if(_Width*_Height == 0)  return false;
		
	for(uint8 m= 0; m<_MipMapCount; m++)
	{
		std::vector<uint8> dataTmp;
		dataTmp.reserve(_Data[m].size());

		for(i=0; i<_Data[m].size(); i++)
		{
			dataTmp.push_back(_Data[m][i]);
		}
		_Data[m] = dataTmp;
	}
	PixelFormat = Alpha;
	return true;
}


/*-------------------------------------------------------------------*\
							alphaLuminanceToAlpha
\*-------------------------------------------------------------------*/
bool CBitmap::alphaLuminanceToAlpha()
{
	uint32 i;

	if(_Width*_Height == 0)  return false;
		
	for(uint8 m= 0; m<_MipMapCount; m++)
	{
		std::vector<uint8> dataTmp;
		dataTmp.reserve(_Data[m].size()/2);

		for(i=0; i<_Data[m].size(); i+=2)
		{
			dataTmp.push_back(_Data[m][i+1]);
		}
		NLMISC::contReset(_Data[m]); 
		_Data[m].resize(0);
		_Data[m] = dataTmp;
	}
	PixelFormat = Alpha;
	return true;
}


/*-------------------------------------------------------------------*\
							convertToLuminance
\*-------------------------------------------------------------------*/
bool CBitmap::convertToLuminance()
{
	switch(PixelFormat)
	{
		case RGBA :
			return rgbaToLuminance();
			break;

		case Luminance :
			return true;
			break;

		case Alpha :
			return alphaToLuminance();
			break;

		case AlphaLuminance :
			return alphaLuminanceToLuminance();
			break;

		default:
			break;
	}
	return false;
}



/*-------------------------------------------------------------------*\
							convertToAlpha
\*-------------------------------------------------------------------*/
bool CBitmap::convertToAlpha()
{
	switch(PixelFormat)
	{
		case RGBA :
			return rgbaToAlpha();
			break;

		case Luminance :
			return luminanceToAlpha();
			break;

		case Alpha :
			return true;
			break;

		case AlphaLuminance :
			return alphaLuminanceToAlpha();
			break;

		default:
			break;
	}
	return false;
}



/*-------------------------------------------------------------------*\
							convertToAlphaLuminance
\*-------------------------------------------------------------------*/
bool CBitmap::convertToAlphaLuminance()
{
	switch(PixelFormat)
	{
		case RGBA :
			return rgbaToAlphaLuminance();
			break;

		case Luminance :
			return luminanceToAlphaLuminance();
			break;

		case Alpha :
			return alphaToAlphaLuminance();
			break;

		case AlphaLuminance :
			return true;
			break;

		default:
			break;
	}
	return false;
}


/*-------------------------------------------------------------------*\
							convertToRGBA
\*-------------------------------------------------------------------*/
bool CBitmap::convertToRGBA()
{
	switch(PixelFormat)
	{
		case DXTC1 :
			return decompressDXT1(false);
			break;

		case DXTC1Alpha :
			return decompressDXT1(true);
			break;

		case DXTC3 :
			return decompressDXT3();	
			break;

		case DXTC5 :
			return decompressDXT5();		
			break;

		case Luminance :
			return luminanceToRGBA();
			break;

		case Alpha :
			return alphaToRGBA();
			break;

		case AlphaLuminance :
			return alphaLuminanceToRGBA();
			break;
		case RGBA:
			return true;
		break;
		default:
			break;
	}
	return false;
}


/*-------------------------------------------------------------------*\
							convertToType
\*-------------------------------------------------------------------*/
bool CBitmap::convertToType(CBitmap::TType type)
{
	if(PixelFormat==type) return true;

	switch(type)
	{
		case RGBA :
			return convertToRGBA();
			break;

		case DXTC5 :
			return convertToDXTC5();		
			break;

		case Luminance :
			return convertToLuminance();
			break;

		case Alpha :
			return convertToAlpha();
			break;

		case AlphaLuminance :
			return convertToAlphaLuminance();
			break;

		default:
			break;
	}
	
	return false;
}




/*-------------------------------------------------------------------*\
							decompressDXT1
\*-------------------------------------------------------------------*/
bool CBitmap::decompressDXT1(bool alpha)
{
	uint32 i,j,k;
	NLMISC::CRGBA	c[4];
	std::vector<uint8> dataTmp[MAX_MIPMAP];
	
	uint32 width= _Width;
	uint32 height= _Height;

	for(uint8 m= 0; m<_MipMapCount; m++)
	{
		uint32 wtmp, htmp;
		if(width<4)
			wtmp = 4;
		else
			wtmp = width;
		if(height < 4)
			htmp = 4;
		else
			htmp = height;
		uint32 mipMapSz = wtmp*htmp*4;
		dataTmp[m].resize(mipMapSz); 
		if(dataTmp[m].capacity()<mipMapSz)
		{
			throw EAllocationFailure();
		}
		uint32 wBlockCount= wtmp/4;
		


		for(i=0; i < _Data[m].size(); i+=8)
		{
			uint16 color0;
			uint16 color1;
			uint32 bits;
			memcpy(&color0,&_Data[m][i],2);
			memcpy(&color1,&_Data[m][i+2],2);
			memcpy(&bits,&_Data[m][i+4],4);

			uncompress(color0,c[0]);
			uncompress(color1,c[1]);	
			
			c[0].A= 0;
			c[1].A= 0;
			c[2].A= 0;
			c[3].A= 0;
			
			if(color0>color1)
			{
				c[2].blendFromui(c[0],c[1],85);
				if(alpha) c[2].A= 255;

				c[3].blendFromui(c[0],c[1],171);	
				if(alpha) c[3].A= 255;
			}
			else
			{
				c[2].blendFromui(c[0],c[1],128);
				if(alpha) c[2].A= 255;

				c[3].set(0,0,0,0);
			}

			// computing the 16 RGBA of the block
			
			uint32 blockNum= i/8; //(64 bits)
			// <previous blocks in above lines> * 4 (rows) * _Width (columns) + 4pix*4rgba*<same line previous blocks>
			uint32 pixelsCount= 4*(blockNum/wBlockCount)*wtmp*4 + 4*4*(blockNum%wBlockCount);
			for(j=0; j<4; j++)
			{
				for(k=0; k<4; k++)
				{
					dataTmp[m][pixelsCount + j*wtmp*4 + 4*k]= c[bits&3].R;
					dataTmp[m][pixelsCount + j*wtmp*4 + 4*k+1]= c[bits&3].G;
					dataTmp[m][pixelsCount + j*wtmp*4 + 4*k+2]= c[bits&3].B;
					dataTmp[m][pixelsCount + j*wtmp*4 + 4*k+3]= c[bits&3].A;
					bits>>=2;
				}
			}
		}

		// Copy result into the mipmap level.
		if(wtmp==width && htmp==height)
		{
			// For mipmaps level >4 pixels.
			_Data[m]= dataTmp[m];
		}
		else
		{
			// For last mipmaps, level <4 pixels.
			_Data[m].resize(width*height*4);
			CRGBA	*src= (CRGBA*)&dataTmp[m][0];
			CRGBA	*dst= (CRGBA*)&_Data[m][0];
			uint	x,y;
			for(y=0;y<height;y++)
			{
				for(x=0;x<width;x++)
					dst[y*width+x]= src[y*wtmp+x];
			}
		}

		// Next mipmap size.
		width = (width+1)/2;
		height = (height+1)/2;
	}
	PixelFormat = RGBA;
	return true;
}




/*-------------------------------------------------------------------*\
							decompressDXT3
\*-------------------------------------------------------------------*/
bool CBitmap::decompressDXT3()
{
	uint32 i,j,k;
	NLMISC::CRGBA	c[4];
	std::vector<uint8> dataTmp[MAX_MIPMAP];
	
	uint32 width= _Width;
	uint32 height= _Height;

	for(uint8 m= 0; m<_MipMapCount; m++)
	{
		uint32 wtmp, htmp;
		if(width<4)
			wtmp = 4;
		else
			wtmp = width;
		if(height < 4)
			htmp = 4;
		else
			htmp = height;
		uint32 mipMapSz = wtmp*htmp*4;
		dataTmp[m].resize(mipMapSz); 
		if(dataTmp[m].capacity()<mipMapSz)
		{
			throw EAllocationFailure();
		}
		uint32 wBlockCount= wtmp/4;
		

		for(i=0; i < _Data[m].size(); i+=16)
		{
			uint8 alpha[16];
			uint64 alphatmp;
			memcpy(&alphatmp,&_Data[m][i],8);

			for(j=0; j<16; j++)
			{
				uint8	a= (uint8)(alphatmp&15);
				// expand to 0-255.
				alpha[j]= a+(a<<4);
				alphatmp>>=4;
			}


			uint16 color0;
			uint16 color1;
			uint32 bits;
			memcpy(&color0,&_Data[m][i+8],2);
			memcpy(&color1,&_Data[m][i+10],2);
			memcpy(&bits,&_Data[m][i+12],4);

			uncompress(color0,c[0]);
			uncompress(color1,c[1]);	
						
			// ignore color0>color1 for DXT3 and DXT5.
			c[2].blendFromui(c[0],c[1],85);
			c[3].blendFromui(c[0],c[1],171);	

			// computing the 16 RGBA of the block
			
			uint32 blockNum= i/16; //(128 bits)
			// <previous blocks in above lines> * 4 (rows) * wtmp (columns) + 4pix*4rgba*<same line previous blocks>
			uint32 pixelsCount= 4*(blockNum/wBlockCount)*wtmp*4 + 4*4*(blockNum%wBlockCount);
			for(j=0; j<4; j++)
			{
				for(k=0; k<4; k++)
				{
					dataTmp[m][pixelsCount + j*wtmp*4 + 4*k]= c[bits&3].R;
					dataTmp[m][pixelsCount + j*wtmp*4 + 4*k+1]= c[bits&3].G;
					dataTmp[m][pixelsCount + j*wtmp*4 + 4*k+2]= c[bits&3].B;
					dataTmp[m][pixelsCount + j*wtmp*4 + 4*k+3]= alpha[4*j+k];
					bits>>=2;
				}
			}
		}

		// Copy result into the mipmap level.
		if(wtmp==width && htmp==height)
		{
			// For mipmaps level >4 pixels.
			_Data[m]= dataTmp[m];
		}
		else
		{
			// For last mipmaps, level <4 pixels.
			_Data[m].resize(width*height*4);
			CRGBA	*src= (CRGBA*)&dataTmp[m][0];
			CRGBA	*dst= (CRGBA*)&_Data[m][0];
			uint	x,y;
			for(y=0;y<height;y++)
			{
				for(x=0;x<width;x++)
					dst[y*width+x]= src[y*wtmp+x];
			}
		}

		// Next mipmap size.
		width = (width+1)/2;
		height = (height+1)/2;
	}
	PixelFormat = RGBA;
	return true;
}




/*-------------------------------------------------------------------*\
							decompressDXT5
\*-------------------------------------------------------------------*/
bool CBitmap::decompressDXT5()
{
	uint32 i,j,k;
	NLMISC::CRGBA	c[4];
	std::vector<uint8> dataTmp[MAX_MIPMAP];
	
	uint32 width= _Width;
	uint32 height= _Height;

	for(uint8 m= 0; m<_MipMapCount; m++)
	{
		uint32 wtmp, htmp;
		if(width<4)
			wtmp = 4;
		else
			wtmp = width;
		if(height < 4)
			htmp = 4;
		else
			htmp = height;
		uint32 mipMapSz = wtmp*htmp*4;
		dataTmp[m].resize(mipMapSz); 
		if(dataTmp[m].capacity()<mipMapSz)
		{
			throw EAllocationFailure();
		}
		uint32 wBlockCount= wtmp/4;
		


		for(i=0; i < _Data[m].size(); i+=16)
		{
			uint64 bitsAlpha;
			memcpy(&bitsAlpha,&_Data[m][i],8);
			bitsAlpha>>= 16;

			uint32 alpha[8];
			alpha[0]= _Data[m][i+0];
			alpha[1]= _Data[m][i+1];
			
			if(alpha[0]>alpha[1])
			{
				alpha[2]= blend(alpha[0], alpha[1], 219);
				alpha[3]= blend(alpha[0], alpha[1], 183);
				alpha[4]= blend(alpha[0], alpha[1], 146);
				alpha[5]= blend(alpha[0], alpha[1], 110);
				alpha[6]= blend(alpha[0], alpha[1], 73);
				alpha[7]= blend(alpha[0], alpha[1], 37);
			}
			else
			{
				alpha[2]= blend(alpha[0], alpha[1], 204);
				alpha[3]= blend(alpha[0], alpha[1], 154);
				alpha[4]= blend(alpha[0], alpha[1], 102);
				alpha[5]= blend(alpha[0], alpha[1], 51);
				alpha[6]= 0;
				alpha[7]= 255;
			}

			uint8 codeAlpha[16];
			for(j=0; j<16; j++)
			{
				codeAlpha[j] = (uint8)bitsAlpha & 7;
				bitsAlpha>>=3;
			}


			uint16 color0;
			uint16 color1;
			uint32 bits;
			memcpy(&color0,&_Data[m][i+8],2);
			memcpy(&color1,&_Data[m][i+10],2);
			memcpy(&bits,&_Data[m][i+12],4);

			uncompress(color0,c[0]);
			uncompress(color1,c[1]);	
			
			// ignore color0>color1 for DXT3 and DXT5.
			c[2].blendFromui(c[0],c[1],85);
			c[3].blendFromui(c[0],c[1],171);	

			// computing the 16 RGBA of the block
			
			uint32 blockNum= i/16; //(128 bits)

			// <previous blocks in above lines> * 4 (rows) * wtmp (columns) + 4pix*<same line previous blocks>
			uint32 pixelsCount= (blockNum/wBlockCount)*wtmp*4 + 4*(blockNum%wBlockCount);
			// *sizeof(RGBA)
			pixelsCount*=4;
			for(j=0; j<4; j++)
			{
				for(k=0; k<4; k++)
				{
					dataTmp[m][pixelsCount + (j*wtmp+k)*4 +0]= c[bits&3].R;
					dataTmp[m][pixelsCount + (j*wtmp+k)*4 +1]= c[bits&3].G;
					dataTmp[m][pixelsCount + (j*wtmp+k)*4 +2]= c[bits&3].B;
					dataTmp[m][pixelsCount + (j*wtmp+k)*4 +3]= (uint8) alpha[codeAlpha[4*j+k]];
					bits>>=2;
				}
			}

		}

		// Copy result into the mipmap level.
		if(wtmp==width && htmp==height)
		{
			// For mipmaps level >4 pixels.
			_Data[m]= dataTmp[m];
		}
		else
		{
			// For last mipmaps, level <4 pixels.
			_Data[m].resize(width*height*4);
			CRGBA	*src= (CRGBA*)&dataTmp[m][0];
			CRGBA	*dst= (CRGBA*)&_Data[m][0];
			uint	x,y;
			for(y=0;y<height;y++)
			{
				for(x=0;x<width;x++)
					dst[y*width+x]= src[y*wtmp+x];
			}
		}

		// Next mipmap size.
		width = (width+1)/2;
		height = (height+1)/2;
	}
	PixelFormat = RGBA;
	return true;

}




/*-------------------------------------------------------------------*\
							blend
\*-------------------------------------------------------------------*/
uint32 CBitmap::blend(uint32 &n0, uint32 &n1, uint32 coef0) 
{
	int	a0 = coef0;
	int	a1 = 256-a0;
	return ((n0*a0 + n1*a1) >>8);
}



/*-------------------------------------------------------------------*\
							uncompress
\*-------------------------------------------------------------------*/
inline void CBitmap::uncompress(uint16 color, NLMISC::CRGBA &r)
{
	r.A= 0;
	r.R= ((color>>11)&31) << 3; r.R+= r.R>>5;
	r.G= ((color>>5)&63) << 2;  r.G+= r.G>>6;
	r.B= ((color)&31) << 3;     r.B+= r.B>>5;
}



/*-------------------------------------------------------------------*\
							getWidth
\*-------------------------------------------------------------------*/
uint32 CBitmap::getWidth(uint32 mipMap) const
{
	if(mipMap==0) return _Width;
	
	uint32 w = _Width;
	uint32 h = _Height;
	uint32 m = 0;
	
	do
	{
		m++;
		w = (w+1)/2;
		h = (h+1)/2;
		if(m==mipMap) return w;
	}
	while(w!=1 || h!=1);

	return 0;
}



/*-------------------------------------------------------------------*\
							getHeight
\*-------------------------------------------------------------------*/
uint32 CBitmap::getHeight(uint32 mipMap) const
{
	if(mipMap==0) return _Height;
	
	uint32 w = _Width;
	uint32 h = _Height;
	uint32 m = 0;

	do
	{
		m++;
		w = (w+1)/2;
		h = (h+1)/2;
		if(m==mipMap) return h;
	}
	while(w!=1 || h!=1);

	return 0;
}


/*-------------------------------------------------------------------*\
							getHeight
\*-------------------------------------------------------------------*/
uint32 CBitmap::getSize(uint32 numMipMap) const
{
	return getHeight(numMipMap)*getWidth(numMipMap);
}



/*-------------------------------------------------------------------*\
							buildMipMaps
\*-------------------------------------------------------------------*/
void CBitmap::buildMipMaps()
{
	uint32 i,j;

	if(PixelFormat!=RGBA) return;
	if(_MipMapCount!=1) return;
	if(!NLMISC::isPowerOf2(_Width)) return;
	if(!NLMISC::isPowerOf2(_Height)) return;
	
	uint32 w = _Width;
	uint32 h = _Height;

	while(w>1 || h>1)
	{
		uint32 precw = w;
		uint32 prech = h;
		w = (w+1)/2;
		h = (h+1)/2;
		uint32	mulw= precw/w;
		uint32	mulh= prech/h;

		_Data[_MipMapCount].resize(w*h*4);
		
	
		NLMISC::CRGBA *pRgba = (NLMISC::CRGBA*)&_Data[_MipMapCount][0];
		NLMISC::CRGBA *pRgbaPrev = (NLMISC::CRGBA*)&_Data[_MipMapCount-1][0];
		for(i=0; i<h; i++)
		{
			sint	i0= mulh*i;
			sint	i1= mulh*i+1;
			if(mulh==1)
				i1=i0;
			i0*=precw;
			i1*=precw;
			for(j=0; j<w; j++)
			{
				sint	j0= mulw*j;
				sint	j1= mulw*j+1;
				if(mulh==1)
					j1=j0;
				CRGBA	&c0= pRgbaPrev[i0+j0];
				CRGBA	&c1= pRgbaPrev[i0+j1];
				CRGBA	&c2= pRgbaPrev[i1+j0];
				CRGBA	&c3= pRgbaPrev[i1+j1];
				pRgba[i*w + j].R = (c0.R +
									c1.R +
									c2.R +
									c3.R + 2 ) /4;
				pRgba[i*w + j].G = (c0.G +
									c1.G +
									c2.G +
									c3.G + 2 ) /4;
				pRgba[i*w + j].B = (c0.B +
									c1.B +
									c2.B +
									c3.B + 2 ) /4;
				pRgba[i*w + j].A = (c0.A +
									c1.A +
									c2.A +
									c3.A + 2 ) /4;
			}
		}

		_MipMapCount++;
	}
}


/*-------------------------------------------------------------------*\
							releaseMipMaps
\*-------------------------------------------------------------------*/
void CBitmap::releaseMipMaps()
{
	if(_MipMapCount<=1) return;

	_MipMapCount=1;
	for(sint i=1;i<MAX_MIPMAP;i++)
	{
		NLMISC::contReset(_Data[i]); 
	}
}


/*-------------------------------------------------------------------*\
							resample
\*-------------------------------------------------------------------*/
void CBitmap::resample(sint32 nNewWidth, sint32 nNewHeight)
{
	nlassert(PixelFormat == RGBA);
	bool needRebuild = false;

	// Deleting mipmaps
	if(_MipMapCount>1)
		needRebuild = true;
	releaseMipMaps();

	if(nNewWidth==0 || nNewHeight==0)
	{
		_Width = _Height = 0;
		return;
	}
	
	std::vector<uint8> pDestui;
	pDestui.resize(nNewWidth*nNewHeight*4);
	NLMISC::CRGBA *pDestRgba = (NLMISC::CRGBA*)&pDestui[0];

	resamplePicture32 ((NLMISC::CRGBA*)&_Data[0][0], pDestRgba, _Width, _Height, nNewWidth, nNewHeight);
	NLMISC::contReset(_Data[0]); // free memory
	_Data[0] =  pDestui;
	_Width= nNewWidth;
	_Height= nNewHeight;

	// Rebuilding mipmaps
	if(needRebuild)
	{
		buildMipMaps();
	}
}


/*-------------------------------------------------------------------*\
							resize
\*-------------------------------------------------------------------*/
void CBitmap::resize (sint32 nNewWidth, sint32 nNewHeight, TType newType)
{
	// Deleting mipmaps
	releaseMipMaps();

	// Change type of bitmap ?
	if (newType!=DonTKnow)
		PixelFormat=newType;

	_Width = nNewWidth;
	_Height = nNewHeight;

	NLMISC::contReset(_Data[0]); // free memory

	// resize the buffer
	_Data[0].resize (((uint32)(nNewWidth*nNewHeight)*bitPerPixels[PixelFormat])/8);
}


/*-------------------------------------------------------------------*\
							reset
\*-------------------------------------------------------------------*/
void CBitmap::reset(TType type)
{
	for(uint i=0; i<_MipMapCount; i++)
	{
		NLMISC::contReset(_Data[i]);
		_Data[i].resize(0);
	}
	_Width = _Height = 0;
	_MipMapCount= 1;
	
	// Change pixel format
	PixelFormat=type;
}



/*-------------------------------------------------------------------*\
							resamplePicture32
\*-------------------------------------------------------------------*/
void CBitmap::resamplePicture32 (const NLMISC::CRGBA *pSrc, NLMISC::CRGBA *pDest, 
								 sint32 nSrcWidth, sint32 nSrcHeight, 
								 sint32 nDestWidth, sint32 nDestHeight)
{
	if ((nSrcWidth<=0)||(nSrcHeight<=0)||(nDestHeight<=0)||(nDestHeight<=0))
		return;
	bool bXMag=(nDestWidth>=nSrcWidth);
	bool bYMag=(nDestHeight>=nSrcHeight);
	bool bXEq=(nDestWidth==nSrcWidth);
	bool bYEq=(nDestHeight==nSrcHeight);
	std::vector<NLMISC::CRGBAF> pIterm (nDestWidth*nSrcHeight);
	
	if (bXMag)
	{
		float fXdelta=(float)(nSrcWidth)/(float)(nDestWidth);
		NLMISC::CRGBAF *pItermPtr=&*pIterm.begin();
		sint32 nY;
		for (nY=0; nY<nSrcHeight; nY++)
		{
			const NLMISC::CRGBA *pSrcLine=pSrc;
			float fX=0.f;
			sint32 nX;
			for (nX=0; nX<nDestWidth; nX++)
			{
				float fVirgule=fX-(float)floor(fX);
				nlassert (fVirgule>=0.f);
				NLMISC::CRGBAF vColor;
				if (fVirgule>=0.5f)
				{
					if (fX<(float)(nSrcWidth-1))
					{
						NLMISC::CRGBAF vColor1 (pSrcLine[(sint32)floor(fX)]);
						NLMISC::CRGBAF vColor2 (pSrcLine[(sint32)floor(fX)+1]);
						vColor=vColor1*(1.5f-fVirgule)+vColor2*(fVirgule-0.5f);
					}
					else
						vColor=NLMISC::CRGBAF (pSrcLine[(sint32)floor(fX)]);
				}
				else
				{
					if (fX>=1.f)
					{
						NLMISC::CRGBAF vColor1 (pSrcLine[(sint32)floor(fX)]);
						NLMISC::CRGBAF vColor2 (pSrcLine[(sint32)floor(fX)-1]);
						vColor=vColor1*(0.5f+fVirgule)+vColor2*(0.5f-fVirgule);
					}
					else
						vColor=NLMISC::CRGBAF (pSrcLine[(sint32)floor(fX)]);
				}
				*(pItermPtr++)=vColor;
				fX+=fXdelta;
			}
			pSrc+=nSrcWidth;
		}
	}
	else if (bXEq)
	{
		NLMISC::CRGBAF *pItermPtr=&*pIterm.begin();
		for (sint32 nY=0; nY<nSrcHeight; nY++)
		{
			const NLMISC::CRGBA *pSrcLine=pSrc;
			sint32 nX;
			for (nX=0; nX<nDestWidth; nX++)
				*(pItermPtr++)=NLMISC::CRGBAF (pSrcLine[nX]);
			pSrc+=nSrcWidth;
		}
	}
	else
	{
		double fXdelta=(double)(nSrcWidth)/(double)(nDestWidth);
		nlassert (fXdelta>1.f);
		NLMISC::CRGBAF *pItermPtr=&*pIterm.begin();
		sint32 nY;
		for (nY=0; nY<nSrcHeight; nY++)
		{
			const NLMISC::CRGBA *pSrcLine=pSrc;
			double fX=0.f;
			sint32 nX;
			for (nX=0; nX<nDestWidth; nX++)
			{
				NLMISC::CRGBAF vColor (0.f, 0.f, 0.f, 0.f);
				double fFinal=fX+fXdelta;
				while (fX<fFinal)
				{
					double fNext=(double)floor (fX)+1.f;
					if (fNext>fFinal)
						fNext=fFinal;
					vColor+=((float)(fNext-fX))*NLMISC::CRGBAF (pSrcLine[(sint32)floor(fX)]);
					fX=fNext;
				}
				nlassert (fX==fFinal);
				vColor/=(float)fXdelta;
				*(pItermPtr++)=vColor;
			}
			pSrc+=nSrcWidth;
		}
	}
				
	if (bYMag)
	{
		double fYdelta=(double)(nSrcHeight)/(double)(nDestHeight);
		sint32 nX;
		for (nX=0; nX<nDestWidth; nX++)
		{
			double fY=0.f;
			sint32 nY;
			for (nY=0; nY<nDestHeight; nY++)
			{
				double fVirgule=fY-(double)floor(fY);
				nlassert (fVirgule>=0.f);
				NLMISC::CRGBAF vColor;
				if (fVirgule>=0.5f)
				{
					if (fY<(double)(nSrcHeight-1))
					{
						NLMISC::CRGBAF vColor1=pIterm[((sint32)floor(fY))*nDestWidth+nX];
						NLMISC::CRGBAF vColor2=pIterm[(((sint32)floor(fY))+1)*nDestWidth+nX];
						vColor=vColor1*(1.5f-(float)fVirgule)+vColor2*((float)fVirgule-0.5f);
					}
					else
						vColor=pIterm[((sint32)floor(fY))*nDestWidth+nX];
				}
				else
				{
					if (fY>=1.f)
					{
						NLMISC::CRGBAF vColor1=pIterm[((sint32)floor(fY))*nDestWidth+nX];
						NLMISC::CRGBAF vColor2=pIterm[(((sint32)floor(fY))-1)*nDestWidth+nX];
						vColor=vColor1*(0.5f+(float)fVirgule)+vColor2*(0.5f-(float)fVirgule);
					}
					else
						vColor=pIterm[((sint32)floor(fY))*nDestWidth+nX];
				}
				pDest[nX+nY*nDestWidth]=vColor;
				fY+=fYdelta;
			}
		}
	}
	else if (bYEq)
	{
		for (sint32 nX=0; nX<nDestWidth; nX++)
		{
			sint32 nY;
			for (nY=0; nY<nDestHeight; nY++)
			{
				pDest[nX+nY*nDestWidth]=pIterm[nY*nDestWidth+nX];
			}
		}
	}
	else
	{
		double fYdelta=(double)(nSrcHeight)/(double)(nDestHeight);
		nlassert (fYdelta>1.f);
		sint32 nX;
		for (nX=0; nX<nDestWidth; nX++)
		{
			double fY=0.f;
			sint32 nY;
			for (nY=0; nY<nDestHeight; nY++)
			{
				NLMISC::CRGBAF vColor (0.f, 0.f, 0.f, 0.f);
				double fFinal=fY+fYdelta;
				while ((fY<fFinal)&&((sint32)fY!=nSrcHeight))
				{
					double fNext=(double)floor (fY)+1.f;
					if (fNext>fFinal)
						fNext=fFinal;
					vColor+=((float)(fNext-fY))*pIterm[((sint32)floor(fY))*nDestWidth+nX];
					fY=fNext;
				}
				vColor/=(float)fYdelta;
				pDest[nX+nY*nDestWidth]=vColor;
			}
		}
	}
}



/*-------------------------------------------------------------------*\
							readTGA
\*-------------------------------------------------------------------*/
uint8 CBitmap::readTGA( NLMISC::IStream &f)
{
	if(!f.isReading()) return 0;

	uint32			size;
	uint32			x,y;
	sint32			slsize;
	uint8			*scanline;
	uint8			r,g,b;
	sint32			i,j,k;

	// TGA file header fields
	uint8	lengthID;
	uint8	cMapType;
	uint8	imageType;
	uint16	origin;
	uint16	length;
	uint8	depth;
	uint16	xOrg;
	uint16	yOrg;
	uint16	width;
	uint16	height;
	uint8	imageDepth;
	uint8	desc;

	// Image/Color map data
	uint8 *imageID;
	
	
	
	// Determining whether file is in Original or New TGA format
	
	bool newTgaFormat;
	uint32 extAreaOffset;
	uint32 devDirectoryOffset;
	char signature[16];

	f.seek (0, f.end);
	newTgaFormat = false;
	if (f.getPos() >= 26)
	{
		f.seek (-26, f.end);
		f.serial(extAreaOffset);
		f.serial(devDirectoryOffset);
		for(i=0; i<16; i++)
		{
			f.serial(signature[i]);
		}
		if(strncmp(signature,"TRUEVISION-XFILE",16)==0)
			newTgaFormat = true;
	}



	// Reading TGA file header
	f.seek (0, f.begin);
		
	f.serial(lengthID);
	f.serial(cMapType);
	f.serial(imageType);
	f.serial(origin);
	f.serial(length);
	f.serial(depth);
	f.serial(xOrg);
	f.serial(yOrg);
	f.serial(width);
	f.serial(height);
	f.serial(imageDepth);
	f.serial(desc);

	if(cMapType!=0)
	{
		nlinfo("readTga : color-map not supported");
	}

	if(lengthID>0)
	{
		imageID = new uint8[lengthID];
		for(i=0; i<lengthID; i++)
			f.serial(imageID[i]);
	}



	// Reading TGA image data
	
	_Width = width;
	_Height = height;
	size = _Width * _Height * (imageDepth/8);
	
	switch(imageType)
	{
		// Uncompressed RGB or RGBA
		case 2:
		{
			_Data[0].resize(_Width*_Height*4);
			uint8 upSideDown = ((desc & (1 << 5))==0);
			slsize = _Width * imageDepth / 8;

			scanline = new uint8[slsize];
			if(!scanline)
			{
				throw EAllocationFailure();
			}

			for(y=0; y<_Height;y++)
			{
				// Serial buffer: more efficient way to load.
				f.serialBuffer (scanline, slsize);

				if(imageDepth==24 || imageDepth==32)
				{
					sint32 mult = 3;
					if(imageDepth==16)
					{
						mult = 2;
					}
					if(imageDepth==32)  
					{
						mult = 4;
					}
					if(imageDepth!=16)
					{
						for(x=0; x<_Width; x++)
						{
							// RGB(A)
							r = scanline[x*mult+0];
							g = scanline[x*mult+1];
							b = scanline[x*mult+2];
							// Switching to BGR(A)
							scanline[x*mult+0] = b;
							scanline[x*mult+1] = g;
							scanline[x*mult+2] = r;
						}
					}
				}
				
				k=0;
				for(i=0; i<width; i++) 
				{
					if(upSideDown)
					{
						if(imageDepth==16)
						{
							uint16 toto = (uint16)scanline[k++];
							toto |= scanline[k++]<<8;
							uint r = toto>>10;
							uint g = (toto>>5)&0x1f;
							uint b = toto&0x1f;
							_Data[0][(height-y-1)*width*4 + 4*i] = (r<<3) | (r>>2);
							_Data[0][(height-y-1)*width*4 + 4*i + 1] = (g<<3) | (g>>2);
							_Data[0][(height-y-1)*width*4 + 4*i + 2] = (b<<3) | (b>>2);
							_Data[0][(height-y-1)*width*4 + 4*i + 3] = 255;
						}
						else
						{
							_Data[0][(height-y-1)*width*4 + 4*i] = scanline[k++];
							_Data[0][(height-y-1)*width*4 + 4*i + 1] = scanline[k++];
							_Data[0][(height-y-1)*width*4 + 4*i + 2] = scanline[k++];
							if(imageDepth==32)
								_Data[0][(height-y-1)*width*4 + 4*i + 3] = scanline[k++];
							else
								_Data[0][(height-y-1)*width*4 + 4*i + 3] = 255;
						}
					}
					else
					{
						if(imageDepth==16)
						{
							uint16 toto = (uint16)scanline[k++];
							toto |= scanline[k++]<<8;
							int r = toto>>10;
							int g = toto&(0x3e0)>>5;
							int b = toto&0x1f;
							_Data[0][y*width*4 + 4*i] = (r<<3) | (r>>2);
							_Data[0][y*width*4 + 4*i + 1] = (g<<3) | (g>>2);
							_Data[0][y*width*4 + 4*i + 2] = (b<<3) | (b>>2);
							_Data[0][y*width*4 + 4*i + 3] = 255;
						}
						else
						{
							_Data[0][y*width*4 + 4*i] = scanline[k++];
							_Data[0][y*width*4 + 4*i + 1] = scanline[k++];
							_Data[0][y*width*4 + 4*i + 2] = scanline[k++];
							if(imageDepth==32)
								_Data[0][y*width*4 + 4*i + 3] = scanline[k++];
							else
								_Data[0][y*width*4 + 4*i + 3] = 255;
						}
					}	
				}
			}

			PixelFormat = RGBA;
			delete scanline;
		};
		break;
		
		// Uncompressed Grayscale bitmap
		case 3:
		{
			_Data[0].resize(_Width*_Height);
			uint8 upSideDown = ((desc & (1 << 5))==0);
			slsize = _Width;

			scanline = new uint8[slsize];
			if(!scanline)
			{
				throw EAllocationFailure();
			}

			for(y=0; y<_Height;y++)
			{
				// Serial buffer: more efficient way to load.
				f.serialBuffer (scanline, slsize);

				k=0;
				for(i=0; i<width; i++) 
				{
					if(upSideDown)
						_Data[0][(height-y-1)*width + i] = scanline[k++];
					else
						_Data[0][y*width + i] = scanline[k++];
				}
			}

			PixelFormat = _LoadGrayscaleAsAlpha?Alpha:Luminance;
			delete scanline;
		};
		break;

		// Compressed RGB or RGBA
		case 10:
		{
			uint8 packet;
			uint8 pixel[4];
			uint32 imageSize = width*height;
			uint32 readSize = 0;
			_Data[0].reserve(_Width*_Height*4);

			while(readSize < imageSize)
			{
				f.serial(packet);
				if((packet & 0x80) > 0) // packet RLE 
				{ 
					for(i=0; i<imageDepth/8; i++)
					{
						f.serial(pixel[i]);
					}
					for (i=0; i < (packet & 0x7F) + 1; i++)
					{
						for(j=0; j<imageDepth/8; j++)
						{
							_Data[0].push_back(pixel[j]);
						}
						if(imageDepth==24)
						{
							_Data[0].push_back(0);
						}
					}
				}
				else	// packet Raw 
				{ 
					for(i=0; i<((packet & 0x7F) + 1); i++)
					{
						for(j=0; j<imageDepth/8; j++)
						{
							f.serial(pixel[j]);
						}
						if(imageDepth==32)
						{
							_Data[0].push_back(pixel[2]);
							_Data[0].push_back(pixel[1]);
							_Data[0].push_back(pixel[0]);
							_Data[0].push_back(pixel[3]);
						}
						if(imageDepth==24)
						{
							_Data[0].push_back(pixel[2]);
							_Data[0].push_back(pixel[1]);
							_Data[0].push_back(pixel[0]);
							_Data[0].push_back(0);
						}
					}
  				}
				readSize += (packet & 0x7F) + 1;
			}
			PixelFormat = RGBA;
		};
		break;

		// Compressed Grayscale bitmap (not tested)
		case 11:
		{
			uint8 packet;
			uint8 pixel[4];
			uint32 imageSize = width*height;
			uint32 readSize = 0;
			_Data[0].reserve(_Width*_Height);

			while(readSize < imageSize)
			{
				f.serial(packet);
				if((packet & 0x80) > 0) // packet RLE 
				{ 
					f.serial(pixel[0]);
					for (i=0; i < (packet & 0x7F) + 1; i++)
					{
						_Data[0].push_back(pixel[0]);
					}
				}
				else	// packet Raw 
				{ 
					for(i=0; i<((packet & 0x7F) + 1); i++)
					{
						f.serial(pixel[0]);
						_Data[0].push_back(pixel[0]);
					}
  				}
				readSize += (packet & 0x7F) + 1;
			}
			PixelFormat = _LoadGrayscaleAsAlpha?Alpha:Luminance;
		};
		break;

		default:
			return 0;
	}

	_MipMapCount = 1;
	return(imageDepth);

}



/*-------------------------------------------------------------------*\
							writeTGA
\*-------------------------------------------------------------------*/
bool CBitmap::writeTGA( NLMISC::IStream &f, uint32 d, bool upsideDown)
{
	if(f.isReading()) return false;
	if(d!=24 && d!=32 && d!=16 && d!=8) return false;
	if ((PixelFormat != RGBA)&&(PixelFormat != Alpha)) return false;
	if ((PixelFormat == Alpha) && (d != 8)) return false;

	sint32	i,j,x,y;
	uint8	* scanline;
	uint8	r,g,b,a;

	uint8	lengthID = 0;
	uint8	cMapType = 0;
	uint8	imageType = 2;
	uint16	origin = 0;
	uint16	length = 0;
	uint8	depth = 0;
	uint16	xOrg = 0;
	uint16	yOrg = 0;
	uint16	width = (uint16)_Width;
	uint16	height = (uint16)_Height;
	uint8	imageDepth = (uint8)d;
	uint8	desc = 0;
	if (upsideDown)
		desc |= 1<<5;

	if (PixelFormat == Alpha)
		imageType = 3; // Uncompressed grayscale

	f.serial(lengthID);
	f.serial(cMapType);
	f.serial(imageType);
	f.serial(origin);
	f.serial(length);
	f.serial(depth);
	f.serial(xOrg);
	f.serial(yOrg);
	f.serial(width);
	f.serial(height);
	f.serial(imageDepth);
	f.serial(desc);

	if (PixelFormat == Alpha)
		scanline = new uint8[width];
	else
		scanline = new uint8[width*4];
	if(!scanline)
	{
		throw EAllocationFailure();
	}

	for(y=0; y<(sint32)height; y++)
	{
		
		uint32 k=0;
		if (PixelFormat == Alpha)
		for(i=0; i<width; ++i) // Alpha
		{
			scanline[k++] = _Data[0][(height-y-1)*width + i];
		}
		else
		for(i=0; i<width*4; i+=4) // 4:RGBA
		{
			if(d==16)
			{
				for(j=0; j<(sint32)4; j++)
				{
					scanline[k++] = _Data[0][(height-y-1)*width*4 + i + j];
				}
			}
			else
			{
				for(j=0; j<(sint32)d/8; j++)
				{
					scanline[k++] = _Data[0][(height-y-1)*width*4 + i + j];
				}
			}
		}
		
		if(d==16)
		{
			for(x=0; x<(sint32)width; x++)
			{
				r = scanline[x*4+0];
				g = scanline[x*4+1];
				b = scanline[x*4+2];
				int rr = r >>3;
				int gg = g >>3;
				int bb = b >>3;
				uint16 c16 = (rr<<10) | (gg<<5) | bb;
				scanline[x*2+0] = c16&0xff;
				scanline[x*2+1] = c16>>8;
			}
		}
		if(d==24)
		{
			for(x=0; x<(sint32)width; x++)
			{
				r = scanline[x*3+0];
				g = scanline[x*3+1];
				b = scanline[x*3+2];
				scanline[x*3+0] = b;
				scanline[x*3+1] = g;
				scanline[x*3+2] = r;
			}
		}
		if(d==32)
		{
			for(x=0; x<(sint32)width; x++)
			{
				r = scanline[x*4+0];
				g = scanline[x*4+1];
				b = scanline[x*4+2];
				a= scanline[x*4+3];
				scanline[x*4+0] = b;
				scanline[x*4+1] = g;
				scanline[x*4+2] = r;				
				scanline[x*4+3] = a;
			}
		}
		
		int finaleSize=width*d/8;
		for(i=0; i<finaleSize; i++)
		{
			f.serial(scanline[i]);
		}		
	}
	delete scanline;
	return true;
}

template<class T>
void rotateCCW (const T* src, T* dst, uint srcWidth, uint srcHeight)
{
	for (uint y=0; y<srcHeight; y++)
	for (uint x=0; x<srcWidth; x++)
	{
		uint dstX=y;
		uint dstY=srcWidth-x-1;
		dst[dstX+dstY*srcHeight]=src[x+y*srcWidth];
	}
}

/*template<class T>
void rotateCCW (const vector<T>& src, vector<T>& dst, uint srcWidth, uint srcHeight)
{
	for (uint y=0; y<srcHeight; y++)
	for (uint x=0; x<srcWidth; x++)
	{
		uint dstX=y;
		uint dstY=srcWidth-x;
		dst[dstX+dstY*srcHeight]=src[x+y*srcWidth];
	}
}
*/
void CBitmap::rotateCCW()
{
	// Copy the array
	std::vector<uint8> copy=_Data[0];

	switch (PixelFormat)
	{
	case RGBA:
		NLMISC::rotateCCW ((uint32*)&(_Data[0][0]), (uint32*)&(copy[0]), _Width, _Height);
		break;
	case Luminance:
	case Alpha:
		NLMISC::rotateCCW (&_Data[0][0], &copy[0], _Width, _Height);
		break;
	case AlphaLuminance:
		NLMISC::rotateCCW ((uint16*)&(_Data[0][0]), (uint16*)&(copy[0]), _Width, _Height);;
		break;
	default: break;
	}

	uint32 tmp=_Width;
	_Width=_Height;
	_Height=tmp;
	_Data[0]=copy;
}

bool CBitmap::blit(const CBitmap *src, sint32 x, sint32 y)
{
	
	nlassert(this->PixelFormat == src->PixelFormat);
	if (this->PixelFormat != src->PixelFormat)
	{
		return false;
	}


	// check for dxtc use

	const bool useDXTC   =  PixelFormat == DXTC1 || PixelFormat == DXTC1Alpha || PixelFormat == DXTC3 || PixelFormat ==	DXTC5;

	// number of bits for a 4x4 pix block
	const uint dxtcNumBits  =  PixelFormat == DXTC1 || PixelFormat == DXTC1Alpha ? 64 : 128;
	

	if (useDXTC)
	{
		// blit pos must be multiple of 4

		nlassert(! (x & 3 || y & 3) );
		if (x & 3 || y & 3) return false;

	}

	nlassert(PixelFormat != DonTKnow);

	// the width to copy
	sint width = src->_Width;
	// the height to copy
	sint height = src->_Height;

	uint destStartX, destStartY;
	uint srcStartX, srcStartY;


	// clip against left
	if (x < 0)
	{
		width += x;
		if (width <= 0) return true;
		destStartX = 0;
		srcStartX = -x;
	}
	else
	{
		destStartX = x;
		srcStartX = 0;
	}

	// clip against top
	if (y < 0)
	{
		height += y;
		if (height <= 0) return true;
		srcStartY = -y;
		destStartY = 0;
	}
	else
	{
		destStartY = y;
		srcStartY = 0;
	}

	// clip against right
	if ((destStartX + width - 1) >= _Width)
	{
		width = _Width - destStartX;
		if (width <= 0) return true;
	}

	// clip against bottom
	if ((destStartY + height - 1) >= _Height)
	{
		height = _Height - destStartY;
		if (width <= 0) return true;
	}


	// divide all distance by 4 when using DXTC
	if (useDXTC)
	{
		destStartX >>= 2;
		destStartY >>= 2;
		srcStartX >>= 2;
		srcStartY >>= 2;
		width >>= 2;
		height >>= 2;
	}
	

	// bytes per pixs is for either one pixel or 16 (a 4x4 block in DXTC)
	const uint bytePerPixs = ( useDXTC ? dxtcNumBits : bitPerPixels[PixelFormat] ) >> 3 /* divide by 8 to get the number of bytes */;


	const uint destRealWidth = useDXTC ? (_Width >> 2) : _Width;
	const uint srcRealWidth = useDXTC ? (src->_Width >> 2) : src->_Width;
	

	// size to go to the next line in the destination
	const uint destStride = destRealWidth * bytePerPixs;

	// size to go to the next line in the source
	const uint srcStride = srcRealWidth * bytePerPixs;
	
	// length in bytes of a line to copy
	const uint lineLength = width * bytePerPixs;


	uint8  *destPos = &(_Data[0][0]) + destStride * destStartY + bytePerPixs * destStartX;
	const uint8 *srcPos = &(src->_Data[0][0]) + srcStride * srcStartY + bytePerPixs * srcStartX;

	// copy each hline
	for (sint k = 0; k < height; ++k)
	{
		::memcpy(destPos, srcPos, lineLength);
		destPos += destStride;
		srcPos += srcStride;
	}

	
	return true;
}

// Private :
float CBitmap::getColorInterp (float x, float y, float colorInXY00, float colorInXY10, float colorInXY01, float colorInXY11) const
{
	float res =	colorInXY00*(1.0f-x)*(1.0f-y) +
				colorInXY10*(     x)*(1.0f-y) +
				colorInXY01*(1.0f-x)*(     y) +
				colorInXY11*(     x)*(     y);
	clamp (res, 0.0f, 255.0f);
	return res;
}

// Public:
CRGBAF CBitmap::getColor (float x, float y) const
{
	if (x < 0.0f) x = 0.0f;
	if (x > 1.0f) x = 1.0f;
	if (y < 0.0f) y = 0.0f;
	if (y > 1.0f) y = 1.0f;

	sint32 nWidth = getWidth(0);
	sint32 nHeight = getHeight(0);

	if (nWidth == 0 || nHeight == 0) return CRGBAF(0, 0, 0, 0);

	const std::vector<uint8> &rBitmap = getPixels(0);
	sint32 nX[4], nY[4];

	x *= nWidth-1;
	y *= nHeight-1;

	// Integer part of (x,y)
	//nX[0] = ((sint32)floor(x-0.5f));
	//nY[0] = ((sint32)floor(y-0.5f));
	nX[0] = ((sint32)floor(x));
	nY[0] = ((sint32)floor(y));

	nX[1] = (nX[0] < (nWidth-1) ? nX[0]+1 : nX[0]);
	nY[1] = nY[0];

	nX[2] = nX[0];
	nY[2] = (nY[0] < (nHeight-1) ? nY[0]+1 : nY[0]);

	nX[3] = nX[1];
	nY[3] = nY[2];

	uint32 i;

	for (i = 0; i < 4; ++i)
	{
		nlassert (nX[i] >= 0);
		nlassert (nY[i] >= 0 );
		nlassert (nX[i] < nWidth);
		nlassert (nY[i] < nHeight);
	}

	// Decimal part of (x,y)
	x = x - (float)nX[0]; 
	y = y - (float)nY[0];

	switch (this->PixelFormat)
	{
		case RGBA:
		case DXTC1:
		case DXTC1Alpha:
		case DXTC3:
		case DXTC5:
		{									
			CRGBAF finalVal;
			CRGBA val[4];

			if (this->PixelFormat == RGBA)
			{
				for (i = 0; i < 4; ++i)
				{
					val[i] = CRGBA (rBitmap[(nX[i]+nY[i]*nWidth)*4+0],
									rBitmap[(nX[i]+nY[i]*nWidth)*4+1],
									rBitmap[(nX[i]+nY[i]*nWidth)*4+2],
									rBitmap[(nX[i]+nY[i]*nWidth)*4+3]);
				}
			}
			else
			{
				// slower version : get from DXT
				for (i = 0; i < 4; ++i)
				{
					val[i] = getPixelColor(nX[i], nY[i]);
				}
			}

			finalVal.R = getColorInterp (x, y, val[0].R, val[1].R, val[2].R, val[3].R);
			finalVal.G = getColorInterp (x, y, val[0].G, val[1].G, val[2].G, val[3].G);
			finalVal.B = getColorInterp (x, y, val[0].B, val[1].B, val[2].B, val[3].B);
			finalVal.A = getColorInterp (x, y, val[0].A, val[1].A, val[2].A, val[3].A);

			return finalVal;			
		}
		break;
		case Alpha:
		case Luminance:
		{
			
			float finalVal;
			float val[4];

			for (i = 0; i < 4; ++i)
				val[i] = rBitmap[(nX[i]+nY[i]*nWidth)];

			finalVal = getColorInterp (x, y, val[0], val[1], val[2], val[3]);

			if (this->PixelFormat == Alpha)
				return CRGBAF (255.0f, 255.0f, 255.0f, finalVal);
			else // Luminance
				return CRGBAF (finalVal, finalVal, finalVal, 255.0f);
		}
		break;
		default: break;
	}

	return CRGBAF (0.0f, 0.0f, 0.0f, 0.0f);
}


void	CBitmap::loadSize(NLMISC::IStream &f, uint32 &retWidth, uint32 &retHeight)
{
	retWidth= 0;
	retHeight= 0;


	nlassert(f.isReading()); 
	
	// testing if DDS
	uint32 fileType = 0;
	f.serial(fileType);
	if(fileType == DDS)
	{
		// read entire DDS header.
		uint32 size = 0;
		f.serial(size); // size in Bytes of header(without "DDS")
		uint32 * _DDSSurfaceDesc = new uint32[size]; 
		std::auto_ptr<uint32> _DDSSurfaceDescAuto(_DDSSurfaceDesc);
		_DDSSurfaceDesc[0]= size;

		for(uint i= 0; i<size/4 - 1; i++)
		{
			f.serial(_DDSSurfaceDesc[i+1]);
		}
		
		// flags determines which members of the header structure contain valid data
		uint32 flags = _DDSSurfaceDesc[1];

		//verify if file have linearsize set
		if(!(flags & DDSD_LINEARSIZE)) 
		{
			throw EDDSBadHeader();
		}
		
		//-------------- extracting and testing useful info
		retWidth = _DDSSurfaceDesc[2];
		retHeight  = _DDSSurfaceDesc[3];
	}
	// assuming it's TGA
	else 
	{
		if(!f.seek (0, NLMISC::IStream::begin))
		{
			throw ESeekFailed();
		}

		// Reading header, 
		// To make sure that the bitmap is TGA, we check imageType and imageDepth.
		uint8	lengthID;
		uint8	cMapType;
		uint8	imageType;
		uint16	tgaOrigin;
		uint16	length;
		uint8	depth;
		uint16	xOrg;
		uint16	yOrg;
		uint16	width;
		uint16	height;
		uint8	imageDepth;
		uint8	desc;
		
		f.serial(lengthID);
		f.serial(cMapType);
		f.serial(imageType);
		if(imageType!=2 && imageType!=3 && imageType!=10 && imageType!=11) return;
		f.serial(tgaOrigin);
		f.serial(length);
		f.serial(depth);
		f.serial(xOrg);
		f.serial(yOrg);
		f.serial(width);
		f.serial(height);
		f.serial(imageDepth);
		if(imageDepth!=8 && imageDepth!=24 && imageDepth!=32) return;
		f.serial(desc);

		// Ok, we have width and height.
		retWidth= width;
		retHeight= height;
	}

	// reset stream.
	if(!f.seek (0, NLMISC::IStream::begin))
	{
		throw ESeekFailed();
	}
}


void	CBitmap::loadSize(const std::string &path, uint32 &retWidth, uint32 &retHeight)
{
	retWidth= 0;
	retHeight= 0;

	CIFile		f(path);
	if(f.open(path))
		loadSize(f, retWidth, retHeight);
}


void	CBitmap::flipH()
{
	if (PixelFormat != RGBA)
		return;

	sint32 nWidth = getWidth(0);
	sint32 nHeight = getHeight(0);
	sint32 i, j;
	NLMISC::CRGBA *pBitmap = (NLMISC::CRGBA*)&_Data[0][0];
	bool needRebuild = false;
	CRGBA temp;

	if(_MipMapCount>1)
		needRebuild = true;
	releaseMipMaps();

	for( i = 0; i < nHeight; ++i )
		for( j = 0; j < nWidth/2; ++j )
		{
			temp = pBitmap[i*nWidth+j];
			pBitmap[i*nWidth+j] = pBitmap[i*nWidth+nWidth-j-1];
			pBitmap[i*nWidth+nWidth-j-1] = temp;
		}

	// Rebuilding mipmaps
	if(needRebuild)
	{
		buildMipMaps();
	}
}


void	CBitmap::flipV()
{
	if (PixelFormat != RGBA)
		return;

	sint32 nWidth = getWidth(0);
	sint32 nHeight = getHeight(0);
	sint32 i, j;
	NLMISC::CRGBA *pBitmap = (NLMISC::CRGBA*)&_Data[0][0];
	bool needRebuild = false;
	CRGBA temp;

	if(_MipMapCount>1)
		needRebuild = true;
	releaseMipMaps();

	for( j = 0; j < nHeight/2; ++j )
		for( i = 0; i < nWidth; ++i )
		{
			temp = pBitmap[j*nWidth+i];
			pBitmap[j*nWidth+i] = pBitmap[(nHeight-j-1)*nWidth+i];
			pBitmap[(nHeight-j-1)*nWidth+i] = temp;
		}

	// Rebuilding mipmaps
	if(needRebuild)
	{
		buildMipMaps();
	}
}


void	CBitmap::rot90CW()
{
	if (PixelFormat != RGBA)
		return;
	sint32 nWidth = getWidth(0);
	sint32 nHeight = getHeight(0);
	sint32 i, j;
	NLMISC::CRGBA *pSrcRgba = (NLMISC::CRGBA*)&_Data[0][0];
	bool needRebuild = false;

	if(_MipMapCount>1)
		needRebuild = true;
	releaseMipMaps();

	std::vector<uint8> pDestui;
	pDestui.resize(nWidth*nHeight*4);
	NLMISC::CRGBA *pDestRgba = (NLMISC::CRGBA*)&pDestui[0];

	for( j = 0; j < nHeight; ++j )
	for( i = 0; i < nWidth;  ++i )
		pDestRgba[j+i*nHeight] = pSrcRgba[i+(nHeight-1-j)*nWidth];

	uint32 nTemp = _Width;
	_Width = _Height;
	_Height = nTemp;

	NLMISC::contReset(_Data[0]); // free memory
	_Data[0] =  pDestui;
	// Rebuilding mipmaps
	if(needRebuild)
	{
		buildMipMaps();
	}
}

void	CBitmap::rot90CCW()
{
	if (PixelFormat != RGBA)
		return;
	sint32 nWidth = getWidth(0);
	sint32 nHeight = getHeight(0);
	sint32 i, j;
	NLMISC::CRGBA *pSrcRgba = (NLMISC::CRGBA*)&_Data[0][0];
	bool needRebuild = false;

	if(_MipMapCount>1)
		needRebuild = true;
	releaseMipMaps();

	std::vector<uint8> pDestui;
	pDestui.resize(nWidth*nHeight*4);
	NLMISC::CRGBA *pDestRgba = (NLMISC::CRGBA*)&pDestui[0];

	for( j = 0; j < nHeight; ++j )
	for( i = 0; i < nWidth;  ++i )
		pDestRgba[j+i*nHeight] = pSrcRgba[nWidth-1-i+j*nWidth];

	uint32 nTemp = _Width;
	_Width = _Height;
	_Height = nTemp;

	NLMISC::contReset(_Data[0]); // free memory
	_Data[0] =  pDestui;
	// Rebuilding mipmaps
	if(needRebuild)
	{
		buildMipMaps();
	}
}

//===========================================================================

void CBitmap::blend(const CBitmap &Bm0, const CBitmap &Bm1, uint16 factor)
{
	nlassert(factor <= 256);

	nlassert(Bm0._Width != 0 && Bm0._Height != 0
			 && Bm1._Width != 0 && Bm1._Height != 0);

	nlassert(Bm0._Width  == Bm1._Width);	// the bitmap should have the same size
	nlassert(Bm0._Height == Bm1._Height);

	const CBitmap *nBm0, *nBm1; // pointer to the bitmap that is used for blending, or to a copy is a conversion wa required

	static CBitmap cp0, cp1; // these bitmap are copies of Bm1 and Bm0 if a conversion was needed

	if (Bm0.PixelFormat != RGBA)
	{
		cp0 = Bm0;
		cp0.convertToRGBA();
		nBm0 = &cp0;
	}
	else
	{
		nBm0 = &Bm0;
	}


	if (Bm1.PixelFormat != RGBA)
	{
		cp1 = Bm1;
		cp1.convertToRGBA();
		nBm1 = &cp1;
	}
	else
	{
		nBm1 = &Bm1;
	}

	this->resize(Bm0._Width, Bm0._Height, RGBA);

	const  uint numPix = _Width * _Height; // 4 component per pixels


	const uint8 *src0		= &(nBm0->_Data[0][0]);
	const uint8 *src1		= &(nBm1->_Data[0][0]);
	uint8 *dest				= &(this->_Data[0][0]);
	uint8 *endPix			= dest + (numPix << 2);


	uint blendFact    = (uint) factor;
	uint invblendFact = 256 - blendFact;

	do
	{
		/// blend 4 component at each pass
		*dest = (uint8) (((blendFact * *src1)		+ (invblendFact * *src0)) >> 8);
		*(dest + 1) = (uint8) (((blendFact * *(src1 + 1)) + (invblendFact * *(src0 + 1))) >> 8);
		*(dest + 2) = (uint8) (((blendFact * *(src1 + 2)) + (invblendFact * *(src0 + 2))) >> 8);
		*(dest + 3)  = (uint8) (((blendFact * *(src1 + 3)) + (invblendFact * *(src0 + 3))) >> 8);

		src0 = src0 + 4;
		src1 = src1 + 4;
		dest = dest + 4;	
	}
	while (dest != endPix);
}



//-----------------------------------------------
CRGBA CBitmap::getRGBAPixel(sint x, sint y, uint32 numMipMap /*=0*/) const
{
	uint w = getWidth(numMipMap);
	uint h = getHeight(numMipMap);
	if (w == 0 || (uint) x >= w || (uint) y >= h) return CRGBA::Black; // include negative cases
	const uint8 *pix = &getPixels(numMipMap)[(x + y * w) << 2];
	return CRGBA(pix[0], pix[1], pix[2], pix[3]);
}

//-----------------------------------------------
CRGBA CBitmap::getDXTCColorFromBlock(const uint8 *block, sint x, sint y)
{
	uint16  col0;
	uint16  col1;
	memcpy(&col0, block, sizeof(uint16));
	memcpy(&col1, block + 2, sizeof(uint16));
	uint	colIndex = (block[4 + (y & 3)] >> ((x & 3) << 1)) & 3;
	CRGBA   result, c0, c1;
	if (col0 > col1)
	{	
		switch(colIndex)
		{
			case 0:
				uncompress(col0, result);				
			break;
			case 1:
				uncompress(col1, result);
			break;
			case 2:
				uncompress(col0, c0);
				uncompress(col1, c1);
				result.blendFromui(c0, c1, 85);
			break;
			case 3:
				uncompress(col0, c0);
				uncompress(col1, c1);
				result.blendFromui(c0, c1, 171);
			break;
		}
		result.A = 255;
	}
	else
	{
		switch(colIndex)
		{
			case 0:
				uncompress(col0, result);
				result.A = 255;
			break;
			case 1:
				uncompress(col1, result);
				result.A = 255;
			break;
			case 2:
				uncompress(col0, c0);
				uncompress(col1, c1);
				result.blendFromui(c0, c1, 128);
				result.A = 255;
			break;
			case 3:
				result.set(0, 0, 0, 0);				
			break;
		}
	}	
	return result;
}

//-----------------------------------------------
CRGBA CBitmap::getDXTC1Texel(sint x, sint y, uint32 numMipMap) const
{
	uint w = getWidth(numMipMap);
	uint h = getHeight(numMipMap);
	if (w == 0 || h == 0 || (uint) x >= w || (uint) y >= h) return CRGBA::Black; // include negative cases	
	uint numRowBlocks   = std::max((w + 3) >> 2, 1u);
	const uint8 *pix    = &getPixels(numMipMap)[0];
	const uint8 *block  = pix + ((y >> 2) * (numRowBlocks << 3) + ((x >> 2) << 3));	
	return getDXTCColorFromBlock(block, x, y);
}


//-----------------------------------------------
CRGBA CBitmap::getDXTC3Texel(sint x, sint y, uint32 numMipMap) const
{
	uint w = getWidth(numMipMap);
	uint h = getHeight(numMipMap);
	if (w == 0 || h == 0 || (uint) x >= w || (uint) y >= h) return CRGBA::Black; // include negative cases	
	uint numRowBlocks   = std::max((w + 3) >> 2, 1u);
	const uint8 *pix    = &getPixels(numMipMap)[0];
	const uint8 *block  = pix + ((y >> 2) * (numRowBlocks << 4) + ((x >> 2) << 4));	
	CRGBA result = getDXTCColorFromBlock(block + 8, x, y);
	// get alpha part
	uint8 alphaByte = block[((y & 3) << 1) + ((x & 2) >> 1)];
	result.A = (x & 1) ?  (alphaByte & 0xf0) : (alphaByte << 4);
	return result;
}

//-----------------------------------------------
CRGBA CBitmap::getDXTC5Texel(sint x, sint y, uint32 numMipMap) const
{
	uint w = getWidth(numMipMap);
	uint h = getHeight(numMipMap);
	if (w == 0 || h == 0 || (uint) x >= w || (uint) y >= h) return CRGBA::Black; // include negative cases	
	uint numRowBlocks   = std::max((w + 3) >> 2, 1u);
	const uint8 *pix    = &getPixels(numMipMap)[0];
	const uint8 *block  = pix + ((y >> 2) * (numRowBlocks << 4) + ((x >> 2) << 4));	
	CRGBA result = getDXTCColorFromBlock(block + 8, x, y);
	// get alpha part
	uint8 alpha0 = block[0];
	uint8 alpha1 = block[1];
	
	uint alphaIndex;
	uint tripletIndex = (x & 3) + ((y & 3) << 2);
	if (tripletIndex < 8)
	{
		alphaIndex = (((uint32 &) block[2]) >> (tripletIndex * 3)) & 7;
	}
	else
	{
		alphaIndex = (((uint32 &) block[5]) >> ((tripletIndex - 8) * 3)) & 7; // we can read a dword there because there are color datas following he alpha datas
	}

	if (alpha0 > alpha1)
	{
		switch (alphaIndex)
		{
			case 0: result.A = alpha0; break;
			case 1: result.A = alpha1; break;
			case 2: result.A = (uint8) ((6 * (uint) alpha0 + (uint) alpha1) / 7); break;
			case 3: result.A = (uint8) ((5 * (uint) alpha0 + 2 * (uint) alpha1) / 7); break;
			case 4: result.A = (uint8) ((4 * (uint) alpha0 + 3 * (uint) alpha1) / 7); break;
			case 5: result.A = (uint8) ((3 * (uint) alpha0 + 4 * (uint) alpha1) / 7); break;
			case 6: result.A = (uint8) ((2 * (uint) alpha0 + 5 * (uint) alpha1) / 7); break;
			case 7: result.A = (uint8) (((uint) alpha0 + (uint) 6 * alpha1) / 7); break;
		}
	}
	else
	{
		switch (alphaIndex)
		{
			case 0: result.A = alpha0; break;
			case 1: result.A = alpha1; break;
			case 2: result.A = (uint8) ((4 * (uint) alpha0 + (uint) alpha1) / 5); break;
			case 3: result.A = (uint8) ((3 * (uint) alpha0 + 2 * (uint) alpha1) / 5); break;
			case 4: result.A = (uint8) ((2 * (uint) alpha0 + 3 * (uint) alpha1) / 5); break;
			case 5: result.A = (uint8) (((uint) alpha0 + 4 * (uint) alpha1) / 5); break;
			case 6: result.A = 0;	break;
			case 7: result.A = 255; break;
		}
	}
	return result;	
}


//-----------------------------------------------
CRGBA CBitmap::getPixelColor(sint x, sint y, uint32 numMipMap /*=0*/) const
{
	
	switch (PixelFormat)
	{
		case RGBA:	
			return getRGBAPixel(x, y, numMipMap);		
		case DXTC1:
		case DXTC1Alpha: 
			return getDXTC1Texel(x, y, numMipMap);
		case DXTC3:
			return getDXTC3Texel(x, y, numMipMap);
		case DXTC5:
			return getDXTC5Texel(x, y, numMipMap);
		default:
			nlstop;
		break;
	}
	return CRGBA::Black;
}


} // NLMISC
