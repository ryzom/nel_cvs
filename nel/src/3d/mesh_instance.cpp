/** \file mesh_instance.cpp
 * <File description>
 *
 * $Id: mesh_instance.cpp,v 1.15 2002/08/07 08:37:40 vizerie Exp $
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

#include "std3d.h"

#include "3d/mesh_instance.h"
#include "3d/mesh.h"
#include "3d/skeleton_model.h"
#include <list>

using namespace std;

namespace NL3D 
{

// ***************************************************************************
CMeshInstance::CMeshInstance()
{
	// LoadBalancing is not usefull for Mesh, because meshs cannot be reduced in faces.
	// Override CTransformShape state.
	CTransform::setIsLoadbalancable(false);
}

// ***************************************************************************
CMeshInstance::~CMeshInstance()
{
	// Auto detach me from skeleton. Must do it here, not in ~CTransform().
	if(_FatherSkeletonModel)
	{
		// detach me from the skeleton.
		// Observers hierarchy is modified.
		_FatherSkeletonModel->detachSkeletonSon(this);
		nlassert(_FatherSkeletonModel==NULL);
	}
}


// ***************************************************************************
void		CMeshInstance::registerBasic()
{
	CMOT::registerModel(MeshInstanceId, MeshBaseInstanceId, CMeshInstance::creator);
}

// ***************************************************************************
void		CMeshInstance::setApplySkin(bool state)
{
	// Call parents method
	CMeshBaseInstance::setApplySkin (state);

	// Get a pointer on the shape
	CMesh *pMesh = NLMISC::safe_cast<CMesh *>((IShape*)Shape);

	// Recompute the id
	if (state)
	{
		pMesh->computeBonesId (_FatherSkeletonModel);
	}

	// update the skeleton usage according to the mesh.
	pMesh->updateSkeletonUsage(_FatherSkeletonModel, state);
}


// ***************************************************************************
const std::vector<sint32>	*CMeshInstance::getSkinBoneUsage() const
{
	// Get a pointer on the shape
	CMesh	*pMesh = NLMISC::safe_cast<CMesh *>((IShape*)Shape);

	// Recompute the id
	pMesh->computeBonesId (_FatherSkeletonModel);

	// get ids.
	return &pMesh->getMeshGeom().getSkinBoneUsage();
}


// ***************************************************************************
bool	CMeshInstance::isSkinnable() const
{
	if(Shape==NULL)
		return false;

	// Get a pointer on the shape
	CMesh *pMesh = NLMISC::safe_cast<CMesh *>((IShape*)Shape);

	// true if the mesh is skinned
	return pMesh->getMeshGeom().isSkinned();
}


// ***************************************************************************
void	CMeshInstance::renderSkin(float alphaMRM)
{
	// Don't setup lighting or matrix in Skin. Done by the skeleton

	if(Shape && getVisibility() == CHrcTrav::Show)
	{
		// Get a pointer on the shape
		CMesh *pMesh = NLMISC::safe_cast<CMesh *>((IShape*)Shape);

		// render the meshGeom
		CMeshGeom	&meshGeom= const_cast<CMeshGeom&>(pMesh->getMeshGeom ());
		meshGeom.renderSkin( this, alphaMRM );
	}
}


} // NL3D
