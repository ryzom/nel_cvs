/* datagram_socket.cpp
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
 * $Id: datagram_socket.cpp,v 1.2 2000/09/21 14:12:10 cado Exp $
 *
 * Implementation for CDatagramSocket
 */

#include "nel/net/datagram_socket.h"
#include "nel/net/message.h"


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


namespace NLNET {


/*
 * Constructor
 */
CDatagramSocket::CDatagramSocket() throw (ESocket) :
	CBaseSocket(),
	_Bound( false )
{
	// Socket creation
	_Sock = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP ); // IPPROTO_UDP or IPPROTO_IP (=0) ?
	if ( _Sock == INVALID_SOCKET )
	{
		throw ESocket("Datagram socket creation failed");
	}
	_Log.display( "Socket %d open\n", _Sock );
}


/** Binds the socket to the specified port. Call bind() if the host acts as a server and waits for
 * messages. If the host acts as a client, call sendTo(), there is no need to bind the socket.
 */
void CDatagramSocket::bind( uint16 port ) throw (ESocket)
{
	// Get local socket name
	const uint MAXLENGTH = 80;
	char localhost [MAXLENGTH];
	if ( gethostname( localhost, MAXLENGTH ) != 0 )
	{
		throw ESocket("Unabled to get local hostname");
	}
	_LocalAddr.setByName( localhost );
	_LocalAddr.setPort( port );

	// Bind the socket
	if ( ::bind( _Sock, (sockaddr*)(_LocalAddr.sockAddr()), sizeof(sockaddr) ) == SOCKET_ERROR )
	{
		#ifdef NL_OS_WINDOWS
			switch ( WSAGetLastError() ) {
			case WSAEADDRINUSE : throw ESocket("Bind failed : address in use");
			case WSAEADDRNOTAVAIL : throw ESocket("Bind failed : address not available");
			default : throw ESocket("Bind failed");
			}
		#elif defined NL_OS_LINUX
			throw ESocket(strerror(errno));
		#endif
	}
	_Bound = true;
	_Log.display( "Socket %d bound at %s/%hu\n", _Sock, _LocalAddr.ipAddress().c_str(), _LocalAddr.port() );
}


/*
 * Sends a message
 */
void CDatagramSocket::sendTo( const CMessage& message, const CInetAddress& addr ) throw (ESocket)
{
	CMessage alldata = message.encode();

	// 5. Send!
	if ( ::sendto( _Sock, (const char*)alldata.buffer(), alldata.length(), 0, (sockaddr*)(addr.sockAddr()), sizeof(sockaddr) ) != (sint32)(alldata.length()) )
	{
		throw ESocket("Unable to send datagram");
	}
	_Log.display( "Socket %d sent %d bytes to %s/%hu\n", _Sock, alldata.length(), addr.ipAddress().c_str(), addr.port() );

	// 5bis. If socket is unbound, retrieve local address
	if ( ! _Bound )
	{
		setLocalAddress();
		_Bound = true;
	}
}


/*
 * Receives data (returns false if !dataAvailable()).
 */
bool CDatagramSocket::receiveFrom( CMessage& message, CInetAddress& addr ) throw (ESocket)
{
	if ( ! dataAvailable() )
	{
		return false;
	}

	// Receive incoming message
	uint32 msgtotalsize = CMessage::maxLength();
	CMessage alldata( true, msgtotalsize );
	sockaddr_in saddr;
	int saddrlen = sizeof(saddr);
	int brecvd = ::recvfrom( _Sock, (char*)alldata.bufferToFill( msgtotalsize ), msgtotalsize , 0, (sockaddr*)&saddr, &saddrlen );
	if ( brecvd == SOCKET_ERROR )
	{
		#ifdef NL_OS_WINDOWS
			throw ESocket("Cannot receive msgtype",WSAGetLastError());
		#else
			throw ESocket("Cannot receive msgtype",errno);
		#endif
	}

	// Get sender's address
	addr.setSockAddr( &saddr );

	// Decode message
	message.decode( alldata );

	_Log.display( "Socket %d received %d bytes from %s/%hu\n", _Sock, alldata.length(), addr.ipAddress().c_str(), addr.port() );

	return true;
}


} // NLNET
