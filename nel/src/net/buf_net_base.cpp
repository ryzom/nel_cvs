/** \file buf_net_base.cpp
 * Network engine, layer 1, base
 *
 * $Id: buf_net_base.cpp,v 1.14 2004/05/07 12:56:21 cado Exp $
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

#include "nel/net/buf_net_base.h"

using namespace NLMISC;
using namespace std;


namespace NLNET {

uint32 	NbNetworkTask = 0;

/***************************************************************************************************
 * User main thread 
 **************************************************************************************************/

 
/*
 * Constructor
 */
CBufNetBase::CBufNetBase() :
	_RecvFifo("CBufNetBase::_RecvFifo"),
	_DataAvailable( false ),
	_DisconnectionCallback( NULL ),
	_DisconnectionCbArg( NULL ),
	_MaxExpectedBlockSize( 10485760 ), // 10M
	_MaxSentBlockSize( 10485760 )
{
	// Debug info for mutexes
#ifdef MUTEX_DEBUG
	initAcquireTimeMap();
#endif
}


/*
 * Push message into receive queue (mutexed)
 * TODO OPTIM never use this function
 */
void	CBufNetBase::pushMessageIntoReceiveQueue( const std::vector<uint8>& buffer )
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
#ifdef NL_OS_UNIX
	// Wake-up main thread (outside the critical section of CFifoAccessor, to allow main thread to be
	// read the fifo; if the main thread sees the Data Available flag is true but the pipe not written
	// yet, it will block on read()).
	uint8 b=0;
	if ( write( _DataAvailablePipeHandle[PipeWrite], &b, 1 ) == -1 )
	{
		nlwarning( "LNETL1: Write pipe failed in pushMessageIntoReceiveQueue" );
	}
#endif
	//nldebug( "BNB: Released." );
	//if ( mbsize > 1 )
	//{
	//	nlwarning( "The receive queue size exceeds %d MB", mbsize );
	//}
}

/*
 * Push message into receive queue (mutexed)
 */
void	CBufNetBase::pushMessageIntoReceiveQueue( const uint8 *buffer, uint32 size )
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
#ifdef NL_OS_UNIX
		// Wake-up main thread
		uint8 b=0;
		if ( write( _DataAvailablePipeHandle[PipeWrite], &b, 1 ) == -1 )
		{
			nlwarning( "LNETL1: Write pipe failed in pushMessageIntoReceiveQueue" );
		}
#endif
	}
	//nldebug( "BNB: Released." );
	/*if ( mbsize > 1 )
	{
		nlwarning( "The receive queue size exceeds %d MB", mbsize );
	}*/
}



NLMISC_VARIABLE(uint32, NbNetworkTask, "Number of server and client thread");
	
} // NLNET
