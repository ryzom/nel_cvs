/** \file udp_sock.h
 * Network engine, layer 0, udp socket
 *
 * $Id: udp_sock.h,v 1.8 2005/02/22 10:14:13 besson Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
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

#ifndef NL_UDP_SOCK_H
#define NL_UDP_SOCK_H

#include "sock.h"


namespace NLNET {


/**
 * CUdpSock: Unreliable datagram socket via UDP.
 * See base class CSock.
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000-2001
 */
class CUdpSock : public CSock
{
public:

	/// @name Socket setup
	//@{

	/**
	 * Constructor.
	 * \param logging Disable logging if the server socket object is used by the logging system, to avoid infinite recursion
	 */
	CUdpSock( bool logging = true );

	/** Binds the socket to the specified port. Call bind() for an unreliable socket if the host acts as a server and expects to receive
	 * messages. If the host acts as a client, call directly sendTo(), in this case you need not bind the socket.
	 */
	void				bind( uint16 port );

	/// Same as bind(uint16) but binds on a specified address/port (useful when the host has several addresses)
	void				bind( const CInetAddress& addr );

	//@}

	/// @name Receiving data
	//@{

	/**  Receives data from the peer. (blocking function)
	 * The socket must be pseudo-connected.
	 */
	bool				receive( uint8 *buffer, uint32& len, bool throw_exception=true );

	/** Receives data and say who the sender is. (blocking function)
	 * The socket must have been bound before, by calling either bind() or sendTo().
	 * \param buffer [in] Address of buffer
	 * \param len [in/out] Requested length of buffer, and actual number of bytes received
	 * \param addr [out] Address of sender
	 */
	bool				receivedFrom( uint8 *buffer, uint& len, CInetAddress& addr, bool throw_exception=true );

	//@}
	

	/// @name Sending data
	//@{

	/// Sends data to the specified host (unreliable sockets only)
	void				sendTo( const uint8 *buffer, uint len, const CInetAddress& addr );

	//@}

private:

	/// True after calling bind() or sendTo()
	bool				_Bound;

};


} // NLNET


#endif // NL_UDP_SOCK_H

/* End of udp_sock.h */
