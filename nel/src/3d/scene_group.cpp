/** \file scene_group.cpp
 * <File description>
 *
 * $Id: scene_group.cpp,v 1.3 2001/05/22 08:37:22 corvazier Exp $
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

#include "nel/3d/scene_group.h"
#include "nel/misc/stream.h"
#include "nel/3d/scene.h"
#include "nel/3d/transform_shape.h"

using namespace NLMISC;

namespace NL3D 
{


// ***************************************************************************

uint CInstanceGroup::getNumInstance () const
{
	return _InstancesInfos.size();
}

// ***************************************************************************

const std::string& CInstanceGroup::getInstanceName (uint instanceNb) const
{
	// Return the name of the n-th instance
	return _InstancesInfos[instanceNb].Name;
}

// ***************************************************************************

const CVector& CInstanceGroup::getInstancePos (uint instanceNb) const
{
	// Return the position vector of the n-th instance
	return _InstancesInfos[instanceNb].Pos;
}

// ***************************************************************************

const CQuat& CInstanceGroup::getInstanceRot (uint instanceNb) const
{
	// Return the rotation vector of the n-th instance
	return _InstancesInfos[instanceNb].Rot;
}

// ***************************************************************************

const CVector& CInstanceGroup::getInstanceScale (uint instanceNb) const
{
	// Return the scale vector of the n-th instance
	return _InstancesInfos[instanceNb].Scale;
}

// ***************************************************************************

const int CInstanceGroup::getInstanceParent (uint instanceNb) const
{
	// Return the scale vector of the n-th instance
	return _InstancesInfos[instanceNb].nParent;
}

// ***************************************************************************

void CInstanceGroup::build (const TInstanceArray& array)
{
	// Copy the array
	_InstancesInfos=array;
}

// ***************************************************************************

void CInstanceGroup::serial (NLMISC::IStream& f)
{
	// Serial a header
	f.serialCheck ((uint32)'TPRG');

	// Serial a version number
	sint version=f.serialVersion (0);

	// Serial the array
	f.serialCont (_InstancesInfos);
}

// ***************************************************************************

void CInstanceGroup::CInstance::serial (NLMISC::IStream& f)
{
	// Serial a version number
	sint version=f.serialVersion (0);

	// Serial the name
	f.serial (Name);

	// Serial the position vector
	f.serial (Pos);

	// Serial the rotation vector
	f.serial (Rot);

	// Serial the scale vector
	f.serial (Scale);

	// Serial the parent location in the vector (-1 if no parent)
	f.serial (nParent);
}

// ***************************************************************************

bool CInstanceGroup::addToScene (CScene& scene)
{
	int i;

	_Instances.resize( _InstancesInfos.size() );

	std::vector<CInstance>::iterator it = _InstancesInfos.begin();
	for( i=0; i<(sint)_InstancesInfos.size(); ++i,++it )
	{
		CInstance &rInstanceInfo = *it;

		// Creation and positionning of the new instance
		_Instances[i] = scene.createInstance ( rInstanceInfo.Name + ".shape" );
		if (_Instances[i])
		{
			_Instances[i]->setPos( rInstanceInfo.Pos );
			_Instances[i]->setRotQuat( rInstanceInfo.Rot );
			_Instances[i]->setScale( rInstanceInfo.Scale );
		}
	}

	// Setup the hierarchy
	// We just have to set the traversal HRC (Hierarchy)

	ITrav *pTrav = scene.getTrav( HrcTravId );

	it = _InstancesInfos.begin();
	for( i=0; i<(sint)_InstancesInfos.size(); ++i,++it )
	{
		CInstance &rInstanceInfo = *it;
		if( rInstanceInfo.nParent != -1 ) // Is the instance get a parent
			pTrav->link( _Instances[rInstanceInfo.nParent], _Instances[i] );
	}

	return true;
}

// ***************************************************************************

bool CInstanceGroup::removeFromScene (CScene& scene)
{
	std::vector<CTransformShape*>::iterator it = _Instances.begin();
	for( int i=0; i<(sint)_InstancesInfos.size(); ++i,++it )
	{
		CTransformShape *pTShape = *it;

		scene.deleteInstance( pTShape );
	}
	return true;
}

} // NL3D
