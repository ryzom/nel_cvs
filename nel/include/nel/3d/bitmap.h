/** \file bitmap.h
 * Class managing bitmaps
 *
 * $Id: bitmap.h,v 1.5 2000/11/13 13:31:07 coutelas Exp $
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

#ifndef NL_BITMAP_H
#define NL_BITMAP_H

#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"
#include "nel/misc/debug.h"
#include <vector>

namespace NLMISC 
{
	class IStream;
}


namespace NL3D {



//------------------ DDS STUFFS --------------------

#ifndef NL_MAKEFOURCC
    #define NL_MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((uint32)(uint8)(ch0) | ((uint32)(uint8)(ch1) << 8) |   \
                ((uint32)(uint8)(ch2) << 16) | ((uint32)(uint8)(ch3) << 24 ))
#endif 

const uint32	DDS = NL_MAKEFOURCC('D', 'D', 'S', ' ');
const uint32	DXT_ = NL_MAKEFOURCC('D','X', 'T', '\0');


// dwLinearSize is valid
#define DDSD_LINEARSIZE         0x00080000l


//---------------- END OF DDS STUFFS ------------------


const uint8	MAX_MIPMAP = 12;



/** 
 * To know if the value is a power of two.
 * \param v an integer.
 * \return true if value is a power of 2, else return false.
 */
bool isPowerOf2(sint32 v);


/** return the power of 2 higher than the value
 * \param v an integer.
 * \return the power of 2.
 */
uint32 getNextPowerOf2(uint32 v);




/**
 * Class Bitmap
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2000
 */
class CBitmap
{
protected :
	std::vector<uint8> _Data[MAX_MIPMAP];

	uint8 _MipMapCount;
	uint32 _Width;
	uint32 _Height;

private :
	

	/** 
	 * blend 2 integers between 0 and 255 .
	 * \param n0 first integer
	 * \param n1 second integer
	 * \param coef coefficient for the first integer (must be in [0,256])
	 * \author Stephane Coutelas
	 * \date 2000
	 */
	uint32 blend(uint32 &n0, uint32 &n1, uint32 coef0);

	/** 
	 * Read a DDS from an IStream. 
	 * The bitmap is readen as a set of bytes and stocked compressed.
	 * Width and Height are multiple of 4.
	 * If there's no mipmap, _MipMapCount is equal to 0. If there are mipmaps
	 * _MipMapCount include the initial image.
	 * \param IStream The stream must be in reading mode.
	 * \return image depth
	 * \author Stephane Coutelas
	 * \date 2000
	 * \throw EDDSBadHeader : surface is header is not valid.
	 */
	uint8 readDDS(NLMISC::IStream &f);


	/** 
	 * Read a TGA from an IStream.
	 * TGA pictures can be in 24 or 32 bits, RLE or uncompressed
	 * \param f IStream (must be a reading stream)
	 * \return image depth if succeed, 0 else
	 * \author Stephane Coutelas
	 * \date 2000
	 */
	uint8 readTGA(	NLMISC::IStream &f);



	/** 
	 * Change bitmap format 
	 *
	 * about DXTC1 to DXTC5 :
	 * Does nothing if the format is not DXTC1
	 * about alpha encoding :
	 *		alpha0 == alpha1
	 *		code(x,y) == 7 for every (x,y)
	 *
	 * about luminance to alpha and alpha to luminance :
	 *      the buffer keeps unchanged
	 *
	 * \author Stephane Coutelas
	 * \date 2000
	 */
	///@{
	bool convertToDXTC5();

	bool convertToRGBA();
	bool luminanceToRGBA();
	bool alphaToRGBA();
	bool alphaLuminanceToRGBA();
	
	bool convertToLuminance();
	bool rgbaToLuminance();
	bool alphaToLuminance();
	bool alphaLuminanceToLuminance();
	
	bool convertToAlpha();
	bool rgbaToAlpha();
	bool luminanceToAlpha();
	bool alphaLuminanceToAlpha();

	bool convertToAlphaLuminance();
	bool rgbaToAlphaLuminance();
	bool luminanceToAlphaLuminance();
	bool alphaToAlphaLuminance();
	
	///@}

	/** 
	 * Decompress bitmap compressed with S3TC DXT1 algorithm. 
	 * \param alpha if alpha is true there's alpha.
	 * \author Stephane Coutelas
	 * \date 2000
	 */
	bool decompressDXT1(bool alpha);

	/** 
	 * Decompress bitmap compressed with S3TC DXT3 algorithm. 
	 * \author Stephane Coutelas
	 * \date 2000
	 * \throw EAllocationFailure : can't allocate memory.
	 */
	bool decompressDXT3();


	/** 
	 * Decompress bitmap compressed with S3TC DXT3 algorithm. 
	 * \author Stephane Coutelas
	 * \date 2000
	 * \throw EAllocationFailure : can't allocate memory.
	 */
	bool decompressDXT5();


