/** \file zone_region.cpp
 * <File description>
 *
 * $Id: zone_region.cpp,v 1.1 2001/12/28 14:58:40 besson Exp $
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

#include "zone_region.h"

using namespace NLMISC;
using namespace std;

namespace NLLIGO
{

string CZoneRegion::_StringOutOfBound;

// ---------------------------------------------------------------------------
CZoneRegion::SZoneUnit::SZoneUnit()
{
	ZoneName = STRING_UNUSED;
	PosX = PosY = 0;
	Rot = Flip = 0;
	SharingMatNames[0] = STRING_UNUSED;
	SharingMatNames[1] = STRING_UNUSED;
	SharingMatNames[2] = STRING_UNUSED;
	SharingMatNames[3] = STRING_UNUSED;
	SharingCutEdges[0] = 0;
	SharingCutEdges[1] = 0;
	SharingCutEdges[2] = 0;
	SharingCutEdges[3] = 0;
}

// ---------------------------------------------------------------------------
void CZoneRegion::SZoneUnit::serial (NLMISC::IStream &f)
{
	f.serial (ZoneName);
	f.serial (PosX);
	f.serial (PosY);
	f.serial (Rot);
	f.serial (Flip);

	for (uint32 i = 0; i < 4; ++i)
	{
		f.serial (SharingMatNames[i]);
		f.serial (SharingCutEdges[i]);
	}
}

// ---------------------------------------------------------------------------
const CZoneRegion::SZoneUnit& CZoneRegion::SZoneUnit::operator=(const CZoneRegion::SZoneUnit&zu)
{
	this->ZoneName	= zu.ZoneName;
	this->PosX		= zu.PosX;
	this->PosY		= zu.PosY;
	this->Rot		= zu.Rot;
	this->Flip		= zu.Flip;
	for (uint32 i = 0; i < 4; ++i)
	{
		this->SharingMatNames[i] = zu.SharingMatNames[i];
		this->SharingCutEdges[i] = zu.SharingCutEdges[i];
	}
	return *this;
}

// ---------------------------------------------------------------------------
CZoneRegion::CZoneRegion()
{
	_StringOutOfBound = STRING_OUT_OF_BOUND;
}

// ---------------------------------------------------------------------------
void CZoneRegion::serial (NLMISC::IStream &f)
{
	sint32 version = f.serialVersion (0);
	f.serialCheck ((uint32)'DNAL');
	f.serial (_MinX);
	f.serial (_MinY);
	f.serial (_MaxX);
	f.serial (_MaxY);
	f.serialCont (_Zones);

}

// ---------------------------------------------------------------------------
const string &CZoneRegion::getName (sint32 x, sint32 y)
{
	if ((x < _MinX) || (x > _MaxX) ||
		(y < _MinY) || (y > _MaxY))
	{
		return _StringOutOfBound;
	}
	else
	{
		return _Zones[(x-_MinX)+(y-_MinY)*(1+_MaxX-_MinX)].ZoneName;
	}
}

// ---------------------------------------------------------------------------
uint8 CZoneRegion::getPosX (sint32 x, sint32 y)
{
	if ((x < _MinX) || (x > _MaxX) ||
		(y < _MinY) || (y > _MaxY))
	{
		return 0;
	}
	else
	{
		return _Zones[(x-_MinX)+(y-_MinY)*(1+_MaxX-_MinX)].PosX;
	}
}

// ---------------------------------------------------------------------------
uint8 CZoneRegion::getPosY (sint32 x, sint32 y)
{
	if ((x < _MinX) || (x > _MaxX) ||
		(y < _MinY) || (y > _MaxY))
	{
		return 0;
	}
	else
	{
		return _Zones[(x-_MinX)+(y-_MinY)*(1+_MaxX-_MinX)].PosY;
	}
}

// ---------------------------------------------------------------------------
uint8 CZoneRegion::getRot (sint32 x, sint32 y)
{
	if ((x < _MinX) || (x > _MaxX) ||
		(y < _MinY) || (y > _MaxY))
	{
		return 0;
	}
	else
	{
		return _Zones[(x-_MinX)+(y-_MinY)*(1+_MaxX-_MinX)].Rot;
	}
}

// ---------------------------------------------------------------------------
uint8 CZoneRegion::getFlip (sint32 x, sint32 y)
{
	if ((x < _MinX) || (x > _MaxX) ||
		(y < _MinY) || (y > _MaxY))
	{
		return 0;
	}
	else
	{
		return _Zones[(x-_MinX)+(y-_MinY)*(1+_MaxX-_MinX)].Flip;
	}
}

} // namespace NLLIGO