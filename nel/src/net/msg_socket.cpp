/** \file msg_socket.cpp
 * Implementation of CMsgSocket.
 * Thanks to Vianney Lecroart <lecroart@nevrax.com> and
 * Daniel Bellen <huck@pool.informatik.rwth-aachen.de> for ideas
 *
 * $Id: msg_socket.cpp,v 1.19 2000/10/24 15:35:51 lecroart Exp $
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

#include "nel/misc/debug.h"
#include "nel/net/msg_socket.h"
#include "nel/net/message.h"
#include "nel/net/naming_client.h"
#include <time.h>

using namespace std;


#ifdef NL_OS_WINDOWS

#include <winsock2.h>

#elif defined NL_OS_UNIX

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


namespace NLNET
{

bool				CMsgSocket::_Binded;
CConnections		CMsgSocket::_Connections;
TSenderId			CMsgSocket::_SenderIdNb;
long				CMsgSocket::_TimeoutS = 0;
long				CMsgSocket::_TimeoutM = 0;

const TCallbackItem	*CMsgSocket::_CallbackArray;
TTypeNum			CMsgSocket::_CbaSize;
CSearchSet			CMsgSocket::_SearchSet;

  
/*
 * Constructs a server object, listening on specified port
 */
CMsgSocket::CMsgSocket( const TCallbackItem *callbackarray, TTypeNum arraysize, uint16 port ) :
	_ClientSock( NULL )
{
	init( callbackarray, arraysize );
	CSocket *listensock = new CSocket();
	listensock->_IsListening = true;
	addNewConnection( listensock );
	listen( listensock, port );
}


const char *service_not_found_cstr = "Service not found";


/* Constructs a client object, that connects to a service. The address of the server provider
 * the service is retrieved using a Naming Service.
 */
CMsgSocket::CMsgSocket( const TCallbackItem *callbackarray, TTypeNum arraysize, const std::string& service ) :
	_ClientSock( NULL ),
	_ServiceName ( service )
{
	init( callbackarray, arraysize );
	connectToService();
	addNewConnection( _ClientSock );
}


/*
 * Constructs a client object, that connects to servaddr.
 */
CMsgSocket::CMsgSocket( const TCallbackItem *callbackarray, TTypeNum arraysize, const CInetAddress& servaddr )
{
	init( callbackarray, arraysize );
	_ClientSock = new CSocket();
	_ClientSock->_OwnerClient = this;
	_ClientSock->_IsListening = false;
	_ClientSock->connect( servaddr );
	addNewConnection( _ClientSock );
}


/* Find a service provider and connect (client mode only)
 * If the msgsocket is already connected, it is disconnected first, unless the new server found
 * is the same as the previous one.
 */
void CMsgSocket::connectToService()
{
	// Look up for service
	CInetAddress servaddr;
	if ( CNamingClient::lookup( _ServiceName, servaddr, _ValidityTime ) )
	{
		// Check if we are already connected
		if ( _ClientSock != NULL )
		{
			// If the service provider hasn't change, we keep the same connection
			if ( servaddr == _ClientSock->remoteAddr() )
			{
				time( &_ConnectTime );
				return;
			}
			// Otherwise, disconnect from the previous one
			delete _ClientSock;
		}

		// Try to connect to the new server
		bool service_ok = false;
		while ( ! service_ok )
		{
			_ClientSock = new CSocket();
			_ClientSock->_OwnerClient = this;
			_ClientSock->_IsListening = false;
			try
			{
				_ClientSock->connect( servaddr );
				time( &_ConnectTime );
				service_ok = true;
			}
			catch ( ESocketConnectionFailed& )
			{
				// If the connection failed, inform the Naming Service and try another server
				delete _ClientSock;
				if ( ! CNamingClient::lookupAlternate( _ServiceName, servaddr, _ValidityTime ) )
				{
					throw ESocket( service_not_found_cstr );
				}
			}
		}
	}
	else
	{
		throw ESocket( service_not_found_cstr );
	}
}


/*
 * Part of constructor contents
 */
void CMsgSocket::init( const TCallbackItem *callbackarray, TTypeNum arraysize )
{
	_Binded = false;
	_SenderIdNb = 0;
	_CallbackArray = callbackarray;
	_CbaSize = arraysize;
	
	const TCallbackItem *pt;
	for ( pt=callbackarray; pt<callbackarray+arraysize; pt++ )
	{
		_SearchSet.insert( CPtCallbackItem(pt) );
	}
}


/*
 * Destructor. It closes all sockets (connections) that have been created by this CMsgSocket object
 */
CMsgSocket::~CMsgSocket()
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
void CMsgSocket::listen( CSocket *listensock, uint16 port ) throw (ESocket)
{
	CInetAddress localaddr = CInetAddress::localHost();
	localaddr.setPort( port );
	listen( listensock, localaddr ); // throw (ESocket)
}


/*
 * Prepares to receive connections on a specified address/port (useful when the host has several addresses)
 */
void CMsgSocket::listen( CSocket *listensock, const CInetAddress& addr ) throw (ESocket)
{
	if ( _Binded )
	{
		return;
	}
	if ( ! addr.isValid() )
	{
		throw ESocket("Invalid address for listening");
	}

	// We use the listensock, pointing to an already constructed socket
	/* // Create a socket
	listensock->_Sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ); // IPPROTO_TCP or IPPROTO_IP (=0) ?
	if ( listensock->_Sock == INVALID_SOCKET )
	{
		throw ESocket("Server socket creation failed");
	}
	nldebug( "Socket %d open as a server socket", listensock->_Sock );
	*/

	// Bind socket to port	
	if ( ::bind( listensock->_Sock, (const sockaddr *)addr.sockAddr(), sizeof(sockaddr_in) ) != 0 )
	{
		throw ESocket("Unable to bind server socket to port");
	}
	listensock->_LocalAddr = addr;
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
	nldebug( "Socket %d listening at %s", listensock->_Sock, addr.asIPString().c_str() );
}


