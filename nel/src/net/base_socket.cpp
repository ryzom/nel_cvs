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
 * $Id: base_socket.cpp,v 1.7 2000/10/03 13:27:12 cado Exp $
 *
 * Implementation of CBaseSocket
 */

#include "nel/net/base_socket.h"
#include "nel/misc/log.h"

/// \todo cado Debug logging class instead
extern NLMISC::CLog Log;

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
CBaseSocket::CBaseSocket( bool logging ) :
	_Sock( INVALID_SOCKET ),
	_Logging( logging )
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
			shutdown( _Sock, SD_BOTH );
			closesocket( _Sock );
		#elif defined NL_OS_LINUX
			shutdown( _Sock, SHUT_RDWR ):
			::close( _Sock );
		#endif
	if ( _Logging )
	{
		Log.display( "Socket %d closed at %s\n", _Sock, _LocalAddr.asIPString().c_str() );
	}
	_Sock = INVALID_SOCKET;
	}
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
		case -1 : throw ESocket("dataAvailable(): select failed"); return false;
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
		ESocket( "Unable to find local address" );
	}
	_LocalAddr.setSockAddr( (const sockaddr_in *)&saddr );
}



} // NLNET
