/** \file collision_mesh_build.h
 * 
 *
 * $Id: collision_mesh_build.h,v 1.3 2001/08/07 14:14:32 legros Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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

#ifndef NL_COLLISION_MESH_BUILD_H
#define NL_COLLISION_MESH_BUILD_H

#include <vector>

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/file.h"

#include "nel/misc/vector.h"
#include "nel/misc/aabbox.h"


namespace NLPACS
{

struct CCollisionFace
{
	/// \name Attributes to set
	// @{

	/// The number of the vertices of the face.
	uint32	V[3];

	/// The visibility of each edge.
	bool	Visibility[3];

	/// The number of the surface of which it is associated. -1 means exterior surface.
	sint32	Surface;

	// @}


	/// \name Internal attributes
	// @{

	/// The link to the neighbor faces -- don't fill
	sint32	Edge[3];

	/// The number of the connex surface associated -- don't fill
	sint32	InternalSurface;

	/// The flags for each edge -- don't fill
	bool	EdgeFlags[3];

	// @}


	/// The exterior/interior surfaces id
	enum 
	{ 
		ExteriorSurface = -1, 
		InteriorSurfaceFirst = 0
	};
	
	/// Serialise the face
	void	serial(NLMISC::IStream &f)
	{
		f.serial(V[0]);
		f.serial(V[1]);
		f.serial(V[2]);

		f.serial(Visibility[0]);
		f.serial(Visibility[1]);
		f.serial(Visibility[2]);

		f.serial(Surface);
	}
};

/**
 * The collision intermediate mesh, used to build the real collision meshes.
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CCollisionMeshBuild
{
public:
	/// The vertices of the mesh
	std::vector<NLMISC::CVector>	Vertices;

	/// The faces of the mesh
	std::vector<CCollisionFace>		Faces;


public:
	void	serial(NLMISC::IStream &f)
	{
		f.serialCont(Vertices);
		f.serialCont(Faces);
	}

	void	translate(const NLMISC::CVector &translation)
	{
		uint	i;
		for (i=0; i<Vertices.size(); ++i)
			Vertices[i] += translation;
	}

	NLMISC::CVector	computeTrivialTranslation() const
	{
		uint	i;
		NLMISC::CAABBox	bbox;

		if (!Vertices.empty())
		{
			bbox.setCenter(Vertices[0]);
			for (i=1; i<Vertices.size(); ++i)
				bbox.extend(Vertices[i]);
		}

		return -bbox.getCenter();
	}
};

}; // NLPACS

#endif // NL_COLLISION_MESH_BUILD_H

/* End of collision_mesh_build.h */
