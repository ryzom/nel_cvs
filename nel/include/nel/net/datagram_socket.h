/* datagram_socket.h
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
 * $Id: datagram_socket.h,v 1.4 2000/09/25 16:07:27 cado Exp $
 *
 * Interface for CDatagramSocket
 */

#ifndef NL_DATAGRAM_SOCKET_H
#define NL_DATAGRAM_SOCKET_H

#include "nel/net/base_socket.h"


#ifdef NL_OS_WINDOWS
	#include <winsock2.h>
#elif defined NL_OS_LINUX
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <errno.h>
	#include <fcntl.h>
	#define SOCKET_ERROR -1
	#define INVALID_SOCKET -1
	typedef int SOCKET;
#endif


namespace NLNET {


class CMessage;


/**
 * Socket for UDP unconnected transfers
 * \test Test program is /code/test/test_rknet/main1.cpp
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CDatagramSocket : public CBaseSocket
{
public:

	/// Constructor
	CDatagramSocket() throw (ESocket);
	
	/** Binds the socket to the specified port. Call bind() if the host acts as a server and waits for
	 * messages. If the host acts as a client, call sendTo(), there is no need to bind the socket.
	 */
	void	bind( uint16 port ) throw (ESocket);

	/// Sends a message
	void	sendTo( const CMessage& message, const CInetAddress& addr ) throw (ESocket);

	/// Receives data (returns false if !dataAvailable()).
	bool	receivedFrom( CMessage& message, CInetAddress& addr ) throw (ESocket);

private:

	bool	_Bound;
	
};


} // NLNET


#endif // NL_DATAGRAM_SOCKET_H

/* End of datagram_socket.h */
