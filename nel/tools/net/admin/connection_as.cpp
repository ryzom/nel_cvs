/** \file connection_as.cpp
 * 
 *
 * $Id: connection_as.cpp,v 1.3 2001/05/31 16:41:59 lecroart Exp $
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
#include "interf.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;





///
/// Get the list of AES on the AS
///
static void cbAESList (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	CAdminService *as = (CAdminService*) from->appId();

	uint32 nbaes;
	msgin.serial(nbaes);

	for (uint32 i = 0; i < nbaes; i++)
	{
		CAdminExecutorService aes;
		msgin.serial (aes.Id, aes.ServerAlias, aes.ServerAddr, aes.Connected);

		AESIT aesit = as->findAdminExecutorService (aes.Id, false);
		if (aesit == as->AdminExecutorServices.end ())
		{
			aes.AS = as;
			as->AdminExecutorServices.push_back (aes);
			interfAddAES (as, &(as->AdminExecutorServices.back()));
		}
		else
		{
			nlinfo ("the aes already exists, update it");

			// new aes connection
			if (aes.Connected && !(*aesit).Connected)
			{
				for (SIT sit = (*aesit).Services.begin (); sit != (*aesit).Services.end (); sit++)
				{
					(*sit).Unknown = false;
					(*sit).Connected = false;
					(*sit).Ready = false;
					interfUpdateS (&(*sit));
				}
			}

			// new aes disconnection
			if (!aes.Connected && (*aesit).Connected)
			{
				for (SIT sit = (*aesit).Services.begin (); sit != (*aesit).Services.end (); sit++)
				{
					(*sit).Unknown = true;
					(*sit).Connected = false;
					(*sit).Ready = false;
					(*sit).Id = 0xFFFFFFFF;
					(*sit).ShortName = (*sit).LongName = "";
					interfUpdateS (&(*sit));
				}
			}

			// copy the new values
			(*aesit).setValues (aes);
			interfUpdateAES (&(*aesit));
		}
	}

	displayServices ();
}

/// Get the list of Services on the AS
static void cbServiceList (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	CAdminService *as = (CAdminService*) from->appId();

	// get the list of all aes
	uint32 nbaes;
	msgin.serial(nbaes);

	for (uint32 i = 0; i < nbaes; i++)
	{
		uint32 aesid;
		msgin.serial(aesid);

		AESIT aesit = as->findAdminExecutorService (aesid);

		// get the list of service in this aes
		uint32 nbs;
		msgin.serial(nbs);

		for (uint32 j = 0; j < nbs; j++)
		{
			CService s;
			msgin.serial (s.Id, s.AliasName, s.ShortName, s.LongName);
			msgin.serial (s.Ready, s.Connected, s.InConfig);

			s.Unknown = !(*aesit).Connected;

			SIT sit = (*aesit).findService (s.AliasName, false);
			if (sit == (*aesit).Services.end ())
			{
				s.AES = &(*aesit);
				(*aesit).Services.push_back (s);
				interfAddS (&(*aesit), &((*aesit).Services.back()));
			}
			else
			{
				nlassert ("the service already exists, update it");
				(*sit).setValues (s);
				interfUpdateS (&(*sit));
			}
		}
	}

	displayServices ();
}








static void cbServiceList_ (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
///////////////////////////
	return;
///////////////////////////

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
			as->AdminExecutorServices.push_back (CAdminExecutorService());
			aes = &(as->AdminExecutorServices.back());
			aes->Id = aesid;
			interfAddAES (as, aes);
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
			aes->Services.push_back(CService());
			CService *s = &(aes->Services.back());
			s->Id = sid;
			msgin.serial(s->ShortName, s->LongName, s->Ready);
			interfAddS(aes, s);
		}
	}

	displayServices ();
}

static void cbServiceAliasList (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	// receive all information about the admin service
	CAdminService *as = (CAdminService*) from->appId();

	uint32 aesid;
	msgin.serial(aesid);

	AESIT aesit = as->findAdminExecutorService (aesid);

//	removeServiceAliasPopup (&(*aesit));

	(*aesit).ServiceAliasList.clear ();
	msgin.serialCont ((*aesit).ServiceAliasList);

//	addServiceAliasPopup (&(*aesit));
}

// todo kan on lance un service, ce deco et se reco, le service est ready mais pas connected

static void cbServiceIdentification (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	CAdminService *as = (CAdminService*) from->appId();

	uint32 aesid, sid;
	string alias;

	msgin.serial (aesid, sid, alias);
	
	AESIT aesit = as->findAdminExecutorService(aesid);

	SIT sit;
	if (!alias.empty())
	{
		sit = (*aesit).findService (alias, false);

		if (sit == (*aesit).Services.end ())
		{
			// the alias is not found
			nlwarning ("new service with alias (%s) but not in my list", alias.c_str());

			(*aesit).Services.push_back (CService ());
			sit = (*aesit).Services.end();
			sit--;
			interfAddS (&(*aesit), &(*sit));
		}
		else
		{
			// normal case
		}
	}
	else
	{
		sit = (*aesit).findService (sid, false);

		if (sit == (*aesit).Services.end ())
		{
			// normal case for unknown services
			nlwarning ("new service with alias (%s) but not in my list", alias.c_str());
		}
		else
		{
			nlwarning ("new service without alias is already in my list with id %d", sid);
		}
		(*aesit).Services.push_back (CService ());
		sit = (*aesit).Services.end();
		sit--;
		interfAddS (&(*aesit), &(*sit));
	}
	
	(*sit).Id = sid;
	(*sit).AliasName = alias;
	(*sit).Connected = true;
	msgin.serial ((*sit).ShortName, (*sit).LongName);

	nlinfo ("%d:%d:%d is identified to be '%s' '%s' '%s'", as->Id, aesid, sid, (*sit).AliasName.c_str(), (*sit).ShortName.c_str(), (*sit).LongName.c_str());

	interfUpdateS (&(*sit));
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

	interfUpdateS (&(*sit));
}

static void cbServiceConnection (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	CAdminService *as = (CAdminService*) from->appId();

	uint32 aesid, sid;
	msgin.serial (aesid, sid);
	
	nlinfo ("%d:%d:%d connected", as->Id, aesid, sid);
/*
	AESIT aesit = as->findAdminExecutorService(aesid);
	(*aesit).Services.push_back (CService());
	CService *s = &((*aesit).Services.back());
	s->Id = sid;
	s->AES = &(*aesit);
	interfAddS(&(*aesit), s);
*/}

