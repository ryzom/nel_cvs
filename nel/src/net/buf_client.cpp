/** \file buf_client.cpp
 * Network engine, layer 1, client
 *
 * $Id: buf_client.cpp,v 1.24 2003/02/07 16:08:25 lecroart Exp $
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

#include "nel/net/buf_client.h"
#include "nel/misc/thread.h"
#include "nel/net/dummy_tcp_sock.h"

#ifdef NL_OS_WINDOWS
#include <winsock2.h>
#elif defined NL_OS_UNIX
#include <netinet/in.h>
#endif

using namespace NLMISC;
using namespace std;


namespace NLNET {


uint32 	NbClientReceiveTask = 0;
	

/***************************************************************************************************
 * User main thread (initialization)
 **************************************************************************************************/

/*
 * Constructor
 */
CBufClient::CBufClient( bool nodelay, bool replaymode ) :
	CBufNetBase(),
	_NoDelay( nodelay ),
	_PrevBytesDownloaded( 0 ),
	_PrevBytesUploaded( 0 ),
	_RecvTask( NULL ),
	_RecvThread( NULL )
	/*_PrevBytesReceived( 0 ),
	_PrevBytesSent( 0 )*/
{
	nlnettrace( "CBufClient::CBufClient" ); // don't define a global object

	if ( replaymode )
	{
		_BufSock = new CNonBlockingBufSock( new CDummyTcpSock() ); // CHANGED: non-blocking client connection
	}
	else
	{
		_BufSock = new CNonBlockingBufSock(); // CHANGED: non-blocking client connection
		_RecvTask = new CClientReceiveTask( this, _BufSock );
	}
}


/*
 * Connects to the specified host
 * Precond: not connected
 */
void CBufClient::connect( const CInetAddress& addr )
{
	nlnettrace( "CBufClient::connect" );
	nlassert( ! _BufSock->Sock->connected() );
	_BufSock->setMaxExpectedBlockSize( maxExpectedBlockSize() );
	_BufSock->connect( addr, _NoDelay, true );
	_BufSock->setNonBlocking(); // ADDED: non-blocking client connection
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

void CBufClient::displayThreadStat (NLMISC::CLog *log)
{
	log->displayNL ("client thread %p nbloop %d", _RecvTask, _RecvTask->NbLoop);
}


/*
 * Sends a message to the remote host
 */
void CBufClient::send( const NLMISC::CMemStream& buffer )
{
	nlnettrace( "CBufClient::send" );
	nlassert( buffer.length() > 0 );
	nlassert( buffer.length() <= maxSentBlockSize() );

	// slow down the layer H_AUTO (CBufServer_send);

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
	// slow down the layer H_AUTO (CBufClient_dataAvailable);
	{
		/* If no data available, enter the 'while' loop and return false (1 volatile test)
		 * If there are user data available, enter the 'while' and return true immediately (1 volatile test + 1 short locking)
		 * If there is a disconnection event (rare), call the callback and loop
		 */
		while ( dataAvailableFlag() )
		{
			// Because _DataAvailable is true, the receive queue is not empty at this point
			uint8 val;
			{
				CFifoAccessor recvfifo( &receiveQueue() );
				val = recvfifo.value().frontLast ();
			}

			// Test if it the next block is a system event
			switch ( val )
			{
				
			// Normal message available
			case CBufNetBase::User:
				return true; // return immediatly, do not extract the message

			// Process disconnection event
			case CBufNetBase::Disconnection:

				nldebug( "Disconnection event" );
				_BufSock->setConnectedState( false );

				// Call callback if needed
				if ( disconnectionCallback() != NULL )
				{
					disconnectionCallback()( id(), argOfDisconnectionCallback() );
				}

				// Unlike the server version, we do not delete the CBufSock object here,
				// it will be done in the destructor of CBufClient
				break;

			default: // should not occur
				{
					CFifoAccessor recvfifo( &receiveQueue() );
					vector<uint8> buffer;
					recvfifo.value().front (buffer);
					nlinfo( "LNETL1: Invalid block type: %hu (should be = %hu)", (uint16)(buffer[buffer.size()-1]), (uint16)val );
					nlinfo( "LNETL1: Buffer (%d B): [%s]", buffer.size(), stringFromVector(buffer).c_str() );
					nlinfo( "LNETL1: Receive queue:" );
					recvfifo.value().display();
					nlerror( "LNETL1: Invalid system event type in client receive queue" );
				}
			}
			// Extract system event
			{
				CFifoAccessor recvfifo( &receiveQueue() );
				recvfifo.value().pop();
				setDataAvailableFlag( ! recvfifo.value().empty() );
			}

		}
		// _DataAvailable is false here
		return false;
	}
}


/* // OLD VERSION
bool CBufClient::dataAvailable()
{
	// slow down the layer H_AUTO (CBufClient_dataAvailable);
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
				uint8 val = recvfifo.value().frontLast ();

				// Test if it the next block is a system event
				switch ( val )
				{
					
				// Normal message available
				case CBufNetBase::User:
					return true; // return immediatly, do not extract the message

				// Process disconnection event
				case CBufNetBase::Disconnection:

					nldebug( "Disconnection event" );
					_BufSock->setConnectedState( false );

					// Call callback if needed
					if ( disconnectionCallback() != NULL )
					{
						disconnectionCallback()( id(), argOfDisconnectionCallback() );
					}

					// Unlike the server version, we do not delete the CBufSock object here,
					// it will be done in the destructor of CBufClient
					break;

				default:
					{
					vector<uint8> buffer;
					recvfifo.value().front (buffer);
					nlinfo( "LNETL1: Invalid block type: %hu (should be = %hu)", (uint16)(buffer[buffer.size()-1]), (uint16)val );
					nlinfo( "LNETL1: Buffer (%d B): [%s]", buffer.size(), stringFromVector(buffer).c_str() );
					nlinfo( "LNETL1: Receive queue:" );
					recvfifo.value().display();
					nlerror( "LNETL1: Invalid system event type in client receive queue" );
					}
				}
				// Extract system event
				recvfifo.value().pop();
			}
		}
		while ( true );
	}
}
*/

/*
 * Receives next block of data in the specified buffer (resizes the vector)
 * Precond: dataAvailable() has returned true
 */
void CBufClient::receive( NLMISC::CMemStream& buffer )
{
	nlnettrace( "CBufClient::receive" );
	//nlassert( dataAvailable() );

	// Extract buffer from the receive queue
	{
		CFifoAccessor recvfifo( &receiveQueue() );
		nlassert( ! recvfifo.value().empty() );
		recvfifo.value().front( buffer );
		recvfifo.value().pop();
		setDataAvailableFlag( ! recvfifo.value().empty() );
	}

	// Extract event type
	nlassert( buffer.buffer()[buffer.length()-1] == CBufNetBase::User );
	//commented for optimisation nldebug( "LNETL1: Client read buffer (%d+%d B)", buffer.length(), sizeof(TSockId)+1 );
	buffer.resize( buffer.length()-1 );
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
		if ( _BufSock->Sock->connected() )
		{
			_BufSock->Sock->disconnect();
		}
		_BufSock->advertiseDisconnection( this, NULL );
	}
}


