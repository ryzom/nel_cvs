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
 * $Id: base_socket.h,v 1.11 2000/10/09 14:09:03 cado Exp $
 *
 * Interface of CBaseSocket
 */

#ifndef NL_BASE_SOCKET_H
#define NL_BASE_SOCKET_H

#include "nel/net/inet_address.h"


namespace NLNET {


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

protected:

	std::string	_Reason;
	uint		_ErrNum;
};


/// Exception raised when a connection is gracefully closed by peer
class ESocketConnectionClosed : public ESocket
{
public:
	/// Constructor
	ESocketConnectionClosed()
	{
		_Reason = "Connection closed";
		_ErrNum = 0;
	}
};


//typedef SOCKET;
#ifdef NL_OS_WINDOWS
	typedef uint SOCKET;
#elif defined NL_OS_LINUX
	typedef int SOCKET;
#endif



/**
 * CBaseSocket allows to send/receive data on a network. An object can be a stream-oriented socket which
 * can connect to a host and use TCP, or a datagram-oriented socket using UDP.
 * This class implements layer 1 of the NeL Network communication subsystem.
 *
 * The "logging" boolean value is necessary because in this implementation we always log
 * to one single global CLog object : there is not one CLog object per socket. Therefore
 * we must prevent the socket used in CNetDisplayer from logging itself... otherwise we
 * would have an infinite recursion.
 *
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

	
	/// @name Socket setup
	//@{

	/**
	 * Constructor.
	 * \param reliable If true, creates a connected socket using TCP, otherwise an unconnected socket using UDP
	 * \param logging Disable logging if the server socket object is used by the logging system, to avoid infinite recursion
	 */
	CBaseSocket( bool reliable = true, bool logging = true );

	/// Construct a CSocket object using an already connected socket descriptor and its associated remote address
	CBaseSocket( SOCKET sock, const CInetAddress& remoteaddr ) throw (ESocket);

	/** Connection (reliable sockets only).
	 * If the socket is unreliable, it does not connect but saves the remote address so that next calls to
	 * send() do the same as sendTo with addr as an argument.
	 */
	void				connect( const CInetAddress& addr ) throw (ESocket);

	/** Binds the socket to the specified port. Call bind() for an unreliable socket if the host acts as a server and expects to receive
	 * messages. If the host acts as a client, call directly sendTo(), in this case you need not bind the socket.
	 */
	void				bind( uint16 port ) throw (ESocket);

	/// Sets/unsets TCP_NODELAY (reliable sockets only)
	void				setNoDelay( bool value ) throw (ESocket);

	/// Closure
	void				close();

	/// Destructor
	~CBaseSocket();

	//@}

	
	/// @name Receiving data
	//@{

	/// Checks if there are some data to receive
	bool				dataAvailable() throw (ESocket);

	/// Receives data, or blocks if !dataAvailable()). Returns false if !connected().
	bool				receive( uint8 *buffer, uint len ) throw (ESocket);

	/// Receives data (returns false if !dataAvailable() and does not block).
	bool				received( uint8 *buffer, uint len ) throw (ESocket);

	/** Receives data (returns false if !dataAvailable()) (unreliable sockets only).
	 * The socket must have been bound before, by calling either bind() or sendTo().
	 * \param buffer [in] Address of buffer
	 * \param len [in] Length of buffer
	 * \param addr [out] Address of sender
	 */
	bool				receivedFrom( uint8 *buffer, uint len, CInetAddress& addr ) throw (ESocket);

	//@}

	
	/// @name Sending data
	//@{

	/// Sends data to the specified host (unreliable sockets only)
	void				sendTo( const uint8 *buffer, uint len, const CInetAddress& addr ) throw (ESocket);

	/// Sends a message
	void				send( const uint8 *buffer, uint len ) throw (ESocket);

	//@}

	
	/// @name Properties
	//@{

	/// Returns if the socket is reliable
	bool				reliable() const { return _Reliable; }

	/// Returns if the socket is connected
	bool				connected() const {	return _Connected; }

	/// Returns a const reference on the local address
	const CInetAddress&	localAddr() const {	return _LocalAddr; }

	/// Returns the address of the remote host
	const CInetAddress&	remoteAddr() const { return _RemoteAddr; }

	/// Returns the socket descriptor
	SOCKET				descriptor() const { return _Sock; }

	//@}

protected:

	/// Sets the local address
	void			setLocalAddress();

	/// Receives data
	void			doReceive( uint8 *buffer, uint len );

	SOCKET			_Sock;

	CInetAddress	_LocalAddr;
	CInetAddress	_RemoteAddr;

	bool			_Reliable;
	bool			_Logging;
	bool			_Bound;
	bool			_Connected;


private:

	static bool		_Initialized;

};


} // NLNET


#endif // NL_BASE_SOCKET_H

/* End of base_socket.h */
