/** \file particle_system_shape.h
 * <File description>
 *
 * $Id: particle_system_shape.h,v 1.16 2002/10/10 13:30:59 vizerie Exp $
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
#include "nel/misc/smart_ptr.h"
#include "3d/shape.h"
#include "3d/track.h"



namespace NL3D {

///////////////////////////////////////////////////////////////////////////////
// THE FOLLOWING CLASS ARE FOR INSERTION OF A PARTICLE SYSTEM IN A MOT MODEL //
///////////////////////////////////////////////////////////////////////////////

// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		ParticleSystemModelId=NLMISC::CClassId(0x3a9b1dc3, 0x49627ff0);


class CParticleSystem;
class CParticlesystemModel;
class CParticleSystemDetailObs;



/** This class helps to instanciate a particle system 
 * (the shape contains a particle system prototype stored as a memory stream)
 *  Use the createInstance method to insert the system in a scene 
 *  To load the shape from a file, use a shape stream
 */
class CParticleSystemShape : public IShape
{
public:

	/// Default ctor
	CParticleSystemShape();


	/** build the shape from a 'raw' particle system. A prototype will be created by copying the system in a memory stream
	 *  NOTE : For now, prefer the instanciation from a file, which do not need reallocation
	 */
	void buildFromPS(const NL3D::CParticleSystem &ps);

	/// Dtor.
	virtual ~CParticleSystemShape() {}

	/** create a particle system instance
	 * \param scene the scene used to createModel().
	 * \return the specialized instance for this shape.
	 */
	virtual	CTransformShape		*createInstance(NL3D::CScene &scene);

	/// \name Inherited from IShape.
	// @{
	/** render() a particle system in a driver, with the specified TransformShape information.
	 * CTransfromShape call this method in the render traversal.
	 */
	virtual void				render(NL3D::IDriver *drv, CTransformShape *trans, bool passOpaque);
	// @}

	/// serial the shape
	virtual void	serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	NLMISC_DECLARE_CLASS(CParticleSystemShape);


	/// get a the user param default tracks
	CTrackDefaultFloat *getUserParamDefaultTrack(uint numTrack)
	{
		nlassert(numTrack < 4);
		return &_UserParamDefaultTrack[numTrack];
	}


	/// get a the trigger default track
	CTrackDefaultBool *getDefaultTriggerTrack(void)
	{
	
		return &_DefaultTriggerTrack;
	}


	/// Always return a unit bounding box, unless the system has a precomputed bbox.
	virtual	void	getAABBox(NLMISC::CAABBox &bbox) const;


	/** this method is meaningless here : the load balancing observer for particle system
	  * compute the number of triangles from the Model, not the shape
	  */
	virtual float				getNumTriangles (float distance) { return 0; }


	/// \name access default tracks.
	// @{
		CTrackDefaultVector*	getDefaultPos ()		{return &_DefaultPos;}
		CTrackDefaultVector*	getDefaultScale ()		{return &_DefaultScale;}
		CTrackDefaultQuat*		getDefaultRotQuat ()	{return &_DefaultRotQuat;}
	// @}

	// Test if the system is shared
	bool			isShared() const { return _Sharing; }		
protected:

	friend class CParticleSystemModel; 
	friend class CParticleSystemDetailObs;
	friend class CParticleSystemClipObs;
	friend class CParticleSystem;

public:
	/** Instanciate a particle system from this shape.
	  * A particle system may need to call this when a system is back in the frustum
	  */
	CParticleSystem *instanciatePS(CScene &scene);
protected:
	/// inherited from ishape
	virtual void				flushTextures (IDriver &driver) {}		

protected:

	/** A memory stream containing a particle system. Each system is instanciated from this prototype
	  * Nevertheless, we store some more system infos which are needed for its lifecycle mgt.
	  */
	NLMISC::CMemStream  _ParticleSystemProto;
	float				_MaxViewDist;							// the max view distance of the system, mirror the PS value	
	NLMISC::CAABBox     _PrecomputedBBox;						// mirror the ps value
	
	/// the default track for animation of user parameters
	CTrackDefaultFloat _UserParamDefaultTrack[4];

	/// Transform default tracks.
	CTrackDefaultVector			_DefaultPos;
	CTrackDefaultVector			_DefaultScale;	
	CTrackDefaultQuat			_DefaultRotQuat;

	/// Trigger default track
	CTrackDefaultBool			_DefaultTriggerTrack;	

	/// For sharing, this tells us if there's a system already instanciated that we could use for sharing
	NLMISC::CRefPtr<CParticleSystem> _SharedSystem; 

	bool                _DestroyWhenOutOfFrustum;				// mirror the ps value	
	bool				_DestroyModelWhenOutOfRange;			// mirror the ps value
	bool				_UsePrecomputedBBox;					// mirror the ps value
	bool				_Sharing;								// mirror the ps value
};

} // NL3D


#endif // NL_PARTICLE_SYSTEM_SHAPE_H

/* End of particle_system_shape.h */
