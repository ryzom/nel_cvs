/*
 * This file contain the Snowballs Frontend Service.
 *
 * $Id: main.cpp,v 1.9 2002/03/18 14:05:04 lecroart Exp $
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

#include <nel/misc/vector.h>

// We're using the NeL Service framework and layer 4.
#include <nel/net/service.h>
#include <nel/net/net_manager.h>
#include <nel/net/login_server.h>

#include <map>
#include <utility>

using namespace NLMISC;
using namespace NLNET;
using namespace std;


//TSockId clientfrom;

/*
 * Keep a list of the players connected to that Frontend. Only map Id
 * to a Connection
 */

struct _player
{
	_player(uint32 Id, TSockId Con) : id(Id), con(Con) { }
	uint32   id;
	TSockId  con;
};

typedef map<uint32, _player> _pmap;

_pmap localPlayers;


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

	nldebug( "SB: Received CHAT message \"%s\" from client \"%s\"",
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

	// Input: process the reply of the chat service
	msgin.serial( message );

	// Output: send the reply to the client
	CMessage msgout( CNetManager::getSIDA( "FS" ), "CHAT" );
	msgout.serial( message );

	// Send the message to all connected clients
	CNetManager::send( "FS", msgout, 0 );

	nldebug( "SB: Sent chat message \"%s\" to all clients", message.c_str());
}


/****************************************************************************
 * cbPosClient
 *
 * Receive position messages from a client and send it to the Position Service.
 ****************************************************************************/
void cbPosClient ( CMessage& msgin, TSockId from, CCallbackNetBase& clientcb )
{
	uint32  id;
	CVector pos;
	float   angle;
	uint32  state;

	// Input from the client is stored.
	msgin.serial( id );
	msgin.serial( pos );
	msgin.serial( angle );
	msgin.serial( state );

	// Prepare the message to send to the Position service
	CMessage msgout( CNetManager::getSIDA( "POS" ), "ENTITY_POS" );
	msgout.serial( id );
	msgout.serial( pos );
	msgout.serial( angle );
	msgout.serial( state );

	/*
	 * The incomming message from the client is sent to the Position service
	 * under the "POS" identification.
	 */
	CNetManager::send( "POS", msgout );

	//nldebug( "SB: Received ENTITY_POS from the client");
}


/****************************************************************************
 * cdPosService
 *
 * Receive position messages from the Position Service to send it to all the
 * clients.
 ****************************************************************************/
void cbPosService ( CMessage& msgin, TSockId from, CCallbackNetBase& servercb )
{
	uint32  id;
	CVector pos;
	float   angle;
	uint32  state;

	// Input: process the reply of the position service
	msgin.serial( id );
	msgin.serial( pos );
	msgin.serial( angle );
	msgin.serial( state );

	// Output: send the reply to the client
	CMessage msgout( CNetManager::getSIDA( "FS" ), "ENTITY_POS" );
	msgout.serial( id );
	msgout.serial( pos );
	msgout.serial( angle );
	msgout.serial( state );

	// Send the message to all connected clients
	CNetManager::send( "FS", msgout, 0 );

	//nldebug( "SB: Sent ENTITY_POS message to all the connected clients");
}


/****************************************************************************
 * cbAddClient
 *
 * Receive an ADD_ENTITY message from a client and send it to the Position
 * Service.
 ****************************************************************************/
void cbAddClient ( CMessage& msgin, TSockId from, CCallbackNetBase& clientcb )
{
	uint32  id;
	string  name;
	uint8   race;
	CVector start;

	// Input from the client is stored.
	msgin.serial( id );
	msgin.serial( name );
	msgin.serial( race );
	msgin.serial( start );

	// Prepare the message to send to the Position service
	CMessage msgout( CNetManager::getSIDA( "POS" ), "ADD_ENTITY" );
	msgout.serial( id );
	msgout.serial( name );
	msgout.serial( race );
	msgout.serial( start );

	/*
	 * The incomming message from the client is sent to the Position service
	 * under the "POS" identification.
	 */
	CNetManager::send( "POS", msgout );

	nldebug( "SB: Received ADD_ENTITY from the client");
}