/*
 * Disconnect the remote host
 */
void CBufClient::disconnect( bool quick )
{
	nlnettrace( "CBufClient::disconnect" );

	// Do not allow to disconnect a socket that is not connected
	nlassert( _BufSock->connectedState() );

	// When the NS tells us to remove this connection AND the connection has physically
	// disconnected but not yet logically (i.e. disconnection event not processed yet),
	// skip flushing and physical active disconnection
	if ( _BufSock->Sock->connected() )
	{
		// Flush sending is asked for
		if ( ! quick )
		{
			_BufSock->flush();
		}

		// Disconnect and prevent from advertising the disconnection
		_BufSock->disconnect( false );
	}

	// Empty the receive queue
	{
		CFifoAccessor recvfifo( &receiveQueue() );
		recvfifo.value().clear();
		setDataAvailableFlag( false );
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

	// Disconnect if not done
	if ( _BufSock->Sock->connected() )
	{
		nlassert( _BufSock->connectedState() );

		disconnect( true );
	}

	// Clean thread termination
	if ( _RecvThread != NULL )
	{
		nldebug( "LNETL1: Waiting for the end of the receive thread..." );
		_RecvThread->wait();
	}

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
	NbClientReceiveTask++;
	NbNetworkTask++;
	nlnettrace( "CClientReceiveTask::run" );

	_NBBufSock->Sock->setTimeOutValue( 60, 0 );

	bool connected = true;
	while ( connected ) // does not call _Sock->connected() to avoid mutex (not needed for client)
	{
		try
		{
			// ADDED: non-blocking client connection

			// Wait until some data are received (sleepin' select inside)
			while ( ! _NBBufSock->Sock->dataAvailable() );

			// Process the data received
			if ( _NBBufSock->receivePart( 1 ) ) // 1 for the event type
			{
				//commented out for optimisation: nldebug( "LNETL1: Client %s received buffer (%u bytes)", _SockId->asString().c_str(), buffer.size()/*, stringFromVector(buffer).c_str()*/ );
				// Add event type
				_NBBufSock->fillEventTypeOnly();

				// Push message into receive queue
				_Client->pushMessageIntoReceiveQueue( _NBBufSock->receivedBuffer() );
			}
			
			NbLoop++;

			/* // OLD: blocking client connection
			// Receive message length (in blocking mode)
			TBlockSize blocklen;
			uint32 lenoflen = sizeof(blocklen);
			sock()->receive( (uint8*)&blocklen, lenoflen );
			uint32 len = ntohl( blocklen );
	
			if ( len != 0 )
			{
				// Test size limit
				if ( len > _Client->maxExpectedBlockSize() )
				{
					nlwarning( "LNETL1: Socket %s received length exceeding max expected, in block header... Disconnecting", _SockId->asString().c_str() );
					throw ESocket( "Received length exceeding max expected", false );
				}

				// Receive message payload (in blocking mode)
				CObjectVector<uint8> buffer;
				buffer.resize(len+1);

				sock()->receive( buffer.getPtr(), len );
				
				//commented out for optimisation: nldebug( "LNETL1: Client %s received buffer (%u bytes)", _SockId->asString().c_str(), buffer.size() );
				// Add event type
				buffer[len] = CBufNetBase::User;

				// Push message into receive queue
				_Client->pushMessageIntoReceiveQueue( buffer.getPtr(), buffer.size() );
			}
			else
			{
				nlwarning( "LNETL1: Socket %s received null length in block header", _SockId->asString().c_str() );
			}
			*/
		}
		catch ( ESocketConnectionClosed& )
		{
			nldebug( "LNETL1: Client connection %s closed", sockId()->asString().c_str() );
			// The socket went to _Connected=false when throwing the exception
			connected = false;
		}
		catch ( ESocket& )
		{
			nldebug( "LNETL1: Client connection %s broken", sockId()->asString().c_str() );
			sockId()->Sock->disconnect();
			connected = false;
		}
	}

	nlnettrace( "Exiting CClientReceiveTask::run()" );
	NbClientReceiveTask--;
	NbNetworkTask--;
}

NLMISC_VARIABLE(uint32, NbClientReceiveTask, "Number of client receive thread");



} // NLNET
