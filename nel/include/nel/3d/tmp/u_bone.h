/** \file u_bone.h
 * <File description>
 *
 * $Id: u_bone.h,v 1.1 2001/04/13 16:39:55 berenguier Exp $
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

#ifndef NL_U_BONE_H
#define NL_U_BONE_H

#include "nel/misc/types_nl.h"
#include "nel/3d/tmp/u_transformable.h"


namespace NL3D 
{

// ***************************************************************************
/**
 * Base interface for manipulating Bones.
 * see UTransformable. A bone comes from a USkeleton.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class UBone : virtual public UTransformable
{
protected:

	/// \name Object
	// @{
	/// Constructor. By default, RotQuat mode.
	UBone() {}
	virtual	~UBone() {}
	// @}

public:


};


} // NL3D


#endif // NL_U_BONE_H

/* End of u_bone.h */