/****************************************************************************
 * cdAddService
 *
 * Receive an ADD_ENTITY messages from the Position Service to send it to all
 * the clients.
 ****************************************************************************/
void cbAddService ( CMessage& msgin, TSockId from, CCallbackNetBase& servercb )
{
	bool    all;
	uint32  to;
	uint32  id;
	string  name;
	uint8   race;
	CVector start;

	// Input: process the reply of the position service
	msgin.serial( all );
	msgin.serial( to );
	msgin.serial( id );
	msgin.serial( name );
	msgin.serial( race );
	msgin.serial( start );

	// Output: prepare the reply to the clients
	CMessage msgout( CNetManager::getSIDA( "FS" ), "ADD_ENTITY" );
	msgout.serial( id );
	msgout.serial( name );
	msgout.serial( race );
	msgout.serial( start );

	if ( all == true )
	{
		// Send the message to all connected clients
		CNetManager::send( "FS", msgout, 0 );

		nldebug( "SB: Sent ADD_ENTITY message to all the connected clients");
	}
	else
	{
		// Send the message about a former connected client to the new client
		_pmap::iterator ItPlayer;
		ItPlayer = localPlayers.find(to);
		if ( ItPlayer == localPlayers.end() )
		{
			nlwarning( "New player id %u not found !", to );
		}
		else
		{
			TSockId conToClient = ((*ItPlayer).second).con;
			CNetManager::send( "FS", msgout, conToClient );

			nldebug( "SB: Sent ADD_ENTITY about all the connected clients to the new client.");
		}
	}

}


/****************************************************************************
 * cbRemoveClient
 *
 * Receive an REMOVE_ENTITY message from a client and send it to the Position
 * Service.
 ****************************************************************************/
void cbRemoveClient ( CMessage& msgin, TSockId from, CCallbackNetBase& clientcb )
{
	uint32  id;

	// Input from the client is stored.
	msgin.serial( id );

	// Prepare the message to send to the Position service
	CMessage msgout( CNetManager::getSIDA( "POS" ), "REMOVE_ENTITY" );
	msgout.serial( id );

	/*
	 * The incomming message from the client is sent to the Position service
	 * under the "POS" identification.
	 */
	CNetManager::send( "POS", msgout );

	nldebug( "SB: Received REMOVE_ENTITY from the client");
}


/****************************************************************************
 * cdRemoveService
 *
 * Receive an REMOVE_ENTITY messages from the Position Service to send it to all
 * the clients.
 ****************************************************************************/
void cbRemoveService ( CMessage& msgin, TSockId from, CCallbackNetBase& servercb )
{
	uint32  id;

	// Input: process the reply of the position service
	msgin.serial( id );

	// Output: send the reply to the client
	CMessage msgout( CNetManager::getSIDA( "FS" ), "REMOVE_ENTITY" );
	msgout.serial( id );

	// Send the message to all connected clients
	CNetManager::send( "FS", msgout, 0 );

	nldebug( "SB: Sent REMOVE_ENTITY message to all the connected clients");
}


/****************************************************************************
 * cdSnowballService
 *
 * Receive an SNOWBALL messages from the Position Service to send it to all
 * the clients.
 ****************************************************************************/
void cbSnowballService ( CMessage& msgin, TSockId from, CCallbackNetBase& servercb )
{
	uint32  id,
			playerId;
	CVector start,
			target;
	float   speed,
			explosionRadius;

	// Input: process the reply of the position service
	msgin.serial( id );
	msgin.serial( playerId );
	msgin.serial( start );
	msgin.serial( target );
	msgin.serial( speed );
	msgin.serial( explosionRadius );

	// Output: send the reply to the client
	CMessage msgout( CNetManager::getSIDA( "FS" ), "SNOWBALL" );
	msgout.serial( id );
	msgout.serial( playerId );
	msgout.serial( start );
	msgout.serial( target );
	msgout.serial( speed );
	msgout.serial( explosionRadius );

	// Send the message to all connected clients
	CNetManager::send( "FS", msgout, 0 );

	nldebug( "SB: Sent SNOWBALL message to all the connected clients");
}


