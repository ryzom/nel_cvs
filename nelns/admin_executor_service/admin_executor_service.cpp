/** \file admin_executor_service.cpp
 * Admin Executor Service (AES)
 *
 * $Id: admin_executor_service.cpp,v 1.15 2002/04/09 12:29:15 lecroart Exp $
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

#include <fcntl.h>
#include <sys/stat.h>

#ifdef NL_OS_WINDOWS
#include <windows.h>
#include <direct.h>
#else
#include <unistd.h>
#endif

#include <string>
#include <list>

#include "nel/misc/debug.h"
#include "nel/misc/config_file.h"
#include "nel/misc/thread.h"
#include "nel/misc/command.h"

#include "nel/net/service.h"
#include "nel/net/net_manager.h"

/*#ifdef NL_OS_WINDOWS
#define getcwd _getcwd
#define chdir _chdir
#endif
*/
 
using namespace std;
using namespace NLMISC;
using namespace NLNET;



struct CService
{
	CService(TSockId s) : SockId(s), Id(NextId++), Ready(false) { }

	TSockId			SockId;			/// connection to the service
	uint32			Id;				/// uint32 to identify the service
	string			AliasName;		/// alias of the service used in the AES and AS to find him (unique per AES)
	string			ShortName;		/// name of the service in short format ("NS" for example)
	string			LongName;		/// name of the service in long format ("naming_service")
	bool			Ready;			/// true if the service is ready
	vector<CSerialCommand>	Commands;

private:
	static	uint32 NextId;
};

uint32 CService::NextId = 1;

list<CService> Services;
typedef list<CService>::iterator SIT;

SIT find (TSockId sid)
{
	SIT sit;
	for (sit = Services.begin(); sit != Services.end(); sit++)
	{
		if ((*sit).SockId == sid) break;
	}
	return sit;
}

SIT findService (uint32 sid, bool asrt = true)
{
	SIT sit;
	for (sit = Services.begin(); sit != Services.end(); sit++)
	{
		if ((*sit).Id == sid) break;
	}
	if (asrt)
		nlassert (sit != Services.end());
	return sit;
}


class CExecuteCommandThread : public IRunnable
{
public:
	string Command;

	CExecuteCommandThread (string command) : Command(command) { }

	void run ()
	{
		nlinfo ("start executing '%s'", Command.c_str());
		
		system (Command.c_str());
		
		nlinfo ("end executing: %s", Command.c_str());
	}
};

class CExecuteServiceThread : public IRunnable
{
public:
	string ServiceCommand;

	CExecuteServiceThread (string serviceCommand) :
		ServiceCommand(serviceCommand) { }

