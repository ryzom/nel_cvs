/** \file admin_executor_service.cpp
 * Admin Executor Service (AES)
 *
 * $Id: admin_executor_service.cpp,v 1.2 2001/05/02 12:36:39 lecroart Exp $
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

#include <io.h>
#include <process.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <string>
#include <list>

#include "nel/misc/debug.h"
#include "nel/misc/config_file.h"
#include "nel/misc/thread.h"

#include "nel/net/service.h"
#include "nel/net/net_manager.h"

 
using namespace std;
using namespace NLMISC;
using namespace NLNET;



struct CService
{
	CService(TSockId s) : ServiceSockId(s), ServiceId(NextServiceId++) { }

	TSockId	ServiceSockId;	// socket number to communicate with
	uint32	ServiceId;		// id to identify it
	string	ShortName;
	string	LongName;

private:

	static	uint32 NextServiceId;
};

uint32 CService::NextServiceId = 0;

list<CService> Services;
typedef list<CService>::iterator sit;

sit find (TSockId sid)
{
	sit it;
	for (it = Services.begin(); it != Services.end(); it++)
	{
		if ((*it).ServiceSockId == sid) break;
	}
	return it;
}

sit find (uint32 sid)
{
	sit it;
	for (it = Services.begin(); it != Services.end(); it++)
	{
		if ((*it).ServiceId == sid) break;
	}
	return it;
}


class CExecuteCommandThread : public IRunnable
{
public:
	string Command;

	CExecuteCommandThread (string command) : Command(command) { }

	void run ()
	{
		nlinfo ("start executing: %s", Command.c_str());
		system (Command.c_str());
		nlinfo ("end executing: %s", Command.c_str());
	}
};


void executeCommand (string command, bool background)
{
	if (command.empty()) return;

/*
	nlinfo ("start executing: %s", command.c_str());
	if (command[command.size()-1] == '&')
	{
		command.resize(command.size()-2);

		if (spawnlp (_P_NOWAIT, cmd.c_str(), command.c_str(), NULL) == -1)
		{
			perror ("ca chie grave!!!!: ");
		}
	}
	else
	{
		if (spawnlp (_P_WAIT, cmd.c_str(), command.c_str(), NULL) == -1)
		{
			perror ("ca chie grave!!!!: ");
		}
	}
	nlinfo ("end executing: %s", command.c_str());
*/

	command += " >NUL:";
	if (background)
	{
		IThread *thread = IThread::create (new CExecuteCommandThread (command));
		thread->start ();
	}
	else
	{
		CExecuteCommandThread cmdt (command);
		cmdt.run ();
	}
}


/*
// execute without 
void executeCommand (string command, TSockId from, CCallbackNetBase &netbase)
{
	if (command.empty()) return;

	#define STDOUT 1
	#define STDERR 2
	int nul, oldstdout, oldstderr;
	char *tmpfilename = tmpnam (NULL);
	nul = _open(tmpfilename, _O_RDWR | _O_CREAT | _O_TRUNC | _O_TEMPORARY | _O_SHORT_LIVED | _O_EXCL, _S_IREAD | _S_IWRITE);
	oldstdout = _dup(STDOUT);
	oldstderr = _dup(STDERR);
	_dup2(nul, STDOUT);
	_dup2(nul, STDERR);
	system(command.c_str());
	_dup2(oldstdout, STDOUT);
	_dup2(oldstderr, STDERR);
	_close(oldstdout);
	_close(oldstderr);

	_lseek (nul, 0L, SEEK_SET);

	while (!_eof(nul))
	{
		uint8 buffer[10000];
		uint32 nbread = _read (nul, buffer, 10000);

		CMessage msgout (netbase.getSIDA(), "ESCR");
		msgout.serial (nbread);
		msgout.serialBuffer (buffer, nbread);
		netbase.send (msgout, from);
	}
	
	_close(nul);
*/
/*
	FILE *fp = fopen ("test.txt", "r");
	do
		{
		char str[1024];
		fgets (str, 1024, fp);
		if (feof(fp)) break;
		result.push_back (str);
	}
	while (true);
	fclose (fp);
//	remove ("test.txt");
*///}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////// CONNECTION TO THE SERVICES //////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void cbServiceIdentification (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	CService *c = (CService*) from->appId();

	msgin.serial (c->ShortName);
	msgin.serial (c->LongName);

	nlinfo ("%s %s %s is identified", from->asString().c_str(), c->ShortName.c_str(), c->LongName.c_str());

	CMessage msgout (CNetManager::getSIDA ("AESAS"), "SID");
	msgout.serial (c->ShortName);
	msgout.serial (c->LongName);
	CNetManager::send ("AESAS", msgout);
}

static void cbServiceReady (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	CService *c = (CService*) from->appId();

	nlinfo ("%s %s %s is ready", from->asString().c_str(), c->ShortName.c_str(), c->LongName.c_str());

	CMessage msgout (CNetManager::getSIDA ("AESAS"), "SR");
	msgout.serial (c->ShortName);
	msgout.serial (c->LongName);
	CNetManager::send ("AESAS", msgout);
}

void serviceConnection (const string &serviceName, TSockId from, void *arg)
{
	Services.push_back (CService(from));
	CService *c = &(Services.back());
	from->setAppId ((uint64)c);

	nlinfo ("%s is connected", from->asString().c_str());
	
	CMessage msgout (CNetManager::getSIDA ("AESAS"), "SC");
	CNetManager::send ("AESAS", msgout);
}

void serviceDisconnection (const string &serviceName, TSockId from, void *arg)
{
	CService *c = (CService*) from->appId();

	nlinfo ("%s %s %s is disconnected", from->asString().c_str(), c->ShortName.c_str(), c->LongName.c_str());

	CMessage msgout (CNetManager::getSIDA ("AESAS"), "SD");
	msgout.serial (c->ShortName);
	msgout.serial (c->LongName);
	CNetManager::send ("AESAS", msgout);
}


/** Callback Array
 */
TCallbackItem ServicesCallbackArray[] =
{
	{ "SID", cbServiceIdentification },
	{ "SR", cbServiceReady },
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////// CONNECTION TO THE AS ////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void cbExecuteSystemCommand (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	string systemCommand;
	uint8 background;
	msgin.serial (systemCommand);
	msgin.serial (background);

	nlinfo ("I have to execute '%s'", systemCommand.c_str());

	executeCommand (systemCommand, background==1);
}

static void cbStopService (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	string service;

	nlinfo ("I have to stop service");

}


TCallbackItem AESASCallbackArray[] =
{
	{ "ESC", cbExecuteSystemCommand },
	{ "SS", cbStopService },
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////// SERVICE IMPLEMENTATION //////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

class CAdminExecutorService : public IService
{
public:

	/// Init the service, load the universal time.
	void		init ()
	{
		CNetManager::setConnectionCallback ("AES", serviceConnection, NULL);
		CNetManager::setDisconnectionCallback ("AES", serviceDisconnection, NULL);

		// install the server for AS
		CNetManager::addServer ("AESAS", 49996);
		CNetManager::addCallbackArray ("AESAS", AESASCallbackArray, sizeof(AESASCallbackArray)/sizeof(AESASCallbackArray[0]));
	}

	bool		update ()
	{
		return true;
	}
};


/// Naming Service
NLNET_SERVICE_MAIN (CAdminExecutorService, "AES", "admin_executor_service", 49997, ServicesCallbackArray);