/****************************************************************************
 * cbSnowballClient
 *
 * Receive an SNOWBALL message from a client and send it to the Position
 * Service.
 ****************************************************************************/
void cbSnowballClient ( CMessage& msgin, TSockId from, CCallbackNetBase& clientcb )
{
	uint32  playerId;
	CVector start,
			target;
	float   speed,
			explosionRadius;

	// Input from the client is stored.
	msgin.serial( playerId );
	msgin.serial( start );
	msgin.serial( target );
	msgin.serial( speed );
	msgin.serial( explosionRadius );

	// Prepare the message to send to the Position service
	CMessage msgout( CNetManager::getSIDA( "POS" ), "SNOWBALL" );
	msgout.serial( playerId );
	msgout.serial( start );
	msgout.serial( target );
	msgout.serial( speed );
	msgout.serial( explosionRadius );

	/*
	 * The incomming message from the client is sent to the Position service
	 * under the "POS" identification.
	 */
	CNetManager::send( "POS", msgout );

	nldebug( "SB: Received SNOWBALL from the client");
}


/****************************************************************************
 * cdHitService
 *
 * Receive an HIT messages from the Position Service to send it to all
 * the clients.
 ****************************************************************************/
void cbHitService ( CMessage& msgin, TSockId from, CCallbackNetBase& servercb )
{
	uint32  snowballId,
			victimId;
	bool    direct;

	// Input: process the reply of the position service
	msgin.serial( snowballId );
	msgin.serial( victimId );
	msgin.serial( direct );

	// Output: send the reply to the client
	CMessage msgout( CNetManager::getSIDA( "FS" ), "HIT" );
	msgout.serial( snowballId );
	msgout.serial( victimId );
	msgout.serial( direct );

	// Send the message to all connected clients
	CNetManager::send( "FS", msgout, 0 );

	nldebug( "SB: Sent HIT message to all the connected clients");
}


/****************************************************************************
 * ClientCallbackArray
 *
 * It define the functions to call when receiving a specific message from the
 * client
 ****************************************************************************/
TCallbackItem ClientCallbackArray[] =
{
	{ "ADD_ENTITY",    cbAddClient      },
	{ "ENTITY_POS",    cbPosClient      },
	{ "CHAT",          cbChatClient     },
	{ "REMOVE_ENTITY", cbRemoveClient   },
	{ "SNOWBALL",      cbSnowballClient }
};


/****************************************************************************
 * ChatCallbackArray
 *
 * It define the functions to call when receiving a specific message from
 * the Chat service
 ****************************************************************************/
TCallbackItem ChatCallbackArray[] =
{
	{ "CHAT", cbChatService }
};


/****************************************************************************
 * PosCallbackArray
 *
 * It define the functions to call when receiving a specific message from
 * the Position service
 ****************************************************************************/
TCallbackItem PosCallbackArray[] =
{
	{ "ADD_ENTITY",    cbAddService      },
	{ "ENTITY_POS",    cbPosService      },
	{ "REMOVE_ENTITY", cbRemoveService   },
	{ "SNOWBALL",      cbSnowballService },
	{ "HIT",           cbHitService }
};


/****************************************************************************
 * Connection callback for the Chat service
 ****************************************************************************/
void onReconnectChat ( const std::string &serviceName, TSockId from, void *arg )
{
	nldebug( "SB: Chat Service reconnected" );
}


/****************************************************************************
 * Disonnection callback for the Chat service
 ****************************************************************************/
void onDisconnectChat ( const std::string &serviceName, TSockId from, void *arg )
{
	/* Note: messages already forwarded should get no reply, but it may occure
	 * (e.g. if the server reconnects before the forwarding of a message and
	 * the reconnection callbacks is called after that). Then onReconnectChat()
	 * may send messagess that have already been sent and the front-end may get
	 * the same message twice. This is partially handled in cbChatService.
	 */

	nldebug( "SB: Chat Service disconnecting: messages will be delayed until reconnection" );
}


/****************************************************************************
 * Connection callback for the Position service
 ****************************************************************************/
void onReconnectPosition ( const std::string &serviceName, TSockId from, void *arg )
{
	nldebug( "SB: Position Service reconnected" );
}


