/* server_socket.cpp
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
 * $Id: msg_socket.cpp,v 1.8 2000/10/02 16:42:23 cado Exp $
 *
 * Implementation of CServerSocket.
 * Thanks to Vianney Lecroart <lecroart@nevrax.com> and
 * Daniel Bellen <huck@pool.informatik.rwth-aachen.de> for ideas
 */

#include "nel/net/server_socket.h"
#include "nel/net/message.h"
#include "nel/misc/log.h"
extern NLMISC::CLog Log;

using namespace std;


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

using namespace std;


namespace NLNET
{

bool				CServerSocket::_Binded;
CConnections		CServerSocket::_Connections;
TSenderId			CServerSocket::_SenderIdNb;
long				CServerSocket::_TimeoutS = 0;
long				CServerSocket::_TimeoutM = 0;

TCallbackItem		*CServerSocket::_CallbackArray;
TTypeNum			CServerSocket::_CbaSize;
CSearchSet			CServerSocket::_SearchSet;

  
/*
 * Constructs a server object, listening on specified port
 */
CServerSocket::CServerSocket( TCallbackItem *callbackarray, TTypeNum arraysize, uint16 port ) :
	_ClientSock( NULL )
{
	init( callbackarray, arraysize );
	CSocket *listensock = new CSocket();
	listensock->_IsListening = true;
	addNewConnection( listensock );
	listen( listensock, port );
}


/*
 * Constructs a client object, that connects to servaddr.
 */
CServerSocket::CServerSocket( TCallbackItem *callbackarray, TTypeNum arraysize, const CInetAddress& servaddr )
{
	init( callbackarray, arraysize );
	_ClientSock = new CSocket();
	_ClientSock->_IsListening = false;
	_ClientSock->connect( servaddr );
	addNewConnection( _ClientSock );
}


/*
 * Part of constructor contents
 */
void CServerSocket::init( TCallbackItem *callbackarray, TTypeNum arraysize )
{
	_Binded = false;
	_SenderIdNb = 0;
	_CallbackArray = callbackarray;
	_CbaSize = arraysize;
	
	TCallbackItem *pt;
	for ( pt=callbackarray; pt<callbackarray+arraysize; pt++ )
	{
		_SearchSet.insert( CPtCallbackItem(pt) );
	}
}


/*
 * Destructor. It closes all sockets (connections) that have been created by this CServerSocket object
 */
CServerSocket::~CServerSocket()
{
	CConnections::iterator its;
	for ( its=_Connections.begin(); its!=_Connections.end(); its++ )
	{
		delete *its;
	}
}


/*
 * Prepares to receive connections on a specified port
 */
void CServerSocket::listen( CSocket *listensock, uint16 port ) throw (ESocket)
{
	CInetAddress localaddr = CInetAddress::localHost();
	localaddr.setPort( port );
	listen( listensock, localaddr ); // throw (ESocket)
}


/*
 * Prepares to receive connections on a specified address/port (useful when the host has several addresses)
 */
void CServerSocket::listen( CSocket *listensock, const CInetAddress& addr ) throw (ESocket)
{
	if ( _Binded )
	{
		return;
	}
	if ( ! addr.isValid() )
	{
		throw ESocket("Invalid address for listening");
	}

	// Create a socket
	listensock->_Sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ); // IPPROTO_TCP or IPPROTO_IP (=0) ?
	if ( listensock->_Sock == INVALID_SOCKET )
	{
		throw ESocket("Server socket creation failed");
	}
	Log.display( "Socket %d open as a server socket\n", listensock->_Sock );

	// Bind socket to port	
	if ( ::bind( listensock->_Sock, (const sockaddr *)addr.sockAddr(), sizeof(sockaddr_in) ) != 0 )
	{
		throw ESocket("Unable to bind server socket to port");
	}
	_Binded = true;

	// Retrieve socket error code (is this really necessary ?)
	int errcode=0;
	int errlen=sizeof(errcode);
	getsockopt( listensock->_Sock, SOL_SOCKET, SO_ERROR, (char*)&errcode, &errlen );
	if ( errcode != 0 )
	{
		throw ESocket("Server socket raised an error after binding");
	}

	// Listen
	if ( ::listen( listensock->_Sock, SOMAXCONN ) != 0 ) // SOMAXCONN = maximum length of the queue of pending connections
	{
		throw ESocket("Unable to listen on specified port");
	}
	Log.display( "Socket %d listening at %s\n", listensock->_Sock, addr.asIPString().c_str() );
}


/*
 * Send a message (client mode only)
 */
void CServerSocket::send( CMessage& outmsg )
{
	if ( _ClientSock != NULL )
	{
		_ClientSock->send( outmsg );
	}
}


/*
 * Send a message to the specified host id
 */
void CServerSocket::send( CMessage& outmsg, TSenderId id )
{
	CSocket *sock = socketFromId( id );
	if ( sock != NULL )
	{
		sock->send( outmsg );
	}
	else
	{
		throw ESocket("Invalid host id");
	}
}


/*
 *
 */
