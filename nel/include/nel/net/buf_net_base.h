/** \file buf_net_base.h
 * Network engine, layer 1, base
 *
 * $Id: buf_net_base.h,v 1.2 2001/05/18 13:58:00 cado Exp $
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
class CReceiveTask;

/// Socket identifier
typedef CBufSock *TSockId;

/// Callback function for message processing
typedef void (*TNetCallback) ( TSockId from, void *arg );

/// Storing a TNetCallback call for future call
typedef std::pair<TNetCallback,TSockId> TStoredNetCallback;

/// Synchronized FIFO buffer
typedef NLMISC::CSynchronized<NLMISC::CBufFIFO> CSynchronizedFIFO;

/// Accessor of mutexed FIFO buffer
typedef CSynchronizedFIFO::CAccessor CFifoAccessor;

/// Size of a block
typedef uint16 TBlockSize;


/**
 * Layer 1
 *
 * Base class for CBufClient and CBufServer
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CBufNetBase
{
public:

	/// Type of incoming events (max 256)
	enum TEventType { User, Connection, Disconnection };

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

protected:

	friend CBufSock;
	
	/// Constructor
	CBufNetBase();

	/// Access to the receive queue
	CSynchronizedFIFO& receiveQueue() { return _RecvFifo; }

	/// Returns the disconnection callback
	TNetCallback	disconnectionCallback() const { return _DisconnectionCallback; }

	/// Returns the argument of the disconnection callback
	void*			argOfDisconnectionCallback() const { return _DisconnectionCbArg; }

	/// Push message into receive queue (mutexed)
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
		}
		//nldebug( "BNB: Released." );
		/*if ( mbsize > 1 )
		{
			nlwarning( "The receive queue size exceeds %d MB", mbsize );
		}*/
	}

private:

	CSynchronizedFIFO	_RecvFifo;

	TNetCallback							_DisconnectionCallback;
	void*									_DisconnectionCbArg;

};


} // NLNET


#endif // NL_BUF_NET_BASE_H

/* End of buf_net_base.h */
