/** \file msg_socket.cpp
 * Implementation of CMsgSocket.
 * Thanks to Vianney Lecroart <lecroart@nevrax.com> and
 * Daniel Bellen <huck@pool.informatik.rwth-aachen.de> for ideas
 *
 * $Id: msg_socket.cpp,v 1.30 2000/11/24 11:22:13 cado Exp $
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

#define ERROR_NUM WSAGetLastError()

typedef sint socklen_t;


#elif defined NL_OS_UNIX

#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>

#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define ERROR_NUM errno

typedef int SOCKET;

#endif


namespace NLNET
{

bool					CMsgSocket::_Binded;
CConnections			CMsgSocket::_Connections;
CConnectionIterators	CMsgSocket::_ConnectionsToDelete;
TSenderId				CMsgSocket::_SenderIdNb;
long					CMsgSocket::_TimeoutS = 0;
long					CMsgSocket::_TimeoutM = 0;

bool					CMsgSocket::_ReceiveAll;

const TCallbackItem		*CMsgSocket::_CallbackArray;
TTypeNum				CMsgSocket::_CbaSize;
CSearchSet				CMsgSocket::_SearchSet;

uint32					CMsgSocket::_PrevBytesReceived = 0;
uint32					CMsgSocket::_PrevBytesSent = 0;

  
/*
 * Constructs a server object, listening on specified port
 */
CMsgSocket::CMsgSocket( const TCallbackItem *callbackarray, TTypeNum arraysize, uint16 port ) :
	_ClientSock( NULL )
{
	init( callbackarray, arraysize );
	CSocket *listensock = new CSocket();
	listensock->setListening( true );
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
	nldebug("Trying to connect to the service \"%s\"", service.c_str());
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
	_ClientSock->setOwnerClient( this );
	_ClientSock->setListening( false );
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
			// Otherwise, disconnect from the previous one (note: it breaks any current transaction)
			delete _ClientSock;
		}

		// Try to connect to the new server
		bool service_ok = false;
		while ( ! service_ok )
		{
			_ClientSock = new CSocket();
			_ClientSock->setOwnerClient( this );
			_ClientSock->setListening( false );
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
	_ReceiveAll = true;
	_PrevBytesReceivedFromHost = 0;
	_PrevBytesSentToHost = 0;
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
/*	int errcode=0;
	int errlen=sizeof(errcode);
	getsockopt( listensock->_Sock, SOL_SOCKET, SO_ERROR, (char*)&errcode, &errlen );
	if ( errcode != 0 )
	{
		throw ESocket("Server socket raised an error after binding");
	}
*/
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
		try
		{
			_ClientSock->send( outmsg );
		}
		catch ( ESocket& )
		{
			CConnections::iterator ilps = _Connections.begin();
			ilps++; // no need to test the first one which is the listening socket
			handleConnectionClosure( find( ilps, _Connections.end(), _ClientSock ) );
		}
	}
}


/*
 * Sends a message to the specified host id
 */
void CMsgSocket::send( CMessage& outmsg, TSenderId id )
{
	CConnections::iterator ilps = iteratorFromId( id );
	if ( (*ilps) != NULL )
	{
		try
		{
			(*ilps)->send( outmsg );
		}
		catch ( ESocket& )
		{
			handleConnectionClosure( ilps );
		}
	}
	else
	{
		throw ESocket("Invalid host id");
	}
}


/*
 * Send an output message to all connected hosts
 */
void CMsgSocket::sendToAll( CMessage& outmsg )
{
	CConnections::iterator ilps = _Connections.begin();
	try
	{
		for ( ilps++; ilps!=_Connections.end(); ++ilps ) // not including the first one which is the listening socket
		{
			(*ilps)->send( outmsg );
		}
	}
	catch ( ESocket& )
	{
		handleConnectionClosure( ilps );
	}
}


/*
 * Send an output message to all connected hosts except the one with the specified id
 */
