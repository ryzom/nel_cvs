/** \file buf_server.cpp
 * Network engine, layer 1, server
 *
 * $Id: buf_server.cpp,v 1.32 2002/07/02 15:56:58 lecroart Exp $
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

#include "nel/net/buf_server.h"

#ifdef NL_OS_WINDOWS
#include <winsock2.h>
//typedef sint socklen_t;

#elif defined NL_OS_UNIX
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
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
CBufServer::CBufServer( TThreadStategy strategy,
	uint16 max_threads, uint16 max_sockets_per_thread, bool nodelay, bool replaymode ) :
	CBufNetBase(),
	_ThreadStrategy( strategy ),
	_NoDelay( nodelay ),
	_MaxThreads( max_threads ),
	_MaxSocketsPerThread( max_sockets_per_thread ),
	_ConnectionCallback( NULL ),
	_ConnectionCbArg( NULL ),
	_BytesPushedOut( 0 ),
	_BytesPoppedIn( 0 ),
	_PrevBytesPoppedIn( 0 ),
	_PrevBytesPushedOut( 0 ),
	_ReplayMode( replaymode ),
	_ListenTask( NULL ),
	_ListenThread( NULL ),
	_NbConnections (0),
	_ThreadPool("CBufServer::_ThreadPool")
{
	nlnettrace( "CBufServer::CBufServer" );
	if ( ! _ReplayMode )
	{
		_ListenTask = new CListenTask( this );
		_ListenThread = IThread::create( _ListenTask );
	}
	/*{
		CSynchronized<uint32>::CAccessor syncbpi ( &_BytesPushedIn );
		syncbpi.value() = 0;
	}*/
}


/*
 * Listens on the specified port
 */
void CBufServer::init( uint16 port )
{
	nlnettrace( "CBufServer::init" );
	if ( ! _ReplayMode )
	{
		_ListenTask->init( port );
		_ListenThread->start();
	}
	else
	{
		nldebug( "LNETL0: Binding listen socket to any address, port %hu", port );
	}
}


/*
 * Begins to listen on the specified port (call before running thread)
 */
void CListenTask::init( uint16 port )
{
	nlnettrace( "CListenTask::init" );
	_ListenSock.init( port );
}


/***************************************************************************************************
 * User main thread (running)
 **************************************************************************************************/


/*
 * Constructor
 */
CServerTask::CServerTask() : _ExitRequired(false)
{
#ifdef NL_OS_UNIX
	pipe( _WakeUpPipeHandle );
#endif
}



#ifdef NL_OS_UNIX
/*
 * Wake the thread up, when blocked in select (Unix only)
 */
void CServerTask::wakeUp()
{
	uint8 b;
	if ( write( _WakeUpPipeHandle[PipeWrite], &b, 1 ) == -1 )
	{
		nldebug( "LNETL1: In CServerTask::wakeUp(): write() failed" );
	}
}
#endif


/*
 * Destructor
 */
CServerTask::~CServerTask()
{
#ifdef NL_OS_UNIX
	close( _WakeUpPipeHandle[PipeRead] );
	close( _WakeUpPipeHandle[PipeWrite] );
#endif
}


/*
 * Destructor
 */
