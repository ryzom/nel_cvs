/** \file texture_far.cpp
 * Texture used to store far textures for several patches.
 *
 * $Id: texture_far.cpp,v 1.22 2002/08/27 14:29:02 berenguier Exp $
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

#include "std3d.h"

#include "3d/texture_far.h"
#include "3d/tile_far_bank.h"
#include "3d/patch.h"
#include "3d/tile_color.h"
#include "3d/zone.h"
#include "3d/landscape.h"
#include "nel/misc/system_info.h"


using namespace NLMISC;
using namespace NL3D;

namespace NL3D {

CRGBA CTextureFar::_LightmapExpanded[NL_NUM_PIXELS_ON_FAR_TILE_EDGE*NL_MAX_TILES_BY_PATCH_EDGE*NL_NUM_PIXELS_ON_FAR_TILE_EDGE*NL_MAX_TILES_BY_PATCH_EDGE];
uint8 CTextureFar::_LumelExpanded[(NL_MAX_TILES_BY_PATCH_EDGE*NL_LUMEL_BY_TILE+1)*(NL_MAX_TILES_BY_PATCH_EDGE*NL_LUMEL_BY_TILE+1)];
CRGBA CTextureFar::_TileTLIColors[(NL_MAX_TILES_BY_PATCH_EDGE+1)*(NL_MAX_TILES_BY_PATCH_EDGE+1)];

CTextureFar::CTextureFar()
{
	// This texture is releasable. It doesn't stays in standard memory after been uploaded into video memory.
	setReleasable (true);

	_ULPrec= this;
	_ULNext= this;
}

CTextureFar::~CTextureFar()
{
	// verify the textureFar is correctly unlinked from any ciruclar list.
	nlassert(_ULPrec==this && _ULNext==this);
}


void CTextureFar::linkBeforeUL(CTextureFar *textNext)
{
	nlassert(textNext);

	// first, unlink others from me. NB: works even if _ULPrec==_ULNext==this.
	_ULNext->_ULPrec= _ULPrec;
	_ULPrec->_ULNext= _ULNext;
	// link to igNext.
	_ULNext= textNext;
	_ULPrec= textNext->_ULPrec;
	// link others to me.
	_ULNext->_ULPrec= this;
	_ULPrec->_ULNext= this;
}

void CTextureFar::unlinkUL()
{
	// first, unlink others from me. NB: works even if _ULPrec==_ULNext==this.
	_ULNext->_ULPrec= _ULPrec;
	_ULPrec->_ULNext= _ULNext;
	// reset
	_ULPrec= this;
	_ULNext= this;
}


void CTextureFar::setSizeOfFarPatch (sint width, sint height)
{
	// Resizing the bitmap
	_OriginalWidth=width*NL_NUM_FAR_PATCHES_BY_EDGE;
	_OriginalHeight=height*NL_NUM_FAR_PATCHES_BY_EDGE;

	// Resize patch array
	contReset (_Patches);
	_Patches.resize (NL_NUM_FAR_PATCHES_BY_TEXTURE);

	// Init count of patch
	_PatchCount=0;

	// Init upload format 16 bits
	setUploadFormat(RGB565);

	// Set filter mode. No mipmap!
	setFilterMode (Linear, LinearMipMapOff);

	// Wrap
	setWrapS (Clamp);
	setWrapT (Clamp);

	// Init patch array
	for (sint p=0; p<NL_NUM_FAR_PATCHES_BY_TEXTURE; p++)
	{
		// Set patch pointer to NULL
		_Patches[p].Patch=NULL;
	}
}

// Add a patch in the CTexture Patch. Must not be full! Return true if the texture is full after adding this patch else false.
bool CTextureFar::addPatch (CPatch *pPatch, float& farUScale, float& farVScale, float& farUBias, float& farVBias, bool& bRot)
{
	// Check that at least a cell is free
	nlassert (_PatchCount<NL_NUM_FAR_PATCHES_BY_TEXTURE);

	// Look for a free cell
	sint p;
	for (p=0; p<NL_NUM_FAR_PATCHES_BY_TEXTURE; p++)
	{
		// Cell is NULL ?
		if (_Patches[p].Patch==NULL)
		{
			// Put the patch here and go out.
			_Patches[p].Patch=pPatch;
			break;
		}
	}
	// Check that at least a cell is free
	nlassert (p<NL_NUM_FAR_PATCHES_BY_TEXTURE);

	// Position of the invalide rectangle
	int x = ((p & NL_NUM_FAR_PATCHES_BY_EDGE_MASK) * _OriginalWidth) >> NL_NUM_FAR_PATCHES_BY_EDGE_SHIFT;
	int y = ((p >> NL_NUM_FAR_PATCHES_BY_EDGE_SHIFT) * _OriginalHeight) >> NL_NUM_FAR_PATCHES_BY_EDGE_SHIFT;

	// Invalidate the rectangle
	CRect rect (x, y, _OriginalWidth>>NL_NUM_FAR_PATCHES_BY_EDGE_SHIFT, _OriginalHeight>>NL_NUM_FAR_PATCHES_BY_EDGE_SHIFT);
	ITexture::touchRect (rect);

	// ** Return some values

	// Rotation flag
	bRot = ( pPatch->getOrderS() < pPatch->getOrderT() );

	// Scale is the same for all
	farUScale=(float)((_OriginalWidth>>NL_NUM_FAR_PATCHES_BY_EDGE_SHIFT)-1)/(float)_OriginalWidth;
	farVScale=(float)((_OriginalHeight>>NL_NUM_FAR_PATCHES_BY_EDGE_SHIFT)-1)/(float)_OriginalHeight;

	// UBias is the same for all
	farUBias=((float)x+0.5f)/(float)_OriginalWidth;

	// UBias is the same for all
	farVBias=((float)y+0.5f)/(float)_OriginalHeight;
	
	// One more patch
	_PatchCount++;

	return (_PatchCount == NL_NUM_FAR_PATCHES_BY_TEXTURE);
}

// Remove a patch in the CTexture Patch
bool CTextureFar::removePatch (CPatch *pPatch)
{
	// Check that at least a cell is used
	nlassert (_PatchCount>0);

	// Look for the patch free cell
	sint p;
	for (p=0; p<NL_NUM_FAR_PATCHES_BY_TEXTURE; p++)
	{
		// Is the good cell ?
		if (_Patches[p].Patch==pPatch)
		{
			// ok, remove it
			_Patches[p].Patch=NULL;
			break;
		}
	}

	// Check it has been found
	nlassert (p<NL_NUM_FAR_PATCHES_BY_TEXTURE);

	// One patch less
	_PatchCount--;

	// Return true if it is empty, else return false
	return (_PatchCount == 0);
}

uint CTextureFar::touchPatch(uint p)
{
	// Check param
	nlassert (p<NL_NUM_FAR_PATCHES_BY_TEXTURE);

	// if there is still a patch here
	if( _Patches[p].Patch!=NULL )
	{
		// Position of the invalide rectangle
		int x = ((p & NL_NUM_FAR_PATCHES_BY_EDGE_MASK) * _OriginalWidth) >> NL_NUM_FAR_PATCHES_BY_EDGE_SHIFT;
		int y = ((p >> NL_NUM_FAR_PATCHES_BY_EDGE_SHIFT) * _OriginalHeight) >> NL_NUM_FAR_PATCHES_BY_EDGE_SHIFT;

		// Invalidate the associated rectangle
		CRect rect (x, y, _OriginalWidth>>NL_NUM_FAR_PATCHES_BY_EDGE_SHIFT, _OriginalHeight>>NL_NUM_FAR_PATCHES_BY_EDGE_SHIFT);
		ITexture::touchRect (rect);

		// return number of pixels touched
		return (_OriginalWidth>>NL_NUM_FAR_PATCHES_BY_EDGE_SHIFT) * (_OriginalHeight>>NL_NUM_FAR_PATCHES_BY_EDGE_SHIFT);
	}
	else
	{
		// no touch
		return 0;
	}
}

// Generate the texture. See ITexture::doGenerate().
void CTextureFar::doGenerate ()
{
	// Resize
	CBitmap::resize (_OriginalWidth, _OriginalHeight, RGBA);

	// Rectangle invalidate ?
	if (_ListInvalidRect.begin()!=_ListInvalidRect.end())
	{
		// Yes, rebuild only those rectangles.

		// For each rectangle to compute
		std::list<NLMISC::CRect>::iterator ite=_ListInvalidRect.begin();
		while (ite!=_ListInvalidRect.end())
		{
			// Compute rectangle coordinates
			sint x=(ite->left()<<NL_NUM_FAR_PATCHES_BY_EDGE_SHIFT)/_Width;
			sint y=(ite->top()<<NL_NUM_FAR_PATCHES_BY_EDGE_SHIFT)/_Height;

			// X and Y coord should be >0 and not be greater or equal to the number of patch stored on a texture edge.
			nlassert (x>=0);
			nlassert (x<NL_NUM_FAR_PATCHES_BY_EDGE);
			nlassert (y>=0);
			nlassert (y<NL_NUM_FAR_PATCHES_BY_EDGE);

			// ReBuild the rectangle. verify first patch still exist.
			if (_Patches[x+(y<<NL_NUM_FAR_PATCHES_BY_EDGE_SHIFT)].Patch)
				rebuildRectangle (x, y);

			// Next rectangle
			ite++;
		}
	}
	else
	{
		// no, rebuild all the rectangle
		for (sint y=0; y<NL_NUM_FAR_PATCHES_BY_EDGE; y++)
		for (sint x=0; x<NL_NUM_FAR_PATCHES_BY_EDGE; x++)
		{
			// Rebuild this rectangle
			if (_Patches[x+(y<<NL_NUM_FAR_PATCHES_BY_EDGE_SHIFT)].Patch)
				rebuildRectangle (x, y);
		}
	}
}


// Rebuild the rectangle passed with coordinate passed in parameter
void CTextureFar::rebuildRectangle (uint x, uint y)
{
	// Patch pointer
	CPatch* patch=_Patches[x+(y<<NL_NUM_FAR_PATCHES_BY_EDGE_SHIFT)].Patch;

	// Check it exists
	nlassert (patch);

	// get the order
	uint nS=patch->getOrderS();
	uint nT=patch->getOrderT();

	// Check it is a 16 bits texture
	nlassert (getPixelFormat()==RGBA);

	// Check pixels exist
	nlassert (getPixels().size()!=0);

	// Base offset of the first pixel of the patch's texture
	uint	nBaseOffset;

	// Delta to add to the destination offset when walk for a pixel to the right in the source tile
	sint dstDeltaX;

	// Delta to add to the destination offset when walk for a pixel to the bottom in the source tile
	sint dstDeltaY;

	// larger size
	uint larger;

	// larger than higher  (regular)
	if (nS>=nT)
	{
		// Regular offset, top left
		nBaseOffset=((x*_Width)>>NL_NUM_FAR_PATCHES_BY_EDGE_SHIFT)+((y*_Height)>>NL_NUM_FAR_PATCHES_BY_EDGE_SHIFT)*_Width;

		// Regular deltaX, to the right
		dstDeltaX=1;

		// Regular deltaY, to the bottom
		dstDeltaY=_Width;

		// Larger size
		larger=nS;
	}
	// higher than larger (goofy), the patch is stored with a rotation of 1 (to the left of course)
	else
	{
		// Goofy offset, bottom left
		nBaseOffset=((x*_Width)>>NL_NUM_FAR_PATCHES_BY_EDGE_SHIFT)+((y*_Height)>>NL_NUM_FAR_PATCHES_BY_EDGE_SHIFT)*_Width;
		nBaseOffset+=((_Height>>NL_NUM_FAR_PATCHES_BY_EDGE_SHIFT)-1)*_Width;

		// Goofy deltaX, to the top
		dstDeltaX=-(sint)_Width;

		// Goofy deltaY, to the right
		dstDeltaY=1;

		// Larger size
		larger=nT;
	}
		
	// Compute the order of the patch
	CTileFarBank::TFarOrder orderX=CTileFarBank::order0;
	uint tileSize=0;
	switch ((larger*NL_NUM_FAR_PATCHES_BY_EDGE*NL_NUM_PIXELS_ON_FAR_TILE_EDGE)/_Width)
	{
	case 4:
		// Ratio 1:4
		orderX=CTileFarBank::order2;
		tileSize=NL_NUM_PIXELS_ON_FAR_TILE_EDGE>>2;
		break;
	case 2:
		// Ratio 1:2
		orderX=CTileFarBank::order1;
		tileSize=NL_NUM_PIXELS_ON_FAR_TILE_EDGE>>1;
		break;
	case 1:
		// Ratio 1:1
		orderX=CTileFarBank::order0;
		tileSize=NL_NUM_PIXELS_ON_FAR_TILE_EDGE;
		break;
	default:
		// no!: must be one of the previous values
		nlassert (0);
	}

#ifdef NL_DEBUG
	// Compute the Y order
	CTileFarBank::TFarOrder orderY;
	switch ((std::min(nS, nT)*NL_NUM_FAR_PATCHES_BY_EDGE*NL_NUM_PIXELS_ON_FAR_TILE_EDGE)/_Height)
	{
	case 4:
		// Ratio 1:4
		orderY=CTileFarBank::order2;
		break;
	case 2:
		// Ratio 1:2
		orderY=CTileFarBank::order1;
		break;
	case 1:
		// Ratio 1:1
		orderY=CTileFarBank::order0;
		break;
	default:
		// no!: must be one of the previous values
		nlassert (0);
	}

	// Check the ratio on Y is the same than on X
	nlassert (orderX == orderY);
#endif // NL_DEBUG
	// Must have a far tile bank pointer set in the CFarTexture
	nlassert (_Bank);

	// For all the tiles in the textures
	sint nTileInPatch=0;

	// ** Fill the struct for the tile fill method for each layers
	NL3D_CComputeTileFar TileFar;
	TileFar.AsmMMX= false;
#ifdef NL_OS_WINDOWS
	TileFar.AsmMMX= NLMISC::CSystemInfo::hasMMX();
#endif

	// Destination pointer

	// Destination delta
	TileFar.DstDeltaX=dstDeltaX;
	TileFar.DstDeltaY=dstDeltaY;

	// ** Build expand lightmap..
	NL3D_CExpandLightmap lightMap;
	
	// Fill the structure
	lightMap.MulFactor=tileSize;
	lightMap.ColorTile=&patch->TileColors[0];
	lightMap.Width=nS+1;
	lightMap.Height=nT+1;
	lightMap.StaticLightColor=patch->getZone()->getLandscape()->getStaticLight();
	lightMap.DstPixels=_LightmapExpanded;
	// Compute current TLI colors.
	patch->computeCurrentTLILightmap(_TileTLIColors);
	lightMap.TLIColor= _TileTLIColors;

	// Expand the shadowmap
	patch->unpackShadowMap (_LumelExpanded);
	lightMap.LumelTile=_LumelExpanded;

	// Expand the patch lightmap now
	NL3D_expandLightmap (&lightMap);

	// DeltaY for lightmap
	TileFar.SrcLightingDeltaY=nS*tileSize;

	// Base Dst pointer on the tile line
	uint nBaseDstTileLine=nBaseOffset;
	for (uint t=0; t<nT; t++)
	{
		// Base Dst pointer on the tile
		uint nBaseDstTilePixels=nBaseDstTileLine;

		// For each tile of the line
		for (uint s=0; s<nS; s++)
		{
			// Base pointer of the destination texture
			TileFar.DstPixels=(CRGBA*)&(getPixels()[0])+nBaseDstTilePixels;

			// Lightmap pointer
			TileFar.SrcLightingPixels=_LightmapExpanded+(s*tileSize)+(t*nS*tileSize*tileSize);

			// For each layer of the tile
			for (sint l=0; l<3; l++)
			{
				// Use of additive in this layer ?
				bool bAdditive=false;

				// Size of the edge far tile
				TileFar.Size=tileSize;

				// Get a tile element reference for this tile.
				const CTileElement &tileElm=patch->Tiles[nTileInPatch];

				// Check for 256 tiles...
				bool	is256x256;
				uint8	uvOff;
				tileElm.getTile256Info(is256x256, uvOff);

				// Get the tile number
				sint tile=tileElm.Tile[l];

				// Is the last layer ?
				bool lastLayer = ( (l == 2) || (tileElm.Tile[l+1] == NL_TILE_ELM_LAYER_EMPTY) );

				// Is an non-empty layer ?
				if (tile!=NL_TILE_ELM_LAYER_EMPTY)
				{
					// Get the read only pointer on the far tile
					const CTileFarBank::CTileFar*	pTile=_Bank->getTile (tile);

					// This pointer must not be null, else the farBank is not valid!
					if (pTile==NULL)
						nlwarning ("FarBank is not valid!");

					// If the tile exist
					if (pTile)
					{
						// Tile exist ?
						if (pTile->isFill (CTileFarBank::diffuse))
						{
							// Get rotation of the tile in this layer
							sint nRot=tileElm.getTileOrient(l);

							// Source pointer
							const CRGBA*	pSrcDiffusePixels=pTile->getPixels (CTileFarBank::diffuse, orderX);
							const CRGBA*	pSrcAdditivePixels=NULL;

							// Additive ?
							if (pTile->isFill (CTileFarBank::additive))
							{
								// Use it
								bAdditive=true;

								// Get additive pointer
								pSrcAdditivePixels=pTile->getPixels (CTileFarBank::additive, orderX);
							}

							// Source size
							sint sourceSize;

							// Source offset (for 256)
							uint sourceOffset=0;

							// 256 ?
							if (is256x256)
							{
								// On the left ?
								if (uvOff&0x02)
									sourceOffset+=tileSize;

								// On the bottom ?
								if ((uvOff==1)||(uvOff==2))
									sourceOffset+=2*tileSize*tileSize;

								// Yes, 256
								sourceSize=tileSize<<1;
							}
							else
							{
								// No, 128
								sourceSize=tileSize;
							}

							// Compute offset and deltas
							switch (nRot)
							{
							case 0:
								// Source pointers
								TileFar.SrcDiffusePixels=pSrcDiffusePixels+sourceOffset;
								TileFar.SrcAdditivePixels=pSrcAdditivePixels+sourceOffset;

								// Source delta
								TileFar.SrcDeltaX=1;
								TileFar.SrcDeltaY=sourceSize;
								break;
							case 1:
								{
									// Source pointers
									uint newOffset=sourceOffset+(tileSize-1);
									TileFar.SrcDiffusePixels=pSrcDiffusePixels+newOffset;
									TileFar.SrcAdditivePixels=pSrcAdditivePixels+newOffset;

									// Source delta
									TileFar.SrcDeltaX=sourceSize;
									TileFar.SrcDeltaY=-1;
								}
								break;
							case 2:
								{
									// Destination pointer
									uint newOffset=sourceOffset+(tileSize-1)*sourceSize+tileSize-1;
									TileFar.SrcDiffusePixels=pSrcDiffusePixels+newOffset;
									TileFar.SrcAdditivePixels=pSrcAdditivePixels+newOffset;

									// Source delta
									TileFar.SrcDeltaX=-1;
									TileFar.SrcDeltaY=-sourceSize;
								}
								break;
							case 3:
								{
									// Destination pointer
									uint newOffset=sourceOffset+(tileSize-1)*sourceSize;
									TileFar.SrcDiffusePixels=pSrcDiffusePixels+newOffset;
									TileFar.SrcAdditivePixels=pSrcAdditivePixels+newOffset;

									// Source delta
									TileFar.SrcDeltaX=-sourceSize;
									TileFar.SrcDeltaY=1;
								}
								break;
							}

							// *** Draw the layer

							// Alpha layer ?
							if (l>0)
							{
								// Additive layer ?
								if (bAdditive && lastLayer)
									NL3D_drawFarTileInFarTextureAdditiveAlpha (&TileFar);
								else	// No additive layer
									NL3D_drawFarTileInFarTextureAlpha (&TileFar);
							}
							else	// no alpha
							{
								// Additive layer ?
								if (bAdditive && lastLayer)
									NL3D_drawFarTileInFarTextureAdditive (&TileFar);
								else	// No additive layer
									NL3D_drawFarTileInFarTexture (&TileFar);
							}
						}
					}
				}
				else
					// Stop, no more layer
					break;
			}
						
			// Next tile
			nTileInPatch++;
			
			// Next tile on the line
			nBaseDstTilePixels+=dstDeltaX*tileSize;
		}

		// Next line of tiles
		nBaseDstTileLine+=dstDeltaY*tileSize;
	}

}

} // NL3D


// ***************************************************************************
// ***************************************************************************
// NL3D_ExpandLightmap. C and Asm Part
// ***************************************************************************
// ***************************************************************************

#ifdef NL_OS_WINDOWS


// EMMS called not in __asm block.
#  pragma warning (disable : 4799)


// ***************************************************************************
inline	void	NL3D_asmEndMMX()
{
	__asm
	{
		// close MMX computation
		emms
	}
}


// ***************************************************************************
/** Expand a line of color with MMX.
 *	NB: start to write at pixel 1.
 */
