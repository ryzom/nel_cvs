/** \file buf_net_base.cpp
 * Network engine, layer 1, base
 *
 * $Id: buf_sock.cpp,v 1.5 2001/05/11 09:29:19 cado Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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

#include "nel/misc/types_nl.h"
#include "nel/net/buf_sock.h"
#include "nel/net/buf_server.h"
#include "nel/misc/debug.h"

#ifdef NL_OS_WINDOWS
#include <winsock2.h>
#elif defined NL_OS_UNIX
#include <netinet/in.h>
#endif

using namespace NLMISC;
using namespace std;


namespace NLNET {


NLMISC::CMutex nettrace_mutex;


/*
 * Constructor
 */
CBufSock::CBufSock( CTcpSock *sock ) :
	Sock( sock ),
	_TriggerTime( 20 ),
	_TriggerSize( -1 ),
	_LastFlushTime( 0 ),
	_KnowConnected( false ),
	_ConnectedState( false ),
	_AppId (0)
{
	nlnettrace( "CBufSock::CBufSock" ); // don't define a global object

	if ( Sock == NULL )
	  {
		Sock = new CTcpSock();
	  }

#ifdef NL_DEBUG
	_FlushTrigger = FTManual;
#endif
	_LastFlushTime = CTime::getLocalTime();
}


/*
 * Destructor
 */
CBufSock::~CBufSock()
{
	nlnettrace( "CBufSock::~CBufSock" );
	delete Sock; // the socket disconnects automatically if needed
}


/*
 * Force to send all data pending in the send queue
 */
bool CBufSock::flush()
{
	//nlnettrace( "CBufSock::flush" );

	// Copy data from the send queue to _ReadyToSendBuffer
	TBlockSize netlen;
	vector<uint8> tmpbuffer;
	
	// Process each element in the send queue
	while ( ! SendFifo.empty() )
	{
		// Extract a temporary buffer from the send queue
		SendFifo.front( tmpbuffer );

		// Compute the size and add it into the beginning of the buffer
		netlen = htons( (TBlockSize)(tmpbuffer.size()) );
		_ReadyToSendBuffer.insert( _ReadyToSendBuffer.end(), sizeof(TBlockSize), 0 );
		memcpy( &*(_ReadyToSendBuffer.end()-2), (uint8*)&netlen, sizeof(TBlockSize) );

		// Append the temporary buffer to the global buffer
		_ReadyToSendBuffer.insert( _ReadyToSendBuffer.end(), tmpbuffer.begin(), tmpbuffer.end() );
		SendFifo.pop();
	}

	// Actual sending of _ReadyToSendBuffer
	if ( ! _ReadyToSendBuffer.empty() )
	{
		// Send
		CSock::TSockResult res;
		if ( ( res = Sock->send( &*_ReadyToSendBuffer.begin(), _ReadyToSendBuffer.size(), false )) == CSock::Ok )
		{
#ifdef NL_DEBUG
			// Debug display
			switch ( _FlushTrigger )
			{
			case FTTime : nldebug( "L1: Time triggered flush for %s:", asString().c_str() ); break;
			case FTSize : nldebug( "L1: Size triggered flush for %s:", asString().c_str() ); break;
			default:	  nldebug( "L1: Manual flush for %s:", asString().c_str() );
			}
			nldebug( "L1: %s sent effectively a buffer (%d B): [%s]", asString().c_str(), _ReadyToSendBuffer.size(), stringFromVector(_ReadyToSendBuffer).c_str() );
			_FlushTrigger = FTManual;
#else
			nldebug( "L1: %s sent effectively a buffer (%d B)", asString().c_str(), _ReadyToSendBuffer.size() );
#endif
			// If sending is ok, clear the global buffer
			_ReadyToSendBuffer.clear();
		}
		else
		{
#ifdef NL_DEBUG
			if ( res == CSock::Error )
			{
				nldebug( "L1: %s failed to send effectively a buffer of %d bytes", asString().c_str(), _ReadyToSendBuffer.size() );
			}
#endif
			// Stop sending (no error if "would block" (note: only the server uses non-blocking sockets))
			return ( res == CSock::WouldBlock );
		}
	}
	return true;

	/* This was not optimal especially if the Nagle algorithm was disabled (setNoDelay(true)).
	 * New (above): construct a buffer with the entire contents of SendFifo, then send it.
	 * If the sending would block, keep the buffer somewhere for later sending.
	 * Warning about the size of the resulting buffer (?).
	 */
	/*//OLD CODE
	vector<uint8> buffer, buffer2;
	while ( ! SendFifo.empty() )
	{
		// Extract buffer from the send queue
		SendFifo.front( buffer );

		// Setup a buffer with length prefix and payload
		TBlockSize len = (TBlockSize)buffer.size();
		buffer2.resize( sizeof(len) + len );
		TBlockSize netlen = htons( len );
		memcpy( &*buffer2.begin(), (uint8*)&netlen, sizeof(netlen) );
		memcpy( (&*buffer2.begin())+sizeof(netlen), &*buffer.begin(), len );

		// Send
		if ( ( res = Sock->send( &*buffer2.begin(), buffer2.size(), false )) == CSock::Ok )
		{
			// If sending is ok, pop data from the queue
			SendFifo.pop();
			nldebug( "L1: A buffer effectively sent" );
#ifdef NL_DEBUG
			string sbuf;
			sbuf.resize( buffer.size() );
			memcpy( &*sbuf.begin(), &*buffer.begin(), buffer.size() );
			nldebug( "L1: Sent buffer: [%s]", sbuf.c_str() );
#endif
		}
		else
		{
			// Stop sending (no error if "would block")
			return ( res == CSock::WouldBlock );
		}
	}
	return true;*/
}


