/* server_socket.h
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: msg_socket.h,v 1.1 2000/09/19 08:28:41 cado Exp $
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
