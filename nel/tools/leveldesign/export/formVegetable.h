/** \file formVegetable.h
 * The structure of a .vegetable in memory
 *
 * $Id: formVegetable.h,v 1.2 2002/01/16 15:26:49 besson Exp $
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

#ifndef LD_FORMVEGETABLE_H
#define LD_FORMVEGETABLE_H

// ---------------------------------------------------------------------------

#include "nel/misc/types_nl.h"

#include <vector>
#include <string>

class CItem;

// ---------------------------------------------------------------------------
struct SPlantInstance
{
	std::string		Name;
	float			Density;
	float			Falloff;
};

// ---------------------------------------------------------------------------
struct SFormVegetable
{
	std::vector<std::string>	IncludePatats;
	std::vector<std::string>	ExcludePatats;
	std::vector<SPlantInstance>	PlantInstances;
	float	JitterPos;
	float	ScaleMin;
	float	ScaleMax;
	bool	PutOnWater;
	float	WaterHeight;
	uint32	RandomSeed;

	void build (CItem &item);
};

#endif