/** \file admin_service.cpp
 * Admin Service (AS)
 *
 * $Id: admin_service.cpp,v 1.4 2001/05/18 16:51:33 lecroart Exp $
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

#include <string>
#include <list>

#include "nel/net/service.h"
#include "nel/misc/debug.h"
#include "nel/misc/config_file.h"

#include "nel/net/net_manager.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

////////////////////////


struct CService
{
	CService () : Id(0xFFFFFFFF), Ready(false), Connected(false), InConfig(false) { }

	uint32	Id;				/// uint32 to identify the service
	string	ServiceAlias;	/// alias of the service used in the AES and AS to find him (unique per AES)
	string	ShortName;		/// name of the service in short format ("NS" for example)
	string	LongName;		/// name of the service in long format ("naming_service")
	bool	Ready;			/// true if the service is ready
	bool	Connected;		/// true if the service is connected to the AES
	bool	InConfig;		/// true if the service is in the configuration
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
		for (SIT sit = (*aesit).Services.begin(); sit != (*aesit).Services.end(); sit++)
		{
			nlinfo ("> %s %s %s %d %d", (*aesit).SockId->asString().c_str(), (*sit).ShortName.c_str(), (*sit).LongName.c_str(), (*aesit).Id, (*sit).Id);
		}
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////// SCRIPT MANAGER /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ScriptRunning = false;
string ScriptName;
uint32 ScriptPos;

void doNextScriptStep ()
{
	nlassert (ScriptRunning);


	// get the script

	try
	{
		CConfigFile::CVar &script = IService::ConfigFile.getVar(ScriptName);

		// check the position

		if (ScriptPos*2 >= (uint32)script.size())
		{
			ScriptRunning = false;
			nlinfo("end of the script");
			// todo send that the script is finish to the admin
			return;
		}

		// get the script line

		string serverAlias;
		string serviceAlias;

		try
		{
			serverAlias = script.asString (ScriptPos*2);
			serviceAlias = script.asString (ScriptPos*2+1);
		}
		catch(EBadSize &)
		{
			nlwarning ("script '%s' not contains a good number of entries (must be a multiple of 2)", ScriptName.c_str());
			ScriptRunning = false;
			return;
		}

		AESIT aesit = findAdminExecutorService (serverAlias, false);
		if (aesit == AdminExecutorServices.end())
		{
			ScriptRunning = false;
			nlwarning("don't find the server");
			return;
		}

		// send the resquest to the AES

		CMessage msgout (CNetManager::getSIDA((*aesit).ServerAlias), "STARTS");
		msgout.serial (serviceAlias);
		CNetManager::send ((*aesit).ServerAlias, msgout);

		ScriptPos++;
	}
	catch(EUnknownVar&)
	{
		nlwarning ("script '%s' not found", ScriptName.c_str());
		ScriptRunning = false;
		return;
	}
}

void initScript(string scriptName)
{
	if (ScriptRunning)
	{
		nlwarning("already running a script, reset it");
		ScriptRunning = false;
	}

	try
	{
		CConfigFile::CVar &script = IService::ConfigFile.getVar(scriptName);

		for (sint i = 0 ; i < script.size (); i+=2)
		{
			string serverAlias = script.asString(i);
			AESIT aesit = findAdminExecutorService (serverAlias, false);
			if (aesit == AdminExecutorServices.end())
			{
				nlwarning("aes not running, can't run the script");
				return;
			}
		}
	}
	catch(EConfigFile &)
	{
		nlwarning ("bad config file");
		return;
	}

	ScriptName = scriptName;
	ScriptPos = 0;
	ScriptRunning = true;

	doNextScriptStep();
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

static void cbServiceList (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
/*	// get the service list from the admin exec and send the list to all admin client
	CAdminExecutorService *aes = (CAdminExecutorService*) from->appId();
	aes->Services.clear();

	CMessage msgout (CNetManager::getSIDA ("AS"), "SL");

	uint32 nbaes = 1;
	msgout.serial (nbaes);
	msgout.serial (aes->Id);

	uint32 nbs;
	msgin.serial (nbs);
	msgout.serial (nbs);

	for (uint32 i = 0; i < nbs; i++)
	{
		uint32 sid;
		msgin.serial(sid);

		aes->Services.push_back (CService(sid));
		CService *s = &(aes->Services.back());

		msgin.serial(s->ShortName, s->LongName, s->Ready);

		msgout.serial (s->Id, s->ShortName, s->LongName, s->Ready);
	}
	CNetManager::send ("AS", msgout, 0);
	
	displayServices ();
*/}

