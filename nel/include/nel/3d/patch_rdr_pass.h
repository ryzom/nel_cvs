/** \file patch_rdr_pass.h
 * <File description>
 *
 * $Id: patch_rdr_pass.h,v 1.1 2000/11/30 10:57:28 berenguier Exp $
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
class	CPatchRdrPass
{
public:
	// The material for this pass.
	CMaterial		Mat;
	// The current number of tris for this rdrpass.
	sint			NTris;
	// Where this RdrPass begin, in the GlobalTriList.
	sint			StartIndex;
	// The current/end index for this RdrPass GlobalTriList.
	sint			CurIndex;
	// The BlockLen index, to know what is the length of a block (in Triangles).
	sint			BlockLenIndex;

	// Format of a single block:
	// |LEN|ID0|ID1|ID2|ID0|ID1|ID2....|JMP|

public:
	CPatchRdrPass();
	void			addTri(uint32 idx0, uint32 idx1, uint32 idx2);
	void			resetTriList();
	void			buildPBlock(CPrimitiveBlock &pb);


public:
	// Must resetTriList() of all material using the GlobalTriList, before calling resetGlobalTriList.
	static void		resetGlobalTriList();

private:
	static sint					CurGlobalIndex;
	static std::vector<uint32>	GlobalTriList;

};



} // NL3D


#endif // NL_PATCH_RDR_PASS_H

/* End of patch_rdr_pass.h */
