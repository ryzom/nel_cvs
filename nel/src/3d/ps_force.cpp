/** \file ps_force.cpp
 * <File description>
 *
 * $Id: ps_force.cpp,v 1.30 2002/08/21 09:39:53 lecroart Exp $
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

#include "std3d.h"

#include "3d/ps_force.h"
#include "3d/driver.h"
#include "3d/primitive_block.h"
#include "3d/material.h"
#include "3d/vertex_buffer.h"
#include "3d/computed_string.h"
#include "3d/font_manager.h"
#include "3d/particle_system.h"
#include "nel/misc/common.h"
#include "3d/ps_util.h"
#include "3d/ps_misc.h"

namespace NL3D {


/*
 * Constructor
 */
CPSForce::CPSForce()
{
}



void CPSForce::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	
		f.serialVersion(1);	
		CPSTargetLocatedBindable::serial(f);	
		CPSLocatedBindable::serial(f);
	
}


void CPSForce::registerToTargets(void)
{
	for (TTargetCont::iterator it = _Targets.begin(); it != _Targets.end(); ++it)
	{
		if (this->isIntegrable())
		{						
			(*it)->registerIntegrableForce(this);
		}
		else
		{
			(*it)->addNonIntegrableForceRef();
		}
	}
}


void CPSForce::step(TPSProcessPass pass, TAnimationTime ellapsedTime, TAnimationTime realEllapsedTime)
{
	switch(pass)
	{
		case PSMotion:
			performDynamic(ellapsedTime);
			break;
		case PSToolRender:
			show(ellapsedTime);
			break;
		default: break;
	}
}



void	CPSForce::attachTarget(CPSLocated *ptr)
{
	nlassert(_Owner);	
	CPSTargetLocatedBindable::attachTarget(ptr);
	// check wether we are integrable, and if so, add us to the list
	if (this->isIntegrable())
	{
		ptr->registerIntegrableForce(this);
	}
	else
	{
		ptr->addNonIntegrableForceRef();
	}
}

void	CPSForce::releaseTargetRsc(CPSLocated *target)
{
	if (this->isIntegrable())
	{
		target->unregisterIntegrableForce(this);
	}
	else
	{
		target->releaseNonIntegrableForceRef();
	}	
}



void	CPSForce::basisChanged(bool systemBasis)
{
	if (!this->isIntegrable()) return;
	for (TTargetCont::iterator it = _Targets.begin(); it != _Targets.end(); ++it)
	{	
		(*it)->integrableForceBasisChanged(systemBasis);
	}		
}


void	CPSForce::cancelIntegrable(void)
{
	nlassert(_Owner);
	bool useSystemBasis = _Owner->isInSystemBasis();
	for (TTargetCont::iterator it = _Targets.begin(); it != _Targets.end(); ++it)
	{
		if ((*it)->isInSystemBasis() == useSystemBasis)
		{
			(*it)->unregisterIntegrableForce(this);
			(*it)->addNonIntegrableForceRef();
		}
	}
}


void	CPSForce::renewIntegrable(void)
{
	nlassert(_Owner);
	bool useSystemBasis = _Owner->isInSystemBasis();
	for (TTargetCont::iterator it = _Targets.begin(); it != _Targets.end(); ++it)
	{
		if ((*it)->isInSystemBasis() == useSystemBasis)
		{
			(*it)->registerIntegrableForce(this);
			(*it)->releaseNonIntegrableForceRef();
		}
	}
}



///////////////////////////////////////
//  CPSForceIntensity implementation //
///////////////////////////////////////

void CPSForceIntensity::setIntensity(float value)
{
	if (_IntensityScheme)
	{
		delete _IntensityScheme;
		_IntensityScheme = NULL;
	}
	_K = value;
	
}

CPSForceIntensity::~CPSForceIntensity()
{
	delete _IntensityScheme;
}

void CPSForceIntensity::setIntensityScheme(CPSAttribMaker<float> *scheme)
{
	nlassert(scheme);
	delete _IntensityScheme;
	_IntensityScheme = scheme;
	if (getForceIntensityOwner() && scheme->hasMemory()) scheme->resize(getForceIntensityOwner()->getMaxSize(), getForceIntensityOwner()->getSize());
}

void CPSForceIntensity::serialForceIntensity(NLMISC::IStream &f) throw(NLMISC::EStream)
{	
	f.serialVersion(1);
	if (!f.isReading())
	{
		if (_IntensityScheme)
		{
			bool bFalse = false;
			f.serial(bFalse);
			f.serialPolyPtr(_IntensityScheme);
		}
		else
		{
			bool bTrue = true;
			f.serial(bTrue);
			f.serial(_K);
		}
	}
	else
	{
		bool constantIntensity;
		f.serial(constantIntensity);
		if (constantIntensity)
		{
			f.serial(_K);
		}
		else
		{
			f.serialPolyPtr(_IntensityScheme);
		}
	}	
}


////////////////////////////////////////
// CPSForceIntensityHelper            //
////////////////////////////////////////


void CPSForceIntensityHelper::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1);
	CPSForce::serial(f);
	serialForceIntensity(f);
	if (f.isReading())
	{
		registerToTargets();
	}
}



////////////////////////////////////////
// CPSDirectionalForce implementation //
////////////////////////////////////////


