/* base_socket.cpp
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
 * $Id: base_socket.cpp,v 1.2 2000/09/21 14:12:10 cado Exp $
 *
 * Implementation of CBaseSocket
 */

#include "nel/net/base_socket.h"


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
			throw ESocket("Winsock initialization failed");
		}
#endif
		CBaseSocket::_Initialized = true;
	}
}



/*
 * Constructor
 */
CBaseSocket::CBaseSocket() :
	_Sock( INVALID_SOCKET )
{
	CBaseSocket::init();
}


/*
 * Construct a CSocket object using an already connected socket 
 */
CBaseSocket::CBaseSocket( SOCKET sock ) throw (ESocket) :
	_Sock( sock )
{
	CBaseSocket::init();
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
			closesocket( _Sock );
		#elif defined NL_OS_LINUX
			::close( _Sock );
		#endif
	_Log.display( "Socket %d closed at %s/%hu\n", _Sock, _LocalAddr.ipAddress().c_str(), _LocalAddr.port() );
	_Sock = INVALID_SOCKET;
	}
}


/*
 * Checks if there are some data to receive
 */
bool CBaseSocket::dataAvailable() throw (ESocket)
{
	fd_set fdset;
	fdset.fd_count = 1; // FD_ZERO
	fdset.fd_array[0] = _Sock; //FD_SET
	timeval tv = {0,0};

	// Test for message received. Note: 1st argument of select ignored on Windows, I don't know on Unix.
	// libsock++ says _Sock+1
	int res;
	res = select( 0, &fdset, NULL, NULL, &tv );
	if ( res > 0 ) {
		return true;
	}
	else
	{
		if ( res == -1 )
		{
			throw ESocket("dataAvailable(): select failed");
		}
	}
	return false;
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
		ESocket( "Unable to find local address" );
	}
	_LocalAddr.setSockAddr( (const sockaddr_in *)&saddr );
}



} // NLNET
