/** \file anim_detail_trav.cpp
 * <File description>
 *
 * $Id: anim_detail_trav.cpp,v 1.4 2001/08/23 10:13:13 berenguier Exp $
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

#include "3d/anim_detail_trav.h"
#include "3d/hrc_trav.h"
#include "3d/clip_trav.h"


namespace NL3D 
{


// ***************************************************************************
IObs				*CAnimDetailTrav::createDefaultObs() const
{
	return new CDefaultAnimDetailObs;
}


// ***************************************************************************
void				CAnimDetailTrav::traverse()
{
	// Inc the date.
	CurrentDate++;

	// Traverse all nodes of the visibility list.
	uint	nObs= _ClipTrav->numVisibleObs();
	for(uint i=0; i<nObs; i++)
	{
		IBaseClipObs	*clipObs= _ClipTrav->getVisibleObs(i);
		clipObs->AnimDetailObs->traverse(NULL);
	}
}


// ***************************************************************************
void	IBaseAnimDetailObs::init()
{
	IObs::init();
	nlassert( dynamic_cast<IBaseHrcObs*> (getObs(HrcTravId)) );
	HrcObs= static_cast<IBaseHrcObs*> (getObs(HrcTravId));
	nlassert( dynamic_cast<IBaseClipObs*> (getObs(ClipTravId)) );
	ClipObs= static_cast<IBaseClipObs*> (getObs(ClipTravId));
}



} // NL3D
