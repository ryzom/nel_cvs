/* inet_address.cpp
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: inet_address.cpp,v 1.2 2000/09/14 16:40:53 cado Exp $
 *
 * Implementation for CInetAddress
 * Thanks to Daniel Bellen <huck@pool.informatik.rwth-aachen.de> for libsock++,
 * from which I took some ideas
 */

#include "nel/net/inet_address.h"
#include "nel/net/socket.h"


#ifdef NL_OS_WINDOWS
	#include <winsock2.h>
#elif defined NL_OS_LINUX
	#include <netinet/in.h>
#endif


#include <sstream>
using namespace std;


namespace NLNET
{

/*
 * Constructor
 */
CInetAddress::CInetAddress()
: _Valid( false )
{
	_SockAddr = new sockaddr_in;
	_SockAddr->sin_family = AF_INET;
	_SockAddr->sin_port = 0; // same as htons(0)
	memset( &_SockAddr->sin_addr, 0, sizeof(in_addr) );
	memset( _SockAddr->sin_zero, 0, 8 );
}



/*
 * Alternate constructor (calls getByName())
 */
CInetAddress::CInetAddress( const std::string& hostName, uint16 port )
{
	CInetAddress();
	setPort( port );
	getByName( hostName );
}


/*
 * Copy constructor
 */
CInetAddress::CInetAddress( const CInetAddress& other )
{
	CInetAddress();
	_HostName = other._HostName;
	memcpy( _SockAddr, other._SockAddr, sizeof( *_SockAddr ) );
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
CInetAddress& CInetAddress::getByName( const std::string& hostName )
{
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
	return *this;
	_Valid = true;
}


/*
 * Sets port
 */
void CInetAddress::setPort( uint16 port )
{
	_SockAddr->sin_port = htons( port );

}


/*
 * Sets internal socket address directly (contents is copied)
 */
void CInetAddress::setSockAddr( const sockaddr_in* saddr )
{
	memcpy( _SockAddr, saddr, sizeof(saddr) );

	// Get host name
	hostent *phostent = gethostbyaddr( (char*)&saddr->sin_addr.s_addr, 4,  AF_INET );
	if ( phostent == NULL )
	{
		throw ESocket("Could not resolve host name from address");
	}
	_HostName = string( phostent->h_name );
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
	stringstream ss;
	ss << _SockAddr->sin_addr.S_un.S_un_b.s_b1 << "." << _SockAddr->sin_addr.S_un.S_un_b.s_b2 << "." << _SockAddr->sin_addr.S_un.S_un_b.s_b3 << "." << _SockAddr->sin_addr.S_un.S_un_b.s_b4;
	return ss.str();
}


/*
 * Returns host name
 */
string CInetAddress::hostName() const
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
 * Creates a CInetAddress object with local host address, port=0
 */
CInetAddress CInetAddress::localHost()
{
	const uint maxlength = 80;
	char localhost [maxlength];
	if ( gethostname( localhost, maxlength ) != 0 )
		throw ESocket("Unable to get local hostname");
	return CInetAddress( string(localhost) );
}

}