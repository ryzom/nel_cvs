/** \file naming_client.h
 * CNamingClient
 *
 * $Id: naming_client.h,v 1.18 2001/03/29 09:36:39 lecroart Exp $
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

#ifndef NL_NAMING_CLIENT_H
#define NL_NAMING_CLIENT_H

#include "nel/net/inet_address.h"
#include "nel/misc/config_file.h"
#include "nel/net/service.h"

#include <string>
#include <map> // only one address per service


namespace NLNET {


class CSocket;
class CCallbackClient;


/// Type of map of registered services
typedef std::map<TServiceId,std::string> CRegServices;


/**
 * Client side of Naming Service. Allows to register/unregister services, and to lookup for
 * a registered service.
 *
 * If you want, you can declare a CNamingClient object instead of calling finalize().
 * This static method will be called at the object's destruction (determined by its scope).
 * Thus, if an exception is raised within the scope, finalize() will be called automatically.
 *
 * By default, "transaction mode" is on, i.e. you don't need to call open() and close(), they
 * are called each time you call lookup(), queryServicePort() and similar methods.
 * If you plan to call several times these methods in a block, call open() at the beginning of the block
 * and close() at the end. For registration/unregistration purpose, do not use transaction mode,
 * call open() at the beginning and close() at the end of the program (it's done in IService::main())
 *
 * The naming service (NS) address is read in a config file called ("ns.cfg" at the moment).
 * If no such file is provided, the default values are used (see NamingServiceDefHost
 * and NamingServiceDefPort).
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CNamingClient
{
public:

	/// Constructor
	CNamingClient() {};

	/// Destructor. Calls finalize().
	~CNamingClient();

	/// Finalization. Unregisters all services registered by registerService() and not unregistered yet.
	static void			finalize();

	/// Connection to the naming service (exits from transaction mode)
	static void			open();

	/// Disconnection from the naming service (reenters transaction mode)
	static void			close();

	/** \name Requests to the Naming Service. 
	 * If TransactionMode is true, these method perform open() and close() themselves.
	 * Do not use transaction mode for (un)registering a service, but only for look-ups !
	 */
	//@{

	/** Requests the naming service to choose a port for the service
	 * \param name [in] Name of the service
	 * \param addr [in] Address of the service (the port can be 0 as returned by CInetAddress::localHost())
	 * \return The allocated port number
	 */
	static uint16		queryServicePort( const std::string& name, const CInetAddress& addr );

	/** Register a service within the naming service.
	 * Returns the service identifier assigned by the NS (or 0 if it failed)
	 */
	static TServiceId	registerService( const std::string& name, const CInetAddress& addr );

	/** Register a service within the naming service, using a specified service identifier.
	 * Returns false if the service identifier is unavailable i.e. the registration failed.
	 */
	static bool			registerServiceWithSId( const std::string& name, const CInetAddress& addr, TServiceId sid );

	// Unregister a service from the naming service, by name & address (*deprecated*)
	//static void		unregisterService( const std::string& name, const CInetAddress& addr );

	/// Unregister a service from the naming service, service identifier
	static void			unregisterService( TServiceId sid );

	/** Returns true and the address of the specified service if it is found, otherwise returns false
	 * \param name [in] Name of the service to find
	 * \param addr [out] Address of the service
	 * \param validitytime [out] After this number of seconds are elapsed, another lookup will be necessary
	 * before sending a message to the service
	 * \return True if all worked fine
	 */
	static bool			lookup( const std::string& name, CInetAddress& addr, uint16& validitytime );

	/// Same as lookup(const string&, CInetAddress&, uint16&)
	static bool			lookup( TServiceId sid, CInetAddress& addr, uint16& validitytime );

	/** Tells the Naming Service the specified address does not respond for the specified service,
	 * and returns true and another address for the service if available, otherwise returns false
	 * \param name [in] Name of the service to find
	 * \param addr [in/out] In: Address of the service that does not respond. Out: Alternative address
	 * \param validitytime [out] After this number of seconds are elapsed, another lookup will be necessary
	 * before sending a message to the service
	 * \return True if all worked fine
	 */
	static bool			lookupAlternate( const std::string& name, CInetAddress& addr, uint16& validitytime );

	/// Same as lookupAlternate(const string&, CInetAddress&, uint16&)
	static bool			lookupAlternate( TServiceId sid, CInetAddress& addr, uint16& validitytime );

	/** Obtains a socket connected to a server providing the service \e name.
	 * In case of failure to connect, the method informs the Naming Service and tries to get another server
	 * \param name [in] Name of the service to find and connected
	 * \param sock [out] The connected socket.
	 * \param validitytime [out] After this number of seconds are elapsed, another lookup will be necessary
	 * before sending a message to the service
	 * \return false if the service was not found
	 */
	static bool			lookupAndConnect( const std::string& name, CSocket& sock, uint16& validitytime );

	/// See lookupAndConnect( const std::string&, CSocket&, uint16& )
	static bool			lookupAndConnect( const std::string& name, CCallbackClient& sock, uint16& validitytime );

	/**
	 * Returns all services corresponding to the specified name.
	 * Ex: lookupAll( "AS", addresses );
	 */
	static void			lookupAll( const std::string& name, std::vector<CInetAddress>& addresses );

	/**
	 * Returns all services corresponding to the specified name with service id as key
	 * Ex: lookupAll( "AS", addressmap );
	 */
	static void			lookupAllServices( const std::string& name, std::map<TServiceId,CInetAddress>& addressmap );

	//@}

	/// Address of naming service
	static CInetAddress NamingServiceAddress;

	/// Config file name
	static const char	*NamingServiceAddrFile;

	/// Default NS host name
	static const char	*NamingServiceDefHost;

	/// Default NS port
	static const uint16	NamingServiceDefPort;

	/// Returns transaction mode
	static bool	transactionMode() { return CNamingClient::_TransactionMode; }

	/// Callback for dynamic config file change
	friend void cbNamingServiceAddrChanged();

protected:

	/// Call doOpen() is _TransactionMode is true
	static void			openT();

	/// Call doClose() if _TransactionMode is true
	static void			closeT();

	/// Performs a socket connection
	static void			doOpen();

	/// Performs a socket disconnection
	static void			doClose();

	/// Helper function for lookup() and loopupAlternate()
	static bool			doReceiveLookupAnswer( const std::string& name, NLNET::CInetAddress& addr, uint16& validitytime );


private:
	
	static CSocket		*_ClientSock;
	static CRegServices	_RegisteredServices;
	static NLMISC::CConfigFile	*_ConfigFile;

	static bool			_TransactionMode;

};


} // NLNET


#endif // NL_NAMING_CLIENT_H

/* End of naming_client.h */