	void run ()
	{
		nlinfo ("start service '%s'", ServiceCommand.c_str());
		
#ifdef NL_OS_WINDOWS
		WinExec (ServiceCommand.c_str(), SW_MINIMIZE/*SW_SHOWNORMAL*/);
#else
		system (ServiceCommand.c_str());
#endif

		nlinfo ("end service '%s'", ServiceCommand.c_str());
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

#ifdef NL_OS_WINDOWS
	command += " >NUL:";
#else
	command += " >/dev/null";
#endif
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
	CService *s = (CService*) (uint) from->appId();

	msgin.serial (s->AliasName, s->ShortName, s->LongName);
	msgin.serialCont (s->Commands);

	nlinfo ("*:*:%d is identified to be '%s' '%s' '%s'", s->Id, s->AliasName.c_str(), s->ShortName.c_str(), s->LongName.c_str());

	// broadcast the message to the admin service
	CMessage msgout (CNetManager::getSIDA ("AESAS"), "SID");
	msgout.serial (s->Id, s->AliasName, s->ShortName, s->LongName);
	msgout.serialCont (s->Commands);
	CNetManager::send ("AESAS", msgout);
}

static void cbServiceReady (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	CService *s = (CService*) (uint) from->appId();

	nlinfo ("*:*:%d is ready", s->Id);
	s->Ready = true;

	// broadcast the message to the admin service
	CMessage msgout (CNetManager::getSIDA ("AESAS"), "SR");
	msgout.serial (s->Id);
	CNetManager::send ("AESAS", msgout);
}

static void cbLog (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	CService *s = (CService*) (uint) from->appId();
	// received an answer for a command, give it to the AS

	// broadcast the message to the admin service
	CMessage msgout (CNetManager::getSIDA ("AESAS"), "XLOG");
	string log;
	msgin.serial (log);
	msgout.serial (s->Id);
	msgout.serial (log);
	CNetManager::send ("AESAS", msgout);
}

void serviceConnection (const string &serviceName, TSockId from, void *arg)
{
	Services.push_back (CService (from));
	CService *s = &(Services.back());
	from->setAppId ((uint64)(uint)s);

	nlinfo ("*:*:%d connected", s->Id);
	
	// broadcast the message to the admin service
	CMessage msgout (CNetManager::getSIDA ("AESAS"), "SC");
	msgout.serial (s->Id);
	CNetManager::send ("AESAS", msgout);
}

void serviceDisconnection (const string &serviceName, TSockId from, void *arg)
{
	CService *s = (CService*) (uint) from->appId();

	nlinfo ("*:*:%d disconnected", s->Id);

	// broadcast the message to the admin service
	CMessage msgout (CNetManager::getSIDA ("AESAS"), "SD");
	msgout.serial (s->Id);
	CNetManager::send ("AESAS", msgout);

	// remove the service from the list
	Services.erase (findService(s->Id));
}


/** Callback Array
 */
TCallbackItem ServicesCallbackArray[] =
{
	{ "SID", cbServiceIdentification },
	{ "SR", cbServiceReady },
	{ "LOG", cbLog },
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////// CONNECTION TO THE AS ////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void errorMessage(string message, TSockId from)
{
	CMessage msgout (CNetManager::getSIDA ("AESAS"), "ERR");
	msgout.serial (message);
	CNetManager::send ("AESAS", msgout, from);
}

static void cbExecuteSystemCommand (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	string command;

	msgin.serial (command);

	IThread *thread = IThread::create (new CExecuteCommandThread (command));
	thread->start ();
}

static void cbStartService (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	string serviceAlias, command, path;
	msgin.serial (serviceAlias);

	nlinfo ("Starting the service alias '%s'", serviceAlias.c_str());

	try
	{
		path = IService::getInstance()->ConfigFile.getVar(serviceAlias).asString(0);
		command = IService::getInstance()->ConfigFile.getVar(serviceAlias).asString(1);
	}
	catch(EConfigFile &e)
	{
		nlwarning ("error in serviceAlias '%s' in config file (%s)", serviceAlias.c_str(), e.what());
		return;
	}

	// give the service alias to the service to forward it back when it will connected to the aes.
	command += " -N";
	command += serviceAlias;

	// set the path for the config file
	command += " -C";
	command += path;

	// set the path for log
	command += " -L";
	command += path;

	// set the path for running
	command += " -A";
	command += path;

#ifdef NL_OS_WINDOWS
	command += " >NUL:";
#else
	command += " >/dev/null";
#endif

	IThread *thread = IThread::create (new CExecuteServiceThread (command));
	thread->start ();
}

static void cbStopService (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	uint32 sid;

	msgin.serial (sid);

	nlinfo ("I have to stop service '%s'");

	SIT sit = findService (sid, false);
	if (sit == Services.end())
	{
		// don't find the aes, send an error message
		errorMessage ("couldn't stop service, aes didn't find the service", from);
		return;
	}

	CMessage msgout (CNetManager::getSIDA("AES"), "STOPS");
	CNetManager::send ("AES", msgout, (*sit).SockId);
}

static void cbExecCommand (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	uint32 sid;
	string command;

	msgin.serial (sid);
	msgin.serial (command);

	SIT sit = findService (sid, false);
	if (sit == Services.end())
	{
		// don't find the aes, send an error message
		errorMessage ("couldn't stop service, aes didn't find the service", from);
		return;
	}

	CMessage msgout (CNetManager::getSIDA("AES"), "EXEC_COMMAND");
	msgout.serial (command);
	CNetManager::send ("AES", msgout, (*sit).SockId);
}

void loadAndSendServicesAliasList (CConfigFile::CVar &var);

void cbASServiceConnection (const string &serviceName, TSockId from, void *arg)
{
	// new admin service, send him all out info about services

	nlinfo ("AS %s is connected", from->asString().c_str());
	
	CMessage msgout (CNetManager::getSIDA ("AESAS"), "SL");
	uint32 nbs = (uint32)Services.size();
	msgout.serial (nbs);
	for (SIT sit = Services.begin(); sit != Services.end(); sit++)
	{
		msgout.serial ((*sit).Id, (*sit).AliasName, (*sit).ShortName, (*sit).LongName, (*sit).Ready);
		msgout.serialCont ((*sit).Commands);
	}
	CNetManager::send ("AESAS", msgout, from);

	loadAndSendServicesAliasList (IService::getInstance()->ConfigFile.getVar ("Services"));
}

TCallbackItem AESASCallbackArray[] =
{
	{ "SYS", cbExecuteSystemCommand },
	{ "STARTS", cbStartService },
	{ "STOPS", cbStopService },
	{ "EXEC_COMMAND", cbExecCommand },
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////// SERVICE IMPLEMENTATION //////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void loadAndSendServicesAliasList (CConfigFile::CVar &services)
{
	vector<string> servicesaliaslist;
	for (sint i = 0 ; i < services.size (); i++)
	{
		servicesaliaslist.push_back (services.asString(i));
	}

	CMessage msgout2 (CNetManager::getSIDA ("AESAS"), "SAL");
	msgout2.serialCont (servicesaliaslist);
	CNetManager::send ("AESAS", msgout2, 0);
}


class CAdminExecutorService : public IService
{
public:

	/// Init the service, load the universal time.
	void		init ()
	{
		CNetManager::setConnectionCallback ("AES", serviceConnection, NULL);
		CNetManager::setDisconnectionCallback ("AES", serviceDisconnection, NULL);

		// install the server for AS
		CNetManager::setConnectionCallback ("AESAS", cbASServiceConnection, NULL);
		CNetManager::addServer ("AESAS", 49996);
		CNetManager::addCallbackArray ("AESAS", AESASCallbackArray, sizeof(AESASCallbackArray)/sizeof(AESASCallbackArray[0]));

		ConfigFile.setCallback ("Services", loadAndSendServicesAliasList);
		loadAndSendServicesAliasList (IService::ConfigFile.getVar ("Services"));
	}

	bool		update ()
	{
		return true;
	}
};


/// Naming Service
NLNET_OLD_SERVICE_MAIN (CAdminExecutorService, "AES", "admin_executor_service", 49997, ServicesCallbackArray, NELNS_CONFIG, NELNS_LOGS);
