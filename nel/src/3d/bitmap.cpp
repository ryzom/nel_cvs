/** \file bitmap.cpp
 * <File description>
 *
 * $Id: bitmap.cpp,v 1.1 2000/10/23 14:16:01 coutelas Exp $
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

#include "nel/3d/bitmap.h"


namespace NL3D {


void blendFromui(NLMISC::CRGBA &c0, NLMISC::CRGBA &c1, uint coef);


/*
 * Constructor
 */
CBitmap::CBitmap()
{
	
}

void CBitmap::load(NLMISC::IStream &f) 
{
	if(f.isReading()) 
	{
		uint32 fileType;
		
		f.serial(fileType);
		if(fileType == DDS_HEADER)
		{
			loadDDS(f);
			int z;
			z=0;
		}
		else
		{
			NLMISC::IStream::TSeekOrigin origin= f.begin;
			if(!f.seek (0, origin))
			{
				//error
			}
		}
	}
	else
	{
		//error
	}

}


void CBitmap::loadDDS(NLMISC::IStream &f)
{
		
	//------------------ Reading Header ------------------------

	//-------------- reading entire header
	
	uint32 size;
	f.serial(size); // size in Bytes of header(without "DDS")
	uint32 *_DDSSurfaceDesc = (uint32*) malloc(size); 
	_DDSSurfaceDesc[0]= size;

	for(uint32 i= 0; i<size/4 - 1; i++)
	{
		f.serial(_DDSSurfaceDesc[i+1]);
	}
	
	
	//-------------- extracting useful info

	_Height = _DDSSurfaceDesc[2];
	_Width  = _DDSSurfaceDesc[3];
	_MipMapCount= _DDSSurfaceDesc[6];
	

	//------------- looking for pixel format

	_PixelFormat= _DDSSurfaceDesc[20];


	//------------- extracting mip levels compressed data

	// flags determines which members of the header structure contain valid data
	uint32 flags = _DDSSurfaceDesc[1];
	uint32 linearSize = _DDSSurfaceDesc[4];
	uint32 lPitch = _DDSSurfaceDesc[4];
	uint32 RGBBitCount = _DDSSurfaceDesc[21];
	
	/*
	if(flags & DDSD_LINEARSIZE ) //file have linearsize set
    {
		for(uint8 i = 0; i<linearSize; i++)
		{
			f.serial(_Data[i]);
		}
	}
	else // file doesn't have linearsize set
	{
	    uint32 bytesPerRow = _Width * RGBBitCount / 8;

		uint8* dataTmp;
		for(uint8 i = 0; i < _Height; i++ )
        {
			for(uint8 j = 0; j < bytesPerRow; j++ )
			{
				f.serial(dataTmp[j]); 
			}
			dataTmp+= lPitch;
        }
	}
	*/

	uint32 wBlocks= _Width/4;
	uint32 hBlocks= _Height/4;
	DXTBlock block;

	
	for(uint32 j=0; j < hBlocks; j++)
	{
		for(uint32 i=0; i < wBlocks; i++)
		{
			block.Alpha= 0;
			block.Bits= 0;
			block.Color0= 0;
			block.Color0= 0;

			if(_PixelFormat== DXT1)
			{
				try 
				{
					f.serial(block.Color0);
					f.serial(block.Color1);
					f.serial(block.Bits);
					_DataComp.push_back(block);
				}
				catch (NLMISC::EStream stream)
				{
					
				}
			}
			else
			if(_PixelFormat== DXT3)
			{
				f.serial(block.Alpha);
				f.serial(block.Color0);
				f.serial(block.Color1);
				f.serial(block.Bits);
				_DataComp.push_back(block);
			}
			else
			if(_PixelFormat== DXT5)
			{
				f.serial(block.Alpha);
				f.serial(block.Color0);
				f.serial(block.Color1);
				f.serial(block.Bits);
				_DataComp.push_back(block);
			}
			else
			{
				// unhandled format
				
			}
		}
	}
}


void CBitmap::convertToRGBA()
{
	switch(_PixelFormat)
	{
		case DXT1 :
			decompressDXT1();
			break;

		case DXT3 :
			decompressDXT3();	
			break;

		case DXT5 :
			decompressDXT5();		
			break;

		default:
			break;
	}
}

/* old version (uint8*)
void CBitmap::decompressDXT1()
{
	uint32	wBlocks;
	uint32	hBlocks;
	NLMISC::CRGBA	c[4];
	std::vector<NLMISC::CRGBA>	bitmap;
	
	wBlocks= _Width/4;
	hBlocks= _Height/4;

	for(uint32 j=0; j < hBlocks; j++)
	{
		for(uint32 i=0; i < wBlocks; i++)
		{
			uint32 k = j*8*wBlocks + i*8; // begining of current block
			uint16	color0 = ((uint16)_Data[k])<<8 | (uint16)_Data[k + 1];
			uint16	color1 = ((uint16)_Data[k + 2])<<8 | (uint16)_Data[k + 3];
			
			uncompress(color0, c[0]);	c[0].A= 1;
			uncompress(color1, c[1]);	c[1].A= 1;
			
			if(color0>color1)
			{
				c[2].blendFromui(c[0], c[1], 85);	c[2].A= 1;
				c[3].blendFromui(c[0], c[1], 171);	c[3].A= 1;
			}
			else
			{
				c[2].blendFromui(c[0], c[1], 128);	c[2].A= 1;
				c[3].set(0,0,0,1);
			}

			// computing the 16 texels of the block
			for(uint8 l=0; l<4; l++)
			{
				uint8 tmp= _Data[k+l];
				for(uint8 m=0; m<4; m++)
				{
					bitmap[k + 4*l+m]= c[tmp&3]; // using the 2 LSB
					tmp>>=2;
				}
			}
		}
	}
}
*/


