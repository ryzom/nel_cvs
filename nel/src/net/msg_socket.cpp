/* server_socket.cpp
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
 * $Id: msg_socket.cpp,v 1.6 2000/09/25 11:14:23 cado Exp $
 *
 * Implementation of CServerSocket.
 * Thanks to Vianney Lecroart <lecroart@nevrax.com> and
 * Daniel Bellen <huck@pool.informatik.rwth-aachen.de> for ideas
 */

#include "nel/net/server_socket.h"
#include "nel/net/message.h"
#include "nel/misc/log.h"
extern NLMISC::CLog Log;

using namespace std;


#ifdef NL_OS_WINDOWS
	#include <winsock2.h>
#elif defined NL_OS_LINUX
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <errno.h>
	#include <fcntl.h>
	#define SOCKET_ERROR -1
	#define INVALID_SOCKET -1
	typedef int SOCKET;
#endif

using namespace std;


namespace NLNET
{


long CServerSocket::NiceLevel = 1;


/*
 * Constructor
 */
CServerSocket::CServerSocket() :
	CBaseSocket(),
	_Binded( false )
{}


/*
 * Destructor. It closes all sockets (connections) that have been created by this CServerSocket object
 */
CServerSocket::~CServerSocket()
{
	vector<CSocket*>::iterator its;
	for ( its=_Connections.begin(); its!=_Connections.end(); its++ )
	{
		delete *its;
	}
}


/*
 * Prepares to receive connections on a specified port
 */
void CServerSocket::listen( uint16 port ) throw (ESocket)
{
	CInetAddress localaddr = CInetAddress::localHost();
	localaddr.setPort( port );
	listen( localaddr ); // throw ESocket
}


/*
 * Prepares to receive connections on a specified address/port (useful when the host has several addresses)
 */
void CServerSocket::listen( const CInetAddress& addr ) throw (ESocket)
{
	if ( _Binded )
	{
		return;
	}
	if ( ! addr.isValid() )
	{
		throw ESocket("Invalid address for listening");
	}
	_LocalAddr = addr;

	// Create a socket
	_Sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ); // IPPROTO_TCP or IPPROTO_IP (=0) ?
	if ( _Sock == INVALID_SOCKET )
	{
		throw ESocket("Server socket creation failed");
	}
	Log.display( "Socket %d open as a server socket\n", _Sock );

	// Bind socket to port	
	if ( ::bind( _Sock, (const sockaddr *)addr.sockAddr(), sizeof(sockaddr_in) ) != 0 )
	{
		throw ESocket("Unable to bind server socket to port");
	}
	_Binded = true;

	// Retrieve socket error code (is this really necessary ?)
	int errcode=0;
	int errlen=sizeof(errcode);
	getsockopt( _Sock, SOL_SOCKET, SO_ERROR, (char*)&errcode, &errlen );
	if ( errcode != 0 )
	{
		throw ESocket("Server socket raised an error after binding");
	}

	// Listen
	if ( ::listen( _Sock, SOMAXCONN ) != 0 ) // SOMAXCONN = maximum length of the queue of pending connections
	{
		throw ESocket("Unable to listen on specified port");
	}
	Log.display( "Socket %d listening at %s/%hu\n", _Sock, _LocalAddr.ipAddress().c_str(), _LocalAddr.port() );
}


	/** Tests if a client requests/closes connection or a message is received from a connected client.
	 *
	 * \li If a new client requests a connection, the server calls accept() (i.e. it creates a new client socket, which
	 * is added to the list of connections). It then calls the callback function. Its argument "message" is NULL.
	 * \li If a connected client closes connection, it calls the callback function. Its argument "message" is NULL.
	 * Then the client socket is removed from the list of connections and deleted.
	 * \li If a message is received from a connected client, it puts it in the input message that is passed,
	 * as a pointer, in argument of the callback function. The callback function needs not delete it.
	 *
	 * \param cbProcessReceivedMsg Callback function to provide. Example: see header file.
	 */
