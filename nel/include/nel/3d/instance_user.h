/** \file instance_user.h
 * <File description>
 *
 * $Id: instance_user.h,v 1.1 2001/02/28 14:42:29 berenguier Exp $
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
#include "nel/3d/scene.h"
#include "nel/3d/transform_user.h"
#include "nel/3d/transform.h"
#include "nel/3d/transform_shape.h"


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

	/// \name Object
	// @{
	/// This model should have been created with Scene::createInstance().
	CInstanceUser(CScene *scene, IModel *trans) : 
	  CTransformUser(scene, trans)
	{
	}
	virtual	~CInstanceUser()
	{
		// deleted in CTransformUser.
	}
	// @}

};


} // NL3D


#endif // NL_INSTANCE_USER_H

/* End of instance_user.h */
