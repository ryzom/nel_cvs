/** \file admin_service.cpp
 * Admin Service (AS)
 *
 * $Id: admin_service.cpp,v 1.15 2002/03/25 09:28:00 lecroart Exp $
 *
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

#include "nel/misc/types_nl.h"

#include <string>
#include <list>

#include "nel/misc/debug.h"
#include "nel/misc/config_file.h"
#include "nel/misc/command.h"

#include "nel/net/service.h"
#include "nel/net/net_manager.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

////////////////////////


struct CService
{
	CService () : Id(0xFFFFFFFF), Ready(false), Connected(false), InConfig(false) { }

	uint32			Id;				/// uint32 to identify the service
	string			AliasName;		/// alias of the service used in the AES and AS to find him (unique per AES)
	string			ShortName;		/// name of the service in short format ("NS" for example)
	string			LongName;		/// name of the service in long format ("naming_service")
	bool			Ready;			/// true if the service is ready
	bool			Connected;		/// true if the service is connected to the AES
	bool			InConfig;		/// true if the service is in the configuration
	std::vector<NLMISC::CSerialCommand>	Commands;

	void setValues (const CService &t)
	{
		// copy all except gtk stuffs
		Id = t.Id;
		AliasName = t.AliasName;
		ShortName = t.ShortName;
		LongName = t.LongName;
		Ready = t.Ready;
		Connected = t.Connected;
		//InConfig = t.InConfig; never change the inconfig value
	}
};

typedef list<CService> TServices;
typedef list<CService>::iterator SIT;

struct CAdminExecutorService
{
	CAdminExecutorService () : Id(NextId++), SockId(NULL), Connected(false) { }

	TSockId	SockId;			/// connection to the AES
	uint32	Id;				/// uint32 to identify the AES where the service is running

	string	ServerAlias;	/// name of the layer4 connection, used to send message to this AES
	string	ServerAddr;		/// address in a string format (only the ip)
	bool	Connected;		/// true if the AES is connected

	TServices Services;

	vector<string>	ServiceAliasList;

	SIT findService (uint32 sid, bool asrt = true)
	{
		SIT sit;
		for (sit = Services.begin(); sit != Services.end(); sit++)
			if ((*sit).Id == sid)
				break;

		if (asrt)
			nlassert (sit != Services.end());
		return sit;
	}

	SIT findService (const string &alias, bool asrt = true)
	{
		SIT sit;
		for (sit = Services.begin(); sit != Services.end(); sit++)
			if ((*sit).AliasName == alias)
				break;

		if (asrt)
			nlassert (sit != Services.end());
		return sit;
	}

private:
	static uint32 NextId;
};

uint32 CAdminExecutorService::NextId = 1;

typedef list<CAdminExecutorService> TAdminExecutorServices;
typedef list<CAdminExecutorService>::iterator AESIT;

TAdminExecutorServices AdminExecutorServices;

/////////////////

AESIT findAdminExecutorService (uint32 aesid, bool asrt = true)
{
	AESIT aesit;
	for (aesit = AdminExecutorServices.begin(); aesit != AdminExecutorServices.end(); aesit++)
		if ((*aesit).Id == aesid)
			break;

	if (asrt)
		nlassert (aesit != AdminExecutorServices.end());
	return aesit;
}

AESIT findAdminExecutorService (string ServerAlias, bool asrt = true)
{
	AESIT aesit;
	for (aesit = AdminExecutorServices.begin(); aesit != AdminExecutorServices.end(); aesit++)
		if ((*aesit).ServerAlias == ServerAlias)
			break;

	if (asrt)
		nlassert (aesit != AdminExecutorServices.end());
	return aesit;
}

void displayServices ()
{
	for (AESIT aesit = AdminExecutorServices.begin(); aesit != AdminExecutorServices.end(); aesit++)
	{
		nlinfo ("> Admin");
		for (SIT sit = (*aesit).Services.begin(); sit != (*aesit).Services.end(); sit++)
		{
			nlinfo ("  > '%s' '%s' '%s' '%s' %d %d", (*aesit).SockId->asString().c_str(), (*sit).AliasName.c_str(), (*sit).ShortName.c_str(), (*sit).LongName.c_str(), (*aesit).Id, (*sit).Id);
		}
	}
}


// send a message to a client. if ok is 0 it s an error or it s a normal 
void messageToClient (uint8 ok, string msg, TSockId from = NULL)
{
	CMessage msgout (CNetManager::getSIDA ("AS"), "MESSAGE");
	msgout.serial (ok, msg);
	CNetManager::send ("AS", msgout, from);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////// SCRIPT MANAGER /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool StartAllServices = false;
uint32 StartAllServicesPos;

void doNextStartAllServicesStep ()
{
	nlassert (StartAllServices);

	// get the script
	try
	{
		CConfigFile::CVar &script = IService::getInstance()->ConfigFile.getVar("Services");

		// check the position

		if (StartAllServicesPos*2 >= (uint32)script.size())
		{
			StartAllServices = false;

			nlinfo("end of the script");
			messageToClient (1, "Start All Service finnished correctly");
			return;
		}

		// get the script line

		string serverAlias;
		string serviceAlias;

		try
		{
			serverAlias = script.asString (StartAllServicesPos*2);
			serviceAlias = script.asString (StartAllServicesPos*2+1);
		}
		catch(EBadSize &)
		{
			messageToClient (1, "'Services' variable does not contains a good number of entries (must be a multiple of 2)");
			nlwarning ("'Services' variable does not contains a good number of entries (must be a multiple of 2)");
			StartAllServices = false;
			return;
		}

		AESIT aesit = findAdminExecutorService (serverAlias, false);
		if (aesit == AdminExecutorServices.end())
		{
			messageToClient (1, "don't find the server");
			nlwarning("don't find the server");
			StartAllServices = false;
			return;
		}

		// check if the service is not currently running

		StartAllServicesPos++;

		SIT sit = (*aesit).findService (serviceAlias);
		if ((*sit).Connected)
		{
			// the service is already running, go to the next process
			doNextStartAllServicesStep ();
		}
		else
		{
			// send the resquest to the AES

			CMessage msgout (CNetManager::getSIDA((*aesit).ServerAlias), "STARTS");
			msgout.serial (serviceAlias);
			CNetManager::send ((*aesit).ServerAlias, msgout);
		}
	}
	catch(EUnknownVar&)
	{
		messageToClient (1, "'Services' variable is not found");
		nlwarning ("'Services' variable is not found");
		StartAllServices = false;
		return;
	}
}

void initStartAllServices ()
{
	if (StartAllServices)
	{
		messageToClient (1, "already running a script, reset it");
		nlwarning("already running a script, reset it");
		StartAllServices = false;
	}

	try
	{
		CConfigFile::CVar &script = IService::getInstance()->ConfigFile.getVar("Services");

		for (sint i = 0 ; i < script.size (); i+=2)
		{
			string serverAlias = script.asString(i);
			AESIT aesit = findAdminExecutorService (serverAlias, false);
			if (aesit == AdminExecutorServices.end())
			{
				messageToClient (1, "an aes is not running, can't run the script");
				nlwarning("aes '%s' isn't running, can't run the script", serverAlias.c_str());
				return;
			}
		}
	}
	catch(EConfigFile &)
	{
		messageToClient (1, "bad config file");
		nlwarning ("bad config file");
		return;
	}

	StartAllServicesPos = 0;
	StartAllServices = true;

	doNextStartAllServicesStep();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////// CONNECTION TO THE AES ///////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


static void cbExecuteSystemCommandResult (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	vector<string> result;
	msgin.serialCont (result);

	nlinfo("command result");
	for (uint i = 0; i < result.size(); i++)
	{
		printf ("%s", result[i].c_str());
	}
	nlinfo("end of command result");
}

// get the service list from the admin exec and send the list to all admin client
static void cbServiceList (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	CAdminExecutorService *aes = (CAdminExecutorService*) (uint) from->appId();

	//
	// Get the list of service from aes
	//

	uint32 nbs;
	msgin.serial (nbs);

	for (uint32 i = 0; i < nbs; i++)
	{
		// find the service
		CService s;

		msgin.serial (s.Id, s.AliasName, s.ShortName, s.LongName, s.Ready);
		msgin.serialCont (s.Commands);
		s.Connected = true;

		if (!s.AliasName.empty())
		{
			SIT sit = aes->findService (s.AliasName, false);
			if (sit == aes->Services.end ())
			{
				aes->Services.push_back (s);
			}
			else
			{
				nlassert ("the service already exists with alias, update it");
				(*sit).setValues (s);
			}
		}
		else
		{
			SIT sit = aes->findService (s.Id, false);
			if (sit == aes->Services.end ())
			{
				aes->Services.push_back (s);
			}
			else
			{
				nlassert ("the service already exists with id, update it");
				(*sit).setValues (s);
			}
		}
	}
	
	displayServices ();

	//
	// Send the new list to all admin
	//

	CMessage msgout (CNetManager::getSIDA ("AS"), "SERVICE_LIST");
	uint32 nbaes = 1;
	msgout.serial (nbaes);
	msgout.serial (aes->Id);
	uint32 ss = aes->Services.size();
	msgout.serial (ss);

	for (SIT sit = aes->Services.begin(); sit != aes->Services.end(); sit++)
	{
		msgout.serial ((*sit).Id, (*sit).AliasName, (*sit).ShortName, (*sit).LongName);
		msgout.serial ((*sit).Ready, (*sit).Connected, (*sit).InConfig);
		msgout.serialCont ((*sit).Commands);
	}
	CNetManager::send ("AS", msgout, 0);
}

static void cbServiceAliasList (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	// get the service list from the admin exec and send the list to all admin client
	CAdminExecutorService *aes = (CAdminExecutorService*) (uint) from->appId();

	aes->ServiceAliasList.clear ();
	msgin.serialCont (aes->ServiceAliasList);

	nlinfo("send SAL to admin callback from aes");

	CMessage msgout (CNetManager::getSIDA ("AS"), "SAL");
	msgout.serial (aes->Id);
	msgout.serialCont (aes->ServiceAliasList);
	CNetManager::send ("AS", msgout, 0);
	
	nlinfo("new service alias list");
}


static void cbServiceIdentification (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	CAdminExecutorService *aes = (CAdminExecutorService*) (uint) from->appId();

	uint32 sid;
	string alias;

	msgin.serial (sid, alias);

	SIT sit;
	if (!alias.empty())
	{
		sit = aes->findService (alias, false);

		if (sit == aes->Services.end ())
		{
			// the alias is not found
			nlwarning ("new service with alias (%s) but not in my list", alias.c_str());

			aes->Services.push_back (CService ());
			sit = aes->Services.end();
			sit--;
		}
		else
		{
			// normal case
		}
	}
	else
	{
		sit = aes->findService (sid, false);

		if (sit == aes->Services.end ())
		{
			// normal case for unknown services
			nlwarning ("new service without alias and not in my list, add it");
		}
		else
		{
			nlwarning ("new service without alias is already in my list with id %d, add it", sid);
		}
		aes->Services.push_back (CService ());
		sit = aes->Services.end();
		sit--;
	}

	(*sit).Id = sid;
	(*sit).AliasName = alias;
	(*sit).Connected = true;
	msgin.serial ((*sit).ShortName, (*sit).LongName);
	msgin.serialCont ((*sit).Commands);

	nlinfo ("*:%d:%d is identified to be '%s' '%s' '%s'", aes->Id, sid, (*sit).AliasName.c_str(), (*sit).ShortName.c_str(), (*sit).LongName.c_str());

	// broadcast the message to all admin client
	CMessage msgout (CNetManager::getSIDA ("AS"), "SID");
	msgout.serial (aes->Id, sid, (*sit).AliasName, (*sit).ShortName, (*sit).LongName);
	msgout.serialCont ((*sit).Commands);

	CNetManager::send ("AS", msgout, 0);
}

static void cbServiceReady (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	CAdminExecutorService *aes = (CAdminExecutorService*) (uint) from->appId();

	uint32 sid;
	msgin.serial (sid);

	SIT sit = aes->findService(sid);
	(*sit).Ready = true;

	nlinfo ("*:%d:%d is ready", aes->Id, sid);

	// broadcast the message to all admin client
	CMessage msgout (CNetManager::getSIDA ("AS"), "SR");
	msgout.serial (aes->Id, sid);
	CNetManager::send ("AS", msgout, 0);

	// if we are in a script execution, continue
	if (StartAllServices)
		doNextStartAllServicesStep();
}

static void cbServiceConnection (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	CAdminExecutorService *aes = (CAdminExecutorService*) (uint) from->appId();

	uint32 sid;
	msgin.serial (sid);

	nlinfo ("*:%d:%d connected", aes->Id, sid);

	// don't do anything. we have to wait identification to add it in out lists

/*
	aes->Services.push_back (CService(sid));
*/
	// broadcast the message to all admin client
	CMessage msgout (CNetManager::getSIDA ("AS"), "SC");
	msgout.serial (aes->Id, sid);
	CNetManager::send ("AS", msgout, 0);
}

