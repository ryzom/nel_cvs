/** \file bitmap.h
 * <File description>
 *
 * $Id: bitmap.h,v 1.1 2000/10/23 14:16:01 coutelas Exp $
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
#include "nel/misc/stream.h"


namespace NL3D {



//------------------ DDS STUFFS --------------------

#ifndef MAKEFOURCC
    #define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((uint32)(uint8)(ch0) | ((uint32)(uint8)(ch1) << 8) |   \
                ((uint32)(uint8)(ch2) << 16) | ((uint32)(uint8)(ch3) << 24 ))
#endif 

const uint32	DDS_HEADER = MAKEFOURCC('D', 'D', 'S', ' ');
const uint32	DXT1 = MAKEFOURCC('D','X', 'T', '1');
const uint32	DXT2 = MAKEFOURCC('D','X', 'T', '2');
const uint32	DXT3 = MAKEFOURCC('D','X', 'T', '3');
const uint32	DXT4 = MAKEFOURCC('D','X', 'T', '4');
const uint32	DXT5 = MAKEFOURCC('D','X', 'T', '5');
const uint32	DXT_ = MAKEFOURCC('D','X', 'T', '\0');

const uint8	MAX_MIPMAP = 12;


// ddsCaps field is valid.
#define DDSD_CAPS               0x00000001l     

//dwHeight field is valid.
#define DDSD_HEIGHT             0x00000002l

//dwWidth field is valid.
#define DDSD_WIDTH              0x00000004l

//lPitch is valid.
#define DDSD_PITCH              0x00000008l

// dwBackBufferCount is valid.
#define DDSD_BACKBUFFERCOUNT    0x00000020l

// dwAlphaBitDepth is valid.
#define DDSD_ALPHABITDEPTH      0x00000080l

//lpSurface is valid.
#define DDSD_LPSURFACE          0x00000800l

//ddpfPixelFormat is valid.
#define DDSD_PIXELFORMAT        0x00001000l

//ddckCKDestOverlay is valid.
#define DDSD_CKDESTOVERLAY      0x00002000l

// ddckCKDestBlt is valid.
#define DDSD_CKDESTBLT          0x00004000l

// ddckCKSrcOverlay is valid.
#define DDSD_CKSRCOVERLAY       0x00008000l

// ddckCKSrcBlt is valid.
#define DDSD_CKSRCBLT           0x00010000l

//dwMipMapCount is valid.
#define DDSD_MIPMAPCOUNT        0x00020000l

// dwRefreshRate is valid
#define DDSD_REFRESHRATE        0x00040000l

// dwLinearSize is valid
#define DDSD_LINEARSIZE         0x00080000l

// dwTextureStage is valid
#define DDSD_TEXTURESTAGE       0x00100000l

// dwFVF is valid
#define DDSD_FVF                0x00200000l

// dwSrcVBHandle is valid
#define DDSD_SRCVBHANDLE        0x00400000l

// All input fields are valid.
#define DDSD_ALL                0x007ff9eel


typedef union 
{ 
	uint64	Alpha;
	uint16	Color0;
	uint16	Color1;
	uint32	Bits;
} 
DXTBlock;


//---------------- END OF DDS STUFFS ------------------



/**
 * <Class description>
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2000
 */
class CBitmap
{
	std::vector<NLMISC::CRGBA> _DataDecomp[MAX_MIPMAP];
	std::vector<uint8> _Data;
	std::vector<DXTBlock> _DataComp;
	
	uint32 _PixelFormat;
	uint32 _MipMapCount;
	uint32 _Width;
	uint32 _Height;

	
	void loadDDS(NLMISC::IStream &f);
	void decompressDXT1();
	void decompressDXT3();
	void decompressDXT5();
	void uncompress(uint16 color, NLMISC::CRGBA &);


public:

	/// Constructor
	CBitmap();

	void load(NLMISC::IStream &f);
	std::vector<NLMISC::CRGBA> getPixels(uint32 numMipMap = 0) const;
	void convertToRGBA();
	uint32 getPixelFormat() const;
	uint32 getWidth() const;
	uint32 getHeight() const;
	uint32 getNumMipMap() const;


};





} // NL3D


#endif // NL_BITMAP_H

/* End of bitmap.h */
