/** \file naming_service.cpp
 * Naming Service (NS)
 *
 * $Id: naming_service.cpp,v 1.1 2001/04/18 13:54:25 valignat Exp $
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

#include "naming_service.h"

#include "nel/misc/debug.h"

#include "nel/net/message.h"
#include "nel/net/msg_socket.h"
#include "nel/net/net_displayer.h"
#include "nel/net/unitime.h"
#include "nel/net/net_log.h"

using namespace std;
using namespace NLNET;
using namespace NLMISC;


/// Default validity time is 2 minutes
const uint16		CNamingService::ValidTime = 120;

/// Ports begin at 51000 (note: in this implementation there can be no more than 14536 services)
const uint16		CNamingService::BasePort = 51000;

/// Allocated SIds begin at 128 (except for Agent Service)
const TServiceId	CNamingService::BaseSId = 128;


CNetDisplayer *nd = NULL;



/*
 * CInetAddressRef
 */

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



/* Important note: the replies do not have a message type because the caller is expected to
 * make a synchronous receive() after sending a request to the NS.
 */


/*
 * Helper procedure for cbLookupAlternate and cbUnregister.
 * Note: name is used for a LOGS.
 */
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

	// Remove association from NamingMap
	/*pair<ItN,ItN> range = NamingMap.equal_range( name );
	for ( inm = range.first; inm != range.second; ++inm )
	{
		if ( (*inm).second == addr )
		{*/

	string servicename = (*inm).first;
	TServiceId sid = (*inm).second.sId();
	NamingMap.erase( inm );
	nlinfo( "Service %s-%hu unregistered at %s", servicename.c_str(), (uint16)sid, addrstr.c_str() );

	if (name == "LOGS" && nd != NULL)
	{
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
			
	/*		return; // assuming there is only one pair (name,addr)
		}
	}*/
}


/*
 * Helper function for cbLookup and cbLookupAlternate.
 * Returns NULL if service not found
 */
CInetAddressRef *doLookupService( const string& name )
{
	// Search service name in NamingMap
	pair<ItN,ItN> range = NamingMap.equal_range( name );
	if ( range.first != range.second )
	{
		return &((*range.first).second); // we don't select the best one yet
		/*for ( ItN inm = range.first; inm != range.second; ++inm )
		{
		}
		*/
	}
	else
	{
		return NULL;
	}
}


/*
 * Helper function for cbLookupSId and cbLookupAlternateSId.
 * Returns NULL if service not found
 */
CInetAddressRef *doLookupServiceBySId( TServiceId sid )
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


/**
 * Callback for service look-up.
 *
 * Message expected : LK
 * - Name of service to find (string)
 *
 * Message emitted : no name (NS)
 * - Validity time in seconds, or 0 if service not found (uint16)
 * - Address of service if service found, otherwise nothing (CInetAddress)
 *
 * \todo Olivier: Select the best service provider, not the first one in the list
 */
void cbLookup( CMessage& message, TSenderId from )
{
	string name;
	message.serial( name );
	nlinfo( "Lookup for service %s", name.c_str() );

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
}


/**
 * Callback for service look-up by identifier
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
void cbLookupSId( CMessage& message, TSenderId from )
{
	TServiceId sid;
	message.serial( sid );
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
}


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
void cbLookupAlternate( CMessage& message, TSenderId from )
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
}


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
void cbLookupAlternateSId( CMessage& message, TSenderId from )
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
}


/*
 * Helper function for cbRegister.
 * If alloc_sid is true, sid is ignored
 * Returns false in case of failure of sid allocation or bad sid provided
 * Note: the reply is included in this function, because it must be done before things such as syncUniTime()
 */
bool doRegister( const string& name, const CInetAddress& addr, bool alloc_sid, TServiceId sid, TSenderId from )
{
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
		CMessage msgout( "" ); // SID
		msgout.serial( sid );
		CMsgSocket::send( msgout, from );
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
		CMessage msgout( "" ); // ASID
		msgout.serial( ok );
		CMsgSocket::send( msgout, from );
		if ( ! ok )
		{
			return false;
		}
	}
	addrr.setSId( sid );

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
		if (name == "TS" && !CUniTime::Sync)
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
	}

#ifdef NL_DEBUG
	NLMISC::DebugLog->displayNL( "List of registered services:" );
	for ( inm=NamingMap.begin(); inm!=NamingMap.end(); ++inm )
	{
		NLMISC::DebugLog->displayRawNL( "> %s %s", (*inm).first.c_str(), (*inm).second.asString().c_str() );
	}
	NLMISC::DebugLog->displayRawNL( "End of list" );
#endif

	return true;
}


