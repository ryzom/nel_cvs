/** \file naming_service.cpp
 * Naming Service (NS)
 *
 * $Id: naming_service.cpp,v 1.9 2001/06/12 15:44:09 lecroart Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NeL Network Services.
 * NEVRAX NeL Network Services is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * NEVRAX NeL Network Services is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NeL Network Services; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

// todo remettre le log dans le log service mais il faut virer le prob de la recursion dans le send

#include <list>

#include "naming_service.h"

#include "nel/misc/debug.h"

#include "nel/net/unitime.h"
#include "nel/net/net_manager.h"

// DEBUG
#include "nel/misc/displayer.h"

using namespace std;
using namespace NLNET;
using namespace NLMISC;



///////////////////
///////////////////

struct CServiceEntry
{
	CServiceEntry (TSockId sock, CInetAddress a, string n, TServiceId s) : SockId(sock), Addr(a), Name(n), SId (s) { }

	// the connection between the service and the naming service
	TSockId			SockId;
	// address to send to the service who wants to lookup this service
	CInetAddress	Addr;
	// name of the service
	string			Name;
	// id of the service
	TServiceId		SId;
};

list<CServiceEntry>	RegisteredServices;

void displayRegisteredServices (CCallbackNetBase &netbase)
{
	nlinfo ("Display the %d registered services :", RegisteredServices.size());
	for (list<CServiceEntry>::iterator it = RegisteredServices.begin(); it != RegisteredServices.end (); it++)
	{
		nlinfo ("> %s '%s' %s-%hu '%s'", (*it).SockId->asString().c_str(), netbase.hostAddress((*it).SockId).asString().c_str(), (*it).Name.c_str(), (uint16)(*it).SId, (*it).Addr.asString().c_str());
	}
	nlinfo ("End ot the list");
}

///////////////////
///////////////////




// adress of the log service
/*CInetAddress ndaddr;
CNetDisplayer *nd = NULL;
*/


/*
 * CInetAddressRef
 */
/*
CInetAddressRef::CInetAddressRef() :
	CInetAddress(),
	_RefCounter( 0 ),
	_SId( 0 )
{}

CInetAddressRef::CInetAddressRef( const std::string& hostName, uint16 port, TServiceId sid ) :
	CInetAddress( hostName, port ),
	_RefCounter( 0 ),
	_SId( sid )
{}

CInetAddressRef::CInetAddressRef( const CInetAddressRef& other ) :
	CInetAddress( other ),
	_RefCounter( other._RefCounter ),
	_SId( other._SId )
{}

CInetAddressRef::CInetAddressRef( const CInetAddress& other ) :
	CInetAddress( other ),
	_RefCounter( 0 ),
	_SId ( 0 )
{}

CInetAddressRef& CInetAddressRef::operator=( const CInetAddressRef& other )
{
	CInetAddress::operator=( other );
	_RefCounter = other._RefCounter;
	_SId = other._SId;
	return *this;
}


typedef CNamingMap::iterator ItN;

/// The naming multimap
CNamingMap	NamingMap;
CAddressSet	AddressSet;
CSIdMap		SIdMap;
*/


/* Important note: the replies do not have a message type because the caller is expected to
 * make a synchronous receive() after sending a request to the NS.
 */


/*
 * Helper procedure for cbLookupAlternate and cbUnregister.
 * Note: name is used for a LOGS.
 */
list<CServiceEntry>::iterator doRemove (list<CServiceEntry>::iterator it)
{
	nldebug ("Unregister the service %s-%hu '%s'", (*it).Name.c_str(), (uint16)(*it).SId, (*it).Addr.asString().c_str());
	
/*	if ((*it).Name == "LOGS" && nd != NULL && ndaddr == (*it).Addr)
	{
		nlinfo ("The Log Service is down, stop logging my info on it!");

		// Remove the log service from displayer
		NetLog.removeDisplayer (nd);

		// Add the net displayer for all debug information
		ErrorLog->removeDisplayer (nd);
		WarningLog->removeDisplayer (nd);
		InfoLog->removeDisplayer (nd);
#ifdef NL_DEBUG
		DebugLog->removeDisplayer (nd);
		AssertLog->removeDisplayer (nd);
#endif

		delete nd;
		nd = NULL;
	}
*/
	// tell to everybody that this service is unregistered

	CMessage msgout ("UNB");
	//	uint16 vt = CNamingService::ValidTime;
	msgout.serial ((*it).Name);
	msgout.serial ((*it).SId);
	msgout.serial ((*it).Addr);
	CNetManager::send ("NS", msgout, 0);
	nlinfo ("Broadcast the Unregistration to everybody");

	// remove the service from the registered service list
	return RegisteredServices.erase (it);
}

