/** \file polygon.h
 * 3D and 2D Polygons classes
 *
 * $Id: polygon.h,v 1.4 2001/11/07 17:05:10 vizerie Exp $
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

#ifndef NL_POLYGON_H
#define NL_POLYGON_H

#include "nel/misc/types_nl.h"
#include "nel/misc/matrix.h"
#include "nel/misc/stream.h"
#include "nel/misc/vector_2f.h"
#include <vector>


namespace NLMISC 
{

using NLMISC::CVector;
using NLMISC::CPlane;
using NLMISC::CMatrix;




// ***************************************************************************
/**
 * A polygon, with an unlimited size of vertices.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CPolygon
{
public:
	std::vector<CVector>	Vertices;
	

public:

	/// Constructor
	CPolygon() {}
	/// Constructor. Init with a triangle.
	CPolygon(const CVector &a, const CVector &b, const CVector &c);

	sint			getNumVertices() const {return Vertices.size();}

	/// Clip a polygon with a set of planes. Cohen-sutherland... clipPolygonBack() is used on planes.
	void			clip(const CPlane *planes, uint nPlanes);
	/// Clip a polygon with a set of planes. Cohen-sutherland clipping... clipPolygonBack() is used on planes.
	void			clip(const std::vector<CPlane> &planes);
	
	/// Serial this polygon
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);

};


/**
  * A 2d polygon
  */
class CPolygon2D
{
public:
	/// default ctor
	CPolygon2D() {}

	/** Build a 2D polygon from this 3D polygon, by using the given projection matrix
	  * The x and y components of projected vertices are used to create the 2D polygon
	  */
	CPolygon2D(const CPolygon &src, CMatrix &projMat);

	/// check wether this polygon is convex
	bool		isConvex();

	/** Build a convex hull from this polygon. The result poly is ordered, so it can also be used to order a convex
	  * poly given its set of vertices.
	  * NB: require this != &dest
	  */
	void		buildConvexHull(CPolygon2D &dest) const;

	/// get the best triplet of vector. e.g the triplet that has the best surface
	void		getBestTriplet(uint &index0, uint &index1, uint &index2);	
	
	/// Serial this polygon
	void		serial(NLMISC::IStream &f) throw(NLMISC::EStream);

	typedef std::pair<sint, sint> TRaster;
	typedef std::vector<TRaster>  TRasterVect;

	/** Compute the borders of this poly with sub-pixel accuracy. No clipping is performed
	  * The output is in a vector of sint pairs. The highestY is the minimum y (assuming the highest vertex of the one with minimum y)
	  *	Rasters are created from the min y to the highest y  
	  */
	void		computeBorders(TRasterVect &borders, sint &highestY);
public:
	typedef std::vector<CVector2f> TVec2fVect;
	TVec2fVect Vertices;
};


} // NLMISC


#endif // NL_POLYGON_H

/* End of polygon.h */
