/** \file texture_far.h
 * <File description>
 *
 * $Id: texture_far.h,v 1.6 2003/04/23 10:07:58 berenguier Exp $
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

#ifndef NL_TEXTURE_FAR_H
#define NL_TEXTURE_FAR_H

#include "nel/misc/types_nl.h"
#include "3d/texture.h"
#include "nel/misc/rect.h"

// Define the number of tile per tile far texture 
#define NL_NUM_FAR_PATCHES_BY_EDGE_SHIFT 2														// 2 (shit)

// Same by precomputed values
#define NL_NUM_FAR_PATCHES_BY_EDGE (1<<NL_NUM_FAR_PATCHES_BY_EDGE_SHIFT)						// 4 patches by edges
#define NL_NUM_FAR_PATCHES_BY_EDGE_MASK (NL_NUM_FAR_PATCHES_BY_EDGE-1)							// 0x3 (mask)
#define NL_NUM_FAR_PATCHES_BY_TEXTURE (NL_NUM_FAR_PATCHES_BY_EDGE*NL_NUM_FAR_PATCHES_BY_EDGE)	// 16 patches by CTextureFar

namespace NLMISC
{
	class CRGBA;
}

namespace NL3D 
{

class CPatch;
class CTileFarBank;
class CTileColor;

/**
 * A CTextureFar is a set of NL_NUM_FAR_PATCHES_BY_TEXTURE texture used to map a whole patch when it is in far Mode. (ie not in tile mode).
 * A CTextureFar handle several patch texture.\\
 *
 * Before adding patch to the texture, you must call setSizeOfFarPatch, to intialize the texture.
 *
 * TODO: keeping the far texture level1 alive when the tile pass in level0 (tile mode), don't erase it.
 * TODO: add an hysteresis to avoid swap of far texture on boundaries of levels
 * TODO: set the upload format in rgba565
 *
 * \author Cyril Corvazier
 * \author Nevrax France
 * \date 2000
 */
class CTextureFar : public ITexture
{
public:
	/// Patch identifier
	struct CPatchIdent
	{
		/// Default constructor. do nothing, only for vector
		CPatchIdent () {};

		/// Constructor
		CPatchIdent (CPatch* patch)
		{
			Patch=patch;
		}

		// Data

		// Patch pointer
		CPatch*	Patch;
	};

	/// Constructor
	CTextureFar();
	virtual ~CTextureFar();

	/**
	  *  Set the size of the patch stored in this texture far. Note that width must be larger than height.
	  *  For patch with a bigger height than width, invert width and height value. So, in this texture far,
	  *  you can store patches with a size of width*height but also patches with a size of height*width.
	  *  
	  *  \param width is the width of the texture far stored in this texture. Can be 64, 32, 16, 8, 4 or 2
	  *  \param height is the height of the texture far stored in this texture. Can be 64, 32, 16, 8, 4 or 2
	  */
	void						setSizeOfFarPatch (sint width, sint height);

	/**
	 *  Add a patch in the CTexture Patch. Must not be full! Return true if the texture is full after adding this patch else false.
	 *
	 *  \param pPatch is the pointer to the patch to add in the landscape
	 *  \param far1UVScale will receive the scale to use to compute the UV coordinates
	 *  \param far1UBias will receive the U Bias to use to compute the UV coordinates
	 *  \param far1VBias will receive the V Bias to use to compute the UV coordinates
	 *  \param bRot will receive true if the texture is rotated of 90° to the left or false. 
	 *         You should take care of this value to compute UV coordinates.
	 */
	bool						addPatch (CPatch *pPatch, float& far1UScale, float& far1VScale, float& far1UBias, float& far1VBias, bool& bRot);

	/**
	 *  Remove a patch in the CTexture Patch.
	 */
	bool						removePatch (CPatch *pPatch);

	/**
	 *  Generate the texture. See ITexture::doGenerate().
	 *
	 *  \see ITexture::doGenerate()
	 */
	virtual void				doGenerate();

	/**
	 *	Touch a patch by its id in texture (call touchRect()).
	 *	\param patchId a value beetween 0 and NL_NUM_FAR_PATCHES_BY_TEXTURE-1, which gives the id of the patch 
	 *	in the texture
	 *	\return number of pixels touched. 0 if Patch==NULL (empty).
	 */
	uint						touchPatch(uint patchId);