void doUnregisterService (TServiceId sid)
{
	list<CServiceEntry>::iterator it;
	for (it = RegisteredServices.begin(); it != RegisteredServices.end (); it++)
	{
		if ((*it).SId == sid)
		{
			// found it, remove it
			doRemove (it);
			return;
		}
	}
	nlwarning ("Service %hu not found", (uint16)sid);
}

void doUnregisterService (TSockId from)
{
	list<CServiceEntry>::iterator it;
	for (it = RegisteredServices.begin(); it != RegisteredServices.end ();)
	{
		if ((*it).SockId == from)
		{
			// it's possible that one "from" have more than one registred service, so we have to find in all the list
			// found it, remove it
			it = doRemove (it);
		}
		else
		{
			it++;
		}
	}
}

void doUnregisterService (const CInetAddress &addr)
{
	list<CServiceEntry>::iterator it;
	for (it = RegisteredServices.begin(); it != RegisteredServices.end (); it++)
	{
		if ((*it).Addr == addr)
		{
			// found it, remove it
			doRemove (it);
			return;
		}
	}
	nlwarning ("Service %s not found", addr.asString().c_str());
}

/*
void doUnregisterService( const CInetAddress& addr, CSIdMap::iterator isidm, const string& name )
{
	// Remove address from AddressSet and SId from SIdMap
	string addrstr = addr.asString(); // save for displaying later
	AddressSet.erase( addr );

	ItN inm;
	if ( isidm!=SIdMap.end() )
	{
		// Iterator on SId provided
		inm = (*isidm).second;
		SIdMap.erase( isidm );
	}
	else
	{
		// Iterator on SId not provided => search in SIdMap
		for ( isidm=SIdMap.begin(); isidm!=SIdMap.end(); ++isidm )
		{
			if ( (*(*isidm).second).second == addr )
			{
				break;
			}
		}
		if ( isidm!=SIdMap.end() )
		{
			inm = (*isidm).second;
			SIdMap.erase( isidm );
		}
	}
*/
	// Remove association from NamingMap
	/*pair<ItN,ItN> range = NamingMap.equal_range( name );
	for ( inm = range.first; inm != range.second; ++inm )
	{
		if ( (*inm).second == addr )
		{*/
/*
	string servicename = (*inm).first;
	TServiceId sid = (*inm).second.sId();
	NamingMap.erase( inm );
	nlinfo( "Service %s-%hu unregistered at %s", servicename.c_str(), (uint16)sid, addrstr.c_str() );
*//****
	if (name == "LOGS" && nd != NULL)
	{
		// Remove the log service from displayer
		NetLog.removeDisplayer (nd);

		// Add the net displayer for all debug information
		ErrorLog.removeDisplayer (nd);
		WarningLog.removeDisplayer (nd);
		InfoLog.removeDisplayer (nd);
#ifdef NL_DEBUG
		DebugLog.removeDisplayer (nd);
		AssertLog.removeDisplayer (nd);
#endif

		delete nd;
		nd = NULL;
	}
*/			
	/*		return; // assuming there is only one pair (name,addr)
		}
	}*/
//}


/*
 * Helper function for cbLookup and cbLookupAlternate.
 * Returns NULL if service not found
 */
/*CInetAddressRef *doLookupService( const string& name )
{
	// Search service name in NamingMap
	pair<ItN,ItN> range = NamingMap.equal_range( name );
	if ( range.first != range.second )
	{
		return &((*range.first).second); // we don't select the best one yet
*/		/*for ( ItN inm = range.first; inm != range.second; ++inm )
		{
		}
		*/
/*	}
	else
	{
		return NULL;
	}
}
*/

/*
 * Helper function for cbLookupSId and cbLookupAlternateSId.			*********** OUT OF DATE ***************
 * Returns NULL if service not found
 */
