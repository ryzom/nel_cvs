/** \file admin_executor_service.cpp
 * Admin Executor Service (AES)
 *
 * $Id: admin_executor_service.cpp,v 1.17 2002/10/24 08:17:28 lecroart Exp $
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
#include "nel/misc/system_info.h"
#include "nel/misc/config_file.h"
#include "nel/misc/thread.h"
#include "nel/misc/command.h"
#include "nel/misc/path.h"

#include "nel/net/service.h"
#include "nel/net/unified_network.h"
#include "nel/net/varpath.h"

//
// Namespaces
//
 
using namespace std;
using namespace NLMISC;
using namespace NLNET;

//
// Structures
//

struct CRequest
{
	CRequest (uint32 id, uint16 sid) : Id(id), NbWaiting(0), NbReceived(0), SId(sid)
	{
		Time = CTime::getSecondsSince1970 ();
	}

	uint32			Id;
	uint			NbWaiting;
	uint32			NbReceived;
	uint16			SId;

	uint32			Time;	// when the request was ask (timeout in 4 secondes)

	vector<pair<vector<string>, vector<string> > > Answers;
};

struct CService
{
	CService() : Ready(false) { }

	string			AliasName;		/// alias of the service used in the AES and AS to find him (unique per AES)
	string			ShortName;		/// name of the service in short format ("NS" for example)
	string			LongName;		/// name of the service in long format ("naming_service")
	uint16			ServiceId;
	bool			Ready;			/// true if the service is ready
	vector<CSerialCommand>	Commands;

	vector<uint32>	WaitingRequestId;		/// contains all request that the server hasn't reply yet

	void init (const string &shortName, uint16 serviceId)
	{
		ShortName = shortName;
		ServiceId = serviceId;
	}

	void reset ()
	{
		ServiceId = 0;
		AliasName = "";
		ShortName = "";
		LongName = "";
		ServiceId = 0;
		Ready = false;
		Commands.clear ();
	}
};

//
// Variables
//

vector<CService> Services;
typedef vector<CService>::iterator SIT;

vector<CRequest> Requests;

//
// Request functions
//

void addRequestWaitingNb (uint32 request)
{
	for (uint i = 0 ; i < Requests.size (); i++)
	{
		if (Requests[i].Id == request)
		{
			Requests[i].NbWaiting++;
			return;
		}
	}
	nlstop;
}

void addRequestAnswer (uint32 rid, const vector<string> &variables, const vector<string> &values)
{
	for (uint i = 0 ; i < Requests.size (); i++)
	{
		if (Requests[i].Id == rid)
		{
			Requests[i].Answers.push_back (make_pair(variables, values));

			Requests[i].NbReceived++;

			return;
		}
	}
	// we received an unknown request, forget it
	nlwarning ("Receive an answer for unknown request %d", rid);
}

bool emptyRequest (uint32 rid)
{
	for (uint i = 0 ; i < Requests.size (); i++)
	{
		if (Requests[i].Id == rid && Requests[i].NbWaiting != 0)
		{
			return false;
		}
	}
	return true;
}

void cleanRequest ()
{
	uint32 currentTime = CTime::getSecondsSince1970 ();

	for (uint i = 0 ; i < Requests.size ();)
	{
		// timeout after 4 seconds
		if (currentTime >= Requests[i].Time+4)
		{
			nlwarning ("Request %d timeouted, only %d on %d services has replied", Requests[i].Id, Requests[i].NbReceived, Requests[i].NbWaiting);
			Requests[i].NbWaiting = Requests[i].NbReceived;
		}

		if (Requests[i].NbWaiting <= Requests[i].NbReceived)
		{
			// the request is over, send to the php

			CMessage msgout("VIEW");
			msgout.serial (Requests[i].Id);

			for (uint j = 0; j < Requests[i].Answers.size (); j++)
			{
				msgout.serialCont (Requests[i].Answers[j].first);
				msgout.serialCont (Requests[i].Answers[j].second);
			}

			if (Requests[i].SId == 0)
				nlinfo ("Receive a answer for the fake request %d", Requests[i].Id);
			else
				CUnifiedNetwork::getInstance ()->send (Requests[i].SId, msgout);

			// set to 0 to erase it
			Requests[i].NbWaiting = 0;
		}

		if (Requests[i].NbWaiting == 0)
		{
			Requests.erase (Requests.begin ()+i);
		}
		else
		{
			i++;
		}
	}
}

//
// Functions
//

SIT findService (uint32 sid, bool asrt = true)
{
	SIT sit;
	for (sit = Services.begin(); sit != Services.end(); sit++)
	{
		if ((*sit).ServiceId == sid) break;
	}
	if (asrt)
		nlassert (sit != Services.end());
	return sit;
}

void addRequest (uint32 rid, const string &rawvarpath, uint16 sid)
{
	nlinfo ("addRequest from %hu: '%s'", sid, rawvarpath.c_str ());

	string str;
	CLog logDisplayVars;
	CMemDisplayer mdDisplayVars;
	logDisplayVars.addDisplayer (&mdDisplayVars);

	CVarPath varpath (rawvarpath);

	// add the request
	Requests.push_back (CRequest(rid, sid));

	for (uint i = 0; i < varpath.Destination.size (); i++)
	{
		string service = varpath.Destination[i].first;

		if (service == "*")
		{
			for (uint j = 0; j < Services.size (); j++)
			{
				if (Services[j].ServiceId != 0)
				{
					addRequestWaitingNb (rid);

					Services[j].WaitingRequestId.push_back (rid);
					CMessage msgout("GET_VIEW");
					msgout.serial(rid);
					msgout.serial (varpath.Destination[i].second);
					CUnifiedNetwork::getInstance ()->send (Services[j].ServiceId, msgout);
					nlinfo ("Sent view '%s' to service '%s-%hu'", varpath.Destination[i].second.c_str(), Services[j].ShortName.c_str(), Services[j].ServiceId);
				}
			}
		}
		else
		{
			if (service.find ("AES") != string::npos)
			{
/*				// special case, the service is me!

				CVarPath subvarpath(varpath.Destination[i].second);

				CMessage msgout("VIEW");
				msgout.serial(rid);

				vector<string> vara;
				vector<string> vala;

				// add default row
				vara.push_back ("service");
				vala.push_back ("AES");

				for (uint j = 0; j < subvarpath.Destination.size (); j++)
				{
					mdDisplayVars.clear ();
					ICommand::execute(subvarpath.Destination[j].first, logDisplayVars, true);
					const std::deque<std::string>	&strs = mdDisplayVars.lockStrings();
					if (strs.size()>0)
					{
						string s_ = strs[0];

						uint32 pos = strs[0].find("=");
						if(pos != string::npos && pos + 2 < strs[0].size())
						{
							uint32 pos2 = string::npos;
							if(strs[0][strs[0].size()-1] == '\n')
								pos2 = strs[0].size() - pos - 2 - 1;

							str = strs[0].substr (pos+2, pos2);
						}
						else
						{
							str = "???";
						}
					}
					else
					{
						str = "???";
					}
					mdDisplayVars.unlockStrings();

					vara.push_back(subvarpath.Destination[j].first);
					vala.push_back (str);
					nlinfo ("Add to result view '%s' = '%s'", subvarpath.Destination[j].first.c_str(), str.c_str());
				}

				msgout.serial (vara);
				msgout.serial (vala);

				CUnifiedNetwork::getInstance ()->send (sid, msgout);
				nlinfo ("Sent result view to service '%s-%hu'", serviceName.c_str(), sid);*/
			}
			else
			{
				uint pos = service.find ("-");
				if (pos == string::npos)
				{
					for (uint j = 0; j < Services.size (); j++)
					{
						if (Services[j].ServiceId != 0 && Services[j].ShortName == service)
						{
							addRequestWaitingNb (rid);

							Services[j].WaitingRequestId.push_back (rid);
							CMessage msgout("GET_VIEW");
							msgout.serial(rid);
							msgout.serial (varpath.Destination[i].second);
							CUnifiedNetwork::getInstance ()->send (Services[j].ServiceId, msgout);
							nlinfo ("Sent view '%s' to service '%s-%hu'", varpath.Destination[i].second.c_str(), Services[j].ShortName.c_str(), Services[j].ServiceId);
						}
					}
				}
				else
				{
					SIT sit = findService (atoi(service.substr(pos+1).c_str()), false);
					if (sit == Services.end ())
					{
						nlwarning ("Service %s is not found in the list", service.c_str ());
					}
					else
					{
						addRequestWaitingNb (rid);
						
						(*sit).WaitingRequestId.push_back (rid);
						CMessage msgout("GET_VIEW");
						msgout.serial(rid);
						msgout.serial (varpath.Destination[i].second);
						CUnifiedNetwork::getInstance ()->send ((*sit).ServiceId, msgout);
						nlinfo ("Sent view '%s' to service '%s-%hu'", varpath.Destination[i].second.c_str(), (*sit).ShortName.c_str(), (*sit).ServiceId);
					}
				}
			}			
		}
	}

	if (emptyRequest(rid))
	{
		// send an empty string to say to php that there's nothing
		CMessage msgout("GET_VIEW");
		msgout.serial(rid);
		CUnifiedNetwork::getInstance ()->send (sid, msgout);
	}
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

