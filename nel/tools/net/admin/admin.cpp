/** \file admin.cpp
 * 
 *
 * $Id: admin.cpp,v 1.2 2001/05/10 08:19:57 lecroart Exp $
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

#include <conio.h>
#include <io.h>
#include <process.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <string>
#include <list>

#include "nel/misc/debug.h"
#include "nel/misc/config_file.h"
#include "nel/misc/thread.h"
#include "nel/misc/command.h"
#include "nel/misc/log.h"
#include "nel/misc/displayer.h"

#include "nel/net/service.h"
#include "nel/net/net_manager.h"

 
using namespace std;
using namespace NLMISC;
using namespace NLNET;


/////////////


struct CService
{
	CService (uint32 sid) : Id(sid), Ready(false) { nlinfo ("adding service %d", sid); }

	~CService() { nlinfo("removing service %d %s %s", Id, ShortName.c_str(), LongName.c_str()); }

	uint32	Id;				/// uint32 to identify the service
	string	ShortName;		/// name of the service in short format ("NS" for example)
	string	LongName;		/// name of the service in long format ("naming_service")
	bool	Ready;			/// true if the service is ready
};

typedef list<CService> TServices;
typedef list<CService>::iterator SIT;


struct CAdminExecutorService
{
	CAdminExecutorService (uint32 aesid) : Id(aesid) { }

	~CAdminExecutorService() { nlinfo("removing AES %d", Id); }

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
};

typedef list<CAdminExecutorService> TAdminExecutorServices;
typedef list<CAdminExecutorService>::iterator AESIT;


struct CAdminService
{
	CAdminService (TSockId s) : SockId(s), Id(NextId++) { }
	
	~CAdminService() { nlinfo("removing AS %d", Id); }

	TSockId	SockId;			/// connection to the AS
	uint32	Id;				/// uint32 to identify the AS

	TAdminExecutorServices AdminExecutorServices;

	AESIT findAdminExecutorService (uint32 aesid, bool asrt=true)
	{
		AESIT aesit;
		for (aesit = AdminExecutorServices.begin(); aesit != AdminExecutorServices.end(); aesit++)
			if ((*aesit).Id == aesid)
				break;

		if (asrt)
			nlassert (aesit != AdminExecutorServices.end());
		return aesit;
	}

private:
	static uint32 NextId;
};

uint32 CAdminService::NextId = 1;

typedef list<CAdminService> TAdminServices;
typedef list<CAdminService>::iterator ASIT;


TAdminServices AdminServices;

////////////////////////


ASIT findAdminService (uint32 asid)
{
	ASIT asit;
	for (asit = AdminServices.begin(); asit != AdminServices.end(); asit++)
		if ((*asit).Id == asid)
			break;

	nlassert (asit != AdminServices.end());
	return asit;
}

void displayServices ()
{
	for (ASIT asit = AdminServices.begin(); asit != AdminServices.end(); asit++)
	{
		nlinfo ("AS %d", (*asit).Id);
		for (AESIT aesit = (*asit).AdminExecutorServices.begin(); aesit != (*asit).AdminExecutorServices.end(); aesit++)
		{
			nlinfo (" AES %d", (*asit).Id);
			for (SIT sit = (*aesit).Services.begin(); sit != (*aesit).Services.end(); sit++)
			{
				nlinfo ("  > %d:%d:%d %s %s %s", (*asit).Id, (*aesit).Id, (*sit).Id, (*asit).SockId->asString().c_str(), (*sit).ShortName.c_str(), (*sit).LongName.c_str());
			}
		}
	}
}



////////////////
/*
struct CService
{
	CService(TSockId s) : ASSockId(s) { }

	TSockId	ASSockId;
	uint32	ASId;			// uint32 to identify the AS where the AES where the service is running the service
	uint32	AESId;			// uint32 to identify the AES where the service is running
	uint32	SId;			// uint32 to identify the service
	string	ShortName;
	string	LongName;
};

list<CService> Services;
typedef list<CService>::iterator sit;

sit find (TSockId asid, uint32 aesid, uint32 sid)
{
	sit it;
	for (it = Services.begin(); it != Services.end(); it++)
	{
		if ((*it).ASSockId == asid && (*it).AESId == aesid && (*it).SId == sid) break;
	}
	return it;
}

sit find (uint32 asid, uint32 aesid, uint32 sid)
{
	sit it;
	for (it = Services.begin(); it != Services.end(); it++)
	{
		if ((*it).ASId == asid && (*it).AESId == aesid && (*it).SId == sid) break;
	}
	return it;
}

void displayServices ()
{
	nlinfo ("There's %d service in the list", Services.size());
	for (sit it = Services.begin(); it != Services.end(); it++)
	{
		nlinfo ("> %s %s %s %d %d %d", (*it).ASSockId->asString().c_str(), (*it).ShortName.c_str(), (*it).LongName.c_str(), (*it).ASId, (*it).AESId, (*it).SId);
	}
	nlinfo ("End of the list");
}
*/
////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////// CONNECTION TO THE AS ////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void cbServiceList (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	// receive all information about the admin service
	CAdminService *as = (CAdminService*) from->appId();

	// get the list of all aes
	uint32 nbaes;
	msgin.serial(nbaes);

	for (uint32 i = 0; i < nbaes; i++)
	{
		uint32 aesid;
		msgin.serial(aesid);

		AESIT aesit = as->findAdminExecutorService (aesid, false);
		// todo normalement, l aes ne doit pas exister => faire un assert si il existe
		CAdminExecutorService *aes;
		if (aesit == as->AdminExecutorServices.end ())
		{
			nlinfo ("push aes");
			as->AdminExecutorServices.push_back (CAdminExecutorService(aesid));
			aes = &(as->AdminExecutorServices.back());
		}
		else
		{
			nlwarning ("the aes already exist but should not");
			aes = &(*aesit);
		}

		// clear everything
		aes->Services.clear ();

		// get the list of service in this aes
		uint32 nbs;
		msgin.serial(nbs);

		for (uint32 j = 0; j < nbs; j++)
		{
			uint32 sid;
			msgin.serial(sid);
			nlinfo ("push ser");
			aes->Services.push_back(CService(sid));
			CService *s = &(aes->Services.back());
			msgin.serial(s->ShortName, s->LongName, s->Ready);
		}
	}

	displayServices ();
}