CBufServer::~CBufServer()
{
	nlnettrace( "CBufServer::~CBufServer" );

	// Clean listen thread exit
	if ( ! _ReplayMode )
	{
		((CListenTask*)(_ListenThread->getRunnable()))->requireExit();
		((CListenTask*)(_ListenThread->getRunnable()))->close();
#ifdef NL_OS_UNIX
		_ListenTask->wakeUp();
#endif
		_ListenThread->wait();
		delete _ListenThread;
		delete _ListenTask;

		// Clean receive thread exits
		CThreadPool::iterator ipt;
		{
			nldebug( "LNETL1: Waiting for end of threads..." );
			CSynchronized<CThreadPool>::CAccessor poolsync( &_ThreadPool );
			for ( ipt=poolsync.value().begin(); ipt!=poolsync.value().end(); ++ipt )
			{
				// Tell the threads to exit and wake them up
				CServerReceiveTask *task = receiveTask(ipt);
				nlnettrace( "Requiring exit" );
				task->requireExit();

				// Wake the threads up
	#ifdef NL_OS_UNIX
				task->wakeUp();
	#else
				CConnections::iterator ipb;
				nlnettrace( "Closing sockets (Win32)" );
				{
					CSynchronized<CConnections>::CAccessor connectionssync( &task->_Connections );
					for ( ipb=connectionssync.value().begin(); ipb!=connectionssync.value().end(); ++ipb )
					{
						(*ipb)->Sock->close();
					}
				}
	#endif

			}
			
			nlnettrace( "Waiting" );
			for ( ipt=poolsync.value().begin(); ipt!=poolsync.value().end(); ++ipt )
			{
				// Wait until the threads have exited
				(*ipt)->wait();
			}

			nldebug( "LNETL1: Deleting sockets, tasks and threads..." );
			for ( ipt=poolsync.value().begin(); ipt!=poolsync.value().end(); ++ipt )
			{
				// Delete the socket objects
				CServerReceiveTask *task = receiveTask(ipt);
				CConnections::iterator ipb;
				{
					CSynchronized<CConnections>::CAccessor connectionssync( &task->_Connections );
					for ( ipb=connectionssync.value().begin(); ipb!=connectionssync.value().end(); ++ipb )
					{
						delete (*ipb);
					}
				}

	#ifdef NL_OS_UNIX
				// Under Unix, close the sockets now
				nlnettrace( "Closing sockets (Unix)" );
				{
					CSynchronized<CConnections>::CAccessor connectionssync( &task->_Connections );
					for ( ipb=connectionssync.value().begin(); ipb!=connectionssync.value().end(); ++ipb )
					{
						(*ipb)->Sock->close();
					}
				}
	#endif
				
				// Delete the task objects
				delete task;

				// Delete the thread objects
				delete (*ipt);
			}
		}
	}

	nlnettrace( "Exiting CBufServer::~CBufServer" );
}


/*
 * Disconnect the specified host
 * Set hostid to NULL to disconnect all connections.
 * If hostid is not null and the socket is not connected, the method does nothing.
 * If quick is true, any pending data will not be sent before disconnecting.
 */
void CBufServer::disconnect( TSockId hostid, bool quick )
{
	nlnettrace( "CBufServer::disconnect" );
	if ( hostid != InvalidSockId )
	{
		// Disconnect only if physically connected
		if ( hostid->Sock->connected() )
		{
			if ( ! quick )
			{
				hostid->flush();
			}
			hostid->Sock->disconnect(); // the connection will be removed by the next call of update()
		}
	}
	else
	{
		// Disconnect all
		CThreadPool::iterator ipt;
		{
			CSynchronized<CThreadPool>::CAccessor poolsync( &_ThreadPool );
			for ( ipt=poolsync.value().begin(); ipt!=poolsync.value().end(); ++ipt )
			{
				CServerReceiveTask *task = receiveTask(ipt);
				CConnections::iterator ipb;
				{
					CSynchronized<CConnections>::CAccessor connectionssync( &task->_Connections );
					for ( ipb=connectionssync.value().begin(); ipb!=connectionssync.value().end(); ++ipb )
					{
						if ( (*ipb)->Sock->connected() )
						{
							if ( ! quick )
							{
								(*ipb)->flush();
							}
							(*ipb)->Sock->disconnect();
						}
					}
				}
			}
		}
	}
}


/*
 * Send a message to the specified host
 */
