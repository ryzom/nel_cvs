/** \file bone_user.h
 * <File description>
 *
 * $Id: bone_user.h,v 1.3 2002/10/28 17:32:13 corvazier Exp $
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

#ifndef NL_BONE_USER_H
#define NL_BONE_USER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/u_bone.h"
#include "3d/bone.h"
#include "3d/transformable_user.h"

#define NL3D_MEM_BONES						NL_ALLOC_CONTEXT( 3dBones )

namespace NL3D {


// ***************************************************************************
/**
 * UTransform implementation.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CBoneUser : virtual public UBone, public CTransformableUser
{
protected:

public:
	
	/// \name Object
	// @{
	/// Give a CBone. CBoneUser DO NOT owns it.
	CBoneUser(CBone *bone) : CTransformableUser( static_cast<ITransformable*>(bone) )
	{
		// do nothing.
		NL3D_MEM_BONES
	}
	virtual	~CBoneUser()
	{
		// do nothing.
		NL3D_MEM_BONES
	}
	// @}

	/// \name Special feature
	// @{

	virtual	const CMatrix	&getLastWorldMatrixComputed() const
	{
		NL3D_MEM_BONES

		return static_cast<CBone*>(_Transformable)->getWorldMatrix();
	}

	// @}

};


} // NL3D


#endif // NL_BONE_USER_H

/* End of bone_user.h */
