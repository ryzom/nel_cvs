/** \file callback_client.cpp
 * Network engine, layer 4, client
 *
 * $Id: callback_client.cpp,v 1.3 2001/02/23 09:48:38 cado Exp $
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

#include "nel/net/callback_client.h"
#include "nel/net/msg_socket.h"


namespace NLNET {


/*
 * Constructor
 */
CCallbackClient::CCallbackClient()
{
}


/*
 * Connects to the specified host
 */
void CCallbackClient::connect( const CInetAddress& addr )
{
	// Connect
	_MsgSocket = new CMsgSocket( _CallbackArray, _CbArraySize, addr );

	// Map TSockId -> this (CCallbackNetBase*)
	CCallbackNetBase::_SockIdMap.insert( std::make_pair(_MsgSocket->id(),this) ); 
}


/*
 * Disconnect the remote host
 */
void CCallbackClient::disconnect()
{
	nlassert( _MsgSocket != NULL );
	CMsgSocket::close( _MsgSocket->id() );
}


/*
 * Send a message to the remote host
 */
void CCallbackClient::send( CMessage& outmsg )
{
	nlassert( _MsgSocket != NULL );
	_MsgSocket->send( outmsg );
}





} // NLNET
