/** \file naming_client.h
 * CNamingClient
 *
 * $Id: naming_client.h,v 1.23 2001/06/13 10:21:02 lecroart Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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

#include "nel/misc/mutex.h"

#include "nel/net/inet_address.h"
#include "nel/net/callback_client.h"

//#include "nel/misc/config_file.h"
//#include "nel/net/service.h"

#include <string>
//#include <map> // only one address per service


namespace NLNET {


typedef uint8 TServiceId;

typedef void (*TBroadcastCallback)(const std::string &name, TServiceId sid, const CInetAddress &addr);

/**
 * Client side of Naming Service. Allows to register/unregister services, and to lookup for
 * a registered service.
 *
 * \author Olivier Cado
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
class CNamingClient
{
public:

	/// Connect to the naming service.
	static void			connect (const CInetAddress& addr);

	/// Return true if the connection to the Naming Service was done.
	static bool			connected () { return _Connection != NULL && _Connection->connected (); }

	/// Close the connection to the naming service.
	static void			disconnect ();


	/** Register a service within the naming service.
	 * Returns the service identifier assigned by the NS (or 0 if it failed)
	 */
	static TServiceId	registerService (const std::string &name, const CInetAddress &addr);

	/** Register a service within the naming service, using a specified service identifier.
	 * Returns false if the service identifier is unavailable i.e. the registration failed.
	 */
	static bool			registerServiceWithSId (const std::string &name, const CInetAddress &addr, TServiceId sid);


	/// Unregister a service from the naming service, service identifier.
	static void			unregisterService (TServiceId sid);

	/// Unregister all services registered by this client. You don't have to
	static void			unregisterAllServices ();


	/** Requests the naming service to choose a port for the service
	 * \return An empty port number
	 */
	static uint16		queryServicePort ();


	/** Returns true and the address of the specified service if it is found, otherwise returns false
	 * \param name [in] Short name of the service to find
	 * \param addr [out] Address of the service
	 * \param validitytime [out] After this number of seconds are elapsed, another lookup will be necessary
	 * before sending a message to the service
	 * \return true if all worked fine
	 */
	static bool			lookup (const std::string &name, CInetAddress &addr);

	/// Same as lookup(const string&, CInetAddress&, uint16&)
	static bool			lookup (TServiceId sid, CInetAddress &addr);

	/** Tells the Naming Service the specified address does not respond for the specified service,
	 * and returns true and another address for the service if available, otherwise returns false
	 * \param name [in] Short name of the service to find
	 * \param addr [in/out] In: Address of the service that does not respond. Out: Alternative address
	 * \param validitytime [out] After this number of seconds are elapsed, another lookup will be necessary
	 * before sending a message to the service
	 * \return true if all worked fine.
	 */
	static bool			lookupAlternate (const std::string& name, CInetAddress& addr);


	/**
	 * Returns all services corresponding to the specified short name.
	 * Ex: lookupAll ("AS", addresses);
	 */
	static void			lookupAll (const std::string &name, std::vector<CInetAddress> &addresses);


	/** Obtains a socket connected to a service providing the service \e name.
	 * In case of failure to connect, the method informs the Naming Service and tries to get another service.
	 * \param name [in] Short name of the service to find and connected
	 * \param sock [out] The connected socket.
	 * \param validitytime [out] After this number of seconds are elapsed, another lookup will be necessary
	 * before sending a message to the service.
	 * \return false if the service was not found.
	 */
	static bool			lookupAndConnect (const std::string &name, CCallbackClient &sock);



	/// Call it evenly
	static void			update ();


	/// You can link a callback if you want to know when a new service is registered (NULL to disable callback)
	static void			setRegistrationBroadcastCallback (TBroadcastCallback cb);


	/// You can link a callback if you want to know when a new service is unregistered (NULL to disable callback)
	static void			setUnregistrationBroadcastCallback (TBroadcastCallback cb);

private:

	static CCallbackClient *_Connection;

	/// Type of map of registered services
	typedef std::map<TServiceId, std::string> TRegServices;

	static TRegServices _RegisteredServices;

	/// Constructor
	CNamingClient() {}

	/// Destructor
	~CNamingClient() {}

	static void doReceiveLookupAnswer (const std::string &name, std::vector<CInetAddress> &addrs);



	struct CServiceEntry
	{
		CServiceEntry (std::string n, TServiceId s, CInetAddress a) : Addr(a), Name(n), SId (s) { }

		// name of the service
		std::string		Name;
		// id of the service
		TServiceId		SId;
		// address to send to the service who wants to lookup this service
		CInetAddress	Addr;
	};

	static std::list<CServiceEntry>	RegisteredServices;
	static NLMISC::CMutex RegisteredServicesMutex;

	static void displayRegisteredServices ()
	{
		RegisteredServicesMutex.enter ();
		nldebug ("Display the %d registered services :", RegisteredServices.size());
		for (std::list<CServiceEntry>::iterator it = RegisteredServices.begin(); it != RegisteredServices.end (); it++)
		{
			nldebug (" > %s-%hu '%s'", (*it).Name.c_str(), (uint16)(*it).SId, (*it).Addr.asString().c_str());
		}
		nldebug ("End ot the list");
		RegisteredServicesMutex.leave ();
	}

	static void find (const std::string &name, std::vector<CInetAddress> &addrs)
	{
		RegisteredServicesMutex.enter ();
		for (std::list<CServiceEntry>::iterator it = RegisteredServices.begin(); it != RegisteredServices.end (); it++)
			if (name == (*it).Name)
				addrs.push_back ((*it).Addr);
		RegisteredServicesMutex.leave ();
	}

	static void find (TServiceId sid, std::vector<CInetAddress> &addrs)
	{
		RegisteredServicesMutex.enter ();
		for (std::list<CServiceEntry>::iterator it = RegisteredServices.begin(); it != RegisteredServices.end (); it++)
			if (sid == (*it).SId)
				addrs.push_back ((*it).Addr);
		RegisteredServicesMutex.leave ();
	}


	friend void cbRegisterBroadcast (CMessage &msgin, TSockId from, CCallbackNetBase &netbase);
	friend void cbUnregisterBroadcast (CMessage &msgin, TSockId from, CCallbackNetBase &netbase);


