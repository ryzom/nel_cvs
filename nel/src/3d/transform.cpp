/* transform.cpp
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: transform.cpp,v 1.2 2000/10/10 16:12:56 berenguier Exp $
 *
 * <Replace this by a description of the file>
 */

#include "nel/3d/transform.h"


namespace	NL3D
{

// ***************************************************************************
static	IModel	*creatorTransform() {return new CTransform;}
static	IObs	*creatorHrcObs() {return new CTransformHrcObs;}
static	IObs	*creatorClipObs() {return new CTransformClipObs;}

void	CTransform::registerBasic()
{
	CMOT::registerModel(TransformId, 0, creatorTransform);
	CMOT::registerObs(HrcTravId, TransformId, creatorHrcObs);
	CMOT::registerObs(ClipTravId, TransformId, creatorClipObs);
}


// ***************************************************************************
CTransform::CTransform()
{
	Touch.resize(Last);

	Visibility= CHrcTrav::Herit;

	Pos= Rot= CVector::Null;
	Scale.set(1,1,1);
	RotOrder= CMatrix::XYZ;
	PosRotScaleMode= true;

	LocalMatrix.identity();
	BadLocalMatrix= false;
}


// ***************************************************************************
void		CTransform::setMatrix(const CMatrix &mat)
{
	PosRotScaleMode= false;
	LocalMatrix= mat;
	foul();
}
// ***************************************************************************
void		CTransform::getMatrix(CMatrix &mat) const
{
	// update the local matrix.
	CTransform	*self= const_cast<CTransform*>(this);
	self->updateLocalMatrix();
	mat= self->LocalMatrix;
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



}