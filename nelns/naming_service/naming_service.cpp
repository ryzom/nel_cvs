/** \file naming_service.cpp
 * Naming Service (NS)
 *
 * $Id: naming_service.cpp,v 1.24 2002/07/18 15:02:09 lecroart Exp $
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

#include "nel/misc/types_nl.h"

#include <list>
#include <string>

#include "nel/misc/debug.h"
#include "nel/misc/command.h"
#include "nel/misc/displayer.h"

#include "nel/net/service.h"

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
	CServiceEntry (TSockId sock, CInetAddress a, string n, TServiceId s) : SockId(sock), Addr(a), Name(n), SId (s), WaitingUnregistration(false) { }

	TSockId				SockId;			// the connection between the service and the naming service
	CInetAddress		Addr;			// address to send to the service who wants to lookup this service
	string				Name;			// name of the service
	TServiceId			SId;			// id of the service

	bool				WaitingUnregistration;			// true if this service is in unregistration process (wait other service ACK)
	TTime				WaitingUnregistrationTime;		// time of the beginning of the inregistration process
	list<TServiceId>	WaitingUnregistrationServices;	// list of service that we wait the answer
};


//
// Variables
//

list<CServiceEntry>	RegisteredServices;		/// List of all registred services

uint16				MinBasePort = 51000;	/// Ports begin at 51000
uint16				MaxBasePort = 52000;	/// (note: in this implementation there can be no more than 1000 services)

const TServiceId	BaseSId = 128;			/// Allocated SIds begin at 128 (except for Agent Service)

const TTime			UnregisterTimeout = 10000;	/// After 10s we remove an unregister service if every server didn't ACK the message

//
// Functions
//

void displayRegisteredServices ()
{
	nlinfo ("Display the %d registered services :", RegisteredServices.size());
	for (list<CServiceEntry>::iterator it = RegisteredServices.begin(); it != RegisteredServices.end (); it++)
	{
		TSockId id = (*it).SockId;
		if (id == NULL)
		{
			nlinfo ("> %s '%s' %s-%hu '%s' %s", "<NULL>", "<NULL>", (*it).Name.c_str(), (uint16)(*it).SId, (*it).Addr.asString().c_str(), (*it).WaitingUnregistration?"WaitUnreg":"");
		}
		else
		{
			nlinfo ("> %s '%s' %s-%hu '%s' %s", (*it).SockId->asString().c_str(), CNetManager::getNetBase ("NS")->hostAddress((*it).SockId).asString().c_str(), (*it).Name.c_str(), (uint16)(*it).SId, (*it).Addr.asString().c_str(), (*it).WaitingUnregistration?"WaitUnreg":"");
		}
	}
	nlinfo ("End of the list");
}


list<CServiceEntry>::iterator effectivelyRemove (list<CServiceEntry>::iterator &it)
{
	// remove the service from the registered service list
	nlinfo ("Effectively remove the service %s-%hu", (*it).Name.c_str(), (uint16)(*it).SId);
	return RegisteredServices.erase (it);
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
	nlinfo ("Broadcast the Unregistration of %s-%hu to everybody", (*it).Name.c_str(), (uint16)(*it).SId);

	// new system, after the unregistation broadcast, we wait ACK from all services before really remove
	// the service, before, we tag the service as 'wait before unregister'
	// if everybody didn't answer before the time out, we remove it

	(*it).SockId = NULL;

	(*it).WaitingUnregistration = true;
	(*it).WaitingUnregistrationTime = CTime::getLocalTime();

	// we remove all service awaiting his ACK because this service is down so it'll never ACK
	for (list<CServiceEntry>::iterator itr = RegisteredServices.begin(); itr != RegisteredServices.end (); itr++)
	{
		for (list<TServiceId>::iterator itw = (*itr).WaitingUnregistrationServices.begin(); itw != (*itr).WaitingUnregistrationServices.end ();)
		{
			if ((*itw) == (*it).SId)
			{
				itw = (*itr).WaitingUnregistrationServices.erase (itw);
			}
			else
			{
				itw++;
			}
		}
	}

	string res;
	for (list<CServiceEntry>::iterator it2 = RegisteredServices.begin(); it2 != RegisteredServices.end (); it2++)
	{
		if (!(*it2).WaitingUnregistration)
		{
			(*it).WaitingUnregistrationServices.push_back ((*it2).SId);
			res += toString((*it2).SId) + " ";
		}
	}

	nlinfo ("Before removing the service, we wait the ACK of %s", res.c_str());

	if ((*it).WaitingUnregistrationServices.empty())
	{
		return effectivelyRemove (it);
	}
	else
	{
		return ++it;
	}
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
 * Helper function for cbRegister.
 * If alloc_sid is true, sid is ignored
 * Returns false in case of failure of sid allocation or bad sid provided
 * Note: the reply is included in this function, because it must be done before things such as syncUniTime()
 */