static void cbServiceDisconnection (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	CAdminExecutorService *aes = (CAdminExecutorService*) (uint) from->appId();

	uint32 sid;
	msgin.serial (sid);

	nlinfo ("*:%d:%d disconnected", aes->Id, sid);

	SIT sit = aes->findService(sid, false);
	
	// broadcast the message to all admin client
	CMessage msgout (CNetManager::getSIDA ("AS"), "SD");
	msgout.serial (aes->Id, sid);
	CNetManager::send ("AS", msgout, 0);

	// the service could disconnect before it's identification, in this case, we don't have it in the service list
	if (sit != aes->Services.end ())
	{
		if ((*sit).InConfig)
		{
			(*sit).Ready = (*sit).Connected = false;
			(*sit).Id = 0xFFFFFFFF;
			(*sit).ShortName = (*sit).LongName = "";
			(*sit).Commands.clear ();
		}
		else
		{
			// erase only if it's not a service in the config
			aes->Services.erase (sit);
		}
	}

	displayServices ();
}

// i'm connected to a new admin executor service
void cbAESConnection (const string &serviceName, TSockId from, void *arg)
{
	AESIT aesit = findAdminExecutorService (serviceName);	
	CAdminExecutorService *aes = &(*aesit);
	
	// set the appid to find the aes in O(1)
	from->setAppId ((uint64)(uint)aes);

	aes->Connected = true;
	nlinfo ("*:%d:* connected", aes->Id);
/*
	// broadcast the message that an admin exec is connected to all admin client
	CMessage msgout (CNetManager::getSIDA ("AS"), "AESC");
	msgout.serial (aes->Id);
	CNetManager::send ("AS", msgout, 0);
*/

	// broadcast the new state of this AES
	CMessage msgout (CNetManager::getSIDA ("AS"), "AES_LIST");
	uint32 nbaes = 1;
	msgout.serial (nbaes);
	msgout.serial (aes->Id, aes->ServerAlias, aes->ServerAddr, aes->Connected);
	CNetManager::send ("AS", msgout, 0);
}

