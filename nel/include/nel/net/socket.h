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
 * $Id: socket.h,v 1.6 2000/09/21 12:31:54 cado Exp $
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
 * \todo Advanced error handling
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CSocket : public CBaseSocket
{
public:

	/// Constructor
	CSocket();

	/// Construct a CSocket object using an already connected socket and its associated address
	CSocket( SOCKET sock, const CInetAddress& remoteaddr ) throw (ESocket);

	/// Closure
	void	close();

	/// Sets/unsets TCP_NODELAY
	void	setNoDelay( bool value ) throw (ESocket);

	/// Connection
	void	connect( const CInetAddress& addr ) throw (ESocket);

	/// Sends a message
	void	send( const CMessage& message ) throw(ESocket);

	/// Checks if there are some data to receive
	bool	dataAvailable() throw (ESocket);

	/// Receives data (returns false if !dataAvailable() and does not block).
	bool	receive( CMessage& message ) throw (ESocket);

	/// Returns the address of the remote host
	const CInetAddress& remoteAddr() const
	{
		return _RemoteAddr;
	}

private:

	CInetAddress	_RemoteAddr;
	bool			_Connected;

};

}

#endif // NL_SOCKET_H

/* End of socket.h */
