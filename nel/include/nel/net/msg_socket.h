/** \file msg_socket.h
 * Implementation of CMsgSocket.
 * Thanks to Vianney Lecroart <lecroart@nevrax.com> and
 * Daniel Bellen <huck@pool.informatik.rwth-aachen.de> for ideas
 *
 * $Id: msg_socket.h,v 1.21 2000/11/20 15:51:49 cado Exp $
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

#ifndef NL_SERVER_SOCKET_H
#define NL_SERVER_SOCKET_H

#include "nel/net/base_socket.h"
#include "nel/net/socket.h"
#include "nel/net/pt_callback_item.h"
#include <list>
#include <set>


namespace NLNET
{

class CInetAddress;
class CMessage;


/// List of connections
typedef std::list<CSocket*> CConnections;

/// List of iterators on connections
typedef std::list<CConnections::iterator> CConnectionIterators;


/// Set of "special pointers to callback items"
typedef std::set<CPtCallbackItem> CSearchSet;


/**
 * A message socket object allows to send/receive messages to/from connected sockets.
 * It has two modes, depending on the constructor called: client and server.
 *
 * A CMsgSocket maintains a list of connections (CSocket objects).
 *
 * Call CMsgSocket::receive() every frame.
 * Several methods and members are static, so that only one "select" is done for all message sockets.
 *
 * \todo cado Replace (TSenderId from) by something faster
 * \todo cado Add a way to prevent for sending binding messages
 * \test Test program is /code/test/test_rknet/main1.cpp
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CMsgSocket
{
public:

	/** Constructs a server object, listening on specified port.
	 * \param callbackarray Define this array statically.
	 * \param arraysize Use sizeof(callbackarray)-sizeof(callbackarray[0])
	 * \param port Port on which the server must listen and accept connections
	 */
	CMsgSocket( const TCallbackItem *callbackarray, TTypeNum arraysize, uint16 port );

	/** Constructs a client object, that connects to a service. The address of the server provider
	 * the service is retrieved using a Naming Service.
	 * \param callbackarray Define this array statically.
	 * \param arraysize Use sizeof(callbackarray)-sizeof(callbackarray[0])
	 * \param service Name of the service
	 */
	CMsgSocket( const TCallbackItem *callbackarray, TTypeNum arraysize, const std::string& service );

	/** Constructs a client object, that connects to servaddr. 
	 * \param callbackarray Define this array statically.
	 * \param arraysize Use sizeof(callbackarray)-sizeof(callbackarray[0])
	 * \param servaddr Address of the server
	 */
	CMsgSocket( const TCallbackItem *callbackarray, TTypeNum arraysize, const CInetAddress& servaddr );

	/// Destructor. It closes all sockets (connections) that have been created by this CMsgSocket object
	~CMsgSocket();

	/// Sets the socket in "receive all" mode
	static void		setReceiveAllMode( bool all )
	{
		_ReceiveAll = all;
	}

	/// Returns the "receive all" mode state
	static bool		receiveAllMode()
	{
		return _ReceiveAll;
	}

	/// Send an outpput message (client mode only)
	void			send( CMessage& outmsg );

	/** Returns true if the client is still connected (client mode only).
	 * It becomes false when the remote server disconnects the client.
	 */
	bool			connected()
	{
		return (_ClientSock != NULL);
	}

	/// Send an output message to the specified host id
	static void		send( CMessage& outmsg, TSenderId id );

	/// Send an output message to all connected hosts
	static void		sendToAll( CMessage& outmsg );

	/// Send an output message to all connected hosts except the one with the specified excluded id
	static void		sendToAllExceptHost( CMessage& outmsg, TSenderId excluded );

	/// Returns the number of connected hosts
	static uint32	numberOfConnections()
	{
		return _Connections.size();
	}

	///@name Statistics
	//@{

	/// Returns the number of bytes received from the host since the beginning
	uint32						bytesReceivedFromHost();

	/// Returns the number of bytes sent to the host since the beginning
	uint32						bytesSentToHost();

	/// Returns the number of bytes downloaded since the previous call to this method
	uint						newBytesReceivedFromHost();

	/// Returns the number of bytes uploaded since the previous call to this method
	uint						newBytesSentToHost();

	/// Returns the number of bytes received since the beginning
	static uint32				bytesReceived();

	/// Returns the number of bytes sent since the beginning
	static uint32				bytesSent();

	/// Returns the number of bytes downloaded since the previous call to this method
	static uint					newBytesReceived();

	/// Returns the number of bytes uploaded since the previous call to this method
	static uint					newBytesSent();

	//@}

	/** Updates the connected sockets and accept new connections.
	 * - When a new connection incomes (server mode only), the callback of name "C" is called if it exists. Its message contains the address of the remote socket (CInetAddress).
	 *
	 * - When a message is received, the callback of name msgTypeAsString() or of index msgTypeAsNumber() is called. An exception is raised it doesn't exist.
	 *
	 * - When a connection is closed, the callback of name "D" is called if it exists. The message is empty.
	 *
	 * When a message with a name (as a string) is received, a binding message is replied to the sender, so that
	 * next time it sends this type of message, the type is represented by a message type code.
	 *
	 * At present, update() processes only 1 received message at a time by connection.
	 */
	static void		update();

	/// Sets timeout for receive() in milliseconds
	static void		setTimeout( uint32 ms );

	/// Returns the internet address of the listening socket (server mode only)
	static const CInetAddress	*listenAddress();

