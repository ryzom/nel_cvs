/** \file particle_system_shape.cpp
 * <File description>
 *
 * $Id: particle_system_shape.cpp,v 1.34 2002/06/28 16:52:10 berenguier Exp $
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
#include "3d/particle_system_shape.h"
#include "3d/particle_system_model.h"
#include "3d/scene.h"
#include "3d/driver.h"
#include "nel/misc/file.h"
#include "nel/misc/mem_stream.h"
#include "nel/misc/hierarchical_timer.h"


namespace NL3D {

using NLMISC::IStream;
using NLMISC::CIFile;




// private usage : macro to check the memory integrity
#if defined(NL_DEBUG) && defined(NL_OS_WINDOWS)
	#include <crtdbg.h>
	//#define PARTICLES_CHECK_MEM nlassert(_CrtCheckMemory());
	#define PARTICLES_CHECK_MEM 
#else
	#define PARTICLES_CHECK_MEM
#endif

///===========================================================================
CParticleSystemShape::CParticleSystemShape() : _MaxViewDist(100.f),
											   _DestroyWhenOutOfFrustum(false),
											   _DestroyModelWhenOutOfRange(false),
											   _UsePrecomputedBBox(false),
											   _Sharing(false)
{
	for (uint k = 0; k < 4; ++k)
	{
		_UserParamDefaultTrack[k].setValue(0);
	}
	_DefaultPos.setValue(CVector::Null);
	_DefaultScale.setValue( CVector(1, 1, 1) );	
	_DefaultRotQuat.setValue(CQuat());
	_DefaultTriggerTrack.setValue(true); // by default, system start as soon as they are instanciated

}

///===========================================================================
void	CParticleSystemShape::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	sint ver = f.serialVersion(6);
	/// version 6 : added sharing flag
	//NLMISC::CVector8 &buf = _ParticleSystemProto.bufferAsVector();
	//f.serialCont(buf);

	if (f.isReading ())
	{
		std::vector<uint8> buf;
		f.serialCont(buf);
		_ParticleSystemProto.fill(&buf[0], buf.size());
	}
	else
	{
		f.serialBufferWithSize ((uint8*)_ParticleSystemProto.buffer(), _ParticleSystemProto.length());
	}

	if (ver > 1)
	{
		// serial default tracks
		for (uint k = 0; k < 4; ++k)
		{
			f.serial(_UserParamDefaultTrack[k]);
		}
	}
	if ( ver > 2)
	{
		f.serial (_DefaultPos);
		f.serial (_DefaultScale);		
		f.serial (_DefaultRotQuat);
	}
	if ( ver > 3)
	{
		f.serial(_MaxViewDist);
		f.serial(_DestroyWhenOutOfFrustum);		
		f.serial(_DestroyModelWhenOutOfRange);
	}
	if ( ver > 4)
	{
		f.serial(_UsePrecomputedBBox);
		if (_UsePrecomputedBBox)
		{
			f.serial(_PrecomputedBBox);
		}
	}
	if ( ver > 5)
	{
		f.serial(_Sharing);
	}
}

///===========================================================================
void CParticleSystemShape::buildFromPS(const CParticleSystem &ps)
{
	// must be sure that we are writting in the stream
	if (_ParticleSystemProto.isReading())
	{
		_ParticleSystemProto.invert();
	}
	
	// to have const correctness in the prototype, we must do this...
	CParticleSystem *myPs = const_cast<CParticleSystem *>(&ps);
	nlassert(myPs);
	// build the prototype
	_ParticleSystemProto.serialPtr(myPs);

	// mirror some system values
	_MaxViewDist = myPs->getMaxViewDist();
	_DestroyWhenOutOfFrustum = myPs->doesDestroyWhenOutOfFrustum();	
	_DestroyModelWhenOutOfRange    = myPs->getDestroyModelWhenOutOfRange();
	if (!myPs->getAutoComputeBBox())
	{
		_UsePrecomputedBBox = true;
		myPs->computeBBox(_PrecomputedBBox);
	}
	else
	{
		_UsePrecomputedBBox = false;
	}
	_Sharing = myPs->isSharingEnabled();
}

///===========================================================================
void	CParticleSystemShape::getAABBox(NLMISC::CAABBox &bbox) const
{
	if (!_UsePrecomputedBBox)
	{
		bbox.setCenter(NLMISC::CVector::Null);
		bbox.setHalfSize(NLMISC::CVector(1, 1, 1));
	}
	else
	{
		bbox = _PrecomputedBBox;
	}
}

///===========================================================================
CParticleSystem *CParticleSystemShape::instanciatePS(CScene &scene)
{
	if (_Sharing && _SharedSystem != NULL) // is sharing enabled, and is a system already instanciated
	{
		return _SharedSystem;
	}

	// copy the datas
	CParticleSystem *myInstance = NULL;

	// serialize from the memory stream	
	if (!_ParticleSystemProto.isReading()) // we must be sure that we are reading the stream
	{
		_ParticleSystemProto.invert();
	}

	_ParticleSystemProto.resetPtrTable();
	_ParticleSystemProto.seek(0, IStream::begin);
	_ParticleSystemProto.serialPtr(myInstance); // instanciate the system	


	myInstance->setScene(&scene);

	if (_Sharing)
	{
		_SharedSystem = myInstance; // set this as the first shared instance
	}

	return myInstance;
}

///===========================================================================
CTransformShape		*CParticleSystemShape::createInstance(CScene &scene)
{
	CParticleSystemModel *psm = NLMISC::safe_cast<CParticleSystemModel *>(scene.createModel(NL3D::ParticleSystemModelId) );
	psm->Shape = this;
	psm->_Scene = &scene; // the model needs the scene to recreate the particle system he holds			
	// by default, we don't instanciate the system. It will be instanciated only if visible and triggered
	// psm->_ParticleSystem = instanciatePS(scene);	

	// Setup position with the default value
	psm->ITransformable::setPos( ((CAnimatedValueVector&)_DefaultPos.getValue()).Value  );
	psm->ITransformable::setRotQuat( ((CAnimatedValueQuat&)_DefaultRotQuat.getValue()).Value  );	
	psm->ITransformable::setScale( ((CAnimatedValueVector&)_DefaultScale.getValue()).Value  );

	// LoadBalancing backward compatibility: ParticleSystems are added to the "Global" Load Balancing Group.
	psm->setLoadBalancingGroup("Global");

	return psm;
}

///===========================================================================
void	CParticleSystemShape::render(IDriver *drv, CTransformShape *trans, bool passOpaque)
{
	H_AUTO ( NL3D_Particles_Render );

	nlassert(drv);
	CParticleSystemModel *psm = NLMISC::safe_cast<CParticleSystemModel *>(trans);
	if (psm->_Invalidated) return;
	CParticleSystem *ps = psm->getPS();
	/// has the system been triggered yet ?
	if (!ps) return;
	
	TAnimationTime delay = psm->getEllapsedTime();
	nlassert(ps->getScene());	

	///////////////////////
	// render particles  //
	///////////////////////

	/// if sharing is enabled,  we should resetup the system matrix
	if (ps->isSharingEnabled())
	{
		ps->setSysMat(trans->getWorldMatrix());
	}
	
	// Setup the matrix.
	/// drv->setupModelMatrix(trans->getWorldMatrix());

	ps->setDriver(drv);
	// draw particle
	PARTICLES_CHECK_MEM;
	if (passOpaque)
	{
		ps->step(CParticleSystem::SolidRender, delay);
	}
	else
	{
		ps->step(CParticleSystem::BlendRender, delay);
	}
	PARTICLES_CHECK_MEM;
	if (psm->isToolDisplayEnabled())
	{
		ps->step(CParticleSystem::ToolRender, delay);
		PARTICLES_CHECK_MEM;
	}
}

} // NL3D
