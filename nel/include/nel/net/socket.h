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
 * $Id: socket.h,v 1.4 2000/09/19 09:14:03 cado Exp $
 *
 * Interface for CSocket
 */

#ifndef NL_SOCKET_H
#define NL_SOCKET_H


#include "nel/net/inet_address.h"


namespace NLNET
{


class CMessage;


/**
 * Network exceptions
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000

 */
class ESocket : public Exception
{
public:

	/// Constructor
	ESocket( const char *reason="", uint errnum=0 )
	{
		_Reason = std::string("Socket error: ") + std::string(reason);
		_ErrNum = errnum;
	}

	/// Returns the reason of the exception	
	virtual const char	*what() const throw() { return _Reason.c_str(); }

	/// Returns the error code
	uint				errNum()
	{
		return _ErrNum;
	}

private:

	std::string	_Reason;
	uint		_ErrNum;
};



//typedef SOCKET;
#ifdef NL_OS_WINDOWS
	typedef uint SOCKET;
#elif NL_OS_LINUX
	typedef int SOCKET;
#endif


/**
 * Client socket (for TCP connected streams). Allows to send/receive CMessage objects.
 * \todo Advanced error handling
 * \todo What about byte swapping for message headers ???
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CSocket
{
public:

	/** Initializes the network engine if it is not already done (under Windows, calls WSAStartup()).
	 * Called by CSocket constructors.
	 */
	static void init() throw (ESocket);

	/// Constructor
	CSocket();

	/// Construct a CSocket object using an already connected socket and its associated address
	CSocket( SOCKET sock, const CInetAddress& remoteaddr ) throw (ESocket);

	// Destructor
	~CSocket();
	
	/// Sets/unsets TCP_NODELAY
	void	setNoDelay( bool value ) throw (ESocket);

	/// Connection
	void	connect( const CInetAddress& addr ) throw (ESocket);

	/// Closure
	void	close();

	/// Sends a message
	void	send( const CMessage& message );

	/// Checks if there are some data to receive
	bool	dataAvailable() throw (ESocket);

	/// Receives data (returns false if !dataAvailable()).
	bool	receive( CMessage& message ) throw (ESocket);

	/// Returns the address of the remote host
	const CInetAddress& remoteAddr() const
	{
		return _RemoteAddr;
	}

private:

	static bool		_Initialized;

	SOCKET			_Sock;
	CInetAddress	_LocalAddr;
	CInetAddress	_RemoteAddr;
	bool			_Connected;

};

}

#endif // NL_SOCKET_H

/* End of socket.h */