inline	void	NL3D_asmExpandLineColor565(const uint16 *src, CRGBA *dst, uint du, uint len)
{
	static	uint64 blank = 0;
	static	uint64 cF800 = 0x0000F8000000F800;
	static	uint64 cE000 = 0x0000E0000000E000;
	static	uint64 c07E0 = 0x000007E0000007E0;
	static	uint64 c0600 = 0x0000060000000600;
	static	uint64 c001F = 0x0000001F0000001F;
	static	uint64 c001C = 0x0000001C0000001C;
	if(len==0)
		return;


	// Loop for pix.
	__asm
	{
		movq	mm7, blank

		// start at pixel 1 => increment dst, and start u= du
		mov		esi, src
		mov		edi, dst
		add		edi, 4
		mov		ecx, len
		mov		edx, du

		// Loop
	myLoop:


		// Read 565 colors
		//----------
		// index u.
		mov		ebx, edx
		shr		ebx, 8

		// pack the 2 colors in eax: // Hedx= color0, Ledx= color1
		xor		eax, eax			// avoid partial stall.
		mov		ax, [esi + ebx*2]
		shl		eax, 16
		mov		ax, [esi + ebx*2 +2]
		
		// store and unpack in mm2: Hmm2= color0, Lmm2= color1
		movd	mm2, eax
		punpcklwd	mm2, mm7

		// reset accumulator mm3 to black
		movq	mm3, mm7

		// Expand 565 to 888: color0 and color1 in parrallel
		// R
		movq	mm0, mm2
		movq	mm1, mm2
		pand	mm0, cF800
		pand	mm1, cE000
		psrld	mm0, 8
		psrld	mm1, 13
		por		mm3, mm0
		por		mm3, mm1
		// G
		movq	mm0, mm2
		movq	mm1, mm2
		pand	mm0, c07E0
		pand	mm1, c0600
		pslld	mm0, 5
		psrld	mm1, 1
		por		mm3, mm0
		por		mm3, mm1
		// B
		movq	mm0, mm2
		movq	mm1, mm2
		pand	mm0, c001F
		pand	mm1, c001C
		pslld	mm0, 19
		pslld	mm1, 14
		por		mm3, mm0
		por		mm3, mm1

		// unpack mm3 quad to mm0=color0 and mm1=color1.
		movq	mm0, mm3
		movq	mm1, mm3
		psrlq	mm0, 32


		// Blend.
		//----------
		// blend factors
		mov		ebx, edx
		mov		eax, 256

		and		ebx, 0xFF
		sub		eax, ebx

		movd	mm2, ebx		// mm2= factor
		movd	mm3, eax		// mm3= 1-factor
		// replicate to the	4 words.
		punpckldq	mm2, mm2	// mm2= 0000 00AA 0000 00AA
		punpckldq	mm3, mm3	// mm3= 0000 00AA 0000 00AA
		packssdw	mm2, mm2	// mm2= 00AA 00AA 00AA 00AA
		packssdw	mm3, mm3	// mm3= 00AA 00AA 00AA 00AA

		// mul
		punpcklbw	mm0, mm7
		punpcklbw	mm1, mm7
		pmullw		mm0, mm3	// color0*(1-factor)
		pmullw		mm1, mm2	// color1*factor
		// add, and unpack
		paddusw		mm0, mm1
		psrlw       mm0, 8
		packuswb    mm0, mm0

		// store
		movd        [edi], mm0


		// next pix
		add	edx, du
		add	edi, 4
		dec ecx
		jnz myLoop
	}
}