void CBufServer::send( const CMemStream& buffer, TSockId hostid )
{
	nlnettrace( "CBufServer::send" );
	nlassert( buffer.length() > 0);
	nlassert( buffer.length() <= maxSentBlockSize() );

	H_AUTO (CBufServer_send);

	if ( hostid != InvalidSockId )
	{
		// debug features, we number all packet to be sure that they are all sent and received
		// \todo remove this debug feature when ok
//		nldebug ("send message number %u", hostid->SendNextValue);
#ifdef NL_BIG_ENDIAN
		uint32 val = NLMISC_BSWAP32(hostid->SendNextValue);
#else
		uint32 val = hostid->SendNextValue;
#endif

		*(uint32*)buffer.buffer() = val;
		hostid->SendNextValue++;

		pushBufferToHost( buffer, hostid );
	}
	else
	{
		// Push into all send queues
		CThreadPool::iterator ipt;
		{
			CSynchronized<CThreadPool>::CAccessor poolsync( &_ThreadPool );
			for ( ipt=poolsync.value().begin(); ipt!=poolsync.value().end(); ++ipt )
			{
				CServerReceiveTask *task = receiveTask(ipt);
				CConnections::iterator ipb;
				{
					CSynchronized<CConnections>::CAccessor connectionssync( &task->_Connections );
					for ( ipb=connectionssync.value().begin(); ipb!=connectionssync.value().end(); ++ipb )
					{
						// Send only if the socket is logically connected
						if ( (*ipb)->connectedState() ) 
						{
							// debug features, we number all packet to be sure that they are all sent and received
							// \todo remove this debug feature when ok
//							nldebug ("send message number %u", (*ipb)->SendNextValue);
#ifdef NL_BIG_ENDIAN
							uint32 val = NLMISC_BSWAP32((*ipb)->SendNextValue);
#else
							uint32 val = (*ipb)->SendNextValue;
#endif
							*(uint32*)buffer.buffer() = val;
							(*ipb)->SendNextValue++;

							pushBufferToHost( buffer, *ipb );
						}
					}
				}
			}
		}
	}
}


/*
 * Checks if there are some data to receive
 */
bool CBufServer::dataAvailable()
{
	H_AUTO (CBufServer_dataAvailable);
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
			  /*sint32 mbsize = recvfifo.value().size() / 1048576;
			  if ( mbsize > 0 )
			    {
			      nlwarning( "The receive queue size exceeds %d MB", mbsize );
			    }*/

				uint8 val = recvfifo.value().frontLast();
				
				/*vector<uint8> buffer;
				recvfifo.value().front( buffer );*/

				// Test if it the next block is a system event
				//switch ( buffer[buffer.size()-1] )
				switch ( val )
				{
					
				// Normal message available
				case CBufNetBase::User:
					return true; // return immediatly, do not extract the message

				// Process disconnection event
				case CBufNetBase::Disconnection:
				{
					vector<uint8> buffer;
					recvfifo.value().front( buffer );

					TSockId sockid = *((TSockId*)(&*buffer.begin()));
					nldebug( "LNETL1: Disconnection event for %p %s", sockid, sockid->asString().c_str());

					sockid->setConnectedState( false );

					// Call callback if needed
					if ( disconnectionCallback() != NULL )
					{
						disconnectionCallback()( sockid, argOfDisconnectionCallback() );
					}

					// Add socket object into the synchronized remove list
					nldebug( "LNETL1: Adding the connection to the remove list" );
					nlassert( ((CServerBufSock*)sockid)->ownerTask() != NULL );
					((CServerBufSock*)sockid)->ownerTask()->addToRemoveSet( sockid );
					break;
				}
				// Process connection event
				case CBufNetBase::Connection:
				{
					vector<uint8> buffer;
					recvfifo.value().front( buffer );

					TSockId sockid = *((TSockId*)(&*buffer.begin()));
					nldebug( "LNETL1: Connection event for %p %s", sockid, sockid->asString().c_str());

					sockid->setConnectedState( true );
					
					// Call callback if needed
					if ( connectionCallback() != NULL )
					{
						connectionCallback()( sockid, argOfConnectionCallback() );
					}
					break;
				}
				default:
					vector<uint8> buffer;
					recvfifo.value().front( buffer );

					nlinfo( "LNETL1: Invalid block type: %hu (should be = to %hu", (uint16)(buffer[buffer.size()-1]), (uint16)(val) );
					nlinfo( "LNETL1: Buffer (%d B): [%s]", buffer.size(), stringFromVector(buffer).c_str() );
					nlinfo( "LNETL1: Receive queue:" );
					recvfifo.value().display();
					nlerror( "LNETL1: Invalid system event type in server receive queue" );

				}

				// Extract system event
				recvfifo.value().pop();
			}
		}
		while ( true );
	}
}

 
/*
 * Receives next block of data in the specified. The length and hostid are output arguments.
 * Precond: dataAvailable() has returned true, phostid not null
 */
