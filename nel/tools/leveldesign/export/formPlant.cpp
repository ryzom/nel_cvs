/** \file formVegetable.cpp
 * Construction with a CItem
 *
 * $Id: formPlant.cpp,v 1.1 2002/02/05 10:03:22 besson Exp $
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

#include "formPlant.h"

#include "nel/misc/common.h"

#include "../georges_lib/item.h"
#include "../georges_lib/itemElt.h"

using namespace std;
using namespace NLMISC;

// ---------------------------------------------------------------------------
void SFormPlant::build (CItem &item)
{
	CItemElt *pElt = item.GetElt ("Name");
	if (pElt != NULL)
		Name = pElt->GetCurrentResult();

	pElt = item.GetElt ("Shape");
	if (pElt != NULL)
		Shape = pElt->GetCurrentResult();

	pElt = item.GetElt ("Shadow");
	if (pElt != NULL)
		Shadow = pElt->GetCurrentResult();

	pElt = item.GetElt ("Collision_radius");
	if (pElt != NULL)
		CollisionRadius = (float)atof(pElt->GetCurrentResult().c_str());

	pElt = item.GetElt ("Bunding_radius");
	if (pElt != NULL)
		BundingRadius = (float)atof(pElt->GetCurrentResult().c_str());
}