/*CInetAddressRef *doLookupServiceBySId( TServiceId sid )
{
	CSIdMap::iterator isidm;
	if ( (isidm=SIdMap.find(sid)) != SIdMap.end() )
	{
		return &((*(*isidm).second).second);
	}
	else
	{
		return NULL;
	}
}
*/
void doLookup (const string &sname, TServiceId sid, TSockId from, CCallbackNetBase &netbase, bool sendAll, bool useSId)
{
	nlstop;

	string name;

	if (useSId)
		name = toString (sid);
	else
		name = sname;

	nlinfo ("Lookup for service '%s' for '%s'...", name.c_str(), netbase.hostAddress(from).asString().c_str());

	displayRegisteredServices (netbase);

	// Send list
	vector<CInetAddress> addrs;
	list<CServiceEntry>::iterator it;
	for (it = RegisteredServices.begin(); it != RegisteredServices.end (); it++)
	{
		if ((useSId && (*it).SId == sid) || (!useSId && (*it).Name == name))
		{
			addrs.push_back ((*it).Addr);
			// if it only want one, stop now to search
			if (!sendAll) break;
		}
	}

	if (addrs.size()==0)
		nldebug ("Service %s is not found", name.c_str());
	else if (addrs.size()==1)
		nldebug ("Service %s is at %s", name.c_str(), addrs[0].asString().c_str());
	else
		nldebug ("%d Services %s found", addrs.size(), name.c_str());
	
	CMessage msgout ("LK");
	msgout.serialCont (addrs);
	netbase.send (msgout, from);
}

/**
 * Callback for service look-up.			*********** OUT OF DATE ***************
 *
 * Message expected : LK
 * - Name of service to find (string)
 *
 * Message emitted : LK
 * - Validity time in seconds, or 0 if service not found (uint16)
 * - Address of service if service found, otherwise nothing (CInetAddress)
 *
 * \todo Olivier: Select the best service provider, not the first one in the list
 */
static void cbLookup (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	nlstop;

	string name;
	msgin.serial (name);
	
	// Find and return a service
	doLookup (name, 0, from, netbase, false, false);

////////////////////////////
/*
	// Return server
	CInetAddressRef *addr = doLookupService( name );
	if ( addr != NULL )
	{
		// Send address
		CMessage msgout( "" ); // NS
		uint16 vt = CNamingService::ValidTime;
		msgout.serial( vt );
		msgout.serial( static_cast<CInetAddress&>( *addr ) );
		CMsgSocket::send( msgout, from );

		// Increment reference counter
		addr->incCounter();
		nlinfo( "Service %s found at %s for %s", name.c_str(), addr->asString().c_str(), CMsgSocket::addressFromId( from )->asString().c_str() );
	}
	else
	{
		// Not found
		CMessage msgout( "" ); // NS
		uint16 v = 0;
		msgout.serial( v );
		CMsgSocket::send( msgout, from );
		nlinfo( "Service %s not found", name.c_str() );
	}
*/}

/**
 * Callback for alternate service look-up when a service is not responding			*********** OUT OF DATE ***************
 *
 * Message expected : LA
 * - Name of service (string)
 * - Address of server not responding (CInetAddress)
 *
 * Message emitted : no name (NS)
 * - Validity time in seconds, or 0 if service not found (uint16)
 * - Address of service if service found, otherwise nothing (CInetAddress)
 */
static void cbLookupAlternate (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	nlstop;

	string name;
	CInetAddress addr;
	msgin.serial (name);
	msgin.serial (addr);

	nlinfo ("Server %s looks down, looking-up for alternative service %s", addr.asString().c_str(), name.c_str() );

	// Unregister down server
	doUnregisterService (addr);

	// Find and return another servive
	doLookup (name, 0, from, netbase, false, false);

/*	// Return another server
	CInetAddressRef *newaddr = doLookupService( name );
	if ( newaddr != NULL )
	{
		// Send address
		CMessage msgout( "" ); // NS
		uint16 vt = CNamingService::ValidTime;
		msgout.serial( vt );
		msgout.serial( static_cast<CInetAddress&>( *newaddr ) );
		CMsgSocket::send( msgout, from );

		// Increment reference counter
		newaddr->incCounter();
		nlinfo( "Service %s found at %s for %s", name.c_str(), newaddr->asString().c_str(), CMsgSocket::addressFromId( from )->asString().c_str() );
	}
	else
	{
		// Not found
		CMessage msgout( "" ); // NS
		uint16 v = 0;
		msgout.serial( v );
		CMsgSocket::send( msgout, from );
		nlinfo( "Service %s not found", name.c_str() );
	}		
*/}

