 /** \file particle_system.cpp
 * <File description>
 *
 * $Id: particle_system.cpp,v 1.71 2003/12/08 15:00:04 vizerie Exp $
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
#include "3d/ps_emitter.h"
#include "3d/ps_sound.h"
#include "3d/particle_system_shape.h"
#include "3d/ps_located.h"
#include "nel/misc/aabbox.h"
#include "nel/misc/file.h"
#include "nel/misc/stream.h"


#ifdef NL_DEBUG
	#define CHECK_INTEGRITY checkIntegrity();
#else
	#define CHECK_INTEGRITY
#endif

namespace NL3D 
{

uint32										CParticleSystem::NbParticlesDrawn = 0;
UPSSoundServer *							CParticleSystem::_SoundServer = NULL;
CParticleSystem::TGlobalValuesMap			CParticleSystem::_GlobalValuesMap;
CParticleSystem::TGlobalVectorValuesMap		CParticleSystem::_GlobalVectorValuesMap;


#ifdef NL_DEBUG
uint	CParticleSystem::_NumInstances = 0;
#endif


static const float PS_MIN_TIMEOUT = 1.f; // the test that check if there are no particles left 

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
									 _UserCoordSystemInfo(NULL),
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
									 _MaxDistLODBias(0.05f),
									 _InvMaxViewDist(1.f / PSDefaultMaxViewDist),
									 _InvCurrentViewDist(1.f / PSDefaultMaxViewDist),
									 _AutoLODEmitRatio(0.f),
									 _DieCondition(none),
									 _DelayBeforeDieTest(-1.f),									 									
									 _MaxNumFacesWanted(0),
									 _AnimType(AnimInCluster),
									 _UserParamGlobalValue(NULL),
									 _BypassGlobalUserParam(0),
									 _PresetBehaviour(UserBehaviour),									 
									 _AutoLODStartDistPercent(0.1f),
									 _AutoLODDegradationExponent(1),																		 
									 _ColorAttenuationScheme(NULL),
									 _GlobalColor(NLMISC::CRGBA::White),
									 _GlobalColorLighted(NLMISC::CRGBA::White),
									 _LightingColor(NLMISC::CRGBA::White),
									 _UserColor(NLMISC::CRGBA::White),
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
									 _EnableLoadBalancing(true),
									 _EmitThreshold(true),
									 _BypassIntegrationStepLimit(false),
									 _ForceGlobalColorLighting(false),
									 _AutoComputeDelayBeforeDeathTest(true),
									 _AutoCount(false),
									 _InverseEllapsedTime(0.f),
									 _HiddenAtCurrentFrame(true),
									 _HiddenAtPreviousFrame(true)

{
	std::fill(_UserParam, _UserParam + MaxPSUserParam, 0);	
	#ifdef NL_DEBUG
		++_NumInstances;
	#endif

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
				if (psl->getBoundObject(l)->getType() == PSSound)
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
		float modelDist = (getSysMat().getPos() - _InvertedViewMat.getPos()).norm();
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
	if (_ColorAttenuationScheme)
		delete _ColorAttenuationScheme;
	if (_UserParamGlobalValue)
		delete _UserParamGlobalValue;
	delete _UserCoordSystemInfo;
	#ifdef NL_DEBUG
		--_NumInstances;
	#endif
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
bool CParticleSystem::hasParticles() const
{
	for (TProcessVect::const_iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
	{					
		if ((*it)->hasParticles()) return true;		
	}
	return false;
}

///=======================================================================================
bool CParticleSystem::hasTemporaryParticles() const
{
	for (TProcessVect::const_iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
	{				
		if ((*it)->isLocated())
		{
			CPSLocated *loc = static_cast<CPSLocated *>(*it);
			if (loc->hasParticles()) return true;		
		}		
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
	CVector sysPos = getSysMat().getPos();
	CVector obsPos = _InvertedViewMat.getPos();
	const CVector d = getSysMat().getPos() - _InvertedViewMat.getPos();		
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
	else
	{
		_GlobalColor = NLMISC::CRGBA::White;
	}
	_GlobalColor.modulateFromColor(_GlobalColor, _UserColor);
	_GlobalColorLighted.modulateFromColor(_GlobalColor, _LightingColor);
}


/*
static void displaySysPos(IDriver *drv, const CVector &pos, CRGBA col)
{
	if (!drv) return;	
	drv->setupModelMatrix(CMatrix::Identity);
	CPSUtil::displayArrow(drv, pos, CVector::K, 1.f, CRGBA::White, col);
}
*/


