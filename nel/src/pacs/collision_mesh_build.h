/** \file collision_mesh_build.h
 * 
 *
 * $Id: collision_mesh_build.h,v 1.1 2001/07/24 08:44:19 legros Exp $
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



namespace NLPACS
{

struct CCollisionFace
{
	uint32	V[3];
	bool	Visibility[3];
	bool	HasExit;
	bool	HasDoor;

	void	serial(NLMISC::IStream &f)
	{
		f.serial(V[0]);
		f.serial(V[1]);
		f.serial(V[2]);

		f.serial(Visibility[0]);
		f.serial(Visibility[1]);
		f.serial(Visibility[2]);

		f.serial(HasExit);
		f.serial(HasDoor);
	}
}

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
};

}; // NLPACS

#endif // NL_COLLISION_MESH_BUILD_H

/* End of collision_mesh_build.h */
