/** \file connection_as.cpp
 * 
 *
 * $Id: connection_as.cpp,v 1.1 2001/05/11 13:50:59 lecroart Exp $
 *
 * \warning the admin client works *only* on Windows because we use kbhit() and getch() functions that are not portable.
 *
 */

/* Copyright, 2001 Nevrax Ltd.
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
 
#include "nel/net/net_manager.h"

#include "datas.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

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
			nlassert ("the aes already exist but should not");
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

static void cbError (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	string error;
	msgin.serial(error);
	nlwarning("%s", error.c_str());
}

static void cbASConnection (const string &serviceName, TSockId from, void *arg)
{
	// i'm connected to a new admin service, add the new admin service in the list

	nlinfo ("push as");
	AdminServices.push_back (CAdminService(from));
	CAdminService *as = &(AdminServices.back());
	from->setAppId ((uint64)as);

	as->NetBaseName = serviceName;

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

static void cbLog (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	// received an answer for a command, give it to all admin client

	string log;
	msgin.serial (log);

	InfoLog->displayRaw("%s",log.c_str());
}


TCallbackItem ASCallbackArray[] =
{
	{ "ERR", cbError },

	{ "SL", cbServiceList },

	{ "SID", cbServiceIdentification },
	{ "SR", cbServiceReady },
	{ "SC", cbServiceConnection },
	{ "SD", cbServiceDisconnection },

	{ "AESC", cbAESConnection },
	{ "AESD", cbAESDisconnection },

	{ "LOG", cbLog },
};

void connectionASInit (string addr)
{
	CNetManager::setConnectionCallback (addr, cbASConnection, NULL);
	CNetManager::setDisconnectionCallback (addr, cbASDisconnection, NULL);
	CNetManager::addClient (addr, addr+":49995");
	CNetManager::addCallbackArray (addr, ASCallbackArray, sizeof(ASCallbackArray)/sizeof(ASCallbackArray[0]));
}
