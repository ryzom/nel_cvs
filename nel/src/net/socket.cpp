/** \file socket.cpp
 * Implementation for CSocket.
 * Thanks to Daniel Bellen <huck@pool.informatik.rwth-aachen.de> for libsock++,
 * from which I took some ideas
 *
 * $Id: socket.cpp,v 1.38 2001/01/02 14:39:21 cado Exp $
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

#include "nel/net/socket.h"
#include "nel/net/message.h"
#include "nel/misc/debug.h"
#include "nel/misc/common.h"
#include "nel/net/net_log.h"


#ifdef NL_OS_WINDOWS

#include <winsock2.h>

#elif defined NL_OS_UNIX

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
CSocket::CSocket( bool reliable, bool logging ) :
	CBaseSocket( reliable, logging ),
	_DataAvailable( false ),
	_SenderId( 0 ),
	_IsListening( false ),
	_OwnerClient( NULL ),
	_AuthCallback( NULL ),
	_CurrentMsgNumberSend( 0 )
{
}


/*
 * Construct a CSocket object using an already connected socket and its associated address
 */
CSocket::CSocket( SOCKET sock, const CInetAddress& remoteaddr ) throw (ESocket) :
	CBaseSocket( sock, remoteaddr ),
	_DataAvailable( false ),
	_SenderId( 0 ),
	_IsListening( false ),
	_OwnerClient( NULL ),
	_AuthCallback( NULL ),
	_CurrentMsgNumberSend( 0 )
{
}


/*
 * Closure
 */
void CSocket::close()
{
	CBaseSocket::close();
}


/*
 * Sends a message
 */
void CSocket::send( CMessage& message ) throw(ESocket)
{
//#ifdef NL_DEBUG
	uint len = message.length();
	//std::string name = message.typeAsString();
//#endif
	CMessage alldata = encode( message );

	nlassert( alldata.length() < 100000 ); // debug check
	CBaseSocket::send( alldata.buffer(), alldata.length() );
//#ifdef NL_DEBUG
	if ( _Logging )
	{
		if ( message.typeIsNumber() )
		{
			nldebug( "P1: Socket %d sent message %hd (%s) (%d bytes +%d)",
				_Sock, message.typeAsNumber(), message.typeAsString().c_str(), len, alldata.length()-len  );
		}
		else
		{
			nldebug( "P1: Socket %d sent message %s (%d bytes +%d)",
				_Sock, message.typeAsString().c_str(), len, alldata.length()-len );
		}
		char buf [128];
		if ( message.typeAsString() == "" )
		{
			NLMISC::smprintf( buf, 128, "%hu", message.typeAsNumber() );
		}
		else
		{
			NLMISC::smprintf( buf, 128, "%s", message.typeAsString().c_str() );
		}
		nlnetoutput( localAddr().asIPString().c_str(), _CurrentMsgNumberSend-1, remoteAddr().asIPString().c_str(), buf, message.length() );
	}
//#endif
}


/*
 * Checks if there are some data to receive
 */
bool CSocket::dataAvailable() throw (ESocket)
{
	if ( dataAvailableFlag() ) // true if a CMsgSocket object has just tested positively the socket
	{
		return true;
	}
	return CBaseSocket::dataAvailable();
}




/*
 * Process an incoming bind message
 */