void CServerSocket::receive( void* caller, TCbProcessReceivedMsg cbProcessReceivedMsg )
{
	// Check data available on all sockets, including the server socket
	vector<bool> available;
	bool ringing;
	if ( getDataAvailableStatus( ringing, available ) )
	{

		if ( ringing )
		{
			// Accept connection request
			cbProcessReceivedMsg( caller, accept(), NULL );
		}

		// Iterate on the sockets where data are available
		// PROBLEM: Erasing a member of the vector !
		uint32 i; // an iterator is not preferable here
		uint32 availsize = available.size();
		for ( i=0; i!=availsize; i++ )
		{
			if ( available[i] )
			{
				CSocket *sock = _Connections[i];
				try
				{
					// Receive message from a connected client
					CMessage msg( true );
					sock->receive( msg );
					cbProcessReceivedMsg( caller, *sock, &msg );
				}
				catch ( ESocket& )
				{
					// Handle a connection closure (gracefull or not), when
					// receive() has thrown an exception. Note: this could be done by boolean result
					sock->close();
					cbProcessReceivedMsg( caller, *sock, NULL );
					_Connections.erase( _Connections.begin() + i );
					delete sock;
					availsize--;
					i--; // ok, not very smart
				}
			}
		}
	}
}


/* Wait for a client to connect, then creates a new socket connected to the client, and adds it to the list of connections.
 * It returns a reference on this socket object, which is maintained by the CServerSocket object.
 * Usage : \code CSocket& sock = servsock.accept(); \endcode
 * If you don't want the server thread to block, use receive() instead.
 */
CSocket& CServerSocket::accept() throw (ESocket)
{
	// Accept connection
	sockaddr_in saddr;
	sint saddrlen = sizeof(saddr);
	SOCKET newsock = ::accept( _Sock, (sockaddr*)&saddr, &saddrlen );
	if ( newsock == INVALID_SOCKET )
	{
		throw ESocket( "accept return an invalid socket");
	}

	// Construct and save a CSocket object
	CInetAddress addr;
	addr.setSockAddr( &saddr );
	CSocket *connection = new CSocket( newsock, addr );
	_Connections.push_back( connection );
	Log.display( "Socket %d accepted an incoming connection from %s/%hu and opened socket %d\n", _Sock, addr.ipAddress().c_str(), addr.port(), newsock );
	return *connection;
}


/* Returns if the listening socket of the server and the connection sockets have incoming data available.
 * \param ringing [out] True if the listening socket has data (e.g. a connection request)
 * \param ringing [out] Vector of bool telling which connections have incoming data.
 * You don't need to initialize "available".
 */
bool CServerSocket::getDataAvailableStatus( bool& ringing, std::vector<bool>& available )
{
	ringing = false;
	available.assign( _Connections.size(), false );

	// Put all socket descriptors in select list and find maximum descriptor number
	SOCKET descmax = _Sock;
	fd_set readers, writers;
	FD_ZERO (&readers);
	FD_ZERO (&writers);
	FD_SET ( _Sock, &readers );
	vector<CSocket*>::iterator itps;
	for ( itps=_Connections.begin(); itps!=_Connections.end(); itps++ )
	{
		FD_SET( (*itps)->descriptor(), &readers );
		if ( (*itps)->descriptor() > descmax )
		{
			descmax = (*itps)->descriptor();
		}
	}

	// Do select
	timeval tv;
	tv.tv_sec = CServerSocket::NiceLevel;
	tv.tv_usec = 0; ;
	int res = select( descmax+1, &readers, NULL, NULL, &tv );
	switch ( res  )
	{
		case  0 : return false;
		case -1 : throw ESocket("getDataAvailableStatus(): select failed"); return false;
	}
	
	// Get results
	ringing = (FD_ISSET( _Sock, &readers ) != 0);
	for ( itps = _Connections.begin(); itps!=_Connections.end(); itps++ )
	{
		if ( FD_ISSET( (*itps)->descriptor(), &readers ) !=0 )
		{
			available[itps-_Connections.begin()] = true;
		}
	}
	return true;
}


}