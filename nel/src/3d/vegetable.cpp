/** \file vegetable.cpp
 * <File description>
 *
 * $Id: vegetable.cpp,v 1.4 2001/11/07 16:41:53 berenguier Exp $
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

#include "3d/vegetable.h"
#include "nel/misc/common.h"
#include "3d/vegetable_manager.h"
#include "3d/fast_floor.h"


using namespace std;
using namespace NLMISC;


namespace NL3D 
{


// ***************************************************************************
// Generate random value, but seed is spacial. Take a high frequency, so it gets more the aspect of random.
static	CNoiseValue		RandomGenerator(0,1, 7.68f);


// ***************************************************************************
CVegetable::CVegetable()
{
	// Ground style density.
	setAngleGround(0);

	// No scale.
	Sxy.Abs= Sz.Abs= 1;
	Sxy.Rand= Sz.Rand= 0;
	// No rotation.
	Rx.Abs= Ry.Abs= Rz.Abs= 0;
	Rx.Rand= Ry.Rand= Rz.Rand= 0;
	// No BendFactor.
	BendFactor.Abs= 1;
	BendFactor.Rand= 0;

	_Manager= NULL;
}


// ***************************************************************************
void	CVegetable::setAngleGround(float cosAngleMin)
{
	_CosAngleMin= cosAngleMin;
	// We must be at densityFactor==1, when cosAngle==1, keeping the same formula.
	_CosAngleMax= 1 + (1-cosAngleMin);

	// precalc
	_CosAngleMiddle= (_CosAngleMin + _CosAngleMax)/2;
	_OOCosAngleDist= 1.0f / (_CosAngleMax - _CosAngleMiddle);
}

// ***************************************************************************
void	CVegetable::setAngleCeiling(float cosAngleMax)
{
	_CosAngleMax= cosAngleMax;
	// We must be at densityFactor==1, when cosAngle==-1, keeping the same formula.
	_CosAngleMin= -1 - (cosAngleMax-(-1));
	
	// precalc
	_CosAngleMiddle= (_CosAngleMin + _CosAngleMax)/2;
	_OOCosAngleDist= 1.0f / (_CosAngleMax - _CosAngleMiddle);
}

// ***************************************************************************
void	CVegetable::setAngleWall(float cosAngleMin, float cosAngleMax)
{
	_CosAngleMin= cosAngleMin;
	_CosAngleMax= cosAngleMax;

	// precalc
	_CosAngleMiddle= (_CosAngleMin + _CosAngleMax)/2;
	_OOCosAngleDist= 1.0f / (_CosAngleMax - _CosAngleMiddle);
}


// ***************************************************************************
void	CVegetable::registerToManager(CVegetableManager *manager)
{
	nlassert(manager);
	_Manager= manager;
	_VegetableShape= _Manager->getVegetableShape(ShapeName);
}


// ***************************************************************************
void	CVegetable::generateGroup(const CVector &posInWorld, const CVector &surfaceNormal, float area, uint vegetSeed, std::vector<CVector2f> &instances) const
{

	// Density evaluation.
	//===================

	// evaluate a density.
	float	nbInst= area * Density.eval(posInWorld);

	// compute cos of angle between surfaceNormal and K(0,0,1).
	float	cosAngle= surfaceNormal.z;
	// compute angleFactor density. Use a quadratic, because f'(_CosAngleMiddle)==0.
	float	angleFact= 1 - sqr((cosAngle - _CosAngleMiddle) * _OOCosAngleDist);
	angleFact= max(0.f, angleFact);
	// modulate density with angleFactor.
	nbInst*= angleFact;

	// Now, 0<=nbInst<+oo. If we have 0.1, it means that we have 10% chance to spawn an instance.
	// So add a "random" value (with help of a noise with High frequency)
	// if nbInst==0, we should never have any instance (which may arise if evalOneLevelRandom()==1).
	// hence the 0.99f* which ensure that we do nbInst+= [0..1[.
	nbInst+= 0.99f * RandomGenerator.evalOneLevelRandom(posInWorld);

	// and then get only the integral part.
	sint	nbInstances= OptFastFloor(nbInst);
	nbInstances= max(0, nbInstances);

	// resize the instances
	instances.resize(nbInstances);

	// Position generation.
	//===================
	// For now, generate them randomly.
	static CVector2f	dSeed(0.513f, 0.267f);	// random values.
	CVector				seed= posInWorld;
	seed.z+= vegetSeed * 0.723f;	// 0.723f is a random value.
	for(sint i=0; i<nbInstances; i++)
	{
		instances[i].x= RandomGenerator.evalOneLevelRandom(seed);
		seed.x+= dSeed.x;
		instances[i].y= RandomGenerator.evalOneLevelRandom(seed);
		seed.y+= dSeed.y;
	}
}


// ***************************************************************************
void	CVegetable::generateInstance(CVegetableInstanceGroup *ig, const NLMISC::CMatrix &posInWorld, 
		const NLMISC::CRGBAF &modulateAmbientColor, const NLMISC::CRGBAF &modulateDiffuseColor) const
{
	nlassert(_Manager);

	CVector		seed= posInWorld.getPos();

	// Generate Matrix.
	// ===============

	// Generate a random Scale / Rotation matrix.
	CMatrix		randomMat;
	// setup rotation
	CVector		rot;
	rot.x= Rx.eval(seed);
	rot.y= Ry.eval(seed);
	rot.z= Rz.eval(seed);
	randomMat.setRot(rot, CMatrix::ZXY);
	// scale.
	if(Sxy.Abs!=0 || Sxy.Rand!=0 || Sz.Abs!=0 || Sz.Rand!=0)
	{
		CVector		scale;
		scale.x= scale.y= Sxy.eval(seed);
		scale.z= Sz.eval(seed);
		randomMat.scale(scale);
	}

	// Final Matrix.
	CMatrix		finalMatrix;
	finalMatrix= posInWorld * randomMat;

	// Generate Color and factor
	// ===============
	CRGBAF		materialColor(1,1,1,1);
	// evaluate gradients. If none, color not modified.
	Color.eval(seed, materialColor);
	// modulate with user
	CRGBAF		ambient, diffuse;
	ambient= modulateAmbientColor * materialColor;
	diffuse= modulateDiffuseColor * materialColor;

	// Generate a bendFactor
	float	bendFactor= BendFactor.eval(seed);
	// Generate a bendPhase
	float	bendPhase= BendPhase.eval(seed);


	// Append to the vegetableManager
	// ===============
	_Manager->addInstance(ig, _VegetableShape, finalMatrix, ambient, diffuse, bendFactor, bendPhase);
}


// ***************************************************************************
void	CVegetable::serial(NLMISC::IStream &f)
{
	sint	ver= f.serialVersion(0);

	f.serial(ShapeName);
	f.serial(Density);
	f.serial(_CosAngleMin, _CosAngleMax, _CosAngleMiddle, _OOCosAngleDist);
	f.serial(Sxy, Sz);
	f.serial(Rx, Ry, Rz);
	f.serial(BendFactor);
	f.serial(BendPhase);
	f.serial(Color);
}


} // NL3D