void CPSDirectionnalForce::performDynamic(TAnimationTime ellapsedTime)
{
	// perform the operation on each target

	CVector toAdd;


	for (uint32 k = 0; k < _Owner->getSize(); ++k)
	{	
		CVector toAddLocal = ellapsedTime * (_IntensityScheme ? _IntensityScheme->get(_Owner, k) : _K ) * _Dir;
		for (TTargetCont::iterator it = _Targets.begin(); it != _Targets.end(); ++it)
		{

			toAdd = CPSLocated::getConversionMatrix(*it, this->_Owner).mulVector(toAddLocal); // express this in the target basis			

			TPSAttribVector::iterator it2 = (*it)->getSpeed().begin(), it2end = (*it)->getSpeed().end();

			// 1st case : non-constant mass
			if ((*it)->getMassScheme())
			{
				TPSAttribFloat::const_iterator invMassIt = (*it)->getInvMass().begin();			
				for (; it2 != it2end; ++it2, ++invMassIt)
				{
					(*it2) += *invMassIt * toAdd;				
					
				}
			}
			else
			{
				toAdd *= (*it)->getInitialMass();
				for (; it2 != it2end; ++it2)
				{
					(*it2) += toAdd;									
				}
			}
		}
	}
}


void CPSDirectionnalForce::show(TAnimationTime ellapsedTime)
{
	CPSLocated *loc;
	uint32 index;
	CPSLocatedBindable *lb;
	_Owner->getOwner()->getCurrentEditedElement(loc, index, lb);

	setupDriverModelMatrix();
	
	for (uint k = 0; k < _Owner->getSize(); ++k)
	{
		const CRGBA col = ((lb == NULL || this == lb) && loc == _Owner && index == k  ? CRGBA::Red : CRGBA(127, 127, 127));
		CPSUtil::displayArrow(getDriver(), _Owner->getPos()[k], _Dir, 1.f, col, CRGBA(80, 80, 0));
	}
}

void CPSDirectionnalForce::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1);		
	CPSForceIntensityHelper::serial(f);	
	f.serial(_Dir);
}

////////////////////////////
// gravity implementation //
////////////////////////////


void CPSGravity::performDynamic(TAnimationTime ellapsedTime)
{	
	
	// perform the operation on each target
	CVector toAdd;
	for (uint32 k = 0; k < _Owner->getSize(); ++k)
	{	
		CVector toAddLocal = ellapsedTime * CVector(0, 0, _IntensityScheme ? - _IntensityScheme->get(_Owner, k) : - _K);
		for (TTargetCont::iterator it = _Targets.begin(); it != _Targets.end(); ++it)
		{

			toAdd = CPSLocated::getConversionMatrix(*it, this->_Owner).mulVector(toAddLocal); // express this in the target basis			

			


			TPSAttribVector::iterator it2 = (*it)->getSpeed().begin(), it2end = (*it)->getSpeed().end();
			
			if (toAdd.x && toAdd.y)
			{
				for (; it2 != it2end; ++it2)
				{
					(*it2) += toAdd;				
					
				}
			}
			else // only the z component is not null, which should be the majority of cases ...
			{
				for (; it2 != it2end; ++it2)
				{
					it2->z += toAdd.z;				
					
				}
			}
		}
	}
}
void CPSGravity::show(TAnimationTime ellapsedTime) 
{	
	CVector I = computeI();
	CVector K = CVector(0,0,1);	    

	// this is not designed for efficiency (target : edition code)
	CPrimitiveBlock	 pb;
	CVertexBuffer vb;
	CMaterial material;
	IDriver *driver = getDriver();
	const float toolSize = 0.2f;

	vb.setVertexFormat(CVertexBuffer::PositionFlag);
	vb.setNumVertices(6);
	vb.setVertexCoord(0, -toolSize * I);
	vb.setVertexCoord(1, toolSize * I);
	vb.setVertexCoord(2, CVector(0, 0, 0));
	vb.setVertexCoord(3, -6.0f * toolSize * K);
	vb.setVertexCoord(4, -toolSize * I  - 5.0f * toolSize * K);
	vb.setVertexCoord(5, toolSize * I - 5.0f * toolSize * K);

	pb.reserveLine(4);
	pb.addLine(0, 1);
	pb.addLine(2, 3);
	pb.addLine(4, 3);
	pb.addLine(3, 5);	
	
	material.setColor(CRGBA(127, 127, 127));
	material.setLighting(false);
	material.setBlendFunc(CMaterial::one, CMaterial::one);
	material.setZWrite(false);
	material.setBlend(true);

	
	CMatrix mat;

	for (TPSAttribVector::const_iterator it = _Owner->getPos().begin(); it != _Owner->getPos().end(); ++it)
	{
		mat.identity();
		mat.translate(*it);
		if (_Owner->isInSystemBasis())
		{
			mat = getSysMat() * mat;
		}
		
		driver->setupModelMatrix(mat);
		driver->activeVertexBuffer(vb);
		driver->render(pb, material);
	


		// affiche un g a cote de la force

		CVector pos = *it + CVector(1.5f * toolSize, 0, -1.2f * toolSize);

		if (_Owner->isInSystemBasis())
		{
			pos = getSysMat() * pos;
		}

		// must have set this
		nlassert(getFontGenerator() && getFontGenerator());
		
		CPSUtil::print(driver, std::string("G")
							, *getFontGenerator()
							, *getFontManager()
							, pos
							, 80.0f * toolSize );								
	}

	
}

