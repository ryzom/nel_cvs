/** \file buf_client.cpp
 * Network engine, layer 1, client
 *
 * $Id: buf_client.cpp,v 1.1 2001/05/02 12:36:31 lecroart Exp $
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

#include "nel/net/buf_client.h"
#include "nel/misc/thread.h"
#include "nel/misc/debug.h"

#ifdef NL_OS_WINDOWS
#include <winsock2.h>
#elif defined NL_OS_UNIX
#include <netinet/in.h>
#endif

using namespace NLMISC;
using namespace std;


namespace NLNET {


/***************************************************************************************************
 * User main thread (initialization)
 **************************************************************************************************/

/*
 * Constructor
 */
CBufClient::CBufClient( bool nodelay ) :
	CBufNetBase(),
	_RecvTask( NULL ),
	_RecvThread( NULL ),
	_NoDelay( nodelay ),
	_PrevBytesDownloaded( 0 ),
	_PrevBytesUploaded( 0 ),
	_Connected( false )
	/*_PrevBytesReceived( 0 ),
	_PrevBytesSent( 0 )*/
{
	nlnettrace( "CBufClient::CBufClient" ); // don't define a global object

	_BufSock = new CBufSock();
	_RecvTask = new CClientReceiveTask( this, _BufSock );
}


/*
 * Connects to the specified host
 */
void CBufClient::connect( const CInetAddress& addr )
{
	nlnettrace( "CBufClient::connect" );
	_BufSock->Sock->connect( addr );
	_Connected = true;
	_BufSock->_KnowConnected = true; // because there is no connection advertising for the client version
	if ( _NoDelay )
	{
		_BufSock->Sock->setNoDelay( true );
	}
	_PrevBytesDownloaded = 0;
	_PrevBytesUploaded = 0;
	/*_PrevBytesReceived = 0;
	_PrevBytesSent = 0;*/

	// Allow reconnection
	if ( _RecvThread != NULL )
	{
		delete _RecvThread;
	}
	_RecvThread = IThread::create( _RecvTask );
	_RecvThread->start();
}


/***************************************************************************************************
 * User main thread (running)
 **************************************************************************************************/


/*
 * Sends a message to the remote host
 */
void CBufClient::send( const std::vector<uint8>& buffer )
{
	nlnettrace( "CBufClient::send" );

	if ( ! _BufSock->pushBuffer( buffer ) )
	{
		// Disconnection event if disconnected
		_BufSock->advertiseDisconnection( this, NULL );
	}
}


/*
 * Checks if there are some data to receive
 */
bool CBufClient::dataAvailable()
{
	{
		CFifoAccessor recvfifo( &receiveQueue() );
		do
		{
			// Check if the receive queue is empty
			if ( recvfifo.value().empty() )
			{
				return false;
			}
			else
			{
				vector<uint8> buffer;
				recvfifo.value().front( buffer );

				// Test if it the next block is a system event
				switch ( buffer[buffer.size()-1] )
				{
					
				// Normal message available
				case CBufNetBase::User:
					return true;

				// Process disconnection event
				case CBufNetBase::Disconnection:

					nldebug( "Disconnection event" );
					_Connected = false;

					// Call callback if needed
					if ( disconnectionCallback() != NULL )
					{
						disconnectionCallback()( *((TSockId*)(&*buffer.begin())), argOfDisconnectionCallback() );
					}

					// Unlike the server version, we do not delete the CBufSock object here,
					// it will be done in the destructor of CBufClient
					break;

				default:
					nlinfo( "L1: Invalid block type: %hu", (uint16)(buffer[buffer.size()-1]) );
					nlinfo( "L1: Buffer (%d B): [%s]", buffer.size(), stringFromVector(buffer).c_str() );
					nlinfo( "L1: Receive queue:" );
					recvfifo.value().display();
					nlerror( "L1: Invalid system event type in client receive queue" );

				}
				// Extract system event
				recvfifo.value().pop();
			}
		}
		while ( true );
	}
}

  
/*
 * Receives next block of data in the specified buffer (resizes the vector)
 */
