/** \file msg_socket.h
 * Implementation of CMsgSocket.
 * Thanks to Vianney Lecroart <lecroart@nevrax.com> and
 * Daniel Bellen <huck@pool.informatik.rwth-aachen.de> for ideas
 *
 * $Id: msg_socket.h,v 1.28 2000/12/14 10:52:21 cado Exp $
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
#include <string>


namespace NLNET
{

class CInetAddress;
class CMessage;


/// List of connections
typedef std::map<TSenderId,CSocket*> CConnections;

/// List of iterators on connections
typedef std::list<CConnections::iterator> CConnectionIterators;

/// Set of "special pointers to callback items"
typedef std::set<CPtCallbackItem> CSearchSet;


// Exception EDuplicateMsgName
class EDuplicateMsgName : public ESocket
{
public:
	EDuplicateMsgName( char *msgname )
	{
		_Reason = "Duplicate message name in callback array: " + std::string(msgname);
		_ErrNum = 0;
	}
};


/**
 * A message socket object allows to send/receive messages to/from connected sockets.
 * 
 * It has two modes, depending on the constructor called: client and server.
 * One object represents either a client object (which is connected to one server),
 * or a server object (which has a listening socket plus one socket per client that
 * connects). A list of all connections (CSocket objects) is maintained staticly.
 * Only one "select()" is done for all CMsgSocket objects.
 * The non-static methods are specific to the client mode.
 *
 * To receive messages, you have to create some callbacks that will be called
 * when you call CMsgSocket::update() (which you are supposed to call every frame).
 * The following callback names are reserved : "C", "D" and "O".
 * Please refer to the documentation of update() to know how to use it.
 *
 * \sa network Network system overview
 * \todo Cado: Replace search for (TSenderId from) in the list by something faster ?
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
	 * \param port Port on which the server must listen and accept connections. If you
	 * don't want to choose a port, you can call CNamingClient::queryServicePort() before)
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

	/** Adds another client callback array
	 * Please call this method before any update() or send()
	 */
	void			addClientCallbackArray( const TCallbackItem *callbackarray, TTypeNum arraysize );

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

	/// Forbids to call a callback that is not the specified callback, for the specified connection
	static void		authorizeOnly( TMsgCallback callback, TSenderId idfrom );

	/// Allows to call any callback, for the specified connection
	static void		authorizeAll( TSenderId idfrom );

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

	/** Updates the connected sockets and accepts new connections.
	 * - When a new connection incomes (server mode only), the callback of name "C" is called if it exists. Its message contains the address of the remote socket (CInetAddress).
	 *
	 * - When a message is received, the callback of name msgTypeAsString() or of index msgTypeAsNumber() is called.
	 * If it doesn't exist, the callback of name "O" (for others) is called it it exists (and the input message
	 * is the received message, thus in the callback you have to check the type of the message, which can be either a number
	 * or a string), otherwise a warning is emitted.
	 *
	 * - When a connection is closed, the callback of name "D" is called if it exists. The message is empty.
	 *
	 * When a message with a name (as a string) is received, a binding message is replied to the sender, so that
	 * next time it sends this type of message, the type is represented by a message type code.
	 */
	static void		update();

	/// Sets timeout for receive() in milliseconds
	static void		setTimeout( uint32 ms );

	/// Returns the internet address of the listening socket (server mode only)
	static const CInetAddress	*listenAddress();

	/// Returns the address of a connected host
	static const CInetAddress	*addressFromId( TSenderId id );

	/// Returns the local address (client mode only)
	const CInetAddress&			localAddr() const
	{
		//nlassert( _ClientSock!=NULL );
		return _ClientSock->localAddr();
	}

	/// Returns the remote address (client mode only)
	const CInetAddress&			remoteAddr() const
	{
		//nlassert( _ClientSock!=NULL );
		return _ClientSock->remoteAddr();
	}

protected:

	/// Part of constructor contents
	void			init( const TCallbackItem *callbackarray, TTypeNum arraysize, bool clientmode );

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

	/// Calls the callback corresponding to "O" if it exists, otherwise returns false
	static bool		callCallbackForOthers( CMessage& msg, CSocket& sock );

	/// Returns if the listening socket of the server and the connection sockets have incoming data available.
	static bool		getDataAvailableStatus();

	/// Returns a pointer to the socket object having the specified sender id
	static CSocket	*socketFromId( TSenderId id );

	/// Returns an iterator to the socket pointer in the list of connections, with the specified sender id
	static CConnections::iterator	iteratorFromId( TSenderId id );

	/// Returns a pointer to a client socket or NULL if id does not correspond to a client socket
	static CMsgSocket				*clientSocket( TSenderId id );

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

	// Client socket
	CSocket						*_ClientSock;

	// Statistics for client mode
	uint32						_PrevBytesReceivedFromHost;
	uint32						_PrevBytesSentToHost;

	// Members for client mode
	std::string					_ServiceName;
	time_t						_ConnectTime;
	uint16						_ValidityTime;

	// Callbacks per client
	TCallbackItem				*_ClientCallbackArray;
	TTypeNum					_ClientCbaSize;
	CSearchSet					_ClientSearchSet;
	bool						_Allocated;

	// True if the listening socket is bound
	static bool					_Binded;

	// The list of connections
	static CConnections			_Connections;

	// Number of allocated sender ids
	static TSenderId			_SenderIdNb;

	// List of connections that are to delete
	static CConnectionIterators	_ConnectionsToDelete;	

	// If true, receive more than 1 message per update()
	static bool					_ReceiveAll;

	// Number of milliseconds to wait in receive(). The higher, the nicer for the speed of the rest of the system.
	static long					_TimeoutS, _TimeoutM;

	// Callback management
	static const TCallbackItem	*_CallbackArray;
	static TTypeNum				_CbaSize;
	static CSearchSet			_SearchSet;

	// Statistics for server mode
	static uint32				_PrevBytesReceived;
	static uint32				_PrevBytesSent;

};

}

#endif // NL_SERVER_SOCKET_H

/* End of msg_socket.h */
