/* server_socket.cpp
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: msg_socket.cpp,v 1.1 2000/09/19 08:28:41 cado Exp $
 *
 * Implementation of CServerSocket.
 * Thanks to Daniel Bellen <huck@pool.informatik.rwth-aachen.de> for libsock++,
 * from which I took some ideas
 */

#include "nel/net/server_socket.h"


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

	
/*
 * Constructor
 */
CServerSocket::CServerSocket()
: _ServSock( 0 ), _Binded( false )
{
	CSocket::init();
}


/*
 * Destructor. It closes all sockets (connections) that have been created by this CServerSocket object
 */
CServerSocket::~CServerSocket()
{
	if ( _Binded )
	{
		#ifdef NL_OS_WINDOWS
			closesocket( _ServSock );
		#elif defined NL_OS_LINUX
			::close( _ServSock );
		#endif
	}
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

	// Create a socket
	_ServSock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ); // IPPROTO_TCP or IPPROTO_IP (=0) ?
	if ( _ServSock == INVALID_SOCKET )
	{
		throw ESocket("Server socket creation failed");
	}

	// Bind socket to port	
	if ( ::bind( _ServSock, (const sockaddr *)addr.sockAddr(), sizeof(sockaddr_in) ) != 0 )
	{
		throw ESocket("Unable to bind server socket to port");
	}
	_Binded = true;

	// Retrieve socket error code (is this really necessary ?)
	int errcode=0;
	int errlen=sizeof(errcode);
	getsockopt( _ServSock, SOL_SOCKET, SO_ERROR, (char*)&errcode, &errlen );
	if ( errcode != 0 )
	{
		throw ESocket("Server socket raised an error after binding");
	}

	// Listen
	const int backlog = 5; // maximum length of the queue of pending connections
	if ( ::listen( _ServSock, backlog ) != 0 )
	{
		throw ESocket("Unable to listen on specified port");
	}
}


/*
 * Wait for a client to connect, and returns a socket connected to the client
 */
CSocket& CServerSocket::accept() throw (ESocket)
{
	// Accept connection
	sockaddr_in saddr;
	sint saddrlen = sizeof(saddr);
	SOCKET newsock = ::accept( _ServSock, (sockaddr*)&saddr, &saddrlen );
	if ( newsock == INVALID_SOCKET )
	{
		throw ESocket( "accept return an invalid socket");
	}

	// Construct and save a CSocket object
	CInetAddress addr;
	addr.setSockAddr( &saddr );
	CSocket *connection = new CSocket( newsock, addr );
	_Connections.push_back( connection );
	return *connection;
}


}