void CPSGravity::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1);	
	CPSForceIntensityHelper::serial(f);	
}


bool	CPSGravity::isIntegrable(void) const
{
	return _IntensityScheme == NULL;
}

void CPSGravity::integrate(float date, CPSLocated *src, uint32 startIndex, uint32 numObjects, NLMISC::CVector *destPos, NLMISC::CVector *destSpeed,
							bool accumulate,
							uint posStride, uint speedStride
							)
{
	#define NEXT_SPEED destSpeed = (NLMISC::CVector *) ((uint8 *) destSpeed + speedStride);
	#define NEXT_POS   destPos   = (NLMISC::CVector *) ((uint8 *) destPos   + posStride);

	float deltaT;

	if (!destPos && !destSpeed) return;

	CPSLocated::TPSAttribParametricInfo::const_iterator it = src->_PInfo.begin() + startIndex,
														endIt = src->_PInfo.begin() + startIndex + numObjects;	
	if (!accumulate) // compute coords from initial condition, and applying this force
	{
		if (destPos && !destSpeed) // fills dest pos only
		{
			while (it != endIt)
			{
				deltaT = date - it->Date;				
				destPos->x = it->Pos.x + deltaT * it->Speed.x;				
				destPos->y = it->Pos.y + deltaT * it->Speed.y;
				destPos->z = it->Pos.z + deltaT * it->Speed.z - 0.5f * deltaT * deltaT * _K;
				++it;
				NEXT_POS;	
			}
		}
		else if (!destPos && destSpeed) // fills dest speed only
		{
			while (it != endIt)
			{
				deltaT = date - it->Date;				
				destSpeed->x = it->Speed.x;				
				destSpeed->y = it->Speed.y;
				destSpeed->z = it->Speed.z - deltaT * _K;
				++it;
				NEXT_SPEED;	
			}
		}
		else // fills both speed and pos
		{
			while (it != endIt)
			{
				deltaT = date - it->Date;				
				destPos->x = it->Pos.x + deltaT * it->Speed.x;				
				destPos->y = it->Pos.y + deltaT * it->Speed.y;
				destPos->z = it->Pos.z + deltaT * it->Speed.z - 0.5f * deltaT * deltaT * _K;

				destSpeed->x = it->Speed.x;				
				destSpeed->y = it->Speed.y;
				destSpeed->z = it->Speed.z - deltaT * _K;

				++it;
				NEXT_POS;
				NEXT_SPEED;
			}
		}
	}
	else // accumulate datas
	{
		if (destPos && !destSpeed) // fills dest pos only
		{
			while (it != endIt)
			{
				deltaT = date - it->Date;								
				destPos->z -= 0.5f * deltaT * deltaT * _K;				
				++it;
				NEXT_POS;	
			}
		}
		else if (!destPos && destSpeed) // fills dest speed only
		{
			while (it != endIt)
			{
				deltaT = date - it->Date;								
				destSpeed->z -= deltaT * _K;				
				++it;
				NEXT_SPEED;	
			}
		}
		else // fills both speed and pos
		{
			while (it != endIt)
			{
				deltaT = date - it->Date;								
				destPos->z -= 0.5f * deltaT * deltaT * _K;
				destSpeed->z -= deltaT * _K;
				++it;
				NEXT_POS;
				NEXT_SPEED;
			}
		}
	}
	
}



void CPSGravity::integrateSingle(float startDate, float deltaT, uint numStep,								 
								 CPSLocated *src, uint32 indexInLocated,
								 NLMISC::CVector *destPos,
								 bool accumulate /*= false*/,
								 uint stride/* = sizeof(NLMISC::CVector)*/)
{		
	nlassert(src->isParametricMotionEnabled());
	nlassert(deltaT > 0);
	nlassert(numStep > 0);
	#ifdef NL_DEBUG
		NLMISC::CVector *endPos = (NLMISC::CVector *) ( (uint8 *) destPos + stride * numStep);
	#endif
	const CPSLocated::CParametricInfo &pi = src->_PInfo[indexInLocated];
	const NLMISC::CVector &startPos   = pi.Pos;	
	if (numStep != 0)
	{
		if (!accumulate)
		{				
			destPos = FillBufUsingSubdiv(startPos, pi.Date, startDate, deltaT, numStep, destPos, stride);
			if (numStep != 0)
			{
				float currDate = startDate - pi.Date;
				nlassert(currDate >= 0);
				const NLMISC::CVector &startSpeed = pi.Speed;			
				do
				{
					#ifdef NL_DEBUG
						nlassert(destPos < endPos);
					#endif
					float halfTimeSquare  = 0.5f * currDate * currDate;
					destPos->x = startPos.x + currDate * startSpeed.x;
					destPos->y = startPos.y + currDate * startSpeed.y;
					destPos->z = startPos.z + currDate * startSpeed.z - _K * halfTimeSquare;
					currDate += deltaT;
					destPos = (NLMISC::CVector *) ( (uint8 *) destPos + stride);
				}
				while (--numStep);
			}
		}
		else
		{
			uint numToSkip = ScaleFloatGE(startDate, deltaT, pi.Date, numStep);		
			if (numToSkip < numStep)
			{
				numStep -= numToSkip;
				float currDate = startDate + deltaT * numToSkip - pi.Date;
				do
				{
					#ifdef NL_DEBUG
						nlassert(destPos < endPos);
					#endif
					float halfTimeSquare  = 0.5f * currDate * currDate;				
					destPos->z -=  _K * halfTimeSquare;
					currDate += deltaT;
					destPos = (NLMISC::CVector *) ( (uint8 *) destPos + stride);
				}
				while (--numStep);
			}				
		}
	}
}