/**
 * Callback for service look-up for all corresponding to a name.			*********** OUT OF DATE ***************
 *
 * Message expected : LKA
 * - Name of service to find (string)
 *
 * Message emitted : no name (ALKA)
 * - List of addresses (vector<CInetAddress>)
 */
static void cbLookupAll (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	nlstop;

	// Receive name
	string name;
	msgin.serial (name);

	// Find and return a service
	doLookup (name, 0, from, netbase, true, false);
}

/**
 * Callback for service look-up by identifier				*********** OUT OF DATE ***************
 *
 * Message expected : LKI
 * - Identifier of service to find (TServiceId)
 *
 * Message emitted : no name (NS)
 * - Validity time in seconds, or 0 if service not found (uint16)
 * - Address of service if service found, otherwise nothing (CInetAddress)
 *
 * \todo Olivier: Select the best service provider, not the first one in the list
 */
static void cbLookupSId (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	nlstop;

	// Receive id
	TServiceId sid;
	msgin.serial (sid);

	// Find and return a service
	doLookup ("", sid, from, netbase, false, true);

/*
	 nlinfo( "Lookup for service %hu", (uint16)sid );

	// Return server
	CInetAddressRef *addr = doLookupServiceBySId( sid );
	if ( addr != NULL )
	{
		// Send address
		CMessage msgout( "" ); // NS
		uint16 vt = CNamingService::ValidTime;
		msgout.serial( vt );
		msgout.serial( static_cast<CInetAddress&>( *addr ) );
		CMsgSocket::send( msgout, from );

		// Increment reference counter
		addr->incCounter();
		nlinfo( "Service %hu found at %s for %s", (uint16)sid, addr->asString().c_str(), CMsgSocket::addressFromId( from )->asString().c_str() );
	}
	else
	{
		// Not found
		CMessage msgout( "" ); // NS
		uint16 v = 0;
		msgout.serial( v );
		CMsgSocket::send( msgout, from );
		nlinfo( "Service %hu not found", (uint16)sid );
	}
*/}


/**
 * Callback for alternate service look-up when a service is not responding
 *
 * Message expected : LA
 * - Name of service (string)
 * - Address of server not responding (CInetAddress)
 *
 * Message emitted : no name (NS)
 * - Validity time in seconds, or 0 if service not found (uint16)
 * - Address of service if service found, otherwise nothing (CInetAddress)
 */
/*static void cbLookupAlternate (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	string name;
	CInetAddress addr;
	message.serial( name );
	message.serial( addr );
	nlinfo( "Server %s looks down, looking-up for alternative service %s", addr.asString().c_str(), name.c_str() );

	// Unregister down server
	doUnregisterService( addr, SIdMap.end(), name );

	// Return another server
	CInetAddressRef *newaddr = doLookupService( name );
	if ( newaddr != NULL )
	{
		// Send address
		CMessage msgout( "" ); // NS
		uint16 vt = CNamingService::ValidTime;
		msgout.serial( vt );
		msgout.serial( static_cast<CInetAddress&>( *newaddr ) );
		CMsgSocket::send( msgout, from );

		// Increment reference counter
		newaddr->incCounter();
		nlinfo( "Service %s found at %s for %s", name.c_str(), newaddr->asString().c_str(), CMsgSocket::addressFromId( from )->asString().c_str() );
	}
	else
	{
		// Not found
		CMessage msgout( "" ); // NS
		uint16 v = 0;
		msgout.serial( v );
		CMsgSocket::send( msgout, from );
		nlinfo( "Service %s not found", name.c_str() );
	}		
}*/


/**
 * Callback for alternate service look-up by identifier when a service is not responding
 *
 * Message expected : LAI
 * - Identifier of service (TServiceId)
 * - Address of server not responding (CInetAddress)
 *
 * Message emitted : no name (NS)
 * - Validity time in seconds, or 0 if service not found (uint16)
 * - Address of service if service found, otherwise nothing (CInetAddress)
 */
