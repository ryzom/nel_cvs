/** \file u_transform.h
 * <File description>
 *
 * $Id: u_transform.h,v 1.3 2001/03/23 10:09:03 berenguier Exp $
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
#include "nel/misc/quat.h"


namespace NL3D 
{

using NLMISC::CVector;
using NLMISC::CMatrix;
using NLMISC::CQuat;

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
	/// Constructor. By default, DirectMatrix mode.
	UTransform() {}
	virtual	~UTransform() {}
	// @}

public:
	// Enum should be the same than in CHrcTrav, and ITransformable.

	/// The visibility flag. In the root case, Herit means Show.
	enum	TVisibility
	{
		Show=0,		// The model is shown in the hierarchy
		Hide,		// The model is hidden in the hierarchy
		Herit,		// The model herit the visibilty from his father

		VisibilityCount
	};

	// Matrix mode.
	enum	TTransformMode
	{
		DirectMatrix=0,		// DirectMatrixMode (default).
		RotEuler,			// Matrix is computed from sperated composantes, with euler rotation.
		RotQuat,			// Matrix is computed from sperated composantes, with quat rotation.

		TransformModeCount
	};


public:


	/// \name Position set
	// @{
	/// Change the transform mode. Components or matrix are not reseted.
	virtual	void			setTransformMode(TTransformMode mode, CMatrix::TRotOrder ro= CMatrix::ZXY)=0;
	/// Work only in Rot* mode(nlassert).
	virtual	void			setPos(const CVector &pos)=0;
	/// Work only in RotEuler mode(nlassert).
	virtual	void			setRotEuler(const CVector &rot)=0;
	/// Work only in RotQuat mode (nlassert).
	virtual	void			setRotQuat(const CQuat &quat)=0;
	/** Work only in RotQuat mode (nlassert). 
	 * Build a quaternion from a forward direction (a J vector). there is no roll... jdir do not need to be noramlized.
	 */
	virtual	void			setRotQuat(const CVector &jdir)=0;
	/** Work only in RotQuat mode (nlassert). 
	 * Build a quaternion from a forward direction (a J vector). the roll is determined with help of the vector up vup... vectors do not need to be noramlized.
	 */
	virtual	void			setRotQuat(const CVector &jdir, const CVector &vup)=0;
	/// Work only in Rot* mode (nlassert).
	virtual	void			setScale(const CVector &scale)=0;
	/// Work only in Rot* mode (nlassert).
	virtual	void			setPivot(const CVector &pivot)=0;

	/// Work only in DirecTMatrix mode (nlassert).
	virtual	void			setMatrix(const CMatrix &mat)=0;
	// @}


	/// \name Position get
	// @{

	/// get the current transform mode.
	virtual	TTransformMode		getTransformMode()=0;
	/// get the current rotorder (information vlaid only when RotEuler mode).
	virtual	CMatrix::TRotOrder	getRotOrder()=0;

	/// Get the matrix, compute her if necessary (work in all modes).
	virtual	const CMatrix	&getMatrix() const	=0;

	/// Work only in Rot* mode(nlassert).
	virtual	void			getPos(CVector &pos)=0;
	/// Work only in RotEuler mode(nlassert).
	virtual	void			getRotEuler(CVector &rot)=0;
	/// Work only in RotQuat mode (nlassert).
	virtual	void			getRotQuat(CQuat &quat)=0;
	/// Work only in Rot* mode (nlassert).
	virtual	void			getScale(CVector &scale)=0;
	/// Work only in Rot* mode (nlassert).
	virtual	void			getPivot(CVector &pivot)=0;

	/// Work only in Rot* mode(nlassert).
	virtual	CVector			getPos()=0;
	/// Work only in RotEuler mode(nlassert).
	virtual	CVector			getRotEuler()=0;
	/// Work only in RotQuat mode (nlassert).
	virtual	CQuat			getRotQuat()=0;
	/// Work only in Rot* mode (nlassert).
	virtual	CVector			getScale()=0;
	/// Work only in Rot* mode (nlassert).
	virtual	CVector			getPivot()=0;
	// @}


	/// \name Hierarchy manipulation
	// @{
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
	  * Setup Matrix by the lookAt method. Work only in DirectMatrix mode and RotQuat mode (not euler...).
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