void CPSGravity::setIntensity(float value)
{
	if (_IntensityScheme)
	{
		CPSForceIntensityHelper::setIntensity(value);
		renewIntegrable(); // integrable again
	}
	else
	{
		CPSForceIntensityHelper::setIntensity(value);
	}
}
	
void CPSGravity::setIntensityScheme(CPSAttribMaker<float> *scheme)
{
	if (!_IntensityScheme)
	{
		cancelIntegrable(); // not integrable anymore
	}			
	CPSForceIntensityHelper::setIntensityScheme(scheme);	
}


/////////////////////////////////////////
// CPSCentralGravity  implementation   //
/////////////////////////////////////////


void CPSCentralGravity::performDynamic(TAnimationTime ellapsedTime)
{
	// for each central gravity, and each target, we check if they are in the same basis
	// if not, we need to transform the central gravity attachment pos into the target basis
	

	uint32 size = _Owner->getSize();

	// a vector that goes from the gravity to the object
	CVector centerToObj;
	float dist;
	
	for (uint32 k = 0; k < size; ++k)
	{	
		const float ellapsedTimexK = ellapsedTime  * (_IntensityScheme ? _IntensityScheme->get(_Owner, k) : _K);

		for (TTargetCont::iterator it = _Targets.begin(); it != _Targets.end(); ++it)
		{
			const CMatrix &m = CPSLocated::getConversionMatrix(*it, this->_Owner);
			const CVector center = m * (_Owner->getPos()[k]);
						
			TPSAttribVector::iterator it2 = (*it)->getSpeed().begin(), it2End = (*it)->getSpeed().end();
			TPSAttribFloat::const_iterator invMassIt = (*it)->getInvMass().begin();
			TPSAttribVector::const_iterator posIt = (*it)->getPos().begin();
			
			for (; it2 != it2End; ++it2, ++invMassIt, ++posIt)
			{
				// our equation does 1 / r attenuation, which is not realistic, but fast ...
				centerToObj = center - *posIt;
	
				dist = centerToObj * centerToObj;
				if (dist > 10E-6f)
				{
					(*it2) += (*invMassIt) * ellapsedTimexK * (1.f / dist) *  centerToObj;								
				}
			}
		}
	}

}


void CPSCentralGravity::show(TAnimationTime ellapsedTime)
{
	CVector I = CVector::I;
	CVector J = CVector::J;

	const CVector tab[] = { -I - J, I - J
							,-I + J, I + J
							, I - J, I + J
							, -I - J, -I + J
							, I + J, -I - J
							, I - J, J - I
							};
	const uint tabSize = sizeof(tab) / (2 * sizeof(CVector));

	const float sSize = 0.08f;
	displayIcon2d(tab, tabSize, sSize);
}

	

void CPSCentralGravity::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1);	
	CPSForceIntensityHelper::serial(f);	
}


/////////////////////////////////
// CPSSpring  implementation   //
/////////////////////////////////



void CPSSpring::performDynamic(TAnimationTime ellapsedTime)
{
	// for each spring, and each target, we check if they are in the same basis
	// if not, we need to transform the spring attachment pos into the target basis
	

	uint32 size = _Owner->getSize();

	
	for (uint32 k = 0; k < size; ++k)
	{	
		const float ellapsedTimexK = ellapsedTime  * (_IntensityScheme ? _IntensityScheme->get(_Owner, k) : _K);

		for (TTargetCont::iterator it = _Targets.begin(); it != _Targets.end(); ++it)
		{
			const CMatrix &m = CPSLocated::getConversionMatrix(*it, this->_Owner);
			const CVector center = m * (_Owner->getPos()[k]);
						
			TPSAttribVector::iterator it2 = (*it)->getSpeed().begin(), it2End = (*it)->getSpeed().end();
			TPSAttribFloat::const_iterator invMassIt = (*it)->getInvMass().begin();
			TPSAttribVector::const_iterator posIt = (*it)->getPos().begin();
			
			for (; it2 != it2End; ++it2, ++invMassIt, ++posIt)
			{
				// apply the spring equation

				(*it2) += (*invMassIt) * ellapsedTimexK * (center - *posIt);								
			}
		}
	}
}


void CPSSpring::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1);	
	CPSForceIntensityHelper::serial(f);	
}



void CPSSpring::show(TAnimationTime ellapsedTime)
{
	CVector I = CVector::I;
	CVector J = CVector::J;

	const CVector tab[] = { -I + 2 * J, I + 2 * J
							, I + 2 * J, -I + J
							, -I + J, I + J
							, I + J, -I
							,  -I, I
							, I, -I - J
							, -I - J, I - J
							, I - J, - I - 2 * J
							, - I - 2 * J, I - 2 * J
							};
	const uint tabSize = sizeof(tab) / (2 * sizeof(CVector));

	const float sSize = 0.08f;
	displayIcon2d(tab, tabSize, sSize);
}


