/** \file alarms.cpp
 * manage services alarms
 *
 * $Id: alarms.cpp,v 1.5 2003/02/07 16:08:25 lecroart Exp $
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
#include "nel/net/alarms.h"
#include "nel/net/varpath.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;


namespace NLNET {

//
// Variables
//

vector<CAlarm> Alarms;

// check alarms every 5 seconds
const uint32 AlarmCheckDelay = 5;

//
// Callbacks
//

static void cbAlarms (CMessage &msgin, const std::string &serviceName, uint16 sid)
{
	// receive the list of all alarms
	vector<string> str;
	msgin.serialCont (str);
	nlinfo ("Updating alarms");

	setAlarms (str);
}	

static TUnifiedCallbackItem CallbackArray[] =
{
	{ "ALARMS", cbAlarms },
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

void initAlarms ()
{
	CUnifiedNetwork::getInstance()->addCallbackArray (CallbackArray, sizeof(CallbackArray)/sizeof(CallbackArray[0]));
}


void updateAlarms ()
{
	string str;
	CLog logDisplayVars;
	CLightMemDisplayer mdDisplayVars;
	logDisplayVars.addDisplayer (&mdDisplayVars);
	
	static uint32 lastCheck = 0;

	if (CTime::getSecondsSince1970()<lastCheck+AlarmCheckDelay)
		return;

	lastCheck = CTime::getSecondsSince1970();

	for (uint i = 0; i < Alarms.size(); )
	{
		mdDisplayVars.clear ();
		ICommand::execute(Alarms[i].Name, logDisplayVars, true);
		const std::deque<std::string>	&strs = mdDisplayVars.lockStrings();

		if (strs.size()>0)
		{
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

void setAlarms (const vector<string> &alarms)
{
	// add only commands that I understand
	Alarms.clear ();
	for (uint i = 0; i < alarms.size(); i+=3)
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
}

//
// Commands
//

NLMISC_COMMAND (displayAlarms, "displays all alarms", "")
{
	log.displayNL("There're %d alarms:", Alarms.size());
	for (uint i = 0; i < Alarms.size(); i++)
	{
		log.displayNL(" %d %s %d %s %s", i, Alarms[i].Name.c_str(), Alarms[i].Limit, (Alarms[i].GT?"gt":"lt"), (Alarms[i].Activated?"on":"off"));
	}
	return true;
}



} // NLNET
