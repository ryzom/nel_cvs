/* naming_client.h
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
 * $Id: naming_client.h,v 1.3 2000/10/10 16:30:52 cado Exp $
 *
 * Interface for CNamingClient
 */

#ifndef NL_NAMING_CLIENT_H
#define NL_NAMING_CLIENT_H

#include "nel/net/inet_address.h"

#include <string>
#include <map> // only one address per service


namespace NLNET {


class CSocket;

typedef std::map<std::string,CInetAddress> CRegServices;


/**
 * Client side of Naming Service. Allows to register/unregister services, and to lookup for
 * a registered service.
 *
 * If you want, you can declare a CNamingClient object instead of calling finalize().
 * This static method will be called at the object's destruction (determined by its scope).
 * Thus, if an exception is raised within the scope, finalize() will be called automatically.
 *
 * By default, TransactionMode is true, i.e. you don't need to call open() and close(), they
 * are called each time you call lookup(), registerService(), and unregisterService().
 * If you plan to call several times these methods in a block, set TransactionMode to false
 * and call open() at the beginning of the block and close() at the end.
 *
 * \todo cado/lecroart: Move service registration to IService (but not for the NS itself) and
 * ensure unregistration is called when stopping the service (Ctrl-C must call release())
 * \todo cado: Manage validity time: a new request must be done when the time is out and
 * the caller must disconnect from its service provider and connect to the new one.
 * \test Test program is /code/test/network/log_service/main.cpp
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CNamingClient
{
public:

	/** Constructor.
	 * \param transactionmode See "Requests to the Naming Service" in the static public methods.
	 */
	CNamingClient( bool transactionmode = true );

	/// Destructor. Calls finalize().
	~CNamingClient();

	/// Finalization. Unregisters all services registered by registerService() and not unregistered yet.
	static void			finalize();

	/// Connection to the naming service
	static void			open();

	/// Disconnection from the naming service
	static void			close();

	/** \name Requests to the Naming Service. 
	 * \brief If TransactionMode is true, these method perform open() and close() themselves.
	 */
	//@{

	/// Register a service within the naming service
	static void			registerService( const std::string& name, const CInetAddress& addr );

	/// Unregister a service from the naming service
	static void			unregisterService( const std::string& name, const CInetAddress& addr );

	/// Returns true and the address of the specified service if it is found, otherwise returns false
	static bool			lookup( const std::string& name, CInetAddress& addr );

	//@}

	/// Address of naming service
	static CInetAddress NamingServiceAddress;

	/// Transaction mode
	static bool			TransactionMode;

protected:

	/// Call doOpen() is TransactionMode is true
	static void			openT();

	/// Call doClose() if TransactionMode is true
	static void			closeT();

	/// Performs a socket connection
	static void			doOpen();

	/// Performs a socket disconnection
	static void			doClose();

private:
	
	static CSocket		*_ClientSock;
	static CRegServices	_RegisteredServices;

};


} // NLNET


#endif // NL_NAMING_CLIENT_H

/* End of naming_client.h */