/////////////////////////////////////////
//  CPSCylindricVortex implementation  //
/////////////////////////////////////////

void CPSCylindricVortex::performDynamic(TAnimationTime ellapsedTime)
{
		uint32 size = _Owner->getSize();
		
	for (uint32 k = 0; k < size; ++k) // for each vortex
	{					
		
		const float invR = 1.f  / _Radius[k];
		const float radius2 = _Radius[k] * _Radius[k];

		// intensity for this vortex
		nlassert(_Owner);
		float intensity =  (_IntensityScheme ? _IntensityScheme->get(_Owner, k) : _K);

		for (TTargetCont::iterator it = _Targets.begin(); it != _Targets.end(); ++it)
		{
			// express the vortex position and plane normal in the located basis
			const CMatrix &m = CPSLocated::getConversionMatrix(*it, this->_Owner);
			const CVector center = m * (_Owner->getPos()[k]);
			const CVector n = m.mulVector(_Normal[k]);

			TPSAttribVector::iterator speedIt = (*it)->getSpeed().begin(), speedItEnd = (*it)->getSpeed().end();
			TPSAttribFloat::const_iterator invMassIt = (*it)->getInvMass().begin();
			TPSAttribVector::const_iterator posIt = (*it)->getPos().begin();
			

			// projection of the current located pos on the vortex axis
			CVector p;
			// a vector that go from the vortex center to the point we're dealing with
			CVector v2p;

			// the square of the dist of the projected pos
			float d2 , d;

			CVector realTangentialSpeed;
			CVector tangentialSpeed;

			
			CVector radialSpeed;


			for (; speedIt != speedItEnd; ++speedIt, ++invMassIt, ++posIt)
			{
				v2p = *posIt - center;
				p = v2p - (v2p * n) * n;

				d2 = p * p;

				

				if (d2 < radius2) // not out of range ?
				{
					if (d2 > 10E-6)
					{
						d = sqrtf(d2);

						p *= 1.f / d;
						// compute the speed vect that we should have (normalized) 
						realTangentialSpeed = n ^ p;

						tangentialSpeed = (*speedIt * realTangentialSpeed) * realTangentialSpeed;
						radialSpeed =  (p * *speedIt) * p;
						
						// update radial speed;
						*speedIt -= _RadialViscosity * ellapsedTime * radialSpeed;
						
						// update tangential speed					
						*speedIt -= _TangentialViscosity * intensity * ellapsedTime * (tangentialSpeed - (1.f - d * invR) * realTangentialSpeed);
					}
				}				
			}
		}
	}
}


void CPSCylindricVortex::show(TAnimationTime ellapsedTime)
{
	
	CPSLocated *loc;
	uint32 index;
	CPSLocatedBindable *lb;
	_Owner->getOwner()->getCurrentEditedElement(loc, index, lb);


	// must have set this
	nlassert(getFontGenerator() && getFontGenerator());
	setupDriverModelMatrix();
	
	for (uint k = 0; k < _Owner->getSize(); ++k)
	{
		const CRGBA col = ((lb == NULL || this == lb) && loc == _Owner && index == k  ? CRGBA::Red : CRGBA(127, 127, 127));
		CMatrix m;
		CPSUtil::buildSchmidtBasis(_Normal[k], m);
		CPSUtil::displayDisc(*getDriver(), _Radius[k], _Owner->getPos()[k], m, 32, col);
		CPSUtil::displayArrow(getDriver(), _Owner->getPos()[k], _Normal[k], 1.f, col, CRGBA(200, 0, 200));
		// display a V letter at the center
		CPSUtil::print(getDriver(), std::string("v"), *getFontGenerator(), *getFontManager(), _Owner->getPos()[k], 80.f);
	}

}

void CPSCylindricVortex::setMatrix(uint32 index, const CMatrix &m)
{
	nlassert(index < _Normal.getSize());
	_Normal[index] = m.getK();
	_Owner->getPos()[index] = m.getPos();	
}

CMatrix CPSCylindricVortex::getMatrix(uint32 index) const
{
	CMatrix m;
	CPSUtil::buildSchmidtBasis(_Normal[index], m);
	m.setPos(_Owner->getPos()[index] ); 
	return m;
}


void CPSCylindricVortex::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1);	
	CPSForceIntensityHelper::serial(f);	
	f.serial(_Normal);
	f.serial(_Radius);
	f.serial(_RadialViscosity);
	f.serial(_TangentialViscosity);
}

void CPSCylindricVortex::newElement(CPSLocated *emitterLocated, uint32 emitterIndex) 
{ 
	CPSForceIntensityHelper::newElement(emitterLocated, emitterIndex); 
	_Normal.insert(CVector::K);
	_Radius.insert(1.f); 
}
void CPSCylindricVortex::deleteElement(uint32 index) 
{ 
	CPSForceIntensityHelper::deleteElement(index);
	_Normal.remove(index); 
	_Radius.remove(index);
}
void CPSCylindricVortex::resize(uint32 size) 
{ 
	nlassert(size < (1 << 16));
	CPSForceIntensityHelper::resize(size); 
	_Normal.resize(size);
	_Radius.resize(size);
}


