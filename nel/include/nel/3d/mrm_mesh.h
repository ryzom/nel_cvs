/** \file mrm_mesh.h
 * Internal header for CMRMBuilder.
 *
 * $Id: mrm_mesh.h,v 1.2 2001/01/02 10:21:43 berenguier Exp $
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

#ifndef NL_MRM_MESH_H
#define NL_MRM_MESH_H

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/vector_h.h"
#include "nel/misc/vector.h"
#include <vector>


namespace NL3D
{


using NLMISC::CVector;
using NLMISC::CVectorH;


// ***************************************************************************
#define	NL3D_MRM_MAX_ATTRIB		12


// ***************************************************************************
/**
 * An internal mesh corner Index representation for MRM.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
struct	CMRMCorner
{
	// The id of the vertex.
	sint	Vertex;
	// The ids of the wedges. Points on Attributes of the mesh.
	sint	Attributes[NL3D_MRM_MAX_ATTRIB];
};


// ***************************************************************************
/**
 * An internal mesh face representation for MRM.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
struct	CMRMFace
{
public:
	// The 3 corner indices of the face.
	CMRMCorner		Corner[3];
	// The id of the material. Used for material boundaries.
	sint			MaterialId;
};



// ***************************************************************************
/**
 * An internal mesh representation for MRM. USER DO NOT USE IT!!
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 * \see CMRMBuilder
 */
class CMRMMesh
{
public:
	// The vertices of the MRMMesh.
	std::vector<CVector>		Vertices;
	// The attributes of the MRMMesh.
	std::vector<CVectorH>		Attributes[NL3D_MRM_MAX_ATTRIB];
	// The number of used attributes of the MRMMesh.
	sint						NumAttributes;
	// The faces of the MRMMesh.
	std::vector<CMRMFace>		Faces;


public:
	/// Constructor
	CMRMMesh();

};


// ***************************************************************************
/**
 * An internal mesh representation for MRM, with geomoprh information. USER DO NOT USE IT.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 * \see CMRMBuilder
 */
class CMRMMeshGeom : public CMRMMesh
{
public:

public:
	/// Constructor
	CMRMMeshGeom();
};


} // NL3D


#endif // NL_MRM_MESH_H

/* End of mrm_mesh.h */
