/** \file network.h
 * Network interface between the game and NeL
 *
 * $Id: network.h,v 1.7 2001/07/19 17:30:39 lecroart Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX SNOWBALLS.
 * NEVRAX SNOWBALLS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX SNOWBALLS is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX SNOWBALLS; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef NETWORK_H
#define NETWORK_H

//
// Includes
//

#include <string>

#include "entities.h"

//
// External definitions
//

namespace NLNET
{
	class CCallbackClient;
}

//
// External variables
//

// Pointer to the connection to the server
extern NLNET::CCallbackClient	*Connection;

//
// External functions
//

// Return true if the client is online
bool	isOnline ();

// Send a chat line to the server
void	sendChatLine (std::string Line);

// Send the user entity position to the server
void	sendEntityPos (CEntity &entity);

void	initNetwork ();
void	updateNetwork ();
void	releaseNetwork ();

#endif // NETWORK_H

/* End of network.h */