// ***************************************************************************
/** Expand a line of color with MMX.
 *	NB: start to write at pixel 1.
 */
inline	void	NL3D_asmExpandLineColor8888(const CRGBA *src, CRGBA *dst, uint du, uint len)
{
	static	uint64 blank = 0;
	if(len==0)
		return;


	// Loop for pix.
	__asm
	{
		movq	mm7, blank

		// start at pixel 1 => increment dst, and start u= du
		mov		esi, src
		mov		edi, dst
		add		edi, 4
		mov		ecx, len
		mov		edx, du

		// Loop
	myLoop:


		// Read 8888 colors
		//----------
		// index u.
		mov		ebx, edx
		shr		ebx, 8

		// read the 2 colors: mm0= color0, mm1= color1
		movd	mm0 , [esi + ebx*4]
		movd	mm1 , [esi + ebx*4 + 4]


		// Blend.
		//----------
		// blend factors
		mov		ebx, edx
		mov		eax, 256

		and		ebx, 0xFF
		sub		eax, ebx

		movd	mm2, ebx		// mm2= factor
		movd	mm3, eax		// mm3= 1-factor
		// replicate to the	4 words.
		punpckldq	mm2, mm2	// mm2= 0000 00AA 0000 00AA
		punpckldq	mm3, mm3	// mm3= 0000 00AA 0000 00AA
		packssdw	mm2, mm2	// mm2= 00AA 00AA 00AA 00AA
		packssdw	mm3, mm3	// mm3= 00AA 00AA 00AA 00AA

		// mul
		punpcklbw	mm0, mm7
		punpcklbw	mm1, mm7
		pmullw		mm0, mm3	// color0*(1-factor)
		pmullw		mm1, mm2	// color1*factor
		// add, and unpack
		paddusw		mm0, mm1
		psrlw       mm0, 8
		packuswb    mm0, mm0

		// store
		movd        [edi], mm0


		// next pix
		add	edx, du
		add	edi, 4
		dec ecx
		jnz myLoop
	}
}