void CBitmap::decompressDXT1()
{
	NLMISC::CRGBA	c[4];
	uint32	wBlocks= _Width/4;
	uint32	hBlocks= _Height/4;

	for(uint32 j=0; j < hBlocks; j++)
	{
		for(uint32 i=0; i < wBlocks; i++)
		{
			uint16 color0= _DataComp[j*wBlocks+i].Color0;
			uint16 color1= _DataComp[j*wBlocks+i].Color1;
			uint32 bits= _DataComp[j*wBlocks+i].Bits;
			
			uncompress(color0, c[0]);	c[0].A= 1;
			uncompress(color1, c[1]);	c[1].A= 1;
			
			if(color0>color1)
			{
				c[2].blendFromui(c[0], c[1], 85);	c[2].A= 1;
				c[3].blendFromui(c[0], c[1], 171);	c[3].A= 1;
			}
			else
			{
				c[2].blendFromui(c[0], c[1], 128);	c[2].A= 1;
				c[3].set(0,0,0,1);
			}

			// computing the 16 RGBA of the block
			for(uint8 l=0; l<16; l++)
			{
				_DataDecomp[0].push_back(c[bits&3]); // using the 2 LSB
				bits>>=2;
			}
		}
	}
}

void CBitmap::decompressDXT3()
{
	uint32	wBlocks;
	uint32	hBlocks;
	NLMISC::CRGBA	c[4];
	uint8 alpha[16];
	std::vector<NLMISC::CRGBA>	bitmap;
	
	wBlocks= _Width/4;
	hBlocks= _Height/4;

	for(uint32 j=0; j < hBlocks; j++)
	{
		for(uint32 i=0; i < wBlocks; i++)
		{
			uint32 k = j*16*wBlocks + i*16; // begining of current block
			
			// computing the 16 alpha values 
			for(uint8 l=0; l<8; l++)
			{
				uint8 tmp= _Data[k+l];
				for(uint8 m=0; m<2; m++)
				{
					alpha[l+k]= (tmp&3) / 15; 
					tmp>>=2;
				}
			}
			k+=8; // begining of RGB part

			uint16	color0 = ((uint16)_Data[k])<<8 | (uint16)_Data[k + 1];
			uint16	color1 = ((uint16)_Data[k + 2])<<8 | (uint16)_Data[k + 3];
			
			uncompress(color0, c[0]);	c[0].A= 1;
			uncompress(color1, c[1]);	c[1].A= 1;
			
			if(color0>color1)
			{
				c[2].blendFromui(c[0], c[1], 85);	c[2].A= 1;
				c[3].blendFromui(c[0], c[1], 171);	c[3].A= 1;
			}
			else
			{
				c[2].blendFromui(c[0], c[1], 128);	c[2].A= 1;
				c[3].set(0,0,0,1);
			}

			// computing the 16 texels of the block
			for(uint8 m=0; m<4; m++)
			{
				uint8 tmp= _Data[k+m];
				for(uint8 n=0; n<4; n++)
				{
					tmp>>=2;
					bitmap[j*16*wBlocks + i*16 + 4*m+n]= c[tmp&3]; // using the 2 LSB
				}
			}
		}
	}
}


uint32 blend(uint32 &n0, uint32 &n1, uint32 coef0) // coef must be in [0,256]
{
	int	a0 = coef0;
	int	a1 = 256-a0;
	return ((n0*a0 + n1*a1) >>8);
}


void CBitmap::decompressDXT5()
{
	NLMISC::CRGBA	c[4];
	uint32	wBlocks= _Width/4;
	uint32	hBlocks= _Height/4;

	for(uint32 j=0; j < hBlocks; j++)
	{
		for(uint32 i=0; i < wBlocks; i++)
		{
			uint64 bitsAlpha= _DataComp[j*wBlocks+i].Alpha;
			uint16 color0= _DataComp[j*wBlocks+i].Color0;
			uint16 color1= _DataComp[j*wBlocks+i].Color1;
			uint32 bits= _DataComp[j*wBlocks+i].Bits;
			
			
			uint32 alpha[8];
			alpha[0]= (uint8)((bitsAlpha>>48)&255);	alpha[0]/=255;
			alpha[1]= (uint8)((bitsAlpha>>56)&255);	alpha[1]/=255;
			
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
				
			uncompress(color0,c[0]);	c[0].A= 1;
			uncompress(color1,c[1]);	c[1].A= 1;
			
			if(color0>color1)
			{
				c[2].blendFromui(c[0],c[1], 85);	c[2].A= 1;
				c[3].blendFromui(c[0],c[1], 171);	c[3].A= 1;
			}

			// computing the 16 RGBA of the block
			for(uint8 l=0; l<16; l++)
			{
				_DataDecomp[0].push_back(c[bits&3]); // using the 2 LSB
				bits>>=2;
			}
		}
	}
}


inline void CBitmap::uncompress(uint16 color, NLMISC::CRGBA &r)
{
	r.A= 0;
	r.R= ((color>>11)&31) << 3; r.R+= r.R>>5;
	r.G= ((color>>5)&63) << 2;  r.G+= r.G>>6;
	r.B= ((color)&31) << 3;     r.B+= r.B>>5;
}

	
inline uint32 CBitmap::getPixelFormat() const
{
	return _PixelFormat; 
}

inline  std::vector<NLMISC::CRGBA> CBitmap::getPixels(uint32 numMipMap) const
{
	return _DataDecomp[numMipMap]; 
}

inline uint32 CBitmap::getWidth() const
{
	return _Width;
}

inline uint32 CBitmap::getHeight() const
{
	return _Height;
}

inline uint32 CBitmap::getNumMipMap() const
{
	return _MipMapCount; 
}




} // NL3D
