/** \file ps_force.cpp
 * <File description>
 *
 * $Id: ps_force.cpp,v 1.16 2001/09/04 16:14:50 vizerie Exp $
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

#include "3d/ps_force.h"
#include "3d/driver.h"
#include "3d/primitive_block.h"
#include "3d/material.h"
#include "3d/vertex_buffer.h"
#include "3d/computed_string.h"
#include "3d/font_generator.h"
#include "3d/font_manager.h"
#include "3d/particle_system.h"
#include "nel/misc/common.h"
#include "3d/ps_util.h"

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




void CPSForce::step(TPSProcessPass pass, CAnimationTime ellapsedTime)
{
	switch(pass)
	{
		case PSMotion:
			performMotion(ellapsedTime);
		break;
		case PSToolRender:
			show(ellapsedTime);
		break;
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
// CPSDirectionalForce implementation //
////////////////////////////////////////


void CPSDirectionnalForce::performMotion(CAnimationTime ellapsedTime)
{
	// perform the operation on each target

	CVector toAdd;


	for (uint32 k = 0; k < _Owner->getSize(); ++k)
	{	
		CVector toAddLocal = ellapsedTime * (_IntensityScheme ? _IntensityScheme->get(_Owner, k) : _K ) * _Dir;
		for (TTargetCont::iterator it = _Targets.begin(); it != _Targets.end(); ++it)
		{

			toAdd = CPSLocated::getConversionMatrix(*it, this->_Owner).mulVector(toAddLocal); // express this in the target basis			

			uint32 size = (*it)->getSize();	
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


void CPSDirectionnalForce::show(CAnimationTime ellapsedTime)
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


void CPSGravity::performMotion(CAnimationTime ellapsedTime)
{	
	
	// perform the operation on each target

	CVector toAdd;


	for (uint32 k = 0; k < _Owner->getSize(); ++k)
	{	
		CVector toAddLocal = ellapsedTime * CVector(0, 0, _IntensityScheme ? - _IntensityScheme->get(_Owner, k) : - _K);
		for (TTargetCont::iterator it = _Targets.begin(); it != _Targets.end(); ++it)
		{

			toAdd = CPSLocated::getConversionMatrix(*it, this->_Owner).mulVector(toAddLocal); // express this in the target basis			
			uint32 size = (*it)->getSize();	

			


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
void CPSGravity::show(CAnimationTime ellapsedTime) 
{	
	CVector I = computeI();
	CVector K = CVector(0,0,1);	    

	// this is not designed for efficiency (target : edition code)
	CPrimitiveBlock	 pb;
	CVertexBuffer vb;
	CMaterial material;
	IDriver *driver = getDriver();
	const float toolSize = 0.2f;

	vb.setVertexFormat(IDRV_VF_XYZ);
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


/////////////////////////////////////////
// CPSCentralGravity  implementation   //
/////////////////////////////////////////


void CPSCentralGravity::performMotion(CAnimationTime ellapsedTime)
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


void CPSCentralGravity::show(CAnimationTime ellapsedTime)
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



void CPSSpring::performMotion(CAnimationTime ellapsedTime)
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



void CPSSpring::show(CAnimationTime ellapsedTime)
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

void CPSCylindricVortex::performMotion(CAnimationTime ellapsedTime)
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


void CPSCylindricVortex::show(CAnimationTime ellapsedTime)
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
		CMatrix m = CPSUtil::buildSchmidtBasis(_Normal[k]);
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
	CMatrix m  = CPSUtil::buildSchmidtBasis(_Normal[index]);
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
	CPSForceIntensityHelper::resize(size); 
	_Normal.resize(size);
	_Radius.resize(size);
}


} // NL3D