// i'm disconnected to an admin executor service
void cbAESDisconnection (const string &serviceName, TSockId from, void *arg)
{
	// get the aes with the appid
	CAdminExecutorService *aes = (CAdminExecutorService*) (uint) from->appId();

	aes->Connected = false;

	SIT sit;
	for (sit = aes->Services.begin(); sit != aes->Services.end();)
	{
		// keep only inconfig services
		if ((*sit).InConfig)
		{
			(*sit).Id = 0xFFFFFFFF;
			(*sit).ShortName = (*sit).LongName = "";
			(*sit).Ready = (*sit).Connected = false;
			sit++;
		}
		else
		{
			// erase only if it's not a service in the config
			sit = aes->Services.erase (sit);
		}
	}

	nlinfo ("*:%d:* disconnected", aes->Id);
/*	
	// broadcast the message to all admin client that an admin exec is disconnected
	CMessage msgout (CNetManager::getSIDA ("AS"), "AESD");
	msgout.serial (aes->Id);
	CNetManager::send ("AS", msgout, 0);
*/

	displayServices ();

	// broadcast the new state of this AES
	CMessage msgout (CNetManager::getSIDA ("AS"), "AES_LIST");
	uint32 nbaes = 1;
	msgout.serial (nbaes);
	msgout.serial (aes->Id, aes->ServerAlias, aes->ServerAddr, aes->Connected);
	CNetManager::send ("AS", msgout, 0);
}

