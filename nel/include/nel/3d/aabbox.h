/** \file aabbox.h
 * <File description>
 *
 * $Id: aabbox.h,v 1.1 2000/10/27 14:30:33 berenguier Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
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

#ifndef NL_AABBOX_H
#define NL_AABBOX_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/plane.h"
#include "nel/misc/common.h"


namespace NL3D {


using	NLMISC::CVector;
using	NLMISC::CPlane;


// ***************************************************************************
/**
 * An Axis Aligned Bouding Box.
 * Note: Center/HalfSize set to private, to have same manipulation for CAABBox and CAABBoxExt.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CAABBox
{
protected:
	/// The center of the bbox.
	CVector			Center;
	/// The size/2 of the bbox.
	CVector			HalfSize;

public:

	/// Empty bbox Constructor.  (for AABBoxExt::getRadius() correctness).
	CAABBox() : Center(0,0,0), HalfSize(0,0,0) {}


	/// \name Builds.
	// @{
	void			setCenter(const CVector &center) {Center= center;}
	void			setHalfSize(const CVector &hs) {HalfSize= hs;}
	void			setSize(const CVector &s) {HalfSize= s/2;}
	/// Build the bbox, with a min/max style bbox.
	void			setMinMax(const CVector &bmin, const CVector &bmax)
	{
		Center= (bmin+bmax)/2;
		HalfSize= bmax-Center;
	}
	//@}


	/// \name Gets.
	// @{
	CVector			getMin() const {return Center-HalfSize;}
	CVector			getMax() const {return Center+HalfSize;}
	CVector			getCenter() const {return Center;}
	CVector			getHalfSize() const {return HalfSize;}
	/// Return the size of the bbox.
	CVector			getSize() const {return HalfSize*2;}
	/// Return the radius of the bbox.
	float			getRadius() const {return HalfSize.norm();}
	// @}

	/// \name Clip
	// @{
	/// Is the bbox partially in front of the plane??
	bool	clip(const CPlane &p) const;
	// @}

};


// ***************************************************************************
/**
 * An Extended Axis Aligned Bouding Box.  Sphere Min/Max Radius is stored for improved clip test.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	CAABBoxExt : private CAABBox
{
protected:
	float			RadiusMin, RadiusMax;

	void			updateRadius()
	{
		// The bounding sphere.
		RadiusMax= CAABBox::getRadius();
		// The including sphere.
		RadiusMin= NLMISC::minof((float)fabs(HalfSize.x), (float)fabs(HalfSize.y), (float)fabs(HalfSize.z));
	}

public:
	/// Empty bbox Constructor
	CAABBoxExt() {RadiusMin= RadiusMax=0;}


	/// \name Builds.
	// @{
	void			setCenter(const CVector &center) {Center= center;}
	void			setHalfSize(const CVector &hs) {HalfSize= hs; updateRadius();}
	void			setSize(const CVector &s) {HalfSize= s/2;  updateRadius();}
	/// Build the bbox, with a min/max style bbox.
	void			setMinMax(const CVector &bmin, const CVector &bmax)
	{
		Center= (bmin+bmax)/2;
		HalfSize= bmax-Center;
		updateRadius();
	}
	CAABBoxExt		&operator=(const CAABBox &o) {Center= o.getCenter(); HalfSize= o.getHalfSize(); updateRadius();}
	//@}


	/// \name Gets.
	// @{
	CVector			getMin() const {return CAABBox::getMin();}
	CVector			getMax() const {return CAABBox::getMax();}
	CVector			getCenter() const {return Center;}
	CVector			getHalfSize() const {return HalfSize;}
	/// Return the size of the bbox.
	CVector			getSize() const {return HalfSize*2;}
	/// Return the (stored!!) radius of the bbox.
	float			getRadius() const {return RadiusMax;}
	// @}

	/// \name Clip
	// @{
	/// Is the bbox partially in front of the plane??
	bool	clip(const CPlane &p) const;
	/// same as clip(), but assume p is normalized.
	bool	clipUnitPlane(const CPlane &p) const;
	// @}

};


} // RK3D


#endif // NL_AABBOX_H

/* End of aabbox.h */
