/** \file admin_executor_service.cpp
 * Admin Executor Service (AES)
 *
 * $Id: admin_executor_service.cpp,v 1.24 2002/12/19 10:45:36 lecroart Exp $
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
#include <sys/types.h>
#include <signal.h>
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
namespace NLNET
{
	void serviceGetView (uint32 rid, const string &rawvarpath, vector<pair<vector<string>, vector<string> > > &answer);
}

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
	uint32			Time;	// when the request was ask

	vector<pair<vector<string>, vector<string> > > Answers;
};

struct CService
{
	CService() { reset (); }

	string			AliasName;		/// alias of the service used in the AES and AS to find him (unique per AES)
									/// If alias name is not empty, it means that the service was registered

	string			ShortName;		/// name of the service in short format ("NS" for example)
	string			LongName;		/// name of the service in long format ("naming_service")
	uint16			ServiceId;		/// service id of the service.
	bool			Ready;			/// true if the service is ready
	bool			Connected;		/// true if the service is connected to the AES
	vector<CSerialCommand>	Commands;
	bool			AutoReconnect;	/// true means that AES must relaunch the service if lost
	uint32			PId;			/// process Id used to kill the application

	vector<uint32>	WaitingRequestId;		/// contains all request that the server hasn't reply yet

	string toString ()
	{
		string res;
		if (!AliasName.empty())
		{
			res = AliasName+"/";
		}
		if (!ShortName.empty())
		{
			res += ShortName+"-";
		}
		res += NLMISC::toString(ServiceId);
		return res;
	}

	void init (const string &shortName, uint16 serviceId)
	{
		ShortName = shortName;
		ServiceId = serviceId;
		Connected = true;
	}

	void reset ()
	{
		AliasName = "";
		ShortName = "";
		LongName = "";
		ServiceId = 0;
		Ready = false;
		Connected = false;
		Commands.clear ();
		AutoReconnect = false;
		PId = 0;
		WaitingRequestId.clear ();
	}
};


//
// Variables
//

vector<CService> Services;
typedef vector<CService>::iterator SIT;

vector<CRequest> Requests;

vector<string> RegisteredServices;

const uint32 RequestTimeout = 5;	// in second

vector<pair<uint32, string> > WaitingToLaunchServices;	// date and alias name


//
// Launch services functions
//

// decode a service in a form 'alias/shortname-sid'
void decodeService (const string &name, string &alias, string &shortName, uint16 &sid)
{
	uint pos1 = 0, pos2 = 0;
	pos1 = name.find ("/");
	if (pos1 != string::npos)
	{
		alias = name.substr (0, pos1);
	}
	else
	{
		alias = "";
		pos1 = 0;
	}
	pos2 = name.find ("-");
	if (pos2 != string::npos)
	{
		shortName = name.substr (pos1+1,pos2-pos1);
		sid = atoi (name.substr (pos2+1).c_str());
	}
	else
	{
		shortName = name.substr (pos1,pos2-pos1);
		sid = 0;
	}

	if (alias.empty())
	{
		alias = shortName;
	}
}


// start a service immediatly
bool startService (const string &name)
{
	string command, path, arg;
	
	string alias, shortName;
	uint16 sid;
	decodeService (name, alias, shortName, sid);

	nlinfo ("Starting the service alias '%s'", alias.c_str());
	
	try
	{
		path = IService::getInstance()->ConfigFile.getVar(alias).asString(0);
		command = IService::getInstance()->ConfigFile.getVar(alias).asString(1);
	}
	catch(EConfigFile &e)
	{
		nlwarning ("Error in serviceAlias '%s' in config file (%s)", alias.c_str(), e.what());
		return false;
	}
	
	// give the service alias to the service to forward it back when it will connected to the aes.
	arg = " -N";
	arg += alias;
/* use the same path as -A for log and config
	// set the path for the config file
	arg += " -C";
	arg += path;
	
	// set the path for log
	arg += " -L";
	arg += path;
*/	
	// set the path for running
	arg += " -A";
	arg += path;
	
#ifdef NL_OS_WINDOWS
	arg += " >NUL:";
#else
	arg += " >/dev/null";
#endif
	
	bool res = launchProgram (command, arg);
	
	// if launching ok, leave 1 second to the new launching service before lauching next one
	if (res)
		nlSleep (1000);
	
	return res;
}



