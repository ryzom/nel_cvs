/** \file buf_sock.cpp
 * Network engine, layer 1, base
 *
 * $Id: buf_sock.cpp,v 1.33.8.3 2003/08/20 10:30:16 cado Exp $
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

#include "stdnet.h"

#include "nel/misc/hierarchical_timer.h"

#include "nel/net/buf_sock.h"
#include "nel/net/buf_server.h"

#ifdef NL_OS_WINDOWS
#include <winsock2.h>
#elif defined NL_OS_UNIX
#include <netinet/in.h>
#endif

using namespace NLMISC;
using namespace std;


namespace NLNET {


NLMISC::CMutex nettrace_mutex("nettrace_mutex");


/*
 * Constructor
 */
CBufSock::CBufSock( CTcpSock *sock ) :
	SendNextValue(0),
	ReceiveNextValue(0),
	Sock( sock ),
	_KnowConnected( false ),
	_LastFlushTime( 0 ),
	_TriggerTime( 20 ),
	_TriggerSize( -1 ),
	_RTSBIndex( 0 ),
	_AppId( 0 ),
	_ConnectedState( false )
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
	nlassert (this != InvalidSockId);	// invalid bufsock

	nlnettrace( "CBufSock::~CBufSock" );
	
	delete Sock; // the socket disconnects automatically if needed
	
	// destroy the structur to be sure that other people will not access to this anymore
	AuthorizedCallback = "";
	Sock = NULL;
	_KnowConnected = false;
	_LastFlushTime = 0;
	_TriggerTime = 0;
	_TriggerSize = 0;
	_ReadyToSendBuffer.clear ();
	_RTSBIndex = 0;
	_AppId = 0;
	_ConnectedState = false;
}


/*
 * Returns a readable string from a vector of bytes, beginning from pos, limited to 'len' characters. '\0' are replaced by ' '
 */
string stringFromVectorPart( const vector<uint8>& v, uint32 pos, uint32 len )
{
	nlassertex( pos+len <= v.size(), ("pos=%u len=%u size=%u", pos, len, v.size()) );

	string s;
	if ( (! v.empty()) && (len!=0) )
	{
		// Copy contents
		s.resize( len );
		memcpy( &*s.begin(), &*v.begin()+pos, len );

		// Replace '\0' characters
		string::iterator is;
		for ( is=s.begin(); is!=s.end(); ++is )
		{
			if ( ! isprint((uint8)(*is)) || (*is) == '%' )
			{
				(*is) = '?';
			}
		}
	}

	return s;
}


/*
 * Force to send all data pending in the send queue
 * Note: this method works with both blocking and non-blocking sockets
 * Precond: the send queue should not contain an empty block
 */
