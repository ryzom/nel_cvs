/** \file point_light_named_array.cpp
 * <File description>
 *
 * $Id: point_light_named_array.cpp,v 1.3 2002/02/28 12:59:50 besson Exp $
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

#include "std3d.h"

#include "3d/point_light_named_array.h"
#include <algorithm>


using namespace std;
using namespace NLMISC;

namespace NL3D {


// ***************************************************************************
CPointLightNamedArray::CPointLightNamedArray()
{
}


// ***************************************************************************
struct	CPointLightNamedSort
{
	const CPointLightNamed	*PointLight;
	uint					SrcId;

	bool	operator<(const CPointLightNamedSort &b) const
	{
		return PointLight->LightGroupName < b.PointLight->LightGroupName;
	}
};


// ***************************************************************************
void			CPointLightNamedArray::clear()
{
	_PointLights.clear();
	_PointLightGroupMap.clear();
}


// ***************************************************************************
void			CPointLightNamedArray::build(const std::vector<CPointLightNamed> &pointLights, std::vector<uint> &indexRemap)
{
	uint	i;

	// sort by name.
	//----------
	// Fill Sort array
	vector<CPointLightNamedSort>	pointLightSorts;
	pointLightSorts.resize(pointLights.size());
	for(i=0; i<pointLightSorts.size(); i++)
	{
		pointLightSorts[i].PointLight= &pointLights[i];
		pointLightSorts[i].SrcId= i;
	}
	// sort
	sort(pointLightSorts.begin(), pointLightSorts.end());
	// Copy data, and Fill indexRemap array
	_PointLights.resize(pointLights.size());
	indexRemap.resize(pointLights.size());
	for(i=0; i<pointLightSorts.size(); i++)
	{
		// Copy yhe PointLight to its new destination
		_PointLights[i]= *pointLightSorts[i].PointLight;
		// set the new index at the old position.
		indexRemap[pointLightSorts[i].SrcId]= i;
	}

	// Regroup.
	// ---------
	_PointLightGroupMap.clear();
	if(_PointLights.size() > 0 )
	{
		bool	first= true;
		string	precName;
		// for all sorted pointLights
		uint i;
		for(i=0;i<_PointLights.size();i++)
		{
			const	std::string &curName= _PointLights[i].LightGroupName;
			if(first || precName!=curName )
			{
				// End last group
				if(first)
					first= false;
				else
					_PointLightGroupMap[precName].EndId= i;

				// Start new group
				_PointLightGroupMap[curName].StartId= i;
				precName= curName;
			}
		}
		// End last group.
		_PointLightGroupMap[precName].EndId= i;
	}
}

// ***************************************************************************
void			CPointLightNamedArray::setPointLightFactor(const std::string &lightGroupName, NLMISC::CRGBA nFactor)
{
	// Search in the map.
	ItPLGMap	it= _PointLightGroupMap.find(lightGroupName);
	if( it != _PointLightGroupMap.end() )
	{
		// Found the group. what entries in the array?
		uint	startId= it->second.StartId;
		uint	endId= it->second.EndId;
		nlassert(endId<=_PointLights.size());

		// for all entries, setLightFactor
		for(uint i=startId;i<endId;i++)
		{
			_PointLights[i].setLightFactor(nFactor);
		}
	}
}


// ***************************************************************************
void			CPointLightNamedArray::serial(NLMISC::IStream &f)
{
	uint	ver= f.serialVersion(0);

	f.serialCont(_PointLights);
	f.serialCont(_PointLightGroupMap);
}



} // NL3D