// start service in future
void startService (uint32 date, const string &serviceAlias)
{
	if (date <= CTime::getSecondsSince1970())
	{
		startService (serviceAlias);
	}
	else
	{
		for(uint i = 0; i < WaitingToLaunchServices.size(); i++)
		{
			if (WaitingToLaunchServices[i].second == serviceAlias)
			{
				nlwarning ("service %s already in waiting queue to launch", serviceAlias.c_str());
				return;
			}
		}

		WaitingToLaunchServices.push_back (make_pair(date, serviceAlias));
	}
}

void checkWaitingServices ()
{
	uint32 d = CTime::getSecondsSince1970();

	for(uint i = 0; i < WaitingToLaunchServices.size(); )
	{
		if (WaitingToLaunchServices[i].first <= d)
		{
			startService (WaitingToLaunchServices[i].second);
			WaitingToLaunchServices.erase (WaitingToLaunchServices.begin()+i);
		}
		else
		{
			i++;
		}
	}
}

//
// Request functions
//



void addRequestWaitingNb (uint32 rid)
{
	for (uint i = 0 ; i < Requests.size (); i++)
	{
		if (Requests[i].Id == rid)
		{
			Requests[i].NbWaiting++;
			// if we add a waiting, reset the timer
			Requests[i].Time = CTime::getSecondsSince1970 ();
			return;
		}
	}
	nlstop;
}

void subRequestWaitingNb (uint32 rid)
{
	for (uint i = 0 ; i < Requests.size (); i++)
	{
		if (Requests[i].Id == rid)
		{
			Requests[i].NbWaiting--;
			return;
		}
	}
	nlstop;
}

void addRequestAnswer (uint32 rid, const vector <pair<vector<string>, vector<string> > >&answer)
{
	for (uint i = 0 ; i < Requests.size (); i++)
	{
		if (Requests[i].Id == rid)
		{
			for (uint t = 0; t < answer.size(); t++)
			{
				nlassert (answer[t].first.size() == answer[t].second.size());
				Requests[i].Answers.push_back (make_pair(answer[t].first, answer[t].second));
			}
			Requests[i].NbReceived++;
			return;
		}
	}
	// we received an unknown request, forget it
	nlwarning ("Receive an answer for unknown request %d", rid);
}