bool CBufSock::flush()
{
	nlassert (this != InvalidSockId);	// invalid bufsock
	//nlnettrace( "CBufSock::flush" );

	// Copy data from the send queue to _ReadyToSendBuffer
	TBlockSize netlen;
//	vector<uint8> tmpbuffer;

	// Process each element in the send queue
	while ( ! SendFifo.empty() )
	{
		uint8 *tmpbuffer;
		uint32 size;
		SendFifo.front( tmpbuffer, size );

		// Compute the size and add it into the beginning of the buffer
		netlen = htonl( (TBlockSize)size );
		uint32 oldBufferSize = _ReadyToSendBuffer.size();
		_ReadyToSendBuffer.resize (oldBufferSize+sizeof(TBlockSize)+size);
		*(TBlockSize*)&(_ReadyToSendBuffer[oldBufferSize])=netlen;
		//nldebug( "O-%u %u+L%u (0x%x)", Sock->descriptor(), oldBufferSize, size, size );


		// Append the temporary buffer to the global buffer
		CFastMem::memcpy (&_ReadyToSendBuffer[oldBufferSize+sizeof(TBlockSize)], tmpbuffer, size);
		SendFifo.pop();
	}

	// Actual sending of _ReadyToSendBuffer
	//if ( ! _ReadyToSendBuffer.empty() )
	if ( _ReadyToSendBuffer.size() != 0 )
	{		
		// Send
		CSock::TSockResult res;
		TBlockSize len = _ReadyToSendBuffer.size() - _RTSBIndex;

		res = Sock->send( _ReadyToSendBuffer.getPtr()+_RTSBIndex, len, false );

		if ( res == CSock::Ok )
		{
/*			// Debug display
			switch ( _FlushTrigger )
			{
			case FTTime : nldebug( "LNETL1: Time triggered flush for %s:", asString().c_str() ); break;
			case FTSize : nldebug( "LNETL1: Size triggered flush for %s:", asString().c_str() ); break;
			default:	  nldebug( "LNETL1: Manual flush for %s:", asString().c_str() );
			}
			_FlushTrigger = FTManual;
			nldebug( "LNETL1: %s sent effectively a buffer (%d on %d B)", asString().c_str(), len, _ReadyToSendBuffer.size() );
*/			
			
			// TODO OPTIM remove the nldebug for speed
			//commented for optimisation nldebug( "LNETL1: %s sent effectively %u/%u bytes (pos %u wantedsend %u)", asString().c_str(), len, _ReadyToSendBuffer.size(), _RTSBIndex, realLen/*, stringFromVectorPart(_ReadyToSendBuffer,_RTSBIndex,len).c_str()*/ );

			if ( _RTSBIndex+len == _ReadyToSendBuffer.size() ) // for non-blocking mode
			{
				// If sending is ok, clear the global buffer
				//nldebug( "O-%u all %u bytes (%u to %u) sent", Sock->descriptor(), len, _RTSBIndex, _ReadyToSendBuffer.size() );
				_ReadyToSendBuffer.clear();
				_RTSBIndex = 0;
			}
			else
			{
				// Or clear only the data that was actually sent
				nlassertex( _RTSBIndex+len < _ReadyToSendBuffer.size(), ("index=%u len=%u size=%u", _RTSBIndex, len, _ReadyToSendBuffer.size()) );
				//nldebug( "O-%u only %u B on %u (%u to %u) sent", Sock->descriptor(), len, _ReadyToSendBuffer.size()-_RTSBIndex, _RTSBIndex, _ReadyToSendBuffer.size() );
				_RTSBIndex += len;
				if ( _ReadyToSendBuffer.size() > 20480 ) // if big, clear data already sent
				{
					uint nbcpy = _ReadyToSendBuffer.size() - _RTSBIndex;
					for (uint i = 0; i < nbcpy; i++)
					{
						_ReadyToSendBuffer[i] = _ReadyToSendBuffer[i+_RTSBIndex];
					}
					_ReadyToSendBuffer.resize(nbcpy);
					//_ReadyToSendBuffer.erase( _ReadyToSendBuffer.begin(), _ReadyToSendBuffer.begin()+_RTSBIndex );
					_RTSBIndex = 0;
					//nldebug( "O-%u Cleared data already sent, %u B remain", Sock->descriptor(), nbcpy );
				}
			}
		}
		else
		{
#ifdef NL_DEBUG
			// can happen in a normal behavior if, for example, the other side is not connected anymore
			nldebug( "LNETL1: %s failed to send effectively a buffer of %d bytes", asString().c_str(), _ReadyToSendBuffer.size() );
#endif
			return false;
		}
	}

	return true;
}


/* Sets the time flush trigger (in millisecond). When this time is elapsed,
 * all data in the send queue is automatically sent (-1 to disable this trigger)
 */
void CBufSock::setTimeFlushTrigger( sint32 ms )
{
	nlassert (this != InvalidSockId);	// invalid bufsock
	_TriggerTime = ms;
	_LastFlushTime = CTime::getLocalTime();
}


/*
 * Update the network sending (call this method evenly). Returns false if an error occured.
 */
