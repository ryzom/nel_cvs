/* socket.cpp
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: socket.cpp,v 1.1 2000/09/14 16:17:21 lecroart Exp $
 *
 * Implementation for CSocket
 */

#include "nel/network/socket.h"
#include "nel/network/message.h"


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


namespace NLNET
{


bool CSocket::_Initialized = false;


/*
 * Constructor
 */
CSocket::CSocket()
: _Sock( 0 ),
  _Connected( false )
{
	if ( ! CSocket::_Initialized )
	{
#ifdef WIN32
		WORD winsock_version = MAKEWORD( 2, 0 ); 
		WSADATA wsaData;
		int err = WSAStartup(winsock_version, &wsaData);
		if ( err == 0 )
		{
			throw ESocket("Winsock initialization failed");
		}
#endif
		CSocket::_Initialized = true;
	}
}


/*
 * Destructor
 */
/*CSocket::~CSocket()
{
	//delete _PSock;
}
*/

/*
 * Connection
 */
void CSocket::connect( const CInetAddress& addr )
{
	// Check address
	if ( ! addr.isValid() )
	{
		throw ESocket("Unable to connect to invalid address");
	}

	// Socket creation
	_Sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ); // IPPROTO_TCP or IPPROTO_IP (=0) ?
	if ( _Sock == INVALID_SOCKET )
	{
		throw ESocket("Socket creation failed");
	}

	// Connection
	// log( "Trying TCP connection on " + addr.ipAddress() );
	if ( ::connect( _Sock, (const sockaddr *)(addr.sockAddr()), sizeof(sockaddr_in) ) != 0 )
	{
		throw ESocket("Connection failed");
	}

	// Get local socket name
	sockaddr saddr;
	int saddrlen = sizeof(saddr);
	if ( getsockname( _Sock, &saddr, &saddrlen ) != 0 )
	{
		// log( "Network error: getsockname() failed (CSocket::connect)" );
	}
	_LocalAddr.setSockAddr( (const sockaddr_in *)&saddr );
	_Connected = true;
}


/*
 * Closure
 */
void CSocket::close()
{
#ifdef NL_OS_WINDOWS
	closesocket( _Sock );
#elif defined NL_OS_LINUX
	::close( _Sock );
#endif
	_Connected = false;
}


/*
 * Sends a message
 */
void CSocket::send( const CMessage& message )
{
	// Prepare message header (note: cannot this method be optimized by sending all header in one block ?)
	sint16 msgtype;
	uint16 msgnamelen = message.msgName().length();
	if ( msgnamelen != 0 )
	{
		msgtype = msgnamelen | 0x8000; // bit15 is 1 when msgtype is msgnamelen
	}
	else
	{
		msgtype = message.msgType();
	}

	// 1. Write message type
	int bsent = ::send( _Sock, (char*)&msgtype, sizeof(msgtype), 0 );
	if ( bsent != sizeof(msgtype) )
	{
		throw "Unable to send msgtype";
	}

	// 2. Write message name (optional)
	bsent = ::send( _Sock, message.msgName().c_str(), msgnamelen, 0 );
	if ( bsent != msgnamelen )
	{
		throw "Unable to send msgname";
	}

	// 3. Write message size
	int msgsize = message.length();
	bsent = ::send( _Sock, (char*)&msgsize, sizeof(msgsize), 0 );
	if ( bsent != sizeof(msgsize) )
	{
		throw "Unable to send msgsize";
	}

	// 4. Write message payload
	bsent = ::send( _Sock, (const char*)message.buffer(), msgsize, 0 );
	if ( bsent != msgsize )
	{
		throw "Unable to send message";
	}
}


/*
 * Checks if there are some data to receive
 */
bool CSocket::dataAvailable()
{
	if ( ! _Connected )
	{
		return false;
	}

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
			throw ESocket("select failed");
		}
	}
	return false;
}


/*
 * Receives data (returns false if !dataAvailable()). The capacity of the message must be large enough.
 */
bool CSocket::receive( CMessage& message )
{
	if ( ! dataAvailable() )
	{
		return false;
	}

	// 1. Read message type
	sint16 msgtype;
	if ( ::recv( _Sock, (char*)&msgtype, sizeof(msgtype), 0 ) == SOCKET_ERROR )
	{
		throw ESocket("Cannot receive msgtype");
	}

	// 2. Read message name (optional)
	uint16 msgnamelen = 0;
	char *msgname = new char[msgnamelen+1];
	if ( msgtype < 0 )
	{
		msgnamelen = msgtype & 0x7FFF; // msgtype when bit 15 is 1 is length of msgname
		if ( ::recv( _Sock, msgname, msgnamelen, 0 ) == SOCKET_ERROR )
		{
			throw ESocket("Cannot receive msgname");
		}
		msgname[msgnamelen] = '\0';
	}
	// 3. Read message payload size
	uint32 msgsize;
	if ( ::recv( _Sock, (char*)&msgsize, sizeof(msgsize), 0 ) == SOCKET_ERROR )
	{
		throw ESocket("Cannot receive msgsize");
	}

	// 4. Read all buffer and dismiss
	message.setHeader( msgtype, std::string(msgname) );
	delete [] msgname;
	if ( ::recv( _Sock, (char*)(message.bufferToFill(msgsize)), msgsize, 0 ) == SOCKET_ERROR )
	{
		throw ESocket("Cannot receive message");
	}

	//message.setLengthFilled( totallen );
	return true;
}


}