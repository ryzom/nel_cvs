/** \file light_trav.cpp
 * <File description>
 *
 * $Id: light_trav.cpp,v 1.7 2002/02/28 12:59:49 besson Exp $
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
using namespace std;
using namespace NLMISC;


namespace	NL3D
{

using namespace	NLMISC;

// ***************************************************************************
CLightTrav::CLightTrav()
{
	_LightedList.reserve(1024);

	LightModelRoot= NULL;
	LightingSystemEnabled= false;
}

// ***************************************************************************
void	CLightTrav::setLightModelRoot(CRootModel *lightModelRoot)
{
	nlassert(lightModelRoot);
	LightModelRoot= lightModelRoot;
}


// ***************************************************************************
IObs		*CLightTrav::createDefaultObs() const
{
	return	new CDefaultLightObs;
}


// ***************************************************************************
void		CLightTrav::clearLightedList()
{
	_LightedList.clear();
}
// ***************************************************************************
void		CLightTrav::addLightedObs(IBaseLightObs *o)
{
	_LightedList.push_back(o);
}


// ***************************************************************************
void		CLightTrav::traverse()
{
	// If lighting System disabled, skip
	if(!LightingSystemEnabled)
		return;


	nlassert(LightModelRoot);

	// clear the quadGrid of dynamicLights
	LightingManager.clearDynamicLights();

	// for each lightModel, process her: recompute position, resetLightedModels(), and append to the quadGrid.
	LightModelRoot->getObs(LightTravId)->traverse(NULL);

	// for each visible lightable transform
	for(uint i=0; i<_LightedList.size(); i++ )
	{
		// traverse(), to recompute light contribution (if needed).
		_LightedList[i]->traverse(NULL);
	}

}



// ***************************************************************************
void		IBaseLightObs::init()
{
	IObs::init();
	nlassert( dynamic_cast<IBaseHrcObs*> (getObs(HrcTravId)) );
	HrcObs= static_cast<IBaseHrcObs*> (getObs(HrcTravId));
	nlassert( dynamic_cast<IBaseClipObs*> (getObs(ClipTravId)) );
	ClipObs= static_cast<IBaseClipObs*> (getObs(ClipTravId));
}




}
