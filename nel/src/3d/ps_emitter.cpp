/** \file ps_emitter.cpp
 * <File description>
 *
 * $Id: ps_emitter.cpp,v 1.15 2001/06/27 16:57:29 vizerie Exp $
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

#include "3d/ps_emitter.h"
#include "3d/material.h"
#include "nel/misc/line.h"
#include "3d/dru.h"

namespace NL3D {



// number of emitter to be processed at once
const uint emitterBuffSize = 512 ;

///////////////////////////////
// CPSEmitter implementation //
///////////////////////////////


CPSEmitter::CPSEmitter() : _EmissionType(regular), _Period(0), _EmittedType(NULL)
							   , _PeriodScheme(NULL), _GenNb(1), _GenNbScheme(NULL), _SpeedInheritanceFactor(0.f)
							   , _SpeedBasisEmission(false) 
{
}



CPSEmitter::~CPSEmitter()
{
	// if a located is emitted, unregister us as an observer
	if (_EmittedType)
	{
		_EmittedType->unregisterDtorObserver(this) ;
	}
}

void CPSEmitter::setEmittedType(CPSLocated *et) 
{
	if (_EmittedType)
	{
		_EmittedType->unregisterDtorObserver(this) ;
	}
	if (et)
	{
		et->registerDtorObserver(this) ;
	}	

	_EmittedType = et ;
}


void CPSEmitter::notifyTargetRemoved(CPSLocated *ptr)
{
	nlassert(ptr == _EmittedType) ;
	_EmittedType = NULL ;
}

void CPSEmitter::setPeriod(float period)
{
	if (_PeriodScheme)
	{
		delete _PeriodScheme ;
		_PeriodScheme = NULL ;
	}
	_Period = period ;
}


void CPSEmitter::setPeriodScheme(CPSAttribMaker<float> *scheme)
{
	
	 delete _PeriodScheme ;	
	_PeriodScheme = scheme ;
}



void CPSEmitter::setGenNb(uint32 GenNb)
{
	if (_GenNbScheme)
	{
		delete _GenNbScheme ;
		_GenNbScheme = NULL ;
	}
	_GenNb = GenNb ;
}


void CPSEmitter::setGenNbScheme(CPSAttribMaker<uint32> *scheme)
{
	delete _GenNbScheme ;	
	_GenNbScheme = scheme ;
}



void CPSEmitter::showTool(void) 
{
	uint32 size = _Owner->getSize() ;
	if (!size) return ;		
	setupDriverModelMatrix() ;	

	const CVector I = computeI() ;
	const CVector K = computeK() ;

	// ugly slow code, but not for runtime
	for (uint  k = 0 ; k < size ; ++k)
	{
		// center of the current particle
		const CVector p = _Owner->getPos()[k]  ;
		const float sSize =0.1f ;	
		std::vector<NLMISC::CLine> lines ;
		NLMISC::CLine l ;
		l.V0 = p - sSize * I ; l.V1 =  p + sSize * I ; lines.push_back(l) ;
		l.V0 = p - sSize * K ; l.V1 =  p + sSize * K ; lines.push_back(l) ;
		l.V0 = p - sSize * (I + K) ; l.V1 = p + sSize * (I + K) ; lines.push_back(l) ;
		l.V0 = p - sSize * (I - K) ; l.V1 = p + sSize * (I - K) ; lines.push_back(l) ;
							
											
		
		
	
		CMaterial mat ;

		mat.setBlendFunc(CMaterial::one, CMaterial::one) ;
		mat.setZWrite(false) ;
		mat.setLighting(false) ;
		mat.setBlend(true) ;
		mat.setZFunc(CMaterial::less) ;
		
	

		CPSLocated *loc ;		
		uint32 index ;
		CPSLocatedBindable *lb ;
		_Owner->getOwner()->getCurrentEditedElement(loc, index, lb) ;

		mat.setColor((lb == NULL || this == lb) && loc == _Owner && index == k  ? CRGBA::Red : CRGBA(127, 127, 127)) ;
		

		CDRU::drawLinesUnlit(lines, mat, *getDriver() ) ;
	}

}

void CPSEmitter::step(TPSProcessPass pass, CAnimationTime ellapsedTime)
{
if (pass == PSToolRender)
{
	showTool() ;
	return ;
}
if (pass != PSMotion) return ;
const uint32 size = _Owner->getSize() ;
if (!size) return ;

if (ellapsedTime == 0.f) return ; // do nothing when paused

// our behaviour depend of the frequency
switch (_EmissionType)
{
	case CPSEmitter::once :
	{
		TPSAttribTime::iterator timeIt = _Phase.begin()
									  , timeEndIt = _Phase.end() ;

		while (timeIt != timeEndIt)
		{
			if (*timeIt == 0.f)
			{
				const uint32 nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, timeIt - _Phase.begin()) : _GenNb ;		
				processEmit(timeIt - _Phase.begin(), nbToGenerate) ;		
				*timeIt = 1.f ;
			}
			++timeIt ;
		}
		
	}
	break ;
	case (CPSEmitter::regular):
	{
		uint leftToDo = size , toProcess ;
	

		float emitPeriod[emitterBuffSize] ; 

		float *currEmitPeriod ;
		uint currEmitPeriodPtrInc = _PeriodScheme ? 1 : 0 ;

		
		uint32 nbToGenerate ;


		TPSAttribTime::iterator phaseIt = _Phase.begin(), endPhaseIt ; 

		// we don't use an iterator here
		// because it could be invalidated if size change (a located ould generate itself)	

		do
		{
			toProcess = leftToDo < emitterBuffSize ? leftToDo : emitterBuffSize ;


			if (_PeriodScheme)
			{
				_PeriodScheme->make(_Owner, size - leftToDo, emitPeriod, sizeof(float), toProcess) ;
				currEmitPeriod = &emitPeriod[0] ;
			}
			else
			{
				currEmitPeriod = &_Period ;
			}

			endPhaseIt = phaseIt + toProcess ;

			do			
			{
				*phaseIt += ellapsedTime ;
				if ( *phaseIt >= *currEmitPeriod) // phase is greater than period -> must emit
				{
					*phaseIt -= *currEmitPeriod ;
					const uint32 k = phaseIt - (_Phase.begin()) ;
					nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, k) : _GenNb ;					
					processEmit(k, nbToGenerate) ;										
				}	
				
				++phaseIt ;
				currEmitPeriod += currEmitPeriodPtrInc ;
			}
			while (phaseIt != endPhaseIt) ;

			leftToDo -= toProcess ;
		}
		while (leftToDo) ;
	}
	break ;	
}
}


void CPSEmitter::newElement(void)
{	
	nlassert(_Phase.getSize() != _Phase.getMaxSize()) ;	

	_Phase.insert(0.f) ;


}

void CPSEmitter::deleteElement(uint32 index)
{	
	if (_EmissionType == CPSEmitter::onDeath)
	{
		const uint32 nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, index) : _GenNb ;		
		processEmit(index, nbToGenerate) ;		
	}		

	_Phase.remove(index) ;
}

void CPSEmitter::resize(uint32 size)
{
	_Phase.resize(size) ;
}

void CPSEmitter::bounceOccured(uint32 index)
{
	// TODO : avoid duplication with deleteElement
	if (_EmissionType == CPSEmitter::onBounce)
	{
		const uint32 nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, index) : _GenNb ;		
		processEmit(index, nbToGenerate) ;
	}		
}


void CPSEmitter::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialCheck((uint32) 'EMIT') ;
	CPSLocatedBindable::serial(f) ;
	f.serialVersion(1) ;	
	f.serialPolyPtr(_EmittedType) ;
	f.serial(_Phase) ;
	f.serial(_SpeedInheritanceFactor) ;
	f.serial(_SpeedBasisEmission) ;
	
	f.serialEnum(_EmissionType) ;

	// this is for use with serial
	bool trueB = true, falseB = false ;

	if (!f.isReading())
	{
		switch (_EmissionType)
		{
			case CPSEmitter::regular:			
				if (_PeriodScheme)
				{
					f.serial(trueB) ;
					f.serialPolyPtr(_PeriodScheme) ;
				}
				else
				{
					 f.serial(falseB) ;
					 f.serial(_Period) ;
				}
				if (_GenNbScheme)
				{
					f.serial(trueB) ;
					f.serialPolyPtr(_GenNbScheme) ;
				}
				else
				{
					 f.serial(falseB) ;
					 f.serial(_GenNb) ;
				}
			break ;
			default:
			break ;
		}
	}
	else
	{
		switch (_EmissionType)
		{
			case CPSEmitter::regular:
			{
				bool useScheme ;
				f.serial(useScheme) ;
				if (useScheme)
				{
					delete _PeriodScheme ;					
					f.serialPolyPtr(_PeriodScheme) ;
				}
				else
				{					 
					 f.serial(_Period) ;
				}
				f.serial(useScheme) ; 
				if (useScheme)
				{
					delete _GenNbScheme ;
					f.serialPolyPtr(_GenNbScheme) ;
				}
				else
				{				
					 f.serial(_GenNb) ;
				}
			}
			break ;
			default:
			break ;
		}
	}
}



void CPSEmitterOmni::emit(uint32 index, CVector &pos, CVector &speed)
{
	// TODO : verifier que ca marche si une particule s'emet elle-mem
	nlassert(_EmittedType) ;	
	
	CVector v( ((rand() % 1000) -500) / 500.0f
				   , ((rand() % 1000) -500) / 500.0f
				   , ((rand() % 1000) -500) / 500.0f) ;
	v.normalize() ;
	v *= _EmitteeSpeedScheme ? _EmitteeSpeedScheme->get(_Owner, index) : _EmitteeSpeed ;		

	pos = _Owner->getPos()[index] ;
	speed = v ;	
}



void CPSEmitterOmni::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	CPSEmitter::serial(f) ;
	CPSModulatedEmitter::serialEmitteeSpeedScheme(f) ;
}



void CPSEmitterDirectionnal::emit(uint32 index, CVector &pos, CVector &speed)
{
	// TODO : verifier que ca marche si une particule s'emet elle-mem
	nlassert(_EmittedType) ;	
	

	speed = (_EmitteeSpeedScheme ? _EmitteeSpeedScheme->get(_Owner, index) : _EmitteeSpeed) * _Dir ;		
	pos = _Owner->getPos()[index] ;	
}



void CPSEmitterDirectionnal::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	CPSEmitter::serial(f) ;
	CPSModulatedEmitter::serialEmitteeSpeedScheme(f) ;
	f.serial(_Dir) ;
}


////////////////////////////////////////////
// implementation of CPSEmitterRectangle  //
////////////////////////////////////////////

void CPSEmitterRectangle::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	CPSEmitter::serial(f) ;
	CPSModulatedEmitter::serialEmitteeSpeedScheme(f) ;
	f.serial(_Basis) ;	
	f.serial(_Width) ;
	f.serial(_Height) ;
	f.serial(_Dir) ;

}


void CPSEmitterRectangle::emit(uint32 index, CVector &pos, CVector &speed)
{
	CVector N = _Basis[index].X ^ _Basis[index].Y ;
	pos = _Owner->getPos()[index] + ((rand() % 32000) * (1.f / 16000) - 1.f) *  _Width[index] *  _Basis[index].X 
								  + ((rand() % 32000) * (1.f / 16000) - 1.f) *  _Height[index] * _Basis[index].Y ;
	speed = (_EmitteeSpeedScheme ? _EmitteeSpeedScheme->get(_Owner, index) : _EmitteeSpeed) 
					* (_Dir.x * _Basis[index].X+ _Dir.y * _Basis[index].Y + _Dir.z *  N) ;
}






void CPSEmitterRectangle::setMatrix(uint32 index, const CMatrix &m)
{
	_Owner->getPos()[index] = m.getPos() ;

	
	 _Basis[index].X = m.getI() ;
	 _Basis[index].Y = m.getJ() ;
}

CMatrix CPSEmitterRectangle::getMatrix(uint32 index) const
{
	CMatrix m ;
	m.setPos(_Owner->getPos()[index]) ;
	m.setRot(_Basis[index].X, _Basis[index].Y, _Basis[index].X ^ _Basis[index].Y, true) ;
	return m ;
}



void CPSEmitterRectangle::setScale(uint32 index, float scale)
{
	_Width[index] = scale ;
	_Height[index] = scale ;
}
void CPSEmitterRectangle::setScale(uint32 index, const CVector &s)
{
	_Width[index] = s.x ;
	_Height[index] = s.y ;
}
CVector CPSEmitterRectangle::getScale(uint32 index) const
{	
	return CVector(_Width[index], _Height[index], 1.f) ; 
}

		
void CPSEmitterRectangle::newElement(void)
{
	CPSEmitter::newElement() ;
	_Basis.insert(CPlaneBasis(CVector::K)) ;
	_Width.insert(1.f) ;
	_Height.insert(1.f) ;
}

	
void CPSEmitterRectangle::deleteElement(uint32 index)
{
	CPSEmitter::deleteElement(index) ;
	_Basis.remove(index) ;
	_Width.remove(index) ;
	_Height.remove(index) ;
}

void CPSEmitterRectangle::resize(uint32 size)
{
	CPSEmitter::resize(size) ;
	_Basis.resize(size) ;
	_Width.resize(size) ;
	_Height.resize(size) ;
}

void CPSEmitterRectangle::showTool(void)
{
	nlassert(_Owner) ;
	const uint size = _Owner->getSize() ;
	if (!size) return ;
	setupDriverModelMatrix() ;	
	CMatrix mat ;
	
	CPSLocated *loc ;
	uint32 index ;
	CPSLocatedBindable *lb ;
	_Owner->getOwner()->getCurrentEditedElement(loc, index, lb) ;

	for (uint k = 0 ; k < size ; ++k) 
	{	
		const CVector &I = _Basis[k].X ;
		const CVector &J = _Basis[k].Y ;
		mat.setRot(I, J , I ^J) ;
		mat.setPos(_Owner->getPos()[k]) ;
		CPSUtil::displayBasis(getDriver() ,getLocatedMat(), mat, 1.f, *getFontGenerator(), *getFontManager()) ;				
		setupDriverModelMatrix() ;	

		const CRGBA col = ((lb == NULL || this == lb) && loc == _Owner && index == k  ? CRGBA::Red : CRGBA(127, 127, 127)) ;
	


		const CVector &pos = _Owner->getPos()[k] ;
		CPSUtil::display3DQuad(*getDriver(), pos + I * _Width[k] + J * _Height[k]
										   , pos + I * _Width[k] - J * _Height[k]
										   , pos - I * _Width[k] - J * _Height[k]
										   , pos - I * _Width[k] + J * _Height[k], col) ;
	}
}



////////////////////////////////////
// CPSEmitterconic implementation //
////////////////////////////////////


void CPSEmitterConic::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	CPSEmitterDirectionnal::serial(f) ;
	f.serial(_Radius) ;
	if (f.isReading())
	{
		// rebuild the precomputed basis
		setDir(_Dir) ;
	}
}
	


void CPSEmitterConic::setDir(const CVector &v)
{
	CPSEmitterDirectionnal::setDir(v) ;
	
}


void CPSEmitterConic::emit(uint32 index, CVector &pos, CVector &speed)
{
	// TODO : optimize that
	nlassert(_EmittedType) ;	
	
	// we choose a custom direction like with omnidirectionnal emitter
	// then we force the direction vect to have the unit size

	CVector dir = 		(((rand() % 32000) / 16000.f) - 1.f) * CVector::I 
					  + (((rand() % 32000) / 16000.f) - 1.f) * CVector::J
					  + (((rand() % 32000) / 16000.f) - 1.f) * CVector::K
					 ;
	const float n =dir.norm() ;

	dir *= _Radius / n ;

	dir -= (_Dir * dir) * _Dir ;
	dir += _Dir ;
	dir.normalize() ;
	
	
	speed = (_EmitteeSpeedScheme ? _EmitteeSpeedScheme->get(_Owner, index) : _EmitteeSpeed) 
		    * dir ;
	pos = _Owner->getPos()[index] ;	
}



} // NL3D
