/** \file grid_traversal.cpp
 * Utility class for incremental grid traversal
 *
 * $Id: grid_traversal.cpp
 */

/* Copyright, 2000, 2006 Nevrax Ltd.
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


#include "stdmisc.h"
#include "nel/misc/grid_traversal.h"
#include "nel/misc/vector_2f.h"

namespace NLMISC
{

//********************************************************************************************************************************
void	CGridTraversal::startTraverse(const NLMISC::CVector2f &start, sint &nextX, sint &nextY)
{
	nextX = (sint) floorf(start.x);
	nextY = (sint) floorf(start.y);
}

//********************************************************************************************************************************
bool	CGridTraversal::traverse(const NLMISC::CVector2f &start, const NLMISC::CVector2f &dir, sint &x, sint &y)
{	
	if (dir.x > 0.f)
	{
		float lambdaX = (x + 1.f - start.x) / dir.x;
		if (dir.y > 0.f)
		{
			float lambdaY = (y + 1 - start.y) / dir.y;
			if (lambdaX < lambdaY)
			{
				if (lambdaX > 1.f) return false;
				++ x;
				return true;
			}
			else
			{
				if (lambdaY > 1.f) return false;
				++ y;
				return true;
			}
		}
		else if (dir.y < 0.f)
		{
			float lambdaY = (y - start.y) / dir.y;
			if (lambdaX < lambdaY)
			{
				if (lambdaX > 1.f) return false;
				++ x;
				return true;
			}
			else
			{
				if (lambdaY > 1.f) return false;
				-- y;
				return true;
			}
		}		
		++ x;
		return x <= (sint) floorf(start.x + dir.x);				
	}
	else if (dir.x < 0.f)
	{
		float lambdaX = (x - start.x) / dir.x;
		if (dir.y > 0.f)
		{
			float lambdaY = (y + 1.f - start.y) / dir.y;
			if (lambdaX < lambdaY)
			{
				if (lambdaX > 1.f) return false;
				-- x;
				return true;
			}
			else
			{
				if (lambdaY > 1.f) return false;
				++ y;
				return true;
			}
		}
		else if (dir.y < 0.f)
		{
			float lambdaY = (y - start.y) / dir.y;
			if (lambdaX < lambdaY)
			{
				if (lambdaX > 1.f) return false;
				-- x;
				return true;
			}
			else
			{
				if (lambdaY > 1.f) return false;
				-- y;
				return true;
			}
		}
		-- x;
		return x >= (sint) floorf(start.x + dir.x);
	}
	
	if (dir.y > 0.f)
	{
		++ y;
		return y <= (sint) floorf(start.y + dir.y);

	}
	else if (dir.y < 0.f)
	{
		-- y;
		return y >= (sint) floorf(start.y + dir.y);
	}
	return false;
}

} // NLMISC