/* inet_address.h
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
 * $Id: inet_address.h,v 1.11 2000/10/10 15:28:15 cado Exp $
 *
 * Interface for CInetAddress
 */

#ifndef NL_INET_ADDRESS_H
#define NL_INET_ADDRESS_H


#include <nel/misc/types_nl.h>
#include <string>


struct sockaddr_in;


namespace NLMISC
{
	class IStream;
}


namespace NLNET
{

class ESocket;


/**
 * Internet address (IP + port).
 * The structure sockaddr_in is internally in network byte order
 * \todo cado Allow to use addreses without hostname
 * \todo cado Test to check if byte ordering is ok.
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CInetAddress
{
public:

	/// Default Constructor
	CInetAddress();

	/// Alternate constructor (calls setByName())
	CInetAddress( const std::string& hostName, uint16 port=0 );

	/// Copy constructor
	CInetAddress( const CInetAddress& other );

	/// Assignment operator
	CInetAddress& operator=( const CInetAddress& other );

	/// Comparison operator
	friend bool operator==( const CInetAddress& a1, const CInetAddress& a2 );

	/// Destructor
	~CInetAddress();

	/// Resolves a name
	CInetAddress&		setByName( const std::string& hostname ); //throw(ESocket);

	/// Sets port
	void				setPort( uint16 port );

	/** Sets internal socket address directly (contents is copied).
	 * It also retrieves the host name if CInetAddress::RetrieveNames is true.
	 */
	void				setSockAddr( const sockaddr_in* saddr ); //throw (ESocket);

	/// Returns if object (address and port) is valid
	bool				isValid() const;

	/// Returns internal socket address (read only)
	const sockaddr_in	 *sockAddr() const;

	/// Returns readable IP address
	std::string			ipAddress() const;

	/// Returns hostname
	const std::string&	hostName() const;

	/// Returns port
	uint16				port() const;

	/// Returns hostname and port as a string
	std::string			asString() const;

	/// Returns IP address and port as a string
	std::string			asIPString() const;

	/// Serialize
	void serial( NLMISC::IStream& s );

	/// Creates a CInetAddress object with local host address, port=0
	static CInetAddress	localHost(); // throw(ESocket);

	/// If true, setSockAddr() always tries to retrieve the host name from the address
	static bool RetrieveNames;

private:

	// Called in all constructors. Calls CBaseSocket::init().
	void				init();

	std::string	_HostName;
	sockaddr_in	*_SockAddr;
	bool		_Valid;

};

}

#endif // NL_INET_ADDRESS_H

/* End of inet_address.h */