bool CBufSock::update()
{
	nlassert (this != InvalidSockId);	// invalid bufsock
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
 * Precond: not connected
 */
void CBufSock::connect( const CInetAddress& addr, bool nodelay, bool connectedstate )
{
	nlassert (this != InvalidSockId);	// invalid bufsock
	nlassert( ! Sock->connected() );

	SendNextValue = ReceiveNextValue = 0;
	
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
	nlassert (this != InvalidSockId);	// invalid bufsock
	Sock->disconnect();
	_ConnectedState = connectedstate;
	_KnowConnected = connectedstate;

	SendNextValue = ReceiveNextValue = 0;
}


/*
 * Returns a string with the characteristics of the object
 */
string CBufSock::asString() const
{
	stringstream ss;
	if (this == InvalidSockId) // tricky
		ss << "<Null>";
	else
	{
		// if it crashs here, it means that the CBufSock was deleted and you try to access to the virtual table that is empty
		// because the object is destroyed.
		ss << typeStr();
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
 * Constructor
 */
CNonBlockingBufSock::CNonBlockingBufSock( CTcpSock *sock ) :
	CBufSock( sock ),
	_NowReadingBuffer( false ),
	_BytesRead( 0 ),
	_Length( 0 ),
	_MaxExpectedBlockSize( 10485760 ) // 10M
{
	nlnettrace( "CNonBlockingBufSock::CNonBlockingBufSock" );
}


/*
 * Constructor with an existing socket (created by an accept())
 */
CServerBufSock::CServerBufSock( CTcpSock *sock ) :
	CNonBlockingBufSock( sock ),
	_Advertised( false ),
	_OwnerTask( NULL )
{
	nlassert (this != InvalidSockId);	// invalid bufsock
	nlnettrace( "CServerBufSock::CServerBufSock" );
}


// In Receive Threads:


/*
 * Receives a part of a message (nonblocking socket only)
 */
bool CNonBlockingBufSock::receivePart( uint32 nbExtraBytes )
{
	nlassert (this != InvalidSockId);	// invalid bufsock
	nlnettrace( "CNonBlockingBufSock::receivePart" );

	TBlockSize actuallen;
	if ( ! _NowReadingBuffer )
	{
		// Receiving length prefix
		actuallen = sizeof(_Length)-_BytesRead;
		Sock->receive( (uint8*)(&_Length)+_BytesRead, actuallen );
		_BytesRead += actuallen;
		if ( _BytesRead == sizeof(_Length ) )
		{
			if ( _Length != 0 )
			{
				_Length = ntohl( _Length );
				//nldebug( "I-%u L%u (0x%x) a%u", Sock->descriptor(), _Length, _Length, actuallen );

				// Test size limit
				if ( _Length > _MaxExpectedBlockSize )
				{
					nlwarning( "LNETL1: Socket %s received header length %u exceeding max expected %u... Disconnecting", asString().c_str(), _Length, _MaxExpectedBlockSize );
					throw ESocket( toString( "Received length %u exceeding max expected %u from %s", _Length, _MaxExpectedBlockSize, Sock->remoteAddr().asString().c_str() ).c_str(), false );
				}

				_NowReadingBuffer = true;
				_ReceiveBuffer.resize( _Length + nbExtraBytes );
			}
			else
			{
				nlwarning( "LNETL1: Socket %s received null length in block header", asString().c_str() );
			}
			_BytesRead = 0;
		}
	}

	if ( _NowReadingBuffer )
	{
		// Receiving payload buffer
		actuallen = _Length-_BytesRead;
		Sock->receive( &*_ReceiveBuffer.begin()+_BytesRead, actuallen );
		_BytesRead += actuallen;

		if ( _BytesRead == _Length )
		{
#ifdef NL_DEBUG
			nldebug( "LNETL1: %s received buffer (%u bytes): [%s]", asString().c_str(), _ReceiveBuffer.size(), stringFromVector(_ReceiveBuffer).c_str() );
#endif
			_NowReadingBuffer = false;
			//nldebug( "I-%u all %u B on %u", Sock->descriptor(), actuallen );
			_BytesRead = 0;
			return true;
		}
		//else
		//{
		//	nldebug( "I-%u only %u B on %u", actuallen, Sock->descriptor(), _Length-(_BytesRead-actuallen) );
		//}
	}

	return false;
}


} // NLNET