void CMsgSocket::sendToAllExceptHost( CMessage& outmsg, TSenderId excluded )
{
	CConnections::iterator ilps = _Connections.begin();
	try
	{
		for ( ilps++; ilps!=_Connections.end(); ++ilps ) // not including the first one which is the listening socket
		{
			if ( (*ilps)->senderId() != excluded )
			{
				(*ilps)->send( outmsg );
			}
		}
	}
	catch ( ESocket& )
	{
		handleConnectionClosure( ilps );
	}

}




/*
 * See header file
 */
void CMsgSocket::update()
{
#ifdef NL_DEBUG
	uint nb_recvd = 0;
	uint bytes_recvd = 0;
#endif

	// Check data available on all sockets, including the server socket
	while ( getDataAvailableStatus() )
	{
		// Iterate on the sockets where data are available
		CConnections::iterator ilps;
		for ( ilps=_Connections.begin(); ilps!=_Connections.end(); ++ilps ) // we don't check the newly added connections because their flag dataAvailable() is false
		{
			if ( (*ilps)->dataAvailable() )
			{
				if ( (*ilps)->isListening() )
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
#ifdef NL_DEBUG
						nb_recvd++;
						bytes_recvd += msg.length();
#endif
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
									nlwarning( "Received a message with invalid type string %s", msg.typeAsString().c_str() );
								}
							}
						}
						// Reset flag
						(*ilps)->setDataAvailableFlag( false );
					}
					catch ( ESocket& )
					{
						// Handle a connection closure (gracefull (if ESocketConnectionClosed) or not), when
						// receive() has thrown an exception.
						handleConnectionClosure( ilps );
					}
				}
			}
		}
		// Delete closed sockets
		CConnectionIterators::iterator iilps;
		for ( iilps=_ConnectionsToDelete.begin(); iilps!=_ConnectionsToDelete.end(); ++iilps )
		{
			delete *(*iilps);
			_Connections.erase( *iilps );
		}
		_ConnectionsToDelete.clear();

#ifdef NL_DEBUG
		nldebug( "Nb: %u - Bytes: %u", nb_recvd, bytes_recvd );
		/*if ( nb_recvd == 33 )
		{
			nlassert( false );
		}*/
#endif

		// Receive only one message per update if not in "receive all" mode
		if ( ! receiveAllMode() )
		{
			break;
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
	socklen_t saddrlen = sizeof(saddr);
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
	connection->setSenderId( newSenderId() );
	_Connections.push_back( connection );
}


/*
 * Handle a connection closure (graceful or not)
 */
void CMsgSocket::handleConnectionClosure( const CConnections::iterator& ilps )
{
	if ( (*ilps)->connected() ) // if not, connection closure has already been handled
	{
		(*ilps)->close();
		(*ilps)->setDataAvailableFlag( false );
		(*ilps)->disable();
		CMessage msg( "D" );
		processReceivedMessage( msg, **ilps );
		if ( (*ilps)->ownerClient() != NULL )
		{
			// If the socket is pointed by a client socket, neutralize its property _ClientSock
			(*ilps)->ownerClient()->_ClientSock = NULL;
		}
		_ConnectionsToDelete.push_back( ilps );
	}
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

		// Add the listening socket
		CConnections::iterator itps = _Connections.begin();
		FD_SET( (*itps)->descriptor(), &readers );
		if ( (*itps)->descriptor() > descmax )
		{
			descmax = (*itps)->descriptor();
		}

		// Add the connections
		for ( itps++; itps!=_Connections.end(); ++itps )
		{
			if ( (*itps)->connected() ) // exclude disconnected sockets that are not deleted
			{
				FD_SET( (*itps)->descriptor(), &readers );
				if ( (*itps)->descriptor() > descmax )
				{
					descmax = (*itps)->descriptor();
				}
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
			case -1 : //throw ESocket("getDataAvailableStatus(): select failed", ERROR_NUM ); return false;
					  nlerror( "getDataAvailableStatus(): select failed: %d", ERROR_NUM ); return false;
		}
		
		// Get results
		for ( itps = _Connections.begin(); itps!=_Connections.end(); itps++ )
		{
			(*itps)->setDataAvailableFlag( FD_ISSET( (*itps)->descriptor(), &readers ) != 0 );
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
			_CallbackArray[num].Callback( msg, sock.senderId() );
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
			// Send a binding message if it has not been sent before
			if ( ! (*its).bindSent() )
			{
				CMessage bindmsg;
				TTypeNum num = (*its).pt() - _CallbackArray;
				bindmsg.setType( "B" );
				bindmsg.serial( s );
				bindmsg.serial( num );
				sock.send( bindmsg );
				const_cast<CPtCallbackItem&>((*its)).setBindSentFlag();
			}
		}

		// Call the callback function
		callback( msg, sock.senderId() );
	}
	return true;
}


