/** \file transform.cpp
 * <File description>
 *
 * $Id: transform.cpp,v 1.10 2001/02/28 14:28:57 berenguier Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
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

#include "nel/3d/transform.h"


namespace	NL3D
{

#define NL3D_CTRANSFORM_VALUE_COUNT 5

// ***************************************************************************
static	IObs	*creatorHrcObs() {return new CTransformHrcObs;}
static	IObs	*creatorClipObs() {return new CTransformClipObs;}

void	CTransform::registerBasic()
{
	CMOT::registerModel(TransformId, 0, CTransform::creator);
	CMOT::registerObs(HrcTravId, TransformId, creatorHrcObs);
	CMOT::registerObs(ClipTravId, TransformId, creatorClipObs);
}


// ***************************************************************************
CTransform::CTransform()
{
	Touch.resize(Last);

	LocalMatrix.identity();
	Visibility= CHrcTrav::Herit;

	// Set number of animated values
	IAnimatable::resize (NL3D_CTRANSFORM_VALUE_COUNT);

	// Set default animation tracks to NULL
	_PosDefault=NULL;
	_RotEulerDefault=NULL;
	_RotQuatDefault=NULL;
	_ScaleDefault=NULL;
	_PivotDefault=NULL;
}
// ***************************************************************************
void		CTransform::setMatrix(const CMatrix &mat)
{
	LocalMatrix= mat;
	foul();
}
// ***************************************************************************
void		CTransform::hide()
{
	// Optim: do nothing if already set (=> not foul() -ed).
	if(Visibility!= CHrcTrav::Hide)
	{
		foul();
		Visibility= CHrcTrav::Hide;
	}
}
// ***************************************************************************
void		CTransform::show()
{
	// Optim: do nothing if already set (=> not foul() -ed).
	if(Visibility!= CHrcTrav::Show)
	{
		foul();
		Visibility= CHrcTrav::Show;
	}
}
// ***************************************************************************
void		CTransform::heritVisibility()
{
	// Optim: do nothing if already set (=> not foul() -ed).
	if(Visibility!= CHrcTrav::Herit)
	{
		foul();
		Visibility= CHrcTrav::Herit;
	}
}


// ***************************************************************************
void		CTransform::lookAt (const CVector& eye, const CVector& target, float roll)
{
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
	setMatrix (targetMT);
}


// ***************************************************************************
uint		CTransform::getValueCount () const
{
	/* 
	 * 4 values,
	 0: translation
	 1: rotation euler
	 2: rotation quaternion
	 3: scale
	 4: pivot
	 */
	return NL3D_CTRANSFORM_VALUE_COUNT;
}
// ***************************************************************************
IAnimatedValue*		CTransform::getValue (uint valueId)
{
	// Only value of the transform
	nlassert (valueId<NL3D_CTRANSFORM_VALUE_COUNT);

	// what value ?
	switch (valueId)
	{
	case 0:
		return &_Pos;
	case 1:
		return &_RotEuler;
	case 2:
		return &_RotQuat;
	case 3:
		return &_Scale;
	case 4:
		return &_Pivot;
	}

	// No, only NL3D_CTRANSFORM_VALUE_COUNT values!
	nlassert (0);

	return NULL;
}
// ***************************************************************************
const std::string CTransform::valueNames [NL3D_CTRANSFORM_VALUE_COUNT]=
{
	std::string ("POS"),
	std::string ("ROTEULER"),
	std::string ("ROTQUAT"),
	std::string ("SCALE"),
	std::string ("PIVOT")
};
// ***************************************************************************
const std::string&	CTransform::getValueName (uint valueId) const
{
	// Only value of the transform
	nlassert (valueId<NL3D_CTRANSFORM_VALUE_COUNT);

	// Return the value
	return valueNames[valueId];
}
// ***************************************************************************
ITrack*		CTransform::getDefaultTrack (uint valueId)
{
	// Only value of the transform
	nlassert (valueId<NL3D_CTRANSFORM_VALUE_COUNT);

	// what value ?
	switch (valueId)
	{
	case 0:
		return _PosDefault;
	case 1:
		return _RotEulerDefault;
	case 2:
		return _RotQuatDefault;
	case 3:
		return _ScaleDefault;
	case 4:
		return _PivotDefault;
	}

	// No, only NL3D_CTRANSFORM_VALUE_COUNT values!
	nlassert (0);

	return NULL;
}


}