void CServerSocket::receive()
{
	// Check data available on all sockets, including the server socket
	if ( getDataAvailableStatus() )
	{
		// Iterate on the sockets where data are available
		bool erased = false;
		CConnections::iterator ilps;
		for ( ilps=_Connections.begin(); ilps!=_Connections.end(); )
		{
			if ( (*ilps)->_DataAvailable )
			{
				if ( (*ilps)->_IsListening )
				{
					// Accept connection request
					CMessage msg;
					CSocket& sock = accept( (*ilps)->descriptor() );
					msg.setType( "C" );
					msg.serial( sock.remoteAddr().hostName() ); // add serial() to CInetAddress ?
					uint16 port = sock.remoteAddr().port();
					msg.serial( port );
					processReceivedMessage( msg, sock );
				}
				else
				{
					try
					{
						// Receive message from a connected client
						CMessage msg( true );
						(*ilps)->receive( msg );
						if ( msgIsBinding( msg ) )
						{
							(*ilps)->processBindMessage( msg );
						}
						else 
						{
							processReceivedMessage( msg, **ilps );
						}
						// Reset flag
						(*ilps)->_DataAvailable = false;
					}
					catch ( ESocket& )
					{
						// Handle a connection closure (gracefull or not), when
						// receive() has thrown an exception. Note: this could be done by boolean result
						(*ilps)->close();
						CMessage msg;
						msg.setType( "D" );
						processReceivedMessage( msg, **ilps );
						delete (*ilps);
						ilps = _Connections.erase( ilps );
						erased = true;
					}
				}
			}
			if ( !erased )
			{
				ilps++;
			}
		}
	}
}



/* Wait for a client to connect, then creates a new socket connected to the client, and adds it to the list of connections.
 * It returns a reference on this socket object, which is maintained by the CServerSocket object.
 * Usage : \code CSocket& sock = servsock.accept(); \endcode
 * If you don't want the server thread to block, use receive() instead.
 */
CSocket& CServerSocket::accept( SOCKET listen_descr ) throw (ESocket)
{
	// Accept connection
	sockaddr_in saddr;
	sint saddrlen = sizeof(saddr);
	SOCKET newsock = ::accept( listen_descr, (sockaddr*)&saddr, &saddrlen );
	if ( newsock == INVALID_SOCKET )
	{
		throw ESocket( "accept return an invalid socket");
	}

	// Construct and save a CSocket object
	CInetAddress addr;
	addr.setSockAddr( &saddr );
	CSocket *connection = new CSocket( newsock, addr );
	addNewConnection( connection );
	Log.display( "Socket %d accepted an incoming connection from %s and opened socket %d\n", listen_descr, addr.asIPString().c_str(), newsock );
	return *connection;
}


/*
 * Add a new connection socket
 */
void CServerSocket::addNewConnection( CSocket *connection )
{
	connection->_SenderId = newSenderId();
	_Connections.push_back( connection );
}


/* Returns if the connection sockets have incoming data available.
 */
bool CServerSocket::getDataAvailableStatus()
{
	// Put all socket descriptors in select list and find maximum descriptor number
	if ( _Connections.empty() )
	{
		return false;
	}
	else
	{
		SOCKET descmax;
		fd_set readers, writers;
		FD_ZERO (&readers);
		FD_ZERO (&writers);
		CConnections::iterator itps;
		for ( itps=_Connections.begin(); itps!=_Connections.end(); itps++ )
		{
			FD_SET( (*itps)->descriptor(), &readers );
			if ( (*itps)->descriptor() > descmax )
			{
				descmax = (*itps)->descriptor();
			}
		}

		// Do select
		timeval tv;
		tv.tv_sec = _TimeoutS;
		tv.tv_usec = _TimeoutM;
		int res = select( descmax+1, &readers, NULL, NULL, &tv );
		switch ( res  )
		{
			case  0 : return false;
			case -1 : throw ESocket("getDataAvailableStatus(): select failed"); return false;
		}
		
		// Get results
		for ( itps = _Connections.begin(); itps!=_Connections.end(); itps++ )
		{
			(*itps)->_DataAvailable = (FD_ISSET( (*itps)->descriptor(), &readers ) != 0);
		}
		return true;
	}
}


/*
 * Sets timeout for receive() in milliseconds
 */
void CServerSocket::setTimeout( uint32 ms )
{
	_TimeoutS = ms/1000;
	_TimeoutM = (_TimeoutS%1000)*1000;
}


/*
 * Returns true if msg is a binding message
 */
bool CServerSocket::msgIsBinding( const CMessage& msg )
{
	return ( msg.typeAsString() == "B" );
}


/* Calls the good callback, and send a binding message if needed
 * \param msg [in] An input message to pass to the callback
 * \param sock [in] The socket from which the message was received
 */
void CServerSocket::processReceivedMessage( CMessage& msg, CSocket& sock )
{
	if ( msg.typeIsNumber() )
	{
		TTypeNum num = msg.typeAsNumber();
		if ( num < _CbaSize )
		{
			// Call the callback by index
			_CallbackArray[num].Callback( msg, sock._SenderId );
		}
		else
		{
			throw EMessageTypeNbr();
		}
	}
	else
	{
		// Get the callback by key string
		TMsgCallback callback;
		string s = msg.typeAsString();
		CSearchSet::iterator its = _SearchSet.find( CPtCallbackItem( s.c_str() ) );
		if ( its != _SearchSet.end() )
		{
			callback = (*its).pt()->Callback;
		}
		else
		{
			if ( (s=="C") || (s=="D") ) // the user does not have to write callback for connection/disconnection
			{
				return;
			}
			else
			{
				throw EMessageTypeStr();
			}
		}

		// Send a binding message
		CMessage bindmsg;
		TTypeNum num = (*its).pt() - _CallbackArray;
		bindmsg.setType( "B" );
		bindmsg.serial( s );
		bindmsg.serial( num );
		sock.send( bindmsg );

		// Call the callback funtion
		callback( msg, sock._SenderId );
	}
}


/*
 * Returns a pointer to the socket object having the specified sender id
 */
CSocket *CServerSocket::socketFromId( TSenderId id )
{
	CConnections::iterator itps;
	for ( itps=_Connections.begin(); itps!=_Connections.end(); itps++ )
	{
		if ( (*itps)->_SenderId == id )
		{
			return *itps;
		}
	}
	return NULL;
}


}