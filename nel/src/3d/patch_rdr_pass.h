/** \file patch_rdr_pass.h
 * <File description>
 *
 * $Id: patch_rdr_pass.h,v 1.4 2002/08/26 13:01:42 berenguier Exp $
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

#ifndef NL_PATCH_RDR_PASS_H
#define NL_PATCH_RDR_PASS_H

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "3d/driver.h"
#include "3d/landscape_def.h"
#include <vector>


namespace NL3D 
{


class	CPatchRdrPass;
class	CPatch;
struct	CTileMaterial;


// ***************************************************************************
class	CRdrPatchId
{
public:
	CPatchRdrPass	*PatchRdrPass;
	CPatch			*Patch;

	CRdrPatchId();

	// For list reading.
	CRdrPatchId		*getNext() {return _Next;}

private:
	friend	class	CPatchRdrPass;
	CRdrPatchId		*_Next;
};

// ***************************************************************************
class	CRdrTileId
{
public:
	CPatchRdrPass	*PatchRdrPass;
	CTileMaterial	*TileMaterial;

	CRdrTileId();

	// For list reading.
	CRdrTileId		*getNext() {return _Next;}

private:
	friend	class	CPatchRdrPass;
	CRdrTileId		*_Next;
};


// ***************************************************************************
/**
 * A render pass for a landscape material (tile or texture far).
 * Has list of Patch / TileId.
 */
class	CPatchRdrPass : public CRefCount
{
public:
	// The refcount to know how many tiles use it (init at 0).
	sint			RefCount;

	/// \name The Tiny material for this pass.
	// @{
	// The diffuse texture (for Far and tiles).
	NLMISC::CSmartPtr<ITexture>		TextureDiffuse;
	// The Alpha texture (for tiles only).
	NLMISC::CSmartPtr<ITexture>		TextureAlpha;
	// @}


	/// \name The Patch/Tile List for this pass.. updated at each render(), (in CPatch::preRender() and CLandscape::render()).
	/**	
	 *	maxRenderedFaces is used to over-estimate the number of faces that will be rendered in this pass.
	 *	Since all Far0/Far1/Tiles are added, this is REALLY over-estimated because in CLandscape::render(),
	 *	Far0/Far1/Tiles render are split.
	 */
	// @{
	void		clearAllRenderList();
	void		appendRdrPatchFar0(CRdrPatchId *rdrPatch, uint maxRenderedFaces)
	{
		_MaxRenderedFaces+= maxRenderedFaces;
		rdrPatch->_Next= _Far0ListRoot;
		_Far0ListRoot= rdrPatch;
	}
	void		appendRdrPatchFar1(CRdrPatchId *rdrPatch, uint maxRenderedFaces)
	{
		_MaxRenderedFaces+= maxRenderedFaces;
		rdrPatch->_Next= _Far1ListRoot;
		_Far1ListRoot= rdrPatch;
	}
	void		appendRdrPatchTile(uint pass, CRdrTileId *rdrTile, uint maxRenderedFaces)
	{
		_MaxRenderedFaces+= maxRenderedFaces;
		rdrTile->_Next= _TileListRoot[pass];
		_TileListRoot[pass]= rdrTile;
	}
	CRdrPatchId *getRdrPatchFar0() {return _Far0ListRoot;}
	CRdrPatchId *getRdrPatchFar1() {return _Far1ListRoot;}
	CRdrTileId  *getRdrTileRoot(uint pass) {return _TileListRoot[pass];}
	uint		getMaxRenderedFaces() const {return _MaxRenderedFaces;}
	// @}


public:
	CPatchRdrPass();

	// The operator which compare the material.
	bool			operator<(const CPatchRdrPass &o) const
	{
		// Compare first the Alphatext, so minmum changes are made during render...
		if(TextureAlpha!=o.TextureAlpha)
			return (void*)TextureAlpha<(void*)o.TextureAlpha;
		else
			return (void*)TextureDiffuse<(void*)o.TextureDiffuse;
	}

private:
	uint				_MaxRenderedFaces;
	// The list for Far0 and Far1 to render.
	CRdrPatchId			*_Far0ListRoot;
	CRdrPatchId			*_Far1ListRoot;
	// The list for each pass of Tiles to render.
	CRdrTileId			*_TileListRoot[NL3D_MAX_TILE_PASS];

};



} // NL3D


#endif // NL_PATCH_RDR_PASS_H

/* End of patch_rdr_pass.h */
