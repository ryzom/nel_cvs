/** \file polygon.cpp
 * <File description>
 *
 * $Id: polygon.cpp,v 1.5 2001/11/14 15:51:54 vizerie Exp $
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

#include "nel/misc/polygon.h"
#include "nel/misc/plane.h"
#include <list>

using namespace std;
using namespace NLMISC;


namespace NLMISC 
{


//==================================//
//		CPolygon implementation     //
//==================================//

// ***************************************************************************
CPolygon::CPolygon(const CVector &a, const CVector &b, const CVector &c)
{
	Vertices.push_back(a);
	Vertices.push_back(b);
	Vertices.push_back(c);
}


// ***************************************************************************
void			CPolygon::clip(const CPlane	 *planes, uint nPlanes)
{
	if(nPlanes==0 || getNumVertices()==0)
		return;

	// The final polygon has at maximum currentVertices+number of clipping planes.
	// For performance, the vectors are static, so reallocation rarely occurs.
	static	vector<CVector>		tab0, tab1;
	tab0.resize(getNumVertices()+nPlanes);
	tab1.resize(getNumVertices()+nPlanes);
	// Init tab0 with Vertices.
	copy(Vertices.begin(), Vertices.end(), tab0.begin());
	CVector				*in=&(*tab0.begin()), *out= &(*tab1.begin());
	sint				nin= getNumVertices(), nout;
	for(sint i=0;i<(sint)nPlanes;i++)
	{
		nout= planes[i].clipPolygonBack(in, out, nin);
		swap(in, out);
		nin= nout;
		if(nin==0)
			break;
	}

	// Final result in "in".
	Vertices.resize(nin);
	if(nin>0)
	{
		memcpy(&(*Vertices.begin()), in, nin*sizeof(CVector));
	}
}


// ***************************************************************************
void			CPolygon::clip(const std::vector<CPlane> &planes)
{
	if(planes.size()==0)
		return;
	clip(&(*planes.begin()), planes.size());
}



// ***************************************************************************
void CPolygon::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(0);
	f.serialCont(Vertices);
}


//====================================//
//		CPolygon2d implementation     //
//====================================//



CPolygon2D::CPolygon2D(const CPolygon &src, CMatrix &projMat)
{
	uint size = src.Vertices.size();
	Vertices.resize(size);
	for (uint k = 0; k < size; ++k)
	{
		CVector proj = projMat * src.Vertices[k];
		Vertices[k].set(proj.x, proj.y);
	}
}

// ***************************************************************************

bool		CPolygon2D::isConvex()
{
	bool Front  = true, Back = false;	
	// we apply a dummy algo for now : check wether every vertex is in the same side 
	// of every plane defined by a segment of this poly
	uint numVerts = Vertices.size();
	if (numVerts < 3) return true;
	CVector		segStart, segEnd;
	CPlane		clipPlane;
	for (TVec2fVect::const_iterator it = Vertices.begin(); it != Vertices.end(); ++it)
	{		
		segStart.set(it->x, it->y, 0);	          // segment start
		segEnd.set((it + 1)->x, (it + 1)->y, 0);  // segment end
		float n = (segStart - segEnd).norm();     // segment norm
		if (n != 0)
		{
			clipPlane.make(segStart, segEnd, (n > 10 ? n : 10) * CVector::K + segStart); // make a plane, with this segment and the poly normal
			// check each other vertices against this plane
			for (TVec2fVect::const_iterator it2 = Vertices.begin(); it2 != Vertices.end(); ++it2)
			{
				if (it2 != it && it2 != (it + 1)) // the vertices must not be part of the test plane (because of imprecision)
				{

					float dist  = clipPlane * CVector(it2->x, it2-> y, 0);
					if (dist != 0) // midlle pos
					{
						if (dist > 0) Front = true; else Back = true;					
						if (Front && Back) return false; // there are both front end back vertices -> failure
					}
				}
			}
		}
	}
	return true;
}

// ***************************************************************************

void		CPolygon2D::buildConvexHull(CPolygon2D &dest) const
{
	nlassert(&dest != this);

	if (this->Vertices.size() == 3) // with 3 points it is always convex
	{
		dest = *this;
		return;
	}
	uint k, l;
	uint numVerts = Vertices.size();	
	CVector2f p, curr, prev;
	uint      pIndex, p1Index, p2Index, pCurr, pPrev, pNew;
	// this is not optimized, but not used in realtime.. =)
	nlassert(numVerts >= 3);
	dest.Vertices.clear();

	// 1°) find the highest point p of the set. We are sure it belongs to the hull
	pIndex = 0;
	p = Vertices[0];
	for (k = 1; k < numVerts; ++k)
	{
		if (Vertices[k].y < p.y)
		{
			pIndex = k;
			p = Vertices[k];
		}
	}

	// find the couple of points (p1, p2) that gives the best cross-product (p2 - p) ^ (p - p1)
	float bestCP = 0;
	p1Index = p2Index = pIndex;

	for (k = 0; k < numVerts; ++k)
	{
		if (k != pIndex)
		{
			for (l = 0; l < numVerts; ++l)
			{
				if (l != pIndex && l != k)
				{
					CVector2f seg1 = (Vertices[l] - p).normed();
					CVector2f seg2 = (p - Vertices[k]).normed();

					CVector cp = CVector(seg1.x, seg1.y, 0) ^ CVector(seg2.x, seg2.y, 0);
					float n = fabsf(cp.z);
					if (n > bestCP)
					{
						p1Index = l;
						p2Index = k;
						bestCP  = n;
					}
				}
			}
		}
	}
	

	// start from the given triplet, and complete the poly until we reach the first point
	pCurr = p2Index;
	pPrev = pIndex;

	curr = Vertices[pCurr];
	prev = Vertices[pPrev];

	// create the first triplet vertices
	dest.Vertices.push_back(Vertices[p1Index]);
	dest.Vertices.push_back(Vertices[pPrev]);
	dest.Vertices.push_back(Vertices[pCurr]); 


	uint step = 0;
	for(;;)
	{
		bestCP = 0;
		pNew   = pCurr;
		for (l = 0; l < numVerts; ++l)
		{
			if (step == 0 && l == p1Index) continue;
			if (l != pCurr && l != pPrev)
			{
				CVector2f seg1 = (Vertices[l] - curr).normed();
				CVector2f seg2 = (curr - prev).normed();

				CVector cp = CVector(seg1.x, seg1.y, 0) ^ CVector(seg2.x, seg2.y, 0);				
				float n = fabsf(cp.z);
				if (n > bestCP)
				{
					if (l == p1Index) return; // if we reach the start point we have finished
					bestCP = n;
					pNew   = l;					
				}
			}
		}
		nlassert(pNew != pCurr);
		prev = curr;
		curr = Vertices[pNew];
		pPrev = pCurr;
		pCurr = pNew;
		// add new point to the destination
		dest.Vertices.push_back(curr); 
		++ step;
	}
}

// ***************************************************************************


void CPolygon2D::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	sint ver = f.serialVersion(0);
	f.serialCont(Vertices);
}

// ***************************************************************************
/// get the best triplet of vector. e.g the triplet that has the best surface
void		CPolygon2D::getBestTriplet(uint &index0, uint &index1, uint &index2)
{
	nlassert(Vertices.size() >= 3);
	uint i, j, k;
	float bestArea = 0.f;
	const uint numVerts = Vertices.size();
	for (i = 0; i < numVerts; ++i)
	{
		for (j = 0; j < numVerts; ++j)
		{
			if (i != j)
			{
				for (k = 0; k < numVerts; ++k)
				{
					if (k != i && k != j)
					{
						CVector2f v0 = Vertices[j] - Vertices[i];
						CVector2f v1 = Vertices[k] - Vertices[i];						
						float area = fabsf((CVector(v0.x, v0.y, 0) ^ CVector(v1.x, v1.y, 0)).norm());
						if (area > bestArea)
						{
							bestArea = area;
							index0 = i;
							index1 = j;
							index2 = k;
						}
					}
				}
			}
		}
	}
}

/// ***************************************************************************************
// scan a an edge of a poly and write it into a table

static void ScanEdge(CPolygon2D::TRasterVect &outputVect, sint topY, const CVector2f &v1, const CVector2f &v2, bool rightEdge = true)
{
	 const uint rol16 = 65536;
	 sint ceilY1 = (sint) ceilf(v1.y);
	 sint height;
	 float deltaX, deltaY;
	 float fInverseSlope;
	 sint  iInverseSlope, iPosX;

	 // check wether this segment gives a contribution to the final poly
	 height = (sint) (ceilf(v2.y) - ceilY1);
	 if (height <= 0) return;

	 // compute slope
	 deltaY = v2.y - v1.y;
	 deltaX = v2.x - v1.x;
	 fInverseSlope = deltaX / deltaY;

	 
	 CPolygon2D::TRasterVect::iterator  outputIt = outputVect.begin() + (ceilY1 - topY);

	 // slope with ints
	 iInverseSlope = (sint) (rol16 * fInverseSlope);	 

	 // sub-pixel accuracy
	 iPosX = (int) (rol16 * (v1.x + fInverseSlope * (ceilY1 - v1.y))); 

	 const CPolygon2D::TRasterVect::iterator endIt = outputIt + height;
	 if (rightEdge)
	 {		
		 do
		 {
		   outputIt->second =  iPosX >> 16;	
		   iPosX += iInverseSlope;
		   ++outputIt;
		 }
		 while (outputIt != endIt);
	 }
	 else
	 {	
		 iPosX += (rol16 - 1);
		 do
		 {
		   outputIt->first =  iPosX >> 16;	
		   iPosX += iInverseSlope;
		   ++outputIt;
		 }
		 while (outputIt != endIt);
	 }
}


// This function alow to cycle forward through a vertex vector like if it was a circular list
static inline CPolygon2D::TVec2fVect::const_iterator Next(const CPolygon2D::TVec2fVect::const_iterator &it, const CPolygon2D::TVec2fVect &cont)
{
	nlassert(cont.size() != 0);
	if ((it + 1) == cont.end()) return cont.begin();
	return (it + 1);
}


// This function alow to cycle backward through a (non null) vertex vector like if it was a circular list
static inline CPolygon2D::TVec2fVect::const_iterator Prev(const CPolygon2D::TVec2fVect::const_iterator &it, const CPolygon2D::TVec2fVect &cont)
{
	nlassert(cont.size() != 0);
	if (it == cont.begin()) return cont.end() - 1;
	return (it - 1);
}



// *******************************************************************************

void	CPolygon2D::computeBorders(TRasterVect &borders, sint &highestY)
{
	// an 'alias' to the vertices
	const TVec2fVect &V = Vertices;
	if (Vertices.size() < 3)
	{
		borders.clear();
		return;
	}
	bool    ccw;  // set to true when it has a counter clock wise orientation
                   
	// compute highest and lowest pos of the poly
	float fHighest = V[0].y;
	float fLowest  = fHighest;

	// iterators to the thighest and lowest vertex
	TVec2fVect::const_iterator pLowest = V.begin(), pHighest = V.begin();
	TVec2fVect::const_iterator it = V.begin() ;
	const TVec2fVect::const_iterator endIt = V.end();
	do
	{
		if (it->y > fLowest)
		{
			fLowest = it->y;
			pLowest = it;
		}
		else
		if (it->y < fHighest)
		{
			fHighest = it->y;
			pHighest = it;
		}
		++it;
	}
	while (it != endIt);
	

	sint iHighest = (sint) ceilf(fHighest) ;
	sint iLowest  = (sint) ceilf(fLowest) ;

	highestY = iHighest;


	/// check poly height, and discard null height
	uint polyHeight = iLowest - iHighest;
	if (polyHeight <= 0)
	{
		borders.clear();
		return;
	}

	borders.resize(polyHeight);

	// iterator to the first vertex that has an y different from the top vertex
	TVec2fVect::const_iterator pHighestRight = pHighest; 
	// we seek this vertex	
	while (Next(pHighestRight, V)->y == fHighest)
	{
		pHighestRight = Next(pHighestRight, V);
	}	

	// iterator to the first vertex after pHighestRight, that has the same y than the highest vertex
	TVec2fVect::const_iterator pHighestLeft = Next(pHighestRight, V);
	// seek the vertex
	while (pHighestLeft->y != fHighest)
	{
		pHighestLeft = Next(pHighestLeft, V);
	}

	TVec2fVect::const_iterator pPrevHighestLeft = Prev(pHighestLeft, V);
  
	// we need to get the orientation of the polygon
	// There are 2 case : flat, and non-flat top

	// check for flat top
	if (pHighestLeft->x != pHighestRight->x)
	{
		// compare right and left side
		if (pHighestLeft->x> pHighestRight->x)
		{
			ccw = true;  // the list is CCW oriented
			std::swap(pHighestLeft, pHighestRight);		
		}
		else
		{
			ccw = false; // the list is CW oriented	
		}
	}
	else
	{
		// The top of the poly is sharp
		// We perform a cross product of the 2 highest vect to get its orientation		
		// l'orientation

		 const float deltaXN = Next(pHighestRight, V)->x - pHighestRight->x;
		 const float deltaYN = Next(pHighestRight, V)->y - pHighestRight->y;
		 const float deltaXP = pPrevHighestLeft->x - pHighestLeft->x;
		 const float deltaYP = pPrevHighestLeft->y - pHighestLeft->y;
		 if ((deltaXN * deltaYP - deltaYN * deltaXP) < 0)
		 {
			ccw = true;  // the list is CCW oriented
			std::swap(pHighestLeft, pHighestRight);				   
		 }
		 else
		 {
			ccw = false; // the list is CW oriented
		 }
	}


	// compute borders
	TVec2fVect::const_iterator currV, nextV; // current and next vertex
	if (!ccw) // clock wise order ?
	{
		currV = pHighestRight ;
		// compute right edge from top to bottom
		do
		{
			nextV = Next(currV, V);
			ScanEdge(borders, iHighest, *currV, *nextV, true);
            currV = nextV;            
		}
		while (currV != pLowest); // repeat until we reach the bottom vertex

		// compute left edge from bottom to top
		do
		{
   			nextV = Next(currV, V);
			ScanEdge(borders, iHighest, *nextV, *currV, false);
			currV = nextV;
		}
		while (currV != pHighestLeft);
	}
	else // ccw order
	{	 
		currV = pHighestLeft;
		// compute left edge from top to bottom
		do
		{
			nextV = Next(currV, V);
			ScanEdge(borders, iHighest, *currV
						   , *nextV
						   , false
					) ;
			currV = nextV;
		}
		while (currV != pLowest) ;

		// compute right edge from bottom to bottom
		do
		{
			nextV = Next(currV, V);
			ScanEdge(borders, iHighest, *nextV
						   , *currV
						   , true
					);
			currV = nextV;
		}
		while (currV != pHighestRight)  ;
	}
}





} // NLMISC
