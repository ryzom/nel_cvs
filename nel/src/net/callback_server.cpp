/** \file callback_server.cpp
 * Network engine, layer 4, server
 *
 * $Id: callback_server.cpp,v 1.4 2001/02/23 15:44:30 cado Exp $
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

#include "nel/net/callback_server.h"
#include "nel/net/msg_socket.h"


namespace NLNET {


CCallbackServer *CCallbackServer::_TheServer = NULL;


/*
 * cbProcessDisconnectionCallback
 */
void cbProcessConnectionCallback( CMessage& msg, TSockId id )
{
	// Map TSockId -> the server object (CCallbackNetBase*)
	CCallbackServer::_SockIdMap.insert( std::make_pair(id,CCallbackServer::_TheServer) ); 

	if ( CCallbackServer::_TheServer->_ConnectionCallback != NULL )
	{
		CCallbackServer::_TheServer->_ConnectionCallback( id );
	}
}


/*
 * DisconnectionCallbackArray
 */
TCallbackItem ConnectionCallbackArray [] =
{
	{ "C", cbProcessConnectionCallback }
};


/*
 * Constructor
 */
CCallbackServer::CCallbackServer() :
	_ConnectionCallback( NULL )
{
	nlassert( CCallbackServer::_TheServer == NULL );
	CCallbackServer::_TheServer = this; // Only one server object in this implementation

	// Setup incoming connection handling
	addCallbackArray( ConnectionCallbackArray, sizeof(ConnectionCallbackArray) / sizeof(TCallbackItem) );
}


/*
 * Listens on the specified port
 */
void CCallbackServer::init( uint16 port )
{
	_MsgSocket = new CMsgSocket( _CallbackArray, _CbArraySize, port );
}


/*
 * Disconnect the specified host
 */
void CCallbackServer::disconnect( TSockId hostid )
{
	CMsgSocket::close( hostid );
}


/*
 * Send a message to the specified host
 */
void CCallbackServer::send( CMessage& outmsg, TSockId hostid )
{
	CMsgSocket::send( outmsg, hostid );
}


/*
 * Returns the internet address of the listening socket
 */
const CInetAddress *CCallbackServer::listenAddress()
{
	return CMsgSocket::listenAddress();
}


} // NLNET