/*static void cbLookupAlternateSId (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	TServiceId sid;
	CInetAddress addr;
	message.serial( sid );
	message.serial( addr );
	nlinfo( "Server %s looks down, looking-up for alternative service %hu", addr.asString().c_str(), (uint16)sid );

	// Unregister down server
	doUnregisterService( addr, SIdMap.find(sid), "" );

	// Return another server
	CInetAddressRef *newaddr = doLookupServiceBySId( sid );
	if ( newaddr != NULL )
	{
		// Send address
		CMessage msgout( "" ); // NS
		uint16 vt = CNamingService::ValidTime;
		msgout.serial( vt );
		msgout.serial( static_cast<CInetAddress&>( *newaddr ) );
		CMsgSocket::send( msgout, from );

		// Increment reference counter
		newaddr->incCounter();
		nlinfo( "Service %u found at %s for %s", (uint16)sid, newaddr->asString().c_str(), CMsgSocket::addressFromId( from )->asString().c_str() );
	}
	else
	{
		// Not found
		CMessage msgout( "" ); // NS
		uint16 v = 0;
		msgout.serial( v );
		CMsgSocket::send( msgout, from );
		nlinfo( "Service %hu not found", (uint16)sid );
	}		
}*/


/*
 * Helper function for cbRegister.
 * If alloc_sid is true, sid is ignored
 * Returns false in case of failure of sid allocation or bad sid provided
 * Note: the reply is included in this function, because it must be done before things such as syncUniTime()
 */
