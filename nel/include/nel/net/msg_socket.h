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
 * $Id: msg_socket.h,v 1.5 2000/09/25 11:14:23 cado Exp $
 *
 * Interface for CServerSocket
 */

#ifndef NL_SERVER_SOCKET_H
#define NL_SERVER_SOCKET_H

#include "nel/net/base_socket.h"
#include "nel/net/socket.h"
#include <vector>


namespace NLNET
{

class CInetAddress;
class CMessage;


/**
 * Type of callbacks for processing incoming messages, passed to CServerSocket::receive().
 * The callback function must process a connection/disconnection (if message==NULL), or a received input message (otherwise).
 * When message!=NULL, it does not need to deallocate it.
 * When the socket is closed, it will be deleted by the caller of the callback.
 *
 * The first argument is a pointer to the caller object.
 */

typedef void (*TCbProcessReceivedMsg)( void*, CSocket&, CMessage*);



/**
 * A server socket object allows to listen on a port and to accept an incoming connection. It returns
 * then a CSocket object.
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CServerSocket : public CBaseSocket
{
public:

	/// Constructor.
	CServerSocket();

	/// Destructor. It closes all sockets (connections) that have been created by this CServerSocket object
	~CServerSocket();

	/// Prepares to receive connections on a specified port
	void		listen( uint16 port ) throw (ESocket);

	/// Prepares to receive connections on a specified address/port (useful when the host has several addresses)
	void		listen( const CInetAddress& addr ) throw (ESocket);

	/** Tests if a client requests/closes connection or a message is received from a connected client.
	 *
	 * \li If a new client requests a connection, the server calls accept() (i.e. it creates a new client socket, which
	 * is added to the list of connections). It then calls the callback function. Its argument "message" is NULL.
	 * \li If a connected client closes connection, it calls the callback function. Its argument "message" is NULL.
	 * Then the client socket is removed from the list of connections and deleted.
	 * \li If a message is received from a connected client, it puts it in the input message that is passed,
	 * as a pointer, in argument of the callback function. The callback function needs not delete it.
	 *
	 * \param caller A pointer to the caller of receive (use "this"). It is passed to the callback function.
	 * \param cbProcessReceivedMsg Callback function to provide. Example:
	 *
	 * \code 
		void cbProcessReceivedMsg( void* caller, CSocket& sock, CMessage *message )
		{
			if ( message == NULL )
			{
				if ( sock.connected() )
				{
					// Process connection request
				}
				else
				{
					// Some stuff in reaction to disconnection of the client
				}
			}
			else
			{
				// Process (*message) that was received
			}
		}
		\encode
	 */
	void		receive( void* caller, TCbProcessReceivedMsg cbProcessReceivedMsg );

	/** Wait for a client to connect, then creates a new socket connected to the client, and adds it to the list of connections.
	 * It returns a reference on this socket object, which is maintained by the CServerSocket object.
	 * Usage : \code CSocket& sock = servsock.accept(); \endcode
	 * If you don't want the server thread to block, use receive() instead.
	 */
	CSocket&	accept() throw (ESocket);

	/** Number of seconds to wait in receive(). The higher, the nicer for the speed of the rest of the system.
	 */
	static long	NiceLevel;

protected:

	/** Returns if the listening socket of the server and the connection sockets have incoming data available.
	 * \param ringing [out] True if the listening socket has data (e.g. a connection request)
	 * \param ringing [out] Vector of bool telling which connections have incoming data.
	 * You don't need to initialize "available".
	 */
	bool getDataAvailableStatus( bool& ringing, std::vector<bool>& available );

private:

	bool					_Binded;
	std::vector<CSocket*>	_Connections;

};

}

#endif // NL_SERVER_SOCKET_H

/* End of server_socket.h */
