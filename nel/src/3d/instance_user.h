/** \file instance_user.h
 * <File description>
 *
 * $Id: instance_user.h,v 1.9 2002/04/26 15:05:00 berenguier Exp $
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

#ifndef NL_INSTANCE_USER_H
#define NL_INSTANCE_USER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/u_instance.h"
#include "3d/scene.h"
#include "3d/transform_user.h"
#include "3d/transform.h"
#include "3d/transform_shape.h"
#include "3d/mesh_base_instance.h"
#include "3d/instance_material_user.h"


namespace NL3D {


// ***************************************************************************
/**
 * UInstance implementation 
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CInstanceUser : virtual public UInstance, public CTransformUser
{
public:
	/// This is the SAME pointer than _Transform, but correctly casted.
	CTransformShape			*_Instance;

	/// This is a mirror of _Instance->Materials
	std::vector<CInstanceMaterialUser>	_Materials;


public:

	/// \name Object
	// @{
	/// This model should have been created with Scene::createInstance().
	CInstanceUser(CScene *scene, IModel *trans);
	virtual	~CInstanceUser()
	{
		// deleted in CTransformUser.
		_Instance= NULL;
		// user mats are auto deleted.
	}
	// @}


	virtual void				getShapeAABBox(NLMISC::CAABBox &bbox) const;

	virtual void				setBlendShapeFactor (const std::string &blendShapeName, float factor, bool dynamic);


	/// \name Material access.
	// @{
	virtual	uint		getNumMaterials() const
	{
		return _Materials.size();
	}
	virtual	UInstanceMaterial		&getMaterial(uint materialId)
	{
		if(materialId>=_Materials.size())
			nlerror("getMaterial(): bad materialId");
		return dynamic_cast<UInstanceMaterial&>(_Materials[materialId]);
	}

	virtual void selectTextureSet(uint id)
	{
		CMeshBaseInstance *mbi  = NLMISC::safe_cast<CMeshBaseInstance *>(_Instance);
		mbi->selectTextureSet(id);
	}
	// @}


	virtual void		changeMRMDistanceSetup(float distanceFinest, float distanceMiddle, float distanceCoarsest);
	virtual void		setShapeDistMax(float distMax);
	virtual float		getShapeDistMax() const;

};


} // NL3D


#endif // NL_INSTANCE_USER_H

/* End of instance_user.h */
