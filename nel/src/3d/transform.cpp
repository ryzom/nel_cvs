/* transform.cpp
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: transform.cpp,v 1.1 2000/10/06 16:43:58 berenguier Exp $
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
	CMOT::registerModel(TransformModelId, 0, creatorTransform);
	CMOT::registerObs(HrcTravId, TransformModelId, creatorHrcObs);
	CMOT::registerObs(ClipTravId, TransformModelId, creatorClipObs);
}


// ***************************************************************************
CTransform::CTransform()
{
	Touch.resize(Last);

	Visibility= CHrcTrav::Herit;

	Pos= Rot= Scale= CVector::Null;
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
	foul();
	Visibility= CHrcTrav::Hide;
}
// ***************************************************************************
void		CTransform::show()
{
	foul();
	Visibility= CHrcTrav::Show;
}
// ***************************************************************************
void		CTransform::heritVisibility()
{
	foul();
	Visibility= CHrcTrav::Herit;
}



}