/** \file tile_far_bank.cpp
 * <File description>
 *
 * $Id: tile_far_bank.cpp,v 1.1 2000/12/19 14:24:45 corvazier Exp $
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

#include "nel/3d/tile_far_bank.h"
#include "nel/misc/stream.h"

using namespace NLMISC;

namespace NL3D {


// ***************************************************************************
// ***************************************************************************
// CTileFarBank::CTileFar.
// ***************************************************************************
// ***************************************************************************

// ***************************************************************************
const sint CTileFarBank::CTileFar::_Version=0x0;
// ***************************************************************************
void CTileFarBank::CTileFar::serial(IStream &f) throw(NLMISC::EStream)
{
	// Serial version
	sint streamver = f.serialVersion(_Version);

	// Serial size
	f.serial (_Width);
	f.serial (_Height);

	// Serial pixels
	f.serialCont (_Pixels);
}
	
// ***************************************************************************
// ***************************************************************************
// CTileFarBank.
// ***************************************************************************
// ***************************************************************************

// ***************************************************************************
CTileFarBank::CTileFarBank()
{
}

// ***************************************************************************
const sint CTileFarBank::_Version=0x0;
// ***************************************************************************
void CTileFarBank::serial(IStream &f) throw(NLMISC::EStream)
{
	// Write/Check "FAR_BANK" in header of the stream
	f.serialCheck ((uint32)'_RAF');
	f.serialCheck ((uint32)'KNAB');

	// Serial version
	sint streamver = f.serialVersion(_Version);

	// Serial tiles
	f.serialCont (_TileVector);
}


} // NL3D
