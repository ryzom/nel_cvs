/** \file u_bone.h
 * <File description>
 *
 * $Id: u_bone.h,v 1.4 2002/03/28 15:06:40 berenguier Exp $
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
#include "nel/3d/u_transformable.h"


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


	/// \name Special feature
	// @{

	/** get the last world matrix computed in last render().
	 *	NB: this WM is computed in last render() only if the object was not clipped. So use it wisely.
	 *
	 *	NB: this WM may also not be computed in last render() for "Lod skeleton" reason. ie if the skeleton
	 *	is too far, the engine may not compute a bone (for speed). To avoid problem, you should ask the artist what
	 *	bones have such a scheme, or you could stickObject() a dummy (eg a UTransform) onto this bone, because 
	 *	in this case, this bone will always be computed.
	 *	
	 */
	virtual	const CMatrix	&getLastWorldMatrixComputed() const =0;

	// @}


};


} // NL3D


#endif // NL_U_BONE_H

/* End of u_bone.h */
