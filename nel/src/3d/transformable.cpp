/** \file transformable.cpp
 * <File description>
 *
 * $Id: transformable.cpp,v 1.10 2001/04/09 15:07:15 berenguier Exp $
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

#include "nel/3d/transformable.h"
#include "nel/3d/channel_mixer.h"


namespace NL3D 
{


// ***************************************************************************
ITransformable::ITransformable()
{
	// Set number of animated values.
	IAnimatable::resize (AnimValueLast);

	// Deriver note: just copy this line in each ctor.

	// Init default values.
	_Mode= RotQuat;
	// matrix init to identity.
	_Pos.Value= CVector::Null;
	_RotEuler.Value= CVector::Null;
	_RotQuat.Value= CQuat::Identity;
	_Scale.Value= CVector(1,1,1);
	_Pivot.Value= CVector::Null;

	_LocalMatrixDate= 0;
}


// ***************************************************************************
IAnimatedValue*		ITransformable::getValue (uint valueId)
{
	// what value ?
	switch (valueId)
	{
	case PosValue:			return &_Pos;
	case RotEulerValue:		return &_RotEuler;
	case RotQuatValue:		return &_RotQuat;
	case ScaleValue:		return &_Scale;
	case PivotValue:		return &_Pivot;
	}

	// No, only ITrnasformable values!
	nlstop;
	// Deriver note: else call BaseClass::getValue(valueId);

	return NULL;
}
// ***************************************************************************
const char 	*ITransformable::getValueName (uint valueId) const
{
	// what value ?
	switch (valueId)
	{
	case PosValue:			return getPosValueName ();
	case RotEulerValue:		return getRotEulerValueName();
	case RotQuatValue:		return getRotQuatValueName();
	case ScaleValue:		return getScaleValueName();
	case PivotValue:		return getPivotValueName();
	}

	// No, only ITrnasformable values!
	nlstop;
	// Deriver note: else call BaseClass::getValueName(valueId);

	return "";
}

// ***************************************************************************
const char	*ITransformable::getPosValueName ()
{
	return "pos";
}
// ***************************************************************************
const char	*ITransformable::getRotEulerValueName()
{
	return "roteuler";
}
// ***************************************************************************
const char	*ITransformable::getRotQuatValueName()
{
	return "rotquat";
}
// ***************************************************************************
const char	*ITransformable::getScaleValueName()
{
	return "scale";
}
// ***************************************************************************
const char	*ITransformable::getPivotValueName()
{
	return "pivot";
}


// ***************************************************************************
void	ITransformable::clearTransformFlags() const
{
	ITransformable	*self= const_cast<ITransformable*>(this);

	// clear my falgs.
	self->clearFlag(PosValue);
	self->clearFlag(RotEulerValue); 
	self->clearFlag(RotQuatValue); 
	self->clearFlag(ScaleValue);
	self->clearFlag(PivotValue);

	// We are OK!
	self->clearFlag(OwnerBit);
}

// ***************************************************************************
bool	ITransformable::needCompute() const
{
	return  _Mode!=DirectMatrix && isTouched(OwnerBit);
}


// ***************************************************************************
void	ITransformable::updateMatrix() const
{
	// should we update?
	if(needCompute())
	{
		clearTransformFlags();
		// update scale date (so sons are informed of change).
		_LocalMatrixDate++;

		// update the matrix.
		_LocalMatrix.identity();

		// father scale will be herited.
		// T*P
		_LocalMatrix.translate(_Pos.Value+_Pivot.Value);

		// R*S*P-1.
		if(_Mode==RotEuler)
			_LocalMatrix.rotate(_RotEuler.Value, _RotOrder);
		else
			_LocalMatrix.rotate(_RotQuat.Value);
		_LocalMatrix.scale(_Scale.Value);
		_LocalMatrix.translate(-_Pivot.Value);
	}
}


// ***************************************************************************
void		ITransformable::lookAt (const CVector& eye, const CVector& target, float roll)
{
	nlassert(_Mode==RotQuat || _Mode==DirectMatrix);

	// Roll matrix
	CMatrix rollMT;
	rollMT.identity();
	if (roll!=0.f)
		rollMT.rotateY (roll);

	// Make the target base
	CVector j=target;
	j-=eye;
	j.normalize();
	CVector i=j^CVector (0,0,1.f);
	CVector k=i^j;
	k.normalize();
	i=j^k;
	i.normalize();

	// Make the target matrix
	CMatrix targetMT;
	targetMT.identity();
	targetMT.setRot (i, j, k);
	targetMT.setPos (eye);

	// Compose matrix
	targetMT*=rollMT;

	// Set the matrix
	if(_Mode==DirectMatrix)
		setMatrix (targetMT);
	else
	{
		// transfrom to quaternion mode.
		setScale(CVector(1,1,1));
		setPivot(CVector::Null);
		setPos(targetMT.getPos());
		setRotQuat(targetMT.getRot());
	}
}



// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// Fucking GCC link bug with abstract classes / inline functions.
#ifndef NL_OS_WINDOWS

const CMatrix	&ITransformable::getMatrix() const {updateMatrix(); return _LocalMatrix;}
bool			ITransformable::compareMatrixDate(uint64 callerDate) const
{
	return callerDate<_LocalMatrixDate || needCompute();
}
uint64			ITransformable::getMatrixDate() const
{
	updateMatrix();
	return _LocalMatrixDate;
}
void	ITransformable::setTransformMode(TTransformMode mode, CMatrix::TRotOrder ro)
{
	_Mode= mode;
	_RotOrder= ro;
	// just for information.
	touch(PosValue, OwnerBit);
}
void	ITransformable::setPos(const CVector &pos)
{
	nlassert(_Mode==RotEuler || _Mode==RotQuat);
	_Pos.Value= pos;
	touch(PosValue, OwnerBit);
}
void	ITransformable::setRotEuler(const CVector &rot)
{
	nlassert(_Mode==RotEuler);
	_RotEuler.Value= rot;
	touch(RotEulerValue, OwnerBit);
}
void	ITransformable::setRotQuat(const CQuat &quat)
{
	nlassert(_Mode==RotQuat);
	_RotQuat.Value= quat;
	touch(RotQuatValue, OwnerBit);
}
void	ITransformable::setScale(const CVector &scale)
{
	nlassert(_Mode==RotEuler || _Mode==RotQuat);
	_Scale.Value= scale;
	touch(ScaleValue, OwnerBit);
}
void	ITransformable::setPivot(const CVector &pivot)
{
	nlassert(_Mode==RotEuler || _Mode==RotQuat);
	_Pivot.Value= pivot;
	touch(PivotValue, OwnerBit);
}
void	ITransformable::setMatrix(const CMatrix &mat)
{
	nlassert(_Mode==DirectMatrix);
	_LocalMatrix= mat;
	// The matrix has changed.
	_LocalMatrixDate++;
}
ITransformable::TTransformMode	ITransformable::getTransformMode()
{
	return _Mode;
}
CMatrix::TRotOrder	ITransformable::getRotOrder()
{
	return _RotOrder;
}
void	ITransformable::getPos(CVector &pos)
{
	nlassert(_Mode==RotEuler || _Mode==RotQuat);
	pos= _Pos.Value;
}
void	ITransformable::getRotEuler(CVector &rot)
{
	nlassert(_Mode==RotEuler);
	rot= _RotEuler.Value;
}
void	ITransformable::getRotQuat(CQuat &quat)
{
	nlassert(_Mode==RotQuat);
	quat= _RotQuat.Value;
}
void	ITransformable::getScale(CVector &scale)
{
	nlassert(_Mode==RotEuler || _Mode==RotQuat);
	scale= _Scale.Value;
}
void	ITransformable::getPivot(CVector &pivot)
{
	nlassert(_Mode==RotEuler || _Mode==RotQuat);
	pivot= _Pivot.Value;
}
CVector	ITransformable::getPos()
{
	nlassert(_Mode==RotEuler || _Mode==RotQuat);
	return _Pos.Value;
}
CVector	ITransformable::getRotEuler()
{
	nlassert(_Mode==RotEuler);
	return _RotEuler.Value;
}
CQuat	ITransformable::getRotQuat()
{
	nlassert(_Mode==RotQuat);
	return _RotQuat.Value;
}
CVector	ITransformable::getScale()
{
	nlassert(_Mode==RotEuler || _Mode==RotQuat);
	return _Scale.Value;
}
CVector ITransformable::getPivot()
{
	nlassert(_Mode==RotEuler || _Mode==RotQuat);
	return _Pivot.Value;
}


#endif	// NL_OS_WINDOWS


} // NL3D
