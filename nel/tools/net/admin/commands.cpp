/** \file coammnds.cpp
 * 
 *
 * $Id: commands.cpp,v 1.6 2002/03/25 10:19:13 lecroart Exp $
 *
 * contains all admin commands
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

#include "nel/misc/types_nl.h"

//#include "nel/misc/debug.h"
//#include "nel/misc/config_file.h"
#include "nel/misc/command.h"
//#include "nel/misc/log.h"
//#include "nel/misc/displayer.h"

//#include "nel/net/service.h"
#include "nel/net/net_manager.h"

#include "datas.h"
#include "connection_as.h"
 
using namespace std;
using namespace NLMISC;
using namespace NLNET;


NLMISC_COMMAND (connect, "connect to the AS", "<asid>")
{
	if(args.size() != 1) return false;

	uint32 asid = atoi (args[0].c_str());
	ASIT asit = findAdminService (asid);

	connectionASInit (&(*asit));
	return true;
}


NLMISC_COMMAND (start_service, "start a service", "<asid> <aesid> <service_alias>")
{
	if(args.size() != 3) return false;

	uint32 asid = atoi(args[0].c_str());
	ASIT asit = findAdminService (asid, false);
	if (asit == AdminServices.end())
	{
		nlwarning("couldn't start service, asid %d doesn't exist", asid);
		return false;
	}

	uint32 aesid = atoi(args[1].c_str());
	AESIT aesit = (*asit).findAdminExecutorService (aesid, false);
	if (aesit == (*asit).AdminExecutorServices.end())
	{
		nlwarning("couldn't start service, aesid %d doesn't exist", aesid);
		return false;
	}

	CMessage msgout (CNetManager::getSIDA((*asit).ASAddr), "STARTS");
	msgout.serial (aesid);
	msgout.serial (const_cast<string &>(args[2]));
	CNetManager::send ((*asit).ASAddr, msgout);

	return true;
}


NLMISC_COMMAND (stop_service, "stop a service", "<asid> <aesid> <sid>")
{
	if(args.size() != 3) return false;

	uint32 asid = atoi(args[0].c_str());
	ASIT asit = findAdminService (asid, false);
	if (asit == AdminServices.end())
	{
		nlwarning("couldn't stop service, asid %d doesn't exist", asid);
		return false;
	}

	uint32 aesid = atoi(args[1].c_str());
	AESIT aesit = (*asit).findAdminExecutorService (aesid, false);
	if (aesit == (*asit).AdminExecutorServices.end())
	{
		nlwarning("couldn't stop service, aesid %d doesn't exist", aesid);
		return false;
	}

	uint32 sid = atoi(args[2].c_str());
	SIT sit = (*aesit).findService (sid, false);
	if (sit == (*aesit).Services.end())
	{
		nlwarning("couldn't stop service, sid %d doesn't exist", sid);
		return false;
	}

	CMessage msgout (CNetManager::getSIDA((*asit).ASAddr), "STOPS");
	msgout.serial (aesid);
	msgout.serial (sid);
	CNetManager::send ((*asit).ASAddr, msgout);
	return true;
}


NLMISC_COMMAND (ec, "execute a command on a service", "<asid> <aesid> <sid> <command>")
{
	if(args.size() < 4) return false;

	uint32 asid = atoi(args[0].c_str());
	ASIT asit = findAdminService (asid, false);
	if (asit == AdminServices.end())
	{
		nlwarning("couldn't execute a command on this service, asid %d doesn't exist", asid);
		return false;
	}

	uint32 aesid = atoi(args[1].c_str());
	AESIT aesit = (*asit).findAdminExecutorService (aesid, false);
	if (aesit == (*asit).AdminExecutorServices.end())
	{
		nlwarning("couldn't execute a command on this service, aesid %d doesn't exist", aesid);
		return false;
	}

	uint32 sid = atoi(args[2].c_str());
	SIT sit = (*aesit).findService (sid, false);
	if (sit == (*aesit).Services.end())
	{
		nlwarning("couldn't execute a command on this service, sid %d doesn't exist", sid);
		return false;
	}

	if (!(*sit).Connected || !(*sit).Ready)
	{
		nlwarning("couldn't execute a command on this service, the service %d is not connected or not ready", sid);
		return false;
	}


	string command;
	for (uint32 i = 3; i < args.size(); i++)
		command += args[i] + " ";

	CMessage msgout (CNetManager::getSIDA((*asit).ASAddr), "EXEC_COMMAND");
	msgout.serial (aesid);
	msgout.serial (sid);
	msgout.serial (command);
	CNetManager::send ((*asit).ASAddr, msgout);
	return true;
}


NLMISC_COMMAND (sys, "execute a system command", "<asid> <aesid> <command>")
{
	if(args.size() != 3) return false;

	uint32 asid = atoi(args[0].c_str());
	ASIT asit = findAdminService (asid, false);
	if (asit == AdminServices.end())
	{
		nlwarning("couldn't execute system command, asid %d doesn't exist", asid);
		return false;
	}

	uint32 aesid = atoi(args[1].c_str());
	AESIT aesit = (*asit).findAdminExecutorService (aesid, false);
	if (aesit == (*asit).AdminExecutorServices.end())
	{
		nlwarning("couldn't execute system command, aesid %d doesn't exist", aesid);
		return false;
	}

	CMessage msgout (CNetManager::getSIDA((*asit).ASAddr), "SYS");
	msgout.serial (aesid);
	msgout.serial (const_cast<string &>(args[2]));
	CNetManager::send ((*asit).ASAddr, msgout);

	return true;
}

NLMISC_COMMAND (list, "display list of service", "")
{
	if(args.size() != 0) return false;

	displayServices ();
	
	return true;
}

NLMISC_COMMAND (startall_services, "start all services on a as", "<asid>")
{
	if(args.size() != 1) return false;

	uint32 asid = atoi(args[0].c_str());
	ASIT asit = findAdminService (asid, false);
	if (asit == AdminServices.end())
	{
		nlwarning ("couldn't run all services, asid %d doesn't exist", asid);
		return false;
	}

	if (!(*asit).Connected)
	{
		nlwarning ("couldn't run all service, asid %d is not connected", asid);
	}

	CMessage msgout (CNetManager::getSIDA((*asit).ASAddr), "START_ALL_SERVICES");
	CNetManager::send ((*asit).ASAddr, msgout);

	return true;
}

NLMISC_COMMAND (stopall_services, "stop all services on a as", "<asid>")
{
	if(args.size() != 1) return false;

	uint32 asid = atoi(args[0].c_str());
	ASIT asit = findAdminService (asid, false);
	if (asit == AdminServices.end())
	{
		nlwarning ("couldn't run all services, asid %d doesn't exist", asid);
		return false;
	}

	if (!(*asit).Connected)
	{
		nlwarning ("couldn't run all service, asid %d is not connected", asid);
	}

	CMessage msgout (CNetManager::getSIDA((*asit).ASAddr), "STOP_ALL_SERVICES");
	CNetManager::send ((*asit).ASAddr, msgout);

	return true;
}
