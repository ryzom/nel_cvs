/** \file naming_service.cpp
 * Naming Service (NS)
 *
 * $Id: naming_service.cpp,v 1.19 2002/03/19 17:42:49 valignat Exp $
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#ifndef NELNS_CONFIG
#define NELNS_CONFIG ""
#endif // NELNS_CONFIG

#ifndef NELNS_LOGS
#define NELNS_LOGS ""
#endif // NELNS_LOGS

//
// Includes
//

#include <list>
#include <string>

#include "nel/misc/debug.h"
#include "nel/misc/command.h"
#include "nel/misc/displayer.h"

#include "nel/net/service.h"
#include "nel/net/net_manager.h"


//
// Namespaces
//

using namespace std;

using namespace NLMISC;
using namespace NLNET;


//
// Structures
//

struct CServiceEntry
{
	CServiceEntry (TSockId sock, CInetAddress a, string n, TServiceId s) : SockId(sock), Addr(a), Name(n), SId (s) { }

	TSockId			SockId;			// the connection between the service and the naming service
	CInetAddress	Addr;			// address to send to the service who wants to lookup this service
	string			Name;			// name of the service
	TServiceId		SId;			// id of the service
};


//
// Variables
//

list<CServiceEntry>	RegisteredServices;		/// List of all registred services

uint16				MinBasePort = 51000;	/// Ports begin at 51000
uint16				MaxBasePort = 52000;	/// (note: in this implementation there can be no more than 1000 services)

const TServiceId	BaseSId = 128;			/// Allocated SIds begin at 128 (except for Agent Service)


//
// Functions
//

void displayRegisteredServices (CCallbackNetBase &netbase)
{
	nlinfo ("Display the %d registered services :", RegisteredServices.size());
	for (list<CServiceEntry>::iterator it = RegisteredServices.begin(); it != RegisteredServices.end (); it++)
	{
		nlinfo ("> %s '%s' %s-%hu '%s'", (*it).SockId->asString().c_str(), netbase.hostAddress((*it).SockId).asString().c_str(), (*it).Name.c_str(), (uint16)(*it).SId, (*it).Addr.asString().c_str());
	}
	nlinfo ("End ot the list");
}



/*
 * Helper procedure for cbLookupAlternate and cbUnregister.
 * Note: name is used for a LOGS.
 */
list<CServiceEntry>::iterator doRemove (list<CServiceEntry>::iterator it)
{
	nldebug ("Unregister the service %s-%hu '%s'", (*it).Name.c_str(), (uint16)(*it).SId, (*it).Addr.asString().c_str());
	
	// tell to everybody that this service is unregistered

	CMessage msgout ("UNB");
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
 * Helper function for cbLookupSId and cbLookupAlternateSId.			*********** OUT OF DATE ***************
 * Returns NULL if service not found
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
}

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
}

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
}



/*
 * Helper function for cbRegister.
 * If alloc_sid is true, sid is ignored
 * Returns false in case of failure of sid allocation or bad sid provided
 * Note: the reply is included in this function, because it must be done before things such as syncUniTime()
 */
bool doRegister (const string &name, const CInetAddress &addr, TServiceId sid, TSockId from, CCallbackNetBase &netbase)
{
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
			sid = BaseSId;
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

	return ok!=0;
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
}


/*
 * Helper function for cbQueryPort
 *
 * \warning QueryPort + Registration is not atomic so more than one service could ask a port before register
 */
uint16 doAllocatePort (const CInetAddress &addr)
{
	static uint16 nextAvailablePort = MinBasePort;

	// check if nextavailableport is free

	if (nextAvailablePort >= MaxBasePort) nextAvailablePort = MinBasePort;

	bool ok;
	do
	{
		ok = true;
		list<CServiceEntry>::iterator it;
		for (it = RegisteredServices.begin(); it != RegisteredServices.end (); it++)
		{
			if ((*it).Addr.port () == nextAvailablePort)
			{
				nextAvailablePort++;
				ok = false;
				break;
			}
		}
	}
	while (!ok);

	return nextAvailablePort++;
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


/*
 * Unregisters a service if it has not been done before.
 * Note: this callback is called whenever someone disconnects from the NS.
 * May be there are too many calls if many clients perform many transactional lookups.
 */
static void cbDisconnect (const string &serviceName, TSockId from, void *arg)
{
	doUnregisterService (from);
	displayRegisteredServices (*CNetManager::getNetBase(serviceName));
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


//
// Callback array
//

TCallbackItem CallbackArray[] =
{
	{ "RG", cbRegister },
	{ "QP", cbQueryPort },
	{ "UNI", cbUnregisterSId },
};


//
// Service
//

class CNamingService : public NLNET::IService
{
public:

	void init()
	{
		// if a baseport is available in the config file, get it
		try
		{
			uint16 newBasePort = ConfigFile.getVar ("BasePort").asInt ();
			nlinfo ("Changing the MinBasePort number from %hu to %hu", MinBasePort, newBasePort);
			sint32 delta = MaxBasePort - MinBasePort;
			nlassert (delta > 0);
			MinBasePort = newBasePort;
			MaxBasePort = MinBasePort + uint16 (delta);
		}
		catch (EUnknownVar &)
		{
		}


		// we don't try to associate message from client
		CNetManager::getNetBase ("NS")->ignoreAllUnknownId (true);

		// add the callback in case of disconnection
		CNetManager::setConnectionCallback ("NS", cbConnect, NULL);
		
		// add the callback in case of disconnection
		CNetManager::setDisconnectionCallback ("NS", cbDisconnect, NULL);

		// DEBUG
		// DebugLog->addDisplayer( new CStdDisplayer() );
	}
};


//
/// Naming Service
//
NLNET_SERVICE_MAIN (CNamingService, "NS", "naming_service", 50000, CallbackArray, NELNS_CONFIG, NELNS_LOGS)


//
// Commands
//


NLMISC_COMMAND (services_ns, "displays the list of all registered services", "")
{
	if(args.size() != 0) return false;

	log.displayNL ("Display the %d registered services :", RegisteredServices.size());
	for (list<CServiceEntry>::iterator it = RegisteredServices.begin(); it != RegisteredServices.end (); it++)
	{
		log.displayNL ("> %s '%s' %s-%hu '%s'", (*it).SockId->asString().c_str(), CNetManager::getNetBase ("NS")->hostAddress((*it).SockId).asString().c_str(), (*it).Name.c_str(), (uint16)(*it).SId, (*it).Addr.asString().c_str());
	}
	log.displayNL ("End ot the list");

	return true;
}