/**
 * Callback for service registration.
 *
 * Message expected : RG
 * - Name of service to register (string)
 * - Address of service (CInetAddress)
 *
 * Message emitted : no name (SID)
 * - Allocated service identifier (TServiceId) or 0 if failed
 */
void cbRegister( CMessage& message, TSenderId from )
{
	string name;
	CInetAddress addr;
	message.serial( name );
	message.serial( addr );

	doRegister( name, addr, true, 0, from );
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
void cbRegisterWithSId( CMessage& message, TSenderId from )
{
	string name;
	CInetAddress addr;
	TServiceId sid;
	message.serial( name );
	message.serial( addr );
	message.serial( sid );

	bool ok = doRegister( name, addr, false, sid, from );
}


/**
 * Callback for service unregistration.
 *
 * Message expected : UN
 * - Name of service to register (string)
 * - Address of service (CInetAddress)
 * \todo cado Add service authentification
 */
void cbUnregister( CMessage& message, TSenderId from )
{
	string name;
	CInetAddress addr;
	message.serial( name );
	message.serial( addr );

	doUnregisterService( addr, SIdMap.end(), name );
}


/**
 * Callback for service unregistration.
 *
 * Message expected : UNI
 * - Service identifier (TServiceId)
 */
void cbUnregisterSId( CMessage& message, TSenderId from )
{
	TServiceId sid;
	message.serial( sid );

	CSIdMap::iterator isidm;
	if ( (isidm=SIdMap.find(sid)) != SIdMap.end() )
	{
		doUnregisterService( (*(*isidm).second).second, isidm, "" );
	}
}


/*
 * Helper function for cbQueryPort
 */
uint16 doAllocatePort( const string& name, const CInetAddress& addr )
{
	// In this implementation, we do not use name.
	uint16 port = CNamingService::BasePort;
	CInetAddress addr2( addr );
	addr2.setPort( port );

	nldebug( "Searching %s in :", addr2.asString().c_str() );
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
 * Message emitted : no name (AQP)
 * - Allocated port number (uint16)
 */
void cbQueryPort( CMessage& message, TSenderId from )
{
	string name;
	CInetAddress addr;
	message.serial( name );
	message.serial( addr );

	// Allocate port
	uint16 port = doAllocatePort( name, addr );

	// Send port back
	CMessage msgout( "" ); // AQP
	msgout.serial( port );
	CMsgSocket::send( msgout, from );

	nlinfo( "Service %s got port %hu", name.c_str(), port );
}


/**
 * Callback for service look-up for all corresponding to a name.
 *
 * Message expected : LKA
 * - Name of service to find (string)
 *
 * Message emitted : no name (ALKA)
 * - List of addresses (vector<CInetAddress>)
 */
void cbLookupAll( CMessage& message, TSenderId from )
{
	// Receive name
	string name;
	message.serial( name );

	// Send list
	vector<CInetAddress> addresses;
	pair<ItN,ItN> range = NamingMap.equal_range( name );
	ItN inm;
	for ( inm=range.first; inm!=range.second; ++inm )
	{
		addresses.push_back( (CInetAddress)((*inm).second) );
	}
	CMessage msgout( "" ); // ALKA
	msgout.serialCont( addresses );
	CMsgSocket::send( msgout, from );

	nlinfo( "%d services found", addresses.size() );
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
void cbLookupAllServices( CMessage& message, TSenderId from )
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
	msgout.serialMap( addresses );
	CMsgSocket::send( msgout, from );

	nlinfo( "%d services found", addresses.size() );
}


/*
 * Unregisters a service if it has not been done before.
 * Note: this callback is called whenever someone disconnects from the NS.
 * May be there are too many calls if many clients perform many transactional lookups.
 */
void cbDisconnect( CMessage& message, TSenderId from )
{
	CInetAddress addr;
	message.serial( addr );
	if ( AddressSet.find( addr ) != AddressSet.end() )
	{
		nlinfo( "Unregistering a disconnected service..." );
		doUnregisterService( addr, SIdMap.end(), "" );
	}
}


/** Callback Array
 * Messages types:
 * - LK: service look-up
 * - RG: service registration
 * - UN: service unregistration
 */
TCallbackItem CallbackArray[] =
{
	{ "LK", cbLookup },
	{ "LA", cbLookupAlternate },
	{ "RG", cbRegister },
	{ "UN", cbUnregister },

	{ "QP", cbQueryPort },

	{ "LKI", cbLookupSId },
	{ "LAI", cbLookupAlternateSId },
	{ "RGI", cbRegisterWithSId },
	{ "UNI", cbUnregisterSId },

	{ "LKA", cbLookupAll },
	{ "LKS", cbLookupAllServices },
	{ "D", cbDisconnect }
};


/// Naming Service
NLNET_SERVICE_MAIN( CNamingService, "NS", 50000 );
