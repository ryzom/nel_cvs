/** \file aabbox.h
 * <File description>
 *
 * $Id: aabbox.h,v 1.4 2000/12/11 15:50:44 berenguier Exp $
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
#include "nel/misc/stream.h"


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
	/** extend the bbox so it contains v.
	 * Warning!! By default, a bbox is the vector 0,0,0. So set the first vertex with setCenter() or else the bbox will 
	 * be the extension of v and (0,0,0)...
	 */
	void			extend(const CVector &v);
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
	bool			clipFront(const CPlane &p) const;
	/// Is the bbox partially in back of the plane??
	bool			clipBack(const CPlane &p) const;
	// @}

	void			serial(NLMISC::IStream &f);
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
	/// Constructor from a normal BBox.
	CAABBoxExt(const CAABBox &o) {RadiusMin= RadiusMax=0; *this=o;}


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
	CAABBoxExt		&operator=(const CAABBox &o) {Center= o.getCenter(); HalfSize= o.getHalfSize(); updateRadius(); return (*this);}
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
	/// Is the bbox partially in front of the plane?? p MUST be normalized.
	bool			clipFront(const CPlane &p) const;
	/// Is the bbox partially in back of the plane?? p MUST be normalized.
	bool			clipBack(const CPlane &p) const;
	// @}

	void			serial(NLMISC::IStream &f);
};


} // NL3D


#endif // NL_AABBOX_H

/* End of aabbox.h */
