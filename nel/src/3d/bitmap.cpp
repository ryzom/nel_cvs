/** \file bitmap.cpp
 * Class managing bitmaps
 *
 * \todo yoyo: readDDS and decompressDXTC* must wirk in BigEndifan and LittleEndian.
 *
 * $Id: bitmap.cpp,v 1.22 2001/01/30 13:44:16 lecroart Exp $
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


#include <memory>
#include <math.h>

#include "nel/3d/bitmap.h"
#include "nel/misc/stream.h"
#include "nel/misc/common.h"


using namespace NLMISC;


namespace NL3D {


struct EDDSBadHeader : public NLMISC::EStream
{
	virtual const char	*what() const throw() { return "Bad or unrecognized DDS file header"; }
};

struct ESeekFailed : public NLMISC::EStream
{
	virtual const char	*what() const throw() { return "Seek failed"; }
};

struct EAllocationFailure : public Exception
{
	virtual const char	*what() const throw() { return "Can't allocate memory"; }
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
	8		// DXTC5
};

const uint32 CBitmap::DXTC1HEADER = NL_MAKEFOURCC('D','X', 'T', '1');
const uint32 CBitmap::DXTC3HEADER = NL_MAKEFOURCC('D','X', 'T', '3');
const uint32 CBitmap::DXTC5HEADER = NL_MAKEFOURCC('D','X', 'T', '5');

/*-------------------------------------------------------------------*\
								load		
\*-------------------------------------------------------------------*/
uint8 CBitmap::load(NLMISC::IStream &f) 
{
	nlassert(f.isReading()); 
	
	// testing if DDS
	uint32 fileType = 0;;
	f.serial(fileType);
	if(fileType == DDS)
	{
		return readDDS(f);
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
		if(imageType!=2 && imageType!=10) return 0;
		f.serial(tgaOrigin);
		f.serial(length);
		f.serial(depth);
		f.serial(xOrg);
		f.serial(yOrg);
		f.serial(width);
		f.serial(height);
		f.serial(imageDepth);
		if(imageDepth!=24 && imageDepth!=32) return 0;
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
		0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,
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
		0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,
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
uint8 CBitmap::readDDS(NLMISC::IStream &f)
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
	

	//------------- reading mipmap levels compressed data
	
	uint32 linearSize = _DDSSurfaceDesc[4];

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
	}

	return 0;
}




/*-------------------------------------------------------------------*\
							convertToDXTC5
\*-------------------------------------------------------------------*/
bool CBitmap::convertToDXTC5()
{
	uint32 i,j;

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
						
			if(color0>color1)
			{
				c[2].blendFromui(c[0],c[1],85);
				c[3].blendFromui(c[0],c[1],171);	
			}
			else
			{
				c[2].blendFromui(c[0],c[1],128);
				c[3].set(0,0,0,255);
			}

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
			
			if(color0>color1)
			{
				c[2].blendFromui(c[0],c[1],85);
				c[3].blendFromui(c[0],c[1],171);	
			}
			else
			{
				c[2].blendFromui(c[0],c[1],128);
				c[3].set(0,0,0,255);
			}

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
				NLMISC::CRGBAF vColor (0.f, 0.f, 0.f);
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
				NLMISC::CRGBAF vColor (0.f, 0.f, 0.f);
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

	f.seek (-26, f.end);
	f.serial(extAreaOffset);
	f.serial(devDirectoryOffset);
	for(i=0; i<16; i++)
	{
		f.serial(signature[i]);
	}
	if(strncmp(signature,"TRUEVISION-XFILE",16)==0)
		newTgaFormat = true;
	else
		newTgaFormat = false;



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
		};
		break;

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

			for(y=0; y<_Height ;y++)
			{
				// Serial buffer: more efficient way to load.
				f.serialBuffer (scanline, slsize);

				if(imageDepth==24 || imageDepth==32)
				{
					sint32 mult = 3;
					if(imageDepth==32)  
					{
						mult = 4;
					}
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
				
				k=0;
				for(i=0; i<width; i++) 
				{
					if(upSideDown)
					{
						_Data[0][(height-y-1)*width*4 + 4*i] = scanline[k++];
						_Data[0][(height-y-1)*width*4 + 4*i + 1] = scanline[k++];
						_Data[0][(height-y-1)*width*4 + 4*i + 2] = scanline[k++];
						if(imageDepth==32)
							_Data[0][(height-y-1)*width*4 + 4*i + 3] = scanline[k++];
						else
							_Data[0][(height-y-1)*width*4 + 4*i + 3] = 255;
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

			delete scanline;
		};
		break;

		default:
			return 0;
	}

	PixelFormat = RGBA;
	_MipMapCount = 1;
	return(imageDepth);

}



/*-------------------------------------------------------------------*\
							writeTGA
\*-------------------------------------------------------------------*/
bool CBitmap::writeTGA( NLMISC::IStream &f, uint32 d, bool upsideDown)
{
	if(f.isReading()) return false;
	if(d!=24 && d!=32) return false;
	if(PixelFormat != RGBA) return false;

	sint32	i,j,x,y;
	sint32	slsize;
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

	
	slsize = width*d/8;
	scanline = new uint8[slsize];
	if(!scanline)
	{
		throw EAllocationFailure();
	}

	for(y=0; y<(sint32)height; y++)
	{
		
		uint32 k=0;
		for(i=0; i<width*4; i+=4) // 4:RGBA
		{
			for(j=0; j<(sint32)d/8; j++)
			{
				scanline[k++] = _Data[0][(height-y-1)*width*4 + i + j];
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
		
		for(i=0; i<slsize; i++)
		{
			f.serial(scanline[i]);
		}		
	}
	delete scanline;
	return true;
}





} // NL3D
