/** \file bounding_box.h
 * CBoundingBox: Bounding box (different from the NL3D ones)
 *
 * $Id: bounding_box.h,v 1.3 2001/08/24 16:55:53 vizerie Exp $
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

#ifndef NL_BOUNDING_BOX_H
#define NL_BOUNDING_BOX_H

#include "nel/misc/types_nl.h"
#include "bounding_shape.h"
#include "nel/misc/vector.h"


namespace NLSOUND {


/**
 * Bounding box (different from the NL3D ones)
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CBoundingBox : public IBoundingShape
{
public:

	/// Constructor
	CBoundingBox();
	/// Static init (call at the very beginning)
	static void						init() 
	{
		NLMISC_REGISTER_CLASS(CBoundingBox); 	
	}


	/// Return true if the box includes the specified point
	virtual bool					include( const NLMISC::CVector& pos ) const;
	/// Return the approximate diameter
	virtual float					getDiameter() const;
	/// Return a reference to the center of the shape
	virtual const NLMISC::CVector&	getCenter() const				{ return _Center; }
	/// Move the shape
	virtual void					setCenter( const NLMISC::CVector& pos )	{ _Center = pos; }
	/// Serialize (if you change this, change the version in CEnvEffect::serialFileHeader() !)
	virtual void					serial( NLMISC::IStream& s )	{ s.serial( _Center ); s.serial( _Rx ); s.serial( _Ry ); s.serial( _Rz ); }
	NLMISC_DECLARE_CLASS(CBoundingBox);


	/// Return the corners (EDIT)
	void							getCorners( NLMISC::CVector& c1, NLMISC::CVector& c2 );
	/// Set the corners (the corners must be ordered: 1 has smaller x, y, z) (EDIT)
	void							setCorners( const NLMISC::CVector& c1, const NLMISC::CVector& c2 );

private:

	// The length of each size
	float					_Rx, _Ry, _Rz;

	// Center
	NLMISC::CVector			_Center;
};


} // NLSOUND


#endif // NL_BOUNDING_BOX_H

/* End of bounding_box.h */