bool doRegister (const string &name, const CInetAddress &addr, TServiceId sid, TSockId from, CCallbackNetBase &netbase, bool reconnection = false)
{
	// find if the service is not already registered
	uint8 ok = true;
	bool needRegister = true;
	/*for (list<CServiceEntry>::iterator it = RegisteredServices.begin(); it != RegisteredServices.end (); it++)
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
	}*/

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
					nlwarning ("Sid %d already used by another service", sid);
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

			if (!reconnection)
			{
				CMessage msgout ("RGB");
				uint8 s = 1;
				msgout.serial (s);
				msgout.serial (const_cast<string &>(name));
				msgout.serial (sid);
				msgout.serial (const_cast<CInetAddress &>(addr));
				CNetManager::send ("NS", msgout, 0);
				nlinfo ("The service is %s-%d, broadcast the Registration to everybody", name.c_str(), sid);
			}
		}
	}

	if (!reconnection)
	{
		// send the answer to the client
		CMessage msgout ("RG");
		msgout.serial (ok);
		if (ok) msgout.serial (sid);
		netbase.send (msgout, from);
		netbase.flush (from);
	}

	from->setAppId (sid);

	//displayRegisteredServices ();

	return ok!=0;
}

void checkWaitingUnregistrationServices ()
{
	for (list<CServiceEntry>::iterator it = RegisteredServices.begin(); it != RegisteredServices.end ();)
	{
		if ((*it).WaitingUnregistration && ((*it).WaitingUnregistrationServices.empty() || CTime::getLocalTime() > (*it).WaitingUnregistrationTime + UnregisterTimeout))
		{
			if ((*it).WaitingUnregistrationServices.empty())
			{
				nlinfo ("Remove the service because all services ACK the remove");
			}
			else
			{
				string res;
				for (list<TServiceId>::iterator it2 = (*it).WaitingUnregistrationServices.begin(); it2 != (*it).WaitingUnregistrationServices.end (); it2++)
				{
					res += toString(*it2) + " ";
				}
				nlwarning ("Remove the service because time out occurs (service numbers %s didn't ACK)", res.c_str());
			}
			it = effectivelyRemove (it);
		}
		else
		{
			it++;
		}
	}
}


/**
 * Callback for service unregistration ACK. Mean that a service was ACK the unregistration broadcast
 */
static void cbACKUnregistration (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	TServiceId sid;
	msgin.serial (sid);

	for (list<CServiceEntry>::iterator it = RegisteredServices.begin(); it != RegisteredServices.end (); it++)
	{
		if ((*it).SId == sid && (*it).WaitingUnregistration)
		{
			for (list<TServiceId>::iterator it2 = (*it).WaitingUnregistrationServices.begin(); it2 != (*it).WaitingUnregistrationServices.end (); it2++)
			{
				if (*it2 == (TServiceId) from->appId())
				{
					// remove the acked service
					(*it).WaitingUnregistrationServices.erase (it2);
					checkWaitingUnregistrationServices ();
					return;
				}
			}
		}
	}
}


/**
 * Callback for service registration when the naming service goes down and up (don't need to broadcast)
 */
static void cbResendRegisteration (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	string name;
	CInetAddress addr;
	TServiceId sid;
	msgin.serial (name);
	msgin.serial (addr);
	msgin.serial (sid);

	doRegister (name, addr, sid, from, netbase, true);
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
	//displayRegisteredServices ();
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

	nlinfo ("The service got port %hu", port);
}


/*
 * Unregisters a service if it has not been done before.
 * Note: this callback is called whenever someone disconnects from the NS.
 * May be there are too many calls if many clients perform many transactional lookups.
 */