static void cbLog (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	// received an answer for a command, give it to all admin client

	// get the aes with the appid
	CAdminExecutorService *aes = (CAdminExecutorService*) (uint) from->appId();

	// broadcast the message to the admin service
	CMessage msgout (CNetManager::getSIDA ("AS"), "XLOG");
	string log;
	uint32 sid;
	msgin.serial (sid);
	msgin.serial (log);

	msgout.serial (aes->Id);
	msgout.serial (sid);
	msgout.serial (log);
	CNetManager::send ("AS", msgout, 0);
}


TCallbackItem AESCallbackArray[] =
{
	{ "ESCR", cbExecuteSystemCommandResult },

	{ "SL", cbServiceList },
	{ "SID", cbServiceIdentification },
	{ "SR", cbServiceReady },
	{ "SC", cbServiceConnection },
	{ "SD", cbServiceDisconnection },

	{ "SAL", cbServiceAliasList },

	{ "XLOG", cbLog },
};


////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////// CONNECTION TO THE CLIENT ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

//
// A new admin client is connected.
//
void clientConnection (const string &serviceName, TSockId from, void *arg)
{
	// new client, send him all out info about services

	nlinfo ("admin %s is connected", from->asString().c_str());

	CNetManager::getNetBase(serviceName)->authorizeOnly ("AUTH", from);
}