/****************************************************************************
 * Disonnection callback for the Position service
 ****************************************************************************/
void onDisconnectPosition ( const std::string &serviceName, TSockId from, void *arg )
{
	/* Note: messages already forwarded should get no reply, but it may occure
	 * (e.g. if the server reconnects before the forwarding of a message and
	 * the reconnection callbacks is called after that). Then onReconnectChat()
	 * may send messagess that have already been sent and the front-end may get
	 * the same message twice. This is partially handled in cbPositionService.
	 */

	nldebug( "SB: Position Service disconnecting: messages will be delayed until reconnection" );
}


/****************************************************************************
 * Connection callback for a client
 ****************************************************************************/
void onConnectionClient (TSockId from, const CLoginCookie &cookie)
{
	uint32 id;

	id = cookie.getUserId();

	nlinfo( "The client with uniq Id %u is connected", id );

	// Add new client to the list of player managed by this FrontEnd
	pair<_pmap::iterator, bool>
		player = localPlayers.insert( _pmap::value_type( id,
														 _player( id,
																  from )));
	
	// store the player info in appId

	_pmap::iterator it = player.first;
	_player *p = &((*it).second);
	from->setAppId((uint64)(uint)p);

	// Output: send the IDENTIFICATION number to the new connected client
	CMessage msgout( CNetManager::getSIDA( "FS" ), "IDENTIFICATION" );
	msgout.serial( id );

	// Send the message to connected client "from"
	CNetManager::send( "FS", msgout, from );

	nldebug( "SB: Sent IDENTIFICATION message to the new client");
}


/****************************************************************************
 * Disonnection callback for a client
 ****************************************************************************/
void onDisconnectClient (const std::string &serviceName, TSockId from, void *arg)
{
	uint32 id;

	uint64 i = from->appId();
	_player *p = (_player *)(uint)i;
	id = p->id;

	nlinfo( "A client with uniq Id %u has disconnected", id );

	// tell the login system that this client is disconnected
	CLoginServer::clientDisconnected ( id );

	// remove the player from the local player list
	localPlayers.erase( id );

	// Output: send the REMOVE_ENTITY to the position manager.
	CMessage msgout( CNetManager::getSIDA( "POS" ), "REMOVE_ENTITY" );
	msgout.serial( id );

	// Send the message to the position manager
	CNetManager::send( "POS", msgout);

	nldebug( "SB: Sent REMOVE_ENTITY message to the position manager.");
}


/****************************************************************************
 * CFrontEndService
 ****************************************************************************/
class CFrontEndService : public IService
{
public:

	// Initialisation
	void init()
	{
		DebugLog->addNegativeFilter ("NETL");
		DebugLog->addNegativeFilter ("SB:");

		// Connect the frontend to the login system
		CLoginServer::init( *getServer(), onConnectionClient, &ConfigFile); 

		/********************************************************************
		 * Client connection management
		 */

		// Set the callbacks for the client disconnection of the Frontend
		CNetManager::setDisconnectionCallback( "FS", onDisconnectClient, NULL );

		/********************************************************************
		 * Chat Service connection management
		 */

		 // Connect (as a client) to the Chat Service (as a server)
		CNetManager::addClient( "CHAT" );

		// Set the callbacks for that connection (comming from the Chat service)
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

		/********************************************************************
		 * Position Service connection management
		 */

		// Connect (as a client) to the Position Service (as a server)
		CNetManager::addClient( "POS" );

		/*
		 * Set the callbacks for that connection (comming from the Position
		 * service)
		 */
		CNetManager::addCallbackArray( "POS",
									   PosCallbackArray,
									   sizeof(PosCallbackArray)/sizeof(PosCallbackArray[0]) );

		/*
		 * Set the callback function when the Position service reconnect to the
		 * frontend
		 */
		CNetManager::setConnectionCallback( "POS", onReconnectPosition, NULL );

		/*
		 * Set the callback function when the Position service disconnect from
		 * frontend
		 */
		CNetManager::setDisconnectionCallback( "POS", onDisconnectPosition, NULL );

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