	/** 
	 * Extracting RGBA infos from a 16bits word. (used by S3TC decompression)
	 * \param color a 16bits integer
	 * \param r a CRGBA
	 * \author Stephane Coutelas
	 * \date 2000
	 */
	void uncompress(uint16 color, NLMISC::CRGBA &);


	/** 
	 * The resample function
	 * \param pSrc CRGBA array
	 * \param pDest CRGBA array for storing resampled texture
	 * \param nSrcWidth original width
	 * \param nSrcHeight original height
	 * \param nDestWidth width after resample
	 * \param nDestHeight height after resample
	 * \author Cyril Corvazier, integrated by Stephane Coutelas
	 * \date 2000
	 */	
	void resamplePicture32 (const NLMISC::CRGBA *pSrc, NLMISC::CRGBA *pDest, 
							 sint32 nSrcWidth, sint32 nSrcHeight, 
							 sint32 nDestWidth, sint32 nDestHeight);


public:

	enum TType { RGBA, 
				 DXTC1 = NL_MAKEFOURCC('D','X', 'T', '1'),
				 DXTC1Alpha,
				 DXTC3 = NL_MAKEFOURCC('D','X', 'T', '3'), 
				 DXTC5 = NL_MAKEFOURCC('D','X', 'T', '5'), 
				 TGA,
				 LUMINANCE,
				 ALPHA,
				 ALPHA_LUMINANCE 
	} PixelFormat;


	CBitmap()
	{
		_MipMapCount = 0;
		_Width = 0;
		_Height = 0;
		PixelFormat = RGBA;
	}



	/** 
	 * Read a bitmap(TGA or DDS) from an IStream. 
	 * Bitmap supported are DDS (DXTC1, DXTC1 with Alpha, DXTC3, DXTC5, and
	 * uncompressed TGA (24 and 32 bits).
	 * \param IStream The stream must be in reading mode.
	 * \return image depth (24 or 32), or 0 if load failed
	 * \author Stephane Coutelas
	 * \date 2000
	 * \throw ESeekFailed : seek has failed
	 */
	uint8 load(NLMISC::IStream &f);

	/** 
	 * Return the pixels buffer of the image, or of one of its mipmap.
	 * Return a reference of an array in pixel format get with getPixelFormat().
	 * \return vector<uint8>& RGBA pixels
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	///@{
	std::vector<uint8>& getPixels(uint32 numMipMap = 0) 
	{ 
		//nlassert (numMipMap<=_MipMapCount);
		nlinfo("%d %d %d %d",_Data[0][0],_Data[0][1],_Data[0][2],_Data[0][3]);
		return _Data[numMipMap];
	}
	const std::vector<uint8>& getPixels(uint32 numMipMap = 0) const
	{
		//nlassert (numMipMap<=_MipMapCount);
		return _Data[numMipMap]; 
	}
	///@}
	
	
	/**
	 * Convert bitmap to another type
	 * \param type new type for the bitmap
	 * \return true if conversion succeeded, false else
	 * \author Stephane Coutelas
	 * \date 2000
	 */
	bool convertToType (TType type);



	/** 
	 * Return the format of pixels stored at the present time in the object buffer.
	 * \return Pixel format (DXTC1,DXTC1A, DXTC3, DXTC5, PIC_TGA, PIC_RGBA)
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	TType getPixelFormat() const
	{
		return PixelFormat; 
	}


	/** 
	 * Return the image width, or a mipmap width.
	 * \param mipMap mipmap level 
	 * \return image width (0 if mipmap not found)
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	uint32 getWidth(uint32 numMipMap = 0) const;


	/** 
	 * Return the image height, or a mipmap height.
	 * \param mipMap mipmap level 
	 * \return image height (0 if mipmap not found)
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	uint32 getHeight(uint32 numMipMap = 0) const;


	/** 
	 * Return the number of mipmaps.
	 * \return number of mipmaps
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	uint32 getMipMapCount() const
	{
		return _MipMapCount; 
	}


	/** 
	 * Build the mipmaps of the bitmap if they don't exist.
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	void buildMiMaps();


	/** 
	 * Reset the buffer. Mipmaps are deleted and bitmap is not valid anymore.
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	void reset();
	
		
	/** 
	 * Resample the bitmap. If mipmaps exist they are deleted, then rebuilt
	 * after resampling.
	 * \param nNewWidth width after resample
	 * \param nNewHeight height after resample
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	void resample (sint32 nNewWidth, sint32 nNewHeight);


	/** 
	 * Write a TGA (24 or 32 bits) from the object pixels buffer.
	 * If the current pixel format is not RGBA then the method does nothing
	 * \param f IStream (must be a reading stream)
	 * \param d depth : 24 or 32
	 * \return 1 if succeed, 0 else
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	uint32 writeTGA(NLMISC::IStream &f, uint32 d);

};


} // NL3D


#endif // NL_BITMAP_H

/* End of bitmap.h */
