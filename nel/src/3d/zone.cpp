/** \file zone.cpp
 * <File description>
 *
 * $Id: zone.cpp,v 1.3 2000/11/02 13:58:54 berenguier Exp $
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

#include "nel/3d/zone.h"


namespace NL3D {


// ***************************************************************************
CZone::CZone()
{
	ComputeTileErrorMetric= false;
	ZoneId= 0;
	Compiled= false;
}
// ***************************************************************************
CZone::~CZone()
{
	// release() must have been called.
	nlassert(!Compiled);
}


// ***************************************************************************
void			CZone::build(uint16 zoneId, const std::vector<CPatchInfo> &patchs, const std::vector<CBorderVertex> &borderVertices)
{
	nlassert(!Compiled);

	ZoneId= zoneId;
	BorderVertices= borderVertices;

	Patchs.resize(patchs.size());
	PatchConnects.resize(patchs.size());

	for(sint i=0;i<(sint)patchs.size();i++)
	{
	//sint					capacity() {return _Verts.size();}
	}
}

// ***************************************************************************
void			CZone::compile(std::map<uint16, CZone*> &loadedZones)
{



	Compiled= true;
}

// ***************************************************************************
void			CZone::release(std::map<uint16, CZone*> &loadedZones)
{



	Compiled= false;
}



} // RK3D
