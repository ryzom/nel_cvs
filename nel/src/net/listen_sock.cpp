/** \file listen_sock.cpp
 * Network engine, layer 0, listening tcp socket
 *
 * $Id: listen_sock.cpp,v 1.8.2.1 2003/05/13 16:26:52 lecroart Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
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

#include "stdnet.h"

#include "nel/net/listen_sock.h"

#ifdef NL_OS_WINDOWS

#include <winsock2.h>
typedef sint socklen_t;

#elif defined NL_OS_UNIX

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
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
CListenSock::CListenSock() : CTcpSock(), _Bound( false )
{
	// Create socket
	createSocket( SOCK_STREAM, IPPROTO_TCP );

	/// \todo cado: tune backlog value, not too small, not to big (20-200) to prevent SYN attacks (see http://www.cyberport.com/~tangent/programming/winsock/advanced.html)
	setBacklog( -1 );
}


/*
 * Prepares to receive connections on a specified port
 */
void CListenSock::init( uint16 port )
{
    // Use any address
	CInetAddress localaddr; // By default, INETADDR_ANY (useful for gateways that have several ip addresses)
	localaddr.setPort( port );
	init( localaddr );

	// Now set the address visible from outside
	_LocalAddr = CInetAddress::localHost();
	_LocalAddr.setPort( port );
	nldebug( "LNETL0: Socket %d listen socket is at %s", _Sock, _LocalAddr.asString().c_str() );
}


/*
 * Prepares to receive connections on a specified address/port (useful when the host has several addresses)
 */
void CListenSock::init( const CInetAddress& addr )
{
	if ( ! addr.isValid() )
	{
		nldebug( "LNETL0: Binding listen socket to any address, port %hu", addr.port() );
	}

#ifndef NL_OS_WINDOWS
	// Set Reuse Address On (does not work on Win98 and is useless on Win2000)
	int value = true;
	if ( setsockopt( _Sock, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value) ) == SOCKET_ERROR )
	{
		throw ESocket( "ReuseAddr failed" );
	}
#endif

	// Bind socket to port	
	if ( ::bind( _Sock, (const sockaddr *)addr.sockAddr(), sizeof(sockaddr_in) ) != 0 )
	{
		throw ESocket( "Unable to bind listen socket to port" );
	}
	_LocalAddr = addr;
	_Bound = true;

	// Listen
	if ( ::listen( _Sock, _BackLog ) != 0 ) // SOMAXCONN = maximum length of the queue of pending connections
	{
		throw ESocket( "Unable to listen on specified port" );
	}
//	nldebug( "LNETL0: Socket %d listening at %s", _Sock, _LocalAddr.asString().c_str() );
}


/*
 * Accepts an incoming connection, and creates a new socket
 */
CTcpSock *CListenSock::accept()
{
	// Accept connection
	sockaddr_in saddr;
	socklen_t saddrlen = sizeof(saddr);
	SOCKET newsock = ::accept( _Sock, (sockaddr*)&saddr, &saddrlen );
	if ( newsock == INVALID_SOCKET )
	{
	  /*nlinfo( "LNETL0: Error accepting a connection");
	  // See accept() man on Linux
	  newsock = ::accept( _Sock, (sockaddr*)&saddr, &saddrlen );
	  if ( newsock == INVALID_SOCKET )*/
	    {
			throw ESocket( "Accept returned an invalid socket");
	    }
	}

	// Construct and save a CTcpSock object
	CInetAddress addr;
	addr.setSockAddr( &saddr );
	nldebug( "LNETL0: Socket %d accepted an incoming connection from %s, opening socket %d", _Sock, addr.asString().c_str(), newsock );
	CTcpSock *connection = new CTcpSock( newsock, addr );
	return connection;
}


/*
 * Sets the number of the pending connections queue. -1 for the maximum possible value.
 */
void CListenSock::setBacklog( sint backlog )
{
	if ( backlog == -1 )
	{
		_BackLog = SOMAXCONN; // SOMAXCONN = maximum length of the queue of pending connections
	}
	else
	{
		_BackLog = backlog;
	}
	if ( _Bound )
	{
		if ( ::listen( _Sock, _BackLog ) != 0 )
		{
			throw ESocket( "Unable to listen on specified port, while changing backlog" );
		}
	}
}


} // NLNET
