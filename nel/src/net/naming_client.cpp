/** \file naming_client.cpp
 * CNamingClient
 *
 * $Id: naming_client.cpp,v 1.55 2002/09/16 14:58:09 lecroart Exp $
 *
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

//
// Includes
//

#include "stdnet.h"

#include "nel/net/naming_client.h"
#include "nel/net/callback_client.h"
#include "nel/net/service.h"


//
// Namespaces
//

using namespace std;
using namespace NLMISC;


namespace NLNET {

//
// Variables
//

CCallbackClient *CNamingClient::_Connection = NULL;
CNamingClient::TRegServices CNamingClient::_RegisteredServices;

static TBroadcastCallback _RegistrationBroadcastCallback = NULL;
static TBroadcastCallback _UnregistrationBroadcastCallback = NULL;

uint	CNamingClient::_ThreadId = 0xFFFFFFFF;

TServiceId CNamingClient::_MySId = 0;


std::list<CNamingClient::CServiceEntry>	CNamingClient::RegisteredServices;
NLMISC::CMutex CNamingClient::RegisteredServicesMutex("CNamingClient::RegisteredServicesMutex");

void CNamingClient::setRegistrationBroadcastCallback (TBroadcastCallback cb)
{
	_RegistrationBroadcastCallback = cb;	
}

void CNamingClient::setUnregistrationBroadcastCallback (TBroadcastCallback cb)
{
	_UnregistrationBroadcastCallback = cb;	
}

//

//

static bool Registered;
static TServiceId RegisteredSuccess;
static TServiceId *RegisteredSID = NULL;

static void cbRegister (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	nlassert(RegisteredSID != NULL);

	msgin.serial (RegisteredSuccess);
	if (RegisteredSuccess)
	{
		msgin.serial (*RegisteredSID);

		// decode the registered services at the register process
		cbRegisterBroadcast (msgin, from, netbase);
	}
	Registered = true;
}

//

static bool QueryPort;
static uint16 QueryPortPort;

static void cbQueryPort (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	msgin.serial (QueryPortPort);
	QueryPort = true;
}

//

//static bool FirstRegisteredBroadcast;

void cbRegisterBroadcast (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	TServiceId size;
	string name;
	TServiceId sid;
	vector<CInetAddress> addr;

	msgin.serial (size);

	for (TServiceId i = 0; i < size; i++)
	{
		msgin.serial (name);
		msgin.serial (sid);
		msgin.serialCont (addr);

		// add it in the list

		std::vector<CInetAddress> addrs;
		CNamingClient::find (sid, addrs);

		if (addrs.size() == 0)
		{
			CNamingClient::RegisteredServicesMutex.enter ();
			CNamingClient::RegisteredServices.push_back (CNamingClient::CServiceEntry (name, sid, addr));
			CNamingClient::RegisteredServicesMutex.leave ();

			nlinfo ("NC: Registration Broadcast of the service %s-%hu '%s'", name.c_str(), (uint16)sid, vectorCInetAddressToString(addr).c_str());

			if (_RegistrationBroadcastCallback != NULL)
				_RegistrationBroadcastCallback (name, sid, addr);
		}
		else if (addrs.size() == 1)
		{
			CNamingClient::RegisteredServicesMutex.enter ();
			for (std::list<CNamingClient::CServiceEntry>::iterator it = CNamingClient::RegisteredServices.begin(); it != CNamingClient::RegisteredServices.end (); it++)
			{
				if (sid == (*it).SId)
				{
					(*it).Name = name;
					(*it).Addr = addr;
					break;
				}
			}
			CNamingClient::RegisteredServicesMutex.leave ();
			nlinfo ("NC: Registration Broadcast (update) of the service %s-%hu '%s'", name.c_str(), (uint16)sid, addr[0].asString().c_str());
		}
		else
		{
			nlstop;
		}
	}

//	FirstRegisteredBroadcast = true;

	//CNamingClient::displayRegisteredServices ();
}
	
//

void cbUnregisterBroadcast (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	string name;
	TServiceId sid;
	vector<CInetAddress> addrs;

	msgin.serial (name);
	msgin.serial (sid);

	// remove it in the list, if the service is not found, ignore it

	CNamingClient::RegisteredServicesMutex.enter ();
	for (std::list<CNamingClient::CServiceEntry>::iterator it = CNamingClient::RegisteredServices.begin(); it != CNamingClient::RegisteredServices.end (); it++)
	{
		if ((*it).SId == sid)
		{
			// check the structure
			nlassertex ((*it).Name == name, ("%s %s",(*it).Name.c_str(), name.c_str()));

			addrs = (*it).Addr;

			CNamingClient::RegisteredServices.erase (it);
			break;
		}
	}
	CNamingClient::RegisteredServicesMutex.leave ();

	nlinfo ("NC: Unregistration Broadcast of the service %s-%hu", name.c_str(), (uint16)sid);

	// send the ACK to the NS

	CMessage msgout (CNamingClient::_Connection->getSIDA(), "ACK_UNI");
	msgout.serial (sid);
	CNamingClient::_Connection->send (msgout);

	// oh my god, it s my sid! but i m alive, why this f*cking naming service want to kill me? ok, i leave it alone!
	if(CNamingClient::_MySId == sid)
	{
		nlwarning ("Naming Service asked me to leave, I leave!");
		IService::getInstance()->exit();
		return;
	}

	if (_UnregistrationBroadcastCallback != NULL)
		_UnregistrationBroadcastCallback (name, sid, addrs);

	//CNamingClient::displayRegisteredServices ();
}

//

static TCallbackItem NamingClientCallbackArray[] =
{
	{ "RG", cbRegister },
	{ "QP", cbQueryPort },

	{ "RGB", cbRegisterBroadcast },
	{ "UNB", cbUnregisterBroadcast },
};

void CNamingClient::connect( const CInetAddress &addr, CCallbackNetBase::TRecordingState rec, const vector<CInetAddress> &addresses )
{
	nlassert (_Connection == NULL || _Connection != NULL && !_Connection->connected ());
	_ThreadId = getThreadId ();

	if (_Connection == NULL)
	{
		_Connection = new CCallbackClient( rec, "naming_client.nmr" );
		_Connection->addCallbackArray (NamingClientCallbackArray, sizeof (NamingClientCallbackArray) / sizeof (NamingClientCallbackArray[0]));
	}

	_Connection->connect (addr);

/*	// send the available addresses
	CMessage msgout (_Connection->getSIDA(), "RS");
	msgout.serialCont (const_cast<vector<CInetAddress>&>(addresses));
	_Connection->send (msgout);
	
	// wait the message that contains all already connected services
	FirstRegisteredBroadcast = false;
	while (!FirstRegisteredBroadcast && _Connection->connected ())
	{
		_Connection->update (-1);
		nlSleep (1);
	}
*/}


