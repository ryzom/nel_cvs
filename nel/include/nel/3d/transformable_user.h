/** \file transformable_user.h
 * <File description>
 *
 * $Id: transformable_user.h,v 1.1 2001/04/13 16:39:03 berenguier Exp $
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

#ifndef NL_TRANSFORMABLE_USER_H
#define NL_TRANSFORMABLE_USER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/tmp/u_transformable.h"
#include "nel/3d/transformable.h"


namespace NL3D
{


// ***************************************************************************
/**
 * UTransformable implementation.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CTransformableUser : virtual public UTransformable
{
protected:
	// The object.
	ITransformable			*_Transformable;

public:


	/// \name Object
	// @{
	/// Give a ITransformable. CTransformableUser DO NOT OWNS IT!
	CTransformableUser(ITransformable *trans)
	{
		nlassert(trans);
		_Transformable= trans;

		// Same enums!!
		nlassert((uint)UTransformable::TransformModeCount == (uint)ITransformable::TransformModeCount);
	}
	virtual	~CTransformableUser()
	{
		_Transformable= NULL;
	}
	// @}

	/// \name Space manipulation
	// @{
	virtual	void			setMatrix(const CMatrix &mat)
	{
		_Transformable->setMatrix(mat);
	}
	virtual	const CMatrix	&getMatrix() const	
	{
		return _Transformable->getMatrix();
	}
	// @}


	/// \name space set/get.
	// @{

	virtual	void			setTransformMode(TTransformMode mode, CMatrix::TRotOrder ro= CMatrix::ZXY)
	{
		_Transformable->setTransformMode((ITransformable::TTransformMode)(uint)mode, ro);
	}
	virtual	void			setPos(const CVector &pos)
	{
		_Transformable->setPos(pos);
	}
	virtual	void			setRotEuler(const CVector &rot)
	{
		_Transformable->setRotEuler(rot);
	}
	virtual	void			setRotQuat(const CQuat &quat)
	{
		_Transformable->setRotQuat(quat);
	}
	virtual	void			setRotQuat(const CVector &jdir)
	{
		CMatrix	mat;
		mat.setRot(CVector::I, jdir, CVector::K);
		mat.normalize(CMatrix::YZX);
		setRotQuat(mat.getRot());
	}
	virtual	void			setRotQuat(const CVector &jdir, const CVector &vup)
	{
		CMatrix	mat;
		mat.setRot(CVector::I, jdir, vup);
		mat.normalize(CMatrix::YZX);
		setRotQuat(mat.getRot());
	}
	virtual	void			setScale(const CVector &scale)
	{
		_Transformable->setScale(scale);
	}
	virtual	void			setPivot(const CVector &pivot)
	{
		_Transformable->setPivot(pivot);
	}

	virtual	TTransformMode		getTransformMode()
	{
		return (TTransformMode)(uint)_Transformable->getTransformMode();
	}
	virtual	CMatrix::TRotOrder	getRotOrder()
	{
		return _Transformable->getRotOrder();
	}

	virtual	void			getPos(CVector &pos)
	{
		_Transformable->getPos(pos);
	}
	virtual	void			getRotEuler(CVector &rot)
	{
		_Transformable->getRotEuler(rot);
	}
	virtual	void			getRotQuat(CQuat &quat)
	{
		_Transformable->getRotQuat(quat);
	}
	virtual	void			getScale(CVector &scale)
	{
		_Transformable->getScale(scale);
	}
	virtual	void			getPivot(CVector &pivot)
	{
		_Transformable->getPivot(pivot);
	}

	virtual	CVector			getPos()
	{
		return _Transformable->getPos();
	}
	virtual	CVector			getRotEuler()
	{
		return _Transformable->getRotEuler();
	}
	virtual	CQuat			getRotQuat()
	{
		return _Transformable->getRotQuat();
	}
	virtual	CVector			getScale()
	{
		return _Transformable->getScale();
	}
	virtual	CVector			getPivot()
	{
		return _Transformable->getPivot();
	}

	// @}


	/// \name Misc
	// @{
	virtual	void			lookAt (const CVector& eye, const CVector& target, float roll=0.f)
	{
		_Transformable->lookAt(eye, target, roll);
	}
	// @}

};


} // NL3D


#endif // NL_TRANSFORMABLE_USER_H

/* End of transformable_user.h */
