/** \file particle_system_model.h
 * <File description>
 *
 * $Id: particle_system_model.h,v 1.24 2002/06/27 16:31:40 berenguier Exp $
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
#include "nel/misc/plane.h"
#include "3d/transform_shape.h"
#include "3d/particle_system.h"
#include "3d/particle_system_manager.h"

#include <vector>

namespace NL3D {

/////////////////////////////////////////////////////////
// THE FOLLOWING CLASS IS FOR INSERTION OF A PARTICLE  //
// SYSTEM IN A MOT MODEL							   //
// SEE also CParticleSystemshape, CParticleSystem	   //	
/////////////////////////////////////////////////////////

class CParticleSystem;
class CParticleSystemClipObs;
class CParticleSystemDetailObs;
class CScene;
class CParticleSystemShape;


/** A particle system model : it is build using a CParticleSystemShape.
  * You should forgot to call the animate() method of the CScene it is part of
  * if you want motion to be performed
  */
 
class CParticleSystemModel : public CTransformShape
{
	public:
	
		///\name Object
			//@{
			/// ctor
			CParticleSystemModel();
			/// dtor
			~CParticleSystemModel();

			/// register the basic models and observers
			static	void				registerBasic();
			//@}


	
		///\name Embedded particle system
			//@{

			/** Get the particle system contained in this transform shape. (NB: This is shared by a smart ptr)
			  * \return pointer to the system, or NULL if no system is currently hold by this model.
			  *			this may happen when the system is not visible and that it has been deleted
			  */
			CParticleSystem				*getPS(void)
			{
				return _ParticleSystem;
			}

			/** Get the particle system (NB : This is shared by a smart ptr) contained in this transform shape. 
			  * This may be null if the model is not visible. 
			  */
			const CParticleSystem		*getPS(void) const
			{		
				return _ParticleSystem;
			}
			
			/** Set the particle system for this transform shape after it has been instanciated (from a memory stream, or by sharing)
			 *  see CParticleSystemShape
			 */
			void						setParticleSystem(CParticleSystem *ps)
			{
				nlassert(!_ParticleSystem);
				_ParticleSystem = ps;	
				updateOpacityInfos();
			}
			//@}

		///\name Life managment 
			//@{
			/**
			 * test wether the system has become invalid. The condition for a system to be invalid
			 * are encoded in the system itself (no more particles for example). When a system has become invalid, you may want to remove it most of the time
			 */
			bool isInvalid(void) const { return _Invalidated; }

			/// interface for object that observe this model. They will be notified when it becomes invalid
			struct IPSModelObserver
			{
				/// called when a system has been invalidated
				virtual void invalidPS(CParticleSystemModel *psm) = 0;
			};
							
			/// register an observer that will be notified when this model becomes invalid
			void registerPSModelObserver(IPSModelObserver *obs);
 
			/** remove an observer
			  * \see registerPSModelObserver
			  */
			void removePSModelObserver(IPSModelObserver *obs);
			
			/// test wether obs observe this model
			bool isPSModelObserver(IPSModelObserver *obs);

			//@}
						

		//\name Time managment
			//@{

			/** when called with true, this force the model to querry himself the ellapsed time to the scene.
			  * This is the default. Otherwise, setEllapsedTime must be called
			  */
			void						enableAutoGetEllapsedTime(bool enable = true) 
			{ 
				_AutoGetEllapsedTime = enable; 
			}

			/** This apply a ratio on the ellapsed time. This can be used to slow down a system
			  * This must be in the >= 0.
			  * 1 means the system run at normal speed
			  */
			void						setEllapsedTimeRatio(float value)
			{
				nlassert(value >= 0);
				_EllapsedTimeRatio = value;
			}
			//
			float						getEllapsedTimeRatio() const { return _EllapsedTimeRatio; }			

			/// tells wether the model will querry himself for the ellapsed time
			bool						isAutoGetEllapsedTimeEnabled(void) const 
			{ 
				return _AutoGetEllapsedTime; 
			}

			/// set the ellapsed time (in second) used for animation. 		  
			void						setEllapsedTime(TAnimationTime ellapsedTime) 
			{ 
				_EllapsedTime = ellapsedTime; 
			}
			
			/// get the ellapsed time used for animation
			TAnimationTime				getEllapsedTime(void) const 
			{ 
				return _EllapsedTime; 
			}

			//@}
		
		///\name Edition related methods
			//@{
			/// activate the display of tool (for edition purpose)
			void						enableDisplayTools(bool enable = true) 
			{ 
				_ToolDisplayEnabled = enable; touchTransparencyState(); 
			}

			// check wether the display of tools is enabled
			bool						isToolDisplayEnabled(void) const 
			{ 
				return _ToolDisplayEnabled; 
			}			

			/** force the edition mode : this will prevent the system from being removed when it is out of range.
			 * When the model is first allocated, the system resource are not allocated until it becomes visible.
			 * This also forces the resources to be allocated.
			 * when there are no more particles in it etc. (this also mean that you can safely keep a pointer on it)
			 * This flag is not saved.
			 */
			void						setEditionMode(bool enable = true) ;
			

			/// test if edition mode is activated
			bool						getEditionMode(void) const 
			{ 
				return _EditionMode; 
			}
			/// edition purpose : touch the system to tell that the transparency state of the system has changed (added/removes opaque/tansparent faces )
			void						touchTransparencyState(void) 
			{ 
				_TransparencyStateTouched = true; 
			}
			//@}
	