/*
 * Returns true if we have to find a new service provider (client mode only)
 */
bool CMsgSocket::serviceExpired()
{
	return ( difftime( time(NULL), _ConnectTime ) > _ValidityTime );
}


/*
 * Sends a message (client mode only)
 */
void CMsgSocket::send( CMessage& outmsg )
{
	if ( _ClientSock != NULL )
	{
		// Check if we must ask again for a service provider
		if ( serviceExpired() )
		{
			connectToService();
		}
		_ClientSock->send( outmsg );
	}
}


/*
 * Sends a message to the specified host id
 */
void CMsgSocket::send( CMessage& outmsg, TSenderId id )
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
 * See header file
 */
void CMsgSocket::update()
{
	// Check data available on all sockets, including the server socket
	if ( getDataAvailableStatus() )
	{
		// Iterate on the sockets where data are available
		CConnections::iterator ilps;
		for ( ilps=_Connections.begin(); ilps!=_Connections.end(); ) // we don't check the newly added connections because their flag _DataAvailable is false
		{
			bool erased = false;
			if ( (*ilps)->_DataAvailable )
			{
				if ( (*ilps)->_IsListening )
				{
					// Accept connection request
					CMessage msgout;
					CSocket& sock = accept( (*ilps)->descriptor() );
					CInetAddress addr = sock.remoteAddr();
					msgout.serial( addr );
					CMessage msgin( "C", true );
					msgin.fill( msgout.buffer(), msgout.length() );
					processReceivedMessage( msgin, sock );
				}
				else
				{
					try
					{
						// Receive message from a connected client
						CMessage msg( "", true );
						(*ilps)->receive( msg );
						if ( msgIsBinding( msg ) )
						{
							(*ilps)->processBindMessage( msg );
						}
						else 
						{
							if ( ! processReceivedMessage( msg, **ilps ) )
							{
								if ( msg.typeIsNumber() )
								{
									nlwarning( "Received a message with invalid type code %hu", msg.typeAsNumber() );
								}
								else
								{
									nlwarning( "Received a message with invalid type string %s", msg.typeAsString() );
								}
							}
						}
						// Reset flag
						(*ilps)->_DataAvailable = false;
					}
					catch ( ESocket& )
					{
						// Handle a connection closure (gracefull (if ESocketConnectionClosed) or not), when
						// receive() has thrown an exception.
						(*ilps)->close();
						CMessage msg( "D" );
						processReceivedMessage( msg, **ilps );
						if ( (*ilps)->_OwnerClient != NULL )
						{
							// If the socket is pointed by a client socket, neutralize its property _ClientSock
							(*ilps)->_OwnerClient->_ClientSock = NULL;
						}
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
 * It returns a reference on this socket object, which is maintained by the CMsgSocket object.
 * Usage : \code CSocket& sock = servsock.accept(); \endcode
 * If you don't want the server thread to block, use receive() instead.
 */
CSocket& CMsgSocket::accept( SOCKET listen_descr ) throw (ESocket)
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
	nldebug( "Socket %d accepted an incoming connection from %s and opened socket %d", listen_descr, addr.asIPString().c_str(), newsock );
	return *connection;
}


/*
 * Add a new connection socket
 */
void CMsgSocket::addNewConnection( CSocket *connection )
{
	connection->_SenderId = newSenderId();
	_Connections.push_back( connection );
}


/* Returns if the connection sockets have incoming data available.
 */
bool CMsgSocket::getDataAvailableStatus()
{
	// Put all socket descriptors in select list and find maximum descriptor number
	if ( _Connections.empty() )
	{
		return false;
	}
	else
	{
		SOCKET descmax = 0;
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
void CMsgSocket::setTimeout( uint32 ms )
{
	_TimeoutS = ms/1000;
	_TimeoutM = (_TimeoutS%1000)*1000;
}


/*
 * Returns true if msg is a binding message
 */
bool CMsgSocket::msgIsBinding( const CMessage& msg )
{
	return ( msg.typeAsString() == "B" );
}


/* Calls the good callback, and send a binding message if needed
 * \param msg [in] An input message to pass to the callback
 * \param sock [in] The socket from which the message was received
 * \return False if an error occurred (i.e. no callback defined for the message type)
 */
bool CMsgSocket::processReceivedMessage( CMessage& msg, CSocket& sock )
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
			return false;
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
			return ( (s=="C") || (s=="D") ); // the user does not have to write callback for connection/disconnection
		}

		if ( ! ((s=="C") || (s=="D") ) )
		{
			// Send a binding message
			CMessage bindmsg;
			TTypeNum num = (*its).pt() - _CallbackArray;
			bindmsg.setType( "B" );
			bindmsg.serial( s );
			bindmsg.serial( num );
			sock.send( bindmsg );
		}

		// Call the callback funtion
		callback( msg, sock._SenderId );
	}
	return true;
}


/*
 * Returns a pointer to the socket object having the specified sender id
 */
CSocket *CMsgSocket::socketFromId( TSenderId id )
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


/*
 * Returns the internet address of the listening socket (server mode only)
 */
const CInetAddress *CMsgSocket::listenAddress()
{
	CConnections::iterator ips = _Connections.begin();
	if ( (*ips)->_IsListening )
	{
		CSocket *sock = *ips;
		return &((*ips)->localAddr());
	}
	else
	{
		return NULL;
	}
}



} // NLNET