/**
 *  a magnetic field that has the given direction
 */

void CPSMagneticForce::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1);
	CPSDirectionnalForce::serial(f);
}

void CPSMagneticForce::performDynamic(TAnimationTime ellapsedTime)
{	
	// perform the operation on each target
	for (uint32 k = 0; k < _Owner->getSize(); ++k)
	{	
		float intensity = ellapsedTime * (_IntensityScheme ? _IntensityScheme->get(_Owner, k) : _K);
		for (TTargetCont::iterator it = _Targets.begin(); it != _Targets.end(); ++it)
		{

			NLMISC::CVector toAdd = CPSLocated::getConversionMatrix(*it, this->_Owner).mulVector(_Dir); // express this in the target basis			

			TPSAttribVector::iterator it2 = (*it)->getSpeed().begin(), it2end = (*it)->getSpeed().end();

			// 1st case : non-constant mass
			if ((*it)->getMassScheme())
			{
				TPSAttribFloat::const_iterator invMassIt = (*it)->getInvMass().begin();			
				for (; it2 != it2end; ++it2, ++invMassIt)
				{
					(*it2) += intensity * *invMassIt * (*it2 ^ toAdd);
					
				}
			}
			else
			{
				float i = intensity * (*it)->getInitialMass();
				for (; it2 != it2end; ++it2)
				{
					(*it2) += i * (*it2 ^ toAdd);
				}
			}
		}
	}
}


/**
 *  Brownian force implementation
 */

const uint BFNumPredefinedPos    = 8192;	// should be a power of 2
const uint BFPredefinedNumInterp = 256;	    /** this should divide BFNumPredefinedPos. This define the number 
											  * of values used to interpolate between 2 position of the npose
											  * (because we don't filter values when we access them)
											  */
const uint BFNumPrecomputedImpulsions = 1024; /// used to avoid to have to call rand for each particle the force applies on...

NLMISC::CVector CPSBrownianForce::PrecomputedPos[BFNumPredefinedPos]; // after the sequence we must be back to the start position
NLMISC::CVector CPSBrownianForce::PrecomputedSpeed[BFNumPredefinedPos];
NLMISC::CVector CPSBrownianForce::PrecomputedImpulsions[BFNumPrecomputedImpulsions];

///==========================================================
CPSBrownianForce::CPSBrownianForce(float intensity /* = 1.f*/) : _ParametricFactor(1.f)
{
	setIntensity(intensity);
	_Name = std::string("BrownianForce");

}

///==========================================================
bool	CPSBrownianForce::isIntegrable(void) const
{
	return _IntensityScheme == NULL;
}


///==========================================================
void CPSBrownianForce::integrate(float date, CPSLocated *src,
								 uint32 startIndex,
								 uint32 numObjects,
								 NLMISC::CVector *destPos,
								 NLMISC::CVector *destSpeed,
								 bool accumulate,
								 uint posStride, uint speedStride
							    )
{
	/// MASS DIFFERENT FROM 1 IS NOT SUPPORTED		
	float deltaT;
	if (!destPos && !destSpeed) return;
	CPSLocated::TPSAttribParametricInfo::const_iterator it = src->_PInfo.begin() + startIndex,
														endIt = src->_PInfo.begin() + startIndex + numObjects;
	float lookUpFactor = _ParametricFactor * BFNumPredefinedPos;
	float speedFactor  = _ParametricFactor * _K;
	if (!accumulate) // compute coords from initial condition, and applying this force
	{
		if (destPos && !destSpeed) // fills dest pos only
		{
			while (it != endIt)
			{
				float deltaT = date - it->Date;
				uint index = (uint) (lookUpFactor * deltaT) & (BFNumPredefinedPos - 1);
				destPos->set(it->Pos.x + deltaT * it->Speed.x + _K * PrecomputedPos[index].x,
							 it->Pos.y + deltaT * it->Speed.y + _K * PrecomputedPos[index].y,
							 it->Pos.z + deltaT * it->Speed.z + _K * PrecomputedPos[index].z );
				++it;
				NEXT_POS;	
			}
		}
		else if (!destPos && destSpeed) // fills dest speed only
		{
			while (it != endIt)
			{
				deltaT = date - it->Date;
				uint index = (uint) (lookUpFactor * deltaT) & (BFNumPredefinedPos - 1);
				destSpeed->x = it->Speed.x  + speedFactor * PrecomputedSpeed[index].x;				
				destSpeed->y = it->Speed.y  + speedFactor * PrecomputedSpeed[index].y;
				destSpeed->z = it->Speed.z  + speedFactor * PrecomputedSpeed[index].z;
				++it;
				NEXT_SPEED;	
			}
		}
		else // fills both speed and pos
		{
			while (it != endIt)
			{
				deltaT = date - it->Date;
				uint index = (uint) (lookUpFactor * deltaT) & (BFNumPredefinedPos - 1);
				destPos->x = it->Pos.x + deltaT * it->Speed.x + _K * PrecomputedPos[index].x;				
				destPos->y = it->Pos.y + deltaT * it->Speed.y + _K * PrecomputedPos[index].y;
				destPos->z = it->Pos.z + deltaT * it->Speed.z + _K * PrecomputedPos[index].z;

				destSpeed->x = it->Speed.x + speedFactor * PrecomputedSpeed[index].x;				
				destSpeed->y = it->Speed.y + speedFactor * PrecomputedSpeed[index].y;
				destSpeed->z = it->Speed.z + speedFactor * PrecomputedSpeed[index].z;

				++it;
				NEXT_POS;
				NEXT_SPEED;
			}
		}
	}
	else // accumulate datas
	{
		if (destPos && !destSpeed) // fills dest pos only
		{
			while (it != endIt)
			{
				deltaT = date - it->Date;
				uint index = (uint) (lookUpFactor * deltaT) & (BFNumPredefinedPos - 1);
				destPos->set(destPos->x + _K * PrecomputedPos[index].x,
							 destPos->y + _K * PrecomputedPos[index].y,
							 destPos->z + _K * PrecomputedPos[index].z);
				++it;
				NEXT_POS;	
			}
		}
		else if (!destPos && destSpeed) // fills dest speed only
		{
			while (it != endIt)
			{
				deltaT = date - it->Date;
				uint index = (uint) (lookUpFactor * deltaT) & (BFNumPredefinedPos - 1);
				destSpeed->set(destSpeed->x + speedFactor * PrecomputedSpeed[index].x,
							   destSpeed->y + speedFactor * PrecomputedSpeed[index].y,
							   destSpeed->z + speedFactor * PrecomputedSpeed[index].z);
				++it;
				NEXT_SPEED;	
			}
		}
		else // fills both speed and pos
		{
			while (it != endIt)
			{
				deltaT = date - it->Date;
				uint index = (uint) (lookUpFactor * deltaT) & (BFNumPredefinedPos - 1);
				destPos->set(destPos->x + _K * PrecomputedPos[index].x,
							 destPos->y + _K * PrecomputedPos[index].y,
							 destPos->z + _K * PrecomputedPos[index].z);
				destSpeed->set(destSpeed->x + speedFactor * PrecomputedSpeed[index].x,
							   destSpeed->y + speedFactor * PrecomputedSpeed[index].y,
							   destSpeed->z + speedFactor * PrecomputedSpeed[index].z);	
				++it;
				NEXT_POS;
				NEXT_SPEED;
			}
		}
	}	
}