// ***************************************************************************
/** Blend 2 lines of color into one line.
 *	NB: start at pix 0 here
 */
inline	void	NL3D_asmBlendLines(CRGBA *dst, const CRGBA *src0, const CRGBA *src1, uint index, uint len)
{
	static	uint64 blank = 0;
	if(len==0)
		return;


	// Loop for pix.
	__asm
	{
		movq	mm7, blank

		// read the factor and expand it to 4 words.
		mov		ebx, index
		mov		eax, 256
		and		ebx, 0xFF
		sub		eax, ebx
		movd	mm2, ebx		// mm2= factor
		movd	mm3, eax		// mm3= 1-factor
		punpckldq	mm2, mm2	// mm2= 0000 00AA 0000 00AA
		punpckldq	mm3, mm3	// mm3= 0000 00AA 0000 00AA
		packssdw	mm2, mm2	// mm2= 00AA 00AA 00AA 00AA
		packssdw	mm3, mm3	// mm3= 00AA 00AA 00AA 00AA

		// setup ptrs
		mov		esi, src0
		mov		edx, src1
		sub		edx, esi	// difference between 2 src
		mov		edi, dst
		mov		ecx, len

		// Loop
	myLoop:

		// Read
		movd	mm0, [esi]
		movd	mm1, [esi+edx]

		// mul
		punpcklbw	mm0, mm7
		punpcklbw	mm1, mm7
		pmullw		mm0, mm3	// color0*(1-factor)
		pmullw		mm1, mm2	// color1*factor
		// add, and unpack
		paddusw		mm0, mm1
		psrlw       mm0, 8
		packuswb    mm0, mm0

		// store
		movd        [edi], mm0


		// next pix
		add	esi, 4
		add	edi, 4
		dec ecx
		jnz myLoop
	}
}


// ***************************************************************************
/**	Lightmap Combining for Far level 2 (farthest)
 *	Average 16 lumels, and deals with UserColor and TLI
 */
static void		NL3D_asmAssembleShading1x1(const uint8 *lumels, const CRGBA *colorMap, 
	const CRGBA *srcTLIs, const CRGBA *srcUSCs, CRGBA *dst, uint lineWidth, uint nbTexel)
{
	static	uint64 blank = 0;
	if(nbTexel==0)
		return;

	// local var
	uint	offsetTLIs= ((uint)srcTLIs-(uint)dst);
	uint	offsetUSCs= ((uint)srcUSCs-(uint)dst);

	// Loop for pix.
	__asm
	{
		movq		mm7, blank

		// setup ptrs
		mov			esi, lumels
		mov			edi, dst
		mov			ecx, nbTexel

		// Loop
	myLoop:

		// Average shade part
		//------------
		mov			ebx, colorMap
		mov			edx, lineWidth

		// read and accumulate shade 
		xor			eax,eax			// avoid partial stall
		// add with line 0
		mov			al, [esi + 0]
		add			al, [esi + 1]
		adc			ah, 0
		add			al, [esi + 2]
		adc			ah, 0
		add			al, [esi + 3]
		adc			ah, 0
		// add with line 1
		add			al, [esi + edx + 0]
		adc			ah, 0
		add			al, [esi + edx + 1]
		adc			ah, 0
		add			al, [esi + edx + 2]
		adc			ah, 0
		add			al, [esi + edx + 3]
		adc			ah, 0
		// add with line 2
		add			al, [esi + edx*2 + 0]
		adc			ah, 0
		add			al, [esi + edx*2 + 1]
		adc			ah, 0
		add			al, [esi + edx*2 + 2]
		adc			ah, 0
		add			al, [esi + edx*2 + 3]
		adc			ah, 0
		// add with line 3
		lea			edx, [edx + edx*2]
		add			al, [esi + edx + 0]
		adc			ah, 0
		add			al, [esi + edx + 1]
		adc			ah, 0
		add			al, [esi + edx + 2]
		adc			ah, 0
		add			al, [esi + edx + 3]
		adc			ah, 0
		// average
		shr			eax, 4

		// convert to RGBA from the color Map
		movd		mm0, [ebx + eax*4]

		// Assemble part
		//------------
		mov			edx, offsetTLIs
		mov			ebx, offsetUSCs

		// Add with TLI, and clamp.
		paddusb		mm0, [edi + edx]

		// mul with USC
		movd		mm1, [edi + ebx]
		punpcklbw	mm0, mm7
		punpcklbw	mm1, mm7
		pmullw		mm0, mm1
		// unpack
		psrlw       mm0, 8
		packuswb    mm0, mm0

		// store
		movd        [edi], mm0


		// next pix
		add			esi, 4		// skip 4 lumels
		add			edi, 4		// next texel
		dec			ecx
		jnz			myLoop
	}
}


// ***************************************************************************
/**	Lightmap Combining for Far level 1 (middle)
 *	Average 4 lumels, and deals with UserColor and TLI
 */
