/** \file bounding_sphere.h
 * CBoundingSphere: Bounding sphere (different from the NL3D ones)
 *
 * $Id: bounding_sphere.h,v 1.5 2001/09/04 16:53:29 cado Exp $
 */

/* Copyright, 2000, 2001 Nevrax Ltd.
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

#ifndef NL_BOUNDING_SPHERE_H
#define NL_BOUNDING_SPHERE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/bsphere.h"
#include "bounding_shape.h"


namespace NLSOUND {


/**
 * Bounding sphere (different from the NL3D ones)
 * 
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CBoundingSphere : public IBoundingShape
{
public:

	/// Static init (call at the very beginning)
	static void						init() 
	{
		NLMISC_REGISTER_CLASS(CBoundingSphere);
	}
 

	/// Return true if the box includes the specified point
	virtual bool					include( const NLMISC::CVector& pos ) const;
	/// Return the approximate diameter
	virtual float					getDiameter() const						{ return _BSphere.Radius*2.0f; }
	/// Return the radius at the intersection of the shape and the line between the specified position and the center of the shape
	virtual float					getRadiusAtIntersect( const NLMISC::CVector& ) const { return _BSphere.Radius; }
	/// Return a reference to the center of the shape
	virtual const NLMISC::CVector&	getCenter() const						{ return _BSphere.Center; }
	/// Move the shape
	virtual void					setCenter( const NLMISC::CVector& pos )	{ _BSphere.Center = pos; }
	/** Return the ratio of a point between this shape and another inner shape.
	 * The point must be included in this shape (the outer one) and not
	 * in the inner shape.
	 * If the point is near this shape, the ratio is near 0.
	 * If the point is near the inner shape, the ratio is near 1.
	 */
	virtual float					getRatio( const NLMISC::CVector& pos, IBoundingShape *inner ) const;
	/// Serialize (if you change this, increment the version number in CEnvSoundUser::load()!)
	virtual void					serial( NLMISC::IStream& s )			{ s.serial( _BSphere.Center ); s.serial( _BSphere.Radius ); }
	NLMISC_DECLARE_CLASS(CBoundingSphere);


	/// Set properties (EDIT)
	void							setProperties( const NLMISC::CVector& center, float radius );
	/// Access the sphere (R/W) (EDIT)
	NLMISC::CBSphere				*getSphere()							{ return &_BSphere; }

private:

	NLMISC::CBSphere				_BSphere;
};


} // NLSOUND


#endif // NL_BOUNDING_SPHERE_H

/* End of bounding_sphere.h */
