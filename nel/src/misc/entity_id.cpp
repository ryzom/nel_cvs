/** \file win_displayer.cpp
 * <File description>
 *
 * $Id: entity_id.cpp,v 1.5 2002/03/06 17:48:10 chafik Exp $
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

#include "stdmisc.h"

#include "nel/misc/entity_id.h"

using namespace std;

namespace NLMISC {

const uint64 CEntityId::MaxEntityId = ((uint64)1 << (40 + 1)) - (uint64)1;
uint8 CEntityId::ServerId = 0;
const CEntityId CEntityId::Unknown;

} // NLMISC
