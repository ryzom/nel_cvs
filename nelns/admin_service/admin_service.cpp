/** \file admin_service.cpp
 * Admin Service (AS)
 *
 * $Id: admin_service.cpp,v 1.2 2001/05/03 13:19:13 lecroart Exp $
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


struct CService
{
	CService(TSockId s) : AESSockId(s), AESId(NextAESId++) { }

	TSockId	AESSockId;		// TSockId of the AES that have this service
	uint32	AESId;			// TSockId of the AES that have this service
	uint32	SId;			// uint32 to identify the service
	string	ShortName;
	string	LongName;

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

static void cbServiceIdentification (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	string ShortName, LongName;

	msgin.serial (ShortName);
	msgin.serial (LongName);

	nlinfo ("%s %s %s is identified", from->asString().c_str(), ShortName.c_str(), LongName.c_str());
}

static void cbServiceReady (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	string ShortName, LongName;

	msgin.serial (ShortName);
	msgin.serial (LongName);

	nlinfo ("%s %s %s is ready", from->asString().c_str(), ShortName.c_str(), LongName.c_str());
}

static void cbServiceConnection (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	nlinfo ("%s a service is connected", from->asString().c_str());
}

static void cbServiceDisconnection (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	string ShortName, LongName;

	msgin.serial (ShortName);
	msgin.serial (LongName);

	nlinfo ("%s %s %s is disconnected", from->asString().c_str(), ShortName.c_str(), LongName.c_str());
}


TCallbackItem AESCallbackArray[] =
{
	{ "ESCR", cbExecuteSystemCommandResult },

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
	
	CMessage msgout (CNetManager::getSIDA ("AESAS"), "SL");
	uint32 size = (uint32)Services.size();
	msgout.serial (size);
	for (sit it = Services.begin(); it != Services.end(); it++)
	{
		msgout.serial ((*it).AESId);
		msgout.serial ((*it).SId);
		msgout.serial ((*it).ShortName);
		msgout.serial ((*it).LongName);
	}
	CNetManager::send ("AS", msgout, from);
}

TCallbackItem ASCallbackArray[] =
{
	{ "", NULL },
};

class CAdminService : public IService
{
public:

	/// Init the service, load the universal time.
	void		init ()
	{
		CNetManager::setConnectionCallback ("AS", clientConnection, NULL);

		// connec to the AES
		CNetManager::addClient ("AES", "localhost:49996");
		CNetManager::addCallbackArray ("AES", AESCallbackArray, sizeof (AESCallbackArray)/sizeof(AESCallbackArray[0]));

	}

	bool		update ()
	{
		return true;
	}
};


/// Naming Service
NLNET_SERVICE_MAIN (CAdminService, "AS", "admin_service", 49995, ASCallbackArray);
