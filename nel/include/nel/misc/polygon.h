/** \file polygon.h
 * 3D and 2D Polygons classes
 *
 * $Id: polygon.h,v 1.6 2002/04/11 08:40:38 corvazier Exp $
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


class CTriangle;

// Used by the method toConvexPolygons
class CBSPNode2v;

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

	/**
	  * Chain the arg polygons with this polygon.
	  *
	  * The polygon a-b-c-d-e-a chained with f-g-h-i-j-f will give a polygon like a-b-f-g-h-i-j-f-b-c-d-e-a
	  * if the edge f-j is not clipped by any edge plane.
	  *
	  * \return false if chain failed.
	  */
	bool			chain (const CPolygon &other);

	/// Serial this polygon
	void			serial(NLMISC::IStream &f) throw(NLMISC::EStream);

	/**
	  * Convert a concave polygon into a list of convex polygons using a 2d projection.
	  * The polygon mustn't overlap itself in the XY plane of the basis passed in parameter.
	  * The polygon must be direct in the XY plane of the basis passed in parameter. (Counter clock wise)
	  *
	  * The subdivison is in non-constant n*log(n) with n is the number of vertices.
	  *
	  * \param outputPolygons is the list filled with clipped convex polygons. The list is not cleared at the begining.
	  * New polygons are just appended at the end.
	  * \param basis is the basis of the polygon projection.
	  * \return true if the polygon has been subdivided. false if the polygon overlap itself in the XY plane of the basis
	  * or if the polygon is not direct (clock wise).
	  */
	bool			toConvexPolygons (std::list<CPolygon>& outputPolygons, const CMatrix& basis) const;

	// Used by the method toConvexPolygons
	static bool		toConvexPolygonsEdgeIntersect (const CVector2f& a0, const CVector2f& a1, const CVector2f& b0, const CVector2f& b1);
	static bool		toConvexPolygonsLeft (const std::vector<CVector> &vertex, uint a, uint b, uint c);
	static bool		toConvexPolygonsLeftOn (const std::vector<CVector> &vertex, uint a, uint b, uint c);
	static bool		toConvexPolygonsInCone (const std::vector<CVector> &vertex, uint a, uint b);
	static bool		toConvexPolygonsDiagonal (const std::vector<CVector> &vertex, const CBSPNode2v &bsp, uint a, uint b);
};

/**
  * A 2d convex polygon
  */
class CPolygon2D
{
public:
	/// default ctor
	CPolygon2D() {}
	
	/** Build a 2D polygon from this 3D polygon, by using the given projection matrix
	  * The x and y components of projected vertices are used to create the 2D polygon
	  */
	CPolygon2D(const CPolygon &src, const CMatrix &projMat = CMatrix::Identity);

	/** Build a 2D polygon from the given triangle, by using the given projection matrix
	  * The x and y components of projected vertices are used to create the 2D polygon
	  */
	CPolygon2D(const CTriangle &tri, const CMatrix &projMat = CMatrix::Identity);	

	/// Check wether this polygon is convex;
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

	/// Test wether this polygon intersect another convex polygon. Currently not optimized.
	bool        intersect(const CPolygon2D &other) const;

	/// Check wether a point is contained by this poly
	bool		contains(const CVector2f &p) const;

public:
	typedef std::vector<CVector2f> TVec2fVect;
	TVec2fVect Vertices;

protected:
	/// Sum the dot product of this poly vertices against a line equation a*x + b*y + c
	float sumDPAgainstLine(float a, float b, float c) const;

	/// Get ref to the first vertex that start at index
	const CVector2f &getSegRef0(uint index) const 
	{ 
		nlassert(index < Vertices.size()); return Vertices[index]; 
	}
	const CVector2f &getSegRef1(uint index) const 
	{ 
		nlassert(index < Vertices.size()); 		
		return index == Vertices.size() - 1 ?
			   Vertices[0]                 :
		       Vertices[index + 1];
	}


	/** Get the index of a segment of this poly that is a non null segment.
	  * \return true if such a segment was found
	  */
	bool  getNonNullSeg(uint &seg) const;

	/// Get a line equation of the given seg
	void  getLineEquation(uint index, float &a, float &b, float &c) const;
};


} // NLMISC


#endif // NL_POLYGON_H

/* End of polygon.h */