static void cbAuthenticateClient (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	//
	// Check the validity of the admin
	//
	
	string password;
	bool ok = false;
	try
	{
		msgin.serial (password);

		if (password == IService::getInstance()->ConfigFile.getVar("Password").asString())
		{
			// good authentification
			ok = true;
		}
	}
	catch (Exception &)
	{
		// problem, eject him
	}

	if (ok)
	{
		netbase.authorizeOnly (NULL, from);
		nlinfo ("Admin authentification success");
	}
	else
	{
		// bad auth => disconnect
		netbase.disconnect (from);
		nlwarning ("Bad admin authentification, tried password: '%s'", password.c_str());
		return;
	}

	//
	// send the list of all the aes
	//

	CMessage msgout2 (CNetManager::getSIDA ("AS"), "AES_LIST");
	AESIT aesit;
	uint32 nbaes = (uint32)AdminExecutorServices.size();
	msgout2.serial (nbaes);
	for (aesit = AdminExecutorServices.begin(); aesit != AdminExecutorServices.end(); aesit++)
	{
		// send info about the AES

		msgout2.serial ((*aesit).Id, (*aesit).ServerAlias, (*aesit).ServerAddr, (*aesit).Connected);
	}
	CNetManager::send ("AS", msgout2, from);

	//
	// send the list of all services
	//

	CMessage msgout (CNetManager::getSIDA ("AS"), "SERVICE_LIST");
	nbaes = (uint32)AdminExecutorServices.size();
	msgout.serial (nbaes);
	for (aesit = AdminExecutorServices.begin(); aesit != AdminExecutorServices.end(); aesit++)
	{
		msgout.serial ((*aesit).Id);

		uint32 nbs = (uint32)(*aesit).Services.size();
		msgout.serial (nbs);

		for (SIT sit = (*aesit).Services.begin(); sit != (*aesit).Services.end(); sit++)
		{
			// send info about services of the AES
			
			msgout.serial ((*sit).Id, (*sit).AliasName, (*sit).ShortName, (*sit).LongName);
			msgout.serial ((*sit).Ready, (*sit).Connected, (*sit).InConfig);
			msgout.serialCont ((*sit).Commands);
		}
	}
	CNetManager::send ("AS", msgout, from);

	displayServices ();

	//
	// send service alias list
	//

	nlinfo("send SAL to admin startup");

	for (aesit = AdminExecutorServices.begin(); aesit != AdminExecutorServices.end(); aesit++)
	{
		CMessage msgout2 (CNetManager::getSIDA ("AS"), "SAL");
		msgout2.serial ((*aesit).Id);
		msgout2.serialCont ((*aesit).ServiceAliasList);
		CNetManager::send ("AS", msgout2, from);
	}
}