static void		NL3D_asmAssembleShading2x2(const uint8 *lumels, const CRGBA *colorMap, 
	const CRGBA *srcTLIs, const CRGBA *srcUSCs, CRGBA *dst, uint lineWidth, uint nbTexel)
{
	static	uint64 blank = 0;
	if(nbTexel==0)
		return;

	// local var
	uint	offsetTLIs= ((uint)srcTLIs-(uint)dst);
	uint	offsetUSCs= ((uint)srcUSCs-(uint)dst);

	// Loop for pix.
	__asm
	{
		movq		mm7, blank

		// setup ptrs
		mov			esi, lumels
		mov			edi, dst
		mov			ecx, nbTexel

		// Loop
	myLoop:

		// Average shade part
		//------------
		mov			ebx, colorMap
		mov			edx, lineWidth

		// read and accumulate shade 
		xor			eax,eax			// avoid partial stall
		mov			al, [esi]		// read lumel
		// add with nbors
		add			al, [esi + 1]
		adc			ah, 0
		add			al, [esi + edx]
		adc			ah, 0
		add			al, [esi + edx + 1]
		adc			ah, 0
		// average
		shr			eax, 2

		// convert to RGBA from the color Map
		movd		mm0, [ebx + eax*4]

		// Assemble part
		//------------
		mov			edx, offsetTLIs
		mov			ebx, offsetUSCs

		// Add with TLI, and clamp.
		paddusb		mm0, [edi + edx]

		// mul with USC
		movd		mm1, [edi + ebx]
		punpcklbw	mm0, mm7
		punpcklbw	mm1, mm7
		pmullw		mm0, mm1
		// unpack
		psrlw       mm0, 8
		packuswb    mm0, mm0

		// store
		movd        [edi], mm0


		// next pix
		add			esi, 2		// skip 2 lumels
		add			edi, 4		// next texel
		dec			ecx
		jnz			myLoop
	}
}


// ***************************************************************************
#  pragma warning (disable : 4731)			// frame pointer register 'ebp' modified by inline assembly code
/**	Lightmap Combining for Far level 0 (nearest)
 *	read 1 lumel, and deals with UserColor and TLI
 */
static void		NL3D_asmAssembleShading4x4(const uint8 *lumels, const CRGBA *colorMap, 
	const CRGBA *srcTLIs, const CRGBA *srcUSCs, CRGBA *dst, uint nbTexel)
{
	static	uint64 blank = 0;
	if(nbTexel==0)
		return;

	// Loop for pix.
	__asm
	{
		// Use ebp as a register for faster access...
		push		ebp

		movq		mm7, blank

		// setup ptrs
		mov			esi, lumels
		mov			edi, dst
		mov			edx, srcTLIs
		sub			edx, edi	// difference src and dest
		mov			ebx, srcUSCs
		sub			ebx, edi	// difference src and dest
		mov			ecx, nbTexel

		// set ebp after reading locals...
		mov			ebp, colorMap

		// Loop
	myLoop:

		// read shade RGBA into the color Map
		xor			eax,eax			// avoid partial stall
		mov			al,[esi]		// read lumel
		movd		mm0, [ebp + eax*4]

		// Add with TLI, and clamp.
		paddusb		mm0, [edi + edx]

		// mul with USC
		movd		mm1, [edi + ebx]
		punpcklbw	mm0, mm7
		punpcklbw	mm1, mm7
		pmullw		mm0, mm1
		// unpack
		psrlw       mm0, 8
		packuswb    mm0, mm0

		// store
		movd        [edi], mm0


		// next pix
		add			esi, 1		// next lumel
		add			edi, 4		// next texel
		dec			ecx
		jnz			myLoop

		// restore
		pop			ebp
	}

}

#  pragma warning (default : 4731)			// frame pointer register 'ebp' modified by inline assembly code


#else // NL_OS_WINDOWS

// Dummy for non-windows platforms
inline	void	NL3D_asmEndMMX() {}
inline	void	NL3D_asmExpandLineColor565(const uint16 *src, CRGBA *dst, uint du, uint len) {}
inline	void	NL3D_asmExpandLineColor8888(const CRGBA *src, CRGBA *dst, uint du, uint len) {}
inline	void	NL3D_asmBlendLines(CRGBA *dst, const CRGBA *src0, const CRGBA *src1, uint index, uint len) {}
static void		NL3D_asmAssembleShading1x1(const uint8 *lumels, const CRGBA *colorMap, 
	const CRGBA *srcTLIs, const CRGBA *srcUSCs, CRGBA *dst, uint lineWidth, uint nbTexel)
{
}
static void		NL3D_asmAssembleShading2x2(const uint8 *lumels, const CRGBA *colorMap, 
	const CRGBA *srcTLIs, const CRGBA *srcUSCs, CRGBA *dst, uint lineWidth, uint nbTexel)
{
}
static void		NL3D_asmAssembleShading4x4(const uint8 *lumels, const CRGBA *colorMap, 
	const CRGBA *srcTLIs, const CRGBA *srcUSCs, CRGBA *dst, uint nbTexel)
{
}

#endif // NL_OS_WINDOWS


