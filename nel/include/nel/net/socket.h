/* socket.h
 *
 * Copyright, 2000 Nevrax Ltd.
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

/*
 * $Id: socket.h,v 1.8 2000/09/25 15:01:47 cado Exp $
 *
 * Interface for CSocket
 */

#ifndef NL_SOCKET_H
#define NL_SOCKET_H


#include "nel/net/base_socket.h"

namespace NLNET
{


class CMessage;


/**
 * Client socket (for TCP connected streams). Allows to send/receive CMessage objects.
 * Note: there are two methods for receiving : receive() which is blocking, and
 * received() which is non-blocking.
 *
 * The "logging" boolean value is necessary because in this implementation we always log
 * to one single global CLog object : there is not one CLog object per socket. Therefore
 * we must prevent the socket used in CNetDisplayer from logging itself... otherwise we
 * would have an infinite recursion.
 *
 * \test /code/test/test_rknet/main1.cpp
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CSocket : public CBaseSocket
{
public:

	/// Constructor. Disable logging if the server socket object is used by the logging system.
	CSocket( bool logging = true );

	/// Construct a CSocket object using an already connected socket and its associated address
	CSocket( SOCKET sock, const CInetAddress& remoteaddr ) throw (ESocket);

	/// Closure
	void	close();

	/// Sets/unsets TCP_NODELAY
	void	setNoDelay( bool value ) throw (ESocket);

	/// Connection
	void	connect( const CInetAddress& addr ) throw (ESocket);

	/// Returns if the socket is connected
	bool	connected() const
	{
		return _Connected;
	}

	/// Sends a message
	void	send( const CMessage& message ) throw(ESocket);

	/// Checks if there are some data to receive
	bool	dataAvailable() throw (ESocket);

	/// Receives data, or blocks if !dataAvailable()). Returns false if !connected().
	bool	receive( CMessage& message ) throw (ESocket);

	/// Receives data (returns false if !dataAvailable() and does not block).
	bool	received( CMessage& message ) throw (ESocket);


	/// Returns the address of the remote host
	const CInetAddress& remoteAddr() const
	{
		return _RemoteAddr;
	}

protected:

	/// Helper method for receive() and received()
	void	doReceive( CMessage& message ) throw (ESocket);

private:

	CInetAddress	_RemoteAddr;
	bool			_Connected;
	bool			_Logging;

};

}

#endif // NL_SOCKET_H

/* End of socket.h */