protected:
	/// \todo debug feature that we should remove one day
	static uint	_ThreadId;
	static void	checkThreadId ();


//////////////////////
////////////////////// OLD NAMING CLIENT
//////////////////////
private:

	/// Finalization. Unregisters all services registered by registerService() and not unregistered yet.
//	static void			finalize();

	/// Connection to the naming service (exits from transaction mode)
//	static void			open();

	/// Disconnection from the naming service (reenters transaction mode)
//	static void			close();

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
//	static uint16		queryServicePort( const std::string& name, const CInetAddress& addr );

	/** Register a service within the naming service, using a specified service identifier.
	 * Returns false if the service identifier is unavailable i.e. the registration failed.
	 */
//	static bool			registerServiceWithSId( const std::string& name, const CInetAddress& addr, TServiceId sid );

	// Unregister a service from the naming service, by name & address (*deprecated*)
	//static void		unregisterService( const std::string& name, const CInetAddress& addr );

	/** Returns true and the address of the specified service if it is found, otherwise returns false
	 * \param name [in] Name of the service to find
	 * \param addr [out] Address of the service
	 * \param validitytime [out] After this number of seconds are elapsed, another lookup will be necessary
	 * before sending a message to the service
	 * \return True if all worked fine
	 */
//	static bool			lookup( const std::string& name, CInetAddress& addr, uint16& validitytime );

	/// Same as lookup(const string&, CInetAddress&, uint16&)
//	static bool			lookup( TServiceId sid, CInetAddress& addr, uint16& validitytime );

	/** Tells the Naming Service the specified address does not respond for the specified service,
	 * and returns true and another address for the service if available, otherwise returns false
	 * \param name [in] Name of the service to find
	 * \param addr [in/out] In: Address of the service that does not respond. Out: Alternative address
	 * \param validitytime [out] After this number of seconds are elapsed, another lookup will be necessary
	 * before sending a message to the service
	 * \return True if all worked fine
	 */
//	static bool			lookupAlternate( const std::string& name, CInetAddress& addr, uint16& validitytime );

	/// Same as lookupAlternate(const string&, CInetAddress&, uint16&)
//	static bool			lookupAlternate( TServiceId sid, CInetAddress& addr, uint16& validitytime );

	/** Obtains a socket connected to a server providing the service \e name.
	 * In case of failure to connect, the method informs the Naming Service and tries to get another server
	 * \param name [in] Name of the service to find and connected
	 * \param sock [out] The connected socket.
	 * \param validitytime [out] After this number of seconds are elapsed, another lookup will be necessary
	 * before sending a message to the service
	 * \return false if the service was not found
	 */
//	static bool			lookupAndConnect( const std::string& name, CTcpSock& sock, uint16& validitytime );

	/// See lookupAndConnect( const std::string&, CSocket&, uint16& )
//	static bool			lookupAndConnect( const std::string& name, CCallbackClient& sock, uint16& validitytime );

	/**
	 * Returns all services corresponding to the specified name.
	 * Ex: lookupAll( "AS", addresses );
	 */
//	static void			lookupAll( const std::string& name, std::vector<CInetAddress>& addresses );

	/**
	 * Returns all services corresponding to the specified name with service id as key
	 * Ex: lookupAll( "AS", addressmap );
	 */
//	static void			lookupAllServices( const std::string& name, std::map<TServiceId,CInetAddress>& addressmap );

	//@}

	/*
	/// Address of naming service
	static CInetAddress NamingServiceAddress;

	/// Config file name
	static const char	*NamingServiceAddrFile;

	/// Default NS host name
	static const char	*NamingServiceDefHost;

	/// Default NS port
	static const uint16	NamingServiceDefPort;
	*/
	
	/// Returns transaction mode
//	static bool	transactionMode() { return CNamingClient::_TransactionMode; }

	/// Callback for dynamic config file change
//	friend void cbNamingServiceAddrChanged();
/*
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
*/	
	/*static CTcpSock		*_ClientSock;
	static NLMISC::CConfigFile	*_ConfigFile;
	*/
//	static bool			_TransactionMode;
};


} // NLNET


#endif // NL_NAMING_CLIENT_H

/* End of naming_client.h */