static void cbServiceIdentification (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	CAdminService *as = (CAdminService*) from->appId();

	uint32 aesid, sid;
	msgin.serial (aesid, sid);

	AESIT aesit = as->findAdminExecutorService(aesid);
	SIT sit = (*aesit).findService(sid);
	msgin.serial ((*sit).ShortName, (*sit).LongName);

	nlinfo ("%d:%d:%d is identified to be '%s' '%s'", as->Id, aesid, sid, (*sit).ShortName.c_str(), (*sit).LongName.c_str());
}

static void cbServiceReady (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	CAdminService *as = (CAdminService*) from->appId();

	uint32 aesid, sid;
	msgin.serial (aesid, sid);

	AESIT aesit = as->findAdminExecutorService(aesid);
	SIT sit = (*aesit).findService(sid);
	(*sit).Ready = true;

	nlinfo ("%d:%d:%d is ready", as->Id, aesid, sid);
}

static void cbServiceConnection (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	CAdminService *as = (CAdminService*) from->appId();

	uint32 aesid, sid;
	msgin.serial (aesid, sid);
	
	nlinfo ("%d:%d:%d connected", as->Id, aesid, sid);

	AESIT aesit = as->findAdminExecutorService(aesid);
	nlinfo ("push ser");
	(*aesit).Services.push_back (CService(sid));
}

static void cbServiceDisconnection (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	// remove the service from the list
	CAdminService *as = (CAdminService*) from->appId();

	uint32 aesid, sid;
	msgin.serial (aesid, sid);

	nlinfo ("%d:%d:%d disconnected", as->Id, aesid, sid);

	AESIT aesit = as->findAdminExecutorService(aesid);
	nlinfo ("erase ser");
	(*aesit).Services.erase ((*aesit).findService(sid));
}


static void cbAESConnection (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	// an AES is disconnected, remove the aes from the list
	CAdminService *as = (CAdminService*) from->appId();

	uint32 aesid;
	msgin.serial (aesid);

	nlinfo ("%d:%d:* connected", as->Id, aesid);

	nlinfo ("push aes");
	as->AdminExecutorServices.push_back (CAdminExecutorService(aesid));
}