	/// For lighting update, insert this before textNext (CiruclarList). textNext must be !NULL
	void						linkBeforeUL(CTextureFar *textNext);
	/// For lighting update, unlink (CiruclarList)
	void						unlinkUL();
	/// For lighting update, get Next (CiruclarList). If ==this, then list is empty
	CTextureFar					*getNextUL() const {return _ULNext;}


	// Get number of patch in this textureFar
	uint32						getPatchCount() const {return _PatchCount;}
	uint32						getPatchWidth() const {return _OriginalWidth/NL_NUM_FAR_PATCHES_BY_EDGE;}
	uint32						getPatchHeight() const {return _OriginalHeight/NL_NUM_FAR_PATCHES_BY_EDGE;}

	// Data

	/**
	 *  Vector of patches which texture far is stored in this CTextureFar
	 *  Should be == to _WidthPatches*_HeightPatches
	 */
	std::vector<CPatchIdent>	_Patches;

	// Count of patch stored in this texture
	uint32						_PatchCount;

	/// A pointer on the far bank.
	CTileFarBank*				_Bank;

private:
	/// The original size
	uint32						_OriginalWidth;
	uint32						_OriginalHeight;

	/**
	 *  Rebuild the rectangle passed in parameter
	 */
	void rebuildRectangle (uint x, uint y);

	/// From IStreamable
	virtual void	serial(NLMISC::IStream &f) throw(NLMISC::EStream) {};

	// Some static buffers
	static NLMISC::CRGBA	_LightmapExpanded[];
	static uint8			_LumelExpanded[];
	static NLMISC::CRGBA	_TileTLIColors[];

	NLMISC_DECLARE_CLASS(CTextureFar);

	/// UpdateLighting. CiruclarList
	CTextureFar					*_ULPrec;
	CTextureFar					*_ULNext;
};

} // NL3D

// For NL3D_drawFarTileInFarTexture external call
struct NL3D_CComputeTileFar
{
public:
	// TileFar pixels
	const NLMISC::CRGBA*		SrcDiffusePixels;

	// TileFar pixels
	const NLMISC::CRGBA*		SrcAdditivePixels;

	// Source deltaY
	sint32						SrcDeltaX;

	// Source deltaY
	sint32						SrcDeltaY;

	// Source lighting
	const NLMISC::CRGBA*		SrcLightingPixels;

	// Delta Y for lighting
	sint32						SrcLightingDeltaY;

	// TileFar pixels
	NLMISC::CRGBA*				DstPixels;

	// Destination deltaX
	sint32						DstDeltaX;

	// Destination deltaY
	sint32						DstDeltaY;

	// Size
	sint32						Size;

	// Can the compute be done in MMX
	bool						AsmMMX;
};

// For NL3D_expandLightmap external call
struct NL3D_CExpandLightmap
{
public:
	// CTileColor array.
	const NL3D::CTileColor*		ColorTile;
	// TLI Color array.
	const NLMISC::CRGBA*		TLIColor;

	// Lumel array. 4x4 lumels by tile.
	const uint8*				LumelTile;

	// Width of the array
	uint32						Width;

	// Height of the array
	uint32						Height;

	// Mul factor for the size (1, 2 or 4)
	uint32						MulFactor;

	// Static light color
	const NLMISC::CRGBA*		StaticLightColor;

	// Destination array
	NLMISC::CRGBA*				DstPixels;
};
	
// Extern ASM functions
extern "C" void NL3D_expandLightmap (const NL3D_CExpandLightmap* pLightmap);
extern "C" void NL3D_drawFarTileInFarTexture (const NL3D_CComputeTileFar* pTileFar);
extern "C" void NL3D_drawFarTileInFarTextureAdditive (const NL3D_CComputeTileFar* pTileFar);
extern "C" void NL3D_drawFarTileInFarTextureAlpha (const NL3D_CComputeTileFar* pTileFar);
extern "C" void NL3D_drawFarTileInFarTextureAdditiveAlpha (const NL3D_CComputeTileFar* pTileFar);

#endif // NL_TEXTURE_FAR_H

/* End of texture_far.h */
