/** \file raw_skin.h
 * Packed struct used for faster Software skinning
 *
 * $Id: raw_skin.h,v 1.1 2002/08/05 12:17:29 berenguier Exp $
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

#ifndef NL_RAW_SKIN_H
#define NL_RAW_SKIN_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/object_vector.h"
#include "3d/mesh.h"


namespace NL3D 
{


using	NLMISC::CVector;


/// Vertices influenced by 1 matrix only.
class	CRawVertexNormalSkin1
{
public:
	// The id of the matrix to use.
	uint32		MatrixId[1];
	CVector		Vertex;
	CVector		Normal;
	// The Dest VertexId
	uint32		VertexId;
};

/// Vertices influenced by 2 matrix only.
class	CRawVertexNormalSkin2
{
public:
	// The id of the matrix to use.
	uint32		MatrixId[2];
	float		Weights[2];
	CVector		Vertex;
	CVector		Normal;
	// The Dest VertexId
	uint32		VertexId;
};

/// Vertices influenced by 3 or 4 matrix only. (simpler and rare)
class	CRawVertexNormalSkin4
{
public:
	CMesh::CSkinWeight	SkinWeight;
	CVector		Vertex;
	CVector		Normal;
	// The Dest VertexId
	uint32		VertexId;
};

/// The array per lod.
class	CRawSkinNormalCache
{
public:
	// The vertices influenced by 1 matrix.
	NLMISC::CObjectVector<CRawVertexNormalSkin1, false>	Vertices1;
	// The vertices influenced by 2 matrix.
	NLMISC::CObjectVector<CRawVertexNormalSkin2, false>	Vertices2;
	// The vertices influenced by 3 matrix.
	NLMISC::CObjectVector<CRawVertexNormalSkin4, false>	Vertices3;
	// The vertices influenced by 4 matrix.
	NLMISC::CObjectVector<CRawVertexNormalSkin4, false>	Vertices4;

	/// What RawSkin lod this cache represent. -1 if NULL
	sint					LodId;
	/// To see if same Data than in the CMeshMRMGeom
	uint					MeshDataId;

	// free up the memory
	void				clearArrays();
};



} // NL3D


#endif // NL_RAW_SKIN_H

/* End of raw_skin.h */
