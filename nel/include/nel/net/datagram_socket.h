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
 * $Id: datagram_socket.h,v 1.7 2000/10/04 14:34:10 cado Exp $
 *
 * Interface for CDatagramSocket
 */

#ifndef NL_DATAGRAM_SOCKET_H
#define NL_DATAGRAM_SOCKET_H

#include "nel/net/base_socket.h"


namespace NLNET {


class CMessage;


/**
 * Socket for UDP unconnected transfers
 * \bug CDatagramSocket is not compatible with the message system yet. Please do not use it :-)
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

	/** Sends a message
	 * \todo cado Update it
	 */
	void	sendTo( const CMessage& message, const CInetAddress& addr ) throw (ESocket);

	/** Receives data (returns false if !dataAvailable()).
	 * \todo cado Update it
	 */
	bool	receivedFrom( CMessage& message, CInetAddress& addr ) throw (ESocket);

private:

	bool	_Bound;
	
};


} // NLNET


#endif // NL_DATAGRAM_SOCKET_H

/* End of datagram_socket.h */
