/** \file base_socket.cpp
 * CBaseSocket class
 *
 * $Id: base_socket.cpp,v 1.17 2000/11/14 15:58:34 cado Exp $
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

#include "nel/net/base_socket.h"
#include "nel/misc/debug.h"


#ifdef NL_OS_WINDOWS

#include <winsock2.h>
#define ERROR_NUM WSAGetLastError()
#define socklen_t int

#elif defined NL_OS_UNIX

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
//#include <fcntl.h>
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define ERROR_NUM errno
typedef int SOCKET;

#endif


namespace NLNET {


bool CBaseSocket::_Initialized = false;


/* Initializes the network engine if it is not already done (under Windows, calls WSAStartup()).
 * Called by CSocket constructors.
 */
void CBaseSocket::init() throw (ESocket)
{
	if ( ! CBaseSocket::_Initialized )
	{
#ifdef WIN32
		WORD winsock_version = MAKEWORD( 2, 0 ); 
		WSADATA wsaData;
		int err = WSAStartup(winsock_version, &wsaData);
		if ( err != 0 )
		{
			throw ESocket( "Winsock initialization failed", ERROR_NUM );
		}
#endif
		CBaseSocket::_Initialized = true;
	}
}



/*
 * Constructor
 */
CBaseSocket::CBaseSocket( bool reliable, bool logging ) :
	_Sock( INVALID_SOCKET ),
	_Reliable( reliable ),
	_Logging( logging ),
	_Connected( false ),
	_BytesReceived( 0 ),
	_BytesSent( 0 )

{
	CBaseSocket::init();

	// Socket creation
	if ( reliable )
	{
		_Sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ); // or IPPROTO_IP (=0) ?
	}
	else
	{
		_Sock = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP ); // or IPPROTO_IP (=0) ?
	}
	if ( _Sock == INVALID_SOCKET )
	{
		throw ESocket("socket creation failed");
	}
	if ( _Logging )
	{
		nldebug( "Socket %d open (%s)", _Sock, _Reliable?"TCP":"UDP" );
	}
}


/*
 * Construct a CSocket object using an already connected reliable socket 
 */
CBaseSocket::CBaseSocket( SOCKET sock, const CInetAddress& remoteaddr ) throw (ESocket) :
	_Sock( sock ),
	_Reliable( true ),
	_Connected( true ),
	_Logging( true ),
	_RemoteAddr( remoteaddr ),
	_BytesReceived( 0 ),
	_BytesSent( 0 )
{
	CBaseSocket::init();

	// Check remote address
	if ( ! _RemoteAddr.isValid() )
	{
		throw ESocket( "Could not init a socket object with an invalid address" );
	}
	// Get local socket name
	setLocalAddress();
}

/*
 * Destructor
 */
CBaseSocket::~CBaseSocket()
{
	close();
}


/*
 * Closure
 */
void CBaseSocket::close()
{
	if ( _Sock != INVALID_SOCKET )
	{
#ifdef NL_OS_WINDOWS
		shutdown( _Sock, SD_BOTH );
		closesocket( _Sock );
#elif defined NL_OS_UNIX
		shutdown( _Sock, SHUT_RDWR );
		::close( _Sock );
#endif
		if ( _Logging )
		{
			nldebug( "Socket %d closed at %s", _Sock, _LocalAddr.asIPString().c_str() );
		}
		_Sock = INVALID_SOCKET;
		_Bound = false;
		_Connected = false;
	}
}



/*
 * Disables a connected socket (sends/receives will do nothing)
 */
void CBaseSocket::disable()
{
	_Connected = false;
}


/*
 * Sets/unsets TCP_NODELAY
 */
void CBaseSocket::setNoDelay( bool value ) throw (ESocket)
{
	if ( ! _Reliable )
	{
		throw ESocket("Cannot setNoDelay on an unreliable socket");
	}
		
	if ( setsockopt( _Sock, IPPROTO_TCP, TCP_NODELAY, (char*)&value, sizeof(value) ) != 0 )
	{
		throw ESocket( "setNoDelay failed. ", ERROR_NUM );
	}
}


/*
 * Connection
 */
void CBaseSocket::connect( const CInetAddress& addr ) throw (ESocketConnectionFailed,ESocket)
{
	// Check address
	if ( ! addr.isValid() )
	{
		throw ESocket( "Unable to connect to invalid address" );
	}

	// Connection (when _Sock is a datagram socket, connect establishes a default destination address)
	if ( ::connect( _Sock, (const sockaddr *)(addr.sockAddr()), sizeof(sockaddr_in) ) != 0 )
	{
		throw ESocketConnectionFailed( ERROR_NUM );
	}
	if ( _Logging )
	{
		nldebug( "Socket %d connected to %s", _Sock, addr.asIPString().c_str() );
	}
	setLocalAddress();
	if ( _Logging )
	{
		nldebug( "Socket %d is at %s", _Sock, _LocalAddr.asIPString().c_str() );
	}	
	_RemoteAddr = addr;
	_Connected = _Reliable;
	_Bound = !_Reliable;
}


/*
 * Checks if there are some data to receive
 */
bool CBaseSocket::dataAvailable() throw (ESocket)
{
	fd_set fdset;
	FD_ZERO( &fdset );
	FD_SET( _Sock, &fdset );
	timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	// Test for message received.
	int res = select( _Sock+1, &fdset, NULL, NULL, &tv );
	switch ( res  )
	{
		case  0 : return false;
		case -1 : throw ESocket( "dataAvailable(): select failed", ERROR_NUM ); return false;
	}
	return true;
}


