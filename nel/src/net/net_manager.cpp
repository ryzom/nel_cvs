/** \file net_manager.cpp
 * Network engine, layer 3, base
 *
 * $Id: net_manager.cpp,v 1.4 2001/05/16 16:21:04 lecroart Exp $
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

#include <string>
#include <map>
#include <vector>
#include <algorithm>

#include "nel/misc/string_id_array.h"
#include "nel/misc/time_nl.h"

#include "nel/net/naming_client.h"

#include "nel/net/callback_client.h"
#include "nel/net/callback_server.h"

#include "nel/net/naming_client.h"

#include "nel/net/net_manager.h"

using namespace std;
using namespace NLMISC;

namespace NLNET {

CNetManager::TBaseMap	CNetManager::_BaseMap;

static void nmNewConnection (TSockId from, void *arg)
{
	nlassert (arg != NULL);
	CBaseStruct *basest = (CBaseStruct *)arg;

	nldebug("L4: nmNewConnection() from service '%s'", basest->Name.c_str ());

	// call the client callback if necessary
	if (basest->ConnectionCallback != NULL)
		basest->ConnectionCallback (basest->Name, from, basest->ConnectionCbArg);
}

static void nmNewDisconnection (TSockId from, void *arg)
{
	nlassert (arg != NULL);
	CBaseStruct *basest = (CBaseStruct *)arg;

	nldebug("L4: nmNewDisconnection() from service '%s'", basest->Name.c_str ());

	// call the client callback if necessary
	if (basest->DisconnectionCallback != NULL)
		basest->DisconnectionCallback (basest->Name, from, basest->DisconnectionCbArg);

	// todo kan c est un group qui se deco, il faut manager le bordel

	// on a client, we have to clear the associations
	if (basest->Type != CBaseStruct::Server)
	{
		nlassert (basest->NetBase.size() == 1);
		basest->NetBase[0]->getSIDA ().clear ();
	}
}


// find a not connected callbackclient or create a new one and connect to the Addr
void createConnection(CBaseStruct &Base, const CInetAddress &Addr)
{
	uint i;
	for (i = 0; i < Base.NetBase.size (); i++)
	{
		if (!Base.NetBase[i]->connected ())
		{
			break;
		}
	}
	if (i == Base.NetBase.size ())
	{
		CCallbackClient *cc = new CCallbackClient;
		Base.NetBase.push_back (cc);
	}
	
	CCallbackClient *cc = dynamic_cast<CCallbackClient *>(Base.NetBase[i]);

	cc->CCallbackNetBase::setDisconnectionCallback (nmNewDisconnection, (void*) &Base);

	try
	{
		cc->connect (Addr);

		if (Base.ConnectionCallback != NULL)
			Base.ConnectionCallback (Base.Name, cc->getSockId(0), Base.ConnectionCbArg);
	}
	catch (ESocketConnectionFailed &e)
	{
		nlinfo ("L4: can't connect now (%s)", e.what ());
	}
}


void RegistrationBroadcast (const std::string &name, TServiceId sid, const CInetAddress &addr)
{
	nldebug("L4: RegistrationBroadcast() of service %s-%hu", name.c_str (), (uint16)sid);

	// find if this new service is interesting
	for (CNetManager::ItBaseMap itbm = CNetManager::_BaseMap.begin (); itbm != CNetManager::_BaseMap.end (); itbm++)
	{
		if ((*itbm).second.Type == CBaseStruct::Client && !(*itbm).second.NetBase[0]->connected())
		{
			if (name == (*itbm).first)
			{
				// ok! it's cool, the service is here, go and connect to him!
				createConnection ((*itbm).second, addr);
			}
		}
		else if ((*itbm).second.Type == CBaseStruct::Group)
		{
			// ok, it's a group, try to see if it wants this!
			for (uint i = 0; i < (*itbm).second.ServiceNames.size (); i++)
			{
				if ((*itbm).second.ServiceNames[i] == name)
				{
					createConnection ((*itbm).second, addr);
					break;
				}
			}
		}
	}

}

static void UnregistrationBroadcast (const std::string &name, TServiceId sid, const CInetAddress &addr)
{
	nldebug("L4: UnregistrationBroadcast() of service %s-%hu", name.c_str (), (uint16)sid);
}

void CNetManager::init (const CInetAddress *addr)
{
	if (addr == NULL) return;

	// connect to the naming service (may generate a ESocketConnectionFailed exception)
	CNamingClient::connect (*addr);

	// connect the callback to know when a new service comes in or goes down
	CNamingClient::setRegistrationBroadcastCallback (RegistrationBroadcast);
	CNamingClient::setUnregistrationBroadcastCallback (UnregistrationBroadcast);
}

void CNetManager::release ()
{
	if (CNamingClient::connected ())
		CNamingClient::disconnect ();

	_BaseMap.clear ();
}

void CNetManager::addServer (const std::string &serviceName, uint16 servicePort)
{
	TServiceId sid = 0;
	addServer (serviceName, servicePort, sid);
}

void CNetManager::addServer (const std::string &serviceName, uint16 servicePort, TServiceId &sid)
{
	nldebug ("L4: Adding server '%s' in CNetManager", serviceName.c_str ());
	ItBaseMap itbm = find (serviceName);

	// check if it's a new server
	nlassert ((*itbm).second.NetBase.empty());
	
	CCallbackServer *cs = new CCallbackServer;
	(*itbm).second.NetBase.push_back (cs);

	(*itbm).second.Type = CBaseStruct::Server;

	// install the server
	
	cs->setConnectionCallback (nmNewConnection, (void*) &((*itbm).second));
	cs->CCallbackNetBase::setDisconnectionCallback (nmNewDisconnection, (void*) &((*itbm).second));

	if (servicePort == 0)
	{
		nlassert (CNamingClient::connected ());
		servicePort = CNamingClient::queryServicePort ();
	}

	cs->init (servicePort);

	// register the server to the naming service if we are connected to Naming Service

	if (CNamingClient::connected ())
	{
		CInetAddress addr = CInetAddress::localHost ();
		addr.setPort (servicePort);

		if (sid == 0)
		{
			sid = CNamingClient::registerService (serviceName, addr);
		}
		else
		{
			CNamingClient::registerServiceWithSId (serviceName, addr, sid);
		}
	}
}


void CNetManager::addClient (const std::string &serviceName, const std::string &addr, bool autoRetry)
{
	nldebug ("L4: Adding client '%s' with addr '%s' in CNetManager", serviceName.c_str (), addr.c_str());
	ItBaseMap itbm = find (serviceName);
	
	// it's a new client, add the connection
	(*itbm).second.Type = CBaseStruct::ClientWithAddr;
	(*itbm).second.AutoRetry = autoRetry;

	if ((*itbm).second.ServiceNames.empty())
	{
		(*itbm).second.ServiceNames.push_back(addr);
	}
	else
	{
		(*itbm).second.ServiceNames[0] = addr;
	}

	nlassert ((*itbm).second.NetBase.size() < 2);

	createConnection ((*itbm).second, addr);
}


void CNetManager::addClient (const std::string &serviceName)
{
	nlassert (CNamingClient::connected ());
	nldebug ("L4: Adding client '%s' in CNetManager", serviceName.c_str ());
	ItBaseMap itbm = find (serviceName);
	
	// check if it's a new client
	nlassert ((*itbm).second.NetBase.empty());

	CCallbackClient *cc = new CCallbackClient;
	(*itbm).second.NetBase.push_back (cc);

	(*itbm).second.Type = CBaseStruct::Client;

	cc->CCallbackNetBase::setDisconnectionCallback (nmNewDisconnection, (void*) &((*itbm).second));

	// find the service in the naming_service and connect if exists
	if (CNamingClient::lookupAndConnect (serviceName, *cc))
	{
		// call the user that we are connected
		if ((*itbm).second.ConnectionCallback != NULL)
			(*itbm).second.ConnectionCallback (serviceName, cc->getSockId(0), (*itbm).second.ConnectionCbArg);
	}
}



void CNetManager::addGroup (const std::string &groupName, const std::string &serviceName)
{
	nlassert (CNamingClient::connected ());
	nldebug ("L4: Adding '%s' to group '%s' in CNetManager", serviceName.c_str (), groupName.c_str());
	ItBaseMap itbm = find (groupName);

	(*itbm).second.Type = CBaseStruct::Group;

	// check if you don't already add this service in this group
	vector<string>::iterator it = std::find ((*itbm).second.ServiceNames.begin(), (*itbm).second.ServiceNames.end(), serviceName);
	nlassert (it == (*itbm).second.ServiceNames.end());

	(*itbm).second.ServiceNames.push_back(serviceName);


	// find the service in the naming_service and connect if exists
	vector<CInetAddress> addrs;
	CNamingClient::lookupAll (serviceName, addrs);

	// connect to all these services
	for (uint i = 0; i < addrs.size (); i++)
	{
		createConnection ((*itbm).second, addrs[i]);
	}
}



NLMISC::CStringIdArray &CNetManager::getSIDA (const std::string &serviceName)
{
	nldebug ("L4: getSIDA() for service '%s'", serviceName.c_str ());
	ItBaseMap itbm = find (serviceName);

	// in case of group, we can return association only if there s only one service on it
	nlassert ((*itbm).second.NetBase.size() == 1);

	return (*itbm).second.NetBase[0]->getSIDA ();
}

void CNetManager::addCallbackArray (const std::string &serviceName, const TCallbackItem *callbackarray, NLMISC::CStringIdArray::TStringId arraysize)
{
	nldebug ("L4: addingCallabckArray() for service '%s'", serviceName.c_str ());
	ItBaseMap itbm = find (serviceName);
	for (uint32 i = 0; i < (*itbm).second.NetBase.size(); i++)
	{
//		if ((*itbm).second.NetBase[i]->connected())
		(*itbm).second.NetBase[i]->addCallbackArray (callbackarray, arraysize);
	}
}


void CNetManager::update ()
{
//	nldebug ("L4: update()");
	for (ItBaseMap itbm = _BaseMap.begin (); itbm != _BaseMap.end (); itbm++)
	{
		for (uint32 i = 0; i < (*itbm).second.NetBase.size(); i++)
		{
			// todo remettre comme avant mais il faut que olivier est fini sa modif car 
			// si on update pas meme kan c est pas connected on est pas au courant de la deconnection
			(*itbm).second.NetBase[i]->update ();
			if ((*itbm).second.NetBase[i]->connected())
			{
				// if connected, update
//				(*itbm).second.NetBase[i]->update ();
			}
			else
			{
				static TTime lasttime = CTime::getLocalTime();
				if (CTime::getLocalTime() > lasttime + 5000)
				{
					lasttime = CTime::getLocalTime();

					// if not connected, try to connect ClientWithAddr
					if ((*itbm).second.Type == CBaseStruct::ClientWithAddr && (*itbm).second.AutoRetry)
					{
						CCallbackClient *cc = dynamic_cast<CCallbackClient *>((*itbm).second.NetBase[i]);
						try
						{
							nlassert ((*itbm).second.ServiceNames.size()==1);
							cc->connect (CInetAddress((*itbm).second.ServiceNames[0]));

							if ((*itbm).second.ConnectionCallback != NULL)
								(*itbm).second.ConnectionCallback ((*itbm).second.Name, cc->getSockId(0), (*itbm).second.ConnectionCbArg);
						}
						catch (ESocketConnectionFailed &e)
						{
							// can't connect now, try later
							nlinfo("L4: can't connect now (%s)", e.what());
						}
					}
				}
			}	
		}
	}

	CNamingClient::update ();
}


void CNetManager::send (const std::string &serviceName, const CMessage &buffer, TSockId hostid)
{
	nldebug ("L4: send for service '%s' message %s to %s", serviceName.c_str(), buffer.toString().c_str(), hostid->asString().c_str());
	ItBaseMap itbm = find (serviceName);
	for (uint32 i = 0; i < (*itbm).second.NetBase.size(); i++)
	{
		if ((*itbm).second.NetBase[i]->connected())
			(*itbm).second.NetBase[i]->send (buffer, hostid);
	}
}

CCallbackNetBase *CNetManager::getNetBase (const std::string &serviceName)
{
	ItBaseMap itbm = find (serviceName);
	return (*itbm).second.NetBase[0];
}

void CNetManager::setConnectionCallback (const std::string &serviceName, TNetManagerCallback cb, void *arg)
{
	nldebug ("L4: setConnectionCallback() for service '%s'", serviceName.c_str ());
	ItBaseMap itbm = find (serviceName);
	(*itbm).second.ConnectionCallback = cb;
	(*itbm).second.ConnectionCbArg = arg;
}

void CNetManager::setDisconnectionCallback (const std::string &serviceName, TNetManagerCallback cb, void *arg)
{
	nldebug ("L4: setDisconnectionCallback() for service '%s'", serviceName.c_str ());
	ItBaseMap itbm = find (serviceName);
	(*itbm).second.DisconnectionCallback = cb;
	(*itbm).second.DisconnectionCbArg = arg;
}


CNetManager::ItBaseMap CNetManager::find (const std::string &serviceName)
{
	// find the service or add it if not found
	pair<ItBaseMap, bool> p;
	p = _BaseMap.insert (make_pair (serviceName, CBaseStruct (serviceName)));
	return p.first;
}



} // NLNET