void addRequestAnswer (uint32 rid, const vector<string> &variables, const vector<string> &values)
{
	nlassert (variables.size() == values.size());
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

	// just a debug check
	for (uint t = 0 ; t < Requests.size (); t++)
	{
		uint32 NbWaiting = Requests[t].NbWaiting;
		uint32 NbReceived = Requests[t].NbReceived;

		uint32 NbRef = 0;
		for (uint j = 0; j < Services.size(); j++)
		{
			if (Services[j].Connected)
			{
				for (uint k = 0; k < Services[j].WaitingRequestId.size(); k++)
				{
					if(Services[j].WaitingRequestId[k] == Requests[t].Id)
					{
						NbRef++;
					}
				}
			}
		}
		nlinfo ("Waiting request %d: NbRef %d NbWaiting %d NbReceived %d", Requests[t].Id, NbRef, NbWaiting, NbReceived);
		nlassert (NbRef == NbWaiting - NbReceived);
	}

	for (uint i = 0 ; i < Requests.size ();)
	{
		// timeout
		if (currentTime >= Requests[i].Time+RequestTimeout)
		{
			nlwarning ("Request %d timeouted, only %d on %d services have replied", Requests[i].Id, Requests[i].NbReceived, Requests[i].NbWaiting);

			vector<string> vara;
			vector<string> vala;
			
			vara.push_back ("service");
			for (uint j = 0; j < Services.size(); j++)
			{
				if (Services[j].Connected)
				{
					for (uint k = 0; k < Services[j].WaitingRequestId.size(); k++)
					{
						if(Services[j].WaitingRequestId[k] == Requests[i].Id)
						{
							// this services didn't answer
							string s;
							if(Services[j].AliasName.empty())
								s = Services[j].ShortName;
							else
								s = Services[j].AliasName;
							s += "-"+toString(Services[j].ServiceId);
							s += "<TIMEOUT>";
							vala.clear ();
							vala.push_back (s);
							addRequestAnswer (Requests[i].Id, vara, vala);
							break;
						}
					}
				}
			}
			nlassert (Requests[i].NbWaiting == Requests[i].NbReceived);
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
			{
				nlinfo ("Receive an answer for the fake request %d with %d answers", Requests[i].Id, Requests[i].Answers.size ());
				for (uint j = 0; j < Requests[i].Answers.size (); j++)
				{
					uint k;
					for (k = 0; k < Requests[i].Answers[j].first.size(); k++)
					{
						InfoLog->displayRaw ("%-10s", Requests[i].Answers[j].first[k].c_str());
					}
					InfoLog->displayRawNL("");
					for (k = 0; k < Requests[i].Answers[j].second.size(); k++)
					{
						InfoLog->displayRaw ("%-10s", Requests[i].Answers[j].second[k].c_str());
					}
					InfoLog->displayRawNL("");
					InfoLog->displayRawNL("-------------------------");
				}	
			}
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

SIT findService (const string &name)
{
	string alias, shortName;
	uint16 sid;
	decodeService(name, alias, shortName, sid);

	SIT sit;
	for (sit = Services.begin(); sit != Services.end(); sit++)
	{
		if ((*sit).AliasName == alias) break;
	}

	// not found in alias, try with short name
	if (sit == Services.end())
	{
		for (sit = Services.begin(); sit != Services.end(); sit++)
		{
			if ((*sit).ShortName == shortName) break;
		}
	}

	return sit;
}

bool isRegisteredService (const string &name)
{
	string alias, shortName;
	uint16 sid;
	decodeService(name, alias, shortName, sid);

	for (uint i = 0; i != RegisteredServices.size(); i++)
		if (RegisteredServices[i] == alias)
			return true;
	return false;
}


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
			// add services that I manage
			for (uint j = 0; j < Services.size (); j++)
			{
				if (Services[j].Connected)
				{
					addRequestWaitingNb (rid);

					// check if the command is not to stop the service
					CVarPath subvarpath(varpath.Destination[i].second);
					for (uint k = 0; k < subvarpath.Destination.size (); k++)
					{
						if (subvarpath.Destination[k].first == "State=0")
						{
							// If we stop the service, we don't have to reconnect the service
							Services[j].AutoReconnect = false;
						}
						else if (subvarpath.Destination[k].first == "State=-1")
						{
							Services[j].AutoReconnect = false;
#ifdef NL_OS_UNIX
							// kill the service
							kill (Services[j].PId, SIGKILL);
							send = false;
#endif // NL_OS_UNIX
						}
					}
					

					Services[j].WaitingRequestId.push_back (rid);
					CMessage msgout("GET_VIEW");
					msgout.serial(rid);
					msgout.serial (varpath.Destination[i].second);
					nlassert (Services[j].ServiceId);
					CUnifiedNetwork::getInstance ()->send (Services[j].ServiceId, msgout);
					nlinfo ("Sent view '%s' to service '%s'", varpath.Destination[i].second.c_str(), Services[j].toString ().c_str());
				}
			}

			// add myself
			addRequestWaitingNb (rid);
			
			vector<pair<vector<string>, vector<string> > > answer;
			
			serviceGetView (rid, varpath.Destination[i].second, answer);
			
			addRequestAnswer (rid, answer);
			nlinfo ("Sent and received view '%s' to my service '%s'", varpath.Destination[i].second.c_str(), service.c_str());
		}
		else if (service == "#")
		{
			uint j;
			// add registered services
			for (j = 0; j < RegisteredServices.size (); j++)
			{
				SIT sit = findService (RegisteredServices[j]);
				if (sit == Services.end())
				{
					// we fake a return value because we want all services, even if they are offline
					addRequestWaitingNb (rid);

					CVarPath subvarpath(varpath.Destination[i].second);

					vector<string> vara;
					vector<string> vala;
					
					// add default row
					vara.push_back ("service");
					vala.push_back (RegisteredServices[j]);
					
					for (uint k = 0; k < subvarpath.Destination.size (); k++)
					{
						uint pos = subvarpath.Destination[k].first.find("=");
						if (pos != string::npos)
							vara.push_back(subvarpath.Destination[k].first.substr(0, pos));
						else
							vara.push_back(subvarpath.Destination[k].first);

						string val = "???";
						// handle special case of non running service
						if (subvarpath.Destination[k].first == "State")
							val = "Offline";
						else if (subvarpath.Destination[k].first == "State=1")
						{
							// we want to start the service
							if (startService (RegisteredServices[j]))
								val = "Launching";
							else
								val = "Failed";
						}

						vala.push_back (val);
					}

					addRequestAnswer (rid, vara, vala);
					nlinfo ("Sent and received view '%s' to offline service '%s'", varpath.Destination[i].second.c_str(), RegisteredServices[j].c_str());
				}
				else
				{
					// send the request to the registered online service
					addRequestWaitingNb (rid);

					(*sit).WaitingRequestId.push_back (rid);
					CMessage msgout("GET_VIEW");
					msgout.serial(rid);
					msgout.serial (varpath.Destination[i].second);
					nlassert ((*sit).ServiceId);
					CUnifiedNetwork::getInstance ()->send ((*sit).ServiceId, msgout);
					nlinfo ("Sent view '%s' to service %s", varpath.Destination[i].second.c_str(), (*sit).toString ().c_str());
				}
			}

			// add services that are online but not registered
			for (j = 0; j < Services.size (); j++)
			{
				if (Services[j].Connected && Services[j].AliasName.empty())
				{
					addRequestWaitingNb (rid);
					
					Services[j].WaitingRequestId.push_back (rid);
					CMessage msgout("GET_VIEW");
					msgout.serial(rid);
					msgout.serial (varpath.Destination[i].second);
					nlassert (Services[j].ServiceId);
					CUnifiedNetwork::getInstance ()->send (Services[j].ServiceId, msgout);
					nlinfo ("Sent view '%s' to service %s", varpath.Destination[i].second.c_str(), Services[j].toString ().c_str());
				}
			}

			// add myself
			addRequestWaitingNb (rid);

			vector<pair<vector<string>, vector<string> > > answer;
			
			serviceGetView (rid, varpath.Destination[i].second, answer);
			addRequestAnswer (rid, answer);
			nlinfo ("Sent and received view '%s' to my service '%s'", varpath.Destination[i].second.c_str(), "AES");
		}
		else
		{
			if (service.find ("AES") != string::npos)
			{
				// it's for me, I don't send message to myself so i manage it right now
				addRequestWaitingNb (rid);
				
				vector<pair<vector<string>, vector<string> > > answer;
				
				serviceGetView (rid, varpath.Destination[i].second, answer);
				addRequestAnswer (rid, answer);
				nlinfo ("Sent and received view '%s' to my service '%s'", varpath.Destination[i].second.c_str(), service.c_str());
			}
			else
			{
/*				// send the request to the good service
				uint pos = service.find ("-");
				if (pos == string::npos)
				{
					bool found = false;
					for (uint j = 0; j < Services.size (); j++)
					{
						if (Services[j].Connected && Services[j].ShortName == service)
						{
							addRequestWaitingNb (rid);

							Services[j].WaitingRequestId.push_back (rid);
							CMessage msgout("GET_VIEW");
							msgout.serial(rid);
							msgout.serial (varpath.Destination[i].second);
							CUnifiedNetwork::getInstance ()->send (Services[j].ServiceId, msgout);
							nlinfo ("Sent view '%s' to service '%s-%hu'", varpath.Destination[i].second.c_str(), Services[j].ShortName.c_str(), Services[j].ServiceId);
							found = true;
						}
					}
				}
				else
				{
*/	
				SIT sit = findService (service);
				if (sit == Services.end ())
				{
					if (!isRegisteredService(service))
					{
						nlwarning ("Service %s is not online and not found in registered service list", service.c_str ());
					}
					else
					{
						addRequestWaitingNb (rid);
						
						CVarPath subvarpath(varpath.Destination[i].second);
						
						vector<string> vara;
						vector<string> vala;
						
						// add default row
						vara.push_back ("service");
						vala.push_back (service);
						
						for (uint k = 0; k < subvarpath.Destination.size (); k++)
						{
							uint pos = subvarpath.Destination[k].first.find("=");
							if (pos != string::npos)
								vara.push_back(subvarpath.Destination[k].first.substr(0, pos));
							else
								vara.push_back(subvarpath.Destination[k].first);
							
							string val = "???";
							// handle special case of non running service
							if (subvarpath.Destination[k].first == "State")
								val = "Offline";
							else if (subvarpath.Destination[k].first == "State=1")
							{
								// we want to start the service
								if (startService (service))
									val = "Launching";
								else
									val = "Failed";
							}
							
							vala.push_back (val);
						}
						
						addRequestAnswer (rid, vara, vala);
						nlinfo ("Sent and received view '%s' to offline service '%s'", varpath.Destination[i].second.c_str(), service.c_str());
					}
				}
				else
				{
					addRequestWaitingNb (rid);
					bool send = true;
					// check if the command is not to stop the service
					CVarPath subvarpath(varpath.Destination[i].second);
					for (uint k = 0; k < subvarpath.Destination.size (); k++)
					{
						if (subvarpath.Destination[k].first == "State=0")
						{
							// If we stop the service, we don't have to reconnect the service
							(*sit).AutoReconnect = false;
						}
						else if (subvarpath.Destination[k].first == "State=-1")
						{
							(*sit).AutoReconnect = false;
#ifdef NL_OS_UNIX
							// kill the service
							kill ((*sit).PId, SIGKILL);
							send = false;
#endif // NL_OS_UNIX
						}
					}
					
					if (send)
					{
						// now send the request to the service
						(*sit).WaitingRequestId.push_back (rid);
						CMessage msgout("GET_VIEW");
						msgout.serial(rid);
						msgout.serial (varpath.Destination[i].second);
						CService &cs = (*sit);
						nlassert ((*sit).ServiceId);
						CUnifiedNetwork::getInstance ()->send ((*sit).ServiceId, msgout);
						nlinfo ("Sent view '%s' to service %s", varpath.Destination[i].second.c_str(), (*sit).toString ().c_str());
					}
				}
			}			
		}
	}

/*	the send will be done automatically by cleanRequest()
	
	  
	if (emptyRequest(rid))
	{
		// send an empty string to say to php that there's nothing
		CMessage msgout("GET_VIEW");
		msgout.serial(rid);
		CUnifiedNetwork::getInstance ()->send (sid, msgout);
	}
	*/
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
	if (sid >= Services.size ())
	{
		nlwarning ("Identification of an unknown service %s-%hu", serviceName.c_str(), sid);
		return;
	}

	if (!Services[sid].Connected)
	{
		nlwarning ("Identification of an unknown service %s-%hu", serviceName.c_str(), sid);
		return;
	}

	msgin.serial (Services[sid].AliasName, Services[sid].LongName, Services[sid].PId);
	msgin.serialCont (Services[sid].Commands);

	// if there's an alias, it means that it s me that launch the services, autoreconnect it
	if (!Services[sid].AliasName.empty())
		Services[sid].AutoReconnect = true;

	nlinfo ("*:*:%d is identified to be '%s/%s-%hu' '%s'", Services[sid].ServiceId, Services[sid].AliasName.c_str(), Services[sid].ShortName.c_str(), Services[sid].ServiceId, Services[sid].LongName.c_str());
}

