/** \file particle_system.h
 * <File description>
 *
 * $Id: particle_system.h,v 1.31 2003/04/07 12:34:45 vizerie Exp $
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

#ifndef NL_PARTICLE_SYSTEM_H
#define NL_PARTICLE_SYSTEM_H

#include "nel/misc/types_nl.h"
#include "nel/misc/matrix.h"
#include "nel/misc/aabbox.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/rgba.h"
#include "nel/3d/animation_time.h"
#include "3d/animated_value.h"
#include "3d/particle_system_process.h"
#include "3d/ps_lod.h"
#include "3d/ps_attrib_maker.h"

#include <map>



namespace NL3D 
{

class CParticleSystemShape;
class CPSLocatedBindable;
class CFontGenerator;
class CFontManager;
class CPSCopyHelper;
class CScene;
class CPSLocated;
class IDriver;
struct UPSSoundServer;



/// number user params for a particle system
const uint MaxPSUserParam = 4;


/** Particles system classes. They can be used as it. If you want to use a particle system in 
 *  a scene (M.O.T model), see particle_system_shape.h and particle_system_instance.h
 *  TODO : give example of use here...
 */




/**
 * This class holds a particle system. Most of the time it is used with a particle system model.
 * See particle_system_shape.h and particle_system_model.h for more details.
 * It can be used directly to create a shape.
 * If you plan to use this without a particle system model, make sure :
 * - you've setup the driver before calls to step()
 * - you've setup the font manager if you want to display font informations
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CParticleSystem : public NLMISC::CRefCount
{
public:
	// the pass that is applied on particles
	enum TPass { Anim, SolidRender, BlendRender, ToolRender };
public:
	//*****************************************************************************************************

	///\name Object
		//@{
			/// ctor
			CParticleSystem();
			/// dtor
			virtual ~CParticleSystem();	
			/// serialize this particle system
			void serial(NLMISC::IStream &f)  throw(NLMISC::EStream);
			/** Merge this system with a system instanciated from the given shape
			  * NB : for edition purpose, this is slow
			  */
			void merge(CParticleSystemShape *toMerge);

			/*** duplication method NOT SUPPORTED for now (duplication is using streams, but it may not last)
			 * \param ch for private use, set to null by default
			 */
			//	CParticleSystem *clone(CPSCopyHelper *ch = NULL) ;
		//@}

	//*****************************************************************************************************

	///\name Sharing system
		//@{
			/** Enable/Disable sharing. When sharing is enabled, the state of a particle system is the same for
			  * all the system that have the same shape. This allow to gain memory.
			  * However, such system should not be built with LOD in mind (for example, less emission with distance)
			  * LOD should be automatic for these system (see Auto-Lod). This means that sharing is only useful for system that
			  * have the same state, and if they are numerous : motion is performed once, but only for one system with no LOD.
			  * LOD is done during display only (when activated).
			  * The default for systems is to have no sharing.
			  */
			void	enableSharing(bool enabled = true) { _Sharing = enabled; }

			/// Test wether sharing is enabled
			bool	isSharingEnabled() const { return _Sharing; }
		//@}

	//*****************************************************************************************************	  

	///\name Driver setup
		//@{
		/// set the driver use to render the system
		void setDriver(IDriver *driver) { _Driver = driver; }

		/// return the driver that will be used for rendering
		IDriver *getDriver(void) { return _Driver; }
		//@}
	//*****************************************************************************************************

	///\name Scene setup
		//@{
		/** Set the scene in which the particle system is inserted. This is needed when
		 * system must add objects to the scene (for particle that are mesh for instance)
		 */
		void setScene(CScene *scene) { _Scene = scene; }

		//// get the scene set by setScene()
		CScene *getScene(void) { return _Scene; }
		//@}

	//*****************************************************************************************************
	
	///\name Position of the system
		//@{


		/** Set the matrix for the system. This only affect elements that are in the same basis
		 * you don't need to call this if it is used with a CPaticleSystemModel : Call ITransformable method. In this case
		 * , setSysMat will be called automatically when needed to mirror the ITransformable matrix.
		 * NB: The previous matrix is backuped during this call (used to interpolate the system when emission occur in the world basis),
		 * so this should be called only once per frame
		 */
		 
		void setSysMat(const NLMISC::CMatrix &m);
		
		/// return the matrix of the system
		const NLMISC::CMatrix &getSysMat(void) const { return _SysMat; }

		/// return the previous matrix of the system
		const NLMISC::CMatrix &getOldSysMat(void) const { return _SysMat; }

		/// return the inverted matrix of the system
		const NLMISC::CMatrix &getInvertedSysMat(void) const { return _InvSysMat; } 

		/** set the view matrix  
		  * This must be called otherwise results can't be correct
		  */
		void setViewMat(const NLMISC::CMatrix &m);

		/// get the view matrix .
		const NLMISC::CMatrix &getViewMat(void) const { return _ViewMat; }

		/// get the inverted view matrix . It is stored each time a new frame is processed	 
		const NLMISC::CMatrix &getInvertedViewMat(void) const { return _InvertedViewMat; }

		//@}


	//*****************************************************************************************************

	///\name Execution of the system
		//@{

		/**
		* execute all the process of the system. It uses the driver that was set by a call to setDriver. 
		* \param ellapsedTime The ellapsed time since the last call
		* \param pass the pass to be executed
		* \see setDriver
		*/
		virtual void step(TPass pass, TAnimationTime ellapsedTime);
		//@}

		

		/// used for benchs. must be reset by the user
		static uint32 NbParticlesDrawn;

	//*****************************************************************************************************

	/**\name Process attachment. Most process are located : set of objects of the same type that have a position
	  * in space
	  */

		//@{
		/** Attach a process (such as a located : see particle_system_process.h, and ps_located.h) to the system. 
		 *  It is then owned by the process and will be deleted by it. 
		 *  if already present -> nl assert	 
		 */
		void						attach(CParticleSystemProcess *process);

		/** Detach a process from the system (but do not delete it)		  
		  */ 
		CParticleSystemProcess		*detach(uint index);

		/** Test wether a process is part of this system
		  */
		bool						isProcess(CParticleSystemProcess *process) const;

		/** Given its pointer, return an index to a process.
		  * The process must be part of the system, otherwise an assertion is raised
		  */
		uint						getIndexOf(const CParticleSystemProcess *process) const;

		/** Remove a process
		 * It is deleted by the system
		 *  if not present -> nl assert
		 */
		void						remove(CParticleSystemProcess *process);

		/// get the number of process that are attached to the system
		uint32						getNbProcess(void) const { return _ProcessVect.size(); }

		/**
		 *  Get a pointer to the nth process.
		 *  Out of range -> nlassert
		 */	
		CParticleSystemProcess		*getProcess(uint32 index)
		{ 
			nlassert(index < _ProcessVect.size());
			return _ProcessVect[index]; 
		}

		/**
		 *  Get a const pointer to the nth process.
		 *  Out of range -> nlassert
		 */	
		const CParticleSystemProcess *getProcess(uint32 index) const
		{ 
			nlassert(index < _ProcessVect.size());
			return _ProcessVect[index]; 
		}

		//@}

	//*****************************************************************************************************

	///\name Date / Time
		//@{

		/// get the time ellapsed since the system was created.
		TAnimationTime getSystemDate(void) const { return _SystemDate; }

		/** Get the date of the system (the number of time it has been drawn in fact)
		 *  This may be used to skip frames in an animation for example.
		 */

		uint64 getDate(void) const
		{
			return _Date;
		}
		//@}

	//*****************************************************************************************************

	/**\name User parameters. They may be or not used by the system. Their meaning is defined during the construction
	  * of the system
	  */

	//@{
		/** Set the value of a user parameter. It must range from 0 to 1. The user value are not saved, and their default value is 0.f.
		  * The max number of user param is MaxPSUserParam.
		  */
		void setUserParam(uint userParamIndex, float value) 
		{
			nlassert(userParamIndex < MaxPSUserParam);
			NLMISC::clamp(value, 0, MaxInputValue);			
			_UserParam[userParamIndex] = value;
		}

		/** Get a user param.
		  * The max number of user param is in MaxPSUserParam.
		  */
		float getUserParam(uint userParamIndex) const
		{
			nlassert(userParamIndex < MaxPSUserParam);
			return _UserParam[userParamIndex];
		}

		/** Bind/Unbind a global value to a user param.
		  * Any further call to setUserParam will then be overriden by the user param.
		  * Example of use : global strenght of wind.
		  * \param globalValueName NULL to unbind the value, or the name of the value
		  */
		void  bindGlobalValueToUserParam(const std::string &globalValueName, uint userParamIndex);
		// Get name of a global value, or NULL if no global value is bound to that user param
		std::string getGlobalValueName(uint userParamIndex) const;		 
		// Set a global value
		static void  setGlobalValue(const std::string &name, float value);
		// Get a global value
		static float getGlobalValue(const std::string &name);
		/** Set a global vector value. Global vector values are set to (0, 0, 0) by default
		  * Global vector values are used in some places. For example, direction for a directionnal force.
		  */
		static void					 setGlobalVectorValue(const std::string &name, const NLMISC::CVector &value);
		// Get a global vector value
		static NLMISC::CVector		 getGlobalVectorValue(const std::string &name);
		// define a handle to a global value
		class CGlobalVectorValueHandle
		{
		public:
			CGlobalVectorValueHandle() { reset(); }
			const NLMISC::CVector &get() const						 { nlassert(_Value); return *_Value; }
			void				   set(const NLMISC::CVector &value) { nlassert(_Value); *_Value = value; }
			const std::string     &getName() const					 { nlassert(_Name); return *_Name; }
			bool				   isValid() const { return _Name != NULL && _Value != NULL; }
			void                   reset() { _Name = NULL; _Value = NULL; }			
		/////////////////////////////
		private:
			friend class CParticleSystem;
			const std::string *_Name;
			NLMISC::CVector   *_Value;		
		};
		// Get a handle on a global value that provide a quick access on it (no map lookup)
		static CGlobalVectorValueHandle     getGlobalVectorValueHandle(const std::string &name);		
	//@}

		

	//*****************************************************************************************************

	///\name Edition methods : provides some tools for an external editor
		// @{
		/** For edition purposes only : this allow to highlight in red the current element being edited.
		 *  \param located The located the current element belongs to, or NULL if no element is selected.
		 *  \index the index of the element in the located.
		 *  \lb the located bindable that is selected into a located (NULL = all)
		 */
		 void setCurrentEditedElement(CPSLocated *loc = NULL , uint32 index = 0, class CPSLocatedBindable *bd = NULL )
		 {
			_CurrEditedElementLocated = loc;
			_CurrEditedElementLocatedBindable = bd; 
			_CurrEditedElementIndex = index;
		 }

		/** Retrieve the current edited element
		 *  \see setCurrentEditedElement()
		 */
		 void getCurrentEditedElement(CPSLocated *&loc , uint32 &index, CPSLocatedBindable *&lb)
		 {
			loc = _CurrEditedElementLocated;
			index = _CurrEditedElementIndex;
			lb = _CurrEditedElementLocatedBindable;
		 }
	
		/// Set a font generator. Useful only for edition. don't need that in runtime.
		void setFontGenerator(CFontGenerator *fg) { _FontGenerator = fg; }

		/// Retrieve the font generator. Edition purpose only.
		CFontGenerator *getFontGenerator(void) { return _FontGenerator; }

		/// Retrieve the font generator (const version). Edition purpose only.
		const CFontGenerator *getFontGenerator(void) const { return _FontGenerator; }

		/// Set a font Manager. Useful only for edition. don't need that in runtime
		void setFontManager(CFontManager *fg) { _FontManager = fg; }

		/// Retrieve the font Manager. Edition purpose only.
		CFontManager *getFontManager(void) { return _FontManager; }

		/// Retrieve the font Manager (const version). Edition purpose only.
		const CFontManager *getFontManager(void) const { return _FontManager; }
		// @}
		
		/// Set the name of the system.
		void setName(const std::string &s) { _Name = s; }

		/// Get the name of the system.
		std::string getName(void) const { return _Name; }
		
	//*****************************************************************************************************

	///\name Transparency / opacity
	// @{
		/// Return true if the system has opaque object in it.
		bool hasOpaqueObjects(void) const;

		/// Return true if the system has transparent objects in it.
		bool hasTransparentObjects(void) const;
	// @}

	//*****************************************************************************************************

	///\name Integration parameters
	// @{
		/** This enable for more accurate integrations of movement. When this is activated,
		  *  integration is performed in a more accurate way when the ellapsed time goes over a threshold, but it is more slow to perform.	  
		  */
		void enableAccurateIntegration(bool enable = true) { _AccurateIntegration = enable; }
		bool isAccurateIntegrationEnabled(void) const { return _AccurateIntegration; }

		/** the the time threshold and the max number of integration to perform, when accurate integration is activated.
		  * The default is 0.15 for time threshold and 2 for max NbIntegrations
		  * \param canSlowDown : Allow the system to slow down in speed but to keep accuracy in its movement.
		  *  It is useful for critical situations where the framerate is very low. The default is true.
		  */
		void setAccurateIntegrationParams(TAnimationTime threshold,
										  uint32 maxNbIntegrations,
										  bool canSlowDown,
										  bool keepEllapsedTimeForLifeUpdate
										  )
		{
			_TimeThreshold = threshold;
			_MaxNbIntegrations = maxNbIntegrations;
			_CanSlowDown = canSlowDown;
			_KeepEllapsedTimeForLifeUpdate = keepEllapsedTimeForLifeUpdate;
		}

		/** get the parameters used for integration.
		  * \see setAccurateIntegrationParams()
		  */
		void getAccurateIntegrationParams(TAnimationTime &threshold,
										  uint32 &maxNbIntegrations,
										  bool &canSlowDown,
										  bool &keepEllapsedTimeForLifeUpdate
										 )
		{
			threshold = _TimeThreshold ;
			maxNbIntegrations = _MaxNbIntegrations;
			canSlowDown = _CanSlowDown;
			keepEllapsedTimeForLifeUpdate = _KeepEllapsedTimeForLifeUpdate;
		}


		
	// @}

	//*****************************************************************************************************

	/**\name LOD managment. LOD, when used  can be performed in 2 ways :
	  *						- Hand tuned LOD (for emission, color, size : this uses LOD as an input for attribute makers).
	  *                     - Auto LOD : - With non-shared systems, it modulates the emission period, quantity etc.. to get the desired result.
	  *									 - With shared systems : One version is animated with full LOD (no hand tuned LOD should be applied !).
	  *                                                          All version are displayed with fewer particle than the full LOD, depending on their distance. Visually, this is not as good as hand-tuned system, or auto-LOD on non-shared systems, however ..
	  */


		// @{

		/// set the max view distance for the system (in meters) . The default is 50 meters.
		void setMaxViewDist(float maxDist) 
		{ 
			nlassert(maxDist > 0.f); 
			_MaxViewDist = maxDist;
			_InvCurrentViewDist = _InvMaxViewDist = 1.f / maxDist; 
		}

		/// get the max view distance
		float getMaxViewDist(void) const { return _MaxViewDist; }

		/// set a percentage that indicate where the 2nd LOD is located. Default is 0.5.
		void setLODRatio(float ratio) { nlassert(ratio > 0 && ratio <= 1.f); _LODRatio =  ratio; }

		/// get the lod ratio.
		float getLODRatio(void) const  { return _LODRatio; }


		/** compute a vector and a distance that are used for LOD computations. 
		  * You'll have for a given pos : pos * v + offset  = 0 at the nearest point, and 1 when
		  * pos is at maxDist from the viewer. This is used by sub-component of the system.
		  */
		void getLODVect(NLMISC::CVector &v, float &offset, bool systemBasis);

		/// get the current LOD of the system. It is based on the distance of the center of the system to the viewer
		TPSLod getLOD(void) const;	

		/// get 1.f - the current lod ratio (it is updated at each motion pass)
		float getOneMinusCurrentLODRatio(void) const { return _OneMinusCurrentLODRatio; }

		/** Enable / disbale auto-lod.
		  * When auto-LOD is enabled, less particles are displayed when the system is far.
		  * This apply to all particles in the systems (unless they override that behaviour).
		  * The default is AutoLOD off.
		  */
		void	enableAutoLOD(bool enabled = true) { _AutoLOD = enabled; }

		/// test wether Auto-LOD is enabled
		bool    isAutoLODEnabled() const { return _AutoLOD; }

		/** Setup auto lod parameters.
		  * \param start A percentage of the max view dist that tells when the auto-lod must start.
		  * \param strenght The degradation speed. It is interpreted as an exponent.
		  */
		void    setupAutoLOD(float startDistPercent, uint8 degradationExponent)
		{
			nlassert(startDistPercent < 1.f);
			nlassert(degradationExponent > 0);
			_AutoLODStartDistPercent    = 	startDistPercent;
			_AutoLODDegradationExponent =	degradationExponent;
		}
		
		/** when auto-lod on a non shared system is used, this set the degradation of the system when it is far
		  * A value of 0 mean no more emissions at all.
		  * A value of 0.1 means 10% of emission and so on.
		  * A value of 1 means there's no LOD at all..
		  */
		void    setMaxDistLODBias(float lodBias);
		float   getMaxDistLODBias() const { return _MaxDistLODBias; }		
		

		
		float	getAutoLODStartDistPercent() const { return _AutoLODStartDistPercent; }
		uint8   getAutoLODDegradationExponent() const { return _AutoLODDegradationExponent; }		

		/** There are 2 modes for the auto-LOD (apply to shared systems only) :
		  * - Particle are skip in the source container when display is performed (the default)
		  * - There are just less particles displayed, but this can lead to 'pulse effect'. This is faster, though.
		  */
		void	setAutoLODMode(bool skipParticles) { _AutoLODSkipParticles = skipParticles; }
		bool    getAutoLODMode() const { return _AutoLODSkipParticles; }

		/** Setup a color attenuation scheme with the distance from the viewer. This doesn't act on a particle basis,
		  * the whole color of the system is changed in an uniform way so it is fast (the same can be achieved on a particle basis).
		  * This bypass the source of the scheme : it is set to 0 when the system is on the user, and to 1 when
		  * it is at its max distance. 
		  * \param scheme A color scheme, that is then owned by this object. NULL disable color attenuation. Any previous scheme is removed
		  */
		  void	setColorAttenuationScheme(CPSAttribMaker<NLMISC::CRGBA> *colScheme)
		  {
			delete _ColorAttenuationScheme;
			_ColorAttenuationScheme = colScheme;
			if (!colScheme)
			{
				_GlobalColor = NLMISC::CRGBA::White;
			}
		  }

		  /// Get the global color attenuation scheme
		  CPSAttribMaker<NLMISC::CRGBA> *getColorAttenuationScheme() { return _ColorAttenuationScheme; }
		  const CPSAttribMaker<NLMISC::CRGBA> *getColorAttenuationScheme() const { return _ColorAttenuationScheme; }

		  /** Get the current global color of the system. (It is updated just before drawing...). It there's
		    * no color attenuation scheme it can be assumed to be white
			*/
		  const NLMISC::CRGBA &getGlobalColor() const {	return _GlobalColor; }
		

		// @}

	//*****************************************************************************************************

		// \name Load balancing
		// @{
			// get an evaluation of how many tris are needed with the system for the given distance
			float getWantedNumTris(float dist);

			/// set the number of tree the system may use. If not clled this will be the max
			void setNumTris(uint numFaces);

			/** Ask for the particle system to reevaluate the max number of faces it may need.
			  * You don't usually need to call this
			  */
			void notifyMaxNumFacesChanged(void);

			/// Test whether load balancing has been activated for that system
			bool isLoadBalancingEnabled() const { return _EnableLoadBalancing; }

			/// Enable / disable load balancing. By default its on
			void enableLoadBalancing(bool enabled = true);
		// @}

	//*****************************************************************************************************

	///\name Bounding box managment
		// @{		
		/** Compute the aabbox of this system, (expressed in thesystem basis)	
		 *  \param aabbox a ref to the result box
		 */
		void computeBBox(NLMISC::CAABBox &aabbox);

		/** When this is set to false, the system will recompute his bbox each time it is querried
		  * This may be needed for systems that move fast. 
		  */
		void setAutoComputeBBox(bool enable = true) { _ComputeBBox = enable; }


		/// test whether the system compute himself his bbox
		bool getAutoComputeBBox(void) const { return _ComputeBBox; }


		/** set a precomputed bbox (expressed in the system basis). This is allowed only when setAutoComputeBBox 
		  * is called with false (nlassert otherwise).
		  */

		void setPrecomputedBBox(const NLMISC::CAABBox &precompBBox) 
		{ 
			nlassert(!_ComputeBBox);
			_PreComputedBBox = precompBBox;
		}
			
		/// get the last computed bbox
		void getLastComputedBBox(NLMISC::CAABBox &dest) { dest = _PreComputedBBox; }
		// @}
	
	//*****************************************************************************************************

	///\name Invalidity flags (no direct effect, just indications for a third party, a model holding the system for example)
		// @{
		/** Tell the system that it is invalid when its out of range. The default is false.	  
		  * This is only a indication flag and must be checked by third party (a model holding the system for example)
		  */
		void				setDestroyModelWhenOutOfRange(bool enable = true) 
		{ 
			_DestroyModelWhenOutOfRange  = enable;
			_PresetBehaviour = UserBehaviour;
		}
		
		/// check whether the system is invalid it's out of range.
		bool				getDestroyModelWhenOutOfRange(void) const { return _DestroyModelWhenOutOfRange; }


		/// this enum give consitions on which the system may be invalid
		enum TDieCondition { none, noMoreParticles, noMoreParticlesAndEmitters   };


		/** when != to none, the Model hodling this sytem will be considered invalid when dieCondition is met
		  * This is only an indication flag and must be checked by third party (a model holding it for example)
		  * that must then use the right methods
		  * \see hasEmitters
		  * \see hasParticles
		  */
		void				setDestroyCondition(TDieCondition dieCondition) 
		{ 
			_DieCondition = dieCondition;
			_PresetBehaviour = UserBehaviour;
		}

		/// get the destroy condition
		TDieCondition		getDestroyCondition(void) const { return _DieCondition; }

		/** Set a delay before to apply the death condition test
		  * This may be necessary : the system could be destroyed because there are no particles
		  * , but no particles were emitted yet
		  * This is an indication, and has no direct effect, and must be check by a third party (a model holding it for example)
		  * \see hasEmitters()
		  * \see hasParticles()
		  * \see getDelayBeforeDeathConditionTest()
		  */
		void setDelayBeforeDeathConditionTest(TAnimationTime delay) 
		{
			_DelayBeforeDieTest  = delay; 
		}

		/// get the a delay before to apply the death condition test	  
		TAnimationTime		getDelayBeforeDeathConditionTest(void) const { return _DelayBeforeDieTest; }

		/** tells the model holding this system that he become invalid when its out of the view frustum.	  
		  * This is only an indication flag and must be checked by third party (a model holding it for example)
		  * It has no direct effects
		  * \see doesDestroyWhenOutOfRange()
		  */
		void				destroyWhenOutOfFrustum(bool enable = true) 
		{ 
			_DestroyWhenOutOfFrustum = enable; 
			_PresetBehaviour = UserBehaviour;
		}

		/** check wether the system must be destroyed when it goes out of the frustum
		  * \see getDelayBeforeDeathConditionTest()
		  */
		bool				doesDestroyWhenOutOfFrustum(void) const { return _DestroyWhenOutOfFrustum; }


		/// return true when there are still emitters in the system
		bool				hasEmitters(void) const;

		/// return true when there are still particles
		bool				hasParticles(void) const;

		/// This enum tells when animation must be performed
		enum TAnimType 
		{  AnimVisible = 0,   /* visible systems only are animated */
		   AnimInCluster, /* systems that are in cluster are animated */
		   AnimAlways,    /* animate always when not too far */
		   LastValue
		};

		/** Deprecated. This set the animation type to AnimInCluster.
          * \see setAnimType(TAnimType animType)
		  */
		void				performMotionWhenOutOfFrustum(bool enable = true) 
		{ 
			_AnimType = enable ? AnimInCluster : AnimVisible;
			_PresetBehaviour = UserBehaviour;
		}

		/** Deprecated. Test if animType == AnimInCluster.
		  * * \see setAnimType(TAnimType animType)
		  */
		bool				doesPerformMotionWhenOutOfFrustum(void) const { return _AnimType == AnimInCluster; }

		/// Tells when animation must be done
		void				setAnimType(TAnimType animType)
		{
			nlassert(animType < LastValue);
			_AnimType = animType;
			_PresetBehaviour = UserBehaviour;
		}

		/// Test what the animation type is
		TAnimType			getAnimType() const { return _AnimType; }

		/** Because choosing the previous parameters can be difficult, this define
		  * presets hat can be used to tune the system easily.
		  * Any call to :
		  * - setDestroyModelWhenOutOfRange
		  * - setAnimType
		  * - setDestroyCondition
		  * - destroyWhenOutOfFrustum
		  * - performMotionWhenOutOfFrustum
		  *
		  * will set the behaviour to 'user'
		  */
		enum TPresetBehaviour
		{
			EnvironmentFX = 0,
			RunningEnvironmentFX, /* an environment fx that should 
								   * run when in parsed cluster : cascade for example,
								   * so that it doesn't start when the player first see
								   * it
								   */
			SpellFX,
			LoopingSpellFX,
			MinorFX,
			UserBehaviour,
			PresetLast
		};

		void activatePresetBehaviour(TPresetBehaviour behaviour);

		TPresetBehaviour getBehaviourType() const 
		{ 
			return _PresetBehaviour; 
		}




		// @}

	//*****************************************************************************************************
	///\name sound managment
		// @{
		/// register a Sound server to this system. All systems share the same sound server. 
		static void					registerSoundServer(UPSSoundServer *soundServer)
		{
			_SoundServer = soundServer;
		}

		/// get the current sound server used by this system. NULL if none
		static UPSSoundServer *		getSoundServer(void)
		{
			return _SoundServer;
		}

		/// immediatly shut down all the sound in this system
		void stopSound();

		/// reactivate the sound in the system
		void reactivateSound();

		// @}
	
	//*****************************************************************************************************
	///\name external access to locatedBindable. PRIVATE PART (to avoid the use of friend)
		// @{
			/** register a locatedBindable, and allow it to be referenced by the given ID
			  * this locatedBindable must belong to this system.
			  * each pair <id, locatedBindable> must be unique, but there may be sevral LB for the same key
			  */
			void registerLocatedBindableExternID(uint32 id, CPSLocatedBindable *lb);

			/// unregister the given located bindable. An assertion is raised if it has not been registered before
			void unregisterLocatedBindableExternID(CPSLocatedBindable *lb);
		// @}

	//*****************************************************************************************************
	///\name external access to locatedBindable. PUBLIC PART
		// @{
				/// return the number the number of located bindable bound with this ID
				uint			   getNumLocatedBindableByExternID(uint32 id) const;
				/** return the nth locatedBindable associtaed with this ID. 
				  * \return NULL if it doesn't exist
				  */
				CPSLocatedBindable *getLocatedBindableByExternID(uint32 id, uint index);
				const CPSLocatedBindable *getLocatedBindableByExternID(uint32 id, uint index) const;
				/// Get the number of IDs in the system
				uint   getNumID() const;
				/// Get the nth ID, or 0 if index is invalid.			  
				uint32 getID(uint index) const;
				/** Get all the IDs in the system. 
				  * \warning As IDs are not internally stored in a vector, it is faster than several calls to getID
				  */
				void getIDs(std::vector<uint32> &dest) const;
		// @}

	//*****************************************************************************************************
	///\name Misc. options
		// @{
			/** When using an emitter, it is allowed to have a period of '0'. This special value means that the emitter
			  * should emit at each frame. This is deprecated now (not framerate independent ..), but some previous systems may use it.
			  * This option force a minimum period for all emitters.
			  * NB : the system should be restarted for this to work correctly
			  * The default is true
			  */
				void enableEmitThreshold(bool enabled = true) { _EmitThreshold = enabled; }
				bool isEmitThresholdEnabled() const { return _EmitThreshold; }
		// @}

	

