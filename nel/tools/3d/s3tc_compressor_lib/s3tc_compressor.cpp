/** \file s3tc_compressor.cpp
 * <File description>
 *
 * $Id: s3tc_compressor.cpp,v 1.1 2002/10/25 16:17:26 berenguier Exp $
 */

/* Copyright, 2000-2002 Nevrax Ltd.
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

#include "s3tc_compressor.h"
#include "s3_intrf.h"
#include "ddraw.h"


using namespace std;


// ***************************************************************************
static void		compressMipMap(uint8 *pixSrc, sint width, sint height, vector<uint8>	&compdata, DDSURFACEDESC &dest, sint algo)
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
CS3TCCompressor::CS3TCCompressor()
{
}


// ***************************************************************************
void		CS3TCCompressor::compress(const NLMISC::CBitmap &bmpSrc, bool optMipMap, uint algo, NLMISC::IStream &output)
{
	vector<uint8>		CompressedMipMaps;
	DDSURFACEDESC		dest;
	NLMISC::CBitmap		picSrc= bmpSrc;


	// For all mipmaps, compress.
	if(optMipMap)
	{
		// Build the mipmaps.
		picSrc.buildMipMaps();
	}
	for(sint mp= 0;mp<(sint)picSrc.getMipMapCount();mp++)
	{
		uint8	*pixDest;
		uint8	*pixSrc= &(*picSrc.getPixels(mp).begin());
		sint	w= picSrc.getWidth(mp);
		sint	h= picSrc.getHeight(mp);
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
	output.serialBuffer((uint8*)std::string("DDS ").c_str(),4);
	output.serialBuffer((uint8*) &dest2, sizeof(dest2));
	output.serialBuffer(&(*CompressedMipMaps.begin()), CompressedMipMaps.size());
}