void CBufServer::receive( CMemStream& buffer, TSockId* phostid )
{
	nlnettrace( "CBufServer::receive" );
	//nlassert( dataAvailable() );
	nlassert( phostid != NULL );
	{
		CFifoAccessor recvfifo( &receiveQueue() );
		nlassert( ! recvfifo.value().empty() );
		recvfifo.value().front( buffer );
		recvfifo.value().pop();
	}

	// Extract hostid (and event type)
	*phostid = *((TSockId*)&(buffer.buffer()[buffer.length()-sizeof(TSockId)-1]));
	nlassert( buffer.buffer()[buffer.length()-1] == CBufNetBase::User );

	// debug features, we number all packet to be sure that they are all sent and received
	// \todo remove this debug feature when ok
#ifdef NL_BIG_ENDIAN
	uint32 val = NLMISC_BSWAP32(*(uint32*)buffer.buffer());
#else
	uint32 val = *(uint32*)buffer.buffer();
#endif

	//	nldebug ("receive message number %u", val);
	if ((*phostid)->ReceiveNextValue != val)
	{
		nlstopex (("LNETL1: !!!LOST A MESSAGE!!! I received the message number %u but I'm waiting the message number %u (cnx %s), warn lecroart@nevrax.com with the log now please", val, (*phostid)->ReceiveNextValue, (*phostid)->asString().c_str()));
		// resync the message number
		(*phostid)->ReceiveNextValue = val;
	}

	(*phostid)->ReceiveNextValue++;

	buffer.resize( buffer.length()-sizeof(TSockId)-1 );

	// TODO OPTIM remove the nldebug for speed
	//commented for optimisation nldebug( "LNETL1: Read buffer (%d+%d B) from %s", buffer.length(), sizeof(TSockId)+1, /*stringFromVector(buffer).c_str(), */(*phostid)->asString().c_str() );

	// Statistics
	_BytesPoppedIn += buffer.length() + sizeof(TBlockSize);
}


/*
 * Update the network (call this method evenly)
 */
void CBufServer::update()
{
	//nlnettrace( "CBufServer::update-BEGIN" );

	_NbConnections = 0;

	// For each thread
	CThreadPool::iterator ipt;
	{
	  //nldebug( "UPD: Acquiring the Thread Pool" );
		CSynchronized<CThreadPool>::CAccessor poolsync( &_ThreadPool );
		//nldebug( "UPD: Acquired." );
		for ( ipt=poolsync.value().begin(); ipt!=poolsync.value().end(); ++ipt )
		{
			// For each thread of the pool
			CServerReceiveTask *task = receiveTask(ipt);
			CConnections::iterator ipb;
			{
				CSynchronized<CConnections>::CAccessor connectionssync( &task->_Connections );
				for ( ipb=connectionssync.value().begin(); ipb!=connectionssync.value().end(); ++ipb )
				{
				    // For each socket of the thread, update sending
				    if ( ! ((*ipb)->Sock->connected() && (*ipb)->update()) )
				    {
						// Update did not work or the socket is not connected anymore
				        nldebug( "LNETL1: Socket %s is disconnected", (*ipb)->asString().c_str() );
						// Disconnection event if disconnected (known either from flush (in update) or when receiving data)
						(*ipb)->advertiseDisconnection( this, *ipb );
					
						/*if ( (*ipb)->advertiseDisconnection( this, *ipb ) )
						{
							// Now the connection removal is in dataAvailable()
							// POLL6
						}*/
				    }
				    else
				    {
						_NbConnections++;
				    }
				}
			}
		}
	}

	//nlnettrace( "CBufServer::update-END" );
}

