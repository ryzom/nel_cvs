/** \file transformable_user.h
 * <File description>
 *
 * $Id: transformable_user.h,v 1.2 2002/10/28 17:32:13 corvazier Exp $
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
#include "nel/3d/u_transformable.h"
#include "3d/transformable.h"

#define NL3D_MEM_TRANSFORMABLE						NL_ALLOC_CONTEXT( 3dTrabl )

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
		NL3D_MEM_TRANSFORMABLE
		nlassert(trans);
		_Transformable= trans;

		// Same enums!!
		nlassert((uint)UTransformable::TransformModeCount == (uint)ITransformable::TransformModeCount);
	}
	virtual	~CTransformableUser()
	{
		NL3D_MEM_TRANSFORMABLE
		_Transformable= NULL;
	}
	// @}

	/// \name Space manipulation
	// @{
	virtual	void			setMatrix(const CMatrix &mat)
	{
		NL3D_MEM_TRANSFORMABLE
		_Transformable->setMatrix(mat);
	}
	virtual	const CMatrix	&getMatrix() const	
	{
		NL3D_MEM_TRANSFORMABLE
		return _Transformable->getMatrix();
	}
	// @}


	/// \name space set/get.
	// @{

	virtual	void			setTransformMode(TTransformMode mode, CMatrix::TRotOrder ro= CMatrix::ZXY)
	{
		NL3D_MEM_TRANSFORMABLE
		_Transformable->setTransformMode((ITransformable::TTransformMode)(uint)mode, ro);
	}
	virtual	void			setPos(const CVector &pos)
	{
		NL3D_MEM_TRANSFORMABLE
		_Transformable->setPos(pos);
	}
	virtual	void			setRotEuler(const CVector &rot)
	{
		NL3D_MEM_TRANSFORMABLE
		_Transformable->setRotEuler(rot);
	}
	virtual	void			setRotQuat(const CQuat &quat)
	{
		NL3D_MEM_TRANSFORMABLE
		_Transformable->setRotQuat(quat);
	}
	virtual	void			setRotQuat(const CVector &jdir)
	{
		NL3D_MEM_TRANSFORMABLE
		CMatrix	mat;
		mat.setRot(CVector::I, jdir, CVector::K);
		mat.normalize(CMatrix::YZX);
		setRotQuat(mat.getRot());
	}
	virtual	void			setRotQuat(const CVector &jdir, const CVector &vup)
	{
		NL3D_MEM_TRANSFORMABLE
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
		NL3D_MEM_TRANSFORMABLE
		_Transformable->setPivot(pivot);
	}

	virtual	TTransformMode		getTransformMode()
	{
		NL3D_MEM_TRANSFORMABLE
		return (TTransformMode)(uint)_Transformable->getTransformMode();
	}
	virtual	CMatrix::TRotOrder	getRotOrder()
	{
		NL3D_MEM_TRANSFORMABLE
		return _Transformable->getRotOrder();
	}

	virtual	void			getPos(CVector &pos)
	{
		NL3D_MEM_TRANSFORMABLE
		_Transformable->getPos(pos);
	}
	virtual	void			getRotEuler(CVector &rot)
	{
		NL3D_MEM_TRANSFORMABLE
		_Transformable->getRotEuler(rot);
	}
	virtual	void			getRotQuat(CQuat &quat)
	{
		NL3D_MEM_TRANSFORMABLE
		_Transformable->getRotQuat(quat);
	}
	virtual	void			getScale(CVector &scale)
	{
		NL3D_MEM_TRANSFORMABLE
		_Transformable->getScale(scale);
	}
	virtual	void			getPivot(CVector &pivot)
	{
		NL3D_MEM_TRANSFORMABLE
		_Transformable->getPivot(pivot);
	}

	virtual	CVector			getPos()
	{
		NL3D_MEM_TRANSFORMABLE
		return _Transformable->getPos();
	}
	virtual	CVector			getRotEuler()
	{
		NL3D_MEM_TRANSFORMABLE
		return _Transformable->getRotEuler();
	}
	virtual	CQuat			getRotQuat()
	{
		NL3D_MEM_TRANSFORMABLE
		return _Transformable->getRotQuat();
	}
	virtual	CVector			getScale()
	{
		NL3D_MEM_TRANSFORMABLE
		return _Transformable->getScale();
	}
	virtual	CVector			getPivot()
	{
		NL3D_MEM_TRANSFORMABLE
		return _Transformable->getPivot();
	}

	// @}


	/// \name Misc
	// @{
	virtual	void			lookAt (const CVector& eye, const CVector& target, float roll=0.f)
	{
		NL3D_MEM_TRANSFORMABLE
		_Transformable->lookAt(eye, target, roll);
	}
	// @}

};


} // NL3D


#endif // NL_TRANSFORMABLE_USER_H

/* End of transformable_user.h */