static void cbServiceReady /*(CMessage& msgin, TSockId from, CCallbackNetBase &netbase)*/(CMessage &msgin, const std::string &serviceName, uint16 sid)
{
	if (sid >= Services.size ())
	{
		nlwarning ("Ready of an unknown service %s-%hu", serviceName.c_str(), sid);
		return;
	}

	if (!Services[sid].Connected)
	{
		nlwarning ("Ready of an unknown service %s-%hu", serviceName.c_str(), sid);
		return;
	}

	nlinfo ("*:*:%d is ready (%s-%hu)", Services[sid].ServiceId, Services[sid].ShortName.c_str (), Services[sid].ServiceId);
	Services[sid].Ready = true;
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

static void cbRegisteredServices (CMessage &msgin, const std::string &serviceName, uint16 sid)
{
	// receive the list of all registered services
	msgin.serialCont (RegisteredServices);
}

static void cbView (CMessage &msgin, const std::string &serviceName, uint16 sid)
{
	// receive an view answer from the service
	SIT sit = findService (sid);
	
	uint32 rid;
	msgin.serial (rid);

	vector<pair<vector<string>, vector<string> >  > answer;
	vector<string> vara, vala;

	while ((uint32)msgin.getPos() < msgin.length())
	{
		vara.clear ();
		vala.clear ();
		
		// adding default row
		
		uint32 i, nb;
		string var, val;
		
		msgin.serial (nb);
		for (i = 0; i < nb; i++)
		{
			msgin.serial (var);
			vara.push_back (var);
		}
		
		msgin.serial (nb);
		for (i = 0; i < nb; i++)
		{
			msgin.serial (val);
			vala.push_back (val);
		}
		answer.push_back (make_pair(vara,vala));
	}
	addRequestAnswer (rid, answer);
	
	// remove the waiting request
	for (uint i = 0; i < (*sit).WaitingRequestId.size();)
	{
		if ((*sit).WaitingRequestId[i] == rid)
		{
			(*sit).WaitingRequestId.erase ((*sit).WaitingRequestId.begin ()+i);
		}
		else
		{
			i++;
		}
	}
}


static void cbGetView (CMessage &msgin, const std::string &serviceName, uint16 sid)
{
	uint32 rid;
	string rawvarpath;

	msgin.serial (rid);
	msgin.serial (rawvarpath);

	addRequest (rid, rawvarpath, sid);
}

void serviceConnection (const std::string &serviceName, uint16 sid, void *arg)
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

}

