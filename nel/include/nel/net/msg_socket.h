/* server_socket.h
 *
 * Copyright, 2000 Nevrax Ltd.
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

/*
 * $Id: msg_socket.h,v 1.2 2000/09/19 09:14:03 cado Exp $
 *
 * Interface for CServerSocket
 */

#ifndef NL_SERVER_SOCKET_H
#define NL_SERVER_SOCKET_H

#include "nel/net/socket.h"
#include <vector>


namespace NLNET
{

class CInetAddress;


/**
 * A server socket object allows to listen on a port and to accept an incoming connection. It returns
 * then a CSocket object.
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CServerSocket
{
public:

	/// Constructor
	CServerSocket();

	/// Destructor. It closes all sockets (connections) that have been created by this CServerSocket object
	~CServerSocket();

	/// Prepares to receive connections on a specified port
	void		listen( uint16 port ) throw (ESocket);

	/// Prepares to receive connections on a specified address/port (useful when the host has several addresses)
	void		listen( const CInetAddress& addr ) throw (ESocket);

	/// Wait for a client to connect, and returns a socket connected to the client
	CSocket&	accept() throw (ESocket);

private:

	SOCKET					_ServSock;
	bool					_Binded;
	std::vector<CSocket*>	_Connections;

};

}

#endif // NL_SERVER_SOCKET_H

/* End of server_socket.h */