static void cbExecuteSystemCommand (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	string command;
	uint32 aesid;
	msgin.serial (aesid);
	msgin.serial (command);

	AESIT aesit = findAdminExecutorService (aesid, false);
	if (aesit == AdminExecutorServices.end())
	{
		// don't find the aes, send an error message
		messageToClient (0, "couldn't execute command, as didn't find the aes", from);
		return;
	}

	// send the resquest to the AES

	CMessage msgout (CNetManager::getSIDA((*aesit).ServerAlias), "SYS");
	msgout.serial (command);
	CNetManager::send ((*aesit).ServerAlias, msgout);
}


static void cbStartService (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	string serviceAlias;
	uint32 aesid;
	msgin.serial (aesid);
	msgin.serial (serviceAlias);

	AESIT aesit = findAdminExecutorService (aesid, false);
	if (aesit == AdminExecutorServices.end())
	{
		// don't find the aes, send an error message
		messageToClient (0, "couldn't start service, as didn't find the aes", from);
		return;
	}

	// send the resquest to the AES

	CMessage msgout (CNetManager::getSIDA((*aesit).ServerAlias), "STARTS");
	msgout.serial (serviceAlias);
	CNetManager::send ((*aesit).ServerAlias, msgout);
}

static void cbStopService (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	uint32 aesid, sid;
	msgin.serial (aesid);
	msgin.serial (sid);

	AESIT aesit = findAdminExecutorService (aesid, false);
	if (aesit == AdminExecutorServices.end())
	{
		messageToClient (0, "couldn't stop service, as didn't find the aes", from);
		// don't find the aes, send an error message
		return;
	}

	SIT sit = (*aesit).findService (sid, false);
	if (sit == (*aesit).Services.end())
	{
		// don't find the s, send an error message
		messageToClient (0, "couldn't stop service, as didn't find the service", from);
		return;
	}

	// send the resquest to the AES

	CMessage msgout (CNetManager::getSIDA((*aesit).ServerAlias), "STOPS");
	msgout.serial (sid);
	CNetManager::send ((*aesit).ServerAlias, msgout);
}

