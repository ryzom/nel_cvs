/** \file particle_system_model.h
 * <File description>
 *
 * $Id: particle_system_model.h,v 1.2 2001/05/23 15:18:00 vizerie Exp $
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

#ifndef NL_PARTICLE_SYSTEM_MODEL_H
#define NL_PARTICLE_SYSTEM_MODEL_H

#include "nel/misc/types_nl.h"
#include "nel/3d/transform_shape.h"


namespace NL3D {

/////////////////////////////////////////////////////////
// THE FOLLOWING CLASS IS FOR INSERTION OF A PARTICLE  //
// SYSTEM IN A MOT MODEL							   //
// SEE also CParticleSystemshape, CParticleSystem	   //	
/////////////////////////////////////////////////////////

class CParticleSystem ;


/// A particle system model : it is build using a CParticleSystemShape
 
class CParticleSystemModel : public CTransformShape
{
	public:
		/// register the basic models and observers
		static	void	registerBasic();

		friend class CParticleSystemShape ;

		/// ctor
		CParticleSystemModel() : _ParticleSystem(NULL), _EllapsedTime(0.01f)
		{}

		/// dtor
		~CParticleSystemModel() ;		

		/// Get the particle system contained in this transform shape
		CParticleSystem *getPS(void)
		{
			nlassert(_ParticleSystem) ; // it should have been set by CParticleSystemShape::createInstance
			return _ParticleSystem ;
		}

		/// Get the particle system contained in this transform shape (const version)
		const CParticleSystem *getPS(void) const
		{
			nlassert(_ParticleSystem) ; // it should have been set by CParticleSystemShape::createInstance
			return _ParticleSystem ;
		}

		/// set the ellapsed time (in second) used for animation.
		void setEllapsedTime(CAnimationTime ellapsedTime) { _EllapsedTime = ellapsedTime ; }
		
		/// get the ellapsed time used for animation
		CAnimationTime getEllapsedTime(void) const { return _EllapsedTime ; }
		

			
		/// to instanciate that model from a scene
		static IModel	*creator() {return new CParticleSystemModel ;}

		/// get the world matrix

		const CMatrix &getWorldMatrix(void)
		{
			IBaseHrcObs *bobs= (IBaseHrcObs *) getObs(HrcTravId);
			return bobs->WorldMatrix;
		}



	protected:
		/** set the particle system for this transform shape after it has been instanciated (from a memory stream)
		 *  see CParticleSystemShape
		 */

		void setParticleSystem(CParticleSystem *ps)
		{
			nlassert(!_ParticleSystem) ;
			_ParticleSystem = ps ;			
		}

		// instance of the particle system that this model holds
		CParticleSystem *_ParticleSystem ;

		CAnimationTime _EllapsedTime ;
} ;




} // NL3D


#endif // NL_PARTICLE_SYSTEM_MODEL_H

/* End of particle_system_model.h */
