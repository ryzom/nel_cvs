/** \file play_list_user.cpp
 * <File description>
 *
 * $Id: play_list_user.cpp,v 1.5 2002/02/28 12:59:50 besson Exp $
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

#include "3d/play_list_user.h"
#include "3d/animation_set_user.h"

#include "nel/misc/debug.h"

using namespace NLMISC;

namespace NL3D {

// ***************************************************************************

TAnimationTime CPlayListUser::getLocalTime (uint8 slot, TGlobalAnimationTime globalTime, const UAnimationSet& animSet) const
{
	// Cast
	const CAnimationSetUser *cAnimSetUser=safe_cast<const CAnimationSetUser*>(&animSet);

	// Get the animationset pointer
	const CAnimationSet* cAnimSet=cAnimSetUser->getAnimationSet ();
	nlassert (cAnimSet);

	return _PlayList.getLocalTime (slot, globalTime, *cAnimSet);
}

// ***************************************************************************

float CPlayListUser::getLocalWeight (uint8 slot, TGlobalAnimationTime globalTime) const
{
	return _PlayList.getLocalWeight (slot, globalTime);
}

// ***************************************************************************

} // NL3D