private:
	typedef std::map<std::string, float> TGlobalValuesMap;
	typedef std::map<std::string, NLMISC::CVector> TGlobalVectorValuesMap;
private:
	friend class CParticleSystemModel;	
	/// process a pass on the bound located
	void					stepLocated(TPSProcessPass pass, TAnimationTime et, TAnimationTime realEt);
	void					updateLODRatio();
	void					updateColor();
	// map that contain global value that can be affected to user param
	static TGlobalValuesMap  _GlobalValuesMap;
	// map that contain gloàbal vector values
	static TGlobalVectorValuesMap _GlobalVectorValuesMap;
	// A bbox that has been specified by the user
	NLMISC::CAABBox			 _PreComputedBBox;
	// the driver used for rendering
	IDriver					 *_Driver;	
		
	typedef std::vector< CParticleSystemProcess *> TProcessVect;
	TProcessVect			 _ProcessVect;
	CFontGenerator			 *_FontGenerator;
	CFontManager			 *_FontManager;
	// the view matrix
	NLMISC::CMatrix			 _ViewMat;

	// the inverted view matrix
	NLMISC::CMatrix			 _InvertedViewMat;

	// the matrix of the system
	NLMISC::CMatrix			 _SysMat; 
	// The previous matrix of the system. It is used to perform emission of particles in the world basis at the right position when the systm is moving
	NLMISC::CMatrix			 _OldSysMat; 
	// the inverted matrix of the system
	NLMISC::CMatrix			 _InvSysMat;
	// Current position of the system. It is interpolated during integration, and is used by emitter to emit at the correct position
	NLMISC::CVector          _CurrentDeltaPos;
	// Delta position of the system
	NLMISC::CVector          _DeltaPos;

	// number of rendered pass on the system, incremented each time the system is redrawn
	uint64					 _Date;	

	/// Last update date of the system. Useful with sharing only, to avoid several motions.
	sint64					 _LastUpdateDate;

	// current edited element located (edition purpose only)
	CPSLocated				 *_CurrEditedElementLocated;
	// current edited located bindable, NULL means all binadable of a located. (edition purpose only)
	CPSLocatedBindable		 *_CurrEditedElementLocatedBindable;
	// current edited element index in its located (edition purpose only)
	uint32					 _CurrEditedElementIndex;


	/** the scene in which the particle system is inserted. This is needed because
	 * the system may add objects to the scene (for particle that are meshs for instance)
	 */

	CScene					*_Scene;


	// contains the name of the system. (VERSION >= 2 only)
	std::string _Name;
	
	TAnimationTime								_TimeThreshold;
	TAnimationTime								_SystemDate;	
	uint32										_MaxNbIntegrations;	


	float										_LODRatio;
	float										_OneMinusCurrentLODRatio;
	float										_MaxViewDist;
	float										_MaxDistLODBias;
	float										_InvMaxViewDist;
	float										_InvCurrentViewDist; // inverse of the current view dist. It can be the same than _InvMaxViewDist
														        // but when there's LOD, the view distance may be reduced	
	float										_AutoLODEmitRatio;

	TDieCondition								_DieCondition;
	TAnimationTime								_DelayBeforeDieTest;	
	uint										_MaxNumFacesWanted;	
	TAnimType									_AnimType;

	static UPSSoundServer                      *_SoundServer;

	float										_UserParam[MaxPSUserParam];
	const TGlobalValuesMap::value_type		    **_UserParamGlobalValue; // usually set to NULL unless some user params mirror a global value, 
	                                                                     // in this case this contains as many pointer into the global map as there are user params
	uint8                                       _BypassGlobalUserParam;  // mask to bypass a global user param. This state is not serialized

	TPresetBehaviour							_PresetBehaviour;

	typedef 
	std::multimap<uint32, CPSLocatedBindable *> TLBMap;
	TLBMap										_LBMap;

	float										_AutoLODStartDistPercent;
	uint8										_AutoLODDegradationExponent;

	CPSAttribMaker<NLMISC::CRGBA>				*_ColorAttenuationScheme;
	NLMISC::CRGBA								_GlobalColor;

	/// \TODO nico replace this with a bitfield (and change serialisation accordingly)
	/// when set to true, the system will compute his BBox every time computeBBox is called
	bool										_ComputeBBox;
	bool										_BBoxTouched;
	bool										_AccurateIntegration;		
	bool										_CanSlowDown;
	bool										_DestroyModelWhenOutOfRange;
	bool										_DestroyWhenOutOfFrustum;
	bool										_Sharing;
	bool										_AutoLOD;
	bool										_KeepEllapsedTimeForLifeUpdate;
	bool										_AutoLODSkipParticles;
	bool										_EnableLoadBalancing;
	bool										_EmitThreshold;

	/// Inverse of the ellapsed time (call to step, valid only for motion pass)
	float										_InverseEllapsedTime;	