// ***************************************************************************
extern "C" void NL3D_expandLightmap (const NL3D_CExpandLightmap* pLightmap)
{
	bool	asmMMX= false;
#ifdef	NL_OS_WINDOWS
	asmMMX= CSystemInfo::hasMMX();
	// A CTileColor must be a 565 only.
	nlassert(sizeof(CTileColor)==2);
#endif

	// Expanded width
	uint dstWidth=(pLightmap->Width-1)*pLightmap->MulFactor;
	uint dstHeight=(pLightmap->Height-1)*pLightmap->MulFactor;

	// *** First expand user color and TLI colors
	// First pass, expand on U
	static CRGBA expandedUserColorLine[ (NL_MAX_TILES_BY_PATCH_EDGE+1)*
		(NL_MAX_TILES_BY_PATCH_EDGE+1)*NL_LUMEL_BY_TILE ];
	static CRGBA expandedTLIColorLine[ (NL_MAX_TILES_BY_PATCH_EDGE+1)*
		(NL_MAX_TILES_BY_PATCH_EDGE+1)*NL_LUMEL_BY_TILE ];
	// Second pass, expand on V.
	static CRGBA expandedUserColor[ (NL_MAX_TILES_BY_PATCH_EDGE+1)*NL_LUMEL_BY_TILE * 
		(NL_MAX_TILES_BY_PATCH_EDGE+1)*NL_LUMEL_BY_TILE ];
	static CRGBA expandedTLIColor[ (NL_MAX_TILES_BY_PATCH_EDGE+1)*NL_LUMEL_BY_TILE * 
		(NL_MAX_TILES_BY_PATCH_EDGE+1)*NL_LUMEL_BY_TILE ];


	// ** Expand on U
	//=========
	uint u, v;

	// Expansion factor
	uint expandFactor=((pLightmap->Width-1)<<8)/(dstWidth-1);

	// Destination  pointer
	CRGBA *expandedUserColorLinePtr= expandedUserColorLine;
	CRGBA *expandedTLIColorLinePtr= expandedTLIColorLine;

	// Source pointer
	const NL3D::CTileColor	*colorTilePtr=pLightmap->ColorTile;
	const NLMISC::CRGBA		*colorTLIPtr= pLightmap->TLIColor;

	// Go for U
	for (v=0; v<pLightmap->Height; v++)
	{
		// First pixel
		expandedUserColorLinePtr[0].set565 (colorTilePtr[0].Color565);
		expandedTLIColorLinePtr[0]= colorTLIPtr[0];

		// MMX implementation.
		//-------------
		if(asmMMX)
		{
			NL3D_asmExpandLineColor565(&colorTilePtr->Color565, expandedUserColorLinePtr, expandFactor, dstWidth-2);
			NL3D_asmExpandLineColor8888(colorTLIPtr, expandedTLIColorLinePtr, expandFactor, dstWidth-2);
		}
		// C implementation
		//-------------
		else
		{
			// Index next pixel
			uint srcIndexPixel=expandFactor;

			for (u=1; u<dstWidth-1; u++)
			{
				// Check
				nlassert ( (u+v*dstWidth) < (sizeof(expandedUserColorLine)/sizeof(CRGBA)) );

				// Color index
				uint srcIndex=srcIndexPixel>>8;
				nlassert (srcIndex>=0);
				nlassert (srcIndex<pLightmap->Width-1);

				// Compute current color
				CRGBA color0;
				CRGBA color1;
				color0.set565 (colorTilePtr[srcIndex].Color565);
				color1.set565 (colorTilePtr[srcIndex+1].Color565);
				expandedUserColorLinePtr[u].blendFromui (color0, color1, srcIndexPixel&0xff);
				// Compute current TLI color
				color0= colorTLIPtr[srcIndex];
				color1= colorTLIPtr[srcIndex+1];
				expandedTLIColorLinePtr[u].blendFromui (color0, color1, srcIndexPixel&0xff);

				// Next index
				srcIndexPixel+=expandFactor;
			}
		}

		// Last pixel
		expandedUserColorLinePtr[dstWidth-1].set565 (colorTilePtr[pLightmap->Width-1].Color565);
		expandedTLIColorLinePtr[dstWidth-1]= colorTLIPtr[pLightmap->Width-1];

		// Next line
		expandedUserColorLinePtr+= dstWidth;
		expandedTLIColorLinePtr+= dstWidth;
		colorTilePtr+=pLightmap->Width;
		colorTLIPtr+=pLightmap->Width;
	}

	// stop MMX if used
	if(asmMMX)
		NL3D_asmEndMMX();

	// ** Expand on V
	//=========

	// Expansion factor
	expandFactor=((pLightmap->Height-1)<<8)/(dstHeight-1);

	// Destination  pointer
	CRGBA *expandedUserColorPtr= expandedUserColor;
	CRGBA *expandedTLIColorPtr= expandedTLIColor;

	// Src pointer
	expandedUserColorLinePtr= expandedUserColorLine;
	expandedTLIColorLinePtr= expandedTLIColorLine;

	// Copy first row
	memcpy(expandedUserColorPtr, expandedUserColorLinePtr, dstWidth*sizeof(CRGBA));
	memcpy(expandedTLIColorPtr, expandedTLIColorLinePtr, dstWidth*sizeof(CRGBA));

	// Next line
	expandedUserColorPtr+=dstWidth;
	expandedTLIColorPtr+=dstWidth;

	// Index next pixel
	uint indexPixel=expandFactor;

	// Go for V
	for (v=1; v<dstHeight-1; v++)
	{
		// Color index
		uint index=indexPixel>>8;

		// Source pointer
		CRGBA *colorTilePtr0= expandedUserColorLine + index*dstWidth;
		CRGBA *colorTilePtr1= expandedUserColorLine + (index+1)*dstWidth;
		CRGBA *colorTLIPtr0= expandedTLIColorLine + index*dstWidth;
		CRGBA *colorTLIPtr1= expandedTLIColorLine + (index+1)*dstWidth;

		// MMX implementation.
		//-------------
		if(asmMMX)
		{
			NL3D_asmBlendLines(expandedUserColorPtr, colorTilePtr0, colorTilePtr1, indexPixel, dstWidth);
			NL3D_asmBlendLines(expandedTLIColorPtr, colorTLIPtr0, colorTLIPtr1, indexPixel, dstWidth);
		}
		// C implementation
		//-------------
		else
		{
			// Copy the row
			for (u=0; u<dstWidth; u++)
			{
				expandedUserColorPtr[u].blendFromui (colorTilePtr0[u], colorTilePtr1[u], indexPixel&0xff);
				expandedTLIColorPtr[u].blendFromui (colorTLIPtr0[u], colorTLIPtr1[u],  indexPixel&0xff);
			}
		}

		// Next index
		indexPixel+=expandFactor;

		// Next line
		expandedUserColorPtr+=dstWidth;
		expandedTLIColorPtr+=dstWidth;
	}

	// stop MMX if used
	if(asmMMX)
		NL3D_asmEndMMX();

	// Last row
	// Destination  pointer
	expandedUserColorPtr= expandedUserColor + dstWidth*(dstHeight-1);
	expandedTLIColorPtr= expandedTLIColor + dstWidth*(dstHeight-1);
	// Src pointer
	expandedUserColorLinePtr= expandedUserColorLine + dstWidth*(pLightmap->Height-1);
	expandedTLIColorLinePtr= expandedTLIColorLine + dstWidth*(pLightmap->Height-1);

	// Copy last row
	memcpy(expandedUserColorPtr, expandedUserColorLinePtr, dstWidth*sizeof(CRGBA));
	memcpy(expandedTLIColorPtr, expandedTLIColorLinePtr, dstWidth*sizeof(CRGBA));

	// *** Now combine with shading
	//=========

	// Switch to the optimal method for each expansion value
	switch (pLightmap->MulFactor)
	{
	case 1:
		{
			// Make 4x4 -> 1x1 blend
			CRGBA *lineUSCPtr= expandedUserColor;
			CRGBA *lineTLIPtr= expandedTLIColor;
			CRGBA *lineDestPtr=pLightmap->DstPixels;
			const uint8 *lineLumelPtr=pLightmap->LumelTile;
			uint lineWidth=dstWidth<<2;
			uint lineWidthx2=lineWidth<<1;
			uint lineWidthx3=lineWidthx2+lineWidth;
			uint lineWidthx4=lineWidth<<2;

			// For each line
			for (v=0; v<dstHeight; v++)
			{
				// MMX implementation.
				//-------------
				if(asmMMX)
				{
					NL3D_asmAssembleShading1x1(lineLumelPtr, pLightmap->StaticLightColor, lineTLIPtr, lineUSCPtr, lineDestPtr,
						lineWidth, dstWidth);
				}
				// C implementation
				//-------------
				else
				{
					// For each lumel block
					for (u=0; u<dstWidth; u++)
					{
						// index
						uint lumelIndex=u<<2;

						// Shading is filtred
						uint shading=
							 ((uint)lineLumelPtr[lumelIndex]+(uint)lineLumelPtr[lumelIndex+1]+(uint)lineLumelPtr[lumelIndex+2]+(uint)lineLumelPtr[lumelIndex+3]
							+(uint)lineLumelPtr[lumelIndex+lineWidth]+(uint)lineLumelPtr[lumelIndex+1+lineWidth]+(uint)lineLumelPtr[lumelIndex+2+lineWidth]+(uint)lineLumelPtr[lumelIndex+3+lineWidth]
							+(uint)lineLumelPtr[lumelIndex+lineWidthx2]+(uint)lineLumelPtr[lumelIndex+1+lineWidthx2]+(uint)lineLumelPtr[lumelIndex+2+lineWidthx2]+(uint)lineLumelPtr[lumelIndex+3+lineWidthx2]
							+(uint)lineLumelPtr[lumelIndex+lineWidthx3]+(uint)lineLumelPtr[lumelIndex+1+lineWidthx3]+(uint)lineLumelPtr[lumelIndex+2+lineWidthx3]+(uint)lineLumelPtr[lumelIndex+3+lineWidthx3]
							)>>4;

						// Add shading with TLI color.
						CRGBA	col;
						col.addRGBOnly(pLightmap->StaticLightColor[shading], lineTLIPtr[u]);

						// Mul by the userColor
						lineDestPtr[u].modulateFromColorRGBOnly(col, lineUSCPtr[u]);
					}
				}

				// Next line
				lineUSCPtr+=dstWidth;
				lineTLIPtr+=dstWidth;
				lineDestPtr+=dstWidth;
				lineLumelPtr+=lineWidthx4;
			}
			break;
		}
	case 2:
		{
			// Make 2x2 -> 1x1 blend
			CRGBA *lineUSCPtr= expandedUserColor;
			CRGBA *lineTLIPtr= expandedTLIColor;
			CRGBA *lineDestPtr=pLightmap->DstPixels;
			const uint8 *lineLumelPtr=pLightmap->LumelTile;
			uint lineWidth=dstWidth*2;
			uint lineWidthx2=lineWidth<<1;

			// For each line
			for (v=0; v<dstHeight; v++)
			{
				// MMX implementation.
				//-------------
				if(asmMMX)
				{
					NL3D_asmAssembleShading2x2(lineLumelPtr, pLightmap->StaticLightColor, lineTLIPtr, lineUSCPtr, lineDestPtr,
						lineWidth, dstWidth);
				}
				// C implementation
				//-------------
				else
				{
					// For each lumel block
					for (u=0; u<dstWidth; u++)
					{
						// index
						uint lumelIndex=u<<1;

						// Shading is filtred
						uint shading=
							((uint)lineLumelPtr[lumelIndex]+(uint)lineLumelPtr[lumelIndex+1]+(uint)lineLumelPtr[lumelIndex+lineWidth]+(uint)lineLumelPtr[lumelIndex+1+lineWidth])>>2;

						// Add shading with TLI color.
						CRGBA	col;
						col.addRGBOnly(pLightmap->StaticLightColor[shading], lineTLIPtr[u]);

						// Mul by the userColor
						lineDestPtr[u].modulateFromColorRGBOnly(col, lineUSCPtr[u]);
					}
				}

				// Next line
				lineUSCPtr+=dstWidth;
				lineTLIPtr+=dstWidth;
				lineDestPtr+=dstWidth;
				lineLumelPtr+=lineWidthx2;
			}
			break;
		}

	case 4:
			// Make copy
			CRGBA *lineUSCPtr= expandedUserColor;
			CRGBA *lineTLIPtr= expandedTLIColor;
			CRGBA *lineDestPtr=pLightmap->DstPixels;
			const uint8 *lineLumelPtr=pLightmap->LumelTile;
			uint nbTexel=dstWidth*dstHeight;

			// MMX implementation.
			//-------------
			if(asmMMX)
			{
				NL3D_asmAssembleShading4x4(lineLumelPtr, pLightmap->StaticLightColor, lineTLIPtr, lineUSCPtr, lineDestPtr,
					nbTexel);
			}
			// C implementation
			//-------------
			else
			{
				// For each pixel
				for (u=0; u<nbTexel; u++)
				{
					// Shading is filtred
					uint shading=lineLumelPtr[u];

					// Add shading with TLI color.
					CRGBA	col;
					col.addRGBOnly(pLightmap->StaticLightColor[shading], lineTLIPtr[u]);

					// Mul by the userColor
					lineDestPtr[u].modulateFromColorRGBOnly(col, lineUSCPtr[u]);
				}
			}
			break;
	}

	// stop MMX if used
	if(asmMMX)
		NL3D_asmEndMMX();

}