static void cbServiceIdentification (CMessage &msgin, const std::string &serviceName, uint16 sid)
{
/*	if (sid >= Services.size ())
	{
		nlwarning ("Identification of an unknown service %s-%hu", serviceName.c_str(), sid);
		return;
	}

	if (Services[sid].Id == 0)
	{
		nlwarning ("Identification of an unknown service %s-%hu", serviceName.c_str(), sid);
		return;
	}

	nlinfo ("*:*:%d is identified to be '%s' '%s-%hu' '%s'", Services[sid].Id, Services[sid].AliasName.c_str(), Services[sid].ShortName.c_str(), Services[sid].ServiceId, Services[sid].LongName.c_str());
*/
/*	CService *s = (CService*) (uint) from->appId();

	msgin.serial (s->AliasName, s->ShortName, s->LongName);
	msgin.serialCont (s->Commands);

	nlinfo ("*:*:%d is identified to be '%s' '%s' '%s'", s->Id, s->AliasName.c_str(), s->ShortName.c_str(), s->LongName.c_str());

	// broadcast the message to the admin service
	CMessage msgout (CNetManager::getSIDA ("AESAS"), "SID");
	msgout.serial (s->Id, s->AliasName, s->ShortName, s->LongName);
	msgout.serialCont (s->Commands);
	CNetManager::send ("AESAS", msgout);*/
}