void CNamingClient::disconnect ()
{
	checkThreadId ();
	
	if (_Connection != NULL)
	{
		if (_Connection->connected ())
		{
			_Connection->disconnect ();
		}
		delete _Connection;
		_Connection = NULL;
	}

	// we don't call unregisterAllServices because when the naming service will see the disconnection,
	// it'll automatically unregister all services registered by this client.
}

string CNamingClient::info ()
{
	string res;

	if (connected ())
	{
		res = "connected to ";
		res += _Connection->remoteAddress().asString();
	}
	else
	{
		res = "Not connected";
	}

	return res;
}

bool CNamingClient::registerService (const std::string &name, const std::vector<CInetAddress> &addr, TServiceId &sid)
{
	checkThreadId ();
	nlassert (_Connection != NULL && _Connection->connected ());

	CMessage msgout (_Connection->getSIDA(), "RG");
	msgout.serial (const_cast<std::string&>(name));
	msgout.serialCont (const_cast<vector<CInetAddress>&>(addr));
	sid = 0;
	msgout.serial (sid);
	_Connection->send (msgout);

	// wait the answer of the naming service "RG"
	Registered = false;
	RegisteredSID = &sid;
	while (!Registered)
	{
		_Connection->update (-1);
		nlSleep (1);
	}
	if (RegisteredSuccess)
	{
		_MySId = sid;
		_RegisteredServices.insert (make_pair (*RegisteredSID, name));
		nldebug ("NC: Registered service %s-%hu at %s", name.c_str(), (uint16)sid, addr[0].asString().c_str());
	}
	else
	{
		nlerror ("NC: Naming service refused to register service %s at %s", name.c_str(), addr[0].asString().c_str());
	}

	RegisteredSID = NULL;

	return RegisteredSuccess == 1;
}

bool CNamingClient::registerServiceWithSId (const std::string &name, const std::vector<CInetAddress> &addr, TServiceId sid)
{
	checkThreadId ();
	nlassert (_Connection != NULL && _Connection->connected ());

	CMessage msgout (_Connection->getSIDA(), "RG");
	msgout.serial (const_cast<std::string&>(name));
	msgout.serialCont (const_cast<vector<CInetAddress>&>(addr));
	msgout.serial (sid);
	_Connection->send (msgout);

	// wait the answer of the naming service "RGI"
	Registered = false;
	RegisteredSID = &sid;
	while (!Registered)
	{
		_Connection->update (-1);
		nlSleep (1);
	}
	if (RegisteredSuccess)
	{
		_MySId = sid;
		_RegisteredServices.insert (make_pair (*RegisteredSID, name));
		nldebug ("NC: Registered service with sid %s-%hu at %s", name.c_str(), (uint16)RegisteredSID, addr[0].asString().c_str());
	}
	else
	{
		nlerror ("NC: Naming service refused to register service with sid %s at %s", name.c_str(), addr[0].asString().c_str());
	}

	return RegisteredSuccess == 1;
}

