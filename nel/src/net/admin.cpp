/** \file admin.cpp
 * manage services admin
 *
 * $Id: admin.cpp,v 1.2 2003/03/26 14:29:03 lecroart Exp $
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

#include "stdnet.h"

#include <time.h>

#include "nel/net/service.h"
#include "nel/net/admin.h"
#include "nel/net/varpath.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;


namespace NLNET {

//
// Variables
//

vector<CAlarm> Alarms;

vector<CGraphUpdate> GraphUpdates;

// check alarms every 5 seconds
const uint32 AlarmCheckDelay = 5;

//
// Callbacks
//

static void cbInformations (CMessage &msgin, const std::string &serviceName, uint16 sid)
{
	nlinfo ("Updating informations");

	vector<string> alarms;
	msgin.serialCont (alarms);
	vector<string> graphupdate;
	msgin.serialCont (graphupdate);
	
	setInformations (alarms, graphupdate);
}	

static TUnifiedCallbackItem CallbackArray[] =
{
	{ "INFORMATIONS", cbInformations },
};


//
// Functions
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

	nlinfo ("Forwarded email to AS with '%s'", str.c_str());
}

void initAdmin ()
{
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
				ICommand::execute(GraphUpdates[j].Name, logDisplayVars, true);
				const std::deque<std::string>	&strs = mdDisplayVars.lockStrings();
				nlassert (strs.size() == 1);

				string name = IService::getInstance()->getServiceAliasName();
				if (name.empty())
					name = IService::getInstance()->getServiceShortName();

				if(empty)
					msgout.serial (CurrentTime);

				msgout.serial (name);
				msgout.serial (GraphUpdates[j].Name);
				msgout.serial (const_cast<string&>(strs[0]));

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
			ICommand::execute(Alarms[i].Name, logDisplayVars, true);
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
				nlwarning ("Alarm problem: variable '%s' returns ??? instead of a good value", Alarms[i].Name.c_str());
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
						nlinfo ("VARIABLE TOO BIG '%s' %u >= %u", Alarms[i].Name.c_str(), val, err);
						Alarms[i].Activated = true;
						sendAdminEmail ("Alarm: Variable %s is %u that is greater or equal than the limit %u", Alarms[i].Name.c_str(), val, err);
					}
				}
				else if (!Alarms[i].GT && val <= err)
				{
					if (!Alarms[i].Activated)
					{
						nlinfo ("VARIABLE TOO LOW '%s' %u <= %u", Alarms[i].Name.c_str(), val, err);
						Alarms[i].Activated = true;
						sendAdminEmail ("Alarm: Variable %s is %u that is lower or equal than the limit %u", Alarms[i].Name.c_str(), val, err);
					}
				}
				else
				{
					if (Alarms[i].Activated)
					{
						nlinfo ("variable is ok '%s' %u %s %u", Alarms[i].Name.c_str(), val, (Alarms[i].GT?"<":">"), err);
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
			nlinfo ("Adding alarm '%s' limit %d order %s (varpath '%s')", name.c_str(), atoi(alarms[i+1].c_str()), alarms[i+2].c_str(), alarms[i].c_str());
			Alarms.push_back(CAlarm(name, atoi(alarms[i+1].c_str()), alarms[i+2]=="gt"));
		}
		else
		{
			if (IService::getInstance()->getServiceUnifiedName().find(servicevarpath.Destination[0].first) == string::npos)
			{
				nlinfo ("Skipping alarm '%s' limit %d order %s (varpath '%s') (not for my service, i'm '%s')", name.c_str(), atoi(alarms[i+1].c_str()), alarms[i+2].c_str(), alarms[i].c_str(), IService::getInstance()->getServiceUnifiedName().c_str());
			}
			else
			{
				nlinfo ("Skipping alarm '%s' limit %d order %s (varpath '%s') (var not exist)", name.c_str(), atoi(alarms[i+1].c_str()), alarms[i+2].c_str(), alarms[i].c_str());
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
		
		string name = servicevarpath.Destination[0].second;
		
		if (IService::getInstance()->getServiceUnifiedName().find(servicevarpath.Destination[0].first) != string::npos && ICommand::exists(name))
		{
			nlinfo ("Adding graphupdate '%s' update %d (varpath '%s')", name.c_str(), atoi(graphupdate[i+1].c_str()), graphupdate[i].c_str());
			GraphUpdates.push_back(CGraphUpdate(name, atoi(graphupdate[i+1].c_str())));
		}
		else
		{
			if (IService::getInstance()->getServiceUnifiedName().find(servicevarpath.Destination[0].first) == string::npos)
			{
				nlinfo ("Skipping graphupdate '%s' limit %d (varpath '%s') (not for my service, i'm '%s')", name.c_str(), atoi(graphupdate[i+1].c_str()), graphupdate[i].c_str(), IService::getInstance()->getServiceUnifiedName().c_str());
			}
			else
			{
				nlinfo ("Skipping graphupdate '%s' limit %d (varpath '%s') (var not exist)", name.c_str(), atoi(graphupdate[i+1].c_str()), graphupdate[i].c_str());
			}
		}
	}
}

//
// Commands
//

NLMISC_COMMAND (displayInformations, "displays all admin informations", "")
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



} // NLNET