///=======================================================================================
void CParticleSystem::step(TPass pass, TAnimationTime ellapsedTime)
{	
	CHECK_INTEGRITY
	nlassert(_CoordSystemInfo.Matrix); // matrix not set for position of system
	if (_UserCoordSystemInfo)
	{
		nlassert(_CoordSystemInfo.Matrix);
	}
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
			// update user param from global value if needed, unless this behaviour is bypassed has indicated by a flag in _BypassGlobalUserParam
			if (_UserParamGlobalValue)
			{
				nlctassert(MaxPSUserParam < 8); // there should be less than 8 parameters because of mask stored in a byte			
				uint8 bypassMask = 1;
				for(uint k = 0; k < MaxPSUserParam; ++k)
				{
					if (_UserParamGlobalValue[k] && !(_BypassGlobalUserParam & bypassMask)) // if there is a global value for this param and if the update is not bypassed
					{
						_UserParam[k] = _UserParamGlobalValue[k]->second;						
					}
					bypassMask <<= 1;
				}
			}
			//
			_BBoxTouched = true;
			TAnimationTime et = ellapsedTime;
			uint nbPass = 1;
			if (_AccurateIntegration)
			{
				if (et > _TimeThreshold)
				{
					nbPass = (uint32) ceilf(et / _TimeThreshold);
					if (!_BypassIntegrationStepLimit && nbPass > _MaxNbIntegrations)
					{ 
						nbPass = _MaxNbIntegrations;
						if (_CanSlowDown)
						{
							et = _TimeThreshold;
							nlassert(_TimeThreshold != 0);
							_InverseEllapsedTime = 1.f / (_TimeThreshold * nbPass);
						}
						else
						{
							et = ellapsedTime / nbPass;
							_InverseEllapsedTime = ellapsedTime != 0 ? 1.f / ellapsedTime : 0.f;
						}						
					}
					else
					{
						et = ellapsedTime / nbPass;
						_InverseEllapsedTime = ellapsedTime != 0 ? 1.f / ellapsedTime : 0.f;
					}
				}
				else
				{
					_InverseEllapsedTime = ellapsedTime != 0 ? 1.f / ellapsedTime : 0.f;
				}
			}
			else
			{
				_InverseEllapsedTime = ellapsedTime != 0 ? 1.f / ellapsedTime : 0.f;
			}
			updateLODRatio();

			if (_AutoLOD && !_Sharing)
			{
				float currLODRatio = 1.f - _OneMinusCurrentLODRatio;
				if (currLODRatio <= _AutoLODStartDistPercent)
				{
					_AutoLODEmitRatio = 1.f; // no LOD applied
				}
				else
				{
					float lodValue = (currLODRatio - 1.f) / (_AutoLODStartDistPercent - 1.f);
					NLMISC::clamp(lodValue, 0.f, 1.f);
					float finalValue = lodValue;
					for(uint l = 1; l < _AutoLODDegradationExponent; ++l)
					{
						finalValue *= lodValue;
					}
					_AutoLODEmitRatio = (1.f - _MaxDistLODBias) * finalValue + _MaxDistLODBias;
				}
			}			
			nlinfo("system %s is %s", _Name.c_str(), _HiddenAtCurrentFrame ? "hidden" : "showed");
			// set start position. Used by emitters that emit from Local basis to world			
			if (!_HiddenAtPreviousFrame && !_HiddenAtCurrentFrame)
			{			
				_CoordSystemInfo.CurrentDeltaPos = _CoordSystemInfo.OldPos - _CoordSystemInfo.Matrix->getPos();						
				if (_UserCoordSystemInfo)
				{
					CCoordSystemInfo &csi = _UserCoordSystemInfo->CoordSystemInfo;
					csi.CurrentDeltaPos = csi.OldPos - csi.Matrix->getPos();
				}
			}
			else
			{
				_CoordSystemInfo.CurrentDeltaPos = NLMISC::CVector::Null;
				_CoordSystemInfo.OldPos = _CoordSystemInfo.Matrix->getPos();
				if (_UserCoordSystemInfo)
				{
					CCoordSystemInfo &csi = _UserCoordSystemInfo->CoordSystemInfo;
					csi.CurrentDeltaPos = NLMISC::CVector::Null;
					csi.OldPos = csi.Matrix->getPos();
				}
			}
			//displaySysPos(_Driver, _CurrentDeltaPos + _OldSysMat.getPos(), CRGBA::Red);
			// process passes
			float realEt = _KeepEllapsedTimeForLifeUpdate ? (ellapsedTime / nbPass)
														  : et;						

			nlinfo("Delta pos = (%f, %f, %f)", _CoordSystemInfo.Matrix->getPos().x - _CoordSystemInfo.OldPos.x,
				                               _CoordSystemInfo.Matrix->getPos().y - _CoordSystemInfo.OldPos.y,
											   _CoordSystemInfo.Matrix->getPos().y - _CoordSystemInfo.OldPos.y);
			do
			{					
				// position of the system at the end of the integration
				_CoordSystemInfo.CurrentDeltaPos += (_CoordSystemInfo.Matrix->getPos() - _CoordSystemInfo.OldPos) * (et * _InverseEllapsedTime);
				if (_UserCoordSystemInfo)
				{
					CCoordSystemInfo &csi = _UserCoordSystemInfo->CoordSystemInfo;
					csi.CurrentDeltaPos += (csi.Matrix->getPos() - csi.OldPos) * (et * _InverseEllapsedTime);
				}

				//displaySysPos(_Driver, _CurrentDeltaPos + _OldSysMat.getPos(), CRGBA::Blue);
				// the order of the following is important...
				stepLocated(PSCollision, et,  realEt);
				for (TProcessVect::iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
				{
					(*it)->updateLife(realEt);
					(*it)->step(PSMotion, et, realEt);					
				}
				_SystemDate += realEt;
				stepLocated(PSEmit, et,  realEt);
								

				if (_BypassIntegrationStepLimit)
				{
					// check that system is finished to avoid unuseful processing
					if (isDestroyConditionVerified())
					{
						return;
					}
				}
			}
			while (--nbPass);
			
			// perform parametric motion if present
			for (TProcessVect::iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
			{
				if ((*it)->isParametricMotionEnabled()) (*it)->performParametricMotion(_SystemDate, ellapsedTime, realEt);
			}	
			
			// memorize position of matrix for next frame (becomes old position)
			_CoordSystemInfo.OldPos = _CoordSystemInfo.Matrix->getPos();
			if (_UserCoordSystemInfo)
			{
				CCoordSystemInfo &csi = _UserCoordSystemInfo->CoordSystemInfo;
				csi.OldPos =  csi.Matrix->getPos();
			}

			_HiddenAtPreviousFrame = _HiddenAtCurrentFrame;
		}
	}	
	CHECK_INTEGRITY
}