static void cbServiceReady /*(CMessage& msgin, TSockId from, CCallbackNetBase &netbase)*/(CMessage &msgin, const std::string &serviceName, uint16 sid)
{
/*	if (sid >= Services.size ())
	{
		nlwarning ("Ready of an unknown service %s-%hu", serviceName.c_str(), sid);
		return;
	}

	if (Services[sid].Id == 0)
	{
		nlwarning ("Ready of an unknown service %s-%hu", serviceName.c_str(), sid);
		return;
	}

	nlinfo ("*:*:%d is ready (%s-%hu)", Services[sid].Id, Services[sid].ShortName.c_str (), Services[sid].ServiceId);
	Services[sid].Ready = true;
*/
/*	CService *s = (CService*) (uint) from->appId();

	nlinfo ("*:*:%d is ready", s->Id);
	s->Ready = true;

	// broadcast the message to the admin service
	CMessage msgout (CNetManager::getSIDA ("AESAS"), "SR");
	msgout.serial (s->Id);
	CNetManager::send ("AESAS", msgout);*/
}

static void cbLog /*(CMessage& msgin, TSockId from, CCallbackNetBase &netbase)*/(CMessage &msgin, const std::string &serviceName, uint16 sid)
{

/*	CService *s = (CService*) (uint) from->appId();
	// received an answer for a command, give it to the AS

	// broadcast the message to the admin service
	CMessage msgout (CNetManager::getSIDA ("AESAS"), "XLOG");
	string log;
	msgin.serial (log);
	msgout.serial (s->Id);
	msgout.serial (log);
	CNetManager::send ("AESAS", msgout);*/
}

