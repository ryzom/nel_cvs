 /** \file particle_system.cpp
 * <File description>
 *
 * $Id: particle_system.cpp,v 1.53 2002/10/14 09:48:57 vizerie Exp $
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

#include "3d/particle_system.h"
#include "3d/ps_located.h"
#include "3d/driver.h"
#include "3d/vertex_buffer.h"
#include "3d/material.h"
#include "3d/primitive_block.h"
#include "3d/nelu.h"
#include "3d/ps_util.h"
#include "3d/ps_particle.h"
#include "3d/ps_sound.h"
#include "3d/particle_system_shape.h"
#include "nel/misc/aabbox.h"
#include "nel/misc/file.h"
#include "nel/misc/stream.h"




namespace NL3D 
{




uint32 CParticleSystem::NbParticlesDrawn = 0;
UPSSoundServer *		 CParticleSystem::_SoundServer = NULL;





///////////////////////////////////
// CPaticleSystem implementation //
///////////////////////////////////


/// the default max distance of view for particle systems
const float PSDefaultMaxViewDist = 300.f;

/*
 * Constructor
 */
CParticleSystem::CParticleSystem() : _Driver(NULL),
									 _FontGenerator(NULL),
									 _FontManager(NULL),
									 _Date(0),
									 _LastUpdateDate(-1),
									 _CurrEditedElementLocated(NULL),
									 _CurrEditedElementIndex(0),
									 _Scene(NULL),
									 _TimeThreshold(0.15f),
									 _SystemDate(0.f),
									 _MaxNbIntegrations(2),									 
									 _LODRatio(0.5f),									 
									 _OneMinusCurrentLODRatio(0),
									 _MaxViewDist(PSDefaultMaxViewDist),
									 _InvMaxViewDist(1.f / PSDefaultMaxViewDist),
									 _InvCurrentViewDist(1.f / PSDefaultMaxViewDist),
									 _DieCondition(none),
									 _DelayBeforeDieTest(0.2f),									 									
									 _MaxNumFacesWanted(0),
									 _AnimType(AnimInCluster),
									 _PresetBehaviour(UserBehaviour),									 
									 _AutoLODStartDistPercent(0.3f),
									 _AutoLODDegradationExponent(1),																		 
									 _ColorAttenuationScheme(NULL),
									 _GlobalColor(NLMISC::CRGBA::White),
									 _ComputeBBox(true),
									 _BBoxTouched(true),
									 _AccurateIntegration(true),
									 _CanSlowDown(true),
									 _DestroyModelWhenOutOfRange(false),
									 _DestroyWhenOutOfFrustum(false),
									 _Sharing(false),
									 _AutoLOD(false),
									 _KeepEllapsedTimeForLifeUpdate(false),
									 _AutoLODSkipParticles(false),
									 _EnableLoadBalancing(true)
{
	for (uint k = 0; k < MaxPSUserParam; ++k) _UserParam[k] = 0;
}


///=======================================================================================
/// immediatly shut down all the sound in this system
void CParticleSystem::stopSound()
{
	for (uint k = 0; k < this->getNbProcess(); ++k)
	{
		CPSLocated *psl = dynamic_cast<NL3D::CPSLocated *>(this->getProcess(k));
		if (psl)
		{
			for (uint l = 0; l < psl->getNbBoundObjects(); ++l)
			{
				if (psl->getBoundObject(l)->getType() == PSSound)
				{
					static_cast<CPSSound *>(psl->getBoundObject(l))->stopSound();

				}
			}
		}
	}	
}

///=======================================================================================
void CParticleSystem::reactivateSound()
{
	for (uint k = 0; k < this->getNbProcess(); ++k)
	{
		CPSLocated *psl = dynamic_cast<NL3D::CPSLocated *>(this->getProcess(k));
		if (psl)
		{
			for (uint l = 0; l < psl->getNbBoundObjects(); ++l)
			{
				if (psl->getBoundObject(l)->getType() == NL3D::PSSound)
				{
					static_cast<CPSSound *>(psl->getBoundObject(l))->reactivateSound();
				}
			}
		}
	}
}


