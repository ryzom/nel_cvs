/** \file particle_system_model.h
 * <File description>
 *
 * $Id: particle_system_model.h,v 1.8 2001/07/18 08:00:36 vizerie Exp $
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
#include "3d/transform_shape.h"


namespace NL3D {

/////////////////////////////////////////////////////////
// THE FOLLOWING CLASS IS FOR INSERTION OF A PARTICLE  //
// SYSTEM IN A MOT MODEL							   //
// SEE also CParticleSystemshape, CParticleSystem	   //	
/////////////////////////////////////////////////////////

class CParticleSystem ;
class CParticleSystemClipObs ;
class CParticleSystemDetailObs ;
class CScene ;
class CParticleSystemShape ;


/// A particle system model : it is build using a CParticleSystemShape
 
class CParticleSystemModel : public CTransformShape
{
	public:
		/// register the basic models and observers
		static	void	registerBasic();
	

		/// ctor
		CParticleSystemModel() ;
		/// dtor
		~CParticleSystemModel() ;

		/**this struct is used to notify that a particle system has removed himself
		  */
		struct IPSModelObserver
		{
			/// called when a ParticleSystemModel has been removed
			virtual void psDestroyed(CParticleSystemModel *psm) = 0 ; 
		} ;
		
		/** This model may remove himself from the scene when the particle system has been created
		  * with a flag that precise it. This allow to register an observer that will be warned when the system is detroyed
		  * This also means that you can't safely keep a pointer on a particle system model in this case ...
		  * An assertion occurs when the caller has been registered before
		  *  \see removeDtorObserver()
		  *  \see isDtorObserver
		  */
		void registerDtorObserver(IPSModelObserver *obs) ;

		/** remove  dtor observer
		  * \see registerDtorObserver()
		  * \see isDtorObserver() ;
		  */
		void removeDtorObserver(IPSModelObserver *obs) ;

		/// check wether obs is an observer of this model
		bool isDtorObserver(IPSModelObserver *obs) ;

		/** Get the particle system contained in this transform shape
		  * \return pointer to the system, or NULL if no system is currently hold by this model.
		  *			this may happen when the system is not visible and that it has been deleted
		  */
		CParticleSystem *getPS(void)
		{
			return _ParticleSystem ;
		}

		/** Get the particle system contained in this transform shape. this may be null if the model
		  * Is not visible
		  */
		const CParticleSystem *getPS(void) const
		{		
			return _ParticleSystem ;
		}

		/** when called with true, this force the model to querry himself the ellapsed time to the scene.
		  * This is the default. Otherwise, setEllapsedTime must be called
		  */
		void enableAutoGetEllapsedTime(bool enable = true) { _AutoGetEllapsedTime = enable ; }

		/// tells wether the model will querry himself for the ellapsed time
		  bool isAutoGetEllapsedTimeEnabled(void) const { return _AutoGetEllapsedTime ; }

		/// set the ellapsed time (in second) used for animation. 		  
		void setEllapsedTime(CAnimationTime ellapsedTime) { _EllapsedTime = ellapsedTime ; }
		
		/// get the ellapsed time used for animation
		CAnimationTime getEllapsedTime(void) const { return _EllapsedTime ; }
		

		/// activate the display of tool (for edition purpose)
		void enableDisplayTools(bool enable = true) { _ToolDisplayEnabled = enable ; touchTransparencyState() ; }

		// check wether the display of tools is enabled
		bool isToolDisplayEnabled(void) const { return _ToolDisplayEnabled ; }
			
		/// to instanciate that model from a scene
		static IModel	*creator() {return new CParticleSystemModel ;}

		/// get the world matrix

		const CMatrix &getWorldMatrix(void)
		{
			IBaseHrcObs *bobs= (IBaseHrcObs *) getObs(HrcTravId);
			return bobs->WorldMatrix;
		}

		/** This update the infos about opacity (e.g are there solid faces and / or transparent faces in the system).
		  * This must be called when the system is instanciated, or when attributes have changed, such as the blending mode
		  */

		void updateOpacityInfos(void) ;


	
		/** set the particle system for this transform shape after it has been instanciated (from a memory stream)
		 *  see CParticleSystemShape
		 */

		void setParticleSystem(CParticleSystem *ps)
		{
			nlassert(!_ParticleSystem) ;
			_ParticleSystem = ps ;	
			updateOpacityInfos() ;
		}


		/// for now, we have 4 animatables value in a system
		enum	TAnimValues
		{
			OwnerBit= CTransformShape::AnimValueLast,
			PSParam0,
			PSParam1,
			PSParam2,
			PSParam3,
			AnimValueLast,
		};

	
		virtual IAnimatedValue* getValue (uint valueId) ;

		virtual const char *getValueName (uint valueId) const ; 

		static const char *getPSParamName (uint valueId) ;

	
		virtual ITrack* getDefaultTrack (uint valueId) ;

	
		virtual	void	registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix=std::string()) ;


		/// edition purpose : touch the system to tell that the transparency state of the system has changed (added/removes opaque/tansparent faces )
		void touchTransparencyState(void) { _TransparencyStateTouched = true ; }


		/** force the edition mode : this will prevent the system from being removed when it is out of range
		 * when there are no more particles in it etc. (this also mean that you can safely keep a pointer on it)
		 * This flag is not saved.
	     */
		void setEditionMode(bool enable = true) { _EditionMode = true ; }

		/// test if edition mode is activated
		bool getEditionMode(void) const { return _EditionMode ; }

	protected:

		friend class CParticleSystemShape ;
		friend class CParticleSystemDetailObs ;
		friend class CParticleSystemClipObs ;


		bool									_AutoGetEllapsedTime ;		
		CParticleSystem						   *_ParticleSystem ;
		CScene							  	   *_Scene ;
		CAnimationTime						    _EllapsedTime ;
		bool									_ToolDisplayEnabled ;		
		bool									_TransparencyStateTouched ;
		bool									_EditionMode ;
		std::vector<IPSModelObserver *>			_Observers ;
} ;


/** Detail animation observer for a particle system. It perform motion of the particles
  */
class	CParticleSystemDetailObs : public CTransformAnimDetailObs
{
public:

	/** this do :
	 *  - call CTransformAnimDetailObs::traverse() => traverseSons.
	 *  - update particles.
	 */
	virtual	void	traverse(IObs *caller) ;	


public:
	static IObs	*creator() {return new CParticleSystemDetailObs;}
};


/** a clip observer for a particle system
  *
  */

class CParticleSystemClipObs : public CTransformClipObs
{
public:
	virtual	bool	isRenderable() const {return true;}
	void	traverse(IObs *caller) ;
	static IObs	*creator() {return new CParticleSystemClipObs ;}
	
};






} // NL3D






#endif // NL_PARTICLE_SYSTEM_MODEL_H

/* End of particle_system_model.h */