/*
 * Returns a pointer to the socket object having the specified sender id
 */
CSocket *CMsgSocket::socketFromId( TSenderId id )
{
	return *iteratorFromId( id );
}


/*
 * Returns an iterator to the socket pointer in the list of connections, with the specified sender id
 */
CConnections::iterator CMsgSocket::iteratorFromId( TSenderId id )
{
	CConnections::iterator itps;
	for ( itps=_Connections.begin(); itps!=_Connections.end(); itps++ )
	{
		if ( (*itps)->senderId() == id )
		{
			return itps;
		}
	}
	return _Connections.end();
}



/*
 * Returns the internet address of the listening socket (server mode only)
 */
const CInetAddress *CMsgSocket::listenAddress()
{
	CConnections::iterator ips = _Connections.begin();
	if ( ips!=_Connections.end() )
	{
		if ( (*ips)->isListening() )
		{
			return &((*ips)->localAddr());
		}
		else
		{
			return NULL;
		}
	}
	else
	{
		return NULL;
	}
}


/*
 * Returns the number of bytes received from the host since the beginning
 */
uint32 CMsgSocket::bytesReceivedFromHost()
{
	nlassert( _ClientSock != NULL );
	return _ClientSock->bytesReceived();
}


/*
 * Returns the number of bytes sent to the host since the beginning
 */
uint32 CMsgSocket::bytesSentToHost()
{
	nlassert( _ClientSock != NULL );
	return _ClientSock->bytesSent();
}


/*
 * Returns the number of bytes downloaded since the previous call to this method
 */
uint CMsgSocket::newBytesReceivedFromHost()
{
	uint32 b = bytesReceivedFromHost();
	uint nbrecvd = b - _PrevBytesReceivedFromHost;
	_PrevBytesReceivedFromHost = b;
	return nbrecvd;

}


/*
 * Returns the number of bytes uploaded since the previous call to this method
 */
uint CMsgSocket::newBytesSentToHost()
{
	uint32 b = bytesSentToHost();
	uint nbrecvd = b - _PrevBytesSentToHost;
	_PrevBytesSentToHost = b;
	return nbrecvd;

}


/*
 * Returns the number of bytes received since the beginning
 */
uint32 CMsgSocket::bytesReceived()
{
	uint32 sum = 0;
	CConnections::iterator ic;
	for ( ic=_Connections.begin(); ic!=_Connections.end(); ++ic )
	{
		sum += (*ic)->bytesReceived();
	}
	return sum;
}


/*
 * Returns the number of bytes sent since the beginning
 */
uint32 CMsgSocket::bytesSent()
{
	uint32 sum = 0;
	CConnections::iterator ic;
	for ( ic=_Connections.begin(); ic!=_Connections.end(); ++ic )
	{
		sum += (*ic)->bytesReceived();
	}
	return sum;
}


/*
 * Returns the number of bytes downloaded since the previous call to this method
 */
uint CMsgSocket::newBytesReceived()
{
	uint32 b = bytesReceived();
	uint nbrecvd = b - _PrevBytesReceived;
	_PrevBytesReceived = b;
	return nbrecvd;
}


/*
 * Returns the number of bytes uploaded since the previous call to this method
 */
uint CMsgSocket::newBytesSent()
{
	uint32 b = bytesSent();
	uint nbrecvd = b - _PrevBytesSent;
	_PrevBytesSent = b;
	return nbrecvd;
}


} // NLNET