public:
	// For use by emitters only : This compute a delta of position to ensure that spaning position are correct when the system moves
	void		interpolatePosDelta(NLMISC::CVector &dest, TAnimationTime deltaT);
	// For use by emitters only : Get the current emit ratio when auto-LOD is used. Valid only during the 'Emit' pass
	float		getAutoLODEmitRatio() const { return _AutoLODEmitRatio; }	
};





// NOT USED FOR NOW
/**
 *	This class holds infos needed to duplicate a particle system
 *  Because of cross referencement, an object of the system may need referencment before it is created
 *  With map holding pointer to already created object, we can duplicate the system safely
 *  for now it is for PRIVATE USE... 
 *  may be useful in NLMISC later as it could be used with other kind of objects ...
 */ 
/*
class CPSCopyHelper
{
	public:
		// duplicate an object using the copy ctor, if it has not been before
		template <class T> T *ctorCopy(const T &src) 
		{
			TCopiedIt it = _Alreadycopied.find(src);
			if (it  != _AlreadyCopied.end())
			{
				return (T *) it;
			}
			else
			{
				T *result = new T(src);
				_AlreadyCopied.insert((void *) result);
				return result;
			}
		}
		// duplicate an object using its clone method, if it has not been before

		template <class T> T *clone(const T &src)
		{
			TCopiedIt it = _AlreadyCopied.find(src);
			if (it  != _AlreadyCopied.end())
			{
				return (T *) *it;
			}
			else
			{
				T *result = src.clone(this);
				_AlreadyCopied.insert((void *) result);
				return result;
			}
		}		


		// insert a value that has been copied by other means
		void insert(void *ptr)
		{
			std::pair<TCopiedIt, bool> result = _AlreadyCopied.insert(ptr);
			nlassert(result.second);
		}	

	private:
		typedef std::set<void *> TAlreadyCopied;
		typedef TAlreadyCopied::iterator TCopiedIt;
		TAlreadyCopied _AlreadyCopied;
};

*/


} // NL3D


#endif // NL_PARTICLE_SYSTEM_H

/* End of particle_system.h */
