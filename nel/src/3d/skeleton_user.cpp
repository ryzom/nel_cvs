/** \file skeleton_user.cpp
 * <File description>
 *
 * $Id: skeleton_user.cpp,v 1.4 2002/03/21 16:07:51 berenguier Exp $
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

#include "3d/skeleton_user.h"


namespace NL3D
{


// ***************************************************************************
uint		CSkeletonUser::getNumBoneComputed() const
{
	return _Skeleton->getNumBoneComputed();
}

// ***************************************************************************
void		CSkeletonUser::setInterpolationDistance(float dist)
{
	_Skeleton->setInterpolationDistance(dist);
}

// ***************************************************************************
float		CSkeletonUser::getInterpolationDistance() const
{
	return _Skeleton->getInterpolationDistance();
}


} // NL3D
