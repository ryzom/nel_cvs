/** \file instance_user.cpp
 * <File description>
 *
 * $Id: instance_user.cpp,v 1.14 2002/10/28 17:32:13 corvazier Exp $
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

#include "3d/instance_user.h"
#include "nel/misc/debug.h"


using	namespace NLMISC;

namespace NL3D 
{



// ***************************************************************************
CInstanceUser::CInstanceUser(CScene *scene, IModel *trans) : 
  CTransformUser(scene, trans)
{
	NL3D_MEM_INSTANCE
	_Instance= safe_cast<CTransformShape*>(_Transform);
	CMeshBaseInstance	*mi= dynamic_cast<CMeshBaseInstance*>(_Instance);

	// If It it is a CMeshBaseInstance (with materials).
	if(mi)
	{
		// create user mats.
		uint	numMat= mi->Materials.size();
		_Materials.reserve(numMat);
		for(uint i=0;i<numMat;i++)
		{
			_Materials.push_back(CInstanceMaterialUser(mi, &mi->Materials[i], &mi->AsyncTextures[i]));
		}
	}
}


// ***************************************************************************
void				CInstanceUser::getShapeAABBox(NLMISC::CAABBox &bbox) const
{
	NL3D_MEM_INSTANCE
	_Instance->getAABBox(bbox);
}

// ***************************************************************************
void CInstanceUser::setBlendShapeFactor (const std::string &blendShapeName, float factor, bool dynamic)
{
	NL3D_MEM_INSTANCE
	CMeshBaseInstance	*mi= dynamic_cast<CMeshBaseInstance*>(_Instance);

	if (mi)
	{
		mi->setBlendShapeFactor (blendShapeName, factor);
		// \todo trap takes care of the dynamic flag
	}
}

// ***************************************************************************
void		CInstanceUser::changeMRMDistanceSetup(float distanceFinest, float distanceMiddle, float distanceCoarsest)
{
	NL3D_MEM_INSTANCE
	CMeshBaseInstance	*mi= dynamic_cast<CMeshBaseInstance*>(_Instance);

	// Just for CMeshBaseInstance.
	if (mi)
	{
		mi->changeMRMDistanceSetup(distanceFinest, distanceMiddle, distanceCoarsest);
	}
}


// ***************************************************************************
void		CInstanceUser::setShapeDistMax(float distMax)
{
	NL3D_MEM_INSTANCE
	if(_Instance && _Instance->Shape)
	{
		_Instance->Shape->setDistMax(distMax);
	}
}

// ***************************************************************************
float		CInstanceUser::getShapeDistMax() const
{
	NL3D_MEM_INSTANCE
	if(_Instance && _Instance->Shape)
	{
		return _Instance->Shape->getDistMax();
	}
	else
		return -1;
}


// ***************************************************************************
void		CInstanceUser::enableAsyncTextureMode(bool enable) 
{
	nlassert(_Instance->isMeshBaseInstance());
	CMeshBaseInstance *mbi  = static_cast<CMeshBaseInstance *>(_Instance);
	mbi->enableAsyncTextureMode(enable) ;
}
// ***************************************************************************
bool		CInstanceUser::getAsyncTextureMode() const 
{
	nlassert(_Instance->isMeshBaseInstance());
	CMeshBaseInstance *mbi  = static_cast<CMeshBaseInstance *>(_Instance);
	return mbi->getAsyncTextureMode() ;
}
// ***************************************************************************
void		CInstanceUser::startAsyncTextureLoading() 
{
	nlassert(_Instance->isMeshBaseInstance());
	CMeshBaseInstance *mbi  = static_cast<CMeshBaseInstance *>(_Instance);
	mbi->startAsyncTextureLoading();
}
// ***************************************************************************
bool		CInstanceUser::isAsyncTextureReady() 
{
	nlassert(_Instance->isMeshBaseInstance());
	CMeshBaseInstance *mbi  = static_cast<CMeshBaseInstance *>(_Instance);
	return mbi->isAsyncTextureReady();
}
// ***************************************************************************
void		CInstanceUser::setAsyncTextureDistance(float dist)
{
	nlassert(_Instance->isMeshBaseInstance());
	CMeshBaseInstance *mbi  = static_cast<CMeshBaseInstance *>(_Instance);
	mbi->setAsyncTextureDistance(dist);
}
// ***************************************************************************
float		CInstanceUser::getAsyncTextureDistance() const
{
	nlassert(_Instance->isMeshBaseInstance());
	CMeshBaseInstance *mbi  = static_cast<CMeshBaseInstance *>(_Instance);
	return mbi->getAsyncTextureDistance();
}


} // NL3D
