/** \file callback_server.h
 * Network engine, layer 4, server
 *
 * $Id: callback_server.h,v 1.3 2001/02/23 09:48:19 cado Exp $
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

#ifndef NL_CALLBACK_SERVER_H
#define NL_CALLBACK_SERVER_H

#include "nel/misc/types_nl.h"
#include "nel/net/callback_net_base.h"


namespace NLNET {


class CInetAddress;


/**
 * Server class for layer 4
 * Implementation: using CMsgSocket
 * Limitation: you can instanciate only 1 server object in this implementation
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CCallbackServer : public CCallbackNetBase
{
public:

	/// Constructor
	CCallbackServer();

	/// Listens on the specified port (call addCallbackArray() before)
	void	init( uint16 port );

	/// Disconnect the specified host
	void	disconnect( TSockId hostid );

	/// Send a message to the specified host
	void	send( CMessage& outmsg, TSockId hostid );

	/// Sets callback for incoming connections
	void	setConnectionCallback( TNetCallback cb );

	/// Returns the internet address of the listening socket
	const CInetAddress	*listenAddress();

	// Internal use
	friend void cbProcessConnectionCallback( CMessage& msg, TSockId id );

private:

	TNetCallback	_ConnectionCallback;

	static CCallbackServer *_TheServer;
};


} // NLNET


#endif // NL_CALLBACK_SERVER_H

/* End of callback_server.h */