static void cbDisconnect (const string &serviceName, TSockId from, void *arg)
{
	doUnregisterService (from);
	//displayRegisteredServices ();
}

/*
 * a service is connected, send him all services infos
 */
static void cbConnect (const string &serviceName, TSockId from, void *arg)
{
	CMessage msgout ("RGB");

	uint8 nb = 0;

	for (list<CServiceEntry>::iterator it2 = RegisteredServices.begin(); it2 != RegisteredServices.end (); it2++)
	{
		// send only services that are available
		if (!(*it2).WaitingUnregistration)
			nb++;
	}

	msgout.serial (nb);

	for (list<CServiceEntry>::iterator it = RegisteredServices.begin(); it != RegisteredServices.end (); it++)
	{
		// send only services that are available
		if (!(*it).WaitingUnregistration)
		{
			msgout.serial ((*it).Name);
			msgout.serial ((*it).SId);
			msgout.serial ((*it).Addr);
		}
	}
	CNetManager::send ("NS", msgout, from);

	nlinfo ("New service connection, sending him all services available");
	//displayRegisteredServices ();

	// set the appid with a bad id (-1)
	from->setAppId (~1);
}


//
// Callback array
//

TCallbackItem CallbackArray[] =
{
	{ "RG", cbRegister },
	{ "RRG", cbResendRegisteration },
	{ "QP", cbQueryPort },
	{ "UNI", cbUnregisterSId },
	{ "ACK_UNI", cbACKUnregistration }
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
		CConfigFile::CVar *var;
		if ((var = ConfigFile.getVarPtr ("BasePort")) != NULL)
		{
			uint16 newBasePort = var->asInt ();
			nlinfo ("Changing the MinBasePort number from %hu to %hu", MinBasePort, newBasePort);
			sint32 delta = MaxBasePort - MinBasePort;
			nlassert (delta > 0);
			MinBasePort = newBasePort;
			MaxBasePort = MinBasePort + uint16 (delta);
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

	bool update ()
	{
		checkWaitingUnregistrationServices ();

		return true;
	}
};


//
/// Naming Service
//
NLNET_OLD_SERVICE_MAIN (CNamingService, "NS", "naming_service", 50000, CallbackArray, NELNS_CONFIG, NELNS_LOGS)


//
// Commands
//


NLMISC_COMMAND (nsServices, "displays the list of all registered services", "")
{
	if(args.size() != 0) return false;

	log.displayNL ("Display the %d registered services :", RegisteredServices.size());
	for (list<CServiceEntry>::iterator it = RegisteredServices.begin(); it != RegisteredServices.end (); it++)
	{
		TSockId id = (*it).SockId;
		if (id == NULL)
		{
			nlinfo ("> %s '%s' %s-%hu '%s' %s", "<NULL>", "<NULL>", (*it).Name.c_str(), (uint16)(*it).SId, (*it).Addr.asString().c_str(), (*it).WaitingUnregistration?"WaitUnreg":"");
		}
		else
		{
			nlinfo ("> %s '%s' %s-%hu '%s' %s", (*it).SockId->asString().c_str(), CNetManager::getNetBase ("NS")->hostAddress((*it).SockId).asString().c_str(), (*it).Name.c_str(), (uint16)(*it).SId, (*it).Addr.asString().c_str(), (*it).WaitingUnregistration?"WaitUnreg":"");
		}
	}
	log.displayNL ("End of the list");

	return true;
}

NLMISC_COMMAND (kill, "kill a service and send an unregister broadcast to other service", "<ServiceShortName>|<ServiceId>")
{
	if(args.size() != 1) return false;

	// try with number

	TServiceId sid = atoi(args[0].c_str());

	if(sid == 0)
	{
		// not a number, try a name
		list<CServiceEntry>::iterator it;
		for (it = RegisteredServices.begin(); it != RegisteredServices.end (); it++)
		{
			if ((*it).Name == args[0])
			{
				sid = (*it).SId;
				break;
			}
		}
		if (it == RegisteredServices.end())
		{
			log.displayNL ("Bad service name or id '%s'", args[0].c_str());
			return false;
		}
	}

	doUnregisterService (sid);
	return true;
}