protected:

	/// Part of constructor contents
	void			init( const TCallbackItem *callbackarray, TTypeNum arraysize );

	/// Prepares to receive connections on a specified port
	void			listen( CSocket *listensock, uint16 port ) throw (ESocket);

	/// Prepares to receive connections on a specified address/port (useful when the host has several addresses)
	void			listen( CSocket *listensock, const CInetAddress& addr ) throw (ESocket);

	/** Wait for a client to connect, then creates a new socket connected to the client, and adds it to the list of connections.
	 * It returns a reference on this socket object, which is maintained by the CMsgSocket object.
	 * Usage : \code CSocket& sock = servsock.accept(); \endcode
	 */
	static CSocket&	accept( SOCKET listen_descr ) throw (ESocket);

	/// Add a new connection socket
	static void		addNewConnection( CSocket *connection );

	/// Handle a connection closure (graceful or not)
	static void		handleConnectionClosure( const CConnections::iterator& ilps );

	/// Returns true if msg is a binding message
	static bool		msgIsBinding( const CMessage& msg );

	/** Calls the good callback, and send a binding message if needed
	 * \param msg [in] An input message to pass to the callback
	 * \param sock [in] The socket from which the message was received
	 * \return False if an error occurred (i.e. no callback defined for the message type)
	 */
	static bool		processReceivedMessage( CMessage& msg, CSocket& sock );

	/// Returns if the listening socket of the server and the connection sockets have incoming data available.
	static bool		getDataAvailableStatus();

	/// Returns a pointer to the socket object having the specified sender id
	static CSocket	*socketFromId( TSenderId id );

	/// Returns an iterator to the socket pointer in the list of connections, with the specified sender id
	static CConnections::iterator	iteratorFromId( TSenderId id );

	/// Gets new sender id
	static TSenderId newSenderId()
	{
		TSenderId sid = _SenderIdNb;
		_SenderIdNb++;
		return sid;
	}

	/// Returns true if we have to find a new service provider (client mode only)
	bool			serviceExpired();

	/** Find a service provider and connect (client mode only)
	 * If the msgsocket is already connected, it is disconnected first unless the new server found
	 * is the same as the previous one.
	 * \todo Cado: When the naming service tells us to connect to a new server, don't disconnect
	 * from the previous one (in order to receive potential responses) but arm a timeout.
	 */
	void			connectToService();

private:

	CSocket						*_ClientSock;
	uint32						_PrevBytesReceivedFromHost;
	uint32						_PrevBytesSentToHost;
	std::string					_ServiceName;
	time_t						_ConnectTime;
	uint16						_ValidityTime;

	static bool					_Binded;
	static CConnections			_Connections;
	static TSenderId			_SenderIdNb;
	static CConnectionIterators	_ConnectionsToDelete;	

	static bool					_ReceiveAll;

	/// Number of milliseconds to wait in receive(). The higher, the nicer for the speed of the rest of the system.
	static long					_TimeoutS, _TimeoutM;


	static const TCallbackItem	*_CallbackArray;
	static TTypeNum				_CbaSize;
	static CSearchSet			_SearchSet;

	static uint32				_PrevBytesReceived;
	static uint32				_PrevBytesSent;
};

}

#endif // NL_SERVER_SOCKET_H

/* End of msg_socket.h */
