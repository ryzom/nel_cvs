/** \file particle_system_shape.cpp
 * <File description>
 *
 * $Id: particle_system_shape.cpp,v 1.12 2001/07/12 15:56:40 vizerie Exp $
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

#include "3d/particle_system.h"
#include "3d/particle_system_shape.h"
#include "3d/particle_system_model.h"
#include "3d/scene.h"
#include "nel/misc/file.h"


namespace NL3D {

using NLMISC::IStream ;
using NLMISC::CIFile ;




// private usage : macro to check the memory integrity
#if defined(NL_DEBUG) && defined(NL_OS_WINDOWS)
	#include <crtdbg.h>
//	#define PARTICLES_CHECK_MEM nlassert(_CrtCheckMemory()) ;
	#define PARTICLES_CHECK_MEM 
#else
	#define PARTICLES_CHECK_MEM
#endif


/////////////////////////////////////////
// CParticleSystemShape implementation //
/////////////////////////////////////////



CParticleSystemShape::CParticleSystemShape()
{
	for (uint k = 0 ; k < 4 ; ++k)
	{
		_UserParamDefaultTrack[k].setValue(0) ;
	}
}


void	CParticleSystemShape::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	sint ver = f.serialVersion(2) ;
	NLMISC::CVector8 &buf = _ParticleSystemProto.bufferAsVector() ;
	f.serialCont(buf) ;
	if (ver > 1)
	{
		// serial default tracks
		for (uint k = 0 ; k < 4 ; ++k)
		{
			f.serial(_UserParamDefaultTrack[k]) ;
		}
	}
}



void CParticleSystemShape::buildFromPS(const CParticleSystem &ps)
{
	// must be sure that we are writting in the stream
	if (_ParticleSystemProto.isReading())
	{
		_ParticleSystemProto.invert() ;
	}
	
	// to have const correctness in the prototype, we must do this...
	CParticleSystem *myPs = const_cast<CParticleSystem *>(&ps) ;


	_ParticleSystemProto.serialPtr(myPs) ;
}


CTransformShape		*CParticleSystemShape::createInstance(CScene &scene)
{
	CTransformShape		*ps = (CTransformShape		*) scene.createModel(NL3D::ParticleSystemModelId);
	ps->Shape= this ;

	// copy the datas
	CParticleSystem *myInstance = NULL ;

	// serialize from the memory stream
	
	if (!_ParticleSystemProto.isReading()) // we must be sure that we are reading the stream
	{
		_ParticleSystemProto.invert() ;
	}

	_ParticleSystemProto.serialPtr(myInstance) ; // instanciate the system
	
	nlassert(dynamic_cast<CParticleSystemModel *>(ps)) ;
	CParticleSystemModel *psi = (CParticleSystemModel *) ps ;
	myInstance->setScene(&scene) ;
	psi->setParticleSystem(myInstance) ;
	return ps ;
}



bool CParticleSystemShape::clip(const std::vector<CPlane>	&pyramid)
{
	// TODO
	return true ;
}


void	CParticleSystemShape::render(IDriver *drv, CTransformShape *trans, bool passOpaque)
{

	nlassert(dynamic_cast<CParticleSystemModel *>(trans)) ;
	nlassert(drv) ;

	CParticleSystemModel *psm = (CParticleSystemModel *) trans ;
	CParticleSystem *ps = psm->getPS() ;
	nlassert(ps) ;
	
	CAnimationTime delay = psm->getEllapsedTime() ;
	nlassert(ps->getScene()) ;	


	// render particles
	

    // TODO : do this during load balancing traversal or the like

	ps->setDriver(drv) ;

	// draw particle
	PARTICLES_CHECK_MEM ;
	if (passOpaque)
	{
		ps->step(PSSolidRender, delay) ;
	}
	else
	{
		ps->step(PSBlendRender, delay) ;
	}

	PARTICLES_CHECK_MEM ;


	if (psm->isToolDisplayEnabled())
	{
		ps->step(PSToolRender, delay) ;
		PARTICLES_CHECK_MEM ;
	}
}

// ***************************************************************************

float CParticleSystemShape::getNumTriangles (float distance)
{
	/// \todo hulud: return num of triangle wanted by this system.
	return 0;
}

// ***************************************************************************

} // NL3D
