/* socket.h
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: socket.h,v 1.2 2000/09/14 16:40:53 cado Exp $
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

	ESocket( const char *reason=NULL )
	{
		_Reason = std::string("Socket error: ") + std::string(reason);
	}
	
	virtual const char	*what() const throw() { return _Reason.c_str(); }

private:

	std::string	_Reason;
};



//typedef SOCKET;
#ifdef NL_OS_WINDOWS
	typedef uint SOCKET;
#elif NL_OS_LINUX
	typedef int SOCKET;
#endif


/**
 * Connected socket (for TCP streams)
 * \todo Advanced error handling
 * \todo What about byte swapping for message headers ???
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CSocket
{
public:

	/// Constructor
	CSocket();

	// Destructor
	//~CSocket();
	
	/// Connection
	void	connect( const CInetAddress& addr );

	/// Closure
	void	close();

	/// Sends a message
	void	send( const CMessage& message );

	/// Checks if there are some data to receive
	bool	dataAvailable();

	/// Receives data (returns false if !dataAvailable()). The capacity of the message must be large enough.
	bool	receive( CMessage& message );

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
