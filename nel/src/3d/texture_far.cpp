/** \file texture_far.cpp
 * Texture used to store far textures for several patches.
 *
 * $Id: texture_far.cpp,v 1.3 2001/01/08 17:58:30 corvazier Exp $
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

#include "nel/3d/texture_far.h"
#include "nel/3d/tile_far_bank.h"
#include "nel/3d/patch.h"

using namespace NLMISC;

namespace NL3D {

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
bool CTextureFar::addPatch (CPatch *pPatch, float& farUVScale, float& farUBias, float& farVBias, bool& bRot)
{
	// Check that at least a cell is free
	nlassert (_PatchCount<NL_NUM_FAR_PATCHES_BY_TEXTURE);

	// Look for a free cell
	for (sint p=0; p<NL_NUM_FAR_PATCHES_BY_TEXTURE; p++)
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
	ITexture::touchRect(CRect (x, y, _OriginalWidth>>NL_NUM_FAR_PATCHES_BY_EDGE_SHIFT, _OriginalHeight>>NL_NUM_FAR_PATCHES_BY_EDGE_SHIFT));

	// ** Return some values

	// Scale is the same for all
	farUVScale=(float)((_OriginalWidth>>NL_NUM_FAR_PATCHES_BY_EDGE_SHIFT)-1)/(float)_OriginalWidth;

	// UBias is the same for all
	farUBias=((float)x+0.5f)/(float)_OriginalWidth;

	// UBias is the same for all
	farVBias=((float)y+0.5f)/(float)_OriginalHeight;

	// Rotation flag
	bRot = ( pPatch->getOrderS() < pPatch->getOrderT() );
	
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
	for (sint p=0; p<NL_NUM_FAR_PATCHES_BY_TEXTURE; p++)
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

// Generate the texture. See ITexture::generate().
void CTextureFar::generate ()
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

			// Build the rectangle
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
	sint nS=patch->getOrderS();
	sint nT=patch->getOrderT();

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
	CTileFarBank::TFarOrder orderX;
	uint tileSize;
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

	// Base Dst pointer on the tile line
	uint nBaseDstTileLine=nBaseOffset;
	for (sint t=0; t<nT; t++)
	{
		// Base Dst pointer on the tile
		uint nBaseDstTilePixels=nBaseDstTileLine;

		// For each tile of the line
		for (sint s=0; s<nS; s++)
		{
			// For each layer of the tile
			for (sint l=0; l<3; l++)
			{
				// ** Fill the struct for the tile fill method for each layers
				NL3D_CComputeTileFar TileFar;

				// Use of additive in this layer ?
				bool bAdditive=false;

				// Size of the edge far tile
				TileFar.Size=tileSize;

				// Base pointer of the texture
				TileFar.DstPixels=(CRGBA*)&(getPixels()[0]);

				// Get a tile element reference for this tile.
				const CTileElement &tileElm=patch->Tiles[nTileInPatch];

				// Check for 256 tiles...
				bool	is256x256;
				uint8	uvOff;
				tileElm.getTile256Info(is256x256, uvOff);

				// Get the tile number
				sint tile=tileElm.Tile[l];

				// Is an non-empty layer ?
				if (tile!=NL_TILE_ELM_LAYER_EMPTY)
				{
					// Get the read only pointer on the far tile
					const CTileFarBank::CTileFar*	pTile=_Bank->getTile (tile);

					// This pointer must not be null
					nlassert (pTile);

					// Tile exist ?
					if (pTile->isFill (CTileFarBank::diffuse))
					{
						// Get rotation of the tile in this layer
						sint nRot=tileElm.getTileOrient(l);

						// Source pointer
						TileFar.SrcDiffusePixels=pTile->getPixels (CTileFarBank::diffuse, orderX);

						// Additive ?
						if (pTile->isFill (CTileFarBank::additive))
						{
							// Use it
							bAdditive=true;

							// Get additive pointer
							TileFar.SrcAdditivePixels=pTile->getPixels (CTileFarBank::additive, orderX);
						}

						// 256 ?
						if (is256x256)
						{
							// Calc offset
							uint nOffset=0;

							// On the left ?
							if (uvOff&0x02)
								nOffset+=tileSize;

							// On the bottom ?
							if ((uvOff==1)||(uvOff==2))
								nOffset+=2*tileSize*tileSize;

							// Change origine of the diffuse bitmaps
							TileFar.SrcDiffusePixels+=nOffset;

							// Change origine of the additive bitmaps
							if (TileFar.SrcAdditivePixels)
								TileFar.SrcAdditivePixels+=nOffset;

							// Yes, 256
							TileFar.SrcDeltaY=tileSize<<1;
						}
						else
						{
							// No, 128
							TileFar.SrcDeltaY=tileSize;
						}

						// Compute offset and deltas
						switch (nRot)
						{
						case 0:
							// Destination pointer
							TileFar.DstOffset=nBaseDstTilePixels;

							// Destination delta
							TileFar.DstDeltaX=dstDeltaX;
							TileFar.DstDeltaY=dstDeltaY;
							break;
						case 1:
							// Destination pointer
							TileFar.DstOffset=nBaseDstTilePixels+dstDeltaY*(tileSize-1);

							// Destination delta
							TileFar.DstDeltaX=-dstDeltaY;
							TileFar.DstDeltaY=dstDeltaX;
							break;
						case 2:
							// Destination pointer
							TileFar.DstOffset=nBaseDstTilePixels+dstDeltaY*(tileSize-1)+dstDeltaX*(tileSize-1);

							// Destination delta
							TileFar.DstDeltaX=-dstDeltaX;
							TileFar.DstDeltaY=-dstDeltaY;
							break;
						case 3:
							// Destination pointer
							TileFar.DstOffset=nBaseDstTilePixels+dstDeltaX*(tileSize-1);

							// Destination delta
							TileFar.DstDeltaX=dstDeltaY;
							TileFar.DstDeltaY=-dstDeltaX;
							break;
						}

						// *** Draw the layer

						// Alpha layer ?
						if (l>0)
						{
							// Additive layer ?
							if (bAdditive)
								NL3D_drawFarTileInFarTextureAdditiveAlpha (&TileFar);
							else	// No additive layer
								NL3D_drawFarTileInFarTextureAlpha (&TileFar);
						}
						else	// no alpha
						{
							// Additive layer ?
							if (bAdditive)
								NL3D_drawFarTileInFarTextureAdditive (&TileFar);
							else	// No additive layer
								NL3D_drawFarTileInFarTexture (&TileFar);
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
// TODO: asm implementation of this function \\//
//#ifdef NL_NO_ASM
void NL3D_drawFarTileInFarTexture (const NL3D_CComputeTileFar* pTileFar)
{
	// Pointer of the Src diffuse pixels
	const CRGBA* pSrcPixels=pTileFar->SrcDiffusePixels;

	// Pointer of the Dst pixels
	CRGBA* pDstPixels=pTileFar->DstPixels+pTileFar->DstOffset;

	// For each pixels
	int x, y;
	for (y=0; y<pTileFar->Size; y++)
	{
		// Pointer of the source line
		const CRGBA* pSrcLine=pSrcPixels;

		// Pointer of the destination line
		CRGBA* pDstLine=pDstPixels;

		// For each pixels on the line
		for (x=0; x<pTileFar->Size; x++)
		{
			// Read and write a pixel
			*pDstLine=*pSrcLine;

			// Next pixel
			pSrcLine++;
			pDstLine+=pTileFar->DstDeltaX;
		}

		// Next line
		pSrcPixels+=pTileFar->SrcDeltaY;
		pDstPixels+=pTileFar->DstDeltaY;
	}
}
//#endif // NL_NO_ASM


// ***************************************************************************
// TODO: asm implementation of this function \\//
//#ifdef NL_NO_ASM
void NL3D_drawFarTileInFarTextureAlpha (const NL3D_CComputeTileFar* pTileFar)
{
	// Pointer of the Src pixels
	const CRGBA* pSrcPixels=pTileFar->SrcDiffusePixels;

	// Pointer of the Dst pixels
	CRGBA* pDstPixels=pTileFar->DstPixels+pTileFar->DstOffset;

	// Fill the buffer with layer 0
	int x, y;
	for (y=0; y<pTileFar->Size; y++)
	{
		// Pointer of the source line
		const CRGBA* pSrcLine=pSrcPixels;

		// Pointer of the Dst pixels
		CRGBA* pDstLine=pDstPixels;

		// For each pixels on the line
		for (x=0; x<pTileFar->Size; x++)
		{
			// Read and write a pixel
			register uint alpha=pSrcLine->A;
			register uint oneLessAlpha=255-pSrcLine->A;
			pDstLine->R=(uint8)(((uint)pSrcLine->R*alpha+(uint)pDstLine->R*oneLessAlpha)>>8);
			pDstLine->G=(uint8)(((uint)pSrcLine->G*alpha+(uint)pDstLine->G*oneLessAlpha)>>8);
			pDstLine->B=(uint8)(((uint)pSrcLine->B*alpha+(uint)pDstLine->B*oneLessAlpha)>>8);

			// Next pixel
			pSrcLine++;
			pDstLine+=pTileFar->DstDeltaX;
		}

		// Next line
		pSrcPixels+=pTileFar->SrcDeltaY;
		pDstPixels+=pTileFar->DstDeltaY;
	}
}
//#endif // NL_NO_ASM


// ***************************************************************************
// TODO: asm implementation of this function \\//
//#ifdef NL_NO_ASM
void NL3D_drawFarTileInFarTextureAdditive (const NL3D_CComputeTileFar* pTileFar)
{
	// Pointer of the Src diffuse pixels
	const CRGBA* pSrcPixels=pTileFar->SrcDiffusePixels;

	// Pointer of the Src additive pixels
	const CRGBA* pSrcAddPixels=pTileFar->SrcDiffusePixels;

	// Pointer of the Dst pixels
	CRGBA* pDstPixels=pTileFar->DstPixels+pTileFar->DstOffset;

	// For each pixels
	int x, y;
	for (y=0; y<pTileFar->Size; y++)
	{
		// Pointer of the source line
		const CRGBA* pSrcLine=pSrcPixels;

		// Pointer of the source line
		const CRGBA* pSrcAddLine=pSrcAddPixels;

		// Pointer of the destination line
		CRGBA* pDstLine=pDstPixels;

		// For each pixels on the line
		for (x=0; x<pTileFar->Size; x++)
		{
			// Read and write a pixel
			uint nTmp=(uint)pSrcLine->R+(uint)pSrcAddLine->R;
			if (nTmp>255)
				nTmp=255;
			pDstLine->R=(uint8)nTmp;
			nTmp=(uint)pSrcLine->G+(uint)pSrcAddLine->G;
			if (nTmp>255)
				nTmp=255;
			pDstLine->G=(uint8)nTmp;
			nTmp=(uint)pSrcLine->B+(uint)pSrcAddLine->B;
			if (nTmp>255)
				nTmp=255;
			pDstLine->B=(uint8)nTmp;

			// Next pixel
			pSrcLine++;
			pSrcAddLine++;
			pDstLine+=pTileFar->DstDeltaX;
		}

		// Next line
		pSrcPixels+=pTileFar->SrcDeltaY;
		pSrcAddPixels+=pTileFar->SrcDeltaY;
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
	CRGBA* pDstPixels=pTileFar->DstPixels+pTileFar->DstOffset;

	// Fill the buffer with layer 0
	int x, y;
	for (y=0; y<pTileFar->Size; y++)
	{
		// Pointer of the source line
		const CRGBA* pSrcLine=pSrcPixels;

		// Pointer of the source line
		const CRGBA* pSrcAddLine=pSrcAddPixels;

		// Pointer of the Dst pixels
		CRGBA* pDstLine=pDstPixels;

		// For each pixels on the line
		for (x=0; x<pTileFar->Size; x++)
		{
			// Read and write a pixel
			register uint alpha=pSrcLine->A;
			register uint oneLessAlpha=255-pSrcLine->A;
			
			// Read and write a pixel
			uint nTmp=(uint)pSrcLine->R+(uint)pSrcAddLine->R;
			if (nTmp>255)
				nTmp=255;
			pDstLine->R=(uint8)((nTmp*alpha+pDstLine->R*oneLessAlpha)>>8);
			nTmp=(uint)pSrcLine->G+(uint)pSrcAddLine->G;
			if (nTmp>255)
				nTmp=255;
			pDstLine->G=(uint8)((nTmp*alpha+pDstLine->G*oneLessAlpha)>>8);
			nTmp=(uint)pSrcLine->B+(uint)pSrcAddLine->B;
			if (nTmp>255)
				nTmp=255;
			pDstLine->B=(uint8)((nTmp*alpha+pDstLine->B*oneLessAlpha)>>8);

			// Next pixel
			pSrcLine++;
			pSrcAddLine++;
			pDstLine+=pTileFar->DstDeltaX;
		}

		// Next line
		pSrcPixels+=pTileFar->SrcDeltaY;
		pSrcAddPixels+=pTileFar->SrcDeltaY;
		pDstPixels+=pTileFar->DstDeltaY;
	}
}
//#endif // NL_NO_ASM
