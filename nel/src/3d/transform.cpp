/** \file transform.cpp
 * <File description>
 *
 * $Id: transform.cpp,v 1.5 2000/11/30 17:53:43 berenguier Exp $
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

	LocalMatrix.identity();
	Visibility= CHrcTrav::Herit;
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



}
