/** \file dummy_tcp_sock.cpp
 * Dummy TCP socket, used ny the message recorder for playback
 *
 * $Id: dummy_tcp_sock.cpp,v 1.5 2001/12/28 10:17:21 lecroart Exp $
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

#include "nel/net/dummy_tcp_sock.h"

using namespace NLMISC;


namespace NLNET {


/*
 * Set only the remote address
 */
void CDummyTcpSock::connect( const CInetAddress& addr )
{
	_RemoteAddr = addr;
	_Sock = 100;

	_BytesReceived = 0;
	_BytesSent = 0;

	//CSynchronized<bool>::CAccessor sync( &_SyncConnected );
	//sync.value() = true;
	_Connected = true;

	nldebug( "LNETL0: Socket connected to %s", addr.asString().c_str() );
}


/*
 *Dummy disconnection
 */
void CDummyTcpSock::disconnect()
{
	nldebug( "LNETL0: Socket disconnecting from %s...", _RemoteAddr.asString().c_str() );

	//CSynchronized<bool>::CAccessor sync( &_SyncConnected );
	//sync.value() = false;
	_Connected = false;
}



} // NLNET
