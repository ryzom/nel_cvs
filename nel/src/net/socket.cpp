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
 * $Id: socket.cpp,v 1.11 2000/10/02 16:42:23 cado Exp $
 *
 * Implementation for CSocket.
 * Thanks to Daniel Bellen <huck@pool.informatik.rwth-aachen.de> for libsock++,
 * from which I took some ideas
 */

#include "nel/net/socket.h"
#include "nel/net/message.h"
#include "nel/misc/log.h"
extern NLMISC::CLog Log;

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
CSocket::CSocket( bool logging ) :
	CBaseSocket( logging ),
	_Connected( false ),
	_DataAvailable( false ),
	_SenderId( true ),
	_IsListening( false )
{
}


/*
 * Construct a CSocket object using an already connected socket and its associated address
 */
CSocket::CSocket( SOCKET sock, const CInetAddress& remoteaddr ) throw (ESocket) :
	CBaseSocket( sock ),
	_Connected( false ),
	_RemoteAddr( remoteaddr ),
	_IsListening( false )
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
	if ( _Logging )
	{
		Log.display( "Socket %d open\n", _Sock );
	}

	// Connection
	// log( "Trying TCP connection on " + addr.ipAddress() );
	if ( ::connect( _Sock, (const sockaddr *)(addr.sockAddr()), sizeof(sockaddr_in) ) != 0 )
	{
		throw ESocket("Connection failed");
	}
	if ( _Logging )
	{
		Log.display( "Socket %d connected to %s\n", _Sock, addr.asIPString() );
	}

	// Get local socket name
	sockaddr saddr;
	int saddrlen = sizeof(saddr);
	if ( getsockname( _Sock, &saddr, &saddrlen ) != 0 )
	{
		if ( _Logging ) {
			Log.display( "Network error: getsockname() failed (CSocket::connect)" );
		}
	}
	_LocalAddr.setSockAddr( (const sockaddr_in *)&saddr );
	if ( _Logging )
	{
		Log.display( "Socket %d is at %s\n", _Sock, _LocalAddr.asIPString().c_str() );
	}
	_RemoteAddr = addr;
	_Connected = true;
}


/*
 * Sends a message
 */
void CSocket::send( CMessage& message ) throw(ESocket)
{
	CMessage alldata = encode( message );

	if ( ::send( _Sock, (const char*)alldata.buffer(), alldata.length(), 0) == SOCKET_ERROR )
	{
		throw ESocket("Unable to send message");
	}
	if ( _Logging )
	{
		if ( message.typeIsNumber() )
		{
			Log.display( "Socket %d sent message %hd (%s) of %d bytes\n",
				_Sock, message.typeAsNumber(), message.typeAsString().c_str(), alldata.length() );
		}
		else
		{
			Log.display( "Socket %d sent message %s of %d bytes\n",
				_Sock, message.typeAsString().c_str(), alldata.length() );
		}
	}
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
	if ( _DataAvailable ) // true if a CServerSocket object has just tested positively the socket
	{
		return true;
	}
	return CBaseSocket::dataAvailable();
}


/*
 * Receives data, or blocks if !dataAvailable()). Returns false if !connected().
 */
bool CSocket::receive( CMessage& message ) throw (ESocket)
{
	if ( ! _Connected )
	{
		return false;
	}

	// Receive incoming message
	doReceive( message );

	return true;
}


/*
 * Receives data (returns false if !dataAvailable() and does not block).
 */
bool CSocket::received( CMessage& message ) throw (ESocket)
{
	if ( ! dataAvailable() )
	{
		return false;
	}

	// Receive incoming message
	doReceive( message );

	return true;
}


/*
 * Process an incoming bind message
 */
void CSocket::processBindMessage( CMessage& message )
{
	std::string key;
	TTypeNum num;
	message.serial( key );
	message.serial( num );
	_MsgMap.insert( TMsgMapItem(key,num) );
	if ( _Logging )
	{
		Log.display( "Socket %d : %s is now known as %hu for received messages\n", _Sock, key.c_str(), num );
	}
}


/*
 * Transforms a message replacing its string type by the corresponding num type if it is bound
 */
void CSocket::packMessage( CMessage& message )
{
	if ( ! message.typeIsNumber() )
	{
		CMsgMap::iterator im = _MsgMap.find( message.typeAsString() );
		if ( im != _MsgMap.end() )
		{
			message.setType( (*im).second );
		}
	}
}


/*
 * Returns an output message with header encoded in the payload buffer
 */
CMessage CSocket::encode( CMessage& msg )
{
	CMessage alldata( false, msg.length()+CMessage::maxHeaderLength() );

	// 1. Write message type
	packMessage( msg );
	TTypeNum code;
	TTypeNum namelen = msg.typeAsString().length();
	if ( msg.typeIsNumber() )
	{
		// Message type number
		code = msg.typeAsNumber();
	}
	else
	{
		// Encoded length of message type string
		code = namelen | 0x8000;
	}
	alldata.serial( code );

	// 2. Write message name (optional)
	if ( ! msg.typeIsNumber() )
	{
		alldata.serialBuffer( (uint8*)const_cast<char*>(msg.typeAsString().c_str()), namelen );
	}

	// 3. Write message size
	uint32 msgsize = msg.length();
	alldata.serial( msgsize );

	// 4. Write message payload
	alldata.serialBuffer( const_cast<uint8*>(msg.buffer()), msg.length() );

	return alldata;
}


/*
 * Helper method for receive() and received()
 */
void CSocket::doReceive( CMessage& message ) throw (ESocket)
{
	// 1. Read message type
	TTypeNum msgtype;
	uint32 brecvd;
	brecvd = ::recv( _Sock, (char*)&msgtype, sizeof(msgtype), 0 );
	switch ( brecvd )
	{
		case 0 :			throw ESocket("Connection closed"); // could be return false if function returned a boolean
		case SOCKET_ERROR :	throw ESocket("Cannot receive msgtype");
	}
	NLMISC_BSWAP16(msgtype);
	//cout << msgtype << " ";

	// 2. Read message name (optional)
	TTypeNum msgnamelen = 0;
	char *msgname = NULL;
	if ( msgtype < 0 )
	{
		msgnamelen = msgtype & 0x7FFF;
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

	// Set message type
	if ( msgtype < 0 )
	{
		message.setType( std::string(msgname) );
	}
	else
	{
		message.setType( msgtype );
	}
	if ( msgname != NULL )
	{
		delete [] msgname;
	}
	// 4. Read all buffer and dismiss
	if ( ::recv( _Sock, (char*)(message.bufferToFill(msgsize)), msgsize, 0 ) == SOCKET_ERROR )
	{
		throw ESocket("Cannot receive message");
	}
	if ( _Logging )
	{
		if ( message.typeIsNumber() )
		{
			Log.display( "Socket %d received message %hd of %d bytes\n",
				_Sock, message.typeAsNumber(), sizeof(msgtype)+msgnamelen+sizeof(msgsize)+message.length() );
		}
		else
		{
			Log.display( "Socket %d received message %s of %d bytes\n",
				_Sock, message.typeAsString().c_str(), sizeof(msgtype)+msgnamelen+sizeof(msgsize)+message.length() );
		}
	}
}


}