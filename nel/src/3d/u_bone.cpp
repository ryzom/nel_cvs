/** \file bone_user.cpp
 * User interface for bones.
 *
 * $Id: u_bone.cpp,v 1.2 2004/05/12 12:51:49 corvazier Exp $
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

#include "3d/bone.h"
#include "nel/3d/u_bone.h"

#define NL3D_MEM_BONES						NL_ALLOC_CONTEXT( 3dBones )

namespace NL3D
{

const CMatrix	&UBone::getLastWorldMatrixComputed() const
{
	NL3D_MEM_BONES
	CBone *object = getObjectPtr();
	return object->getWorldMatrix();
}


} // NL3D