// ***************************************************************************
// ***************************************************************************
// NL3D_drawFarTileInFar*. C and Asm Part
// ***************************************************************************
// ***************************************************************************


#ifdef NL_OS_WINDOWS


// ***************************************************************************
inline	void	NL3D_asmModulateLineColors(CRGBA *dst, const CRGBA *src0, const CRGBA *src1, 
	uint len, uint	src0DeltaX, uint dstDeltaX)
{
	static	uint64	blank= 0;
	if(len==0)
		return;

	__asm
	{
		movq		mm7, blank

		mov			esi, src0	// esi point to src Pixels
		mov			edx, src1	// edx point to src lighting pixels
		mov			edi, dst
		mov			ecx, len
		// compute increments for esi and edi
		mov			eax, src0DeltaX
		mov			ebx, dstDeltaX
		sal			eax, 2
		sal			ebx, 2

	myLoop:
		// read colors
		movd		mm0, [esi]
		movd		mm1, [edx]

		// mul mm0 and mm1
		punpcklbw	mm0, mm7
		punpcklbw	mm1, mm7
		pmullw		mm0, mm1
		psrlw       mm0, 8
		// pack
		packuswb    mm0, mm0

		// out
		movd		[edi], mm0

		// increment
		add			esi, eax
		add			edi, ebx
		add			edx, 4
		dec			ecx
		jnz			myLoop
	}
}


// ***************************************************************************
inline	void	NL3D_asmModulateAndBlendLineColors(CRGBA *dst, const CRGBA *src0, const CRGBA *src1, 
	uint len, uint	src0DeltaX, uint dstDeltaX)
{
	static	uint64	blank= 0;
	static	uint64	one= 0x0100010001000100;
	if(len==0)
		return;

	__asm
	{
		movq		mm7, blank
		movq		mm6, one

		mov			esi, src0	// esi point to src Pixels
		mov			edx, src1	// edx point to src lighting pixels
		mov			edi, dst
		mov			ecx, len
		// compute increments for esi and edi
		mov			eax, src0DeltaX
		mov			ebx, dstDeltaX
		sal			eax, 2
		sal			ebx, 2

	myLoop:
		// read colors
		movd		mm0, [esi]
		movd		mm1, [edx]

		// save and unpack Alpha. NB: ABGR
		movq		mm2, mm0
		psrld		mm2, 24		// mm2= 0000 0000 0000 00AA
		punpckldq	mm2, mm2	// mm2= 0000 00AA 0000 00AA
		packssdw	mm2, mm2	// mm2= 00AA 00AA 00AA 00AA
		// negate with 256.
		movq		mm3, mm6
		psubusw		mm3, mm2

		// mul mm0 and mm1
		punpcklbw	mm0, mm7
		punpcklbw	mm1, mm7
		pmullw		mm0, mm1
		psrlw       mm0, 8

		// Alpha Blend with mm3 and mm2
		movd		mm1, [edi]	// read dest
		punpcklbw	mm1, mm7
		pmullw		mm0, mm2	// mm0= srcColor*A
		pmullw		mm1, mm3	// mm1= dstColor*(1-A)

		// add and pack
		paddusw		mm0, mm1
		psrlw       mm0, 8
		packuswb    mm0, mm0

		// out
		movd		[edi], mm0

		// increment
		add			esi, eax
		add			edi, ebx
		add			edx, 4
		dec			ecx
		jnz			myLoop
	}
}


#else // NL_OS_WINDOWS

// Dummy for non-windows platforms
inline	void	NL3D_asmModulateLineColors(CRGBA *dst, const CRGBA *src0, const CRGBA *src1, 
	uint len, uint	src0DeltaX, uint dstDeltaX)
{
}
inline	void	NL3D_asmModulateAndBlendLineColors(CRGBA *dst, const CRGBA *src0, const CRGBA *src1, 
	uint len, uint	src0DeltaX, uint dstDeltaX)
{
}

#endif

// ***************************************************************************
void NL3D_drawFarTileInFarTexture (const NL3D_CComputeTileFar* pTileFar)
{
	// Pointer of the Src diffuse pixels
	const CRGBA* pSrcPixels=pTileFar->SrcDiffusePixels;

	// Pointer of the Dst pixels
	const CRGBA* pSrcLightPixels=pTileFar->SrcLightingPixels;

	// Pointer of the Dst pixels
	CRGBA* pDstPixels=pTileFar->DstPixels;

	// For each pixels
	int x, y;
	for (y=0; y<pTileFar->Size; y++)
	{
		// MMX implementation
		//---------
		if(pTileFar->AsmMMX)
		{
			NL3D_asmModulateLineColors(pDstPixels, pSrcPixels, pSrcLightPixels, 
				pTileFar->Size, pTileFar->SrcDeltaX, pTileFar->DstDeltaX);
		}
		// C Implementation.
		//---------
		else
		{
			// Pointer of the source line
			const CRGBA* pSrcLine=pSrcPixels;

			// Pointer of the source lighting line
			const CRGBA* pSrcLightingLine=pSrcLightPixels;
			
			// Pointer of the destination line
			CRGBA* pDstLine=pDstPixels;

			// For each pixels on the line
			for (x=0; x<pTileFar->Size; x++)
			{
				// Read and write a pixel
				pDstLine->R=(uint8)(((uint)pSrcLine->R*(uint)pSrcLightingLine->R)>>8);
				pDstLine->G=(uint8)(((uint)pSrcLine->G*(uint)pSrcLightingLine->G)>>8);
				pDstLine->B=(uint8)(((uint)pSrcLine->B*(uint)pSrcLightingLine->B)>>8);

				// Next pixel
				pSrcLine+=pTileFar->SrcDeltaX;
				pSrcLightingLine++;
				pDstLine+=pTileFar->DstDeltaX;
			}
		}

		// Next line
		pSrcPixels+=pTileFar->SrcDeltaY;
		pSrcLightPixels+=pTileFar->SrcLightingDeltaY;
		pDstPixels+=pTileFar->DstDeltaY;
	}

	// stop MMX if used
	if(pTileFar->AsmMMX)
		NL3D_asmEndMMX();
}


