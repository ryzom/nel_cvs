/** \file admin_service.cpp
 * Admin Service (AS)
 *
 * $Id: admin_service.cpp,v 1.3 2001/05/10 08:20:06 lecroart Exp $
 *
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX D.T.C. SYSTEM.
 * NEVRAX D.T.C. SYSTEM is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * NEVRAX D.T.C. SYSTEM is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with NEVRAX D.T.C. SYSTEM; see the file COPYING. If not, write to the
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
	CService (uint32 sid) : Id(sid), Ready(false) { }

	uint32	Id;				/// uint32 to identify the service
	string	ShortName;		/// name of the service in short format ("NS" for example)
	string	LongName;		/// name of the service in long format ("naming_service")
	bool	Ready;			/// true if the service is ready
};

typedef list<CService> TServices;
typedef list<CService>::iterator SIT;

struct CAdminExecutorService
{
	CAdminExecutorService (TSockId s) : SockId(s), Id(NextId++) { }

	TSockId	SockId;			/// connection to the AES
	uint32	Id;				/// uint32 to identify the AES where the service is running

	TServices Services;

	SIT findService (uint32 sid)
	{
		SIT sit;
		for (sit = Services.begin(); sit != Services.end(); sit++)
			if ((*sit).Id == sid)
				break;

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

AESIT findAdminExecutorService (uint32 aesid)
{
	AESIT aesit;
	for (aesit = AdminExecutorServices.begin(); aesit != AdminExecutorServices.end(); aesit++)
		if ((*aesit).Id == aesid)
			break;

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



////////////////////////
/*
struct CService
{
	CService(TSockId s) : AESSockId(s), AESId(NextAESId++) { }

	TSockId	AESSockId;		// TSockId of the AES that have this service
	uint32	AESId;			// uint32 to identify the AES where the service is running
	uint32	SId;			// uint32 to identify the service
	string	ShortName;
	string	LongName;
//ajouter ready dans la liste
private:

	static	uint32 NextAESId;
};

uint32 CService::NextAESId = 0;

list<CService> Services;
typedef list<CService>::iterator sit;

sit find (TSockId aesid, uint32 sid)
{
	sit it;
	for (it = Services.begin(); it != Services.end(); it++)
	{
		if ((*it).AESSockId== aesid && (*it).SId == sid) break;
	}
	return it;
}

sit find (uint32 aesid, uint32 sid)
{
	sit it;
	for (it = Services.begin(); it != Services.end(); it++)
	{
		if ((*it).AESId == aesid && (*it).SId == sid) break;
	}
	return it;
}

void displayServices ()
{
	nlinfo ("There's %d service in the list", Services.size());
	for (sit it = Services.begin(); it != Services.end(); it++)
	{
		nlinfo ("> %s %s %s %d %d", (*it).AESSockId->asString().c_str(), (*it).ShortName.c_str(), (*it).LongName.c_str(), (*it).AESId, (*it).SId);
	}
	nlinfo ("End of the list");
}
*/
/*
NLMISC_COMMAND (start, "start a service", "<service_name>")
{
	if(args.size() != 1 && args.size() != 2) return false;

	CMessage msgout (CNetManager::getSIDA("AES"), "ESC");
	msgout.serial (args[1]);

	uint8 background = 0;

	if (args.size() == 2)
	{
		if (args[1] == "&")
			uint8 background = 1;
		else
			return false;
	}

	msgout.serial (background);
	CNetManager::send ("AES", msgout);
	return true;
}


CLog logstdout;
CStdDisplayer dispstdout;

NLMISC_COMMAND (stop, "stop a service", "<aesid> <sid>")
{
	if(args.size() != 3) return false;

	CMessage msgout (CNetManager::getSIDA("AES"), "SS");
	msgout.serial (args[2]);
	CNetManager::send ("AES", msgout);
}
*/

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
	// get the service list from the admin exec and send the list to all admin client
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
}

static void cbServiceConnection (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	CAdminExecutorService *aes = (CAdminExecutorService*) from->appId();

	uint32 sid;
	msgin.serial (sid);

	nlinfo ("*:%d:%d connected", aes->Id, sid);

	aes->Services.push_back (CService(sid));

	// broadcast the message to all admin client
	CMessage msgout (CNetManager::getSIDA ("AS"), "SC");
	msgout.serial (aes->Id, sid);
	CNetManager::send ("AS", msgout, 0);
}

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
	AdminExecutorServices.push_back (CAdminExecutorService(from));
	CAdminExecutorService *aes = &(AdminExecutorServices.back());
	from->setAppId ((uint64)aes);

	nlinfo ("*:%d:* connected", aes->Id);
	
	// broadcast the message that an admin exec is connected to all admin client
	CMessage msgout (CNetManager::getSIDA ("AS"), "AESC");
	msgout.serial (aes->Id);
	CNetManager::send ("AS", msgout, 0);
}

// i'm disconnected to an admin executor service
void cbAESDisconnection (const string &serviceName, TSockId from, void *arg)
{
	// lost the connection to an admin exec service
	CAdminExecutorService *aes = (CAdminExecutorService*) from->appId();

	nlinfo ("*:%d:* disconnected", aes->Id);
	
	// broadcast the message to all admin client that an admin exec is disconnected
	CMessage msgout (CNetManager::getSIDA ("AS"), "AESD");
	msgout.serial (aes->Id);
	CNetManager::send ("AS", msgout, 0);

	// remove the admin exec service
	AdminExecutorServices.erase (findAdminExecutorService(aes->Id));
}

TCallbackItem AESCallbackArray[] =
{
	{ "ESCR", cbExecuteSystemCommandResult },

	{ "SL", cbServiceList },
	{ "SID", cbServiceIdentification },
	{ "SR", cbServiceReady },
	{ "SC", cbServiceConnection },
	{ "SD", cbServiceDisconnection },
};


////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////// CONNECTION TO THE CLIENT ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void clientConnection (const string &serviceName, TSockId from, void *arg)
{
	// new client, send him all out info about services

	nlinfo ("client %s is connected", from->asString().c_str());
	
	CMessage msgout (CNetManager::getSIDA ("AS"), "SL");

	uint32 nbaes = (uint32)AdminExecutorServices.size();
	msgout.serial (nbaes);
	for (AESIT aesit = AdminExecutorServices.begin(); aesit != AdminExecutorServices.end(); aesit++)
	{
		msgout.serial ((*aesit).Id);

		uint32 nbs = (uint32)(*aesit).Services.size();
		msgout.serial (nbs);

		for (SIT sit = (*aesit).Services.begin(); sit != (*aesit).Services.end(); sit++)
		{
			msgout.serial ((*sit).Id);
			msgout.serial ((*sit).ShortName);
			msgout.serial ((*sit).LongName);
			msgout.serial ((*sit).Ready);
		}
	}
	CNetManager::send ("AS", msgout, from);
}

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

		// connect to the AES
		CNetManager::setConnectionCallback ("AESAS", cbAESConnection, NULL);
		CNetManager::setDisconnectionCallback ("AESAS", cbAESDisconnection, NULL);
		CNetManager::addClient ("AESAS", "localhost:49996");
		CNetManager::addCallbackArray ("AESAS", AESCallbackArray, sizeof (AESCallbackArray)/sizeof(AESCallbackArray[0]));

	}

	bool		update ()
	{
		return true;
	}
};


// AS is a server connection to the admin client
// AESAS is a client connection to the admin executor

/// Naming Service
NLNET_SERVICE_MAIN (CAdminService, "AS", "admin_service", 49995, EmptyCallbackArray);
