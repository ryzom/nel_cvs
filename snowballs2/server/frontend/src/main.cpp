/*
 * This file conatin the Snowballs FrontEnd Service.
 *
 * $Id: main.cpp,v 1.1 2001/07/19 15:26:41 valignat Exp $
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


// This include is mandatory to use NeL. It include NeL types.
#include <nel/misc/types_nl.h>

// We're using the NeL Service framework and layer 4.
#include <nel/net/service.h>
#include <nel/net/net_manager.h>
#include <nel/net/login_server.h>


using namespace NLNET;
using namespace std;

//TSockId clientfrom;



/****************************************************************************
 * cbChatClient
 *
 * Receive chat messages from a client and send it to the Chat Service.
 ****************************************************************************/
void cbChatClient ( CMessage& msgin, TSockId from, CCallbackNetBase& clientcb )
{
	string message;

	// Input from the client is stored.
	msgin.serial( message );

	// Prepare the message to send to the CHAT service
	CMessage msgout( CNetManager::getSIDA( "CHAT" ), "CHAT" );
	msgout.serial( message );

	/*
	 * The incomming message from the client is sent to the CHAT service
	 * under the "CHAT" identification.
	 */
	CNetManager::send( "CHAT", msgout );

	nlinfo( "Received CHAT message \"%s\" from \"%s\"",
			message.c_str(),
			clientcb.hostAddress(from).asString().c_str() );
}


/****************************************************************************
 * cdChatService
 *
 * Receive chat messages from the Chat Service to send it to all the clients.
 ****************************************************************************/
void cbChatService ( CMessage& msgin, TSockId from, CCallbackNetBase& servercb )
{
	string  message;

	// Input: process the reply of the ping service
	msgin.serial( message );

	// Output: send the reply to the client
	CMessage msgout( CNetManager::getSIDA( "FS" ), "CHAT" );
	msgout.serial( message );
	CNetManager::send( "FS", msgout, 0 );

	nlinfo( "Sent chat message \"%s\" to \"%s\"",
			message.c_str(),
			servercb.hostAddress(from).asString().c_str() );
}


/****************************************************************************
 * ClientCallbackArray
 *
 * It define the functions to call when receiving a specific message from the
 * client
 ****************************************************************************/
TCallbackItem ClientCallbackArray[] =
{
	{ "CHAT", cbChatClient }
};


/****************************************************************************
 * ChatCallbackArray
 *
 * It define the functions to call when receiving a specific message from a
 * shard service
 ****************************************************************************/
TCallbackItem ChatCallbackArray[] =
{
	{ "CHAT", cbChatService }
};


/****************************************************************************
 * Connection callback for the Chat service
 ****************************************************************************/
void onReconnectChat ( const std::string &serviceName, TSockId from, void *arg )
{
	uint32 i;
	string message;

	nlinfo( "Chat Service reconnected" );
}


/****************************************************************************
 * Disonnection callback for the Chat service
 ****************************************************************************/
void onDisconnectChat ( const std::string &serviceName, TSockId from, void *arg )
{
	/* Note: messages already forwarded should get no reply, but it may occur
	 * (e.g. if the server reconnects before the forwarding of a message and
	 * the reconnection callbacks is called after that). Then onReconnectChat()
	 * may send messagess that have already been sent and the front-end may get
	 * the same message twice. This is partially handled in cbChatService.
	 */

	nlinfo( "Chat Service disconnecting: messages will be delayed until reconnection" );
}


/****************************************************************************
 * Connection callback for a client
 ****************************************************************************/
void onConnectionClient (TSockId from, const CLoginCookie &cookie)
{
	nlinfo ("The client with uniq Id %d is connected", cookie.getUserId() );

	// store the user id in appId
	from->setAppId (cookie.getUserId() );
}


/****************************************************************************
 * Disonnection callback for a client
 ****************************************************************************/
void onDisconnectClient (const std::string &serviceName, TSockId from, void *arg)
{
	nlinfo ("A client with uniq Id %d has disconnected", from->appId ());

	// tell the login system that this client is disconnected
	CLoginServer::clientDisconnected ((uint32) from->appId ());
}

/****************************************************************************
 * CFrontEndService
 *
 * Class 
 ****************************************************************************/
class CFrontEndService : public IService
{
public:

	// Initialisation
	void init()
	{
		// connect the front end login system
		CLoginServer::init (*getServer (), onConnectionClient); 

		// Set the callbacks for the client disconnection of the Frontend
		CNetManager::setDisconnectionCallback ("FS", onDisconnectClient, NULL);

		/*
		 * Connect (as a client) to the Chat Service (as a server)
		 */
		CNetManager::addClient( "CHAT" );

		// Set the callbacks for that connection (comming form the Chat service)
		CNetManager::addCallbackArray( "CHAT",
									   ChatCallbackArray,
									   sizeof(ChatCallbackArray)/sizeof(ChatCallbackArray[0]) );

		/*
		 * Set the callback function when the Chat service reconnect to the
		 * frontend
		 */
		CNetManager::setConnectionCallback( "CHAT", onReconnectChat, NULL );

		/*
		 * Set the callback function when the Chat service disconnect from
		 * frontend
		 */
		CNetManager::setDisconnectionCallback( "CHAT", onDisconnectChat, NULL );
	}
};


/****************************************************************************   
 * SNOWBALLS FRONTEND SERVICE MAIN Function
 *
 * This call create a main function for a service:
 *
 *    - based on the "CFrontEndService" class
 *    - having the short name "FS"
 *    - having the long name "frontend_service"
 *    - listening on the port "37000"
 *    - and callback actions set to "ClientCallbackArray"
 *
 ****************************************************************************/
NLNET_SERVICE_MAIN( CFrontEndService,
					"FS",
					"frontend_service",
					37000,
					ClientCallbackArray )


/* end of file */