void serviceDisconnection (const std::string &serviceName, uint16 sid, void *arg)
{
	// don't remove AS
	if (serviceName == "AS")
		return;

	if (sid >= Services.size ())
	{
		nlwarning ("Disconnection of an unknown service %s-%hu", serviceName.c_str(), sid);
		return;
	}

	if (!Services[sid].Connected)
	{
		nlwarning ("Disconnection of an unknown service %s-%hu", serviceName.c_str(), sid);
		return;
	}

	// we need to remove pending request

	for(uint i = 0; i < Services[sid].WaitingRequestId.size (); i++)
	{
		subRequestWaitingNb (Services[sid].WaitingRequestId[i]);
	}

	nlinfo ("%s-%hu disconnected", Services[sid].ShortName.c_str (), Services[sid].ServiceId);

	if (Services[sid].AutoReconnect)
	{
		// we have to relaunch it
		nlinfo ("Waiting few seconds and relaunching the service %s", Services[sid].toString().c_str ());
		startService (CTime::getSecondsSince1970()+IService::getInstance()->ConfigFile.getVar("RestartDelay").asInt(), Services[sid].AliasName);
	}

	Services[sid].reset();
}


/** Callback Array
 */
TUnifiedCallbackItem CallbackArray[] =
{
	{ "SID", cbServiceIdentification },
	{ "SR", cbServiceReady },
	{ "LOG", cbLog },

	{ "GET_VIEW", cbGetView },
	{ "VIEW", cbView },

	{ "REGISTERED_SERVICES", cbRegisteredServices },
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
	}

	bool		update ()
	{
		cleanRequest ();
		checkWaitingServices ();

		return true;
	}
};