static void cbServiceAliasList (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	// get the service list from the admin exec and send the list to all admin client
	CAdminExecutorService *aes = (CAdminExecutorService*) from->appId();

	aes->ServiceAliasList.clear ();
	msgin.serialCont (aes->ServiceAliasList);

	CMessage msgout (CNetManager::getSIDA ("AS"), "SAL");
	msgout.serial (aes->Id);
	msgout.serialCont (aes->ServiceAliasList);
	CNetManager::send ("AS", msgout, 0);
	
	nlinfo("new service alias list");
}


static void cbServiceIdentification (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	CAdminExecutorService *aes = (CAdminExecutorService*) from->appId();

	uint32 sid;
	msgin.serial (sid);

	SIT sit = aes->findService(sid);
	msgin.serial ((*sit).ShortName, (*sit).LongName);

	nlinfo ("*:%d:%d is identified to be '%s' '%s'", aes->Id, sid, (*sit).ShortName.c_str(), (*sit).LongName.c_str());

	// broadcast the message to all admin client
	CMessage msgout (CNetManager::getSIDA ("AS"), "SID");
	msgout.serial (aes->Id, sid, (*sit).ShortName, (*sit).LongName);
	CNetManager::send ("AS", msgout, 0);
}

static void cbServiceReady (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	CAdminExecutorService *aes = (CAdminExecutorService*) from->appId();

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
	if (ScriptRunning)
		doNextScriptStep();
}

static void cbServiceConnection (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
/*	CAdminExecutorService *aes = (CAdminExecutorService*) from->appId();

	uint32 sid;
	msgin.serial (sid);

	nlinfo ("*:%d:%d connected", aes->Id, sid);

	aes->Services.push_back (CService(sid));

	// broadcast the message to all admin client
	CMessage msgout (CNetManager::getSIDA ("AS"), "SC");
	msgout.serial (aes->Id, sid);
	CNetManager::send ("AS", msgout, 0);
*/}

static void cbServiceDisconnection (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	CAdminExecutorService *aes = (CAdminExecutorService*) from->appId();

	uint32 sid;
	msgin.serial (sid);

	nlinfo ("*:%d:%d disconnected", aes->Id, sid);

	SIT sit = aes->findService(sid);

	// broadcast the message to all admin client
	CMessage msgout (CNetManager::getSIDA ("AS"), "SD");
	msgout.serial (aes->Id, (*sit).Id);
	CNetManager::send ("AS", msgout, 0);

	aes->Services.erase (sit);
}

