/** \file inet_address.cpp
 * Class CInetAddress (IP address + port)
 *
 * $Id: inet_address.cpp,v 1.32 2001/06/21 08:45:13 cado Exp $
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

#include "nel/net/inet_address.h"
#include "nel/net/sock.h"
#include "nel/misc/stream.h"
#include "nel/misc/string_stream.h"
#include "nel/misc/debug.h"
#include <sstream>


#ifdef NL_OS_WINDOWS

#include <winsock2.h>

#elif defined NL_OS_UNIX

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#define WSAGetLastError() 0

#endif


using namespace std;


namespace NLNET
{

bool CInetAddress::RetrieveNames = true;


/*
 * Constructor
 */
CInetAddress::CInetAddress() :
  _Valid( false )
{
	init();
	_SockAddr->sin_port = 0; // same as htons(0)
	memset( &_SockAddr->sin_addr, 0, sizeof(in_addr) ); // same as htonl(INADDR_ANY)
}



/*
 * Alternate constructor (calls setByName())
 */
CInetAddress::CInetAddress( const std::string& hostName, uint16 port ) :
	_Valid( false )
{
	init();
	setPort( port );
	setByName( hostName );
}


/*
 * Alternate constructor (calls setNameAndPort())
 */
CInetAddress::CInetAddress( const std::string& hostNameAndPort ) :
	_Valid( false )
{
	init();
	setNameAndPort( hostNameAndPort );
}


/*
 * Copy constructor
 */
CInetAddress::CInetAddress( const CInetAddress& other ) :
	_Valid( false )
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
 * Comparison == operator
 */
bool operator==( const CInetAddress& a1, const CInetAddress& a2 )
{
	// Compares the sockaddr structure except the last 8 bytes equal to zero.
	return ( memcmp( a1._SockAddr, a2._SockAddr, sizeof(sockaddr_in)-8 ) == 0 );
}


/*
 * Comparison < operator
 */
bool operator<( const CInetAddress& a1, const CInetAddress& a2 )
{
#ifdef NL_OS_WINDOWS
	if ( a1._SockAddr->sin_addr.S_un.S_addr == a2._SockAddr->sin_addr.S_un.S_addr )
	{
		return ( a1.port() < a2.port() );
	}
	else
	{
		return ( a1._SockAddr->sin_addr.S_un.S_addr < a2._SockAddr->sin_addr.S_un.S_addr );
	}
#elif defined NL_OS_UNIX
	if ( a1._SockAddr->sin_addr.s_addr == a2._SockAddr->sin_addr.s_addr )
	{
		return ( a1.port() < a2.port() );
	}
	else
	{
		return ( a1._SockAddr->sin_addr.s_addr < a2._SockAddr->sin_addr.s_addr );
	}
#endif	
}


/*
 * Constructor contents
 */
void CInetAddress::init()
{
	CSock::initNetwork(); // TODO: handle exception (because init() is called within a constructor)
	
	_SockAddr = new sockaddr_in;
	_SockAddr->sin_family = AF_INET;
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
 * Sets hostname and port (ex: www.nevrax.com:80)
 */
void CInetAddress::setNameAndPort( const std::string& hostNameAndPort )
{
	int pos = hostNameAndPort.find_first_of (':');
	if (pos != string::npos)
	{
		setPort( atoi(hostNameAndPort.substr(pos + 1).c_str()) );
	}
	else
	{
		setPort( 0 );
	}

	// if pos == -1, it will copy all the string
	setByName( hostNameAndPort.substr (0, pos) );
}


/*
 * Resolves a name
 */
CInetAddress& CInetAddress::setByName( const std::string& hostName )
{
	// Try to convert directly for addresses such as a.b.c.d
	in_addr iaddr;
#ifdef NL_OS_WINDOWS
	iaddr.S_un.S_addr = inet_addr( hostName.c_str() );
	if ( iaddr.S_un.S_addr == INADDR_NONE )
#elif defined NL_OS_UNIX
	iaddr.s_addr = inet_addr( hostName.c_str() );
	if ( iaddr.s_addr == INADDR_NONE )
#endif
	{

		// Otherwise use the traditional DNS look-up
		hostent *phostent = gethostbyname( hostName.c_str() );
		if ( phostent == NULL )
		{
			_Valid = false;
			nldebug( "L0: Network error: resolution of hostname '%s' failed", hostName.c_str() );
			// return *this;
			throw ESocket( (string("Hostname resolution failed for ")+hostName).c_str() );
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
void CInetAddress::setSockAddr( const sockaddr_in* saddr )
{
	memcpy( _SockAddr, saddr, sizeof(*saddr) );

	// Get host name
	// Warning: when it can't find it, it take more than 4 seconds
	if ( CInetAddress::RetrieveNames )
	{
		hostent *phostent = gethostbyaddr( (char*)&saddr->sin_addr.s_addr, 4,  AF_INET );
		if ( phostent == NULL )
		{
			_HostName = ipAddress();
		}
		else
		{
			_HostName = string( phostent->h_name );
		}
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
 * Returns internal IP address
 */
uint32 CInetAddress::internalIPAddress() const
{
	return _SockAddr->sin_addr.s_addr;
}


/*
 * Returns readable IP address. (ex: "195.68.21.195")
 */
string CInetAddress::ipAddress() const
{
	/*stringstream ss; // or use inet_ntoa
	ss << inet_ntoa ( _SockAddr->sin_addr );
	return ss.str();*/
	return string( inet_ntoa( _SockAddr->sin_addr ) );
}


/*
 * Returns host name. (ex: "www.nevrax.org")
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
 * Returns hostname and port as a string. (ex: "www.nevrax.org:80 (195.68.21.195)")
 */
std::string CInetAddress::asString() const
{
	stringstream ss;
	ss << hostName() << ":" << port() << " (" << ipAddress() << ")";
	return ss.str();
}


/*
 * Returns IP address and port as a string. (ex: "195.68.21.195:80")
 */
std::string CInetAddress::asIPString() const
{
	stringstream ss;
	ss << ipAddress() << ":" << port();
	return ss.str();
}


/*
 * Serialize from/to string stream
 */
void CInetAddress::serial( NLMISC::CStringStream& s )
{
	string addrs;
	if ( s.isReading() )
	{
		s.serial( addrs );
		setNameAndPort( addrs );
	}
	else
	{
		addrs = asIPString();
		s.serial( addrs );
	}
	s.serial( _Valid );
}


/*
 * Serialize
 */
void CInetAddress::serial( NLMISC::IStream& s )
{
	// If serial() is called by a method from IStream, the method called is serial( IStream& )
	// even if the real type of s is CStringStream&.
	if ( dynamic_cast< NLMISC::CStringStream*>(&s) )
	{
		serial( static_cast< NLMISC::CStringStream&>(s) );
	}
	else
	{
		s.serialBuffer( (uint8*)_SockAddr, sizeof(*_SockAddr) ); // this is possible only because the contents of _SockAddr is platform-independant !
		s.serial( _Valid );
	}
}


/*
 * Creates a CInetAddress object with local host address, port=0
 */
CInetAddress CInetAddress::localHost()
{
	const uint maxlength = 80;
	char localhost [maxlength];
	if ( gethostname( localhost, maxlength ) != 0 )
		throw ESocket( "Unable to get local hostname" );
	CInetAddress localaddr = CInetAddress( string(localhost) );
	nlassert( localaddr.ipAddress() != "127.0.0.1" );
	return localaddr;
}

}
