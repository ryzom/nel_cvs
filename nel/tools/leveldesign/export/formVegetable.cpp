/** \file formVegetable.cpp
 * Construction with a CItem
 *
 * $Id: formVegetable.cpp,v 1.1 2001/12/28 14:47:59 besson Exp $
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

#include "formVegetable.h"

#include "nel/misc/common.h"

#include "../georges_lib/item.h"
#include "../georges_lib/itemElt.h"

using namespace std;
using namespace NLMISC;

// ---------------------------------------------------------------------------
void SFormVegetable::build (CItem &item)
{
	uint32 i;

	IncludePatats.clear ();
	ExcludePatats.clear ();
	PlantInstances.clear ();

	// Read the Include_patats field
	CItemElt *pElt = item.GetElt ("Include_patats");
	if (pElt != NULL)
	{
		i = 0;
		string diese = "#" + toString (i);
		CItemElt *pTmpElt = pElt->GetElt (diese);
		while (pTmpElt != NULL)
		{
			IncludePatats.push_back (pTmpElt->GetCurrentResult());
			++i;
			diese = "#" + toString (i);
			pTmpElt = pElt->GetElt (diese);
		}
	}

	// Read the Include_patats field
	pElt = item.GetElt ("Exclude_patats");
	if (pElt != NULL)
	{
		i = 0;
		string diese = "#" + toString (i);
		CItemElt *pTmpElt = pElt->GetElt (diese);
		while (pTmpElt != NULL)
		{
			ExcludePatats.push_back (pTmpElt->GetCurrentResult());
			++i;
			diese = "#" + toString (i);
			pTmpElt = pElt->GetElt (diese);
		}
	}
	
	// Read the Plants field
	pElt = item.GetElt ("Plants");
	if (pElt != NULL)
	{
		i = 0;
		string diese = "#" + toString (i);
		CItemElt *pTmpElt = pElt->GetElt (diese);
		while (pTmpElt != NULL)
		{
			CItemElt* pielt = pTmpElt->GetElt( "File name" );
			SPlantInstance piTmp;
			piTmp.Name = pielt->GetCurrentResult();
			pielt = pTmpElt->GetElt( "Density" );
			string s1 = pielt->GetCurrentResult();
			piTmp.Density = (float)atof(s1.c_str());
			pielt = pTmpElt->GetElt( "Falloff" );
			string s2 = pielt->GetCurrentResult();
			piTmp.Falloff = (float)atof(s2.c_str());
			PlantInstances.push_back (piTmp);
			++i;
			diese = "#" + toString (i);
			pTmpElt = pElt->GetElt (diese);
		}
	}
}
