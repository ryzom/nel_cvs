/** \file primitive.cpp
 * <File description>
 *
 * $Id: primitive.cpp,v 1.2 2002/04/24 16:26:02 besson Exp $
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
bool CPrimZone::contains (NLMISC::CVector &v)
{
	uint32 i;
	CVector vMin, vMax;

	if (VPoints.size() == 0)
		return false;
	
	// Get the bounding rectangle of the zone
	vMax = vMin = VPoints[0];
	for (i = 0; i < VPoints.size(); ++i)
	{
		if (vMin.x > VPoints[i].x)
			vMin.x = VPoints[i].x;
		if (vMin.y > VPoints[i].y)
			vMin.y = VPoints[i].y;

		if (vMax.x < VPoints[i].x)
			vMax.x = VPoints[i].x;
		if (vMax.y < VPoints[i].y)
			vMax.y = VPoints[i].y;
	}

	if ((v.x < vMin.x) || (v.y < vMin.y) || (v.x > vMax.x) || (v.y > vMax.y))
		return false;

	uint32 nNbIntersection = 0;
	for (i = 0; i < VPoints.size(); ++i)
	{
		CVector &p1 = VPoints[i];
		CVector &p2 = VPoints[(i+1)%VPoints.size()];

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