// ***************************************************************************
void NL3D_drawFarTileInFarTextureAlpha (const NL3D_CComputeTileFar* pTileFar)
{
	// Pointer of the Src pixels
	const CRGBA* pSrcPixels=pTileFar->SrcDiffusePixels;

	// Pointer of the Dst pixels
	const CRGBA* pSrcLightPixels=pTileFar->SrcLightingPixels;

	// Pointer of the Dst pixels
	CRGBA* pDstPixels=pTileFar->DstPixels;

	// Fill the buffer with layer 0
	int x, y;
	for (y=0; y<pTileFar->Size; y++)
	{
		// MMX implementation
		//---------
		if(pTileFar->AsmMMX)
		{
			NL3D_asmModulateAndBlendLineColors(pDstPixels, pSrcPixels, pSrcLightPixels, 
				pTileFar->Size, pTileFar->SrcDeltaX, pTileFar->DstDeltaX);
		}
		// C Implementation.
		//---------
		else
		{
			// Pointer of the source line
			const CRGBA* pSrcLine=pSrcPixels;

			// Pointer of the source lighting line
			const CRGBA* pSrcLightingLine=pSrcLightPixels;

			// Pointer of the Dst pixels
			CRGBA* pDstLine=pDstPixels;

			// For each pixels on the line
			for (x=0; x<pTileFar->Size; x++)
			{
				// Read and write a pixel
				register uint alpha=pSrcLine->A;
				register uint oneLessAlpha=255-pSrcLine->A;
				pDstLine->R=(uint8)(((((uint)pSrcLine->R*(uint)pSrcLightingLine->R)>>8)*alpha+(uint)pDstLine->R*oneLessAlpha)>>8);
				pDstLine->G=(uint8)(((((uint)pSrcLine->G*(uint)pSrcLightingLine->G)>>8)*alpha+(uint)pDstLine->G*oneLessAlpha)>>8);
				pDstLine->B=(uint8)(((((uint)pSrcLine->B*(uint)pSrcLightingLine->B)>>8)*alpha+(uint)pDstLine->B*oneLessAlpha)>>8);

				// Next pixel
				pSrcLine+=pTileFar->SrcDeltaX;
				pSrcLightingLine++;
				pDstLine+=pTileFar->DstDeltaX;
			}
		}

		// Next line
		pSrcPixels+=pTileFar->SrcDeltaY;
		pSrcLightPixels+=pTileFar->SrcLightingDeltaY;
		pDstPixels+=pTileFar->DstDeltaY;
	}

	// stop MMX if used
	if(pTileFar->AsmMMX)
		NL3D_asmEndMMX();
}


// ***************************************************************************
// TODO: asm implementation of this function \\//
//#ifdef NL_NO_ASM
void NL3D_drawFarTileInFarTextureAdditive (const NL3D_CComputeTileFar* pTileFar)
{
	// Pointer of the Src diffuse pixels
	const CRGBA* pSrcPixels=pTileFar->SrcDiffusePixels;

	// Pointer of the Src additive pixels
	const CRGBA* pSrcAddPixels=pTileFar->SrcAdditivePixels;

	// Pointer of the Dst pixels
	const CRGBA* pSrcLightPixels=pTileFar->SrcLightingPixels;

	// Pointer of the Dst pixels
	CRGBA* pDstPixels=pTileFar->DstPixels;

	// For each pixels
	int x, y;
	for (y=0; y<pTileFar->Size; y++)
	{
		// Pointer of the source line
		const CRGBA* pSrcLine=pSrcPixels;

		// Pointer of the source line
		const CRGBA* pSrcAddLine=pSrcAddPixels;

		// Pointer of the source lighting line
		const CRGBA* pSrcLightingLine=pSrcLightPixels;

		// Pointer of the destination line
		CRGBA* pDstLine=pDstPixels;

		// For each pixels on the line
		for (x=0; x<pTileFar->Size; x++)
		{
			// Read and write a pixel
			uint nTmp=(((uint)pSrcLine->R*(uint)pSrcLightingLine->R)>>8)+(uint)pSrcAddLine->R;
			if (nTmp>255)
				nTmp=255;
			pDstLine->R=(uint8)nTmp;
			nTmp=(((uint)pSrcLine->G*(uint)pSrcLightingLine->G)>>8)+(uint)pSrcAddLine->G;
			if (nTmp>255)
				nTmp=255;
			pDstLine->G=(uint8)nTmp;
			nTmp=(((uint)pSrcLine->B*(uint)pSrcLightingLine->B)>>8)+(uint)pSrcAddLine->B;
			if (nTmp>255)
				nTmp=255;
			pDstLine->B=(uint8)nTmp;

			// Next pixel
			pSrcLine+=pTileFar->SrcDeltaX;
			pSrcAddLine+=pTileFar->SrcDeltaX;
			pSrcLightingLine++;
			pDstLine+=pTileFar->DstDeltaX;
		}

		// Next line
		pSrcPixels+=pTileFar->SrcDeltaY;
		pSrcAddPixels+=pTileFar->SrcDeltaY;
		pSrcLightPixels+=pTileFar->SrcLightingDeltaY;
		pDstPixels+=pTileFar->DstDeltaY;
	}
}
//#endif // NL_NO_ASM


// ***************************************************************************
// TODO: asm implementation of this function \\//
//#ifdef NL_NO_ASM
void NL3D_drawFarTileInFarTextureAdditiveAlpha (const NL3D_CComputeTileFar* pTileFar)
{
	// Pointer of the Src pixels
	const CRGBA* pSrcPixels=pTileFar->SrcDiffusePixels;

	// Pointer of the Src pixels
	const CRGBA* pSrcAddPixels=pTileFar->SrcAdditivePixels;

	// Pointer of the Dst pixels
	const CRGBA* pSrcLightPixels=pTileFar->SrcLightingPixels;

	// Pointer of the Dst pixels
	CRGBA* pDstPixels=pTileFar->DstPixels;

	// Fill the buffer with layer 0
	int x, y;
	for (y=0; y<pTileFar->Size; y++)
	{
		// Pointer of the source line
		const CRGBA* pSrcLine=pSrcPixels;

		// Pointer of the source line
		const CRGBA* pSrcAddLine=pSrcAddPixels;

		// Pointer of the source lighting line
		const CRGBA* pSrcLightingLine=pSrcLightPixels;

		// Pointer of the Dst pixels
		CRGBA* pDstLine=pDstPixels;

		// For each pixels on the line
		for (x=0; x<pTileFar->Size; x++)
		{
			// Read and write a pixel
			register uint alpha=pSrcLine->A;
			register uint oneLessAlpha=255-pSrcLine->A;
			
			// Read and write a pixel
			uint nTmp=(((uint)pSrcLine->R*(uint)pSrcLightingLine->R)>>8)+(uint)pSrcAddLine->R;
			if (nTmp>255)
				nTmp=255;
			pDstLine->R=(uint8)((nTmp*alpha+pDstLine->R*oneLessAlpha)>>8);
			nTmp=(((uint)pSrcLine->G*(uint)pSrcLightingLine->G)>>8)+(uint)pSrcAddLine->G;
			if (nTmp>255)
				nTmp=255;
			pDstLine->G=(uint8)((nTmp*alpha+pDstLine->G*oneLessAlpha)>>8);
			nTmp=(((uint)pSrcLine->B*(uint)pSrcLightingLine->B)>>8)+(uint)pSrcAddLine->B;
			if (nTmp>255)
				nTmp=255;
			pDstLine->B=(uint8)((nTmp*alpha+pDstLine->B*oneLessAlpha)>>8);

			// Next pixel
			pSrcLine+=pTileFar->SrcDeltaX;
			pSrcAddLine+=pTileFar->SrcDeltaX;
			pSrcLightingLine++;
			pDstLine+=pTileFar->DstDeltaX;
		}

		// Next line
		pSrcPixels+=pTileFar->SrcDeltaY;
		pSrcAddPixels+=pTileFar->SrcDeltaY;
		pSrcLightPixels+=pTileFar->SrcLightingDeltaY;
		pDstPixels+=pTileFar->DstDeltaY;
	}
}
//#endif // NL_NO_ASM