void CSocket::processBindMessage( CMessage& message )
{
	std::string key;
	TTypeNum num = 0;
	message.serial( key );
	message.serial( num );
	_MsgMap.insert( TMsgMapItem(key,num) );
	if ( _Logging )
	{
		nldebug( "P1: Socket %d : %s is now known as %hu for received messages", _Sock, key.c_str(), num );
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
CMessage CSocket::encode( CMessage& msg ) throw (ESocket)
{
	CMessage alldata( "", false, msg.length()+CMessage::maxHeaderLength() );

	// 1. Write message type
	packMessage( msg );
	TTypeNum code;
	TTypeNum namelen = msg.typeAsString().length();
	if ( msg.typeIsNumber() )
	{
		// Message type number
		code = msg.typeAsNumber();
		nlassert( code < 60 );
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

	// 4. Write message number
	alldata.serial( _CurrentMsgNumberSend );
	_CurrentMsgNumberSend++;

	// 5. Write message payload
	if ( msgsize != 0 )
	{
		alldata.serialBuffer( const_cast<uint8*>(msg.buffer()), msg.length() );
	}

	return alldata;
}


/*
 * Returns an input message with header extracted from the payload buffer
 */
CMessage CSocket::decode( CMessage& alldata ) throw (ESocket)
{
	// 1. Read message type
	TTypeNum msgtype = 0;
	alldata.serial( msgtype );

	// 2. Read message name (optional)
	TTypeNum msgnamelen = 0;
	char *msgname = NULL;
	if ( msgtype < 0 )
	{
		msgnamelen = msgtype & 0x7FFF;
		msgname = new char[msgnamelen+1];
		alldata.serialBuffer( (uint8*)msgname, msgnamelen );
		msgname[msgnamelen] = '\0';
	}
	// 3. Read message payload size
	uint32 msgsize = 0;
	alldata.serial( msgsize );

	// Set message type
	CMessage msg( "", true );
	if ( msgtype < 0 )
	{
		msg.setType( std::string(msgname) );
	}
	else
	{
		msg.setType( msgtype );
	}
	if ( msgname != NULL )
	{
		delete [] msgname;
	}

	// 4. Read message number
	uint8 recvd_msg_number;
	alldata.serial( recvd_msg_number );

	// 5. Read buffer
	if ( msgsize > 0 )
	{
		if ( msgsize > 100000 ) // debug check
		{
			nlwarning( "Message size is %u", msgsize );
			throw ESocket( "Invalid message size" );
		}
		alldata.serialBuffer( msg.bufferToFill(msgsize), msgsize );
	}

	return msg;
}


/*
 * Receives data, or blocks if !dataAvailable()). Returns false if !connected().
 */
bool CSocket::receive( CMessage& message ) throw (ESocket)
{
	if ( (! _Connected) && _Reliable )
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
	if ( ( (! _Connected) && _Reliable ) && (! dataAvailable()) )
	{
		return false;
	}

	// Receive incoming message
	doReceive( message );

	return true;
}


/*
 * Helper method for receive() and received()
 */
void CSocket::doReceive( CMessage& message ) throw (ESocket)
{
	// 1. Read message type
	TTypeNum msgtype;
	CBaseSocket::doReceive( (uint8*)&msgtype, sizeof(msgtype) );
#ifdef NL_BIG_ENDIAN
	NLMISC_BSWAP16(msgtype);
#endif
	//cout << msgtype << " ";

	// 2. Read message name (optional)
	TTypeNum msgnamelen = 0;
	char *msgname = NULL;
	if ( msgtype < 0 )
	{
		msgnamelen = msgtype & 0x7FFF;
		msgname = new char[msgnamelen+1];
		CBaseSocket::doReceive( (uint8*)msgname, msgnamelen );
		msgname[msgnamelen] = '\0';
	}
	// 3. Read message payload size
	uint32 msgsize;
	CBaseSocket::doReceive( (uint8*)&msgsize, sizeof(msgsize) );
#ifdef NL_BIG_ENDIAN
	NLMISC_BSWAP32(msgsize);
#endif
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

	// 4. Read message number
	uint8 recvd_msg_number;
	CBaseSocket::doReceive( (uint8*)&recvd_msg_number, sizeof(recvd_msg_number) );

	// 5. Read all buffer
	if ( msgsize > 0 )
	{
		if ( msgsize > 100000 ) // debug check
		{
			nlwarning( "Message size is %u !", msgsize );
			throw ESocket( "Invalid message size" );
		}
		CBaseSocket::doReceive( message.bufferToFill(msgsize), msgsize );
	}

	if ( _Logging )
	{
		if ( message.typeIsNumber() )
		{
			nldebug( "P1: Socket %d received message %hd (%d bytes +%d)",
				_Sock, message.typeAsNumber(), message.length(), sizeof(msgtype)+msgnamelen+sizeof(msgsize) );
		}
		else
		{
			nldebug( "P1: Socket %d received message %s (%d bytes +%d)",
				_Sock, message.typeAsString().c_str(), message.length(), sizeof(msgtype)+msgnamelen+sizeof(msgsize) );
		}
		nlnetinput( remoteAddr().asIPString().c_str(), recvd_msg_number, localAddr().asIPString().c_str() );
	}
}


/*
 * Sends a message
 */
void CSocket::sendTo( CMessage& message, const CInetAddress& addr ) throw (ESocket)
{
	CMessage alldata = encode( message );
	CBaseSocket::sendTo( alldata.buffer(), alldata.length(), addr );
	if ( _Logging )
	{
		if ( message.typeIsNumber() )
		{
			nldebug( "P1: Socket %d sent message %hd (%s)",
				_Sock, message.typeAsNumber(), message.typeAsString().c_str() );
		}
		else
		{
			nldebug( "P1: Socket %d sent message %s of %d bytes",
				_Sock, message.typeAsString().c_str() );
		}
	}

}


/*
 * Receives data (returns false if !dataAvailable()).
 */
bool CSocket::receivedFrom( CMessage& message, CInetAddress& addr )
{
	// Receive incoming message
	uint32 msgtotalsize = CMessage::maxLength();
	CMessage alldata( "", true, msgtotalsize );
	if ( CBaseSocket::receivedFrom( alldata.bufferToFill( msgtotalsize ), msgtotalsize, addr ) )
	{
		message = decode( alldata );
		if ( _Logging )
		{
			if ( message.typeIsNumber() )
			{
				nldebug( "P1: Socket %d received message %hd",
					_Sock, message.typeAsNumber() );
			}
			else
			{
				nldebug( "P1: Socket %d received message %s",
					_Sock, message.typeAsString().c_str() );
			}
		}
		return true;
	}
	return false;
}


} // NLNET
