/** \file bezier_patch.h
 * <File description>
 *
 * $Id: bezier_patch.h,v 1.7 2001/01/15 15:45:23 corvazier Exp $
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

#ifndef NL_BEZIER_PATCH_H
#define NL_BEZIER_PATCH_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/vectord.h"
#include "nel/misc/matrix.h"


namespace NL3D 
{


using NLMISC::CVector;
using NLMISC::CVectorD;
using NLMISC::CMatrix;


// ***************************************************************************
/**
 * A standard bezier patch of float.  (quadpatch only).
 * QuadPatch layout (same notations as 3ds Max SDK).
 * 
 *   A---> ad ---- da <---D
 *   |                    |
 *   |                    |
 *   v                    v
 *   ab    ia      id     dc
 *
 *   |                    |
 *   |                    |
 *
 *   ba    ib      ic     cd
 *   ^                    ^
 *   |                    |
 *   |                    |
 *   B---> bc ---- cb <---C
 *
 * 
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CBezierPatch
{
public:
	/// The vertices a,b,c,d of the quad patch.
	CVector		Vertices[4];		
	/// The tangents ab, ba, bc, cb, cd, dc, da, ad. NB: tangents are points, not vectors.
	CVector		Tangents[8];		
	/// The interiors, ia,ib,ic,id. NB: interiors are points, not vectors.
	CVector		Interiors[4];		


public:
	/// Constructor: undefined Bezier patch!!
	CBezierPatch() {}

	/// complete computation of Vertices, Tangents, and Interiors, provinding the vertices and vertex normals.
	void		make(CVector vertices[4], CVector	normals[4]);
	/// make default Interiors, according to Vertices and Tangents.
	void		makeInteriors();
	/// Do the matrix transformation m*patch.
	void		applyMatrix(const CMatrix &m);

	/// Evaluate.
	CVector		eval(float s, float t) const;			// s,t coordinates for quad.
	/// Evaluate, but return a vector double.
	CVectorD	evalDouble(double s, double t) const;	// s,t coordinates for quad.
	/// Evaluate the normal at (s,t). returned vector is normalized.
	CVector		evalNormal(float s, float t) const;		// s,t coordinates for quad.

};



} // NL3D


#endif // NL_BEZIER_PATCH_H

/* End of bezier_patch.h */
