/* base_socket.h
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
 * $Id: base_socket.h,v 1.3 2000/09/25 11:14:23 cado Exp $
 *
 * Interface of CBaseSocket
 */

#ifndef NL_BASE_SOCKET_H
#define NL_BASE_SOCKET_H


#include "nel/net/inet_address.h"


namespace NLNET {


/**
 * Network exceptions. Note: this exception class is called ESocket and not EBaseSocket
 * but it is used by CSocket, CDatagramSocket and CServerSocket.
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
 * Base class for CSocket (TCP) and CDatagramSocket (UDP).
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CBaseSocket
{
public:

	/** Initializes the network engine if it is not already done (under Windows, calls WSAStartup()).
	 * Called by CSocket constructors.
	 */
	static void init() throw (ESocket);

	/// Constructor
	CBaseSocket();

	/// Construct a CSocket object using an already connected socket descriptor 
	CBaseSocket( SOCKET sock ) throw (ESocket);

	/// Destructor
	~CBaseSocket();

	/// Closure
	void				close();

	/// Checks if there are some data to receive
	bool				dataAvailable() throw (ESocket);

	/// Returns a const reference on the local address
	const CInetAddress&	localAddr() const
	{
		return _LocalAddr;
	}

	/// Returns the socket descriptor
	SOCKET				descriptor()
	{
		return _Sock;
	}

protected:

	/// Sets the local address
	void	setLocalAddress();

	SOCKET			_Sock;
	CInetAddress	_LocalAddr;

private:

	static bool		_Initialized;

};


} // NLNET


#endif // NL_BASE_SOCKET_H

/* End of base_socket.h */