uint32 CBufServer::getSendQueueSize( TSockId destid )
{
	if ( destid != InvalidSockId )
	{
		return destid->SendFifo.size();
	}
	else
	{
		// add all client buffers

		uint32 total = 0;

		// For each thread
		CThreadPool::iterator ipt;
		{
			CSynchronized<CThreadPool>::CAccessor poolsync( &_ThreadPool );
			for ( ipt=poolsync.value().begin(); ipt!=poolsync.value().end(); ++ipt )
			{
				// For each thread of the pool
				CServerReceiveTask *task = receiveTask(ipt);
				CConnections::iterator ipb;
				{
					CSynchronized<CConnections>::CAccessor connectionssync( &task->_Connections );
					for ( ipb=connectionssync.value().begin(); ipb!=connectionssync.value().end(); ++ipb )
					{
						// For each socket of the thread, update sending
						total = (*ipb)->SendFifo.size ();
					}
				}
			}
		}
		return total;
	}
}


/*
 * Returns the number of bytes received since the previous call to this method
 */
uint64 CBufServer::newBytesReceived()
{
	uint64 b = bytesReceived();
	uint64 nbrecvd = b - _PrevBytesPoppedIn;
	//nlinfo( "b: %"NL_I64"u   new: %"NL_I64"u", b, nbrecvd );
	_PrevBytesPoppedIn = b;
	return nbrecvd;
}

/*
 * Returns the number of bytes sent since the previous call to this method
 */
uint64 CBufServer::newBytesSent()
{
	uint64 b = bytesSent();
	uint64 nbsent = b - _PrevBytesPushedOut;
	//nlinfo( "b: %"NL_I64"u   new: %"NL_I64"u", b, nbsent );
	_PrevBytesPushedOut = b;
	return nbsent;
}


/***************************************************************************************************
 * Listen thread
 **************************************************************************************************/


/*
 * Code of listening thread
 */
void CListenTask::run()
{
	nlnettrace( "CListenTask::run" );

#ifdef NL_OS_UNIX
	SOCKET descmax;
	fd_set readers;
	timeval tv;
	descmax = _ListenSock.descriptor()>_WakeUpPipeHandle[PipeRead]?_ListenSock.descriptor():_WakeUpPipeHandle[PipeRead];
#endif

	// Accept connections
	while ( ! exitRequired() )
	{
		try
		{
			// Get and setup the new socket
#ifdef NL_OS_UNIX
			FD_ZERO( &readers );
			FD_SET( _ListenSock.descriptor(), &readers );
			FD_SET( _WakeUpPipeHandle[PipeRead], &readers );
			tv.tv_sec = 60; /// \todo ace: we perhaps could put NULL to never wake up the select (look at the select man page)
			tv.tv_usec = 0;
			int res = ::select( descmax+1, &readers, NULL, NULL, &tv );

			switch ( res )
			{
			case  0 : continue; // time-out expired, no results
			case -1 :
				// we'll ignore message (Interrupted system call) caused by a CTRL-C
				if (CSock::getLastError() == 4)
				{
					nldebug ("LNETL1: Select failed (in listen thread): %s (code %u) but IGNORED", CSock::errorString( CSock::getLastError() ).c_str(), CSock::getLastError());
					continue;
				}
				nlerror( "LNETL1: Select failed (in listen thread): %s (code %u)", CSock::errorString( CSock::getLastError() ).c_str(), CSock::getLastError() );
			}

			if ( FD_ISSET( _WakeUpPipeHandle[PipeRead], &readers ) )
			{
				uint8 b;
				if ( read( _WakeUpPipeHandle[PipeRead], &b, 1 ) == -1 ) // we were woken-up by the wake-up pipe
				{
					nldebug( "LNETL1: In CListenTask::run(): read() failed" );
				}
				nldebug( "LNETL1: listen thread select woken-up" );
				continue;
			}
#endif
			nldebug( "LNETL1: Incoming connection..." );
			CServerBufSock *bufsock = new CServerBufSock( _ListenSock.accept() );
			nldebug( "New connection : %s", bufsock->asString().c_str() );
			bufsock->Sock->setNonBlockingMode( true );
			if ( _Server->noDelay() )
			{
				bufsock->Sock->setNoDelay( true );
			}

			// Notify the new connection
			bufsock->advertiseConnection( _Server );

			// Dispatch the socket into the thread pool
			_Server->dispatchNewSocket( bufsock );
		}
		catch ( ESocket& e )
		{
			nlinfo( "Exception in listen thread: %s", e.what() ); // It can occur in normal behavior (e.g. when exiting)
			// It can also occur when too many sockets are open (e.g. 885 connections)
		}
	}

	nlnettrace( "Exiting CListenTask::run" );
}