static void cbView (CMessage &msgin, const std::string &serviceName, uint16 sid)
{
	// receive an view answer from the service
	uint32 rid;
	vector<string> vara;
	vector<string> vala;

	msgin.serial (rid);

	msgin.serialCont (vara);
	msgin.serialCont (vala);

	SIT sit = findService (sid);
	remove ((*sit).WaitingRequestId.begin (), (*sit).WaitingRequestId.end (), rid);

	addRequestAnswer (rid, vara, vala);
}


static void cbGetView (CMessage &msgin, const std::string &serviceName, uint16 sid)
{
	uint32 rid;
	string rawvarpath;

	msgin.serial (rid);
	msgin.serial (rawvarpath);

	addRequest (rid, rawvarpath, sid);
}

void serviceConnection /*(const string &serviceName, TSockId from, void *arg)*/(const std::string &serviceName, uint16 sid, void *arg)
{
	// don't add AS
	if (serviceName == "AS")
		return;

	if (sid >= Services.size ())
	{
		Services.resize (sid+1);
	}

	Services[sid].init (serviceName, sid);

	nlinfo ("%s-%hu connected", Services[sid].ShortName.c_str (), Services[sid].ServiceId);

/*	Services.push_back (CService (from));
	CService *s = &(Services.back());
	from->setAppId ((uint64)(uint)s);

	nlinfo ("*:*:%d connected", s->Id);
	
	// broadcast the message to the admin service
	CMessage msgout (CNetManager::getSIDA ("AESAS"), "SC");
	msgout.serial (s->Id);
	CNetManager::send ("AESAS", msgout);
*/}

void serviceDisconnection /*(const string &serviceName, TSockId from, void *arg)*/(const std::string &serviceName, uint16 sid, void *arg)
{
	// don't remove AS
	if (serviceName == "AS")
		return;

	if (sid >= Services.size ())
	{
		nlwarning ("Disconnection of an unknown service %s-%hu", serviceName.c_str(), sid);
		return;
	}

	if (Services[sid].ServiceId == 0)
	{
		nlwarning ("Disconnection of an unknown service %s-%hu", serviceName.c_str(), sid);
		return;
	}

	// we need to remove pending request

	for(uint i = 0; i < Services[sid].WaitingRequestId.size (); i++)
	{
		Requests[Services[sid].WaitingRequestId[i]].NbWaiting--;
	}

	nlinfo ("%s-%hu disconnected", Services[sid].ShortName.c_str (), Services[sid].ServiceId);

	Services[sid].reset();

/*	CService *s = (CService*) (uint) from->appId();


	// broadcast the message to the admin service
	CMessage msgout (CNetManager::getSIDA ("AESAS"), "SD");
	msgout.serial (s->Id);
	CNetManager::send ("AESAS", msgout);

	// remove the service from the list
	Services.erase (findService(s->Id));
*/}


/** Callback Array
 */