bool doRegister (const string &name, const CInetAddress &addr, TServiceId sid, TSockId from, CCallbackNetBase &netbase)
{/*
	CInetAddressRef addrr ( addr );

	// Check if the association is not already in NamingMap
	bool replace_entry = false;
	pair<ItN,ItN> range = NamingMap.equal_range( name );
	ItN inm;
	for ( inm=range.first; inm!=range.second; ++inm )
	{
		if ( (*inm).second == addr ) // uses CInetAddress comparison operator (does not compare ref counter and SId)
		{
			replace_entry = true;
			break;
		}
	}
	if ( alloc_sid )
	{
		if ( replace_entry )
		{
			// Get the same sid
			sid = (*inm).second.sId();
		}
		else
		{
			// Allocate service identifier, searching for a free SId
			if ( name == "AS" )
			{
				sid = 0; // AS: 0 to 127
			}
			else		 // Others: 128 to 255
			{
				sid = CNamingService::BaseSId;
			}
			while ( SIdMap.find( sid ) != SIdMap.end() )
			{
				sid++;
				if ( sid == 0 ) // round the clock (TODO: stop at 128 for AS?)
				{
					nlwarning( "Service identifier allocation overflow" );
					break;
				}
			}
		}
		// answer of the registration
//		CMessage msgout ("RG");
//		msgout.serial (sid);
//		netbase.send (msgout, from);
		if ( sid == 0 )
		{
			return false;
		}
	}
	else
	{
		// Set specified service identifier
		bool ok;
		if ( replace_entry )
		{
			// Check that the same sid is being attributed
			ok = (sid == (*inm).second.sId());
		}
		else
		{
			// Check that the sid is not attributed yet
			ok = ( SIdMap.find(sid) == SIdMap.end() );
		}
		CMessage msgout ("RGI");
		msgout.serial (ok);
		netbase.send ( msgout, from );
		if ( ! ok )
		{
			return false;
		}
	}
	addrr.setSId( sid );
*/
//////////////

	// find if the service is not already registered
	uint8 ok = true;
	bool needRegister = true;
	for (list<CServiceEntry>::iterator it = RegisteredServices.begin(); it != RegisteredServices.end (); it++)
	{
		if ((*it).Addr.asIPString() == addr.asIPString() )
		{
			// we already have a service on this address, remplace it if it's the same name
			if ((*it).Name == name)
			{
				// it's the same service, replace it
				(*it).SockId = from;
				sid = (*it).SId;
				nlinfo ("Replace the service %s", name.c_str());
			}
			else
			{
				nlwarning ("Try to register %s to %s but the service %s already on this address. ignore it!", name.c_str(), addr.asIPString().c_str(), (*it).Name.c_str());
				ok = false;
			}
			needRegister = false;
			break;
		}
	}

	if (needRegister)
	{
		if (sid == 0)
		{
			// we have to find a sid
			sid = CNamingService::BaseSId;
			bool found = false;
			while (!found)
			{
				list<CServiceEntry>::iterator it;
				for (it = RegisteredServices.begin(); it != RegisteredServices.end (); it++)
				{
					if ((*it).SId == sid)
					{
						break;
					}
				}
				if (it == RegisteredServices.end ())
				{
					// ok, we have an empty sid
					found = true;
				}
				else
				{
					sid++;
					if (sid == 0) // round the clock
					{
						nlwarning ("Service identifier allocation overflow");
						ok = false;
						break;
					}
				}
			}
		}
		else
		{
			// we have to check that the user provided sid is available
			list<CServiceEntry>::iterator it;
			for (it = RegisteredServices.begin(); it != RegisteredServices.end (); it++)
			{
				if ((*it).SId == sid)
				{
					ok = false;
					break;
				}
			}
			if (it != RegisteredServices.end ())
			{
				ok = true;
			}
		}

		// if ok, register the service and send a broadcast to other people
		if (ok)
		{
			RegisteredServices.push_back (CServiceEntry(from, addr, name, sid));

			// tell to everybody that this service is registered

			CMessage msgout ("RGB");
			uint8 s = 1;
			msgout.serial (s);
			msgout.serial (const_cast<string &>(name));
			msgout.serial (sid);
			msgout.serial (const_cast<CInetAddress &>(addr));
			CNetManager::send ("NS", msgout, 0);
			nlinfo ("Broadcast the Registration to everybody");
		}
	}

	// send the answer to the client
	CMessage msgout ("RG");
	msgout.serial (ok);
	if (ok) msgout.serial (sid);
	netbase.send (msgout, from);
	netbase.flush (from);

	displayRegisteredServices (netbase);

	if (ok)
	{
		//
		// now, for specific services, we do special things
		//
		
		// for the Time Service, we sync the time
		if (name == "TS" && !CUniTime::Sync)
		{
			// don't call the sync if it's the Time Service and Naming Service
			nlinfo ("I found a Time Service, get the Unified time!");
			CUniTime::syncUniTimeFromService (&addr);
		}
/*		else if (name == "LOGS" && nd == NULL)
		{
			// we have a log service, so we can log our message!
			// Setup Net Displayer
			nlinfo ("I found a Log Service, Try to put my info on it!");
			nd = new CNetDisplayer(false);
			nd->setLogServer (addr);
			ndaddr = addr;

			if ( nd->connected() )
			{
				NetLog.addDisplayer (nd);

				// Add the net displayer for all debug information
				ErrorLog->addDisplayer (nd);
				WarningLog->addDisplayer (nd);
				InfoLog->addDisplayer (nd);
#ifdef NL_DEBUG
				DebugLog->addDisplayer (nd);
				AssertLog->addDisplayer (nd);
#endif
			}
			else
			{
				delete nd;
				nd = NULL;
			}
		}
*/	}

	return ok!=0;

///////////
/*


	// Insert association in NamingMap and in SIdMap
	if ( replace_entry )
	{
		nlinfo( "Service %s-%hu replaced at %s", name.c_str(), (uint16)sid, addr.asString().c_str() );
	}
	else
	{
		inm = NamingMap.insert( make_pair( name, addrr ) );
		SIdMap.insert( make_pair(sid,inm) );
		nlinfo( "Service %s-%hu registered at %s", name.c_str(), (uint16)sid, addr.asString().c_str() );

		// Insert address in AddressSet if it has not been allocated by the naming service
		if ( AddressSet.find( addr ) == AddressSet.end() )
		{
			AddressSet.insert( addr );
		}

		// Get the universal time (useful for debugging)
*//****		if (name == "TS" && !CUniTime::Sync)
		{
			// don't call the sync if it's the Time Service and Naming Service
			nlinfo ("I found a Time Service, get the Unified time!");
			CUniTime::syncUniTimeFromService (&addr);
		}
		else if (name == "LOGS" && nd == NULL)
		{
			// we have a log service, so we can log our message!
			// Setup Net Displayer
			nlinfo ("I found a Log Service, Try to put my info on it!");
			nd = new CNetDisplayer(false);
			nd->setLogServer (addr);

			if ( nd->connected() )
			{
				NetLog.addDisplayer (nd);

				// Add the net displayer for all debug information
				ErrorLog.addDisplayer (nd);
				WarningLog.addDisplayer (nd);
				InfoLog.addDisplayer (nd);
	#ifdef NL_DEBUG
				DebugLog.addDisplayer (nd);
				AssertLog.addDisplayer (nd);
	#endif
			}
			else
			{
				delete nd;
				nd = NULL;
			}
			
		}
	}*/
/*
#ifdef NL_DEBUG
	NLMISC::DebugLog->displayNL( "List of registered services:" );
	for ( inm=NamingMap.begin(); inm!=NamingMap.end(); ++inm )
	{
		NLMISC::DebugLog->displayRawNL( "> %s %s", (*inm).first.c_str(), (*inm).second.asString().c_str() );
	}
	NLMISC::DebugLog->displayRawNL( "End of list" );
#endif
*/
//	return true;
}


