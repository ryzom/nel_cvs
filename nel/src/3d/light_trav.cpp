/** \file light_trav.cpp
 * <File description>
 *
 * $Id: light_trav.cpp,v 1.11 2003/03/27 16:51:45 berenguier Exp $
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

#include "std3d.h"

#include "3d/light_trav.h"
#include "3d/hrc_trav.h"
#include "3d/clip_trav.h"
#include "3d/root_model.h"
#include "nel/misc/hierarchical_timer.h"
#include "3d/point_light_model.h"

using namespace std;
using namespace NLMISC;


namespace	NL3D
{

using namespace	NLMISC;

// ***************************************************************************
CLightTrav::CLightTrav()
{
	_LightedList.reserve(1024);

	LightingSystemEnabled= false;
}

// ***************************************************************************
void		CLightTrav::clearLightedList()
{
	_LightedList.clear();
}


// ***************************************************************************
void		CLightTrav::addPointLightModel(CPointLightModel *pl)
{
	_DynamicLightList.insert(pl, &pl->_PointLightNode);
}


// ***************************************************************************
void		CLightTrav::traverse()
{
	H_AUTO( NL3D_TravLight );

	uint i;


	// If lighting System disabled, skip
	if(!LightingSystemEnabled)
		return;


	// clear the quadGrid of dynamicLights
	LightingManager.clearDynamicLights();

	// for each lightModel, process her: recompute position, resetLightedModels(), and append to the quadGrid.
	CPointLightModel	**pLight= _DynamicLightList.begin();
	uint	numPls= _DynamicLightList.size();
	for(;numPls>0;numPls--, pLight++)
	{
		(*pLight)->traverseLight(NULL);
	}

	// for each visible lightable transform
	for(i=0; i<_LightedList.size(); i++ )
	{
		// traverse(), to recompute light contribution (if needed).
		_LightedList[i]->traverseLight(NULL);
	}

}


}
