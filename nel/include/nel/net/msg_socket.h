/* msg_socket.h
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
 * $Id: msg_socket.h,v 1.12 2000/10/09 14:09:03 cado Exp $
 *
 * Interface for CMsgSocket
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
 * \todo cado Replace (TSenderId from) by something faster;
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
	CMsgSocket( TCallbackItem *callbackarray, TTypeNum arraysize, uint16 port );

	/** Constructs a client object, that connects to servaddr. 
	 * \param callbackarray Define this array statically.
	 * \param arraysize Use sizeof(callbackarray)-sizeof(callbackarray[0])
	 * \param port Address of the server
	 */
	CMsgSocket( TCallbackItem *callbackarray, TTypeNum arraysize, const CInetAddress& servaddr );

	/// Destructor. It closes all sockets (connections) that have been created by this CMsgSocket object
	~CMsgSocket();

	/// Send a message (client mode only)
	void			send( CMessage& outmsg );

	/** Returns true if the client is still connected (client mode only).
	 * It becomes false when the remote server disconnects the client.
	 */
	bool			connected()
	{
		return (_ClientSock != NULL);
	}

	/// Send a message to the specified host id
	static void		send( CMessage& outmsg, TSenderId id );

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

protected:

	/// Part of constructor contents
	void			init( TCallbackItem *callbackarray, TTypeNum arraysize );

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

	/// Returns if the listening socket of the server and the connection sockets have incoming data available.
	static bool		getDataAvailableStatus();

	/// Returns true if msg is a binding message
	static bool		msgIsBinding( const CMessage& msg );

	/** Calls the good callback, and send a binding message if needed
	 * \param msg [in] An input message to pass to the callback
	 * \param sock [in] The socket from which the message was received
	 */
	static void		processReceivedMessage( CMessage& msg, CSocket& sock );

	/// Returns a pointer to the socket object having the specified sender id
	static CSocket	*socketFromId( TSenderId id );

	/// Gets new sender id
	static TSenderId newSenderId()
	{
		TSenderId sid = _SenderIdNb;
		_SenderIdNb++;
		return sid;
	}

private:

	CSocket						*_ClientSock;

	static bool					_Binded;
	static CConnections			_Connections;
	static TSenderId			_SenderIdNb;

	/// Number of milliseconds to wait in receive(). The higher, the nicer for the speed of the rest of the system.
	static long					_TimeoutS, _TimeoutM;


	static TCallbackItem		*_CallbackArray;
	static TTypeNum				_CbaSize;
	static CSearchSet			_SearchSet;

};

}

#endif // NL_SERVER_SOCKET_H

/* End of msg_socket.h */