static void cbAESDisconnection (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	// an AES is disconnected, remove the aes from the list
	CAdminService *as = (CAdminService*) from->appId();

	uint32 aesid;
	msgin.serial (aesid);

	nlinfo ("%d:%d:* disconnected", as->Id, aesid);

	nlinfo ("erase aes");
	as->AdminExecutorServices.erase (as->findAdminExecutorService(aesid));
}


static void cbASConnection (const string &serviceName, TSockId from, void *arg)
{
	// i'm connected to a new admin service, add the new admin service in the list

	nlinfo ("push as");
	AdminServices.push_back (CAdminService(from));
	CAdminService *as = &(AdminServices.back());
	from->setAppId ((uint64)as);

	nlinfo ("%d:*:* connected", as->Id);
}

static void cbASDisconnection (const string &serviceName, TSockId from, void *arg)
{
	// lost the connection to an admin service
	CAdminService *as = (CAdminService*) from->appId();

	nlinfo ("%d:*:* disconnected", as->Id);

	// remove the admin service
	nlinfo ("erase as");
	AdminServices.erase (findAdminService(as->Id));
}

TCallbackItem ASCallbackArray[] =
{
	{ "SL", cbServiceList },

	{ "SID", cbServiceIdentification },
	{ "SR", cbServiceReady },
	{ "SC", cbServiceConnection },
	{ "SD", cbServiceDisconnection },

	{ "AESC", cbAESConnection },
	{ "AESD", cbAESDisconnection },
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// COMMANDS ////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

NLMISC_COMMAND (connect, "connect to the AS", "<ip>")
{
	if(args.size() != 1) return false;

	CNetManager::setConnectionCallback (args[0], cbASConnection, NULL);
	CNetManager::setDisconnectionCallback (args[0], cbASDisconnection, NULL);
	CNetManager::addClient (args[0], args[0]+":49995");
	CNetManager::addCallbackArray (args[0], ASCallbackArray, sizeof(ASCallbackArray)/sizeof(ASCallbackArray[0]));

	return true;
}

NLMISC_COMMAND (start, "start a service", "<asid> <aesid> <service_name>")
{
	if(args.size() != 3) return false;

	
	return true;
}

NLMISC_COMMAND (exec, "execute a command", "<asid> <aesid> <service_name>")
{
	if(args.size() != 2 && args.size() != 3) return false;

	CMessage msgout (CNetManager::getSIDA("AES"), "ESC");
	msgout.serial (const_cast<string &>(args[1]));

	uint8 background = 0;

	if (args.size() == 2)
	{
		if (args[1] == "&")
			uint8 background = 1;
		else
			return false;
	}

//	msgout.serial (background);
//	CNetManager::send ("AES", msgout);
	return true;
}

NLMISC_COMMAND (stop, "stop a service", "<asid> <aesid> <sid>")
{
	if(args.size() != 2) return false;

	CMessage msgout (CNetManager::getSIDA("AES"), "SS");
	msgout.serial (const_cast<string &>(args[2]));
	CNetManager::send ("AES", msgout);
	return true;
}

NLMISC_COMMAND (list, "display list of service", "")
{
	if(args.size() != 0) return false;

	displayServices ();
	
	return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////// IMPLEMENTATION //////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void printLine(string line)
{
	printf("\r> %s \b", line.c_str());
}

CLog logstdout;
CStdDisplayer dispstdout;

int main (int argc, char **argv)
{
	logstdout.addDisplayer (&dispstdout);

	nlinfo("Admin client for NeL Shard administration ("__DATE__" "__TIME__")\n");

	DebugLog->addNegativeFilter ("L0:");
	DebugLog->addNegativeFilter ("L1:");
	DebugLog->addNegativeFilter ("L2:");

	string command;
	printLine(command);

	bool end = false;
	while (!end)
	{
		while (kbhit())
		{
			int c = getch();
			switch (c)
			{
			case  8: if (command.size()>0) command.resize (command.size()-1); printLine(command); break;
			case 27: end = true; break;
			case 13: printf("\n"); nlinfo("execute command: %s", command.c_str()); ICommand::execute(command, logstdout); command = ""; printLine(command); break;
			default: command += c; printLine(command); break;
			}
		}

		CNetManager::update();
	}

	return EXIT_SUCCESS;
}