void CBufClient::receive( std::vector<uint8>& buffer )
{
	nlnettrace( "CBufClient::receive" );

	// Extract buffer from the receive queue
	{
		CFifoAccessor recvfifo( &receiveQueue() );
		nlassert( ! recvfifo.value().empty() );
		recvfifo.value().front( buffer );
		recvfifo.value().pop();
	}

	// Extract event type
	nlassert( buffer[buffer.size()-1] == CBufNetBase::User );
	nldebug( "L1: Client read buffer (%d+%d B): [%s]", buffer.size(), sizeof(TSockId)+1, stringFromVector(buffer).c_str() );
	buffer.resize( buffer.size()-1 );
}


/*
 * Update the network (call this method evenly)
 */
void CBufClient::update()
{
	//nlnettrace( "CBufClient::update" );

	// Update sending
	bool sendingok = _BufSock->update();

	// Disconnection event if disconnected
	if ( ! ( _BufSock->Sock->connected() && sendingok ) )
	{
		_BufSock->advertiseDisconnection( this, NULL );
	}
}


/*
 * Disconnect the remote host
 */
void CBufClient::disconnect( bool quick )
{
	nlnettrace( "CBufClient::disconnect" );
	
	// Flush sending is asked for
	if ( ! quick )
	{
		_BufSock->flush();
	}

	// Disconnect
	_BufSock->Sock->disconnect();

	// Reset _Connected and prevent from advertising the disconnection
	_Connected = false;
	_BufSock->_KnowConnected = false;

	// Empty the receive queue
	{
		CFifoAccessor recvfifo( &receiveQueue() );
		recvfifo.value().clear();
	}
}


// Utility function for newBytes...()
inline uint64 updateStatCounter( uint64& counter, uint64 newvalue )
{
	uint64 result = newvalue - counter;
	counter = newvalue;
	return result;
}


/*
 * Returns the number of bytes downloaded since the previous call to this method
 */
uint64 CBufClient::newBytesDownloaded()
{
	return updateStatCounter( _PrevBytesDownloaded, bytesDownloaded() );
}


/*
 * Returns the number of bytes uploaded since the previous call to this method
 */
uint64 CBufClient::newBytesUploaded()
{
	return updateStatCounter( _PrevBytesUploaded, bytesUploaded() );
}


/*
 * Returns the number of bytes popped by receive() since the previous call to this method
 */
/*uint64 CBufClient::newBytesReceived()
{
	return updateStatCounter( _PrevBytesReceived, bytesReceived() );
}*/


/*
 * Returns the number of bytes pushed by send() since the previous call to this method
 */
/*uint64 CBufClient::newBytesSent()
{
	return updateStatCounter( _PrevBytesSent, bytesSent() );
}*/


/*
 * Destructor
 */
CBufClient::~CBufClient()
{
	nlnettrace( "CBufClient::~CBufClient" );

	if ( _RecvTask != NULL )
		delete _RecvTask;

	if ( _RecvThread != NULL )
		delete _RecvThread;

	if ( _BufSock != NULL )
		delete _BufSock;

	nlnettrace( "Exiting CBufClient::~CBufClient" );
}


/***************************************************************************************************
 * Receive thread 
 **************************************************************************************************/


/*
 * Code of receiving thread for clients
 */
void CClientReceiveTask::run()
{
	nlnettrace( "CClientReceiveTask::run" );

	bool connected = true;
	while ( connected ) // does not call _Sock->connected() to avoid mutex (not needed for client)
	{
		try
		{
			// Receive message length (in blocking mode)
			TBlockSize len16;
			uint lenoflen = sizeof(len16);
			sock()->receive( (uint8*)&len16, lenoflen );
			uint len = (uint)ntohs( len16 );

			// Receive message payload (in blocking mode)
			vector<uint8> buffer ( len );
			sock()->receive( &*buffer.begin(), len );
#ifdef NL_DEBUG
			nldebug( "L1: Client %s received buffer (%d B): [%s]", _SockId->asString().c_str(), buffer.size(), stringFromVector(buffer).c_str() );
#endif
			// Add event type
			buffer.push_back( CBufNetBase::User );

			// Push message into receive queue
			_Client->pushMessageIntoReceiveQueue( buffer );
		}
		catch ( ESocketConnectionClosed& )
		{
			nldebug( "L1: Client connection %s closed", _SockId->asString().c_str() );
			connected = false;
		}
		catch ( ESocket& )
		{
			nldebug( "L1: Client connection %s broken", _SockId->asString().c_str() );
			sock()->disconnect();
			connected = false;
		}
	}
}


} // NLNET