// i'm connected to a new admin executor service
void cbAESConnection (const string &serviceName, TSockId from, void *arg)
{
	AESIT aesit = findAdminExecutorService (serviceName);	
	CAdminExecutorService *aes = &(*aesit);
	
	// set the appid to find the aes in O(1)
	from->setAppId ((uint64)aes);

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
	CAdminExecutorService *aes = (CAdminExecutorService*) from->appId();

	aes->Connected = false;
	nlinfo ("*:%d:* disconnected", aes->Id);
/*	
	// broadcast the message to all admin client that an admin exec is disconnected
	CMessage msgout (CNetManager::getSIDA ("AS"), "AESD");
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

static void cbLog (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	// received an answer for a command, give it to all admin client

	// broadcast the message to the admin service
	CMessage msgout (CNetManager::getSIDA ("AS"), "LOG");
	string log;
	msgin.serial (log);
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

	{ "LOG", cbLog },
};


////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////// CONNECTION TO THE CLIENT ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void errorMessage(string message, TSockId from, CCallbackNetBase &netbase)
{
	CMessage msgout (netbase.getSIDA (), "ERR");
	msgout.serial (message);
	netbase.send (msgout, from);
}

//
// A new admin client is connected.
//
void clientConnection (const string &serviceName, TSockId from, void *arg)
{
	// new client, send him all out info about services

	nlinfo ("client %s is connected", from->asString().c_str());

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
			
			msgout.serial ((*sit).Id, (*sit).ServiceAlias, (*sit).ShortName, (*sit).LongName);
			msgout.serial ((*sit).Ready, (*sit).Connected, (*sit).InConfig);
		}
	}
	CNetManager::send ("AS", msgout, from);

	//
	// send service alias list
	//

	for (aesit = AdminExecutorServices.begin(); aesit != AdminExecutorServices.end(); aesit++)
	{
		CMessage msgout2 (CNetManager::getSIDA ("AS"), "SAL");
		msgout2.serial ((*aesit).Id);
		msgout2.serialCont ((*aesit).ServiceAliasList);
		CNetManager::send ("AS", msgout2, from);
	}
}


static void cbExecuteScript (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	string script;
	msgin.serial (script);
	initScript (script);
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
		errorMessage ("couldn't execute command, as didn't find the aes", from, netbase);
		return;
	}

	// send the resquest to the AES

	CMessage msgout (CNetManager::getSIDA((*aesit).ServerAlias), "SYS");
	msgout.serial (command);
	CNetManager::send ((*aesit).ServerAlias, msgout);
}


static void cbStartService (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	string serviceName;
	uint32 aesid;
	msgin.serial (aesid);
	msgin.serial (serviceName);

	AESIT aesit = findAdminExecutorService (aesid, false);
	if (aesit == AdminExecutorServices.end())
	{
		// don't find the aes, send an error message
		errorMessage ("couldn't start service, as didn't find the aes", from, netbase);
		return;
	}

	// send the resquest to the AES

	CMessage msgout (CNetManager::getSIDA((*aesit).ServerAlias), "STARTS");
	msgout.serial (serviceName);
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
		errorMessage ("couldn't stop service, as didn't find the aes", from, netbase);
		// don't find the aes, send an error message
		return;
	}

	SIT sit = (*aesit).findService (sid, false);
	if (sit == (*aesit).Services.end())
	{
		// don't find the s, send an error message
		errorMessage ("couldn't stop service, as didn't find the service", from, netbase);
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
		errorMessage ("couldn't stop service, as didn't find the aes", from, netbase);
		// don't find the aes, send an error message
		return;
	}

	SIT sit = (*aesit).findService (sid, false);
	if (sit == (*aesit).Services.end())
	{
		// don't find the s, send an error message
		errorMessage ("couldn't stop service, as didn't find the service", from, netbase);
		return;
	}

	// send the resquest to the AES

	CMessage msgout (CNetManager::getSIDA((*aesit).ServerAlias), "EXEC_COMMAND");
	msgout.serial (sid);
	msgout.serial (command);
	CNetManager::send ((*aesit).ServerAlias, msgout);
}


TCallbackItem ClientCallbackArray[] =
{
	{ "SYS", cbExecuteSystemCommand },
	{ "EXEC", cbExecuteScript },
	{ "STARTS", cbStartService },
	{ "STOPS", cbStopService },
	{ "EXEC_COMMAND", cbExecCommand },
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
		DebugLog->addNegativeFilter ("L0:");
		DebugLog->addNegativeFilter ("L1:");
		DebugLog->addNegativeFilter ("L2:");
		
		
		CNetManager::setConnectionCallback ("AS", clientConnection, NULL);

		//
		// Get the list of AESHosts, add in the structures and create connection to all AES
		//

		CConfigFile::CVar &host = ConfigFile.getVar ("AESHosts");
		for (sint i = 0 ; i < host.size (); i+=2)
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
			s.ServiceAlias = serviceAlias;
			s.InConfig = true;
			(*aesit).Services.push_back (s);
		}
	}

	bool		update ()
	{
		return true;
	}
};


// AS is a server connection to the admin client
// AESAS is a client connection to the admin executor

/// Naming Service
NLNET_SERVICE_MAIN (CAdminService, "AS", "admin_service", 49995, ClientCallbackArray);
