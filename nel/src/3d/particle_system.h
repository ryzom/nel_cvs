/** \file particle_system.h
 * <File description>
 *
 * $Id: particle_system.h,v 1.1 2001/06/15 16:24:43 corvazier Exp $
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
#include "3d/mot.h"
#include "nel/misc/stream.h"
#include "nel/3d/animation_time.h" 
#include "3d/particle_system.h"
#include "nel/misc/matrix.h"
#include "nel/misc/mem_stream.h"



namespace NLMISC
{
	class CAABBox ;
}

namespace NL3D {
using NLMISC::CMatrix ;
class CParticleSystem ;
class CFontGenerator ;
class CFontManager ;
class CPSCopyHelper ;
class CScene ;
class CPSLocated ;


/** Particles system classes. They can be used as it. If you want to use a particle system in 
 *  a scene (M.O.T model), see particle_system_shape.h and particle_system_instance.h
 *  TODO : give example of use here...
 */





/** rendering and process passes for a particle system.
 *  PSCollision : collisions with collision zones (see CPSZone)
 *  PSMotion    : motion computation
 *  PSSolidRender : render particle that can modify z-buffer
 *  PSBlendRender : render transparency (no z-buffer write)
 *  PSToolRender  : for edition purpose, show representations for forces, emitters...
 */
enum TPSProcessPass { PSCollision, PSMotion, PSSolidRender, PSBlendRender, PSToolRender } ;


/**
 *	A system particle process; A process is anything that can be called at each update of the system
 */

class CParticleSystemProcess : public NLMISC::IStreamable
{
	public:
		/**
		* execute this process, telling how much time ellapsed 
		*/
		virtual void step(TPSProcessPass pass, CAnimationTime ellapsedTime) = 0 ;

		/// ctor
		CParticleSystemProcess() : _Owner(NULL), _SystemBasisEnabled(false) {}
		
		/// dtor
		virtual ~CParticleSystemProcess()  {}




		/** Compute the aabbox of this located, (expressed in world basis
		*  \return true if there is any aabbox
		*  \param aabbox a ref to the result box
		*/

		virtual bool computeBBox(NLMISC::CAABBox &aabbox) const = 0 ;

		/// set the process owner. Called by the particle system during attachment
		void setOwner(CParticleSystem *ps) { _Owner = ps ; }

		/// retrieve the particle system that owns this process
		CParticleSystem *getOwner(void) { return _Owner ; }

		/// retrieve the particle system that owns this process (const version)
		const CParticleSystem *getOwner(void) const { return _Owner ; }

		/// Shortcut to get a font generator if one was set (edition mode)
		CFontGenerator *getFontGenerator(void) ;

		/// Shortcut to get a font generator if one was set, const version  (edition mode)
		const CFontGenerator *getFontGenerator(void) const ;

		/// Shortcut to get a font Manager if one was set (edition mode)
		CFontManager *getFontManager(void) ;

		/// Shortcut to get a font Manager if one was set, const version  (edition mode)
		const CFontManager *getFontManager(void) const ;

		/**	
		* return true if the process is in the particle system basis, false if it's in the world basis
		*/
		bool isInSystemBasis(void) const 
		{ 
			return _SystemBasisEnabled ; 
		}

		/** Choose the basis for this process. Warning : This won't change any existing coordinate
		 *  By default, all process are expressed in the world basis
		 *  \param sysBasis truer if particles are in the system basis
		 */

		void setSystemBasis(bool sysBasis = true) { _SystemBasisEnabled = sysBasis ; }

		/** serialize the whole system
		* Everything is saved, except for the fontManager and the fontGenerator
		* They must be set again if the edition pass, that show forces and zone, is used
		*/
		virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;
		

	
		
		
	protected:
		CParticleSystem *_Owner ;

		// true if the system basis is used for display and motion
		bool _SystemBasisEnabled ;

} ;


/**
 * This class holds a particle system
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CParticleSystem /*: public IModel, NLMISC::IEventEmitter*/
{

public:
	/**
	* execute all the process of the system
	* \param ellapsedTime The ellapsed time since the last call
    * \param pass the pass to be executed
	*/
	virtual void step(TPSProcessPass pass, CAnimationTime ellapsedTime) ;

	/// serialize this particle system
	void serial(NLMISC::IStream &f)  throw(NLMISC::EStream) ;

	/// ctor
	CParticleSystem();


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
	*  \return true if there is any aabbox
	*  \param aabbox a ref to the result box
	*/

	bool computeBBox(NLMISC::CAABBox &aabbox) const ;


	/** Set the matrix for the system. This only affect elements that are in the same basis
	 * you don't need to call this if it is used in a CPaticleSystemModel : Call ITransformable method. In this case
	 * , setSysMat will be called automatically when needed to mirror the ITransformable matrix
	 */
	 
	void setSysMat(const CMatrix &m) ;
	
	/// return the matrix of the system
	const CMatrix &getSysMat(void) const { return _SysMat ; }

	/// return the inverted matrix of the system

	const CMatrix &getInvertedSysMat(void) const { return _InvSysMat ; } 



	/// get the view matrix . It is stored each time a new frame is processed	 
	const CMatrix &getViewMat(void) const { return _ViewMat ; }

	/// get the inverted view matrix . It is stored each time a new frame is processed	 
	const CMatrix &getInvertedViewMat(void) const { return _InvertedViewMat ; }

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
	 */
	 void setCurrentEditedElement(CPSLocated *loc = NULL , uint32 index = 0)
	 {
		_CurrEditedElementLocated = loc ;
		_CurrEditedElementIndex = index ;
	 }

	/** retrieve the current edited element
	 *  \see setCurrentEditedElement()
	 */
	 void getCurrentEditedElement(CPSLocated *&loc , uint32 &index)
	 {
		loc = _CurrEditedElementLocated ;
		index = _CurrEditedElementIndex ;
	 }
     		
		
		

protected:
		
	typedef std::vector< CParticleSystemProcess *> TProcessVect ;
	TProcessVect _ProcessVect ;
	CFontGenerator *_FontGenerator ;
	CFontManager *_FontManager ;
	// the view matrix
	CMatrix _ViewMat ;

	// the inverted view matrix
	CMatrix _InvertedViewMat ;

	// the matrix of the system
	CMatrix _SysMat ; 
	// the inverted matrix of the system
	CMatrix _InvSysMat ;

	// number of rendered pass on the system, incremented each time the system is redrawn
	uint64 _Date ;

	// current edited element located (edition purpose only)
	CPSLocated *_CurrEditedElementLocated ;
	// current edited element index in its located (edition purpose only)
	uint32 _CurrEditedElementIndex ;


	/** the scene in which the particle system is inserted. This is needed because
	 * the system may add objects to the scene (for particle that are mesh for instance)
	 */

	CScene *_Scene ;

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
