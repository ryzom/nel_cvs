/** \file particle_system.h
 * <File description>
 *
 * $Id: particle_system.h,v 1.1 2001/04/25 08:38:06 vizerie Exp $
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
#include "nel/3d/mot.h"
#include "nel/misc/stream.h"
#include "nel/3d/tmp/animation_time.h" 
#include "nel/3d/particle_system.h"
#include "nel/misc/matrix.h"


namespace NLMISC
{
	class CAABBox ;
}

namespace NL3D {


using NLMISC::CSmartPtr ;
using NLMISC::CMatrix ;
class CParticleSystem ;
class CFontGenerator ;
class CFontManager ;



// rendering and process passes for a particle system
enum TPSProcessPass { PSCollision, PSMotion, PSSolidRender, PSBlendRender, PSToolRender } ;





/**
 *	A system particle process; A process is anything that can be called at each update 
 */

class CParticleSystemProcess : public NLMISC::IStreamable, public NLMISC::CRefCount
{
	public:
		/**
		* execute this process, telling how much time ellapsed 
		*/
		virtual void step(TPSProcessPass pass, CAnimationTime ellapsedTime) = 0 ;

		/// ctor
		CParticleSystemProcess() : _Owner(NULL), _SystemBasisEnabled(false) {}

		/// dtor
		virtual ~CParticleSystemProcess()  {} ;

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
		bool isInSystemBasis(void) const { return _SystemBasisEnabled ; }

		/** Choose the basis for this process. Warning : This won't change any existing coordinate
		 *  By default, all process are expressed in the world basis
		 *  \param sysBasis truer if particles are in the system basis
		 */

		void setSystemBasis(bool sysBasis = true) { _SystemBasisEnabled = sysBasis ; }

	

		
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
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;

	/// ctor
	CParticleSystem();

	/** attach a process (such as a located : see particle_system_located.h) to the system
     *  if already present -> nl assert
	 */
	void attach(CSmartPtr<CParticleSystemProcess> ptr) ;

	/** detach a process
     *  if not present -> nl assert
	 */

	void detach(const CSmartPtr<CParticleSystemProcess> &ptr) ;

	/// get the number of process that are attached to the system

	uint32 getNbProcess(void) const { return _ProcessVect.size() ; }

	/**
	 *  Get a ref to the nth process.
	 *  Out of range -> nlassert
	 */	
	CSmartPtr<CParticleSystemProcess> &getProcess(uint32 index)
	{ 
		nlassert(index < _ProcessVect.size()) ;
		return _ProcessVect[index] ; 
	}

	/**
	 *  Get a const ref to the nth process.
	 *  Out of range -> nlassert
	 */	
	const CSmartPtr<CParticleSystemProcess> &getProcess(uint32 index) const
	{ 
		nlassert(index < _ProcessVect.size()) ;
		return _ProcessVect[index] ; 
	}

	/** Compute the aabbox of this located, (expressed in world basis
	*  \return true if there is any aabbox
	*  \param aabbox a ref to the result box
	*/

	bool computeBBox(NLMISC::CAABBox &aabbox) const ;


	/// Set the matrix for the system. This only affect elements that are in the same basis
	void setSysMat(const CMatrix &m) ;
	
	/// return the matrix of the system
	const CMatrix &getSysMat(void) const { return _SysMat ; }

	/// return the inverted matrix of the system

	const CMatrix &getInvertedSysMat(void) const { return _InvSysMat ; } 



	/// shortcut to get the view matrix (transposed version) . It is stored each time a new frame is processed	 
	 
	const CMatrix &getViewMat(void) const { return _ViewMat ; }

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

protected:
	
	typedef std::vector<CSmartPtr<CParticleSystemProcess> > TProcessVect ;
	TProcessVect _ProcessVect ;
	CFontGenerator *_FontGenerator ;
	CFontManager *_FontManager ;
	// the transposed view matrix
	CMatrix _ViewMat ;
	// the matrix of the system
	CMatrix _SysMat ; 
	// the inverted matrix of the system
	CMatrix _InvSysMat ;
};






/** this class helps to instanciate a particle system from its shape
 *
 *
 */

/*class CParticleSystemShape : public IShape
{
public:

/// Constructor
IShape() {}
/// Dtor.
virtual ~IShape() {}

/** create a particle system instance
 * \param scene the scene used to createModel().
 * \return the specialized instance for this shape.
 */
/*virtual	CTransformShape		*createInstance(CScene &scene);
*/


/** render() a particle system in a driver, with the specified TransformShape information.
 * CTransfromShape call this method in the render traversal.
 */
/*virtual void				render(IDriver *drv, CTransformShape *trans)=0;

protected:
	// all particles system instances instances are duplicated from this one
	CParticleSystem _ParticleSystem ; 
} ;


*/


/** a particle system transform shape : it is build using a CParticleSystemShape
 *
 */
/*
class CParticleSystemTransformShape : public CTransformShape
{
public:
*/
	/**
	* build this transform shape by copying a particle system
	*/
/*	CParticleSystemTransformShape(const CParticleSystem ps) : _ParticleSystem(ps) {}
protected:
	CParticleSystem _ParticleSystem ;
} ;
*/


} // NL3D


#endif // NL_PARTICLE_SYSTEM_H

/* End of particle_system.h */