/**
 * Callback for service registration.
 *
 * Message expected : RG
 * - Name of service to register (string)
 * - Address of service (CInetAddress)
 *
 * Message emitted : RG
 * - Allocated service identifier (TServiceId) or 0 if failed
 */
static void cbRegister (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	string name;
	CInetAddress addr;
	TServiceId sid;
	msgin.serial (name);
	msgin.serial (addr);
	msgin.serial (sid);

	doRegister (name, addr, sid, from, netbase);
}


/**
 * Callback for service registration.
 *
 * Message expected : RGI
 * - Name of service to register (string)
 * - Address of service (CInetAddress)
 * - Service identifier (TServiceId)
 *
 * Message emitted : no name (ASID)
 * - Result of registration (bool)
 */
/*static void cbRegisterWithSId (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	string name;
	CInetAddress addr;
	TServiceId sid;
	message.serial( name );
	message.serial( addr );
	message.serial( sid );

	bool ok = doRegister( name, addr, false, sid, from );
}*/


/**
 * Callback for service unregistration.
 *
 * Message expected : UN
 * - Name of service to register (string)
 * - Address of service (CInetAddress)
 * \todo cado Add service authentification
 */
/*static void cbUnregister (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	string name;
	CInetAddress addr;
	message.serial( name );
	message.serial( addr );

	doUnregisterService( addr, SIdMap.end(), name );
}*/


/**
 * Callback for service unregistration.
 *
 * Message expected : UNI
 * - Service identifier (TServiceId)
 */
static void cbUnregisterSId (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	TServiceId sid;
	msgin.serial( sid );

	doUnregisterService (sid);
	displayRegisteredServices (netbase);

///////////////
/*	CSIdMap::iterator isidm;
	if ( (isidm=SIdMap.find(sid)) != SIdMap.end() )
	{
		doUnregisterService( (*(*isidm).second).second, isidm, "" );
	}
*/}


/*
 * Helper function for cbQueryPort
 */
uint16 doAllocatePort (const CInetAddress &addr)
{
	uint16 port = CNamingService::BasePort;
	bool found = false;
	while (!found)
	{
		list<CServiceEntry>::iterator it;
		for (it = RegisteredServices.begin(); it != RegisteredServices.end (); it++)
		{
			if ((*it).Addr.port () == port)
			{
				break;
			}
		}
		if (it == RegisteredServices.end ())
		{
			// ok, we have an empty sid
			found = true;
		}
		else
		{
			port++;
			if (port == 0) // round the clock
			{
				nlwarning ("Service port allocation overflow");
				break;
			}
		}
	}

/*	nldebug( "Searching %s in :", addr2.asString().c_str() );
	for ( CAddressSet::iterator it=AddressSet.begin(); it!=AddressSet.end(); ++it )
	{
		NLMISC::DebugLog->displayRawNL( "> %s", (*it).asString().c_str() );
	}

	// Find a free address
	while ( AddressSet.find( addr2 ) != AddressSet.end() )
	{
		nldebug( "Port %hu is not free", port );
		port++;
		addr2.setPort( port );
		if ( port == 0 ) // round the clock
		{
			nlwarning( "Port number allocation overflow" );
			break;
		}
	}

	// Insert address in AddressSet
	AddressSet.insert( addr2 );
*/	
	return port;
}


/**
 * Callback for port allocation
 * Note: if a service queries a port but does not register itself to the naming service, the
 * port will remain allocated and unused.
 *
 * Message expected : QP
 * - Name of service to register (string)
 * - Address of service (CInetAddress) (its port can be 0)
 *
 * Message emitted : QP
 * - Allocated port number (uint16)
 */