TUnifiedCallbackItem CallbackArray[] =
{
	{ "SID", cbServiceIdentification },
	{ "SR", cbServiceReady },
	{ "LOG", cbLog },

	{ "GET_VIEW", cbGetView },
	{ "VIEW", cbView },
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////// CONNECTION TO THE AS ////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
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
*/


void ASConnection (const string &serviceName, uint16 sid, void *arg)
{
	nlinfo ("Connected to %s-%hu", serviceName.c_str (), sid);
}

static void ASDisconnection (const string &serviceName, uint16 sid, void *arg)
{
	nlinfo ("Disconnected to %s-%hu", serviceName.c_str (), sid);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////// SERVICE IMPLEMENTATION //////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

/*void loadAndSendServicesAliasList (CConfigFile::CVar &services)
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
*/

class CAdminExecutorService : public IService
{
public:

	/// Init the service
	void		init ()
	{
		// be warn when a new service comes
		CUnifiedNetwork::getInstance()->setServiceUpCallback ("*", serviceConnection, NULL);
		CUnifiedNetwork::getInstance()->setServiceDownCallback ("*", serviceDisconnection, NULL);

		// add connection to the admin service
		CUnifiedNetwork::getInstance()->setServiceUpCallback ("AS", ASConnection, NULL);
		CUnifiedNetwork::getInstance()->setServiceDownCallback ("AS", ASDisconnection, NULL);
		//CUnifiedNetwork::getInstance()->addCallbackArray (ASCallbackArray, sizeof(ASCallbackArray)/sizeof(ASCallbackArray[0]));

		string ASHost = ConfigFile.getVar ("ASHost").asString ();
		if (ASHost.find (":") == string::npos)
			ASHost += ":49996";

		CUnifiedNetwork::getInstance()->addService ("AS", CInetAddress(ASHost));

/*		CNetManager::setConnectionCallback ("AES", serviceConnection, NULL);
		CNetManager::setDisconnectionCallback ("AES", serviceDisconnection, NULL);

		// install the server for AS
		CNetManager::setConnectionCallback ("AESAS", cbASServiceConnection, NULL);
		CNetManager::addServer ("AESAS", 49996);
		CNetManager::addCallbackArray ("AESAS", AESASCallbackArray, sizeof(AESASCallbackArray)/sizeof(AESASCallbackArray[0]));
*/
//		ConfigFile.setCallback ("Services", loadAndSendServicesAliasList);
//		loadAndSendServicesAliasList (IService::ConfigFile.getVar ("Services"));
	}

	bool		update ()
	{
		cleanRequest ();

		return true;
	}
};


/// Naming Service
NLNET_SERVICE_MAIN (CAdminExecutorService, "AES", "admin_executor_service", 49997, CallbackArray, NELNS_CONFIG, NELNS_LOGS);

uint32 toto = 123, tata = 5456;

NLMISC_VARIABLE(uint32, toto, "test the get view system");
NLMISC_VARIABLE(uint32, tata, "test the get view system");

NLMISC_COMMAND (getView, "send a view and receive an array as result", "<varpath>")
{
	if(args.size() != 1) return false;

	addRequest (0, args[0], 0);

	return true;
}

NLMISC_COMMAND (clearRequests, "clear all pending requests", "")
{
	if(args.size() != 0) return false;

	// for all request, set the NbWaiting to NbReceived, next cleanRequest() will send answer and clear all request
	for (uint i = 0 ; i < Requests.size (); i++)
	{
		if (Requests[i].NbWaiting <= Requests[i].NbReceived)
		{
			Requests[i].NbWaiting = Requests[i].NbReceived;
		}
	}

	return true;
}

NLMISC_COMMAND (displayRequests, "display all pending requests", "")
{
	if(args.size() != 0) return false;

	log.displayNL ("Display %d pending requests", Requests.size ());
	for (uint i = 0 ; i < Requests.size (); i++)
	{
		log.displayNL ("id: %d wait: %d recv: %d sid: %hu", Requests[i].Id, Requests[i].NbWaiting, Requests[i].NbReceived, Requests[i].SId);
	}
	log.displayNL ("End of display pending requests");

	return true;
}