/* Sets the time flush trigger (in millisecond). When this time is elapsed,
 * all data in the send queue is automatically sent (-1 to disable this trigger)
 */
void CBufSock::setTimeFlushTrigger( sint32 ms )
{
	_TriggerTime = ms;
	_LastFlushTime = CTime::getLocalTime();
}


/*
 * Update the network sending (call this method evenly). Returns false if an error occured.
 */
bool CBufSock::update()
{
//	nlnettrace( "CBufSock::update-BEGIN" );
	// Time trigger
	if ( _TriggerTime != -1 )
	{
		TTime now = CTime::getLocalTime();
		if ( (sint32)(now-_LastFlushTime) >= _TriggerTime )
		{
#ifdef NL_DEBUG
			_FlushTrigger = FTTime;
#endif
			if ( flush() )
			{
				_LastFlushTime = now;
//				nlnettrace ( "CBufSock::update-END time 1" );
				return true;
			}
			else
			{
//				nlnettrace ( "CBufSock::update-END time 0" );
				return false;
			}
		}
	}
	// Size trigger
	if ( _TriggerSize != -1 )
	{
		if ( (sint32)SendFifo.size() > _TriggerSize )
		{
#ifdef NL_DEBUG
			_FlushTrigger = FTSize;
#endif
//			nlnettrace( "CBufSock::update-END size" );
			return flush();
		}
	}
//	nlnettrace( "CBufSock::update-END nosend" );
	return true;
}


/*
 * Connects to the specified addr; set connectedstate to true if no connection advertising is needed
 */
void CBufSock::connect( const CInetAddress& addr, bool nodelay, bool connectedstate )
{
	Sock->connect( addr );
	_ConnectedState = connectedstate;
	_KnowConnected = connectedstate;
	if ( nodelay )
	{
		Sock->setNoDelay( true );
	}
}


/*
 * Disconnects; set connectedstate to false if no disconnection advertising is needed
 */
void CBufSock::disconnect( bool connectedstate )
{
	Sock->disconnect();
	_ConnectedState = connectedstate;
	_KnowConnected = connectedstate;
}


/*
 * Returns a string with the characteristics of the object
 */
string CBufSock::asString() const
{
	stringstream ss;
	if (this == NULL)
		ss << "<Null>";
	else
	{
		ss << hex << this << dec << " (socket ";
		
		if (Sock == NULL)
			ss << "<Null>";
		else
			ss << Sock->descriptor();

		ss << ")";
	}
	return ss.str();
}


/*
 * Constructor with an existing socket (created by an accept())
 */
CServerBufSock::CServerBufSock( CTcpSock *sock ) :
	CBufSock( sock ),
	_NowReadingBuffer( false ),
	_BytesRead( 0 ),
	_Length( 0 ),
	_Advertised( false ),
	_OwnerTask( NULL )
{
	nlnettrace( "CServerBufSock::CServerBufSock" );
}


// In Receive Threads:


/*
 * Receives a part of a message (nonblocking socket only)
 */
bool CServerBufSock::receivePart()
{
	nlnettrace( "CServerBufSock::receivePart" );

	if ( ! _NowReadingBuffer )
	{
		// Receiving length prefix
		uint actuallen = sizeof(_Length)-_BytesRead;
		Sock->receive( (uint8*)(&_Length)+_BytesRead, actuallen );
		_BytesRead += actuallen;
		if ( _BytesRead == sizeof(_Length ) )
		{
			_Length = ntohs( _Length );
			_NowReadingBuffer = true;
			_ReceiveBuffer.resize( _Length );
			_BytesRead = 0;
		}
	}

	if ( _NowReadingBuffer )
	{
		// Receiving payload buffer
		uint actuallen = _Length-_BytesRead;
		Sock->receive( &*_ReceiveBuffer.begin()+_BytesRead, actuallen );
		_BytesRead += actuallen;

		if ( _BytesRead == _Length )
		{
#ifdef NL_DEBUG
			nldebug( "L1: %s received buffer (%d B): [%s]", asString().c_str(), _ReceiveBuffer.size(), stringFromVector(_ReceiveBuffer).c_str() );
#endif
			_NowReadingBuffer = false;
			_BytesRead = 0;
			return true;
		}
	}

	return false;
}

} // NLNET