static void cbExecCommand (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	uint32 aesid, sid;
	string command;
	msgin.serial (aesid);
	msgin.serial (sid);
	msgin.serial (command);

	AESIT aesit = findAdminExecutorService (aesid, false);
	if (aesit == AdminExecutorServices.end())
	{
		messageToClient (0, "couldn't stop service, as didn't find the aes", from);
		// don't find the aes, send an error message
		return;
	}

	SIT sit = (*aesit).findService (sid, false);
	if (sit == (*aesit).Services.end())
	{
		// don't find the s, send an error message
		messageToClient (0, "couldn't stop service, as didn't find the service", from);
		return;
	}

	// send the resquest to the AES

	CMessage msgout (CNetManager::getSIDA((*aesit).ServerAlias), "EXEC_COMMAND");
	msgout.serial (sid);
	msgout.serial (command);
	CNetManager::send ((*aesit).ServerAlias, msgout);
}

static void cbStartAllServices (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	initStartAllServices ();
}

static void cbStopAllServices (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	for (AESIT aesit = AdminExecutorServices.begin(); aesit != AdminExecutorServices.end(); aesit++)
	{
		for (SIT sit = (*aesit).Services.begin(); sit != (*aesit).Services.end(); sit++)
		{
			if ((*sit).Connected)
			{
				CMessage msgout (CNetManager::getSIDA((*aesit).ServerAlias), "STOPS");
				msgout.serial ((*sit).Id);
				CNetManager::send ((*aesit).ServerAlias, msgout);
			}
		}
	}
}

TCallbackItem ClientCallbackArray[] =
{
	{ "AUTH", cbAuthenticateClient },
	{ "SYS", cbExecuteSystemCommand },
	{ "STARTS", cbStartService },
	{ "STOPS", cbStopService },
	{ "EXEC_COMMAND", cbExecCommand },
	{ "START_ALL_SERVICES", cbStartAllServices },
	{ "STOP_ALL_SERVICES", cbStopAllServices },
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////// SERVICE IMPLEMENTATION //////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

class CAdminService : public IService
{
public:

	/// Init the service, load the universal time.
	void		init ()
	{
//		DebugLog->addNegativeFilter ("L0:");
//		DebugLog->addNegativeFilter ("L1:");
//		DebugLog->addNegativeFilter ("L2:");
		
		
		CNetManager::setConnectionCallback ("AS", clientConnection, NULL);

		//
		// Get the list of AESHosts, add in the structures and create connection to all AES
		//

		CConfigFile::CVar &host = ConfigFile.getVar ("AESHosts");
		sint i;
		for (i = 0 ; i < host.size (); i+=2)
		{
			string serverAlias = host.asString(i);
			string serverAddr = host.asString(i+1);

			// add to the list
			CAdminExecutorService aes;
			aes.ServerAlias = serverAlias;
			aes.ServerAddr = serverAddr;
			AdminExecutorServices.push_back (aes);

			// connect to the AES
			CNetManager::setConnectionCallback (serverAlias, cbAESConnection, NULL);
			CNetManager::setDisconnectionCallback (serverAlias, cbAESDisconnection, NULL);
			CNetManager::addClient (serverAlias, serverAddr+":49996");
			CNetManager::addCallbackArray (serverAlias, AESCallbackArray, sizeof (AESCallbackArray)/sizeof(AESCallbackArray[0]));
		}

		//
		// Get the list of services in the shard
		//

		CConfigFile::CVar &serv = ConfigFile.getVar ("Services");
		for (i = 0 ; i < serv.size (); i+=2)
		{
			string serverAlias = serv.asString(i);
			string serviceAlias = serv.asString(i+1);

			AESIT aesit = findAdminExecutorService (serverAlias);

			// add new AES in the list
			CService s;
			s.AliasName = serviceAlias;
			s.InConfig = true;
			(*aesit).Services.push_back (s);
		}
		displayServices ();
	}
};


// AS is a server connection to the admin client
// AESAS is a client connection to the admin executor

/// Naming Service
NLNET_OLD_SERVICE_MAIN (CAdminService, "AS", "admin_service", 49995, ClientCallbackArray, NELNS_CONFIG, NELNS_LOGS);
