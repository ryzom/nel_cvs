/** \file particle_system.h
 * <File description>
 *
 * $Id: particle_system.h,v 1.10 2001/07/24 08:44:36 vizerie Exp $
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
#include "nel/3d/animation_time.h"
#include "3d/mot.h"
#include "3d/animated_value.h"
#include "3d/particle_system_process.h"
#include "3d/ps_lod.h"


namespace NL3D {

class CParticleSystem ;
class CFontGenerator ;
class CFontManager ;
class CPSCopyHelper ;
class CScene ;
class CPSLocated ;
class IDriver ;





/// number user params for a particle system
const uint MaxPSUserParam = 4 ;


/** Particles system classes. They can be used as it. If you want to use a particle system in 
 *  a scene (M.O.T model), see particle_system_shape.h and particle_system_instance.h
 *  TODO : give example of use here...
 */




/**
 * This class holds a particle system
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CParticleSystem /*: public IModel, NLMISC::IEventEmitter*/
{

public:
	/// ctor
	CParticleSystem();

	/**
	* execute all the process of the system. It uses the driver that was set by a call to setDriver
	* \param ellapsedTime The ellapsed time since the last call
    * \param pass the pass to be executed
	* \see setDriver
	*/
	virtual void step(TPSProcessPass pass, CAnimationTime ellapsedTime) ;


	/// get the time ellapsed since the system was created
	CAnimationTime getSystemDate(void) const { return _SystemDate ; }


	/** Set the value of a user parameter. It must range from 0 to 1. The user value are not saved, and their default value is 0.f
	  * The max number of user param is in MaxPSUserParam
	  */
	void setUserParam(uint numParam, float value) 
	{
		nlassert(numParam < MaxPSUserParam) ;
		nlassert(value >= 0.f && value <= 1.f) ;
		_UserParam[numParam].Value = value ;
	}

	/** get a user param
	  * * The max number of user param is in MaxPSUserParam
	  */
	float getUserParam(uint numParam) const
	{
		nlassert(numParam < MaxPSUserParam) ;
		return _UserParam[numParam].Value ;
	}

	/// get a pointer to a param, as an animated value
	CAnimatedValueFloat *getUserParamAnimatedValue(uint numParam)
	{
		nlassert(numParam < MaxPSUserParam) ;
		return &_UserParam[numParam] ;
	}

	
	  


	/// set the driver use to render the system
	void setDriver(IDriver *driver) { _Driver = driver ; }

	/// return the driver that will be used for rendering
	IDriver *getDriver(void) { return _Driver ; }

	/// serialize this particle system
	void serial(NLMISC::IStream &f)  throw(NLMISC::EStream) ;

	

	/*** duplication method NOT SUPPORTED
	 * \param ch for private use, set to null by default
	 */
	//	CParticleSystem *clone(CPSCopyHelper *ch = NULL)  ;

	/// dtor
	~CParticleSystem() ;

	/** attach a process (such as a located : see particle_system_located.h) to the system
	 *  It is then owned by the process and will be deleted by it
     *  if already present -> nl assert	 
	 */
	void attach(CParticleSystemProcess *ptr) ;

	/** remove a process
	 * It is deleted by the system
     *  if not present -> nl assert
	 */
	void remove(CParticleSystemProcess *ptr) ;

	/// get the number of process that are attached to the system

	uint32 getNbProcess(void) const { return _ProcessVect.size() ; }

	/**
	 *  Get a ref to the nth process.
	 *  Out of range -> nlassert
	 */	
	CParticleSystemProcess *getProcess(uint32 index)
	{ 
		nlassert(index < _ProcessVect.size()) ;
		return _ProcessVect[index] ; 
	}

	/**
	 *  Get a const ref to the nth process.
	 *  Out of range -> nlassert
	 */	
	const CParticleSystemProcess *getProcess(uint32 index) const
	{ 
		nlassert(index < _ProcessVect.size()) ;
		return _ProcessVect[index] ; 
	}

	/** Compute the aabbox of this located, (expressed in world basis
	*  TODO : express it in system basis ...	
	*  \param aabbox a ref to the result box
	*/

	void computeBBox(NLMISC::CAABBox &aabbox) ;


	/** Set the matrix for the system. This only affect elements that are in the same basis
	 * you don't need to call this if it is used in a CPaticleSystemModel : Call ITransformable method. In this case
	 * , setSysMat will be called automatically when needed to mirror the ITransformable matrix
	 */
	 
	void setSysMat(const NLMISC::CMatrix &m) ;
	
	/// return the matrix of the system
	const NLMISC::CMatrix &getSysMat(void) const { return _SysMat ; }

	/// return the inverted matrix of the system

	const NLMISC::CMatrix &getInvertedSysMat(void) const { return _InvSysMat ; } 

	/** set the view matrix  
	  * This must be called otherwise results can't be correct
	  */
	void setViewMat(const NLMISC::CMatrix &m) ;

	/// get the view matrix .
	const NLMISC::CMatrix &getViewMat(void) const { return _ViewMat ; }

	/// get the inverted view matrix . It is stored each time a new frame is processed	 
	const NLMISC::CMatrix &getInvertedViewMat(void) const { return _InvertedViewMat ; }

	/// Set a font generator. Useful only for edition. don't need that in runtime
	void setFontGenerator(CFontGenerator *fg) { _FontGenerator = fg ; }

	/// retrieve the font generator. Edition purpose only
	CFontGenerator *getFontGenerator(void) { return _FontGenerator ; }

	/// retrieve the font generator (const version). Edition purpose only
	const CFontGenerator *getFontGenerator(void) const { return _FontGenerator ; }

	/// Set a font Manager. Useful only for edition. don't need that in runtime
	void setFontManager(CFontManager *fg) { _FontManager = fg ; }

	/// retrieve the font Manager. Edition purpose only
	CFontManager *getFontManager(void) { return _FontManager ; }

	/// retrieve the font Manager (const version). Edition purpose only
	const CFontManager *getFontManager(void) const { return _FontManager ; }


	/// used for benchs
	static uint32 _NbParticlesDrawn ;


	/** get the date of the system (the number of time it has been drawn in fact)
	 *  This may be used to skip frames in an animation for example
	 */

	uint64 getDate(void) const
	{
		return _Date ;
	}

	/** Set the scene in which the particle system is inserted. This is needed when
	 * system must add objects to the scene (for particle that are mesh for instance)
	 */
	void setScene(CScene *scene) { _Scene = scene ; }

	//// get the scene set by setScene()
	CScene *getScene(void) { return _Scene ; }


	/** For edition purposes only : this allow to highlight in red the current element being edited
	 *  \param located The located the current element belongs to, or NULL if no element is selected
	 *  \index the index of the element in the located
	 *  \lb the located bindable that is selected into a located (NULL = all)
	 */
	 void setCurrentEditedElement(CPSLocated *loc = NULL , uint32 index = 0, class CPSLocatedBindable *bd = NULL )
	 {
		_CurrEditedElementLocated = loc ;
		_CurrEditedElementLocatedBindable = bd ; 
		_CurrEditedElementIndex = index ;
	 }

	/** retrieve the current edited element
	 *  \see setCurrentEditedElement()
	 */
	 void getCurrentEditedElement(CPSLocated *&loc , uint32 &index, CPSLocatedBindable *&lb)
	 {
		loc = _CurrEditedElementLocated ;
		index = _CurrEditedElementIndex ;
		lb = _CurrEditedElementLocatedBindable ;
	 }
     		
	/// set the name of the system
	void setName(const std::string &s) { _Name = s ; }

	/// get the name of the system
	std::string getName(void) const { return _Name ; }
		

	/// return true if the system has opaque object in it
	bool hasOpaqueObjects(void) const ;

	/// return true if the system has transparent objects in it
	bool hasTransparentObjects(void) const ;

	/** This enable for more accurate integrations of movement. When this is activated,
	  *  integration is performed in a more accurate way when the ellapsed time goes over a threshold, but it is more slow to perform	  
	  */
	void enableAccurateIntegration(bool enable = true) { _AccurateIntegration = enable ; }
	bool isAccurateIntegrationEnabled(void) const { return _AccurateIntegration ; }

	/** the the time threshold and the max number of integration to perform, when accurate integration is activated
	  * The default is 0.10 for time threshold and 4 for max NbIntegrations
	  * \param canSlowDown : Allow the system to slow down in speed but to keep accuracy in its movement.
	  *  It is useful for critical situations where the framerate is very low. The default is true.
	  */
	void setAccurateIntegrationParams(CAnimationTime threshold, uint32 maxNbIntegrations, bool canSlowDown)
	{
		_TimeThreshold = threshold ;
		_MaxNbIntegrations = maxNbIntegrations ;
		_CanSlowDown = canSlowDown ;
	}

	/// get the parameters used for integration
	void getAccurateIntegrationParams(CAnimationTime &threshold, uint32 &maxNbIntegrations, bool &canSlowDown)
	{
		threshold = _TimeThreshold  ;
		maxNbIntegrations = _MaxNbIntegrations ;
		canSlowDown = _CanSlowDown ;
	}


	/// set the max view distance for the system (in meters) . The default is 50 meter
	void setMaxViewDist(float maxDist) 
	{ 
		nlassert(maxDist > 0.f) ; 
		_MaxViewDist = maxDist ;
		_InvCurrentViewDist = _InvMaxViewDist = 1.f / maxDist ; 
	}

	/// get the max view distance
	float getMaxViewDist(void) const { return _MaxViewDist ; }

	/// set a percentage that indicate where the 2nd LOD is located. Default is 0.5
	void setLODRatio(float ratio) { nlassert(ratio > 0 && ratio <= 1.f) ; _LODRatio =  ratio ; }

	/// get the lod ratio
	float getLODRatio(void) const  { return _LODRatio ; }


	/** compute a vector and a distance that are used for LOD computations. 
	  * You'll have for a given pos : pos * v + offset  = 0 atthe nearest point, and 1 when
	  * pos is at maxDist from the viewer
	  */
	void getLODVect(NLMISC::CVector &v, float &offset, bool systemBasis) ;


	/// get the current LOD of the system. It is based on the distance of the center of the system to the viewer
	TPSLod getLOD(void) const ;


	/** When this is set to false, the system will recompute his bbox each time it is querried
	  * This may be needed for systems that move fast. 
	  */

	void setAutoComputeBBox(bool enable = true) { _ComputeBBox = enable ; }


	/// test whether the system compute himself his bbox
	bool getAutoComputeBBox(void) const { return _ComputeBBox ; }


	/** set a precomputed bbox (expressed in the system basis). This is allowed only when setAutoComputeBBox 
	  * is called with false (nlassert otherwise).
	  */

	void setPrecomputedBBox(const NLMISC::CAABBox &precompBBox) 
	{ 
		nlassert(!_ComputeBBox) ;
		_PreComputedBBox = precompBBox ;
	}
		
	/// get the last computed bbox
	void getLastComputedBBox(NLMISC::CAABBox &dest) { dest = _PreComputedBBox ; }
	

	/** tell the system to delete himself when its out of range (in fact, the Model holding it will destroy it when this
	  * flag is set)  The default is false.
	  */
	void setDestroyModelWhenOutOfRange(bool enable = true) { _DestroyModelWhenOutOfRange  = enable ; }
	
	/// check whether the system must be destroyed when it's out of range.
	bool getDestroyModelWhenOutOfRange(void) const { return _DestroyModelWhenOutOfRange ; }


	/// this enum give consitions on which the system may be destroyed
	enum TDieCondition { none, noMoreParticles, noMoreParticlesAndEmitters   } ;


	/// when != to none, the Model hodling this sytem will be destroyed when dieCondition is met
	void setDestroyCondition(TDieCondition dieCondition) { _DieCondition = dieCondition ; }

	/// get the destroy condition
	TDieCondition getDestroyCondition(void) const { return _DieCondition ; }

	/** Set a delay before to apply the death condition test
	  * This may be necessary : the system could be destroyed because there are no particles
	  * , but no particles were emitted yet
	  */
	void setDelayBeforeDeathConditionTest(CAnimationTime delay) { _DelayBeforeDieTest  = delay ; }

	/// get the a delay before to apply the death condition test	  
	CAnimationTime getDelayBeforeDeathConditionTest(void) const { return _DelayBeforeDieTest ; }

	/** tells the model holding this system that he become invalid when its out of the view frustrum.
	  * The system will be destroy, and then recreated when its back in frustrum, unless
	  *  setDestroyWhenOutOfRange() has been called with true, in which case the model
	  *  holding the system is destroyed too.
	  * (otherwise, distance only is taken in account, which is the default)	  	  
	  * \see setDestroyWhenOutOfRange()
	  */
	void destroyWhenOutOfFrustrum(bool enable = true) { _DestroyWhenOutOfFrustrum = enable ; }

	/// check wether the system must be destroyed when it goes out of the frustrum
	bool doesDestroyWhenOutOfFrustrum(void) const { return _DestroyWhenOutOfFrustrum ; }


	/// return true when there are still emitters in the system
	bool hasEmitters(void) const ;

	/// return true when there are still partciels
	bool hasParticles(void) const ;

	/// get 1.f - the current lod ratio (it is updated at each motion pass)
	float getOneMinusCurrentLODRatio(void) const { return _OneMinusCurrentLODRatio ; }

	// get an evaluation of how many tris are needed with the system for the given distance
	float getWantedNumTris(float dist) ;

	/// set the number of tree the system may use. If not clled this will be the max
	void setNumTris(uint numFaces) ;


	/// for the particle system to reevaluate the max number of faces it may need
	void notifyMaxNumFacesChanged(void) ;

protected:

	/// when set to true, the system will compute his BBox every time computeBBox is called
	bool _ComputeBBox ;
	NLMISC::CAABBox _PreComputedBBox ;

	// the driver used for rendering
	IDriver *_Driver ;

	/// user params of the system
	CAnimatedValueFloat _UserParam[MaxPSUserParam] ;
		
	typedef std::vector< CParticleSystemProcess *> TProcessVect ;
	TProcessVect _ProcessVect ;
	CFontGenerator *_FontGenerator ;
	CFontManager *_FontManager ;
	// the view matrix
	NLMISC::CMatrix _ViewMat ;

	// the inverted view matrix
	NLMISC::CMatrix _InvertedViewMat ;

	// the matrix of the system
	NLMISC::CMatrix _SysMat ; 
	// the inverted matrix of the system
	NLMISC::CMatrix _InvSysMat ;

	// number of rendered pass on the system, incremented each time the system is redrawn
	uint64 _Date ;	

	// current edited element located (edition purpose only)
	CPSLocated *_CurrEditedElementLocated ;
	// current edited located bindable, NULL means all binadable of a located. (edition purpose only)
	CPSLocatedBindable *_CurrEditedElementLocatedBindable ;
	// current edited element index in its located (edition purpose only)
	uint32 _CurrEditedElementIndex ;


	/** the scene in which the particle system is inserted. This is needed because
	 * the system may add objects to the scene (for particle that are mesh for instance)
	 */

	CScene *_Scene ;


	// contains the name of the system. (VERSION >= 2 only)
	std::string _Name ;


	bool _AccurateIntegration ;	
	CAnimationTime _TimeThreshold ;
	CAnimationTime _SystemDate ;
	uint32 _MaxNbIntegrations ;
	bool _CanSlowDown ;

	float _LODRatio, _OneMinusCurrentLODRatio ;
	float _MaxViewDist, _InvMaxViewDist ;
	float _InvCurrentViewDist ; // inverse of the current view dist. It can be the same than _InvMaxViewDist
								// but when there's LOD, the view distance may be reduced
	bool _Touch ;

	TDieCondition  _DieCondition ;
	CAnimationTime _DelayBeforeDieTest ;
	bool		   _DestroyModelWhenOutOfRange ;
	bool		   _DestroyWhenOutOfFrustrum ;
	
	uint _MaxNumFacesWanted ;	
	
	
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
			TCopiedIt it = _Alreadycopied.find(src) ;
			if (it  != _AlreadyCopied.end())
			{
				return (T *) it ;
			}
			else
			{
				T *result = new T(src) ;
				_AlreadyCopied.insert((void *) result) ;
				return result ;
			}
		}
		// duplicate an object using its clone method, if it has not been before

		template <class T> T *clone(const T &src)
		{
			TCopiedIt it = _AlreadyCopied.find(src) ;
			if (it  != _AlreadyCopied.end())
			{
				return (T *) *it ;
			}
			else
			{
				T *result = src.clone(this) ;
				_AlreadyCopied.insert((void *) result) ;
				return result ;
			}
		}		


		// insert a value that has been copied by other means
		void insert(void *ptr)
		{
			std::pair<TCopiedIt, bool> result = _AlreadyCopied.insert(ptr) ;
			nlassert(result.second) ;
		}	

	private:
		typedef std::set<void *> TAlreadyCopied ;
		typedef TAlreadyCopied::iterator TCopiedIt ;
		TAlreadyCopied _AlreadyCopied ;
} ;

*/


} // NL3D


#endif // NL_PARTICLE_SYSTEM_H

/* End of particle_system.h */
