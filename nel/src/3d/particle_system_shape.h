/** \file particle_system_shape.h
 * <File description>
 *
 * $Id: particle_system_shape.h,v 1.1 2001/06/15 16:24:43 corvazier Exp $
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

#ifndef NL_PARTICLE_SYSTEM_SHAPE_H
#define NL_PARTICLE_SYSTEM_SHAPE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/class_id.h"
#include "nel/misc/mem_stream.h"
#include "3d/shape.h"



namespace NL3D {

///////////////////////////////////////////////////////////////////////////////
// THE FOLLOWING CLASS ARE FOR INSERTION OF A PARTICLE SYSTEM IN A MOT MODEL //
///////////////////////////////////////////////////////////////////////////////

// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		ParticleSystemModelId=NLMISC::CClassId(0x3a9b1dc3, 0x49627ff0) ;


// ***************************************************************************


/** This class helps to instanciate a particle system 
 * (the shape contains a particle system prototype stored as a memory stream)
 *  Use the createInstance method to insert the system in a scene 
 */
class CParticleSystemShape : public IShape
{
public:

/// Default ctor
CParticleSystemShape() ;

/// build the shape from a file (2^32 max size)
void buildFromFile(const std::string &fileName) throw(NLMISC::EStream) ;

/** build the shape from a 'raw' particle system. A prototype will be created by copying the system in a memory stream
 *  NOTE : For now, prefer the instanciation from a file, which do not need reallocation
 */
void buildFromPS(const CParticleSystem &ps) ;

/// Dtor.
virtual ~CParticleSystemShape() {}

/** create a particle system instance
 * \param scene the scene used to createModel().
 * \return the specialized instance for this shape.
 */
virtual	CTransformShape		*createInstance(CScene &scene) ;



/** clip this system with a pyramid.
 * the pyramid is given in object space so the shape do not need to know the matrix of the object.
 * \param pyramid the clipping polytope, planes MUST be normalized.
 * \return true if the object is visible, false otherwise. The default behavior is to return true (never clipped).
 */
virtual bool				clip(const std::vector<CPlane>	&pyramid) ;

/** render() a particle system in a driver, with the specified TransformShape information.
 * CTransfromShape call this method in the render traversal.
 */
virtual void	render(IDriver *drv, CTransformShape *trans) ;

	/// serial the shape
	virtual void	serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	NLMISC_DECLARE_CLASS(CParticleSystemShape) ; 
protected:

	
	// A memory stream containing a particle system. Each system is instanciated from this prototype
	NLMISC::CMemStream  _ParticleSystemProto ; 
} ;

} // NL3D


#endif // NL_PARTICLE_SYSTEM_SHAPE_H

/* End of particle_system_shape.h */
