/** \file admin.cpp
 * manage services admin
 *
 * $Id: admin.cpp,v 1.17.8.1 2004/11/04 11:18:19 legros Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX NeL Network Services.
 * NEVRAX NeL Network Services is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NeL Network Services is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NeL Network Services; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */


//
// Includes
//

#include "stdnet.h"

#include <time.h>

#ifdef NL_OS_WINDOWS
#   include <windows.h>
#endif

#include "nel/net/service.h"
#include "nel/net/admin.h"
#include "nel/net/varpath.h"


//
// Namspaces
//

using namespace std;
using namespace NLMISC;
using namespace NLNET;


namespace NLNET {


//
// Structures
//

struct CRequest
{
	CRequest (uint32 id, uint16 sid) : Id(id), NbWaiting(0), NbReceived(0), SId(sid)
	{
		nldebug ("ADMIN: ++ NbWaiting %d NbReceived %d", NbWaiting, NbReceived);
		Time = CTime::getSecondsSince1970 ();
	}
	
	uint32			Id;
	uint			NbWaiting;
	uint32			NbReceived;
	uint16			SId;
	uint32			Time;	// when the request was ask
	
	vector<pair<vector<string>, vector<string> > > Answers;
};
	

//
// Variables
//

TRemoteClientCallback RemoteClientCallback = 0;

vector<CAlarm> Alarms;

vector<CGraphUpdate> GraphUpdates;

// check alarms every 5 seconds
const uint32 AlarmCheckDelay = 5;

vector<CRequest> Requests;

uint32 RequestTimeout = 4;	// in second


//
// Callbacks
//

static void cbInfo (CMessage &msgin, const std::string &serviceName, uint16 sid)
{
	nlinfo ("ADMIN: Updating admin informations");

	vector<string> alarms;
	msgin.serialCont (alarms);
	vector<string> graphupdate;
	msgin.serialCont (graphupdate);
	
	setInformations (alarms, graphupdate);
}	

static void cbServGetView (CMessage &msgin, const std::string &serviceName, uint16 sid)
{
	uint32 rid;
	string rawvarpath;

	msgin.serial (rid);
	msgin.serial (rawvarpath);

	Requests.push_back (CRequest(rid, sid));

	vector<pair<vector<string>, vector<string> > > answer;
	// just send the view in async mode, don't retrieve the answer
	serviceGetView (rid, rawvarpath, answer, true);
	nlassert (answer.empty());

/*
	CMessage msgout("VIEW");
	msgout.serial(rid);
	
	for (uint i = 0; i < answer.size(); i++)
	{
		msgout.serialCont (answer[i].first);
		msgout.serialCont (answer[i].second);
	}
	
	CUnifiedNetwork::getInstance ()->send (sid, msgout);
	nlinfo ("ADMIN: Sent result view to service '%s-%hu'", serviceName.c_str(), sid);
*/

}


static void cbExecCommand (CMessage &msgin, const std::string &serviceName, uint16 sid)
{
	string command;
	msgin.serial (command);
	
	nlinfo ("ADMIN: Executing command from network : '%s'", command.c_str());
	ICommand::execute (command, IService::getInstance()->CommandLog);
}


// AES wants to know if i'm not dead, I have to answer faster as possible or i'll be killed
static void cbAdminPing (CMessage &msgin, const std::string &serviceName, uint16 sid)
{
	// Send back a pong to say to the AES that I'm alive
	CMessage msgout("ADMIN_PONG");
	CUnifiedNetwork::getInstance()->send(sid, msgout);
}

static void cbStopService (CMessage &msgin, const std::string &serviceName, uint16 sid)
{
	nlinfo ("ADMIN: Receive a stop from service %s-%d, need to quit", serviceName.c_str(), sid);
	IService::getInstance()->exit (0xFFFF);
}


void cbAESConnection (const string &serviceName, uint16 sid, void *arg)
{
	// established a connection to the AES, identify myself

	//
	// Sends the identification message with the name of the service and all commands available on this service
	//

	CMessage msgout ("SID");
	uint32 pid = getpid ();
	msgout.serial (IService::getInstance()->_AliasName, IService::getInstance()->_LongName, pid);
	ICommand::serialCommands (msgout);
	CUnifiedNetwork::getInstance()->send("AES", msgout);

	if (IService::getInstance()->_Initialized)
	{
		CMessage msgout2 ("SR");
		CUnifiedNetwork::getInstance()->send("AES", msgout2);
	}
}


static void cbAESDisconnection (const std::string &serviceName, uint16 sid, void *arg)
{
	nlinfo("Lost connection to the %s-%hu", serviceName.c_str(), sid);
}


static TUnifiedCallbackItem CallbackArray[] =
{
	{ "INFO",			cbInfo },
	{ "GET_VIEW",		cbServGetView },
	{ "STOPS",			cbStopService },
	{ "EXEC_COMMAND",	cbExecCommand },
	{ "ADMIN_PING",		cbAdminPing },
};


//
// Functions
//

void setRemoteClientCallback (TRemoteClientCallback cb)
{
	RemoteClientCallback = cb;
}


//
// Request functions
//

static void addRequestWaitingNb (uint32 rid)
{
	for (uint i = 0 ; i < Requests.size (); i++)
	{
		if (Requests[i].Id == rid)
		{
			Requests[i].NbWaiting++;
			nldebug ("ADMIN: ++ i %d rid %d NbWaiting+ %d NbReceived %d", i, Requests[i].Id, Requests[i].NbWaiting, Requests[i].NbReceived);
			// if we add a waiting, reset the timer
			Requests[i].Time = CTime::getSecondsSince1970 ();
			return;
		}
	}
	nlwarning ("ADMIN: addRequestWaitingNb: can't find the rid %d", rid);
}

static void subRequestWaitingNb (uint32 rid)
{
	for (uint i = 0 ; i < Requests.size (); i++)
	{
		if (Requests[i].Id == rid)
		{
			Requests[i].NbWaiting--;
			nldebug ("ADMIN: ++ i %d rid %d NbWaiting- %d NbReceived %d", i, Requests[i].Id, Requests[i].NbWaiting, Requests[i].NbReceived);
			return;
		}
	}
	nlwarning ("ADMIN: subRequestWaitingNb: can't find the rid %d", rid);
}
/*
void addRequestAnswer (uint32 rid, const vector <pair<vector<string>, vector<string> > >&answer)
{
	for (uint i = 0 ; i < Requests.size (); i++)
	{
		if (Requests[i].Id == rid)
		{
			for (uint t = 0; t < answer.size(); t++)
			{
				if (!answer[t].first.empty() && answer[t].first[0] == "__log")
				{	nlassert (answer[t].first.size() == 1); }
				else
				{	nlassert (answer[t].first.size() == answer[t].second.size()); }
				Requests[i].Answers.push_back (make_pair(answer[t].first, answer[t].second));
			}
			Requests[i].NbReceived++;
			nldebug ("ADMIN: ++ i %d rid %d NbWaiting %d NbReceived+ %d", i, Requests[i].Id, Requests[i].NbWaiting, Requests[i].NbReceived);
			return;
		}
	}
	// we received an unknown request, forget it
	nlwarning ("ADMIN: Receive an answer for unknown request %d", rid);
}
*/

void addRequestAnswer (uint32 rid, const vector<string> &variables, const vector<string> &values)
{
	if (!variables.empty() && variables[0] == "__log")
	{	nlassert (variables.size() == 1); }
	else
	{	nlassert (variables.size() == values.size()); }

	for (uint i = 0 ; i < Requests.size (); i++)
	{
		if (Requests[i].Id == rid)
		{
			Requests[i].Answers.push_back (make_pair(variables, values));

			Requests[i].NbReceived++;
			nldebug ("ADMIN: ++ i %d rid %d NbWaiting %d NbReceived+ %d", i, Requests[i].Id, Requests[i].NbWaiting, Requests[i].NbReceived);
			
			return;
		}
	}
	// we received an unknown request, forget it
	nlwarning ("ADMIN: Receive an answer for unknown request %d", rid);
}

static bool emptyRequest (uint32 rid)
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

static void cleanRequest ()
{
	uint32 currentTime = CTime::getSecondsSince1970 ();

	for (uint i = 0 ; i < Requests.size ();)
	{
		// timeout
		if (currentTime >= Requests[i].Time+RequestTimeout)
		{
			nlwarning ("ADMIN: **** i %d rid %d -> Requests[i].NbWaiting (%d) != Requests[i].NbReceived (%d)", i, Requests[i].Id, Requests[i].NbWaiting, Requests[i].NbReceived);
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
			{
				nlinfo ("ADMIN: Receive an answer for the fake request %d with %d answers", Requests[i].Id, Requests[i].Answers.size ());
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
			{
				nlinfo ("ADMIN: The request is over, send the result to AES");
				CUnifiedNetwork::getInstance ()->send (Requests[i].SId, msgout);
			}

			// set to 0 to erase it
			Requests[i].NbWaiting = 0;
			nldebug ("ADMIN: ++ i %d rid %d NbWaiting0 %d NbReceived %d", i, Requests[i].Id, Requests[i].NbWaiting, Requests[i].NbReceived);
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

// all remote command start with rc or RC
bool isRemoteCommand(string &str)
{
	if (str.size()<2) return false;
	return tolower(str[0]) == 'r' && tolower(str[1]) == 'c';
}


// this callback is used to create a view for the admin system
void serviceGetView (uint32 rid, const string &rawvarpath, vector<pair<vector<string>, vector<string> > > &answer, bool async)
{
	string str;
	CLog logDisplayVars;
	CLightMemDisplayer mdDisplayVars;
	logDisplayVars.addDisplayer (&mdDisplayVars);
	mdDisplayVars.setParam (4096);

	CVarPath varpath(rawvarpath);

	if (varpath.empty())
		return;

	if (varpath.isFinal())
	{
		vector<string> vara, vala;

		// add default row
		vara.push_back ("service");
		vala.push_back (IService::getInstance ()->getServiceUnifiedName());
		
		for (uint j = 0; j < varpath.Destination.size (); j++)
		{
			string cmd = varpath.Destination[j].first;

			// replace = with space to execute the command
			uint eqpos = cmd.find("=");
			if (eqpos != string::npos)
			{
				cmd[eqpos] = ' ';
				vara.push_back(cmd.substr(0, eqpos));
			}
			else
				vara.push_back(cmd);
			
			mdDisplayVars.clear ();
			ICommand::execute(cmd, logDisplayVars, !ICommand::isCommand(cmd));
			const std::deque<std::string>	&strs = mdDisplayVars.lockStrings();

			if (ICommand::isCommand(cmd))
			{
				// we want the log of the command
				if (j == 0)
				{
					vara.clear ();
					vara.push_back ("__log");
					vala.clear ();
				}
				
				vala.push_back ("----- Result from "+IService::getInstance()->getServiceUnifiedName()+" of command '"+cmd+"'\n");
				for (uint k = 0; k < strs.size(); k++)
				{
					vala.push_back (strs[k]);
				}
			}
			else
			{

				if (strs.size()>0)
				{
					str = strs[0].substr(0,strs[0].size()-1);
					// replace all spaces into udnerscore because space is a reserved char
					for (uint i = 0; i < str.size(); i++) if (str[i] == ' ') str[i] = '_';
					
				/*
					uint32 pos = strs[0].find("=");
					if(pos != string::npos && pos + 2 < strs[0].size())
					{
						uint32 pos2 = string::npos;
						if(strs[0][strs[0].size()-1] == '\n')
							pos2 = strs[0].size() - pos - 2 - 1;
						
						str = strs[0].substr (pos+2, pos2);
						
						// replace all spaces into udnerscore because space is a reserved char
						for (uint i = 0; i < str.size(); i++) if (str[i] == ' ') str[i] = '_';
					}
					else
					{
						str = "???";
					}*/
				}
				else
				{
					str = "???";
				}
				vala.push_back (str);
				nlinfo ("ADMIN: Add to result view '%s' = '%s'", varpath.Destination[j].first.c_str(), str.c_str());
			}
			mdDisplayVars.unlockStrings();
		}

		if (!async)
			answer.push_back (make_pair(vara, vala));
		else
		{
			addRequestWaitingNb (rid);
			addRequestAnswer (rid, vara, vala);
		}
	}
	else
	{
		// there s an entity in the varpath, manage this case

		vector<string> *vara=0, *vala=0;
		
		// varpath.Destination		contains the entity number
		// subvarpath.Destination	contains the command name
		
		for (uint i = 0; i < varpath.Destination.size (); i++)
		{
			CVarPath subvarpath(varpath.Destination[i].second);
			
			for (uint j = 0; j < subvarpath.Destination.size (); j++)
			{
				// set the variable name
				string cmd = subvarpath.Destination[j].first;

				if (isRemoteCommand(cmd))
				{
					if (async && RemoteClientCallback != 0)
					{
						// ok we have to send the request to another side, just send and wait
						addRequestWaitingNb (rid);
						RemoteClientCallback (rid, cmd, varpath.Destination[i].first);
					}
				}
				else
				{
					// replace = with space to execute the command
					uint eqpos = cmd.find("=");
					if (eqpos != string::npos)
					{
						cmd[eqpos] = ' ';
						// add the entity
						cmd.insert(eqpos, " "+varpath.Destination[i].first);
					}
					else
					{
						// add the entity
						cmd += " "+varpath.Destination[i].first;
					}
					
					mdDisplayVars.clear ();
					ICommand::execute(cmd, logDisplayVars, true);
					const std::deque<std::string>	&strs = mdDisplayVars.lockStrings();
					for (uint k = 0; k < strs.size(); k++)
					{
						const string &str = strs[k];

						uint32 pos = str.find(" ");
						if(pos == string::npos)
							continue;
						
						string entity = str.substr(0, pos);
						string value = str.substr(pos+1, str.size()-pos-2);
						for (uint u = 0; u < value.size(); u++) if (value[u] == ' ') value[u] = '_';
						
						// look in the array if we already have something about this entity

						if (!async)
						{
							uint y;
							for (y = 0; y < answer.size(); y++)
							{
								if (answer[y].second[1] == entity)
								{
									// ok we found it, just push_back new stuff
									vara = &(answer[y].first);
									vala = &(answer[y].second);
									break;
								}
							}
							if (y == answer.size ())
							{
								answer.push_back (make_pair(vector<string>(), vector<string>()));

								vara = &(answer[answer.size()-1].first);
								vala = &(answer[answer.size()-1].second);
								
								// don't add service if we want an entity
		// todo when we work on entity, we don't need service name and server so we should remove them and collapse all var for the same entity
								vara->push_back ("service");
								string name = IService::getInstance ()->getServiceUnifiedName();
								vala->push_back (name);
								
								// add default row
								vara->push_back ("entity");
								vala->push_back (entity);
							}

							vara->push_back (cmd.substr(0, cmd.find(" ")));
							vala->push_back (value);
						}
						else
						{
							addRequestWaitingNb (rid);

							vector<string> vara, vala;
							vara.push_back ("service");
							string name = IService::getInstance ()->getServiceUnifiedName();
							vala.push_back (name);

							// add default row
							vara.push_back ("entity");
							vala.push_back (entity);

							vara.push_back (cmd.substr(0, cmd.find(" ")));
							vala.push_back (value);
	
							addRequestAnswer (rid, vara, vala);
						}
						nlinfo ("ADMIN: Add to result view for entity '%s', '%s' = '%s'", varpath.Destination[i].first.c_str(), subvarpath.Destination[j].first.c_str(), str.c_str());
					}
					mdDisplayVars.unlockStrings();
				}
			}
		}
	}
}


//
// Alarms functions
//

void sendAdminEmail (char *format, ...)
{
	char *text;
	NLMISC_CONVERT_VARGS (text, format, 4096);
	
	time_t t = time (&t);

	string str;
	str  = asctime (localtime (&t));
	str += " Server " + CInetAddress::localHost().hostName();
	str += " service " + IService::getInstance()->getServiceUnifiedName();
	str += " : ";
	str += text;

	CMessage msgout("ADMIN_EMAIL");
	msgout.serial (str);
	if(IService::getInstance ()->getServiceShortName()=="AES")
		CUnifiedNetwork::getInstance ()->send ("AS", msgout);
	else
		CUnifiedNetwork::getInstance ()->send ("AES", msgout);

	nlinfo ("ADMIN: Forwarded email to AS with '%s'", str.c_str());
}

void initAdmin (bool dontUseAES)
{
	if (!dontUseAES)
	{
		CUnifiedNetwork::getInstance()->setServiceUpCallback ("AES", cbAESConnection, NULL);
		CUnifiedNetwork::getInstance()->setServiceDownCallback ("AES", cbAESDisconnection, NULL);
		CUnifiedNetwork::getInstance()->addService ("AES", CInetAddress("localhost:49997"));
	}
	CUnifiedNetwork::getInstance()->addCallbackArray (CallbackArray, sizeof(CallbackArray)/sizeof(CallbackArray[0]));
}


void updateAdmin()
{
	string str;
	CLog logDisplayVars;
	CLightMemDisplayer mdDisplayVars;
	logDisplayVars.addDisplayer (&mdDisplayVars);
	
	uint32 CurrentTime = CTime::getSecondsSince1970();


	//
	// check admin requests
	//

	cleanRequest ();


	//
	// Check graph updates
	//

	static uint32 lastGraphUpdateCheck = 0;
	
	if (CurrentTime >= lastGraphUpdateCheck+1)
	{
		lastGraphUpdateCheck = CurrentTime;

		CMessage msgout ("GRAPH_UPDATE");
		bool empty = true;
		for (uint j = 0; j < GraphUpdates.size(); j++)
		{
			if (CurrentTime >= GraphUpdates[j].LastUpdate + GraphUpdates[j].Update)
			{
				// have to send a new update for this var
				ICommand::execute(GraphUpdates[j].Name, logDisplayVars, true, false);
				const std::deque<std::string>	&strs = mdDisplayVars.lockStrings();
				sint32 val;
				if (strs.size() != 1)
				  {
					nlwarning ("ADMIN: The graph update command execution not return exactly 1 line but %d", strs.size());
					for (uint i = 0; i < strs.size(); i++)
					  nlwarning ("ADMIN: line %d: '%s'", i, strs[i].c_str());
					val = 0;
				  }
				else
				  {
					val = atoi(strs[0].c_str());
				  }
				mdDisplayVars.unlockStrings ();
				mdDisplayVars.clear ();
				
				string name = IService::getInstance()->getServiceAliasName();
				if (name.empty())
					name = IService::getInstance()->getServiceShortName();

				if(empty)
					msgout.serial (CurrentTime);

				msgout.serial (name);
				msgout.serial (GraphUpdates[j].Name);
				msgout.serial (val);

				empty = false;

				GraphUpdates[j].LastUpdate = CurrentTime;
			}
		}

		if(!empty)
		{
			if(IService::getInstance ()->getServiceShortName()=="AES")
				CUnifiedNetwork::getInstance ()->send ("AS", msgout);
			else
				CUnifiedNetwork::getInstance ()->send ("AES", msgout);
		}
	}


	//
	// Check alarms
	//

	static uint32 lastAlarmsCheck = 0;

	if (CurrentTime >= lastAlarmsCheck+AlarmCheckDelay)
	{
		lastAlarmsCheck = CTime::getSecondsSince1970();

		for (uint i = 0; i < Alarms.size(); )
		{
			mdDisplayVars.clear ();
			ICommand::execute(Alarms[i].Name, logDisplayVars, true, false);
			const std::deque<std::string>	&strs = mdDisplayVars.lockStrings();

			if (strs.size()>0)
			{
				str = strs[0].substr(0,strs[0].size()-1);
			}
			else
			{
				str = "???";
			}

			mdDisplayVars.unlockStrings();
			
			if (str == "???")
			{
				// variable doesn't exist, remove it from alarms
				nlwarning ("ADMIN: Alarm problem: variable '%s' returns ??? instead of a good value", Alarms[i].Name.c_str());
				Alarms.erase (Alarms.begin()+i);
			}
			else
			{
				// compare the value
				uint32 err = Alarms[i].Limit;
				uint32 val = humanReadableToBytes(str);
				if (Alarms[i].GT && val >= err)
				{
					if (!Alarms[i].Activated)
					{
						nlinfo ("ADMIN: VARIABLE TOO BIG '%s' %u >= %u", Alarms[i].Name.c_str(), val, err);
						Alarms[i].Activated = true;
						sendAdminEmail ("Alarm: Variable %s is %u that is greater or equal than the limit %u", Alarms[i].Name.c_str(), val, err);
					}
				}
				else if (!Alarms[i].GT && val <= err)
				{
					if (!Alarms[i].Activated)
					{
						nlinfo ("ADMIN: VARIABLE TOO LOW '%s' %u <= %u", Alarms[i].Name.c_str(), val, err);
						Alarms[i].Activated = true;
						sendAdminEmail ("Alarm: Variable %s is %u that is lower or equal than the limit %u", Alarms[i].Name.c_str(), val, err);
					}
				}
				else
				{
					if (Alarms[i].Activated)
					{
						nlinfo ("ADMIN: variable is ok '%s' %u %s %u", Alarms[i].Name.c_str(), val, (Alarms[i].GT?"<":">"), err);
						Alarms[i].Activated = false;
					}
				}
				
				i++;
			}
		}
	}
}

void setInformations (const vector<string> &alarms, const vector<string> &graphupdate)
{
	uint i;

	// add only commands that I understand
	Alarms.clear ();
	for (i = 0; i < alarms.size(); i+=3)
	{
		CVarPath shardvarpath (alarms[i]);
		if(shardvarpath.Destination.size() == 0 || shardvarpath.Destination[0].second.empty())
			continue;
		CVarPath servervarpath (shardvarpath.Destination[0].second);
		if(servervarpath.Destination.size() == 0 || servervarpath.Destination[0].second.empty())
			continue;
		CVarPath servicevarpath (servervarpath.Destination[0].second);
		if(servicevarpath.Destination.size() == 0 || servicevarpath.Destination[0].second.empty())
			continue;
	
		string name = servicevarpath.Destination[0].second;

		if (IService::getInstance()->getServiceUnifiedName().find(servicevarpath.Destination[0].first) != string::npos && ICommand::exists(name))
		{
			nlinfo ("ADMIN: Adding alarm '%s' limit %d order %s (varpath '%s')", name.c_str(), atoi(alarms[i+1].c_str()), alarms[i+2].c_str(), alarms[i].c_str());
			Alarms.push_back(CAlarm(name, atoi(alarms[i+1].c_str()), alarms[i+2]=="gt"));
		}
		else
		{
			if (IService::getInstance()->getServiceUnifiedName().find(servicevarpath.Destination[0].first) == string::npos)
			{
				nlinfo ("ADMIN: Skipping alarm '%s' limit %d order %s (varpath '%s') (not for my service, i'm '%s')", name.c_str(), atoi(alarms[i+1].c_str()), alarms[i+2].c_str(), alarms[i].c_str(), IService::getInstance()->getServiceUnifiedName().c_str());
			}
			else
			{
				nlinfo ("ADMIN: Skipping alarm '%s' limit %d order %s (varpath '%s') (var not exist)", name.c_str(), atoi(alarms[i+1].c_str()), alarms[i+2].c_str(), alarms[i].c_str());
			}
		}
	}

	// do the same with graph update
	GraphUpdates.clear ();
	for (i = 0; i < graphupdate.size(); i+=2)
	{
		CVarPath shardvarpath (graphupdate[i]);
		if(shardvarpath.Destination.size() == 0 || shardvarpath.Destination[0].second.empty())
			continue;
		CVarPath servervarpath (shardvarpath.Destination[0].second);
		if(servervarpath.Destination.size() == 0 || servervarpath.Destination[0].second.empty())
			continue;
		CVarPath servicevarpath (servervarpath.Destination[0].second);
		if(servicevarpath.Destination.size() == 0 || servicevarpath.Destination[0].second.empty())
			continue;
		
		string VarName = servicevarpath.Destination[0].second;
		string ServiceName = servicevarpath.Destination[0].first;

		if (ICommand::exists(VarName) && (ServiceName == "*" || IService::getInstance()->getServiceShortName() == ServiceName))
		{
			nlinfo ("ADMIN: Adding graphupdate '%s' update %d (varpath '%s')", VarName.c_str(), atoi(graphupdate[i+1].c_str()), graphupdate[i].c_str());
			GraphUpdates.push_back(CGraphUpdate(VarName, atoi(graphupdate[i+1].c_str())));
		}
		else
		{
			if (IService::getInstance()->getServiceShortName() != ServiceName)
			{
				nlinfo ("ADMIN: Skipping graphupdate '%s' limit %d (varpath '%s') (not for my service, i'm '%s')", VarName.c_str(), atoi(graphupdate[i+1].c_str()), graphupdate[i].c_str(), IService::getInstance()->getServiceUnifiedName().c_str());
			}
			else
			{
				nlinfo ("ADMIN: Skipping graphupdate '%s' limit %d (varpath '%s') (var not exist)", VarName.c_str(), atoi(graphupdate[i+1].c_str()), graphupdate[i].c_str());
			}
		}
	}
}

//
// Commands
//

NLMISC_CATEGORISED_COMMAND(nel, displayInformations, "displays all admin informations", "")
{
	uint i;

	log.displayNL("There're %d alarms:", Alarms.size());
	for (i = 0; i < Alarms.size(); i++)
	{
		log.displayNL(" %d %s %d %s %s", i, Alarms[i].Name.c_str(), Alarms[i].Limit, (Alarms[i].GT?"gt":"lt"), (Alarms[i].Activated?"on":"off"));
	}
	log.displayNL("There're %d graphupdate:", GraphUpdates.size());
	for (i = 0; i < GraphUpdates.size(); i++)
	{
		log.displayNL(" %d %s %d %d", i, GraphUpdates[i].Name.c_str(), GraphUpdates[i].Update, GraphUpdates[i].LastUpdate);
	}
	return true;
}

NLMISC_CATEGORISED_COMMAND(nel, getView, "send a view and receive an array as result", "<varpath>")
{
	if(args.size() != 1) return false;
	
	vector<pair<vector<string>, vector<string> > > answer;
	serviceGetView (0, args[0], answer);
	
	log.displayNL("have %d answer", answer.size());
	for (uint i = 0; i < answer.size(); i++)
	{
		log.displayNL("  have %d value", answer[i].first.size());
		
		nlassert (answer[i].first.size() == answer[i].second.size());
		
		for (uint j = 0; j < answer[i].first.size(); j++)
		{
			log.displayNL("    %s -> %s", answer[i].first[j].c_str(), answer[i].second[j].c_str());
		}
	}
	
	return true;
}

} // NLNET