///==========================================================
void CPSBrownianForce::integrateSingle(float startDate, float deltaT, uint numStep,								 
								 CPSLocated *src, uint32 indexInLocated,
								 NLMISC::CVector *destPos,
								 bool accumulate,
								 uint stride)
{
	nlassert(src->isParametricMotionEnabled());
	nlassert(deltaT > 0);
	nlassert(numStep > 0);
	#ifdef NL_DEBUG
		NLMISC::CVector *endPos = (NLMISC::CVector *) ( (uint8 *) destPos + stride * numStep);
	#endif
	const CPSLocated::CParametricInfo &pi = src->_PInfo[indexInLocated];
	const NLMISC::CVector &startPos   = pi.Pos;	
	if (numStep != 0)
	{
		float lookUpFactor = _ParametricFactor * BFPredefinedNumInterp;
		if (!accumulate)
		{				
			/// fill start of datas (particle didn't exist at that time, so we fill by the start position)
			destPos = FillBufUsingSubdiv(startPos, pi.Date, startDate, deltaT, numStep, destPos, stride);
			if (numStep != 0)
			{
				float currDate = startDate - pi.Date;
				nlassert(currDate >= 0);				
				const NLMISC::CVector &startSpeed = pi.Speed;			
				do
				{
					#ifdef NL_DEBUG
						nlassert(destPos < endPos);
					#endif
					uint index = (uint) (lookUpFactor * currDate) & (BFNumPredefinedPos - 1);
					destPos->x = startPos.x + currDate * startSpeed.x + _K * PrecomputedPos[index].x;
					destPos->y = startPos.y + currDate * startSpeed.y + _K * PrecomputedPos[index].y;
					destPos->z = startPos.z + currDate * startSpeed.z + _K * PrecomputedPos[index].z;
					currDate += deltaT;
					destPos = (NLMISC::CVector *) ( (uint8 *) destPos + stride);
				}
				while (--numStep);
			}
		}
		else
		{
			uint numToSkip = ScaleFloatGE(startDate, deltaT, pi.Date, numStep);		
			if (numToSkip < numStep)
			{
				numStep -= numToSkip;				
				float currDate = startDate + deltaT * numToSkip - pi.Date;
				do
				{
					#ifdef NL_DEBUG
						nlassert(destPos < endPos);
					#endif
					uint index = (uint) (lookUpFactor * currDate) & (BFNumPredefinedPos - 1);
					destPos->x += _K * PrecomputedPos[index].x;
					destPos->y += _K * PrecomputedPos[index].y;
					destPos->z += _K * PrecomputedPos[index].z;
					currDate += deltaT;
					destPos = (NLMISC::CVector *) ( (uint8 *) destPos + stride);
				}
				while (--numStep);				
			}
		}
	}	
}