/// Naming Service
NLNET_SERVICE_MAIN (CAdminExecutorService, "AES", "admin_executor_service", 49997, CallbackArray, NELNS_CONFIG, NELNS_LOGS);

uint32 toto = 123, tata = 5456;

NLMISC_VARIABLE(uint32, toto, "test the get view system");
NLMISC_VARIABLE(uint32, tata, "test the get view system");

NLMISC_COMMAND (getViewAES, "send a view and receive an array as result", "<varpath>")
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

#ifdef NL_OS_LINUX

static inline char *skipToken(const char *p)
{
    while (isspace(*p)) p++;
    while (*p && !isspace(*p)) p++;
    return (char *)p;
}

// col: 0        1       2    3    4    5     6          7         8        9       10   11   12   13    14      15
//      receive                                                    sent
//      bytes    packets errs drop fifo frame compressed multicast bytes    packets errs drop fifo colls carrier compressed
uint64 getSystemNetwork (uint col)
{
	if (col > 15)
		return 0;

    char buffer[4096+1];
    int fd, len;
    char *p;
	
	fd = open("/proc/net/dev", O_RDONLY);
	len = read(fd, buffer, sizeof(buffer)-1);
	close(fd);
	buffer[len] = '\0';
	
	p = strchr(buffer, '\n')+1;
	p = strchr(buffer, '\n')+1;
	
	uint64 val = 0;
	while (true)
	{
		p = strchr(buffer, ':');
		if (p == NULL)
			break;
		p++;
		for (uint i = 0; i < col; i++)
		{
			p = skipToken(p);
		}
		val += strtoul(p, &p, 10);
	}
	return val;
}

NLMISC_DYNVARIABLE(uint64, NetBytesSent, "Amount of bytes sent to all networks cards in bytes")
{
	if (get) *pointer = getSystemNetwork (8);
}

NLMISC_DYNVARIABLE(uint64, NetBytesReceived, "Amount of bytes received to all networks cards in bytes")
{
	if (get) *pointer = getSystemNetwork (0);
}

NLMISC_DYNVARIABLE(uint32, NetError, "Number of error on all networks cards")
{
	if (get) *pointer = getSystemNetwork (2) + getSystemNetwork (10);
}

#endif // NL_OS_LINUX
