/** \file patch.cpp
 * <File description>
 *
 * $Id: patch.cpp,v 1.2 2000/10/23 14:08:30 berenguier Exp $
 */

/** Copyright, 2000 Nevrax Ltd.
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


#include "nel/3d/patch.h"
#include "nel/3d/tessellation.h"
#include "nel/3d/bezier_patch.h"
#include "nel/misc/vector.h"
using	namespace	std;
using	namespace	NLMISC;


namespace NL3D 
{


// ***************************************************************************
CBezierPatch	CPatch::CachePatch;
CPatch			*CPatch::LastPatch= NULL;


// ***************************************************************************
CPatch::CPatch()
{
}


// ***************************************************************************
CBezierPatch	*CPatch::unpackIntoCache()
{
	if(LastPatch!=this)
	{
		unpack(CachePatch);
		LastPatch=this;
	}
	return &CachePatch;
}
// ***************************************************************************
void			CPatch::unpack(CBezierPatch	&p)
{
	// TODODODODODODODODO
}

	}
}

} // NL3D

