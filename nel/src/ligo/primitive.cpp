/** \file primitive.cpp
 * <File description>
 *
 * $Id: primitive.cpp,v 1.4 2002/11/04 15:40:43 boucher Exp $
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

#include "nel/ligo/primitive.h"

using namespace NLMISC;
using namespace std;

namespace NLLIGO
{

// ***************************************************************************
void CPrimPoint::serial (IStream &f)
{
	f.xmlPushBegin ("POINT");

	f.xmlSetAttrib ("NAME");
	f.serial (Name);
	f.xmlSetAttrib ("LAYER");
	f.serial (LayerName);
	f.xmlPushEnd ();

	f.serial (Point);

	f.xmlPop ();
}

// ***************************************************************************
void CPrimPath::serial (IStream &f)
{
	f.xmlPushBegin ("PATH");
	
	f.xmlSetAttrib ("NAME");
	f.serial (Name);
	f.xmlSetAttrib ("LAYER");
	f.serial (LayerName);
	f.xmlPushEnd ();

	f.serialCont (VPoints);

	f.xmlPop ();
}

// ***************************************************************************
bool CPrimZone::contains (const NLMISC::CVector &v, const std::vector<NLMISC::CVector> &points)
{
	uint32 i;
	CVector vMin, vMax;

	// Point or line can't contains !
	if (points.size() < 3)
		return false;
	
	// Get the bounding rectangle of the zone
	vMax = vMin = points[0];
	for (i = 0; i < points.size(); ++i)
	{
		if (vMin.x > points[i].x)
			vMin.x = points[i].x;
		if (vMin.y > points[i].y)
			vMin.y = points[i].y;

		if (vMax.x < points[i].x)
			vMax.x = points[i].x;
		if (vMax.y < points[i].y)
			vMax.y = points[i].y;
	}

	if ((v.x < vMin.x) || (v.y < vMin.y) || (v.x > vMax.x) || (v.y > vMax.y))
		return false;

	uint32 nNbIntersection = 0;
	for (i = 0; i < points.size(); ++i)
	{
		const CVector &p1 = points[i];
		const CVector &p2 = points[(i+1)%points.size()];

		if (((p1.y-v.y) < 0.0)&&((p2.y-v.y) < 0.0))
			continue;
		if (((p1.y-v.y) > 0.0)&&((p2.y-v.y) > 0.0))
			continue;
		float xinter = p1.x + (p2.x-p1.x) * ((v.y-p1.y)/(p2.y-p1.y));
		if (xinter > v.x)
			++nNbIntersection;
	}
	if ((nNbIntersection&1) == 1) // odd intersections so the vertex is inside
		return true;
	else
		return false;
}



bool CPrimZone::contains (const NLMISC::CVector &v, const std::vector<NLMISC::CVector> &points, float &distance, NLMISC::CVector &nearPos, bool isPath)
{
	uint32 i;
	CVector vMin, vMax;
	float nearest = FLT_MAX;
	CVector pos;

	// Point or line can't contains !
	if (points.size() < 3 || isPath)
	{
		// only compute the distance.
		if (points.size() == 1)
		{
			distance = (points[0] - v).norm();
			nearPos = points[0];
		}
		else if (points.size() == 2)
		{
			distance = getSegmentDist(v, points[0], points[1], nearPos);
		}
		else
		{
			// compute nearest segment
			for (i = 0; i < points.size()-1; ++i)
			{
				const CVector &p1 = points[i];
				const CVector &p2 = points[i+1];

				float dist = getSegmentDist(v, p1, p2, pos);
				if( dist < nearest)
				{
					nearest = dist;
					nearPos = pos;
				}
			}
			distance = nearest;
		}
		return false;
	}
	
	// Get the bounding rectangle of the zone
	vMax = vMin = points[0];
	for (i = 0; i < points.size(); ++i)
	{
		vMin.x = min(vMin.x, points[i].x);
		vMin.y = min(vMin.y, points[i].y);
		vMax.x = max(vMax.x, points[i].x);
		vMax.y = max(vMax.y, points[i].y);
	}

	if ((v.x < vMin.x) || (v.y < vMin.y) || (v.x > vMax.x) || (v.y > vMax.y))
	{
		// find the nearest distance of all segment
		for (uint i=0; i<points.size(); ++i)
		{
			float dist = getSegmentDist(v, points[i], points[(i+1) % points.size()], pos);

			if (dist < nearest)
			{
				nearest = dist;
				nearPos = pos;
			}
		}
		distance = nearest;
		return false;
	}

	uint32 nNbIntersection = 0;
	for (i = 0; i < points.size(); ++i)
	{
		const CVector &p1 = points[i];
		const CVector &p2 = points[(i+1)%points.size()];

		float dist = getSegmentDist(v, p1, p2, pos);
		if( dist < nearest)
		{
			nearest = dist;
			nearPos = pos;
		}

		if (((p1.y-v.y) < 0.0)&&((p2.y-v.y) < 0.0))
			continue;
		if (((p1.y-v.y) > 0.0)&&((p2.y-v.y) > 0.0))
			continue;
		float xinter = p1.x + (p2.x-p1.x) * ((v.y-p1.y)/(p2.y-p1.y));
		if (xinter > v.x)
			++nNbIntersection;
	}

	distance = nearest;
	if ((nNbIntersection&1) == 1) // odd intersections so the vertex is inside
		return true;
	else
		return false;
}

float CPrimZone::getSegmentDist(const NLMISC::CVector v, const NLMISC::CVector &p1, const NLMISC::CVector &p2, NLMISC::CVector &nearPos)
{
	// too point, compute distance to the segment.
	CVector V = (p2-p1).normed();
	float t = ((v-p1)*V)/(p2-p1).norm();
	float distance;
	if (t < 0.0f)
	{
		nearPos = p1;
		distance = (p1-v).norm();
	}
	else if (t > 1.0f)
	{
		nearPos = p2;
		distance = (p2-v).norm();
	}
	else
	{
		nearPos = p1 + t*(p2-p1);
		distance = (v-nearPos).norm();
	}

	return distance;
}



// ***************************************************************************
void CPrimZone::serial (IStream &f)
{
	f.xmlPushBegin ("ZONE");
	
	f.xmlSetAttrib ("NAME");
	f.serial (Name);
	f.xmlSetAttrib ("LAYER");
	f.serial (LayerName);
	f.xmlPushEnd ();

	f.serialCont (VPoints);

	f.xmlPop ();
}

// ***************************************************************************
void CPrimRegion::serial (IStream &f)
{
	f.xmlPushBegin ("REGION");
	
	f.xmlSetAttrib ("NAME");
	f.serial (Name);

	f.xmlPushEnd();

	sint version = 2;
	version = f.serialVersion (version);
	string check = "REGION";
	f.serialCheck (check);

	f.xmlPush ("POINTS");
		f.serialCont (VPoints);
	f.xmlPop ();
	f.xmlPush ("PATHES");
		f.serialCont (VPaths);
	f.xmlPop ();
	f.xmlPush ("ZONES");
		f.serialCont (VZones);
	f.xmlPop ();

	if (version > 1)
	{
		f.xmlPush ("HIDEPOINTS");
			f.serialCont (VHidePoints);
		f.xmlPop ();
		f.xmlPush ("HIDEZONES");
			f.serialCont (VHideZones);
		f.xmlPop ();
		f.xmlPush ("HIDEPATHS");
			f.serialCont (VHidePaths);
		f.xmlPop ();
	}
	else
	{
		VHidePoints.resize	(VPoints.size(), false);
		VHideZones.resize	(VZones.size(),	false);
		VHidePaths.resize	(VPaths.size(), false);
	}
}

} // namespace NLLIGO