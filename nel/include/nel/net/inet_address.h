/* inet_address.h
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: inet_address.h,v 1.2 2000/09/18 17:13:15 cado Exp $
 *
 * Interface for CInetAddress
 */

#ifndef NL_INET_ADDRESS_H
#define NL_INET_ADDRESS_H


#include <nel/misc/types_nl.h>
#include <string>


struct sockaddr_in;


namespace NLNET
{

class ESocket;


/**
 * Internet address (IP + port)
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CInetAddress
{
public:

	/// Default Constructor
	CInetAddress();

	/// Alternate constructor (calls getByName())
	CInetAddress( const std::string& hostName, uint16 port=0 );

	/// Copy constructor
	CInetAddress( const CInetAddress& other );

	/// Assignment operator
	CInetAddress& operator=( const CInetAddress& other );

	/// Destructor
	~CInetAddress();

	/// Resolves a name
	CInetAddress&		getByName( const std::string& hostname ) throw(ESocket);

	/// Sets port
	void				setPort( uint16 port );

	/** Sets internal socket address directly (contents is copied).
	 * It also retrieve the host name if CInetAddress::RetrieveNames is true.
	 */
	void				setSockAddr( const sockaddr_in* saddr ) throw (ESocket);

	/// Returns if object (address and port) is valid
	bool				isValid() const;

	/// Returns internal socket address (read only)
	const sockaddr_in	 	*sockAddr() const;

	/// Returns readable IP address
	std::string			ipAddress() const;

	/// Returns hostname
	std::string			hostName() const;

	/// Returns port
	uint16				port() const;

	/// Creates a CInetAddress object with local host address, port=0
	static CInetAddress	localHost() throw(ESocket);

	/// If true, setSockAddr() always tries to retrieve the host name from the address
	static bool RetrieveNames;

private:

	// Constructor contents
	void				init();

	std::string	_HostName;
	sockaddr_in	*_SockAddr;
	bool		_Valid;

};

}

#endif // NL_INET_ADDRESS_H

/* End of inet_address.h */
