/** \file ig_surface_light.cpp
 * <File description>
 *
 * $Id: ig_surface_light.cpp,v 1.3 2002/02/28 12:59:49 besson Exp $
 */

/* Copyright, 2000-2002 Nevrax Ltd.
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

#include "std3d.h"

#include "3d/ig_surface_light.h"


namespace NL3D 
{

// ***************************************************************************
CIGSurfaceLight::CIGSurfaceLight()
{
	_Owner= NULL;
	_CellSize= 1;
	_OOCellSize= 1;
}

// ***************************************************************************
void			CIGSurfaceLight::setOwner(CInstanceGroup *owner)
{
	nlassert(owner!=NULL);
	_Owner= owner;
}

// ***************************************************************************
void			CIGSurfaceLight::build(const TRetrieverGridMap &retrieverGridMap, float cellSize,
	const std::vector<uint>	&plRemap)
{
	_RetrieverGridMap= retrieverGridMap;
	nlassert(cellSize>0);
	_CellSize= cellSize;
	_OOCellSize= 1.f/_CellSize;

	// remap indices.
	ItRetrieverGridMap	it;
	for(it= _RetrieverGridMap.begin(); it!=_RetrieverGridMap.end(); it++)
	{
		// For all grids of this retriever
		for(uint iGrid= 0; iGrid<it->second.Grids.size(); iGrid++)
		{
			CSurfaceLightGrid	&grid= it->second.Grids[iGrid];

			// For all cells of this grid.
			for(uint iCell= 0; iCell<grid.Cells.size(); iCell++)
			{
				CSurfaceLightGrid::CCellCorner	&cell= grid.Cells[iCell];

				// For all point light id.
				for(uint lid= 0; lid<CSurfaceLightGrid::NumLightPerCorner; lid++)
				{
					if(cell.Light[lid] == 0xFF)
						break;
					else
						cell.Light[lid]= plRemap[cell.Light[lid]];
				}

				// remap ambient light
				if(cell.LocalAmbientId!=0xFF)
					cell.LocalAmbientId= plRemap[cell.LocalAmbientId];
			}
		}
	}
}

// ***************************************************************************
void			CIGSurfaceLight::clear()
{
	_RetrieverGridMap.clear();
	_CellSize= 1;
	_OOCellSize= 1;
}

// ***************************************************************************
void			CIGSurfaceLight::serial(NLMISC::IStream &f)
{
	uint	ver= f.serialVersion(0);

	f.serial(_CellSize);
	f.serial(_OOCellSize);
	f.serialCont(_RetrieverGridMap);
}

// ***************************************************************************
bool			CIGSurfaceLight::getStaticLightSetup(const std::string &retrieverIdentifier, sint surfaceId, const CVector &localPos, 
	std::vector<CPointLightInfluence> &pointLightList, uint8 &sunContribution, NLMISC::CRGBA &localAmbient)
{
	nlassert(_Owner);

	// default
	sunContribution= 255;
	localAmbient.set(0,0,0,0);

	ItRetrieverGridMap	it;
	it= _RetrieverGridMap.find(retrieverIdentifier);
	// If bad ident
	if(it==_RetrieverGridMap.end())
		return false;
	CRetrieverLightGrid		&rlg= it->second;
	// if bad surfaceId
	if(surfaceId<0 || surfaceId>= (sint)rlg.Grids.size())
		return false;

	// Else, ok, get it.
	rlg.Grids[surfaceId].getStaticLightSetup(localPos, pointLightList, sunContribution, *this, localAmbient);
	return true;
}




} // NL3D