///==========================================================
void CPSBrownianForce::initPrecalc()
{
	/// create the pos table
	nlassert(BFNumPredefinedPos % BFPredefinedNumInterp == 0);
	
	NLMISC::CVector p0(0, 0, 0), p1;
	const uint numStep = BFNumPredefinedPos / BFPredefinedNumInterp;
	NLMISC::CVector *dest = PrecomputedPos;
	uint k, l;
	for (k = 0; k < numStep; ++k)
	{
		if (k != numStep - 1)
		{
			p1.set(2.f * (NLMISC::frand(1.f) - 0.5f),
				   2.f * (NLMISC::frand(1.f) - 0.5f),
				   2.f * (NLMISC::frand(1.f) - 0.5f));
		}
		else
		{
			p1.set(0, 0, 0);
		}
		float lambda     = 0.f;
		float lambdaStep = 1.f / BFPredefinedNumInterp;
		for (l = 0; l < BFPredefinedNumInterp; ++l)
		{
			*dest++ = lambda * p1 + (1.f - lambda) * p0;
			lambda += lambdaStep;
		}
		p0 = p1;				
	}

	// now, filter the table several time to get something more smooth
	for (k = 0; k < (BFPredefinedNumInterp << 2) ; ++k)
	{
		for (l = 1; l < (BFNumPredefinedPos - 1); ++l)
		{
			PrecomputedPos[l] = 0.5f * (PrecomputedPos[l - 1] + PrecomputedPos[l + 1]);
		}
	}


	// compute the table of speeds, by using on a step of 1.s
	for (l = 1; l < (BFNumPredefinedPos - 1); ++l)
	{
		PrecomputedSpeed[l] = 0.5f * (PrecomputedPos[l + 1] - PrecomputedPos[l - 1]);
	}
	PrecomputedSpeed[BFNumPredefinedPos - 1] = NLMISC::CVector::Null;

	// compute the table of impulsion
	for (k = 0; k < BFNumPrecomputedImpulsions; ++k)
	{
		static double divRand = (2.f / RAND_MAX);
		PrecomputedImpulsions[k].set( (float) (rand() * divRand - 1),
									  (float) (rand() * divRand - 1),
									  (float) (rand() * divRand - 1) 
									);
	}
}

///==========================================================
void CPSBrownianForce::setIntensity(float value)
{
	if (_IntensityScheme)
	{
		CPSForceIntensity::setIntensity(value);
		renewIntegrable(); // integrable again
	}
	else
	{
		CPSForceIntensity::setIntensity(value);
	}
}


///==========================================================
void CPSBrownianForce::setIntensityScheme(CPSAttribMaker<float> *scheme)
{
	if (!_IntensityScheme)
	{
		cancelIntegrable(); // not integrable anymore
	}			
	CPSForceIntensity::setIntensityScheme(scheme);	
}

///==========================================================
void CPSBrownianForce::performDynamic(TAnimationTime ellapsedTime)
{
	// perform the operation on each target	
	for (uint32 k = 0; k < _Owner->getSize(); ++k)
	{		
		float intensity = _IntensityScheme ? _IntensityScheme->get(_Owner, k) : _K;
		for (TTargetCont::iterator it = _Targets.begin(); it != _Targets.end(); ++it)
		{			
			uint32 size = (*it)->getSize();

			if (!size) continue;

			TPSAttribVector::iterator it2 = (*it)->getSpeed().begin(), it2End;
			/// start at a random position in the precomp impulsion tab
			uint startPos = (uint) ::rand() % BFNumPrecomputedImpulsions;
			NLMISC::CVector *imp = PrecomputedImpulsions + startPos;

			if ((*it)->getMassScheme())
			{
				float intensityXtime = intensity * ellapsedTime;
				TPSAttribFloat::const_iterator invMassIt = (*it)->getInvMass().begin();						
				do
				{			
					uint toProcess = std::min((uint) (BFNumPrecomputedImpulsions - startPos), (uint) size);
					it2End = it2 + toProcess;
					do
					{
						float factor = intensityXtime * *invMassIt;
						it2->set(it2->x + factor * imp->x,
								it2->y + factor * imp->y,
								it2->z + factor * imp->x);
						++invMassIt;
						++imp;
						++it2;
					}
					while (it2 != it2End);
					startPos = 0;
					imp = PrecomputedImpulsions;	
					size -= toProcess;
				}
				while (size != 0);
			}
			else
			{				
				do
				{			
					uint toProcess = std::min((uint) (BFNumPrecomputedImpulsions - startPos) , (uint) size);
					it2End = it2 + toProcess;
					float factor = intensity * ellapsedTime * (*it)->getInitialMass();
					do
					{						
						it2->set(it2->x + factor * imp->x,
								it2->y + factor * imp->y,
								it2->z + factor * imp->x);						
						++imp;
						++it2;
					}
					while (it2 != it2End);
					startPos = 0;
					imp = PrecomputedImpulsions;	
					size -= toProcess;
				}
				while (size != 0);
			}
			
		}
	}	
}

///=======================================================================
void CPSBrownianForce::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	sint ver = f.serialVersion(3);		
	if (ver <= 2)
	{
		uint8 dummy;
		f.serial(dummy); // old data in version 2 not used anymore	
		CPSForce::serial(f);
		f.serial(dummy); // old data in version 2 not used anymore	
		serialForceIntensity(f);
		if (f.isReading())
		{
			registerToTargets();
		}
	}

	if (ver >= 2)
	{
		CPSForceIntensityHelper::serial(f);
		f.serial(_ParametricFactor);
	}
}


} // NL3D
