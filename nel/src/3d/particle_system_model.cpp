/** \file particle_system_model.cpp
 * <File description>
 *
 * $Id: particle_system_model.cpp,v 1.3 2001/07/12 15:58:13 vizerie Exp $
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

#include "3d/particle_system_model.h"
#include "3d/particle_system_shape.h"
#include "3d/particle_system.h"
#include "3d/scene.h"
#include "nel/misc/debug.h"

namespace NL3D {

/// ctor
CParticleSystemModel::CParticleSystemModel() : _ParticleSystem(NULL), _EllapsedTime(0.01f), _ToolDisplayEnabled(false)
						, _AutoGetEllapsedTime(true), _TransparencyStateTouched(true)
{
	setOpacity(false) ;
	setTransparency(true) ;
	IAnimatable::resize(AnimValueLast);
}

void CParticleSystemModel::registerBasic()
{
	CMOT::registerModel(ParticleSystemModelId, TransformShapeId, CParticleSystemModel::creator);	
	CMOT::registerObs(AnimDetailTravId, ParticleSystemModelId, CParticleSystemDetailObs::creator) ;
}



void CParticleSystemModel::updateOpacityInfos(void)
{
	nlassert(_ParticleSystem) ;
	if (!_TransparencyStateTouched) return ;
	nlassert(_ParticleSystem) ;
	setOpacity(_ParticleSystem->hasOpaqueObjects() || _ToolDisplayEnabled) ;
	setTransparency(_ParticleSystem->hasTransparentObjects()) ;
	_TransparencyStateTouched = false ;
}

CParticleSystemModel::~CParticleSystemModel()
{
	delete _ParticleSystem ;
}

IAnimatedValue* CParticleSystemModel::getValue (uint valueId)
{
	nlassert(valueId < AnimValueLast) ;
	if (valueId < OwnerBit) return CTransformShape::getValue(valueId) ;
	nlassert(_ParticleSystem) ;
	return _ParticleSystem->getUserParamAnimatedValue(valueId - (uint)  PSParam0) ;	
}

const char *CParticleSystemModel::getPSParamName (uint valueId)
{	
	nlassert(valueId < AnimValueLast) ;
	const char *name[] = { "PSParam0", "PSParam1", "PSParam2", "PSParam3" } ;	
	return name[valueId - (uint) PSParam0] ;
}

const char *CParticleSystemModel::getValueName (uint valueId) const 
{ 
	nlassert(valueId < AnimValueLast) ;
	if (valueId < OwnerBit) return CTransformShape::getValueName(valueId) ;
	return getPSParamName(valueId) ; 
}

ITrack* CParticleSystemModel::getDefaultTrack (uint valueId)
{
	nlassert(valueId < AnimValueLast) ;
	nlassert(Shape) ;
	if (valueId < OwnerBit) return CTransformShape::getDefaultTrack(valueId) ;
	return (NLMISC::safe_cast<CParticleSystemShape *>((IShape *) Shape)->getUserParamDefaultTrack(valueId - (uint) PSParam0)) ;
}

	
void CParticleSystemModel::registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix /* =std::string() */)
{
	CTransformShape::registerToChannelMixer(chanMixer, prefix) ;
	addValue(chanMixer, PSParam0, OwnerBit, prefix, true);
	addValue(chanMixer, PSParam1, OwnerBit, prefix, true);
	addValue(chanMixer, PSParam2, OwnerBit, prefix, true);
	addValue(chanMixer, PSParam3, OwnerBit, prefix, true);	
}

//////////////////////////////////////////////
// CParticleSystemDetailObs implementation  //
//////////////////////////////////////////////


void	CParticleSystemDetailObs ::traverse(IObs *caller)
{
	CTransformAnimDetailObs::traverse(caller);

	// test if bones must be updated.
	nlassert(dynamic_cast<CParticleSystemModel *>(Model)) ;
	CParticleSystemModel *psm= (CParticleSystemModel *)Model;

	
	
		
	CParticleSystem *ps = psm->getPS() ;
	nlassert(ps) ; 
	if (psm->isAutoGetEllapsedTimeEnabled())
	{
		psm->setEllapsedTime(ps->getScene()->getEllapsedTime()) ;
	}
	CAnimationTime delay = psm->getEllapsedTime() ;

	psm->updateOpacityInfos() ;

	ps->setSysMat(psm->getWorldMatrix()) ;
	nlassert(ps->getScene()) ;	

	// animate particles
	ps->step(PSCollision, delay) ;
	ps->step(PSMotion, delay) ;	 		
}


} // NL3D