		///\name User params / animation
			//@{
			/// for now, we have 4 animatables value in a system
			enum	TAnimValues
			{
				OwnerBit= CTransformShape::AnimValueLast,
				PSParam0,
				PSParam1,
				PSParam2,
				PSParam3,
				PSTrigger, // trigger the instanciation of the system
				AnimValueLast,
			};

	
			virtual IAnimatedValue		*getValue (uint valueId);
			virtual const char			*getValueName (uint valueId) const; 
			static const char			*getPSParamName (uint valueId);			
			virtual ITrack				*getDefaultTrack (uint valueId);		
			virtual	void				registerToChannelMixer(CChannelMixer *chanMixer
															   , const std::string &prefix=std::string());
			//@}


		/** This update the infos about opacity (e.g are there solid faces and / or transparent faces in the system).
		  * This must be called when the system is instanciated, or when attributes have changed, such as the blending mode
		  */
		void						updateOpacityInfos(void);

		/// get the world matrix
		const CMatrix				&getWorldMatrix(void)
		{
			IBaseHrcObs *bobs= (IBaseHrcObs *) getObs(HrcTravId);
			return bobs->WorldMatrix;
		}		

		virtual void				getAABBox(NLMISC::CAABBox &bbox) const;


		/// inherited from CTransformShape. Returns the number of triangles wanted depeneding on the distance
		virtual float				getNumTriangles (float distance);

			
		/// to instanciate that model from a scene
		static IModel				*creator() 
		{
			return new CParticleSystemModel;
		}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

	protected:

		friend class CParticleSystemShape;
		friend class CParticleSystemDetailObs;
		friend class CParticleSystemClipObs;
		friend class CParticleSystemRenderObs;
		friend class CParticleSystemManager;


		/// Called when the resource (attached system) for this system must be reallocated
		void reallocRsc();

		/** Called by the particle system manager to see wether this model resources must be released
		  * If this is the case, this releases the resource and return true.
		  * IMPORTANT: the handle of the model in the p.s manager is automatically removed
		  * when this returns true
		  */
		bool refreshRscDeletion(const std::vector<CPlane>	&worldFrustumPyramid,  const NLMISC::CVector &viewerPos);

		// Release the resources (attached system) of this model, but doesn't 
		void releaseRsc();

		// Mark this system model as invalid, delete the attached system, and calls his observers
		void releaseRscAndInvalidate();


		/// Return true if the system is in the given world pyramid
		bool checkAgainstPyramid(const std::vector<CPlane>	&worldFrustumPyramid) const;

	protected:		
		CParticleSystemManager::TModelHandle    _ModelHandle; /** a handle to say when the resources
																* of the model (_ParticleSystem) are deleted
																*/

		CParticleSystemManager::TModelHandle    _AnimatedModelHandle; // handle for permanenlty animated models
																
		bool									_AutoGetEllapsedTime;		
		NLMISC::CSmartPtr<CParticleSystem>		_ParticleSystem;
		CScene							  	   *_Scene;
		TAnimationTime						    _EllapsedTime;
		float									_EllapsedTimeRatio;

		///\todo nico : may optimize this with a bitfield...
		bool									_ToolDisplayEnabled;		
		bool									_TransparencyStateTouched;
		bool									_EditionMode;
		bool									_Invalidated; /// it false, system should be recreated
		bool									_InsertedInVisibleList;
		bool									_InClusterAndVisible;
		std::vector<IPSModelObserver *>			_Observers;		

		CAnimatedValueBool						_TriggerAnimatedValue;
		/// user params of the system
		CAnimatedValueFloat						_UserParam[MaxPSUserParam];
};


/** Detail animation observer for a particle system. It perform motion of the particles
  * (so, motion occurs only when the system has not be clipped)
  */
class	CParticleSystemDetailObs : public CTransformAnimDetailObs
{
public:

	/** this do :
	 *  - call CTransformAnimDetailObs::traverse()
	 *  - update particles.
	 */
	virtual	void	traverse(IObs *caller);	


public:
	static IObs	*creator() {return new CParticleSystemDetailObs;}
};


/** a clip observer for a particle system
  *
  */

class CParticleSystemClipObs : public CTransformClipObs
{
public:
	void				traverse(IObs *caller);
	static IObs			*creator() {return new CParticleSystemClipObs;}
	
	// insert the observer in the Clip/AnimDetail/LoadBalacing visible list.
	void				insertInVisibleList()
	{
		CParticleSystemModel	*m= (CParticleSystemModel*)Model;	
		CClipTrav				*trav= (CClipTrav*)Trav;
		// if not already not inserted
		if (!m->_InsertedInVisibleList)
		{
			m->_InsertedInVisibleList = true;
			// add to clip/anim/load Trav.
			trav->addVisibleObs(this);
			// NB: no need to test isAnimDetailable()... for PS, always add them
			trav->AnimDetailTrav->addVisibleObs(static_cast<CTransformAnimDetailObs*>(AnimDetailObs));
			trav->LoadBalancingTrav->addVisibleObs(LoadBalancingObs);
		}
	}
};

/// a render observer for a particle system

class CParticleSystemRenderObs : public CTransformShapeRenderObs
{
public:
	/// render the instance and Don't traverseSons().
	virtual	void	traverse(IObs *caller);	
	static IObs	*creator() {return new CParticleSystemRenderObs;}
};




} // NL3D






#endif // NL_PARTICLE_SYSTEM_MODEL_H

/* End of particle_system_model.h */
