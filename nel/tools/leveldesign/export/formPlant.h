/** \file formPlant.h
 * The structure of a .plant in memory
 *
 * $Id: formPlant.h,v 1.1 2002/02/05 10:03:22 besson Exp $
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

#ifndef LD_FORMPLANT_H
#define LD_FORMPLANT_H

// ---------------------------------------------------------------------------

#include <string>

class CItem;

// ---------------------------------------------------------------------------
struct SFormPlant
{
	std::string Name;
	std::string Shape;
	std::string Shadow;
	float		CollisionRadius;
	float		BundingRadius;

	void build (CItem &item);
};

#endif