///=======================================================================================
void CParticleSystem::enableLoadBalancing(bool enabled /*=true*/)
{
	if (enabled)
	{
		notifyMaxNumFacesChanged();
	}
	_EnableLoadBalancing = enabled;
}

///=======================================================================================
void CParticleSystem::notifyMaxNumFacesChanged(void)
{
	if (!_EnableLoadBalancing) return;
	_MaxNumFacesWanted = 0;	
	for (TProcessVect::iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
	{				
		_MaxNumFacesWanted += (*it)->querryMaxWantedNumFaces();
	}
}


///=======================================================================================
float CParticleSystem::getWantedNumTris(float dist)
{
	if (!_EnableLoadBalancing) return 0; // no contribution to the load balancing
	if (dist > _MaxViewDist) return 0;
	float retValue = ((1.f - dist * _InvMaxViewDist) * _MaxNumFacesWanted);	
	///nlassertex(retValue >= 0 && retValue < 10000, ("dist = %f, _MaxViewDist = %f, _MaxNumFacesWanted = %d, retValue = %f",  dist, _MaxViewDist, _MaxNumFacesWanted, retValue));
	return retValue;
}


///=======================================================================================
void CParticleSystem::setNumTris(uint numFaces)
{
	if (_EnableLoadBalancing)
	{	
		float modelDist = (_SysMat.getPos() - _InvertedViewMat.getPos()).norm();
		/*uint numFaceWanted = (uint) getWantedNumTris(modelDist);*/

		const float epsilon = 10E-5f;


		uint wantedNumTri = (uint) getWantedNumTris(modelDist);
		if (numFaces >= wantedNumTri || wantedNumTri == 0 || _MaxNumFacesWanted == 0 || modelDist < epsilon)
		{ 
			_InvCurrentViewDist = _InvMaxViewDist;
		}
		else
		{
			
			_InvCurrentViewDist = (_MaxNumFacesWanted - numFaces) / ( _MaxNumFacesWanted * modelDist);
		}
	}
	else
	{
		// always take full detail when there's no load balancing
		_InvCurrentViewDist = _InvMaxViewDist;
	}
}


///=======================================================================================
/// dtor
CParticleSystem::~CParticleSystem()
{
 	for (TProcessVect::iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
	{
		delete *it;
	}
	delete _ColorAttenuationScheme;
}

///=======================================================================================
void CParticleSystem::setViewMat(const NLMISC::CMatrix &m)
{
	_ViewMat = m;
	_InvertedViewMat = m.inverted();
}				

///=======================================================================================
bool CParticleSystem::hasEmitters(void) const
{
	for (TProcessVect::const_iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
	{
		if ((*it)->hasEmitters()) return true;
	}
	return false;
}

///=======================================================================================
bool CParticleSystem::hasParticles(void) const
{
	for (TProcessVect::const_iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
	{
		if ((*it)->hasParticles()) return true;
	}
	return false;
}

///=======================================================================================
void CParticleSystem::stepLocated(TPSProcessPass pass, TAnimationTime et, TAnimationTime realEt)
{	
	for (TProcessVect::iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
	{
		(*it)->step(pass, et, realEt);
	}	
}


///=======================================================================================
inline void CParticleSystem::updateLODRatio()
{
	// temp
	CVector sysPos = _SysMat.getPos();
	CVector obsPos = _InvertedViewMat.getPos();
	const CVector d = _SysMat.getPos() - _InvertedViewMat.getPos();		
	_OneMinusCurrentLODRatio = 1.f - (d.norm() * _InvCurrentViewDist);
	NLMISC::clamp(_OneMinusCurrentLODRatio, 0.f, 1.f);
}

///=======================================================================================
inline void CParticleSystem::updateColor()
{
	if (_ColorAttenuationScheme)
	{
		float ratio = 1.00f - _OneMinusCurrentLODRatio;
		_GlobalColor = 	_ColorAttenuationScheme->get(ratio > 0.f ? ratio : 0.f);
	}
}

///=======================================================================================
void CParticleSystem::step(TPass pass, TAnimationTime ellapsedTime)
{		
	switch (pass)
	{
		case SolidRender:
			/// When shared, the LOD ratio must be computed there
			if (_Sharing)
			{
				updateLODRatio();
			}
			// update time
			++_Date; 	
			// update global color
			updateColor();
			stepLocated(PSSolidRender, ellapsedTime, ellapsedTime);

		break;
		case BlendRender:			
			/// When shared, the LOD ratio must be computed there
			if (_Sharing)
			{
				updateLODRatio();
			}
			// update time
			++_Date; 
			// update global color
			updateColor();			
			stepLocated(PSBlendRender, ellapsedTime, ellapsedTime);
		break;
		case ToolRender:
			stepLocated(PSToolRender, ellapsedTime, ellapsedTime);
		break;
		case Anim:
		{
		
			_BBoxTouched = true;
			TAnimationTime et = ellapsedTime;
			uint nbPass = 1;
			if (_AccurateIntegration)
			{
				if (et > _TimeThreshold)
				{
					nbPass = (uint32) ceilf(et / _TimeThreshold);
					if (nbPass > _MaxNbIntegrations)
					{ 
						nbPass = _MaxNbIntegrations;
						et = _CanSlowDown ? _TimeThreshold : (ellapsedTime / nbPass);
					}
					else
					{
						et = ellapsedTime / nbPass;
					}
				}			
			}
			updateLODRatio();

			// process passes
			float realEt = _KeepEllapsedTimeForLifeUpdate ? (ellapsedTime / nbPass)
														  : et;
			do
			{					
				// the order of the following is important...
				stepLocated(PSCollision, et,  realEt);
				for (TProcessVect::iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
				{
					(*it)->updateLife(realEt);
					(*it)->step(PSMotion, et, realEt);					
				}
				_SystemDate += realEt;
				stepLocated(PSEmit, et,  realEt);								
			}
			while (--nbPass);
			
			// perform parametric motion if present
			for (TProcessVect::iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
			{
				if ((*it)->isParametricMotionEnabled()) (*it)->performParametricMotion(_SystemDate, ellapsedTime, realEt);
			}				
			
		}
	}			
}



///=======================================================================================
void CParticleSystem::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{		
	sint version =  f.serialVersion(11);			
	// version 11: enable load balancing flag 
	// version 9: Sharing flag added
	//            Auto-lod parameters
	//            New integration flag
	//			  Global color attenuation
	// version 8: Replaced the attribute '_PerformMotionWhenOutOfFrustum' by a _AnimType field which allow more precise control

	//f.serial(_ViewMat);
	f.serial(_SysMat);
	f.serial(_Date);
	if (f.isReading())
	{
		delete _ColorAttenuationScheme;
		// delete previous multimap
		_LBMap.clear();
		// delete previously attached process
		for (TProcessVect::iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
		{
			delete (*it);
		}

		_ProcessVect.clear();

		f.serialContPolyPtr(_ProcessVect);		
	
		_InvSysMat = _SysMat.inverted();
		_FontGenerator = NULL;
		_FontManager = NULL;
	}
	else
	{
		f.serialContPolyPtr(_ProcessVect);	
	}
	
	if (version > 1) // name of the system
	{
		f.serial(_Name);
	}

	if (version > 2) // infos about integration, and LOD
	{
		f.serial(_AccurateIntegration);
		if (_AccurateIntegration) f.serial(_CanSlowDown, _TimeThreshold, _MaxNbIntegrations);
		f.serial(_InvMaxViewDist, _LODRatio);	
		_MaxViewDist = 1.f / _InvMaxViewDist;
		_InvCurrentViewDist = _InvMaxViewDist;
	}

	if (version > 3) // tell wether the system must compute his bbox, hold a precomputed bbox
	{
		f.serial(_ComputeBBox);
		if (!_ComputeBBox)
		{
			f.serial(_PreComputedBBox);
		}
	}

	if (version > 4) // lifetime informations
	{
		f.serial(_DestroyModelWhenOutOfRange);
		f.serialEnum(_DieCondition);
		if (_DieCondition != none)
		{
			f.serial(_DelayBeforeDieTest);
		}
	}	

	if (version > 5)
	{
		f.serial(_DestroyWhenOutOfFrustum);
	}

	if (version > 6 && version < 8)
	{
		bool performMotionWOOF;
		if (f.isReading())
		{
			f.serial(performMotionWOOF);
			performMotionWhenOutOfFrustum(performMotionWOOF);
		}
		else
		{
			performMotionWOOF = doesPerformMotionWhenOutOfFrustum();
			f.serial(performMotionWOOF);
		}
	}

	if (version > 7)
	{
		f.serialEnum(_AnimType);
		f.serialEnum(_PresetBehaviour);
	}

	if (version > 8)
	{
		f.serial(_Sharing);
		f.serial(_AutoLOD);
		if (_AutoLOD)
		{
			f.serial(_AutoLODStartDistPercent, _AutoLODDegradationExponent);
			f.serial(_AutoLODSkipParticles);
		}
		f.serial(_KeepEllapsedTimeForLifeUpdate);
		f.serialPolyPtr(_ColorAttenuationScheme);
	}

	if (version >= 11)
	{
		f.serial(_EnableLoadBalancing);
	}
	

	if (f.isReading())
	{
		notifyMaxNumFacesChanged();
	}
}


///=======================================================================================
void CParticleSystem::attach(CParticleSystemProcess *ptr)
{
	nlassert(std::find(_ProcessVect.begin(), _ProcessVect.end(), ptr) == _ProcessVect.end() );
	//nlassert(ptr->getOwner() == NULL); // deja attache a un autre systeme
	_ProcessVect.push_back(ptr);
	ptr->setOwner(this);
	notifyMaxNumFacesChanged();
}

///=======================================================================================
void CParticleSystem::remove(CParticleSystemProcess *ptr)
{
	TProcessVect::iterator it = std::find(_ProcessVect.begin(), _ProcessVect.end(), ptr);
	nlassert(it != _ProcessVect.end() );	
	_ProcessVect.erase(it);
	delete ptr;
}

///=======================================================================================
void CParticleSystem::computeBBox(NLMISC::CAABBox &aabbox)
{
	if (!_ComputeBBox || !_BBoxTouched)
	{
		aabbox = _PreComputedBBox;
		return;
	}

	bool foundOne = false;
	NLMISC::CAABBox tmpBox;
	for (TProcessVect::const_iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
	{
		if ((*it)->computeBBox(tmpBox))
		{
			if (!(*it)->isInSystemBasis())
			{
				// rotate the aabbox so that it is in the correct basis
				tmpBox = NLMISC::CAABBox::transformAABBox(_InvSysMat, tmpBox);
			}
			if (foundOne)
			{
				aabbox = NLMISC::CAABBox::computeAABBoxUnion(aabbox, tmpBox);
			}
			else
			{
				aabbox = tmpBox;
				foundOne = true;
			}
		}
	}

	if (!foundOne)
	{
		aabbox.setCenter(NLMISC::CVector::Null);
		aabbox.setHalfSize(NLMISC::CVector::Null);
	}
	
	_BBoxTouched = false;
	_PreComputedBBox = aabbox;
}

///=======================================================================================
void CParticleSystem::setSysMat(const CMatrix &m)
{
	_SysMat = m;
	_InvSysMat = _SysMat.inverted();
}

///=======================================================================================
bool CParticleSystem::hasOpaqueObjects(void) const
{
	/// for each process
	for (TProcessVect::const_iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
	{
		if (dynamic_cast<CPSLocated *>(*it))
		{
			for (uint k = 0; k < ((CPSLocated *) *it)->getNbBoundObjects(); ++k)
			{
				CPSLocatedBindable *lb = ((CPSLocated *) *it)->getBoundObject(k);
				if (lb->getType() == PSParticle)
				{
					if (((CPSParticle *) lb)->hasOpaqueFaces()) return true;
				}
			}
		}
	}
	return false;
}

///=======================================================================================
bool CParticleSystem::hasTransparentObjects(void) const
{
	/// for each process
	for (TProcessVect::const_iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
	{
		if (dynamic_cast<CPSLocated *>(*it))
		{
			for (uint k = 0; k < ((CPSLocated *) *it)->getNbBoundObjects(); ++k)
			{
				CPSLocatedBindable *lb = ((CPSLocated *) *it)->getBoundObject(k);
				if (lb->getType() == PSParticle)
				{
					if (((CPSParticle *) lb)->hasTransparentFaces()) return true;
				}
			}
		}
	}
	return false;
}

///=======================================================================================
void CParticleSystem::getLODVect(NLMISC::CVector &v, float &offset,  bool systemBasis)
{
	if (!systemBasis)
	{
		v = _InvCurrentViewDist * _InvertedViewMat.getJ();
		offset = - _InvertedViewMat.getPos() * v;
	}
	else
	{
		const CVector tv = _InvSysMat.mulVector(_InvertedViewMat.getJ());
		const CVector org = _InvSysMat * _InvertedViewMat.getPos();
		v = _InvCurrentViewDist * tv;
		offset = - org * v;
	}
}

///=======================================================================================
TPSLod CParticleSystem::getLOD(void) const
{
	const float dist = fabsf(_InvCurrentViewDist * (_SysMat.getPos() - _InvertedViewMat.getPos()) * _InvertedViewMat.getJ());
	return dist > _LODRatio ? PSLod2 : PSLod1;
}


///=======================================================================================
void CParticleSystem::registerLocatedBindableExternID(uint32 id, CPSLocatedBindable *lb)
{
	nlassert(lb);
	nlassert(lb->getOwner() && lb->getOwner()->getOwner() == this); // the located bindable must belong to that system
	#ifdef NL_DEBUG		
		// check that this lb hasn't been inserted yet
		TLBMap::iterator lbd = _LBMap.lower_bound(id), ubd = _LBMap.upper_bound(id);
		nlassert(std::find(lbd, ubd, TLBMap::value_type (id, lb)) == ubd);
		nlassert(std::find(lbd, ubd, TLBMap::value_type (id, lb)) == ubd );

	#endif
		_LBMap.insert(TLBMap::value_type (id, lb) );
}

///=======================================================================================
void CParticleSystem::unregisterLocatedBindableExternID(CPSLocatedBindable *lb)
{
	nlassert(lb);	
	nlassert(lb->getOwner() && lb->getOwner()->getOwner() == this); // the located bindable must belong to that system
	uint32 id = lb->getExternID();
	if (!id) return;
	TLBMap::iterator lbd = _LBMap.lower_bound(id), ubd = _LBMap.upper_bound(id);
	TLBMap::iterator el = std::find(lbd, ubd, TLBMap::value_type (id, lb));
	nlassert(el != ubd); 
	_LBMap.erase(el);
}

///=======================================================================================
uint CParticleSystem::getNumLocatedBindableByExternID(uint32 id) const
{
	return _LBMap.count(id);
}

///=======================================================================================
CPSLocatedBindable *CParticleSystem::getLocatedBindableByExternID(uint32 id, uint index)
{
	if (index >= _LBMap.count(id))
	{
		return NULL;
	}
	TLBMap::const_iterator el = _LBMap.lower_bound(id);
	uint left = index;
	while (left--) ++el;
	return  el->second;

}

///=======================================================================================
const CPSLocatedBindable *CParticleSystem::getLocatedBindableByExternID(uint32 id, uint index) const
{
	if (index >= _LBMap.count(id))
	{
		return NULL;
	}
	TLBMap::const_iterator el = _LBMap.lower_bound(id);
	uint left = index;
	while (left--) ++el;
	return  el->second;
}

///=======================================================================================
void CParticleSystem::merge(CParticleSystemShape *pss)
{
	nlassert(pss);	
	CParticleSystem *duplicate = pss->instanciatePS(*this->_Scene); // duplicate the p.s. to merge
	// now we transfer the located of the duplicated ps to this object...
	for (TProcessVect::iterator it = duplicate->_ProcessVect.begin(); it != duplicate->_ProcessVect.end(); ++it)
	{		
		attach(*it);		
	}
	duplicate->_ProcessVect.clear();
	delete duplicate;
}

///=======================================================================================
void CParticleSystem::activatePresetBehaviour(TPresetBehaviour behaviour)
{

	switch(behaviour)
	{
		case EnvironmentFX:
			setDestroyModelWhenOutOfRange(false);
			setDestroyCondition(none);
			destroyWhenOutOfFrustum(false);
			setAnimType(AnimVisible);
		break;
		case RunningEnvironmentFX:
			setDestroyModelWhenOutOfRange(false);
			setDestroyCondition(none);
			destroyWhenOutOfFrustum(false);
			setAnimType(AnimInCluster);
		break;
		case SpellFX:
			setDestroyModelWhenOutOfRange(true);
			setDestroyCondition(noMoreParticles);
			destroyWhenOutOfFrustum(false);
			setAnimType(AnimAlways);
		break;
		case LoopingSpellFX:
			setDestroyModelWhenOutOfRange(true);
			setDestroyCondition(noMoreParticles);
			destroyWhenOutOfFrustum(false);
			setAnimType(AnimInCluster);
		break;
		case MinorFX:
			setDestroyModelWhenOutOfRange(true);
			setDestroyCondition(noMoreParticles);
			destroyWhenOutOfFrustum(true);
			setAnimType(AnimVisible);
		break;		
		default: break;
	}
	_PresetBehaviour = behaviour;
}


///=======================================================================================
CParticleSystemProcess *CParticleSystem::detach(uint index)
{
	nlassert(index < _ProcessVect.size());
	CParticleSystemProcess *proc = _ProcessVect[index];	
	// release references other process may have to this system
	for(TProcessVect::iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
	{
		(*it)->releaseRefTo(proc);
	}
	// erase from the vector
	_ProcessVect.erase(_ProcessVect.begin() + index);
	proc->setOwner(NULL);
	// not part of this system any more	
	return proc;
}

///=======================================================================================
bool CParticleSystem::isProcess(CParticleSystemProcess *process) const
{
	for(TProcessVect::const_iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
	{
		if (*it == process) return true;
	}
	return false;
}

///=======================================================================================
uint CParticleSystem::getIndexOf(const CParticleSystemProcess *process) const
{
	for(uint k = 0; k < _ProcessVect.size(); ++k)
	{
		if (_ProcessVect[k] == process) return k;
	}
	nlassert(0); // not a process of this system
	return 0; // for warning
}

///=======================================================================================
uint CParticleSystem::getNumID() const
{
	return _LBMap.size();
}

///=======================================================================================
uint32 CParticleSystem::getID(uint index) const
{
	TLBMap::const_iterator it = _LBMap.begin();
	for(uint k = 0; k < index; ++k)
	{
		if (it == _LBMap.end()) return 0;
		++it;
	}
	return it->first;
}

///=======================================================================================
void CParticleSystem::getIDs(std::vector<uint32> &dest) const
{
	dest.resize(_LBMap.size());
	uint k = 0;
	for(TLBMap::const_iterator it = _LBMap.begin(); it != _LBMap.end(); ++it)
	{
		dest[k] = it->first;
		++k;
	}
}



} // NL3D
