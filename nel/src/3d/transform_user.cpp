/** \file transform_user.cpp
 * <File description>
 *
 * $Id: transform_user.cpp,v 1.13 2003/03/26 10:20:55 berenguier Exp $
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

#include "3d/transform_user.h"
#include "3d/instance_group_user.h"
#include "scene_group.h"


namespace NL3D
{

// ***************************************************************************
void CTransformUser::setClusterSystem (UInstanceGroup *pIG)
{
	nlassert(_Transform) ; // object invalid now ...
	if ((pIG == NULL) || (pIG == (UInstanceGroup*)-1))
	{
		if (pIG == NULL)
			_Transform->setClusterSystem (NULL);
		else
			_Transform->setClusterSystem ((CInstanceGroup*)-1);
	}
	else
		_Transform->setClusterSystem (&((CInstanceGroupUser*)pIG)->_InstanceGroup);
	_pIG = pIG;
}

// ***************************************************************************
UInstanceGroup *CTransformUser::getClusterSystem ()
{
	return _pIG;
}

// ***************************************************************************
void			CTransformUser::getLastParentClusters(std::vector<CCluster*> &clusters) const
{
	// look in the list of parent of the transform object and extract the CCluster parents
	if (_Scene == NULL)
		return;

	CClipTrav	&clipTrav= _Scene->getClipTrav();

	uint	num= _Transform->clipGetNumParents();
	for(uint i=0;i<num;i++)
	{
		CCluster *pcluster = dynamic_cast<CCluster*>(_Transform->clipGetParent(i));
		if (pcluster != NULL)
			clusters.push_back(pcluster);
	}

	// If the object is link to a QuadCluster, add the RootCluster to the list
	CTransformShape	*trShp= dynamic_cast<CTransformShape*>( _Transform );
	if( trShp && trShp->isLinkToQuadCluster() )
		clusters.push_back(clipTrav.RootCluster);
}


// ***************************************************************************
void			CTransformUser::freezeHRC()
{
	nlassert(_Transform) ; // object invalid now ...
	_Transform->freezeHRC();
}

// ***************************************************************************
void			CTransformUser::unfreezeHRC()
{
	nlassert(_Transform) ; // object invalid now ...
	_Transform->unfreezeHRC();
}


// ***************************************************************************
void			CTransformUser::setLoadBalancingGroup(const std::string &group)
{
	_Transform->setLoadBalancingGroup(group);
}
// ***************************************************************************
const std::string	&CTransformUser::getLoadBalancingGroup() const
{
	return _Transform->getLoadBalancingGroup();
}

// ***************************************************************************
void			CTransformUser::setMeanColor(NLMISC::CRGBA color)
{
	_Transform->setMeanColor(color);
}
// ***************************************************************************
NLMISC::CRGBA	CTransformUser::getMeanColor() const
{
	return _Transform->getMeanColor();
}

// ***************************************************************************
const CMatrix	&CTransformUser::getLastWorldMatrixComputed() const
{
	return _Transform->getWorldMatrix();
}

} // NL3D
