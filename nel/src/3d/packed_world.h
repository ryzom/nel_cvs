/** \file packed_world.h
 * Collision on a set of packed zones (packed representation)
 *
 * $Id:
 */

/* Copyright, 2000-2006 Nevrax Ltd.
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

#ifndef NL_PACKED_WORLD
#define NL_PACKED_WORLD


#include "packed_zone.h"
//
#include "nel/misc/array_2d.h"
#include "nel/misc/stream.h"
#include "nel/misc/triangle.h"
//
#include <vector>



namespace NLMISC
{
	class CVector;	
}

namespace NL3D
{

/** A set of packed zones
  * Allows for world scale collision test with a packed format
  *  
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2005
  */
class CPackedWorld
{
public:
	// list of zones that goes into this packed world (used by incremental build to know if some
	// zones need to be rebuilt)
	std::vector<std::string> ZoneNames;
public:		
	// build world from a set of packed zones
	void build(std::vector<TPackedZoneBaseSPtr> &packesZones);
	bool raytrace(const NLMISC::CVector &start, const NLMISC::CVector &end, NLMISC::CVector &inter, std::vector<NLMISC::CTriangle> *testedTriangles = NULL, NLMISC::CVector *normal = NULL);
	void getZones(std::vector<TPackedZoneBaseSPtr> &zones);
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	// just serialize the header, containing name of the zones this CPackedWorld was built from
	void serialZoneNames(NLMISC::IStream &f) throw(NLMISC::EStream);
	/** Roughly select triangles that are within a convex 2D polygon (world coordinates)
	  * Selection is not exact, because limited to the resolution of the grid into which is packed each zone.
	  * Triangle that are within are guaranteed to be selected, however.
	  */
	void select(const NLMISC::CPolygon2D &poly, std::vector<NLMISC::CTriangle> &selectedTriangles) const;
private:
	class CZoneInfo
	{
	public:
		TPackedZoneBaseSPtr Zone;
		uint32				RaytraceCounter;
		void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
		{
			f.serialVersion(1);
			CPackedZoneBase *pz = Zone;
			f.serialPolyPtr(pz);
			Zone = pz;
		}
	};
	std::vector<CZoneInfo>				 _Zones;	
	class CZoneIndexList
	{
	public:
		std::vector<uint32> IDs; // make a class from this vector just for serialization
		void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
		{
			f.serialCont(IDs);
		}
	};
	NLMISC::CArray2D<CZoneIndexList>	   _ZoneGrid;
	sint32								   _ZoneMinX;
	sint32								   _ZoneMinY;
	uint32								   _RaytraceCounter;
};

} // NL3D


#endif