/*
 * Binds a new socket and send buffer to an existing or a new thread
 * Note: this method is called in the listening thread.
 */
void CBufServer::dispatchNewSocket( CServerBufSock *bufsock )
{
	nlnettrace( "CBufServer::dispatchNewSocket" );

	CSynchronized<CThreadPool>::CAccessor poolsync( &_ThreadPool );
	if ( _ThreadStrategy == SpreadSockets )	
	{
		// Find the thread with the smallest number of connections and check if all
		// threads do not have the same number of connections
		uint min = 0xFFFFFFFF;
		uint max = 0;
		CThreadPool::iterator ipt, iptmin, iptmax;
		for ( ipt=poolsync.value().begin(); ipt!=poolsync.value().end(); ++ipt )
		{
			uint noc = receiveTask(ipt)->numberOfConnections();
			if ( noc < min )
			{
				min = noc;
				iptmin = ipt;
			}
			if ( noc > max )
			{
				max = noc;
				iptmax = ipt;
			}
		}

		// Check if we make the pool of threads grow (if we have not found vacant room
		// and if it is allowed to)
		if ( (poolsync.value().empty()) ||
			 ((min == max) && (poolsync.value().size() < _MaxThreads)) )
		{
			addNewThread( poolsync.value(), bufsock );
		}
		else
		{
			// Dispatch socket to an existing thread of the pool
			CServerReceiveTask *task = receiveTask(iptmin);
			bufsock->setOwnerTask( task );
			task->addNewSocket( bufsock );
#ifdef NL_OS_UNIX
			task->wakeUp();
#endif			
			
			if ( min >= (uint)_MaxSocketsPerThread )
			{
				nlwarning( "LNETL1: Exceeding the maximum number of sockets per thread" );
			}
			nldebug( "LNETL1: New socket dispatched to thread %d", iptmin-poolsync.value().begin() );
		}

	}
	else // _ThreadStrategy == FillThreads
	{
		CThreadPool::iterator ipt;
		for ( ipt=poolsync.value().begin(); ipt!=poolsync.value().end(); ++ipt )
		{
			uint noc = receiveTask(ipt)->numberOfConnections();
			if ( noc < _MaxSocketsPerThread )
			{
				break;
			}
		}

		// Check if we have to make the thread pool grow (if we have not found vacant room)
		if ( ipt == poolsync.value().end() )
		{
			if ( poolsync.value().size() == _MaxThreads )
			{
				nlwarning( "LNETL1: Exceeding the maximum number of threads" );
			}
			addNewThread( poolsync.value(), bufsock );
		}
		else
		{
			// Dispatch socket to an existing thread of the pool
			CServerReceiveTask *task = receiveTask(ipt);
			bufsock->setOwnerTask( task );
			task->addNewSocket( bufsock );
#ifdef NL_OS_UNIX
			task->wakeUp();
#endif			
			nldebug( "LNETL1: New socket dispatched to thread %d", ipt-poolsync.value().begin() );
		}
	}
}


/*
 * Creates a new task and run a new thread for it
 * Precond: bufsock not null
 */