///=======================================================================================
void CParticleSystem::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{		
	sint version =  f.serialVersion(19);

	// version 19: sysmat no more serialized (useless)
	// version 18: _AutoComputeDelayBeforeDeathTest
	// version 17: _ForceGlobalColorLighting flag
	// version 16: _BypassIntegrationStepLimit flag
	// version 14: emit threshold
	// version 13: max dist lod bias for auto-LOD
	// version 12: global userParams
	// version 11: enable load balancing flag 
	// version 9: Sharing flag added
	//            Auto-lod parameters
	//            New integration flag
	//			  Global color attenuation
	// version 8: Replaced the attribute '_PerformMotionWhenOutOfFrustum' by a _AnimType field which allow more precise control

	//f.serial(_ViewMat);
	if (version < 19)
	{	
		NLMISC::CMatrix dummy;
		f.serial(dummy);
	}
	f.serial(_Date);
	if (f.isReading())
	{
		delete _ColorAttenuationScheme;
		// delete previous multimap
		_LBMap.clear();
		// delete previously attached process
		TProcessVect::iterator it;
		for (it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
		{
			delete (*it);
		}

		_ProcessVect.clear();

		f.serialContPolyPtr(_ProcessVect);		
	
		_FontGenerator = NULL;
		_FontManager = NULL;
		if (_UserParamGlobalValue)
			delete _UserParamGlobalValue;
		_UserParamGlobalValue = NULL;
		_BypassGlobalUserParam = 0;
		// see if some process need to access the user matrix
		delete _UserCoordSystemInfo;
		_UserCoordSystemInfo = NULL;
		for (TProcessVect::iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
		{			
			addRefForUserSysCoordInfo((*it)->getUserMatrixUsageCount());			
		}
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
		bool accurateIntegration = _AccurateIntegration; // read from bitfield
		f.serial(accurateIntegration);
		_AccurateIntegration = accurateIntegration;
		if (_AccurateIntegration) 
		{
			bool canSlowDown = _CanSlowDown;
			f.serial(canSlowDown);
			_CanSlowDown = canSlowDown;
			f.serial(_TimeThreshold, _MaxNbIntegrations);
		}
		f.serial(_InvMaxViewDist, _LODRatio);	
		_MaxViewDist = 1.f / _InvMaxViewDist;
		_InvCurrentViewDist = _InvMaxViewDist;
	}

	if (version > 3) // tell wether the system must compute his bbox, hold a precomputed bbox
	{
		bool computeBBox = _ComputeBBox;
		f.serial(computeBBox);
		_ComputeBBox = computeBBox;
		if (!computeBBox)
		{
			f.serial(_PreComputedBBox);
		}		
	}

	if (version > 4) // lifetime informations
	{
		bool destroyModelWhenOutOfRange = _DestroyModelWhenOutOfRange; // read from bitfield
		f.serial(destroyModelWhenOutOfRange);
		_DestroyModelWhenOutOfRange = destroyModelWhenOutOfRange;
		f.serialEnum(_DieCondition);
		if (_DieCondition != none)
		{
			f.serial(_DelayBeforeDieTest);
		}
	}	

	if (version > 5)
	{
		bool destroyWhenOutOfFrustum = _DestroyWhenOutOfFrustum; // read from bitfield
		f.serial(destroyWhenOutOfFrustum);
		_DestroyWhenOutOfFrustum = destroyWhenOutOfFrustum;
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
		bool sharing = _Sharing; // read from bitfield
		f.serial(sharing);
		_Sharing = sharing;
		bool autoLOD = _AutoLOD; // read from bitfield
		f.serial(autoLOD);
		_AutoLOD = autoLOD;

		if (_AutoLOD)
		{
			f.serial(_AutoLODStartDistPercent, _AutoLODDegradationExponent);
			bool autoLODSkipParticles = _AutoLODSkipParticles; // read from bitfield
			f.serial(autoLODSkipParticles);
			_AutoLODSkipParticles = autoLODSkipParticles;
		}
		bool keepEllapsedTimeForLifeUpdate = _KeepEllapsedTimeForLifeUpdate;
		f.serial(keepEllapsedTimeForLifeUpdate);
		_KeepEllapsedTimeForLifeUpdate = keepEllapsedTimeForLifeUpdate;
		f.serialPolyPtr(_ColorAttenuationScheme);
	}

	if (version >= 11)
	{
		bool enableLoadBalancing = _EnableLoadBalancing; // read from bitfield
		f.serial(enableLoadBalancing);
		_EnableLoadBalancing = enableLoadBalancing;
	}

	if (version >= 12)
	{
		// serial infos about global user params
		nlctassert(MaxPSUserParam < 8); // In this version mask of used global user params are stored in a byte..		
		if (f.isReading())
		{			
			uint8 mask;
			f.serial(mask);
			if (mask)
			{		
				std::string globalValueName;
				uint8 testMask = 1;
				for(uint k = 0; k < MaxPSUserParam; ++k)
				{					
					if (mask & testMask)
					{						
						f.serial(globalValueName);
						bindGlobalValueToUserParam(globalValueName.c_str(), k);
					}
					testMask <<= 1;
				}
			}
		}
		else
		{
			uint8 mask = 0;
			if (_UserParamGlobalValue)
			{
				for(uint k = 0; k < MaxPSUserParam; ++k)
				{
					if (_UserParamGlobalValue[k]) mask |= (1 << k);
				}
			}
			f.serial(mask);
			if (_UserParamGlobalValue)
			{
				for(uint k = 0; k < MaxPSUserParam; ++k)
				{				
					if (_UserParamGlobalValue[k]) 
					{
						std::string valueName = _UserParamGlobalValue[k]->first;	
						f.serial(valueName);
					}
				}
			}
		}		
	}	
	if (version >= 13)
	{
		if (_AutoLOD && !_Sharing)
		{
			f.serial(_MaxDistLODBias);
		}
	}
	if (version >= 14)
	{
		bool emitThreshold = _EmitThreshold; // read from bitfiled
		f.serial(emitThreshold);
		_EmitThreshold = emitThreshold;
	}

	if (version >= 15)
	{
		bool bypassIntegrationStepLimit = _BypassIntegrationStepLimit; // read from bitfield
		f.serial(bypassIntegrationStepLimit);
		_BypassIntegrationStepLimit = bypassIntegrationStepLimit;
	}

	if (version >= 17)
	{
		bool forceGlobalColorLighting = _ForceGlobalColorLighting; // read from bitfield
		f.serial(forceGlobalColorLighting);
		_ForceGlobalColorLighting = forceGlobalColorLighting;
	}

	if (version >= 18)
	{
		bool autoComputeDelayBeforeDeathTest = _AutoComputeDelayBeforeDeathTest; // read from bitfield
		f.serial(autoComputeDelayBeforeDeathTest);
		_AutoComputeDelayBeforeDeathTest = autoComputeDelayBeforeDeathTest;
	}
	else
	{
		nlassert(f.isReading());				
		// for all previously created system, force to eval the system duration in an automatyic way
		setDelayBeforeDeathConditionTest(-1.f);
		_AutoComputeDelayBeforeDeathTest = true;
	}

	if (f.isReading())
	{
		notifyMaxNumFacesChanged();
		activatePresetBehaviour(_PresetBehaviour); // apply behaviour changes
	}
	CHECK_INTEGRITY
}

///=======================================================================================
bool CParticleSystem::attach(CParticleSystemProcess *ptr)
{
	nlassert(ptr);
	nlassert(std::find(_ProcessVect.begin(), _ProcessVect.end(), ptr) == _ProcessVect.end() ); // can't attach twice
	//nlassert(ptr->getOwner() == NULL);
	_ProcessVect.push_back(ptr);
	ptr->setOwner(this);
	notifyMaxNumFacesChanged();
	if (getBypassMaxNumIntegrationSteps())
	{
		if (!canFinish())
		{
			remove(ptr);
			nlwarning("<void CParticleSystem::attach> Can't attach object : this causes the system to last forever, and it has been flagged with 'BypassMaxNumIntegrationSteps'. Object is not attached");
			return false;
		}
	}
	systemDurationChanged();	
	return true;
}

///=======================================================================================
void CParticleSystem::remove(CParticleSystemProcess *ptr)
{
	TProcessVect::iterator it = std::find(_ProcessVect.begin(), _ProcessVect.end(), ptr);
	nlassert(it != _ProcessVect.end() );	
	ptr->setOwner(NULL);
	_ProcessVect.erase(it);
	delete ptr;
	systemDurationChanged();
}

///=======================================================================================
void CParticleSystem::forceComputeBBox(NLMISC::CAABBox &aabbox)
{
	bool foundOne = false;
	NLMISC::CAABBox tmpBox;	
	for (TProcessVect::const_iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
	{
		if ((*it)->computeBBox(tmpBox))
		{			
			// rotate the aabbox so that it is in the correct basis			
			const CMatrix &convMat = CPSLocated::getConversionMatrix(*this, PSFXWorldMatrix, (*it)->getMatrixMode());
			tmpBox = NLMISC::CAABBox::transformAABBox(convMat, tmpBox);
			
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
}

///=======================================================================================
void CParticleSystem::computeBBox(NLMISC::CAABBox &aabbox)
{
	if (!_ComputeBBox || !_BBoxTouched)
	{
		aabbox = _PreComputedBBox;
		return;
	}
	forceComputeBBox(aabbox);		
	_BBoxTouched = false;
	_PreComputedBBox = aabbox;
}

///=======================================================================================
void CParticleSystem::setSysMat(const CMatrix *m)
{	
	_CoordSystemInfo.Matrix = m;
	if (_SystemDate == 0.f)
	{			
		_CoordSystemInfo.OldPos = m ? m->getPos() : CVector::Null;
	}
	if (!m) return;
	_CoordSystemInfo.InvMatrix = _CoordSystemInfo.Matrix->inverted();	
}

///=======================================================================================
void CParticleSystem::setUserMatrix(const NLMISC::CMatrix *m)
{	
	if (!_UserCoordSystemInfo) return; // no process in the system references the user matrix
	CCoordSystemInfo &csi = _UserCoordSystemInfo->CoordSystemInfo;
	csi.Matrix = m;
	if (_SystemDate == 0.f)
	{	
		csi.OldPos = m ? m->getPos() : getSysMat().getPos(); // _CoordSystemInfo.Matrix is relevant if at least one call to setSysMat has been performed before
	}	
	if (!m) return;
	csi.InvMatrix = csi.Matrix->inverted();
	// build conversion matrix between father user matrix & fx matrix
	// TODO : lazy evaluation for this ?
	_UserCoordSystemInfo->UserBasisToFXBasis = _CoordSystemInfo.InvMatrix * *(csi.Matrix);
	_UserCoordSystemInfo->FXBasisToUserBasis =  csi.InvMatrix * getSysMat();
}

///=======================================================================================
bool CParticleSystem::hasOpaqueObjects(void) const
{
	/// for each process
	for (TProcessVect::const_iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
	{
		if ((*it)->isLocated())
		{
			CPSLocated *loc = static_cast<CPSLocated *>(*it);		
			for (uint k = 0; k < loc->getNbBoundObjects(); ++k)
			{
				CPSLocatedBindable *lb = loc->getBoundObject(k);
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
		if ((*it)->isLocated())
		{
			CPSLocated *loc = static_cast<CPSLocated *>(*it);
			for (uint k = 0; k < loc->getNbBoundObjects(); ++k)
			{
				CPSLocatedBindable *lb = loc->getBoundObject(k);
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
bool CParticleSystem::hasLightableObjects() const
{
	if (_ForceGlobalColorLighting) return true;
	/// for each process
	for (TProcessVect::const_iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
	{
		if ((*it)->isLocated())
		{
			CPSLocated *loc = static_cast<CPSLocated *>(*it);
			for (uint k = 0; k < loc->getNbBoundObjects(); ++k)
			{
				CPSLocatedBindable *lb = loc->getBoundObject(k);
				if (lb->getType() == PSParticle)
				{
					if (((CPSParticle *) lb)->hasLightableFaces()) return true;
					if (((CPSParticle *) lb)->usesGlobalColorLighting()) return true;
				}
			}
		}
	}
	return false;
}

///=======================================================================================
void CParticleSystem::getLODVect(NLMISC::CVector &v, float &offset,  TPSMatrixMode matrixMode)
{
	switch(matrixMode)
	{
		case PSFXWorldMatrix:
		{			
			const CVector tv = getInvertedSysMat().mulVector(_InvertedViewMat.getJ());
			const CVector org = getInvertedSysMat() * _InvertedViewMat.getPos();
			v = _InvCurrentViewDist * tv;
			offset = - org * v;
		}
		break;
		case PSIdentityMatrix:
		{
			v = _InvCurrentViewDist * _InvertedViewMat.getJ();
			offset = - _InvertedViewMat.getPos() * v;
		}
		break;
		case PSUserMatrix:
		{
			const CVector tv = getInvertedUserMatrix().mulVector(_InvertedViewMat.getJ());
			const CVector org = getInvertedUserMatrix() * _InvertedViewMat.getPos();
			v = _InvCurrentViewDist * tv;
			offset = - org * v;
		}
		break;
		default:
			nlassert(0);
		break;
	}	
}

///=======================================================================================
TPSLod CParticleSystem::getLOD(void) const
{
	const float dist = fabsf(_InvCurrentViewDist * (getSysMat().getPos() - _InvertedViewMat.getPos()) * _InvertedViewMat.getJ());
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
bool CParticleSystem::merge(CParticleSystemShape *pss)
{
	nlassert(pss);	
	nlassert(_Scene);
	CParticleSystem *duplicate = pss->instanciatePS(*this->_Scene); // duplicate the p.s. to merge
	// now we transfer the located of the duplicated ps to this object...	
	for (TProcessVect::iterator it = duplicate->_ProcessVect.begin(); it != duplicate->_ProcessVect.end(); ++it)
	{		
		if (!attach(*it))
		{
			for (TProcessVect::iterator clearIt = duplicate->_ProcessVect.begin(); clearIt != it; ++it)
			{
				detach(getIndexOf(*it));
			}
			nlwarning("<CParticleSystem::merge> Can't do the merge : this causes the system to last forever, and it has been flagged with 'BypassMaxNumIntegrationSteps'. Merge is not done.");
			return false;
		}
	}
	//
	if (getBypassMaxNumIntegrationSteps())
	{
		if (!canFinish())
		{
			for (TProcessVect::iterator it = duplicate->_ProcessVect.begin(); it != duplicate->_ProcessVect.end(); ++it)
			{		
				detach(getIndexOf(*it));
			}
			nlwarning("<CParticleSystem::merge> Can't do the merge : this causes the system to last forever, and it has been flagged with 'BypassMaxNumIntegrationSteps'. Merge is not done.");
			return false;
		}
	}
	//
	duplicate->_ProcessVect.clear();	
	delete duplicate;
	systemDurationChanged();
	CHECK_INTEGRITY
	return true;
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
			setBypassMaxNumIntegrationSteps(false);
			_KeepEllapsedTimeForLifeUpdate = false;
		break;
		case RunningEnvironmentFX:
			setDestroyModelWhenOutOfRange(false);
			setDestroyCondition(none);
			destroyWhenOutOfFrustum(false);
			setAnimType(AnimInCluster);
			setBypassMaxNumIntegrationSteps(false);
			_KeepEllapsedTimeForLifeUpdate = false;
		break;
		case SpellFX:
			setDestroyModelWhenOutOfRange(true);
			setDestroyCondition(noMoreParticles);
			destroyWhenOutOfFrustum(false);
			setAnimType(AnimAlways);
			setBypassMaxNumIntegrationSteps(true);
			_KeepEllapsedTimeForLifeUpdate = false;
		break;
		case LoopingSpellFX:
			setDestroyModelWhenOutOfRange(true);
			setDestroyCondition(noMoreParticles);
			destroyWhenOutOfFrustum(false);
			setAnimType(AnimInCluster); // TODO : AnimAlways could be better
			setBypassMaxNumIntegrationSteps(false);
			_KeepEllapsedTimeForLifeUpdate = false;
		break;
		case MinorFX:
			setDestroyModelWhenOutOfRange(true);
			setDestroyCondition(noMoreParticles);
			destroyWhenOutOfFrustum(true);
			setAnimType(AnimVisible);
			setBypassMaxNumIntegrationSteps(false);
			_KeepEllapsedTimeForLifeUpdate = false;
		break;
		case MovingLoopingFX:
			setDestroyModelWhenOutOfRange(false);
			setDestroyCondition(none);
			destroyWhenOutOfFrustum(false);
			setAnimType(AnimVisible);
			setBypassMaxNumIntegrationSteps(false);
			_KeepEllapsedTimeForLifeUpdate = true;
		break;
		case SpawnedEnvironmentFX:
			setDestroyModelWhenOutOfRange(true);
			setDestroyCondition(noMoreParticles);
			destroyWhenOutOfFrustum(false);
			setAnimType(AnimAlways);
			setBypassMaxNumIntegrationSteps(false);
			_KeepEllapsedTimeForLifeUpdate = false;
		break;
		case GroundFX:
			setDestroyModelWhenOutOfRange(false);
			setDestroyCondition(none);
			destroyWhenOutOfFrustum(false);
			setAnimType(AnimAlways);
			setBypassMaxNumIntegrationSteps(false);
			_KeepEllapsedTimeForLifeUpdate = true;
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
	//
	systemDurationChanged();
	// not part of this system any more	
	return proc;
}

///=======================================================================================
bool CParticleSystem::isProcess(const CParticleSystemProcess *process) const
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

///=======================================================================================
void CParticleSystem::interpolateFXPosDelta(NLMISC::CVector &dest, TAnimationTime deltaT)
{
	nlassert(_CoordSystemInfo.Matrix);
	dest = _CoordSystemInfo.CurrentDeltaPos - (deltaT * _InverseEllapsedTime) * (_CoordSystemInfo.Matrix->getPos() - _CoordSystemInfo.OldPos);
}

///=======================================================================================
void CParticleSystem::interpolateUserPosDelta(NLMISC::CVector &dest, TAnimationTime deltaT)
{
	if (!_UserCoordSystemInfo) 
	{
		interpolateFXPosDelta(dest, deltaT);
	}
	else
	{
		CCoordSystemInfo &csi = _UserCoordSystemInfo->CoordSystemInfo;
		dest = csi.CurrentDeltaPos - (deltaT * _InverseEllapsedTime) * (csi.Matrix->getPos() - csi.OldPos);
	}
}

///=======================================================================================
void CParticleSystem::bindGlobalValueToUserParam(const std::string &globalValueName, uint userParamIndex)
{
	nlassert(userParamIndex < MaxPSUserParam);
	if (globalValueName.empty()) // disable a user param global value
	{
		if (!_UserParamGlobalValue) return;
		_UserParamGlobalValue[userParamIndex] = NULL;
		for(uint k = 0; k < MaxPSUserParam; ++k)
		{
			if (_UserParamGlobalValue[k] != NULL) return;
		}
		// no more entry used
		delete _UserParamGlobalValue;
		_UserParamGlobalValue = NULL;
	}
	else // enable a user param global value
	{
		if (!_UserParamGlobalValue) 
		{
			// no table has been allocated yet, so create one
			_UserParamGlobalValue = new const TGlobalValuesMap::value_type *[MaxPSUserParam];
			std::fill(_UserParamGlobalValue, _UserParamGlobalValue + MaxPSUserParam, (TGlobalValuesMap::value_type *) NULL);
		}
		// has the global value be created yet ?
		TGlobalValuesMap::const_iterator it = _GlobalValuesMap.find(globalValueName);
		if (it != _GlobalValuesMap.end())
		{
			// yes, make a reference on it
			_UserParamGlobalValue[userParamIndex] = &(*it);
		}
		else
		{
			// create a new entry
			std::pair<TGlobalValuesMap::iterator, bool> itPair = _GlobalValuesMap.insert(TGlobalValuesMap::value_type(globalValueName, 0.f));
			_UserParamGlobalValue[userParamIndex] = &(*(itPair.first));
		}
	}
}

///=======================================================================================
void CParticleSystem::setGlobalValue(const std::string &name, float value)
{
	nlassert(!name.empty());
	NLMISC::clamp(value, 0.f, 1.f);
	_GlobalValuesMap[name] = value;
}

///=======================================================================================
float CParticleSystem::getGlobalValue(const std::string &name)
{
	TGlobalValuesMap::const_iterator it = _GlobalValuesMap.find(name);
	if (it != _GlobalValuesMap.end()) return it->second;
	return 0.f; // not a known value
}

///=======================================================================================
std::string CParticleSystem::getGlobalValueName(uint userParamIndex) const
{
	nlassert(userParamIndex < MaxPSUserParam);
	if (!_UserParamGlobalValue) return "";
	if (!_UserParamGlobalValue[userParamIndex]) return "";
	return _UserParamGlobalValue[userParamIndex]->first;
}

///=======================================================================================
void CParticleSystem::setGlobalVectorValue(const std::string &name, const NLMISC::CVector &value)
{
	nlassert(!name.empty());
	_GlobalVectorValuesMap[name] = value;	
}


///=======================================================================================
NLMISC::CVector CParticleSystem::getGlobalVectorValue(const std::string &name)
{
	nlassert(!name.empty());
	TGlobalVectorValuesMap::const_iterator it = _GlobalVectorValuesMap.find(name);
	if (it != _GlobalVectorValuesMap.end()) return it->second;
	return NLMISC::CVector::Null; // not a known value  	
}

///=======================================================================================
CParticleSystem::CGlobalVectorValueHandle CParticleSystem::getGlobalVectorValueHandle(const std::string &name)
{
	nlassert(!name.empty());	
	TGlobalVectorValuesMap::iterator it = _GlobalVectorValuesMap.find(name);
	if (it == _GlobalVectorValuesMap.end())
	{
		it = _GlobalVectorValuesMap.insert(TGlobalVectorValuesMap::value_type(name, NLMISC::CVector::Null)).first;		
	}
	CGlobalVectorValueHandle handle;
	handle._Value = &it->second;	
	handle._Name = &it->first;
	return handle;
}

///=======================================================================================
void CParticleSystem::setMaxDistLODBias(float lodBias)
{
	NLMISC::clamp(lodBias, 0.f, 1.f);
	_MaxDistLODBias = lodBias;
}

///=======================================================================================
bool CParticleSystem::canFinish(CPSLocatedBindable **lastingForeverObj /*= NULL*/) const
{
	if (hasLoop(lastingForeverObj)) return false;	
	for(uint k = 0; k < _ProcessVect.size(); ++k)
	{
		if (_ProcessVect[k]->isLocated())
		{
			CPSLocated *loc = static_cast<CPSLocated *>(_ProcessVect[k]);
			if (loc->getLastForever())
			{			
				for(uint l = 0; l < loc->getNbBoundObjects(); ++l)
				{
					CPSEmitter *em = dynamic_cast<CPSEmitter *>(loc->getBoundObject(l));
					if (em && em->testEmitForever())
					{
						if (lastingForeverObj) *lastingForeverObj = em;
						return false;
					}
					CPSParticle *p = dynamic_cast<CPSParticle *>(loc->getBoundObject(l));
					if (p)
					{
						if (lastingForeverObj) *lastingForeverObj = p;
						return false; // particles shouldn't live forever, too
					}
				}
			}
		}
	}
	return true;
}

///=======================================================================================
bool CParticleSystem::hasLoop(CPSLocatedBindable **loopingObj /*= NULL*/) const
{	
	// we want to check for loop like A emit B emit A
	// NB : there's room for a smarter algo here, but should not be useful for now 
	for(uint k = 0; k < _ProcessVect.size(); ++k)
	{
		if (_ProcessVect[k]->isLocated())
		{
			CPSLocated *loc = static_cast<CPSLocated *>(_ProcessVect[k]);
			for(uint l = 0; l < loc->getNbBoundObjects(); ++l)
			{
				CPSEmitter *em = dynamic_cast<CPSEmitter *>(loc->getBoundObject(l));
				if (em)
				{
					if (em->checkLoop()) 
					{
						if (loopingObj) *loopingObj = em;
						return true;
					}
				}
			}
		}
	}
	return false;
}

///=======================================================================================
void CParticleSystem::systemDurationChanged()
{
	if (getAutoComputeDelayBeforeDeathConditionTest())
	{
		setDelayBeforeDeathConditionTest(-1.f);
	}
}

///=======================================================================================
void CParticleSystem::setAutoComputeDelayBeforeDeathConditionTest(bool computeAuto)
{
	if (computeAuto == _AutoComputeDelayBeforeDeathTest) return;
	_AutoComputeDelayBeforeDeathTest = computeAuto;
	if (computeAuto) setDelayBeforeDeathConditionTest(-1.f);
}

///=======================================================================================
TAnimationTime CParticleSystem::getDelayBeforeDeathConditionTest() const
{
	if (_DelayBeforeDieTest < 0.f)
	{
		_DelayBeforeDieTest = evalDuration();
	}
	return std::max(PS_MIN_TIMEOUT, _DelayBeforeDieTest);
}

///=======================================================================================
// struct to eval duration of an emitter chain
struct CToVisitEmitter
{
	float		Duration; // cumuled duration of thi emitter parent emitters
	const CPSLocated *Located;
};

///=======================================================================================
float CParticleSystem::evalDuration() const
{
	std::vector<const CPSLocated *> visitedEmitter;
	std::vector<CToVisitEmitter> toVisitEmitter;
	float maxDuration = 0.f;
	for(uint k = 0; k < _ProcessVect.size(); ++k)
	{
		if (_ProcessVect[k]->isLocated())
		{
			bool emitterFound = false;
			const CPSLocated *loc = static_cast<const CPSLocated *>(_ProcessVect[k]);
			for(uint l = 0; l < loc->getNbBoundObjects(); ++l)
			{
				const CPSLocatedBindable *bind = loc->getBoundObject(l);
				if (loc->getSize() > 0)
				{
					switch(bind->getType())
					{
						case  PSParticle:
						{						
							if (loc->getLastForever())
							{
								return -1;								
							}
							else
							{							
								maxDuration = std::max(maxDuration, loc->evalMaxDuration());
							}
						}
						break;
						case PSEmitter:
						{
							if (!emitterFound)
							{				
								CToVisitEmitter tve;
								tve.Located = loc;
								tve.Duration = 0.f;
								toVisitEmitter.push_back(tve);
								emitterFound = true;
							}
						}
						break;
					}
				}
			}
			visitedEmitter.clear();
			while (!toVisitEmitter.empty())
			{
				const CPSLocated *loc = toVisitEmitter.back().Located;
				float duration = toVisitEmitter.back().Duration;
				toVisitEmitter.pop_back();				
				visitedEmitter.push_back(loc);
				bool emitterFound = false;
				for(uint m = 0; m < loc->getNbBoundObjects(); ++m)
				{
					const CPSLocatedBindable *bind = loc->getBoundObject(m);
					if (bind->getType() == PSEmitter)
					{					
						const CPSEmitter *em = NLMISC::safe_cast<const CPSEmitter *>(loc->getBoundObject(m));
						const CPSLocated *emittedType = em->getEmittedType();
						// continue if there's no loop
						if (std::find(visitedEmitter.begin(), visitedEmitter.end(), emittedType) == visitedEmitter.end())
						{													
							if (emittedType != NULL)
							{	
								emitterFound = true;
								CToVisitEmitter tve;
								tve.Located = emittedType;								
								// if emitter has limited lifetime, use it
								if (!loc->getLastForever())
								{
									tve.Duration = duration + loc->evalMaxDuration();
								}
								else
								{						
									// try to eval duration depending on type
									switch(em->getEmissionType())
									{
										case CPSEmitter::regular:
										{
											if (em->getMaxEmissionCount() != 0)
											{
												float period = em->getPeriodScheme() ? em->getPeriodScheme()->getMaxValue() : em->getPeriod();
												tve.Duration = duration + em->getEmitDelay() + 	period * em->getMaxEmissionCount();
											}
											else
											{
												tve.Duration = duration + em->getEmitDelay();
											}	
										}
										break;
										case CPSEmitter::onDeath:
										case CPSEmitter::once:
										case CPSEmitter::onBounce:
										case CPSEmitter::externEmit:
											tve.Duration = duration; // can't eval duration ..
										break;
									}
								}
								toVisitEmitter.push_back(tve);
							}
						}						
					}
				}				
				if (!emitterFound)
				{
					if (!loc->getLastForever())
					{
						duration += loc->evalMaxDuration();
					}
					maxDuration = std::max(maxDuration, duration);
				}
			}
		}
	}
	return maxDuration;
}

///=======================================================================================
bool CParticleSystem::isDestroyConditionVerified() const
{
	if (getDestroyCondition() != CParticleSystem::none)
	{
		if (getSystemDate() > getDelayBeforeDeathConditionTest())
		{
			switch (getDestroyCondition())
			{
				case CParticleSystem::noMoreParticles: return !hasParticles();				
				case CParticleSystem::noMoreParticlesAndEmitters: return !hasParticles() && !hasEmitters();									
				default: nlassert(0); return false;
			}
		}
	}
	return false;
}

///=======================================================================================
void CParticleSystem::setSystemDate(float date)
{
	if (date == _SystemDate) return;
	_SystemDate = date;
	for(uint k = 0; k < _ProcessVect.size(); ++k)
	{
		_ProcessVect[k]->systemDateChanged();
	}	
}

///=======================================================================================
void CParticleSystem::registerSoundServer(UPSSoundServer *soundServer)
{
	if (!_SoundServer)
	{
		CParticleSystemManager::stopSoundForAllManagers();
	}
	else
	{
		CParticleSystemManager::reactivateSoundForAllManagers();
	}
	_SoundServer = soundServer;
}

///=======================================================================================
void CParticleSystem::activateEmitters(bool active)
{
	for(uint k = 0; k < getNbProcess(); ++k)
	{	
		if (getProcess(k)->isLocated())
		{
			CPSLocated *loc = static_cast<CPSLocated *>(getProcess(k));
			if (loc)
			{
				for(uint l = 0; l < loc->getNbBoundObjects(); ++l)
				{
					if (loc->getBoundObject(l)->getType() == PSEmitter)	
						loc->getBoundObject(l)->setActive(active);
				}
			}
		}
	}
}

///=======================================================================================
bool CParticleSystem::hasActiveEmitters() const
{
	for(uint k = 0; k < getNbProcess(); ++k)
	{	
		if (getProcess(k)->isLocated())
		{
			const CPSLocated *loc = static_cast<const CPSLocated *>(getProcess(k));
			if (loc)
			{
				for(uint l = 0; l < loc->getNbBoundObjects(); ++l)
				{
					if (loc->getBoundObject(l)->getType() == PSEmitter)	
					{
						if (loc->getBoundObject(l)->isActive()) return true;
					}
				}
			}
		}
	}
	return false;
}

///=======================================================================================
bool CParticleSystem::hasEmittersTemplates() const
{
	for(uint k = 0; k < getNbProcess(); ++k)
	{	
		if (getProcess(k)->isLocated())
		{
			const CPSLocated *loc = static_cast<const CPSLocated *>(getProcess(k));
			if (loc)
			{
				for(uint l = 0; l < loc->getNbBoundObjects(); ++l)
				{
					if (loc->getBoundObject(l)->getType() == PSEmitter)	
					{
						return true;
					}
				}
			}
		}
	}
	return false;
}

///=======================================================================================
void CParticleSystem::matchArraySize()
{
	for(uint k = 0; k < getNbProcess(); ++k)
	{
		if (getProcess(k)->isLocated())
		{
			CPSLocated *loc = static_cast<CPSLocated *>(getProcess(k));
			loc->resize(loc->getSize()); // match the max size with the number of instances
		}
	}
}

///=======================================================================================
uint CParticleSystem::getMaxNumParticles() const
{
	uint numParts = 0;
	for(uint k = 0; k < getNbProcess(); ++k)
	{	
		if (getProcess(k)->isLocated())
		{
			const CPSLocated *loc = static_cast<const CPSLocated *>(getProcess(k));
			if (loc)
			{
				for(uint l = 0; l < loc->getNbBoundObjects(); ++l)
				{
					if (loc->getBoundObject(l)->getType() == PSParticle)	
					{
						numParts += loc->getMaxSize();
					}
				}
			}
		}
	}
	return numParts;
}

///=======================================================================================
uint CParticleSystem::getCurrNumParticles() const
{
	uint numParts = 0;
	for(uint k = 0; k < getNbProcess(); ++k)
	{	
		if (getProcess(k)->isLocated())
		{
			const CPSLocated *loc = static_cast<const CPSLocated *>(getProcess(k));
			if (loc)
			{
				for(uint l = 0; l < loc->getNbBoundObjects(); ++l)
				{
					if (loc->getBoundObject(l)->getType() == PSParticle)	
					{
						numParts += loc->getSize();
					}
				}
			}
		}
	}
	return numParts;
}

///=======================================================================================
void CParticleSystem::getTargeters(const CPSLocated *target, std::vector<CPSTargetLocatedBindable *> &targeters)
{
	nlassert(target);
	nlassert(isProcess(target));
	targeters.clear();
	for(uint k = 0; k < getNbProcess(); ++k)
	{	
		if (getProcess(k)->isLocated())
		{
			CPSLocated *loc = static_cast<CPSLocated *>(getProcess(k));
			if (loc)
			{
				for(uint l = 0; l < loc->getNbBoundObjects(); ++l)
				{
					CPSTargetLocatedBindable *targeter = dynamic_cast<CPSTargetLocatedBindable *>(loc->getBoundObject(l));
					if (targeter)
					{
						for(uint m = 0; m < targeter->getNbTargets(); ++m)
						{
							if (targeter->getTarget(m) == target)
							{
								targeters.push_back(targeter);
								break;
							}
						}
					}					
				}
			}
		}
	}
}

///=======================================================================================
void CParticleSystem::matrixModeChanged(CParticleSystemProcess *proc, TPSMatrixMode oldMode, TPSMatrixMode newMode)
{
	nlassert(proc);
	// check that the located belong to that system
	nlassert(isProcess(proc));
	if (oldMode != PSUserMatrix && newMode == PSUserMatrix)
	{
		addRefForUserSysCoordInfo();
	}
	else if (oldMode == PSUserMatrix && newMode != PSUserMatrix)
	{
		releaseRefForUserSysCoordInfo();
	}
}

///=======================================================================================
void CParticleSystem::addRefForUserSysCoordInfo(uint numRefs)
{
	if (!numRefs) return;
	if (!_UserCoordSystemInfo)
	{
		_UserCoordSystemInfo = new CUserCoordSystemInfo;
	}
	nlassert(_UserCoordSystemInfo)
	_UserCoordSystemInfo->NumRef += numRefs;
	
}

///=======================================================================================
void CParticleSystem::releaseRefForUserSysCoordInfo(uint numRefs)
{
	if (!numRefs) return;
	nlassert(_UserCoordSystemInfo);	
	nlassert(numRefs <= _UserCoordSystemInfo->NumRef)
	_UserCoordSystemInfo->NumRef -= numRefs;
	if (_UserCoordSystemInfo->NumRef == 0)
	{
		delete _UserCoordSystemInfo;
		_UserCoordSystemInfo = NULL;
	}
}

///=======================================================================================
void CParticleSystem::checkIntegrity()
{
	// do some checks
	uint userMatrixUsageCount = 0;
	for (TProcessVect::iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
	{			
		userMatrixUsageCount += (*it)->getUserMatrixUsageCount();
	}
	if (userMatrixUsageCount == 0)
	{	
		nlassert(_UserCoordSystemInfo == NULL);
	}
	else
	{
		nlassert(_UserCoordSystemInfo != NULL);
		nlassert(_UserCoordSystemInfo->NumRef == userMatrixUsageCount);
	}
}


} // NL3D





















