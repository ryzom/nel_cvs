/** \file buf_net_base.h
 * Network engine, layer 1, base
 *
 * $Id: buf_net_base.h,v 1.11.8.1 2003/08/12 16:47:10 cado Exp $
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

#ifndef NL_BUF_NET_BASE_H
#define NL_BUF_NET_BASE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/mutex.h"
#include "nel/misc/buf_fifo.h"
#include "nel/misc/thread.h"
#include "nel/misc/debug.h"
#include "nel/misc/common.h"

namespace NLNET {


class CBufSock;

/// Socket identifier
typedef CBufSock *TSockId;

static const TSockId InvalidSockId = (TSockId) NULL;

/// Callback function for message processing
typedef void (*TNetCallback) ( TSockId from, void *arg );

/// Storing a TNetCallback call for future call
typedef std::pair<TNetCallback,TSockId> TStoredNetCallback;

/// Synchronized FIFO buffer
typedef NLMISC::CSynchronized<NLMISC::CBufFIFO> CSynchronizedFIFO;

/// Accessor of mutexed FIFO buffer
typedef CSynchronizedFIFO::CAccessor CFifoAccessor;

/// Size of a block
typedef uint32 TBlockSize;

extern uint32 	NbNetworkTask;

/**
 * Layer 1
 *
 * Base class for CBufClient and CBufServer.
 * The max block sizes for sending and receiving are controlled by setMaxSentBlockSize()
 * and setMaxExpectedBlockSize(). Their default value is the maximum number contained in a sint32,
 * that is 2^31-1 (i.e. 0x7FFFFFFF). The limit for sending is checked only in debug mode.
 *
 * \author Nevrax France
 * \date 2001
 */
class CBufNetBase
{
public:

	/// Type of incoming events (max 256)
	enum TEventType { User = 'U', Connection = 'C', Disconnection = 'D' };

	/// Destructor
	virtual ~CBufNetBase() {};

	/// Sets callback for detecting a disconnection (or NULL to disable callback)
	void	setDisconnectionCallback( TNetCallback cb, void* arg ) { _DisconnectionCallback = cb; _DisconnectionCbArg = arg; }

	/// Returns the size of the receive queue (mutexed)
	uint32	getReceiveQueueSize()
	{
		CFifoAccessor syncfifo( &_RecvFifo );
		return syncfifo.value().size();
	}

	void displayReceiveQueueStat (NLMISC::CLog *log = NLMISC::InfoLog)
	{
		CFifoAccessor syncfifo( &_RecvFifo );
		syncfifo.value().displayStats(log);
	}
	
	/**
	 * Sets the max size of the received messages.
	 * If receiving a message bigger than the limit, the connection will be dropped.
	 *
	 * Default value: 1 MegaByte
	 * If you put a negative number as limit, the max size is reset to the default value.
	 * Warning: you can call this method only at initialization time, before connecting (for a client)
	 * or calling init() (for a server) !
	 */
	void	setMaxExpectedBlockSize( sint32 limit )
	{
		if ( limit < 0 )
			_MaxExpectedBlockSize = 1048576;
		else
			_MaxExpectedBlockSize = (uint32)limit;
	}

	/**
	 * Sets the max size of the sent messages.
	 * Note: Limiting of sending not implemented, currently
	 *
	 * Default value: 1 MegaByte
	 * If you put a negative number as limit, the max size is reset to the default value.
	 * Warning: you can call this method only at initialization time, before connecting (for a client)
	 * or calling init() (for a server) !
	 */
	void	setMaxSentBlockSize( sint32 limit )
	{
		if ( limit < 0 )
			_MaxSentBlockSize = 1048576;
		else
			_MaxSentBlockSize = (uint32)limit;
	}

	/// Returns the max size of the received messages (default: 2^31-1)
	uint32	maxExpectedBlockSize() const
	{
		return _MaxExpectedBlockSize;
	}

	/// Returns the max size of the sent messages (default: 2^31-1)
	uint32	maxSentBlockSize() const
	{
		return _MaxSentBlockSize;
	}

protected:

	friend class NLNET::CBufSock;
	
	/// Constructor
	CBufNetBase();

	/// Access to the receive queue
	CSynchronizedFIFO& receiveQueue() { return _RecvFifo; }

	/// Returns the disconnection callback
	TNetCallback	disconnectionCallback() const { return _DisconnectionCallback; }

	/// Returns the argument of the disconnection callback
	void*			argOfDisconnectionCallback() const { return _DisconnectionCbArg; }

	/// Push message into receive queue (mutexed)
	// TODO OPTIM never use this function
	void	pushMessageIntoReceiveQueue( const std::vector<uint8>& buffer )
	{
		//sint32 mbsize;
		{
			//nldebug( "BNB: Acquiring the receive queue... ");
			CFifoAccessor recvfifo( &_RecvFifo );
			//nldebug( "BNB: Acquired, pushing the received buffer... ");
			recvfifo.value().push( buffer );
			//nldebug( "BNB: Pushed, releasing the receive queue..." );
			//mbsize = recvfifo.value().size() / 1048576;
			setDataAvailableFlag( true );
		}
		//nldebug( "BNB: Released." );
		//if ( mbsize > 1 )
		//{
		//	nlwarning( "The receive queue size exceeds %d MB", mbsize );
		//}
	}

	void	pushMessageIntoReceiveQueue( const uint8 *buffer, uint32 size)
	{
		//sint32 mbsize;
		{
			//nldebug( "BNB: Acquiring the receive queue... ");
			CFifoAccessor recvfifo( &_RecvFifo );
			//nldebug( "BNB: Acquired, pushing the received buffer... ");
			recvfifo.value().push( buffer, size );
			//nldebug( "BNB: Pushed, releasing the receive queue..." );
			//mbsize = recvfifo.value().size() / 1048576;
			setDataAvailableFlag( true );
		}
		//nldebug( "BNB: Released." );
		/*if ( mbsize > 1 )
		{
			nlwarning( "The receive queue size exceeds %d MB", mbsize );
		}*/
	}

	/// Sets _DataAvailable
	void				setDataAvailableFlag( bool da ) { _DataAvailable = da; }

	/// Return _DataAvailable
	volatile bool		dataAvailableFlag() const { return _DataAvailable; }

private:

	/// The receive queue, protected by a mutex-like device
	CSynchronizedFIFO	_RecvFifo;

	/// True if there is data available (avoids locking a mutex)
	volatile bool		_DataAvailable;

	/// Callback for disconnection
	TNetCallback		_DisconnectionCallback;

	/// Argument of the disconnection callback
	void*				_DisconnectionCbArg;

	/// Max size of received messages (limited by the user)
	uint32				_MaxExpectedBlockSize;

	/// Max size of sent messages (limited by the user)
	uint32				_MaxSentBlockSize;
};


} // NLNET


#endif // NL_BUF_NET_BASE_H

/* End of buf_net_base.h */