void CNamingClient::resendRegisteration (const std::string &name, const std::vector<CInetAddress> &addr, TServiceId sid)
{
	checkThreadId ();
	nlassert (_Connection != NULL && _Connection->connected ());

	CMessage msgout (_Connection->getSIDA(), "RRG");
	msgout.serial (const_cast<std::string&>(name));
	msgout.serialCont (const_cast<vector<CInetAddress>&>(addr));
	msgout.serial (sid);
	_Connection->send (msgout);
}

void CNamingClient::unregisterService (TServiceId sid)
{
	checkThreadId ();
	nlassert (_Connection != NULL && _Connection->connected ());

	CMessage msgout (_Connection->getSIDA(), "UNI");
	msgout.serial (sid);
	_Connection->send (msgout);

	nldebug ("NC: Unregistering service %s-%hu", _RegisteredServices[sid].c_str(), sid);
	_RegisteredServices.erase (sid);
}

void CNamingClient::unregisterAllServices ()
{
	checkThreadId ();
	nlassert (_Connection != NULL && _Connection->connected ());

	while (!_RegisteredServices.empty())
	{
		TRegServices::iterator irs = _RegisteredServices.begin();
		TServiceId sid = (*irs).first;
		unregisterService (sid);
	}
}

uint16 CNamingClient::queryServicePort ()
{
	checkThreadId ();
	nlassert (_Connection != NULL && _Connection->connected ());

	CMessage msgout (_Connection->getSIDA(), "QP");
	_Connection->send (msgout);

	// wait the answer of the naming service "QP"
	QueryPort = false;
	while (!QueryPort)
	{
		_Connection->update (-1);
		nlSleep (1);
	}

	nlinfo ("NC: Received the answer of the query port (%hu)", QueryPortPort);

	return QueryPortPort;
}

bool CNamingClient::lookup (const std::string &name, CInetAddress &addr)
{
	nlassert (_Connection != NULL && _Connection->connected ());

	vector<CInetAddress> addrs;
	find (name, addrs);

	if (addrs.size()==0)
		return false;

	nlassert (addrs.size()==1);
	addr = addrs[0];

	return true;
}

bool CNamingClient::lookup (TServiceId sid, CInetAddress &addr)
{
	nlassert (_Connection != NULL && _Connection->connected ());

	vector<CInetAddress> addrs;
	find (sid, addrs);

	if (addrs.size()==0)
		return false;

	nlassert (addrs.size()==1);
	addr = addrs[0];
	
	return true;
}

/// \todo ace: now the lookupAlternate doesn't say to the naming service that this addr is bad so the NS can't remove it from his list. find a solution
bool CNamingClient::lookupAlternate (const std::string &name, CInetAddress &addr)
{
	nlassert (_Connection != NULL && _Connection->connected ());

	// remove it from his local list
	
	RegisteredServicesMutex.enter ();
	for (std::list<CServiceEntry>::iterator it = RegisteredServices.begin(); it != RegisteredServices.end (); it++)
	{
		if ((*it).Addr[0] == addr)
		{
			RegisteredServices.erase (it);
			break;
		}
	}
	RegisteredServicesMutex.leave ();

	vector<CInetAddress> addrs;
	find (name, addrs);

	if (addrs.size()==0)
		return false;

	nlassert (addrs.size()==1);
	addr = addrs[0];

	return true;
}

void CNamingClient::lookupAll (const std::string &name, std::vector<CInetAddress> &addrs)
{
	nlassert (_Connection != NULL && _Connection->connected ());

	find (name, addrs);
}

bool CNamingClient::lookupAndConnect (const std::string &name, CCallbackClient &sock)
{
	nlassert (_Connection != NULL && _Connection->connected ());

	// look up for service
	CInetAddress servaddr;
	
	// if service not found, return false
	if (!CNamingClient::lookup (name, servaddr))
		return false;

	do
	{
		try
		{
			// try to connect to the server
			sock.connect (servaddr);

			// connection succeeded
			return true;
		}
		catch (ESocketConnectionFailed &e)
		{
			nldebug( "Connection to %s failed: %s, tring another service if available", servaddr.asString().c_str(), e.what() );

			// try another server and if service is not found, return false
			if (!CNamingClient::lookupAlternate (name, servaddr))
				return false;
		}
	}
	while (true);
}



void CNamingClient::update ()
{
	checkThreadId ();
	// get message for naming service (new registration for example)
	if (_Connection != NULL && _Connection->connected ())
		_Connection->update ();
}

void CNamingClient::checkThreadId ()
{
	if (getThreadId () != _ThreadId)
	{
		nlerror ("You try to access to the CNamingClient with 2 differents thread (%d and %d)", _ThreadId, getThreadId());
	}
}


//
// Commands
//

NLMISC_COMMAND(services, "displays registered services", "")
{
	if(args.size() != 0) return false;

	CNamingClient::displayRegisteredServices (&log);

	return true;
}

} // NLNET
