/** \file patch_rdr_pass.h
 * <File description>
 *
 * $Id: patch_rdr_pass.h,v 1.6 2001/02/20 11:03:39 berenguier Exp $
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
#include "nel/3d/driver.h"
#include <vector>


namespace NL3D 
{


// ***************************************************************************
/**
 * A render pass for a landscape material (tile or texture far).
 * A Primitive Block is not used, for speed improvement: must not realloc at each face or at each patch...
 *
 * How does it work: class CPatchRdrPass maintain a GlobalTriList array, where tri indices are stored.
 * Since Tris may be added in a interleaved fashion (33 tris indices for material 0, then 12 tris indices for material 1, 
 * then 60 tris indices for material 0 etc...), this GlobalTriList is so interleaved (with some indices used for interleaving).
 *
 * NB: GlobalTriList is a "grow only" vector, even across frames.... So reallocation never happens, but at begin of program.
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


	/// \name The "Primitive List" for this pass..
	// @{
	// in preRender() pass, count how max primitive this pass will use.
	sint			NMaxTris;
	// Where this RdrPass begin, in the GlobalTriList.
	sint			StartIndex;
	// The current number of tris for this rdrpass.
	sint			NTris;
	// The current index .
	sint			CurIndex;
	// @}


public:
	CPatchRdrPass();

	/* BEFORE LANDSCAPE RENDER, MUST CALL IN ORDER:
		- resetGlobalTriList.
		- prePass, to compute MaxTris of ALL renderPass. (=> call before resetMaxTriList()).
		- resetGlobalTriList.
		- prePassBis, to compute StartIndex of ALL pass.
		- N* 
			- Fill Pass, with resetTriList each time.
			- render.
	*/

	// PrePass: count tris max wanted.
	void			resetMaxTriList();			// Reset MaxTris and StartIndex.
	void			addMaxTris(sint ntris);		// may allocate GlobalTriList.

	// PrePassBis: compute StartIndex.
	void			computeStartIndex();

	// FillPass: fill the "primitive block".
	void			resetTriList();			// Reset NTris=0, and CurrentIndex= StartIndex.
	void			addTri(uint32 idx0, uint32 idx1, uint32 idx2);

	// Render Pass: with the pointer and Ntrsi, render!!
	uint32			*getStartPointer() {return (uint32*)&(*GlobalTriList.begin()) + StartIndex;}

	// The operator which compare the material.
	bool			operator<(const CPatchRdrPass &o) const
	{
		// Compare first the Alphatext, so minmum changes are made during render...
		if(TextureAlpha!=o.TextureAlpha)
			return (void*)TextureAlpha<(void*)o.TextureAlpha;
		else
			return (void*)TextureDiffuse<(void*)o.TextureDiffuse;
	}

public:
	static	void	resetGlobalIndex();

private:
	static sint		MaxGlobalIndex;
	static std::vector<uint32>	GlobalTriList;

};



// ***************************************************************************
inline void			CPatchRdrPass::addTri(uint32 idx0, uint32 idx1, uint32 idx2)
{
	// An error may occurs if resetGlobalTriList() called, but not resetTriList().
	nlassert(CurIndex<MaxGlobalIndex);
	nlassert(NTris<NMaxTris);

	GlobalTriList[CurIndex++]= idx0;
	GlobalTriList[CurIndex++]= idx1;
	GlobalTriList[CurIndex++]= idx2;
	NTris++;
}


} // NL3D


#endif // NL_PATCH_RDR_PASS_H

/* End of patch_rdr_pass.h */