/*
 * Sets the local address
 */
void CBaseSocket::setLocalAddress()
{
	sockaddr saddr;
	int saddrlen = sizeof(saddr);
	if ( getsockname( _Sock, &saddr, &saddrlen ) != 0 )
	{
		ESocket( "Unable to find local address", ERROR_NUM );
	}
	_LocalAddr.setSockAddr( (const sockaddr_in *)&saddr );
}


/** Binds the socket to the specified port. Call bind() if the host acts as a server and waits for
 * messages. If the host acts as a client, call sendTo(), there is no need to bind the socket.
 */
void CBaseSocket::bind( uint16 port ) throw (ESocket)
{
	// Get local socket name
	const uint MAXLENGTH = 80;
	char localhost [MAXLENGTH];
	if ( gethostname( localhost, MAXLENGTH ) != 0 )
	{
		throw ESocket("Unabled to get local hostname");
	}
	_LocalAddr.setByName( localhost );
	_LocalAddr.setPort( port );

	// Bind the socket
	if ( ::bind( _Sock, (sockaddr*)(_LocalAddr.sockAddr()), sizeof(sockaddr) ) == SOCKET_ERROR )
	{
#ifdef NL_OS_WINDOWS
		switch ( WSAGetLastError() ) {
			case WSAEADDRINUSE : throw ESocket("Bind failed : address in use");
			case WSAEADDRNOTAVAIL : throw ESocket("Bind failed : address not available");
			default : throw ESocket("Bind failed");
		}
#elif defined NL_OS_UNIX
		throw ESocket(strerror(errno));
#endif
	}
	_Bound = true;
	if ( _Logging )
	{
		nldebug( "Socket %d bound at %s", _Sock, _LocalAddr.asIPString().c_str() );
	}
}


/*
 * Sends a message
 */
void CBaseSocket::sendTo( const uint8 *buffer, uint len, const CInetAddress& addr ) throw (ESocket)
{
	//  Send
	if ( ::sendto( _Sock, (const char*)buffer, len, 0, (sockaddr*)(addr.sockAddr()), sizeof(sockaddr) ) != (sint32)len )
	{
		throw ESocket( "Unable to send datagram", ERROR_NUM );
	}
	_BytesSent += len;

	if ( _Logging )
	{
		nldebug( "Socket %d sent %d bytes to %s", _Sock, len, addr.asIPString().c_str() );
	}

	// If socket is unbound, retrieve local address
	if ( ! _Bound )
	{
		setLocalAddress();
		_Bound = true;
	}
}


/*
 * Receives data (returns false if !dataAvailable()).
 */
bool CBaseSocket::receivedFrom( uint8 *buffer, uint len, CInetAddress& addr ) throw (ESocket)
{
	if ( ! ( _Bound && dataAvailable() ) )
	{
		return false;
	}

	// Receive incoming message
	sockaddr_in saddr;
	int saddrlen = sizeof(saddr);
	int brecvd = ::recvfrom( _Sock, (char*)buffer, len , 0, (sockaddr*)&saddr, &saddrlen );
	if ( brecvd == SOCKET_ERROR )
	{
		throw ESocket( "Cannot receive data", ERROR_NUM );
	}

	// Get sender's address
	addr.setSockAddr( &saddr );

	_BytesReceived += len;
	if ( _Logging )
	{
		nldebug( "Socket %d received %d bytes from %s", _Sock, len, addr.asIPString().c_str() );
	}

	return true;
}


/*
 * Sends a message
 */
void CBaseSocket::send( const uint8* buffer, uint len ) throw (ESocket)
{
	if ( ::send( _Sock, (const char*)buffer, len, 0 ) == SOCKET_ERROR )
	{
		throw ESocket( "Unable to send data", ERROR_NUM );
	}
	_BytesSent += len;
	
	if ( _Logging )
	{
		nldebug( "Socket %d sent %d bytes", _Sock, len );
	}
}




/*
 * Receives data, or blocks if !dataAvailable()). Returns false if !connected().
 */
bool CBaseSocket::receive( uint8 *buffer, uint len ) throw (ESocket)
{
	if ( (! _Connected) && _Reliable )
	{
		return false;
	}

	// Receive incoming message
	doReceive( buffer, len );

	return true;
}


/*
 * Receives data (returns false if !dataAvailable() and does not block).
 */
bool CBaseSocket::received( uint8 *buffer, uint len ) throw (ESocket)
{
	if ( ( (! _Connected) && _Reliable ) && (! dataAvailable()) )
	{
		return false;
	}

	// Receive incoming message
	doReceive( buffer, len );

	return true;
}


/*
 * Receives data
 */
void CBaseSocket::doReceive( uint8 *buffer, uint len )
{
	uint total = 0;
	uint brecvd;
	while ( total < len )
	{
		brecvd = ::recv( _Sock, (char*)(buffer+total), len-total, 0 );
		switch ( brecvd )
		{
			case 0 :			throw ESocketConnectionClosed();
			case SOCKET_ERROR :	throw ESocket( "Unable to receive data", ERROR_NUM );
		}
		total += brecvd;
	}
	/*if ( _Logging )
	{
		nldebug( "Socket %d received %d bytes", _Sock, len );
	}*/
	_BytesReceived += len;
}




} // NLNET
