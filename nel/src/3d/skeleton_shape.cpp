/** \file skeleton_shape.cpp
 * <File description>
 *
 * $Id: skeleton_shape.cpp,v 1.9 2002/03/20 11:17:25 berenguier Exp $
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

#include "3d/skeleton_shape.h"
#include "3d/skeleton_model.h"
#include "3d/scene.h"
#include "nel/misc/bsphere.h"

using namespace NLMISC;

namespace NL3D
{


// ***************************************************************************
CSkeletonShape::CSkeletonShape()
{
	// By default for now....
	// Temp. Have a huge BBox, so clip badly. 
	_BBox.setCenter(CVector(0,0,1.5));
	_BBox.setSize(CVector(3,3,3));
}


// ***************************************************************************
sint32			CSkeletonShape::getBoneIdByName(const std::string &name) const
{
	std::map<std::string, uint32>::const_iterator	it= _BoneMap.find(name);
	if(it==_BoneMap.end())
		return -1;
	else
		return it->second;
}


// ***************************************************************************
void			CSkeletonShape::build(const std::vector<CBoneBase> &bones)
{
	// copy bones.
	_Bones= bones;

	// build the map.
	for(uint i=0;i<_Bones.size();i++)
	{
		_BoneMap[_Bones[i].Name]= i;
	}
}


// ***************************************************************************
CTransformShape		*CSkeletonShape::createInstance(CScene &scene)
{
	// Create a CSkeletonModel, an instance of a mesh.
	//===============================================
	CSkeletonModel		*sm= (CSkeletonModel*)scene.createModel(NL3D::SkeletonModelId);
	sm->Shape= this;

	// setup bones.
	//=================
	sm->Bones.reserve(_Bones.size());
	for(sint i=0;i<(sint)_Bones.size();i++)
	{
		// Append a new bone.
		sm->Bones.push_back( CBone(&_Bones[i]) );

		// Must set the Animatable father of the bone (the skeleton model!).
		sm->Bones[i].setFather(sm, CSkeletonModel::OwnerBit);
	}

	// Must create and init skeleton bone usage to 0.
	sm->initBoneUsages();

	return sm;
}

// ***************************************************************************
void			CSkeletonShape::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	sint	ver= f.serialVersion(0);

	f.serialCont(_Bones);
	f.serialCont(_BoneMap);
}

// ***************************************************************************

float CSkeletonShape::getNumTriangles (float distance)
{
	// No polygons
	return 0;
}


// ***************************************************************************
bool	CSkeletonShape::clip(const std::vector<CPlane>	&pyramid, const CMatrix &worldMatrix)
{
	// Speed Clip: clip just the sphere.
	CBSphere	localSphere(_BBox.getCenter(), _BBox.getRadius());
	CBSphere	worldSphere;

	// transform the sphere in WorldMatrix (with nearly good scale info).
	localSphere.applyTransform(worldMatrix, worldSphere);

	// if out of only plane, entirely out.
	for(sint i=0;i<(sint)pyramid.size();i++)
	{
		// We are sure that pyramid has normalized plane normals.
		// if SpherMax OUT return false.
		float	d= pyramid[i]*worldSphere.Center;
		if(d>worldSphere.Radius)
			return false;
	}

	return true;
}


// ***************************************************************************
void		CSkeletonShape::getAABBox(NLMISC::CAABBox &bbox) const
{
	bbox= _BBox;
}


} // NL3D
