/** \file u_transform.h
 * <File description>
 *
 * $Id: u_transform.h,v 1.1 2001/02/28 14:01:33 berenguier Exp $
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

#ifndef NL_U_TRANSFORM_H
#define NL_U_TRANSFORM_H

#include "nel/misc/types_nl.h"
#include "nel/misc/matrix.h"


namespace NL3D 
{

using NLMISC::CVector;
using NLMISC::CMatrix;

// ***************************************************************************
/**
 * Base interface for manipulating Movable Objects: camera, lights, instances etc...
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class UTransform
{
protected:

	/// \name Object
	// @{
	UTransform() {}
	virtual	~UTransform() {}
	// @}

public:
	// Enum should be the same than in CHrcTrav.

	/// The visibility flag. In the root case, Herit means Show.
	enum	TVisibility
	{
		Show=0,		// The model is shown in the hierarchy
		Hide,		// The model is hidden in the hierarchy
		Herit,		// The model herit the visibilty from his father

		VisibilityCount
	};

public:


	/// \name Space manipulation
	// @{
	virtual	void			setMatrix(const CMatrix &mat)=0;
	virtual	const CMatrix	&getMatrix() const	=0;
	/** Hierarchy edit. unlink this from oldparent, and make this be a son of newFather.
	 * if this was already a son of newFather, no-op.
	 * \param newFather the new Father. If NULL, the transform will be linked to the root of the hierarchy (Default!).
	 */
	virtual	void			parent(UTransform *newFather)=0;
	// @}


	/// \name visibility
	// @{
	/// Hide the object and his sons.
	virtual	void			hide()=0;
	/// Show the objet and his sons.
	virtual	void			show()=0;
	/// herit the visibility from his father. (default behavior).
	virtual	void			heritVisibility()=0;
	/// Get the local visibility state.
	virtual	TVisibility		getVisibility()=0;
	// @}


	/// \name Misc
	// @{
	/** 
	  * Setup Matrix by the lookAt method.
	  * 
	  * \param eye is the coordinate of the object.
	  * \param target is the point the object look at.
	  * \param roll is the roll angle in radian along the object's Y axis.
	  */
	virtual	void			lookAt (const CVector& eye, const CVector& target, float roll=0.f) =0;
	// @}


};


} // NL3D


#endif // NL_U_TRANSFORM_H

/* End of u_transform.h */
