/*
 * This file contain the Snowballs Chat Service.
 *
 * $Id: main.cpp,v 1.7 2002/10/10 17:51:46 lecroart Exp $
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

// We're using the NeL Service framework, and layer 5
#include <nel/net/service.h>


using namespace std;
using namespace NLMISC;
using namespace NLNET;

/****************************************************************************
 * Function:   cbChat
 *             Callback function called when the Chat Service receive a "CHAT"
 *             message
 ****************************************************************************/
void cbChat (CMessage &msgin, const std::string &serviceName, uint16 sid)
{
	string message;

	// Extract the incomming message content from the Frontend and print it
	msgin.serial( message );
	nldebug( "SB: Received CHAT line: \"%s\"", message.c_str() );

	// Prepare to send back the message.
	CMessage msgout( "CHAT" );
	msgout.serial( message );

	/*
	 * Send the message to all the connected Frontend.
	 */
	CUnifiedNetwork::getInstance ()->send( "FS", msgout );

	nldebug( "SB: Sent to every front end service CHAT line: \"%s\"", message.c_str() );
}


/****************************************************************************
 * CallbackArray
 *
 * It define the functions to call when receiving a specific message
 ****************************************************************************/
TUnifiedCallbackItem CallbackArray[] =
{
	{ "CHAT", cbChat }
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
NLNET_SERVICE_MAIN( IService, "CHAT", "chat_service", 0, CallbackArray, SNOWBALLS_CONFIG, SNOWBALLS_LOGS )


/* end of file */
