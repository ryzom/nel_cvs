/* inet_address.cpp
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
 * $Id: inet_address.cpp,v 1.14 2000/10/11 16:25:25 cado Exp $
 *
 * Implementation for CInetAddress.
 * Thanks to Daniel Bellen <huck@pool.informatik.rwth-aachen.de> for libsock++,
 * from which I took some ideas
 */

#include "nel/net/inet_address.h"
#include "nel/net/socket.h"
#include "nel/misc/stream.h"
#include <sstream>


#ifdef NL_OS_WINDOWS

#include <winsock2.h>

#elif defined NL_OS_UNIX

#include <netinet/in.h>
#define WSAGetLastError() 0

#endif


#include <sstream>
using namespace std;


namespace NLNET
{

bool CInetAddress::RetrieveNames = true;


/*
 * Constructor
 */
CInetAddress::CInetAddress()
: _Valid( false )
{
	init();
}



/*
 * Alternate constructor (calls setByName())
 */
CInetAddress::CInetAddress( const std::string& hostName, uint16 port )
{
	init();
	setPort( port );
	setByName( hostName );
}



/*
 * Copy constructor
 */
CInetAddress::CInetAddress( const CInetAddress& other )
{
	init();
	_HostName = other._HostName;
	memcpy( _SockAddr, other._SockAddr, sizeof( *_SockAddr ) );
	_Valid = other._Valid;
}


/*
 * Assignment operator
 */
CInetAddress& CInetAddress::operator=( const CInetAddress& other )
{
	_HostName = other._HostName;
	memcpy( _SockAddr, other._SockAddr, sizeof( *_SockAddr ) );
	_Valid = other._Valid;
	return *this;
}


/*
 * Comparison operator
 */
bool operator==( const CInetAddress& a1, const CInetAddress& a2 )
{
	// Compares the sockaddr structure except the last 8 bytes equal to zero.
	return ( memcmp( a1._SockAddr, a2._SockAddr, sizeof(sockaddr_in)-8 ) == 0 );
}


/*
 * Constructor contents
 */
void CInetAddress::init()
{
	CBaseSocket::init();
	_SockAddr = new sockaddr_in;
	_SockAddr->sin_family = AF_INET;
	_SockAddr->sin_port = 0; // same as htons(0)
	memset( &_SockAddr->sin_addr, 0, sizeof(in_addr) );
	memset( _SockAddr->sin_zero, 0, 8 );
}


/*
 * Destructor
 */
CInetAddress::~CInetAddress()
{
	delete _SockAddr;
	// _Valid = false;
}


/*
 * Resolves a name
 */
CInetAddress& CInetAddress::setByName( const std::string& hostName ) throw (ESocket)
{
	// Try to convert directly for addresses such as a.b.c.d
	in_addr iaddr;
	iaddr.S_un.S_addr = inet_addr( hostName.c_str() );
	if ( iaddr.S_un.S_addr == INADDR_NONE )
	{
		// Otherwise use the traditional DNS look-up
		hostent *phostent = gethostbyname( hostName.c_str() );
		if ( phostent == NULL )
		{
			_Valid = false;
			// log("Network error: Hostname resolution failed (CInetAddress::getByName)");
			// return *this;
			throw ESocket("Hostname resolution failed");
		}
		_HostName = string( phostent->h_name );
		memcpy( &_SockAddr->sin_addr, phostent->h_addr, sizeof(in_addr) );
	}
	else
	{
		_HostName = hostName;
		memcpy( &_SockAddr->sin_addr, &iaddr, sizeof(iaddr) );
	}
	_Valid = true;
	return *this;
}


/*
 * Sets port
 */
void CInetAddress::setPort( uint16 port )
{
	_SockAddr->sin_port = htons( port );

}


/* Sets internal socket address directly (contents is copied).
 * It also retrieves the host name if CInetAddress::RetrieveNames is true.
 */
void CInetAddress::setSockAddr( const sockaddr_in* saddr ) throw (ESocket)
{
	memcpy( _SockAddr, saddr, sizeof(*saddr) );

	// Get host name
	// Warning: when it can't find it, it take more than 4 seconds
	if ( CInetAddress::RetrieveNames )
	{
		hostent *phostent = gethostbyaddr( (char*)&saddr->sin_addr.s_addr, 4,  AF_INET );
		if ( phostent == NULL )
		{
			throw ESocket("Could not resolve host name from address",WSAGetLastError());
		}
		_HostName = string( phostent->h_name );
	}
	_Valid = true;
}


/*
 * Returns if object (address and port) is valid
 */
bool CInetAddress::isValid() const
{
	return ( _Valid && _SockAddr->sin_port!=0 ); // same as ntohs(0)
}


/*
 * Returns internal socket address (read only)
 */
const sockaddr_in *CInetAddress::sockAddr() const
{
	return _SockAddr;
}


/*
 * Returns readable IP address
 */
string CInetAddress::ipAddress() const
{
	stringstream ss; // or use inet_ntoa
	ss << _SockAddr->sin_addr.S_un.S_un_b.s_b1 << "." << _SockAddr->sin_addr.S_un.S_un_b.s_b2 << "." << _SockAddr->sin_addr.S_un.S_un_b.s_b3 << "." << _SockAddr->sin_addr.S_un.S_un_b.s_b4;
	return ss.str();
}


/*
 * Returns host name
 */
const string& CInetAddress::hostName() const
{
	return _HostName;
}


/*
 * Returns port
 */
uint16 CInetAddress::port() const
{
	return ntohs( _SockAddr->sin_port );
}


/*
 * Returns hostname and port as a string
 */
std::string CInetAddress::asString() const
{
	stringstream ss;
	ss << hostName() << "/" << port();
	return ss.str();
}


/*
 * Returns IP address and port as a string
 */
std::string CInetAddress::asIPString() const
{
	stringstream ss;
	ss << ipAddress() << "/" << port();
	return ss.str();
}


/*
 * Serialize
 */
void CInetAddress::serial( NLMISC::IStream& s )
{
	s.serial( _HostName );
	s.serialBuffer( (uint8*)_SockAddr, sizeof(*_SockAddr) ); // this is possible only because the contents of _SockAddr is platform-independant !
	s.serial( _Valid );
}


/*
 * Creates a CInetAddress object with local host address, port=0
 */
CInetAddress CInetAddress::localHost() throw (ESocket)
{
	const uint maxlength = 80;
	char localhost [maxlength];
	if ( gethostname( localhost, maxlength ) != 0 )
		throw ESocket("Unable to get local hostname");
	return CInetAddress( string(localhost) );
}

}