static void cbQueryPort (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	// Allocate port
	uint16 port = doAllocatePort (netbase.hostAddress (from));

	// Send port back
	CMessage msgout ("QP");
	msgout.serial (port);
	netbase.send (msgout, from);

	nlinfo ("A service got port %hu", port);
}


/**
 * Callback for service look-up for all corresponding to a name (+service id).
 *
 * Message expected : LKS
 * - Name of service to find (string)
 *
 * Message emitted : no name (ALKS)
 * - List of service ids and corresponding addresses (map<TServiceId,CInetAddress>)
 */
/*static void cbLookupAllServices (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	// Receive name
	string name;
	message.serial( name );

	// Send list
	map<TServiceId,CInetAddress> addresses;
	pair<ItN,ItN> range = NamingMap.equal_range( name );
	ItN inm;
	for ( inm=range.first; inm!=range.second; ++inm )
	{
		addresses.insert( make_pair((*inm).second.sId(),(CInetAddress)((*inm).second)) );
	}
	CMessage msgout( "" ); // ALKS
	msgout.serialCont( addresses );
	CMsgSocket::send( msgout, from );

	nlinfo( "%d services found", addresses.size() );
}*/


/*
 * Unregisters a service if it has not been done before.
 * Note: this callback is called whenever someone disconnects from the NS.
 * May be there are too many calls if many clients perform many transactional lookups.
 */
static void cbDisconnect (const string &serviceName, TSockId from, void *arg)
{
	doUnregisterService (from);
	displayRegisteredServices (*CNetManager::getNetBase(serviceName));
	
/////////
/*	CInetAddress addr;
	addr = CNetManager::getNetBase(serviceName)->hostAddress(from);
	if ( AddressSet.find( addr ) != AddressSet.end() )
	{
		nlinfo( "Unregistering a disconnected service..." );
		doUnregisterService( addr, SIdMap.end(), "" );
	}
*/
}

/*
 * a service is connected, send him all services infos
 */
static void cbConnect (const string &serviceName, TSockId from, void *arg)
{
	CMessage msgout ("RGB");

	uint8 s = RegisteredServices.size ();
	msgout.serial (s);

	for (list<CServiceEntry>::iterator it = RegisteredServices.begin(); it != RegisteredServices.end (); it++)
	{
		msgout.serial ((*it).Name);
		msgout.serial ((*it).SId);
		msgout.serial ((*it).Addr);
	}
	CNetManager::send ("NS", msgout, from);

	nlinfo ("Sending all services available to the new client");
	displayRegisteredServices (*CNetManager::getNetBase(serviceName));
}


/** Callback Array
 * Messages types:
 * - LK: service look-up
 * - RG: service registration
 * - UN: service unregistration
 */
TCallbackItem CallbackArray[] =
{
//	{ "LK", cbLookup },				// now the client have a cache so it haven't to ask to the naming_service
//	{ "LA", cbLookupAlternate },	/// \todo the client doesn't say anymore that a service doesn't work
	{ "RG", cbRegister },
//	{ "UN", cbUnregister },

	{ "QP", cbQueryPort },

//	{ "LKI", cbLookupSId },
//	{ "LAI", cbLookupAlternateSId },
//	{ "RGI", cbRegisterWithSId },	// done by RG now
	{ "UNI", cbUnregisterSId },

//	{ "LKA", cbLookupAll },
//	{ "LKS", cbLookupAllServices },
};


void CNamingService::init()
{
	// we don't try to associate message from client
	CNetManager::getNetBase ("NS")->ignoreAllUnknownId (true);

	// add the callback in case of disconnection
	CNetManager::setConnectionCallback ("NS", cbConnect, NULL);
	
	// add the callback in case of disconnection
	CNetManager::setDisconnectionCallback ("NS", cbDisconnect, NULL);

	// DEBUG
	// DebugLog->addDisplayer( new CStdDisplayer() );
}


/// Default validity time is 2 minutes
const uint16		CNamingService::ValidTime = 120;

/// Ports begin at 51000 (note: in this implementation there can be no more than 14536 services)
const uint16		CNamingService::BasePort = 51000;

/// Allocated SIds begin at 128 (except for Agent Service)
const TServiceId	CNamingService::BaseSId = 128;


/// Naming Service
NLNET_SERVICE_MAIN (CNamingService, "NS", "naming_service", 50000, CallbackArray);