void CBufServer::addNewThread( CThreadPool& threadpool, CServerBufSock *bufsock )
{
	nlnettrace( "CBufServer::addNewThread" );
	nlassert( bufsock != NULL );

	// Create new task and dispatch the socket to it
	CServerReceiveTask *task = new CServerReceiveTask( this );
	bufsock->setOwnerTask( task );
	task->addNewSocket( bufsock );

	// Add a new thread to the pool, with this task
	IThread *thr = IThread::create( task );
	{
		threadpool.push_back( thr );
		thr->start();
		nldebug( "LNETL1: Added a new thread; pool size is %d", threadpool.size() );
		nldebug( "LNETL1: New socket dispatched to thread %d", threadpool.size()-1 );
	}
}


/***************************************************************************************************
 * Receive threads
 **************************************************************************************************/


/*
 * Code of receiving threads for servers
 */
void CServerReceiveTask::run()
{
	nlnettrace( "CServerReceiveTask::run" );

	SOCKET descmax;
	fd_set readers;

	// Time-out value for select (it can be long because we do not do any thing else in this thread)
	timeval tv;
#if defined NL_OS_UNIX
	// POLL7
	nice( 2 );
#endif // NL_OS_UNIX
	
	// Copy of _Connections
	vector<TSockId>	connections_copy;	

	while ( ! exitRequired() )
	{
		// 1. Remove closed connections
		clearClosedConnections();

		// POLL8

		// 2-SELECT-VERSION : select() on the sockets handled in the present thread

		descmax = 0;
		FD_ZERO( &readers );
		bool skip;
		bool alldisconnected = true;
		CConnections::iterator ipb;
		{
			// Lock _Connections
			CSynchronized<CConnections>::CAccessor connectionssync( &_Connections );

			// Prepare to avoid select if there is no connection
			skip = connectionssync.value().empty();

			// Fill the select array and copy _Connections
			connections_copy.clear();
			for ( ipb=connectionssync.value().begin(); ipb!=connectionssync.value().end(); ++ipb )
			{
				if ( (*ipb)->Sock->connected() ) // exclude disconnected sockets that are not deleted
												 // Note: there is a mutex in there !
				{
					alldisconnected = false;
					// Copy _Connections element
					connections_copy.push_back( *ipb );

					// Add socket descriptor to the select array
					FD_SET( (*ipb)->Sock->descriptor(), &readers );

					// Calculate descmax for select
					if ( (*ipb)->Sock->descriptor() > descmax )
					{
						descmax = (*ipb)->Sock->descriptor();
					}
				}
			}

#ifdef NL_OS_UNIX
			// Add the wake-up pipe into the select array
			FD_SET( _WakeUpPipeHandle[PipeRead], &readers );
			if ( _WakeUpPipeHandle[PipeRead]>descmax )
			{
				descmax = _WakeUpPipeHandle[PipeRead];
			}
#endif
			
			// Unlock _Connections, use connections_copy instead
		}

#ifndef NL_OS_UNIX
		// Avoid select if there is no connection (Windows only)
		if ( skip || alldisconnected )
		{
			nlSleep( 1 ); // nice
			continue;
		}
#endif

#ifdef NL_OS_WINDOWS
		tv.tv_sec = 0; // short time because the newly added connections can't be added to the select fd_set
		tv.tv_usec = 10000; // NEW: set to 500ms because otherwise new connections handling are too slow
#elif defined NL_OS_UNIX
		// POLL7
		tv.tv_sec = 3600;		// 1 hour (=> 1 select every 3.6 second for 1000 connections)
		tv.tv_usec = 0;
#endif // NL_OS_WINDOWS

		// Call select
		int res = ::select( descmax+1, &readers, NULL, NULL, &tv );

		// POLL9

		// 3. Test the result
		switch ( res )
		{
			case  0 : continue; // time-out expired, no results

			/// \todo cado: the error code is not properly retrieved
			case -1 :
				// we'll ignore message (Interrupted system call) caused by a CTRL-C
				/*if (CSock::getLastError() == 4)
				{
					nldebug ("LNETL1: Select failed (in receive thread): %s (code %u) but IGNORED", CSock::errorString( CSock::getLastError() ).c_str(), CSock::getLastError());
					continue;
				}*/
				//nlerror( "LNETL1: Select failed (in receive thread): %s (code %u)", CSock::errorString( CSock::getLastError() ).c_str(), CSock::getLastError() );
				nldebug( "LNETL1: Select failed (in receive thread): %s (code %u)", CSock::errorString( CSock::getLastError() ).c_str(), CSock::getLastError() );
				return;
		}

		// 4. Get results

		vector<TSockId>::iterator ic;
		for ( ic=connections_copy.begin(); ic!=connections_copy.end(); ++ic )
		{
			if ( FD_ISSET( (*ic)->Sock->descriptor(), &readers ) != 0 )
			{
				CServerBufSock *serverbufsock = static_cast<CServerBufSock*>(static_cast<CBufSock*>(*ic));
				try
				{
					// 4. Receive data
					if ( serverbufsock->receivePart() )
					{
						// Copy sockid
						vector<uint8> hidvec;
						hidvec.resize( sizeof(TSockId)+1 );
						memcpy( &*hidvec.begin(), &(*ic), sizeof(TSockId) );

						// Add event type to hidvec
						hidvec[sizeof(TSockId)] = (uint8)CBufNetBase::User;

						// Push message into receive queue
						//uint32 bufsize;
						//sint32 mbsize;
						{
							//nldebug( "RCV: Acquiring the receive queue... ");
							CFifoAccessor recvfifo( &_Server->receiveQueue() );
							//nldebug( "RCV: Acquired, pushing the received buffer... ");
							recvfifo.value().push( serverbufsock->receivedBuffer(), hidvec );
							//nldebug( "RCV: Pushed, releasing the receive queue..." );
							//recvfifo.value().display();
							//bufsize = serverbufsock->receivedBuffer().size();
							//mbsize = recvfifo.value().size() / 1048576;
						}
						//nldebug( "RCV: Released." );
						/*if ( mbsize > 1 )
						{
							nlwarning( "The receive queue size exceeds %d MB", mbsize );
						}*/
						/*
						// Statistics
						{
							CSynchronized<uint32>::CAccessor syncbpi ( &_Server->syncBytesPushedIn() );
							syncbpi.value() += bufsize;
						}
						*/
					}
				}
				catch ( ESocketConnectionClosed& )
				{
					nldebug( "LNETL1: Connection %s closed", serverbufsock->asString().c_str() );
				}
				catch ( ESocket& )
				{
					nldebug( "LNETL1: Connection %s broken", serverbufsock->asString().c_str() );
					(*ic)->Sock->disconnect();
				}
/*
#ifdef NL_OS_UNIX
				skip = true; // don't check _WakeUpPipeHandle (yes, check it to read any written byte)
#endif

*/
			}

		}

#ifdef NL_OS_UNIX
		// Test wake-up pipe
		if ( (!skip) && (FD_ISSET( _WakeUpPipeHandle[PipeRead], &readers )) )
		{
			uint8 b;
			if ( read( _WakeUpPipeHandle[PipeRead], &b, 1 ) == -1 ) // we were woken-up by the wake-up pipe
			{
				nldebug( "LNETL1: In CServerReceiveTask::run(): read() failed" );
			}
			nldebug( "LNETL1: Receive thread select woken-up" );
		}
#endif
		
	}
	nlnettrace( "Exiting CServerReceiveTask::run" );
}


/*
 * Delete all connections referenced in the remove list (double-mutexed)
 */

void CServerReceiveTask::clearClosedConnections()
{
	CConnections::iterator ic;
	{
		NLMISC::CSynchronized<CConnections>::CAccessor removesetsync( &_RemoveSet );
		{
			if ( ! removesetsync.value().empty() )
			{
				// Delete closed connections
				NLMISC::CSynchronized<CConnections>::CAccessor connectionssync( &_Connections );
				for ( ic=removesetsync.value().begin(); ic!=removesetsync.value().end(); ++ic )
				{
					nldebug( "LNETL1: Removing a connection" );

					TSockId sid = (*ic);

					// Remove from the connection list
					connectionssync.value().erase( *ic );

					// Delete the socket object
					delete sid;
				}
				// Clear remove list
				removesetsync.value().clear();
			}
		}
	}
}


} // NLNET