static void cbServiceDisconnection (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	// remove the service from the list
	CAdminService *as = (CAdminService*) from->appId();

	uint32 aesid, sid;
	msgin.serial (aesid, sid);

	nlinfo ("%d:%d:%d disconnected", as->Id, aesid, sid);

	AESIT aesit = as->findAdminExecutorService(aesid);
	SIT sit = (*aesit).findService(sid);

	if ((*sit).InConfig)
	{
		(*sit).Ready = (*sit).Connected = false;
		(*sit).Id = 0xFFFFFFFF;
		(*sit).ShortName = (*sit).LongName = "";
		interfUpdateS (&(*sit));
	}
	else
	{
		interfRemoveS (&(*sit));
		(*aesit).Services.erase (sit);
	}
}


static void cbAESConnection (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	// an AES is disconnected, remove the aes from the list
	CAdminService *as = (CAdminService*) from->appId();

	uint32 aesid;
	msgin.serial (aesid);

	nlinfo ("%d:%d:* connected", as->Id, aesid);

	as->AdminExecutorServices.push_back (CAdminExecutorService());
	CAdminExecutorService *aes = &(as->AdminExecutorServices.back());
	aes->AS = as;
	aes->Id = aesid;
	interfAddAES(as, aes);
}


static void cbAESDisconnection (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	// an AES is disconnected, remove the aes from the list
	CAdminService *as = (CAdminService*) from->appId();

	uint32 aesid;
	msgin.serial (aesid);

	nlinfo ("%d:%d:* disconnected", as->Id, aesid);

	AESIT aesit = as->findAdminExecutorService(aesid);
	interfRemoveAES(&(*aesit));
	as->AdminExecutorServices.erase (aesit);
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

	CAdminService *as = (CAdminService *) arg;
	from->setAppId ((uint64)as);

	as->Connected = true;
	as->SockId = from;

//	interfAddAS(as);
	setBitmap ("as_cnt.xpm", as->Bitmap);

	nlinfo ("%d:*:* connected", as->Id);
}

static void cbASDisconnection (const string &serviceName, TSockId from, void *arg)
{
	// lost the connection to an admin service
	CAdminService *as = (CAdminService*) from->appId();

	nlinfo ("%d:*:* disconnected", as->Id);

	// remove the admin service
	//ASIT asit = findAdminService(as->Id);
	//interfRemoveAS(&(*asit));
	//AdminServices.erase (asit);
	as->Connected = false;
	as->SockId = NULL;
	setBitmap ("as_dcnt.xpm", as->Bitmap);
	removeSubTree (as);
	as->AdminExecutorServices.clear();
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

	{ "SL", cbServiceList_ },
	{ "SAL", cbServiceAliasList },

	{ "SID", cbServiceIdentification },
	{ "SR", cbServiceReady },
	{ "SC", cbServiceConnection },
	{ "SD", cbServiceDisconnection },

	{ "AESC", cbAESConnection },
	{ "AESD", cbAESDisconnection },

	{ "LOG", cbLog },


	{ "AES_LIST", cbAESList },
	{ "SERVICE_LIST", cbServiceList },
};

void connectionASInit (CAdminService *as)
{
	CNetManager::setConnectionCallback (as->ASAddr, cbASConnection, as);
	CNetManager::setDisconnectionCallback (as->ASAddr, cbASDisconnection, NULL);
	CNetManager::addClient (as->ASAddr, as->ASAddr+":49995", false);
	CNetManager::addCallbackArray (as->ASAddr, ASCallbackArray, sizeof(ASCallbackArray)/sizeof(ASCallbackArray[0]));
}

void connectionASRelease (CAdminService *as)
{
	CNetManager::getNetBase(as->ASAddr)->disconnect ();
	cbASDisconnection (as->ASAddr, as->SockId, NULL);
}
