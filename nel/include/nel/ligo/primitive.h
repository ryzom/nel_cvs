/** \file primitive.h
 * <File description>
 *
 * $Id: primitive.h,v 1.6 2002/11/04 15:40:42 boucher Exp $
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

#ifndef __PRIMITIVE_H__
#define __PRIMITIVE_H__

#include "nel/misc/vector.h"
#include <vector>

namespace NLLIGO
{

// There are only x and y used in the vector
// LayerName is the Group to which the primitive belongs to
// Hide say if the primitive is hidden or not


// ***************************************************************************
class CPrimPoint
{

public:

	NLMISC::CVector		Point;
	std::string			Name;
	std::string			LayerName;

public:

	void serial (NLMISC::IStream &f);
};

// ***************************************************************************
class CPrimPath
{

public:

	std::vector<NLMISC::CVector>	VPoints;
	std::string						Name;
	std::string						LayerName;

public:

	void serial (NLMISC::IStream &f);
};


// ***************************************************************************
class CPrimZone
{

public:

	std::vector<NLMISC::CVector>	VPoints;
	std::string						Name;
	std::string						LayerName;

	static float getSegmentDist(const NLMISC::CVector v, const NLMISC::CVector &p1, const NLMISC::CVector &p2, NLMISC::CVector &nearPos);

public:

	bool contains (const NLMISC::CVector &v) const { return CPrimZone::contains(v, VPoints); }
	bool contains(const NLMISC::CVector &v, float &distance, NLMISC::CVector &nearPos, bool isPath) const { return CPrimZone::contains(v, VPoints, distance, nearPos, isPath); }
	void serial (NLMISC::IStream &f);

	// returns true if the vector v is inside of the patatoid
	static bool contains (const NLMISC::CVector &v, const std::vector<NLMISC::CVector> &points);
	// returns true if the vector v is inside of the patatoid and set the distance of the nearest segement and the position of the nearsest point.
	static bool contains (const NLMISC::CVector &v, const std::vector<NLMISC::CVector> &points, float &distance, NLMISC::CVector &nearPos, bool isPath);
};

// ***************************************************************************
class CPrimRegion
{

public:

	std::string				Name;
	std::vector<CPrimPoint> VPoints;
	std::vector<CPrimZone>	VZones;
	std::vector<CPrimPath>	VPaths;

	std::vector<bool>		VHidePoints;
	std::vector<bool>		VHideZones;
	std::vector<bool>		VHidePaths;

public:

	void serial (NLMISC::IStream &f);
};

} // namespace NLLIGO

#endif // __PRIMITIVE_H__

