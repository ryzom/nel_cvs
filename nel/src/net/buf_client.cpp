/** \file buf_client.cpp
 * Network engine, layer 1, client
 *
 * $Id: buf_client.cpp,v 1.16 2002/07/02 15:56:58 lecroart Exp $
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


/***************************************************************************************************
 * User main thread (initialization)
 **************************************************************************************************/

/*
 * Constructor
 */
CBufClient::CBufClient( bool nodelay, bool replaymode ) :
	CBufNetBase(),
	_RecvTask( NULL ),
	_RecvThread( NULL ),
	_NoDelay( nodelay ),
	_PrevBytesDownloaded( 0 ),
	_PrevBytesUploaded( 0 )
	/*_PrevBytesReceived( 0 ),
	_PrevBytesSent( 0 )*/
{
	nlnettrace( "CBufClient::CBufClient" ); // don't define a global object

	if ( replaymode )
	{
		_BufSock = new CBufSock( new CDummyTcpSock() );
	}
	else
	{
		_BufSock = new CBufSock();
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
	_BufSock->connect( addr, _NoDelay, true );
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
void CBufClient::send( const NLMISC::CMemStream& buffer )
{
	nlnettrace( "CBufClient::send" );
	nlassert( buffer.length() > 0 );
	nlassert( buffer.length() <= maxSentBlockSize() );

	H_AUTO (CBufServer_send);

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
	H_AUTO (CBufClient_dataAvailable);
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

  
/*
 * Receives next block of data in the specified buffer (resizes the vector)
 * Precond: dataAvailbable() has returned true
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

	// Flush sending is asked for
	if ( ! quick )
	{
		_BufSock->flush();
	}

	// Disconnect and prevent from advertising the disconnection
	_BufSock->disconnect( false );

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

	// Disconnect if not done
	if ( _BufSock->Sock->connected() )
	{
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
	nlnettrace( "CClientReceiveTask::run" );

	bool connected = true;
	while ( connected ) // does not call _Sock->connected() to avoid mutex (not needed for client)
	{
		try
		{
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
				
				// TODO OPTIM remove the nldebug for speed
				//commented for optimisation nldebug( "LNETL1: Client %s received buffer (%u bytes)", _SockId->asString().c_str(), buffer.size()/*, stringFromVector(buffer).c_str()*/ );
				// Add event type
				buffer[len] = CBufNetBase::User;

				// Push message into receive queue
				_Client->pushMessageIntoReceiveQueue( buffer.getPtr(), buffer.size() );
			}
			else
			{
				nlwarning( "LNETL1: Socket %s received null length in block header", _SockId->asString().c_str() );
			}
		}
		catch ( ESocketConnectionClosed& )
		{
			nldebug( "LNETL1: Client connection %s closed", _SockId->asString().c_str() );
			connected = false;
		}
		catch ( ESocket& )
		{
			nldebug( "LNETL1: Client connection %s broken", _SockId->asString().c_str() );
			sock()->disconnect();
			connected = false;
		}
	}

	nlnettrace( "Exiting CClientReceiveTask::run()" );
}


} // NLNET
