/* socket.cpp
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
 * $Id: socket.cpp,v 1.7 2000/09/21 14:12:10 cado Exp $
 *
 * Implementation for CSocket.
 * Thanks to Daniel Bellen <huck@pool.informatik.rwth-aachen.de> for libsock++,
 * from which I took some ideas
 */

#include "nel/net/socket.h"
#include "nel/net/message.h"

/*#include <iostream> //debug
using namespace std;*/

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


/*
 * Constructor
 */
CSocket::CSocket() :
	CBaseSocket(),
	_Connected( false )
{
}


/*
 * Construct a CSocket object using an already connected socket and its associated address
 */
CSocket::CSocket( SOCKET sock, const CInetAddress& remoteaddr ) throw (ESocket) :
	CBaseSocket( sock ),
	_Connected( false ),
	_RemoteAddr( remoteaddr )
{
	// Check remote address
	if ( ! _RemoteAddr.isValid() )
	{
		throw ESocket( "Could not init a socket object with an invalid address" );
	}

	// Get local socket name
	setLocalAddress();

	_Connected = true;
}


/*
 * Closure
 */
void CSocket::close()
{
	_Connected = false;
	CBaseSocket::close();
}


/*
 * Sets/unsets TCP_NODELAY
 */
void CSocket::setNoDelay( bool value ) throw (ESocket)
{
	if ( setsockopt( _Sock, IPPROTO_TCP, TCP_NODELAY, (char*)&value, sizeof(value) ) != 0 )
	{
		throw ESocket( "Setsockopt(TCP_NODELAY) failed. " );
	}
}


/*
 * Connection
 */
void CSocket::connect( const CInetAddress& addr ) throw (ESocket)
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
	_Log.display( "Socket %d open\n", _Sock );

	// Connection
	// log( "Trying TCP connection on " + addr.ipAddress() );
	if ( ::connect( _Sock, (const sockaddr *)(addr.sockAddr()), sizeof(sockaddr_in) ) != 0 )
	{
		throw ESocket("Connection failed");
	}
	_Log.display( "Socket %d connected to %s/%hu\n", _Sock, addr.ipAddress().c_str(), addr.port() );

	// Get local socket name
	sockaddr saddr;
	int saddrlen = sizeof(saddr);
	if ( getsockname( _Sock, &saddr, &saddrlen ) != 0 )
	{
		// log( "Network error: getsockname() failed (CSocket::connect)" );
	}
	_LocalAddr.setSockAddr( (const sockaddr_in *)&saddr );
	_Log.display( "Socket %d is at %s/%hu\n", _Sock, _LocalAddr.ipAddress().c_str(), _LocalAddr.port() );
	_RemoteAddr = addr;
	_Connected = true;
}


/*
 * Sends a message
 */
void CSocket::send( const CMessage& message ) throw(ESocket)
{
	CMessage alldata = message.encode();

	if ( ::send( _Sock, (const char*)alldata.buffer(), alldata.length(), 0) == SOCKET_ERROR )
	{
		throw ESocket("Unable to send message");
	}
	_Log.display( "Socket %d sent %d bytes\n", _Sock, alldata.length() );

	/*// Old code

	// 1. Write message type
	sint16 msgtype = message.encodedMsgType();
	NLMISC_BSWAP16(msgtype);
	int bsent = ::send( _Sock, (char*)&msgtype, sizeof(msgtype), 0 );
	if ( bsent == SOCKET_ERROR )
	{
		throw ESocket("Unable to send msgtype");
	}

	// 2. Write message name (optional)
	uint16 msgnamelen = message.msgName().length();
	if ( msgnamelen != 0 )
	{
		bsent = ::send( _Sock, message.msgName().c_str(), msgnamelen, 0 );
		if ( bsent == SOCKET_ERROR )
		{
			throw ESocket("Unable to send msgname");
		}
	}

	// 3. Write message size
	int msgsize = message.length();
	int enmsgsize = msgsize;
	NLMISC_BSWAP32(enmsgsize);
	bsent = ::send( _Sock, (char*)&enmsgsize, sizeof(enmsgsize), 0 );
	if ( bsent == SOCKET_ERROR )
	{
		throw ESocket("Unable to send msgsize");
	}

	// 4. Write message payload
	bsent = ::send( _Sock, (const char*)message.buffer(), msgsize, 0 );
	if ( bsent == SOCKET_ERROR )
	{
		throw ESocket("Unable to send message");
	}*/
}


/*
 * Checks if there are some data to receive
 */
bool CSocket::dataAvailable() throw (ESocket)
{
	if ( ! _Connected )
	{
		return false;
	}
	return CBaseSocket::dataAvailable();
}


/*
 * Receives data (returns false if !dataAvailable()).
 */
bool CSocket::receive( CMessage& message ) throw (ESocket)
{
	if ( ! dataAvailable() )
	{
		return false;
	}

	// Receive incoming message (this is not done by CMessage::decode())

	// 1. Read message type
	sint16 msgtype;
	if ( ::recv( _Sock, (char*)&msgtype, sizeof(msgtype), 0 ) == SOCKET_ERROR )
	{
		throw ESocket("Cannot receive msgtype");
	}
	NLMISC_BSWAP16(msgtype);
	//cout << msgtype << " ";

	// 2. Read message name (optional)
	uint16 msgnamelen = 0;
	char *msgname = NULL;
	if ( CMessage::decodeLenInMsgType( msgtype, &msgnamelen ) )
	{
		msgname = new char[msgnamelen+1];
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
	NLMISC_BSWAP32(msgsize);
	//cout << (int)msgsize << endl;

	// 4. Read all buffer and dismiss
	message.setHeader( msgtype, std::string( msgname!=NULL ? msgname : "" ) );
	delete [] msgname;
	if ( ::recv( _Sock, (char*)(message.bufferToFill(msgsize)), msgsize, 0 ) == SOCKET_ERROR )
	{
		throw ESocket("Cannot receive message");
	}
	_Log.display( "Socket %d received %d bytes\n", _Sock, sizeof(msgtype)+msgnamelen+sizeof(msgsize)+message.length() );

	return true;
}


}