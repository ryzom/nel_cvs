/*
 * This file contain the Snowballs Chat Service.
 *
 * $Id: main.cpp,v 1.6 2002/03/25 10:16:32 lecroart Exp $
 */

/*
 * Copyright, 2000 - 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX SNOWBALLS.
 * NEVRAX SNOWBALLS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * NEVRAX SNOWBALLS is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with NEVRAX SNOWBALLS; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#ifndef SNOWBALLS_CONFIG
#define SNOWBALLS_CONFIG ""
#endif // SNOWBALLS_CONFIG

#ifndef SNOWBALLS_LOGS
#define SNOWBALLS_LOGS ""
#endif // SNOWBALLS_LOGS

// This include is mandatory to use NeL. It include NeL types.
#include <nel/misc/types_nl.h>

#include <string>

#include <nel/misc/debug.h>

// We're using the NeL Service framework, and layer 4
#include <nel/net/service.h>


using namespace std;
using namespace NLMISC;
using namespace NLNET;

/****************************************************************************
 * Function:   cbChat
 *             Callback function called when the Chat Service receive a "CHAT"
 *             message
 * Arguments:
 *             - msgin:  the incoming message
 *             - from:   the "sockid" of the sender (usually useless for a
 *                       CCallbackClient)
 *             - server: the CCallbackNetBase object (which really is a
 *                       CCallbackServer object, for a server)
 ****************************************************************************/
void cbChat ( CMessage& msgin, TSockId from, CCallbackNetBase& server )
{
	string message;

	// Extract the incomming message content from the Frontend and print it
	msgin.serial( message );
	nldebug( "SB: Received CHAT line: \"%s\"", message.c_str() );

	// Prepare to send back the message.
	CMessage msgout( CNetManager::getSIDA( "CHAT" ), "CHAT" );
	msgout.serial( message );

	/*
	 * Send the message to all the connected Frontend. If we decide to send
	 * it back to the sender, that last argument should be 'from' inteed of '0'
	 */
	CNetManager::send( "CHAT", msgout, 0 );

	nldebug( "SB: Send CHAT line: \"%s\"", message.c_str() );
}


/****************************************************************************
 * CallbackArray
 *
 * It define the functions to call when receiving a specific message
 ****************************************************************************/
TCallbackItem CallbackArray[] =
{
	{ "CHAT", cbChat }
};

/****************************************************************************
 * CChatService
 ****************************************************************************/
class CChatService : public IService
{
public:

	// Initialisation
	void init()
	{
		DebugLog->addNegativeFilter ("NETL");
		DebugLog->addNegativeFilter ("SB:");
	}
};


/****************************************************************************
 * SNOWBALLS CHAT SERVICE MAIN Function
 *
 * This call create a main function for the CHAT service:
 *
 *    - based on the base service class "IService", no need to inherit from it
 *    - having the short name "CHAT"
 *    - having the long name "chat_service"
 *    - listening on an automatically allocated port (0) by the naming service
 *    - and callback actions set to "CallbackArray"
 *
 ****************************************************************************/
NLNET_OLD_SERVICE_MAIN( CChatService,
					"CHAT",
					"chat_service",
					0,
					CallbackArray,
					SNOWBALLS_CONFIG,
					SNOWBALLS_LOGS )


/* end of file */
