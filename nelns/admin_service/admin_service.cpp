/** \file admin_service.cpp
 * Admin Service (AS)
 *
 * $Id: admin_service.cpp,v 1.38 2003/10/20 16:23:19 lecroart Exp $
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

#include <string>
#include <list>

#include <mysql.h>

#include "nel/misc/debug.h"
#include "nel/misc/config_file.h"
#include "nel/misc/path.h"
#include "nel/misc/command.h"

#include "nel/net/service.h"
#include "nel/net/varpath.h"
#include "nel/net/email.h"

#include "connection_web.h"


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
	CRequest (uint32 id, TSockId from) : Id(id), NbWaiting(0), NbReceived(0), From(from), NbRow(0), NbLines(1)
	{
		Time = CTime::getSecondsSince1970 ();
	}

	uint32			Id;
	uint			NbWaiting;
	uint32			NbReceived;
	TSockId			From;
	uint32			Time;	// when the request was ask
	
	uint32			NbRow;
	uint32			NbLines;

	vector<vector<string> > Array;	// it's the 2 dimensional array that will be send to the php for variables
	vector<string> Log;				// this log contains the answer if a command was asked, othewise, Array contains the results

	

	uint32 getVariable(const string &variable)
	{
		for (uint32 i = 0; i < NbRow; i++)
			if (Array[i][0] == variable)
				return i;

		// need to add the variable
		vector<string> NewRow;
		NewRow.resize (NbLines);
		NewRow[0] = variable;
		Array.push_back (NewRow);
		return NbRow++;
	}

	void addLine ()
	{
		for (uint32 i = 0; i < NbRow; i++)
			Array[i].push_back("");

		NbLines++;
	}

	void display ()
	{
		if (Log.empty())
		{
			nlinfo ("Display answer array for request %d: %d row %d lines", Id, NbRow, NbLines);
			for (uint i = 0; i < NbLines; i++)
			{
				for (uint j = 0; j < NbRow; j++)
				{
					nlassert (Array.size () == NbRow);
					InfoLog->displayRaw ("%-20s", Array[j][i].c_str());
				}
				InfoLog->displayRawNL ("");
			}
			InfoLog->displayRawNL ("End of the array");
		}
		else
		{
			nlinfo ("Display the log for request %d: %d lines", Id, Log.size());
			for (uint i = 0; i < Log.size(); i++)
			{
				InfoLog->displayRaw ("%s", Log[i].c_str());
			}
			InfoLog->displayRawNL ("End of the log");
		}
	}
};

/*
struct CService
{
	CService () : Ready(false), Connected(false), InConfig(false) { }

	string			AliasName;		/// alias of the service used in the AES and AS to find him (unique per AES)
	string			ShortName;		/// name of the service in short format ("NS" for example)
	string			LongName;		/// name of the service in long format ("naming_service")
	bool			Ready;			/// true if the service is ready
	bool			Connected;		/// true if the service is connected to the AES
	bool			InConfig;		/// true if the service is in the configuration
	std::vector<NLMISC::CSerialCommand>	Commands;

	void setValues (const CService &t)
	{
		// copy all except gtk stuffs
		AliasName = t.AliasName;
		ShortName = t.ShortName;
		LongName = t.LongName;
		Ready = t.Ready;
		Connected = t.Connected;
		//InConfig = t.InConfig; never change the inconfig value
	}
};
*/
//typedef list<CService> TServices;
//typedef list<CService>::iterator SIT;

struct CAdminExecutorService
{
	CAdminExecutorService (const string &shard, const string &name, uint16 sid) : Shard(shard), SId(sid), Name(name) { }

	string  Shard;			/// Name of the shard
	uint16	SId;			/// uniq number to identify the AES
	string	Name;			/// name of the admin executor service

	vector<uint32>	WaitingRequestId;		/// contains all request that the server hasn't reply yet

/*	SIT findService (uint16 sid, bool asrt = true)
	{
		SIT sit;
		for (sit = Services.begin(); sit != Services.end(); sit++)
			if ((*sit).SId == sid)
				break;

		if (asrt)
			nlassert (sit != Services.end());
		return sit;
	}

	SIT findService (const string &name, bool asrt = true)
	{
		SIT sit;
		for (sit = Services.begin(); sit != Services.end(); sit++)
			if ((*sit).Name == name)
				break;

		if (asrt)
			nlassert (sit != Services.end());
		return sit;
	}
*/};

typedef list<CAdminExecutorService> TAdminExecutorServices;
typedef list<CAdminExecutorService>::iterator AESIT;

//
// Variables
//

TAdminExecutorServices AdminExecutorServices;

MYSQL *DatabaseConnection = NULL;

vector<CRequest> Requests;

uint32 RequestTimeout = 5;	// in second

// cumulate 5 seconds of alert
sint32 AdminAlertAccumlationTime = 5;

//
// Functions
//

AESIT findAES (uint16 sid, bool asrt = true)
{
	AESIT aesit;
	for (aesit = AdminExecutorServices.begin(); aesit != AdminExecutorServices.end(); aesit++)
		if ((*aesit).SId == sid)
			break;
		
		if (asrt)
			nlassert (aesit != AdminExecutorServices.end());
		return aesit;
}

AESIT findAES (const string &name, bool asrt = true)
{
	AESIT aesit;
	for (aesit = AdminExecutorServices.begin(); aesit != AdminExecutorServices.end(); aesit++)
		if ((*aesit).Name == name)
			break;
		
		if (asrt)
			nlassert (aesit != AdminExecutorServices.end());
		
		return aesit;
}


//
// SQL helpers
//

MYSQL_RES *sqlCurrentQueryResult = NULL;

MYSQL_ROW sqlQuery (const char *format, ...)
{
	char *query;
	NLMISC_CONVERT_VARGS (query, format, 1024);
	
	if (DatabaseConnection == 0)
	{
		nlwarning ("MYSQL: mysql_query (%s) failed: DatabaseConnection is 0", query);
		return NULL;
	}
	
	int ret = mysql_query (DatabaseConnection, query);
	if (ret != 0)
	{
		nlwarning ("MYSQL: mysql_query () failed for query '%s': %s", query,  mysql_error(DatabaseConnection));
		return 0;
	}
	
	sqlCurrentQueryResult = mysql_store_result(DatabaseConnection);
	if (sqlCurrentQueryResult == 0)
	{
		nlwarning ("MYSQL: mysql_store_result () failed for query '%s': %s", query,  mysql_error(DatabaseConnection));
		return 0;
	}
	
	MYSQL_ROW row = mysql_fetch_row(sqlCurrentQueryResult);
	if (row == 0)
	{
		nlwarning ("MYSQL: mysql_fetch_row () failed for query '%s': %s", query,  mysql_error(DatabaseConnection));
	}
	
	nldebug ("MYSQL: sqlQuery(%s) returns %d rows", query, mysql_num_rows(sqlCurrentQueryResult));
	
	return row;	
}

MYSQL_ROW sqlNextRow ()
{
	if (sqlCurrentQueryResult == 0)
		return 0;
	
	return mysql_fetch_row(sqlCurrentQueryResult);
}

//
// Admin functions
//

string Email;
uint32 FirstEmailTime = 0;

void sendAdminAlert (const char *format, ...)
{
	char *text;
	NLMISC_CONVERT_VARGS (text, format, 4096);

	if (AdminAlertAccumlationTime == -1)
	{
		// we don't send email so just display a warning
		nlwarning ("ALERT: %s", text);
	}
	else
	{
		if(Email.empty() && FirstEmailTime == 0)
		{
			Email += text;
			FirstEmailTime = CTime::getSecondsSince1970();
		}
		else
		{
			Email += "\n";
			Email += text;
		}
		nldebug ("ALERT: pushing email into queue: %s", text);
	}
}

void updateSendAdminAlert ()
{
	if(!Email.empty() && FirstEmailTime != 0 && AdminAlertAccumlationTime >=0 && CTime::getSecondsSince1970() > FirstEmailTime + AdminAlertAccumlationTime)
	{
		vector<string> lines;
		explode (Email, "\n", lines, true);
		if (!lines.empty())
		{

			if (IService::getInstance()->ConfigFile.exists("SysLogPath") && IService::getInstance()->ConfigFile.exists("SysLogParams"))
			{
				// syslog
				string param;
				if (lines.size() > 1)
				{
					param = "Multiple problems, first is: ";
				}
				param += lines[0];
				string res = toString(IService::getInstance()->ConfigFile.getVar("SysLogParams").asString().c_str(), param.c_str());
				launchProgram(IService::getInstance()->ConfigFile.getVar("SysLogPath").asString(), res);
			}

			if (IService::getInstance()->ConfigFile.exists("AdminEmail"))
			{
				// email
				string subject;
				if (lines.size() == 1)
				{
					subject = lines[0];
				}
				else
				{
					subject = "Multiple problems";
				}
				
				CConfigFile::CVar &var = IService::getInstance()->ConfigFile.getVar("AdminEmail");
				for (sint i = 0; i < var.size(); i++)
				{
					if (!sendEmail ("", CInetAddress::localHost().hostName()+"@admin.org", var.asString(i), subject, Email))
					{
						nlwarning ("Can't send email to '%s'", var.asString(i).c_str());
					}
					else
					{
						nlinfo ("ALERT: Sent email to admin %s the subject: %s", var.asString(i).c_str(), subject.c_str());
					}
				}
			}
		}

		Email = "";
		FirstEmailTime = 0;
	}
}


static void cbAdminEmail (CMessage &msgin, const std::string &serviceName, uint16 sid)
{
	string str;
	msgin.serial(str);
	sendAdminAlert (str.c_str());
}

static void cbGraphUpdate (CMessage &msgin, const std::string &serviceName, uint16 sid)
{
	uint32 CurrentTime;
	msgin.serial (CurrentTime);

	while (msgin.getPos() < (sint32)msgin.length())
	{
		string var, service;
		sint32 val;
		msgin.serial (service, var, val);

		AESIT aesit = findAES (sid);
		
		string shard, server;
		shard = (*aesit).Shard;
		server = (*aesit).Name;
		
		if (!shard.empty() && !server.empty() && !service.empty() && !var.empty())
		{
			string path = CPath::standardizePath (IService::getInstance()->ConfigFile.getVar("RRDVarPath").asString());
			string rrdfilename = path + shard+"."+server+"."+service+"."+var+".rrd";

			string arg;
			
			if (!NLMISC::CFile::fileExists(rrdfilename))
			{
				MYSQL_ROW row = sqlQuery ("select graph_update from variable where path like '%%%s' and graph_update!=0", var.c_str());
				if (row != NULL)
				{
					uint32 freq = atoi(row[0]);
					arg = "create "+rrdfilename+" --step "+toString(freq)+" DS:var:GAUGE:"+toString(freq*2)+":U:U RRA:AVERAGE:0.5:1:1000 RRA:AVERAGE:0.5:10:1000 RRA:AVERAGE:0.5:100:1000";
					launchProgram(IService::getInstance()->ConfigFile.getVar("RRDToolPath").asString(), arg);
				}
				else
				{
					nlwarning ("Can't create the rrd because no graph_update in database");
				}
			}

			arg = "update " + rrdfilename + " " + toString (CurrentTime) + ":" + toString(val);
			launchProgram(IService::getInstance()->ConfigFile.getVar("RRDToolPath").asString(), arg);
		}
		else
		{
			nlwarning ("Shard server service var val is empty");
		}
	}
}

//
// Request functions
//

uint32 newRequest (TSockId from)
{
	static uint32 NextId = 5461231;

	Requests.push_back (CRequest(NextId, from));

	return NextId++;
}

void addRequestWaitingNb (uint32 rid)
{
	for (uint i = 0 ; i < Requests.size (); i++)
	{
		if (Requests[i].Id == rid)
		{
			Requests[i].NbWaiting++;
			Requests[i].Time = CTime::getSecondsSince1970 ();
			return;
		}
	}
	nlwarning ("REQUEST: Received an answer from an unknown resquest %d (perhaps due to a AS timeout)", rid);
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
	nlwarning ("REQUEST: Received an answer from an unknown resquest %d (perhaps due to a AS timeout)", rid);
}

void addRequestReceived (uint32 rid)
{
	for (uint i = 0 ; i < Requests.size (); i++)
	{
		if (Requests[i].Id == rid)
		{
			Requests[i].NbReceived++;
//			Requests[i].display ();
			return;
		}
	}
	nlwarning ("REQUEST: Received an answer from an unknown resquest %d (perhaps due to a AS timeout)", rid);
}

void addRequestAnswer (uint32 rid, const vector<string> &variables, const vector<string> &values)
{
	for (uint i = 0 ; i < Requests.size (); i++)
	{
		Requests[i].addLine ();
		if (Requests[i].Id == rid)
		{
			if (!variables.empty() && variables[0]=="__log")
			{
				nlassert (variables.size() == 1);

				for (uint j = 0; j < values.size(); j++)
				{
					Requests[i].Log.push_back (values[j]);
				}
			}
			else
			{
				nlassert (variables.size() == values.size ());
				for (uint j = 0; j < variables.size(); j++)
				{
					uint32 pos = Requests[i].getVariable (variables[j]);
					Requests[i].Array[pos][Requests[i].NbLines-1] = values[j];
				}
			}
			return;
		}
	}
	nlwarning ("REQUEST: Received an answer from an unknown resquest %d (perhaps due to a AS timeout)", rid);
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

	bool timeout;

	for (uint i = 0 ; i < Requests.size ();)
	{
		// the AES doesn't answer quickly
		timeout = (currentTime >= Requests[i].Time+RequestTimeout);

		if (Requests[i].NbWaiting <= Requests[i].NbReceived || timeout)
		{
			// the request is over, send to the php

			string str;

			if (timeout)
			{
				nlwarning ("REQUEST: Request %d timeouted, only %d on %d services have replied", Requests[i].Id, Requests[i].NbReceived, Requests[i].NbWaiting);
			}
			
			if (Requests[i].Log.empty())
			{
				if (Requests[i].NbRow == 0 && timeout)
				{
					str = "1 ((TIMEOUT))";
				}
				else
				{
					str = toString(Requests[i].NbRow) + " ";
					for (uint k = 0; k < Requests[i].NbLines; k++)
					{
						for (uint j = 0; j < Requests[i].NbRow; j++)
						{
							nlassert (Requests[i].Array.size () == Requests[i].NbRow);
							if (Requests[i].Array[j][k].empty ())
								str += "??? ";
							else
							{
								str += Requests[i].Array[j][k];
								if (timeout)
									str += "((TIMEOUT))";
								str += " ";
							}
						}
					}
				}
			}
			else
			{
				for (uint k = 0; k < Requests[i].Log.size(); k++)
				{
					str += Requests[i].Log[k];
					if (timeout)
						str += "((TIMEOUT))";
				}
			}

//			Requests[i].display ();
			sendString (Requests[i].From, str);

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
// SQL functions
//

void sqlInit ()
{
	MYSQL *db = mysql_init(NULL);
	if(db == NULL)
	{
		nlerror ("mysql_init() failed");
	}

	DatabaseConnection = mysql_real_connect(db,
		IService::getInstance()->ConfigFile.getVar("DatabaseHost").asString().c_str(),
		IService::getInstance()->ConfigFile.getVar("DatabaseLogin").asString().c_str(),
		IService::getInstance()->ConfigFile.getVar("DatabasePassword").asString().c_str(),
		IService::getInstance()->ConfigFile.getVar("DatabaseName").asString().c_str(),
		0,NULL,0);
	if (DatabaseConnection == NULL || DatabaseConnection != db)
	{
		nlerror ("mysql_real_connect() failed to '%s' with login '%s' and database name '%s' with %s",
			IService::getInstance()->ConfigFile.getVar("DatabaseHost").asString().c_str(),
			IService::getInstance()->ConfigFile.getVar("DatabaseLogin").asString().c_str(),
			IService::getInstance()->ConfigFile.getVar("DatabaseName").asString().c_str(),
			(IService::getInstance()->ConfigFile.getVar("DatabasePassword").asString().empty()?"empty password":"password")
			);
	}
}


//
// Functions
//

/*void displayServices ()
{
	for (AESIT aesit = AdminExecutorServices.begin(); aesit != AdminExecutorServices.end(); aesit++)
	{
		nlinfo ("> Admin");
		for (SIT sit = (*aesit).Services.begin(); sit != (*aesit).Services.end(); sit++)
		{
			nlinfo ("  > '%s' '%s' '%s' '%s' %d %d", (*aesit).SockId->asString().c_str(), (*sit).AliasName.c_str(), (*sit).ShortName.c_str(), (*sit).LongName.c_str(), (*aesit).Id, (*sit).Id);
		}
	}
}
*/
/*
// send a message to a client. if ok is 0 it s an error or it s a normal 
void messageToClient (uint8 ok, string msg, TSockId from = NULL)
{
	CMessage msgout (CNetManager::getSIDA ("AS"), "MESSAGE");
	msgout.serial (ok, msg);
	CNetManager::send ("AS", msgout, from);
}
*/

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////// SCRIPT MANAGER /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
bool StartAllServices = false;
uint32 StartAllServicesPos;

void doNextStartAllServicesStep ()
{
	nlassert (StartAllServices);

	// get the script
	try
	{
		CConfigFile::CVar &script = IService::getInstance()->ConfigFile.getVar("Services");

		// check the position

		if (StartAllServicesPos*2 >= (uint32)script.size())
		{
			StartAllServices = false;

			nlinfo("end of the script");
			messageToClient (1, "Start All Service finnished correctly");
			return;
		}

		// get the script line

		string serverAlias;
		string serviceAlias;

		try
		{
			serverAlias = script.asString (StartAllServicesPos*2);
			serviceAlias = script.asString (StartAllServicesPos*2+1);
		}
		catch(EBadSize &)
		{
			messageToClient (1, "'Services' variable does not contains a good number of entries (must be a multiple of 2)");
			nlwarning ("'Services' variable does not contains a good number of entries (must be a multiple of 2)");
			StartAllServices = false;
			return;
		}

		AESIT aesit = findAdminExecutorService (serverAlias, false);
		if (aesit == AdminExecutorServices.end())
		{
			messageToClient (1, "don't find the server");
			nlwarning("don't find the server");
			StartAllServices = false;
			return;
		}

		// check if the service is not currently running

		StartAllServicesPos++;

		SIT sit = (*aesit).findService (serviceAlias);
		if ((*sit).Connected)
		{
			// the service is already running, go to the next process
			doNextStartAllServicesStep ();
		}
		else
		{
			// send the resquest to the AES

			CMessage msgout (CNetManager::getSIDA((*aesit).ServerAlias), "STARTS");
			msgout.serial (serviceAlias);
			CNetManager::send ((*aesit).ServerAlias, msgout);
		}
	}
	catch(EUnknownVar&)
	{
		messageToClient (1, "'Services' variable is not found");
		nlwarning ("'Services' variable is not found");
		StartAllServices = false;
		return;
	}
}

void initStartAllServices ()
{
	if (StartAllServices)
	{
		messageToClient (1, "already running a script, reset it");
		nlwarning("already running a script, reset it");
		StartAllServices = false;
	}

	try
	{
		CConfigFile::CVar &script = IService::getInstance()->ConfigFile.getVar("Services");

		for (sint i = 0 ; i < script.size (); i+=2)
		{
			string serverAlias = script.asString(i);
			AESIT aesit = findAdminExecutorService (serverAlias, false);
			if (aesit == AdminExecutorServices.end())
			{
				messageToClient (1, "an aes is not running, can't run the script");
				nlwarning("aes '%s' isn't running, can't run the script", serverAlias.c_str());
				return;
			}
		}
	}
	catch(EConfigFile &)
	{
		messageToClient (1, "bad config file");
		nlwarning ("bad config file");
		return;
	}

	StartAllServicesPos = 0;
	StartAllServices = true;

	doNextStartAllServicesStep();
}
*/

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////// CONNECTION TO THE AES ///////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
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

// get the service list from the admin exec and send the list to all admin client
static void cbServiceList (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	CAdminExecutorService *aes = (CAdminExecutorService*) (uint) from->appId();

	//
	// Get the list of service from aes
	//

	uint32 nbs;
	msgin.serial (nbs);

	for (uint32 i = 0; i < nbs; i++)
	{
		// find the service
		CService s;

		msgin.serial (s.Id, s.AliasName, s.ShortName, s.LongName, s.Ready);
		msgin.serialCont (s.Commands);
		s.Connected = true;

		if (!s.AliasName.empty())
		{
			SIT sit = aes->findService (s.AliasName, false);
			if (sit == aes->Services.end ())
			{
				aes->Services.push_back (s);
			}
			else
			{
				nlassert ("the service already exists with alias, update it");
				(*sit).setValues (s);
			}
		}
		else
		{
			SIT sit = aes->findService (s.Id, false);
			if (sit == aes->Services.end ())
			{
				aes->Services.push_back (s);
			}
			else
			{
				nlassert ("the service already exists with id, update it");
				(*sit).setValues (s);
			}
		}
	}
	
	displayServices ();

	//
	// Send the new list to all admin
	//

	CMessage msgout (CNetManager::getSIDA ("AS"), "SERVICE_LIST");
	uint32 nbaes = 1;
	msgout.serial (nbaes);
	msgout.serial (aes->Id);
	uint32 ss = aes->Services.size();
	msgout.serial (ss);

	for (SIT sit = aes->Services.begin(); sit != aes->Services.end(); sit++)
	{
		msgout.serial ((*sit).Id, (*sit).AliasName, (*sit).ShortName, (*sit).LongName);
		msgout.serial ((*sit).Ready, (*sit).Connected, (*sit).InConfig);
		msgout.serialCont ((*sit).Commands);
	}
	CNetManager::send ("AS", msgout, 0);
}

static void cbServiceAliasList (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	// get the service list from the admin exec and send the list to all admin client
	CAdminExecutorService *aes = (CAdminExecutorService*) (uint) from->appId();

	aes->ServiceAliasList.clear ();
	msgin.serialCont (aes->ServiceAliasList);

	nlinfo("send SAL to admin callback from aes");

	CMessage msgout (CNetManager::getSIDA ("AS"), "SAL");
	msgout.serial (aes->Id);
	msgout.serialCont (aes->ServiceAliasList);
	CNetManager::send ("AS", msgout, 0);
	
	nlinfo("new service alias list");
}*/


/*static void cbServiceIdentification (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	CAdminExecutorService *aes = (CAdminExecutorService*) (uint) from->appId();

	uint32 sid;
	string alias;

	msgin.serial (sid, alias);

	SIT sit;
	if (!alias.empty())
	{
		sit = aes->findService (alias, false);

		if (sit == aes->Services.end ())
		{
			// the alias is not found
			nlwarning ("new service with alias (%s) but not in my list", alias.c_str());

			aes->Services.push_back (CService ());
			sit = aes->Services.end();
			sit--;
		}
		else
		{
			// normal case
		}
	}
	else
	{
		sit = aes->findService (sid, false);

		if (sit == aes->Services.end ())
		{
			// normal case for unknown services
			nlwarning ("new service without alias and not in my list, add it");
		}
		else
		{
			nlwarning ("new service without alias is already in my list with id %d, add it", sid);
		}
		aes->Services.push_back (CService ());
		sit = aes->Services.end();
		sit--;
	}

	(*sit).Id = sid;
	(*sit).AliasName = alias;
	(*sit).Connected = true;
	msgin.serial ((*sit).ShortName, (*sit).LongName);
	msgin.serialCont ((*sit).Commands);

	nlinfo ("*:%d:%d is identified to be '%s' '%s' '%s'", aes->Id, sid, (*sit).AliasName.c_str(), (*sit).ShortName.c_str(), (*sit).LongName.c_str());

	// broadcast the message to all admin client
	CMessage msgout (CNetManager::getSIDA ("AS"), "SID");
	msgout.serial (aes->Id, sid, (*sit).AliasName, (*sit).ShortName, (*sit).LongName);
	msgout.serialCont ((*sit).Commands);

	CNetManager::send ("AS", msgout, 0);
}*/

/*static void cbServiceReady (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	CAdminExecutorService *aes = (CAdminExecutorService*) (uint) from->appId();

	uint32 sid;
	msgin.serial (sid);

	SIT sit = aes->findService(sid);
	(*sit).Ready = true;

	nlinfo ("*:%d:%d is ready", aes->Id, sid);

	// broadcast the message to all admin client
	CMessage msgout (CNetManager::getSIDA ("AS"), "SR");
	msgout.serial (aes->Id, sid);
	CNetManager::send ("AS", msgout, 0);

	// if we are in a script execution, continue
	if (StartAllServices)
		doNextStartAllServicesStep();
}*/

/*static void cbServiceConnection (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	CAdminExecutorService *aes = (CAdminExecutorService*) (uint) from->appId();

	uint32 sid;
	msgin.serial (sid);

	nlinfo ("*:%d:%d connected", aes->Id, sid);

	// don't do anything. we have to wait identification to add it in out lists


//	aes->Services.push_back (CService(sid));

	// broadcast the message to all admin client
	CMessage msgout (CNetManager::getSIDA ("AS"), "SC");
	msgout.serial (aes->Id, sid);
	CNetManager::send ("AS", msgout, 0);
}
*/
/*static void cbServiceDisconnection (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	CAdminExecutorService *aes = (CAdminExecutorService*) (uint) from->appId();

	uint32 sid;
	msgin.serial (sid);

	nlinfo ("*:%d:%d disconnected", aes->Id, sid);

	SIT sit = aes->findService(sid, false);
	
	// broadcast the message to all admin client
	CMessage msgout (CNetManager::getSIDA ("AS"), "SD");
	msgout.serial (aes->Id, sid);
	CNetManager::send ("AS", msgout, 0);

	// the service could disconnect before it's identification, in this case, we don't have it in the service list
	if (sit != aes->Services.end ())
	{
		if ((*sit).InConfig)
		{
			(*sit).Ready = (*sit).Connected = false;
			(*sit).Id = 0xFFFFFFFF;
			(*sit).ShortName = (*sit).LongName = "";
			(*sit).Commands.clear ();
		}
		else
		{
			// erase only if it's not a service in the config
			aes->Services.erase (sit);
		}
	}

	displayServices ();
}*/

void sendAESInformations (uint16 sid)
{
	AESIT aesit = findAES (sid);

	vector<string> informations;

	CMessage msgout("AES_INFO");
	
	//
	// send services that should be running on this AES
	//
	informations.clear ();
	MYSQL_ROW row = sqlQuery ("select name from service where server='%s'", (*aesit).Name.c_str());
	while (row != NULL)
	{
		string service = row[0];
		nlinfo ("Adding '%s' in registered services to AES-%hu", row[0], sid);
		informations.push_back (service);
		row = sqlNextRow ();
	}
	msgout.serialCont (informations);

	//
	// send variable alarms for services that should running on this AES
	//
	informations.clear ();
	row = sqlQuery ("select path, error_bound, alarm_order from variable where error_bound!=-1");
	while (row != NULL)
	{
		nlinfo ("Adding '%s' '%s' '%s' in alarm to AES-%hu", row[0], row[1], row[2], sid);
		informations.push_back (row[0]);
		informations.push_back (row[1]);
		informations.push_back (row[2]);
		row = sqlNextRow ();
	}
	msgout.serialCont (informations);
	
	//
	// send graph update for services that should running on this AES
	//
	informations.clear ();
	row = sqlQuery ("select path, graph_update from variable where graph_update!=0");
	while (row != NULL)
	{
		CVarPath varpath (row[0]);

		for(uint i = 0; i < varpath.Destination.size(); i++)
		{
			string a  = varpath.Destination[i].first, b = (*aesit).Shard;
			if(varpath.Destination[i].first == "*" || varpath.Destination[i].first == (*aesit).Shard)
			{
				CVarPath varpath2 (varpath.Destination[i].second);

				for(uint j = 0; j < varpath2.Destination.size(); j++)
				{
					string c  = varpath2.Destination[j].first, d = (*aesit).Name;
					if(varpath2.Destination[j].first == "*" || varpath2.Destination[j].first == (*aesit).Name)
					{
						nlinfo ("Adding '%s' '%s' in graph to AES-%hu", row[0], row[1], sid);
						informations.push_back (row[0]);
						informations.push_back (row[1]);
					}
				}
			}
		}
		row = sqlNextRow ();
	}
	msgout.serialCont (informations);
	
	nlinfo ("Sending all informations about %s AES-%hu (hostedservices, alarms,grapupdate)", (*aesit).Name.c_str(), (*aesit).SId);
	CUnifiedNetwork::getInstance ()->send (sid, msgout);
}

void rejectAES(uint16 sid, const string &res)
{
	CMessage msgout("REJECTED");
	msgout.serial ((string &)res);
	CUnifiedNetwork::getInstance ()->send (sid, msgout);
}

// i'm connected to a new admin executor service
static void cbNewAESConnection (const std::string &serviceName, uint16 sid, void *arg)
{
	TSockId from;
	CCallbackNetBase *cnb = CUnifiedNetwork::getInstance ()->getNetBase (sid, from);
	const CInetAddress &ia = cnb->hostAddress (from);

	AESIT aesit = findAES (sid, false);

	if (aesit != AdminExecutorServices.end ())
	{
		nlwarning ("Connection of an AES that already are in the list (%s)", ia.asString ().c_str ());
		rejectAES (sid, "This AES is already in the AS list");
		return;
	}

	MYSQL_ROW row = sqlQuery ("select name from server where address='%s'", ia.ipAddress().c_str());

	if (row == NULL)
	{
		nlwarning ("Connection of an AES that is not in database server list (%s)", ia.asString ().c_str ());
		rejectAES (sid, "This AES is not registered in the database");
		return;
	}

	string server = row[0];

	row = sqlQuery ("select shard from service where server='%s'", server.c_str());
	
	if (row == NULL)
	{
		nlwarning ("Connection of an AES that is not in database server list (%s)", ia.asString ().c_str ());
		rejectAES (sid, "This AES is not registered in the database");
		return;
	}
	
	string shard = row[0];
	
	
	AdminExecutorServices.push_back (CAdminExecutorService(shard, server, sid));

	nlinfo ("%s-%hu, server name %s, for shard %s connected and added in the list", serviceName.c_str(), sid, server.c_str(), shard.c_str());
	
	// send him services that should run on this server
	sendAESInformations (sid);
	//sendHostedServices(sid);
	//sendAlarms (sid);
	//sendGraphUpdate (sid);

/*
	// broadcast the message that an admin exec is connected to all admin client
	CMessage msgout (CNetManager::getSIDA ("AS"), "AESC");
	msgout.serial (aes->Id);
	CNetManager::send ("AS", msgout, 0);
*/
/*
	// broadcast the new state of this AES
	CMessage msgout (CNetManager::getSIDA ("AS"), "AES_LIST");
	uint32 nbaes = 1;
	msgout.serial (nbaes);
	msgout.serial (aes->Id, aes->ServerAlias, aes->ServerAddr, aes->Connected);
	CNetManager::send ("AS", msgout, 0);
*/}

// i'm disconnected to an admin executor service
static void cbNewAESDisconnection (const std::string &serviceName, uint16 sid, void *arg)
{
	TSockId from;
	CCallbackNetBase *cnb = CUnifiedNetwork::getInstance ()->getNetBase (sid, from);
	const CInetAddress &ia = cnb->hostAddress (from);

	AESIT aesit = findAES (sid, false);

	if (aesit == AdminExecutorServices.end ())
	{
		nlwarning ("Disconnection of %s-%hu that is not in my list (%s)", serviceName.c_str (), sid, ia.asString ().c_str ());
		return;
	}

	nlinfo ("%s-%hu, shard name %s, disconnected and removed from the list", serviceName.c_str(), sid, (*aesit).Name.c_str ());

	// we need to remove pending request

	for(uint i = 0; i < (*aesit).WaitingRequestId.size (); i++)
	{
		subRequestWaitingNb ((*aesit).WaitingRequestId[i]);
	}

	AdminExecutorServices.erase (aesit);
	
/*	
	// get the aes with the appid
	CAdminExecutorService *aes = (CAdminExecutorService*) (uint) from->appId();

	aes->Connected = false;

	SIT sit;
	for (sit = aes->Services.begin(); sit != aes->Services.end();)
	{
		// keep only inconfig services
		if ((*sit).InConfig)
		{
			(*sit).Id = 0xFFFFFFFF;
			(*sit).ShortName = (*sit).LongName = "";
			(*sit).Ready = (*sit).Connected = false;
			sit++;
		}
		else
		{
			// erase only if it's not a service in the config
			sit = aes->Services.erase (sit);
		}
	}

	nlinfo ("*:%d:* disconnected", aes->Id);
*//*	
	// broadcast the message to all admin client that an admin exec is disconnected
	CMessage msgout (CNetManager::getSIDA ("AS"), "AESD");
	msgout.serial (aes->Id);
	CNetManager::send ("AS", msgout, 0);
*/
/*
	displayServices ();

	// broadcast the new state of this AES
	CMessage msgout (CNetManager::getSIDA ("AS"), "AES_LIST");
	uint32 nbaes = 1;
	msgout.serial (nbaes);
	msgout.serial (aes->Id, aes->ServerAlias, aes->ServerAddr, aes->Connected);
	CNetManager::send ("AS", msgout, 0);
*/
}

/*static void cbLog (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	// received an answer for a command, give it to all admin client

	// get the aes with the appid
	CAdminExecutorService *aes = (CAdminExecutorService*) (uint) from->appId();

	// broadcast the message to the admin service
	CMessage msgout (CNetManager::getSIDA ("AS"), "XLOG");
	string log;
	uint32 sid;
	msgin.serial (sid);
	msgin.serial (log);

	msgout.serial (aes->Id);
	msgout.serial (sid);
	msgout.serial (log);
	CNetManager::send ("AS", msgout, 0);
}*/

static void cbView (CMessage &msgin, const std::string &serviceName, uint16 sid)
{
	uint32 rid;
	msgin.serial (rid);

	AESIT aesit = findAES (sid);

	for (uint i = 0; i < (*aesit).WaitingRequestId.size();)
	{
		if ((*aesit).WaitingRequestId[i] == rid)
		{
			(*aesit).WaitingRequestId.erase ((*aesit).WaitingRequestId.begin ()+i);
		}
		else
		{
			i++;
		}
	}

	MYSQL_ROW row = sqlQuery ("select distinct shard from service where server='%s'", (*aesit).Name.c_str ());

	// shard name is find using the "service" table, so, if there s no shard name in it, it returns ???
	string shardName;
	if (row != NULL) shardName = row[0];
	else shardName = "???";

	vector<string> vara, vala;

	while ((uint32)msgin.getPos() < msgin.length())
	{
		vara.clear ();
		vala.clear ();

		// adding default row
		vara.push_back ("shard");
		vara.push_back ("server");

		vala.push_back (shardName);
		vala.push_back ((*aesit).Name);

		uint32 i, nb;
		string var, val;

		msgin.serial (nb);
		for (i = 0; i < nb; i++)
		{
			msgin.serial (var);
			if (var == "__log")
			{
				vara.clear ();
				vala.clear ();
			}
			vara.push_back (var);
		}

		if (vara.size() > 0 && vara[0] == "__log")
			vala.push_back ("----- Result from Shard "+shardName+" Server "+(*aesit).Name+"\n");

		msgin.serial (nb);
		for (i = 0; i < nb; i++)
		{
			msgin.serial (val);
			vala.push_back (val);
		}
		addRequestAnswer (rid, vara, vala);
	}

	// inc the NbReceived counter
	addRequestReceived (rid);
}



TUnifiedCallbackItem CallbackArray[] =
{
	{ "VIEW", cbView },
	{ "ADMIN_EMAIL", cbAdminEmail },
	{ "GRAPH_UPDATE", cbGraphUpdate },
	
/*	{ "ESCR", cbExecuteSystemCommandResult },

	{ "SL", cbServiceList },
	{ "SID", cbServiceIdentification },
	{ "SR", cbServiceReady },
	{ "SC", cbServiceConnection },
	{ "SD", cbServiceDisconnection },

	{ "SAL", cbServiceAliasList },

	{ "XLOG", cbLog },*/
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////// CONNECTION TO THE CLIENT ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
//
// A new admin client is connected.
//
void clientConnection (const string &serviceName, TSockId from, void *arg)
{
	// new client, send him all out info about services

	nlinfo ("admin %s is connected", from->asString().c_str());

	CNetManager::getNetBase(serviceName)->authorizeOnly ("AUTH", from);
}

static void cbAuthenticateClient (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	//
	// Check the validity of the admin
	//
	
	string password;
	bool ok = false;
	try
	{
		msgin.serial (password);

		if (password == IService::getInstance()->ConfigFile.getVar("Password").asString())
		{
			// good authentification
			ok = true;
		}
	}
	catch (Exception &)
	{
		// problem, eject him
	}

	if (ok)
	{
		netbase.authorizeOnly (NULL, from);
		nlinfo ("Admin authentification success");
	}
	else
	{
		// bad auth => disconnect
		netbase.disconnect (from);
		nlwarning ("Bad admin authentification, tried password: '%s'", password.c_str());
		return;
	}

	//
	// send the list of all the aes
	//

	CMessage msgout2 (CNetManager::getSIDA ("AS"), "AES_LIST");
	AESIT aesit;
	uint32 nbaes = (uint32)AdminExecutorServices.size();
	msgout2.serial (nbaes);
	for (aesit = AdminExecutorServices.begin(); aesit != AdminExecutorServices.end(); aesit++)
	{
		// send info about the AES

		msgout2.serial ((*aesit).Id, (*aesit).ServerAlias, (*aesit).ServerAddr, (*aesit).Connected);
	}
	CNetManager::send ("AS", msgout2, from);

	//
	// send the list of all services
	//

	CMessage msgout (CNetManager::getSIDA ("AS"), "SERVICE_LIST");
	nbaes = (uint32)AdminExecutorServices.size();
	msgout.serial (nbaes);
	for (aesit = AdminExecutorServices.begin(); aesit != AdminExecutorServices.end(); aesit++)
	{
		msgout.serial ((*aesit).Id);

		uint32 nbs = (uint32)(*aesit).Services.size();
		msgout.serial (nbs);

		for (SIT sit = (*aesit).Services.begin(); sit != (*aesit).Services.end(); sit++)
		{
			// send info about services of the AES
			
			msgout.serial ((*sit).Id, (*sit).AliasName, (*sit).ShortName, (*sit).LongName);
			msgout.serial ((*sit).Ready, (*sit).Connected, (*sit).InConfig);
			msgout.serialCont ((*sit).Commands);
		}
	}
	CNetManager::send ("AS", msgout, from);

	displayServices ();

	//
	// send service alias list
	//

	nlinfo("send SAL to admin startup");

	for (aesit = AdminExecutorServices.begin(); aesit != AdminExecutorServices.end(); aesit++)
	{
		CMessage msgout2 (CNetManager::getSIDA ("AS"), "SAL");
		msgout2.serial ((*aesit).Id);
		msgout2.serialCont ((*aesit).ServiceAliasList);
		CNetManager::send ("AS", msgout2, from);
	}
}


static void cbExecuteSystemCommand (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	string command;
	uint32 aesid;
	msgin.serial (aesid);
	msgin.serial (command);

	AESIT aesit = findAdminExecutorService (aesid, false);
	if (aesit == AdminExecutorServices.end())
	{
		// don't find the aes, send an error message
		messageToClient (0, "couldn't execute command, as didn't find the aes", from);
		return;
	}

	// send the resquest to the AES

	CMessage msgout (CNetManager::getSIDA((*aesit).ServerAlias), "SYS");
	msgout.serial (command);
	CNetManager::send ((*aesit).ServerAlias, msgout);
}


static void cbStartService (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	string serviceAlias;
	uint32 aesid;
	msgin.serial (aesid);
	msgin.serial (serviceAlias);

	AESIT aesit = findAdminExecutorService (aesid, false);
	if (aesit == AdminExecutorServices.end())
	{
		// don't find the aes, send an error message
		messageToClient (0, "couldn't start service, as didn't find the aes", from);
		return;
	}

	// send the resquest to the AES

	CMessage msgout (CNetManager::getSIDA((*aesit).ServerAlias), "STARTS");
	msgout.serial (serviceAlias);
	CNetManager::send ((*aesit).ServerAlias, msgout);
}

static void cbStopService (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	uint32 aesid, sid;
	msgin.serial (aesid);
	msgin.serial (sid);

	AESIT aesit = findAdminExecutorService (aesid, false);
	if (aesit == AdminExecutorServices.end())
	{
		messageToClient (0, "couldn't stop service, as didn't find the aes", from);
		// don't find the aes, send an error message
		return;
	}

	SIT sit = (*aesit).findService (sid, false);
	if (sit == (*aesit).Services.end())
	{
		// don't find the s, send an error message
		messageToClient (0, "couldn't stop service, as didn't find the service", from);
		return;
	}

	// send the resquest to the AES

	CMessage msgout (CNetManager::getSIDA((*aesit).ServerAlias), "STOPS");
	msgout.serial (sid);
	CNetManager::send ((*aesit).ServerAlias, msgout);
}

static void cbExecCommand (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	uint32 aesid, sid;
	string command;
	msgin.serial (aesid);
	msgin.serial (sid);
	msgin.serial (command);

	AESIT aesit = findAdminExecutorService (aesid, false);
	if (aesit == AdminExecutorServices.end())
	{
		messageToClient (0, "couldn't stop service, as didn't find the aes", from);
		// don't find the aes, send an error message
		return;
	}

	SIT sit = (*aesit).findService (sid, false);
	if (sit == (*aesit).Services.end())
	{
		// don't find the s, send an error message
		messageToClient (0, "couldn't stop service, as didn't find the service", from);
		return;
	}

	// send the resquest to the AES

	CMessage msgout (CNetManager::getSIDA((*aesit).ServerAlias), "EXEC_COMMAND");
	msgout.serial (sid);
	msgout.serial (command);
	CNetManager::send ((*aesit).ServerAlias, msgout);
}

static void cbStartAllServices (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	initStartAllServices ();
}

static void cbStopAllServices (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	for (AESIT aesit = AdminExecutorServices.begin(); aesit != AdminExecutorServices.end(); aesit++)
	{
		for (SIT sit = (*aesit).Services.begin(); sit != (*aesit).Services.end(); sit++)
		{
			if ((*sit).Connected)
			{
				CMessage msgout (CNetManager::getSIDA((*aesit).ServerAlias), "STOPS");
				msgout.serial ((*sit).Id);
				CNetManager::send ((*aesit).ServerAlias, msgout);
			}
		}
	}
}

TCallbackItem ClientCallbackArray[] =
{
	{ "AUTH", cbAuthenticateClient },
	{ "SYS", cbExecuteSystemCommand },
	{ "STARTS", cbStartService },
	{ "STOPS", cbStopService },
	{ "EXEC_COMMAND", cbExecCommand },
	{ "START_ALL_SERVICES", cbStartAllServices },
	{ "STOP_ALL_SERVICES", cbStopAllServices },
};
*/




void addRequest (const string &rawvarpath, TSockId from)
{
	nlinfo ("addRequest from %s: '%s'", from->asString ().c_str (), rawvarpath.c_str ());

	if(rawvarpath.empty ())
	{
		// send an empty string to say to php that there's nothing
		string str;
		sendString (from, str);
	}

	//
	// special cases
	//

	if(rawvarpath == "reload")
	{
		// it means the we have to resend the list of services managed by AES from the mysql tables
		for (AESIT aesit = AdminExecutorServices.begin(); aesit != AdminExecutorServices.end(); aesit++)
		{
			sendAESInformations ((*aesit).SId);
			//sendHostedServices ((*aesit).SId);
			//sendAlarms((*aesit).SId);
			//sendGraphUpdate (sid);
		}

		// send an empty string to say to php that there's nothing
		string str;
		sendString (from, str);
		return;
	}

	//
	// normal cases
	//

	CVarPath varpath (rawvarpath);

	uint32 rid = newRequest (from);

	for (uint i = 0; i < varpath.Destination.size (); i++)
	{
		string shard = varpath.Destination[i].first;

		CVarPath subvarpath (varpath.Destination[i].second);

		for (uint j = 0; j < subvarpath.Destination.size (); j++)
		{
			string server = subvarpath.Destination[j].first;

			if (shard == "*" && server == "*")
			{
				// Send the request to all online servers of all online shards

				AESIT aesit;
				for (aesit = AdminExecutorServices.begin(); aesit != AdminExecutorServices.end(); aesit++)
				{
					addRequestWaitingNb (rid);
					(*aesit).WaitingRequestId.push_back (rid);

					CMessage msgout("AES_GET_VIEW");
					msgout.serial (rid);
					msgout.serial (subvarpath.Destination[j].second);
					CUnifiedNetwork::getInstance ()->send ((*aesit).SId, msgout);
					nlinfo ("REQUEST: Sent view '%s' to shard name %s 'AES-%hu'", subvarpath.Destination[j].second.c_str(), (*aesit).Name.c_str(), (*aesit).SId);
				}
			}
			else if (shard == "*" && server == "#")
			{
				// Select all shard all server including offline one
				
				MYSQL_ROW row = sqlQuery ("select distinct server, shard from service");
				
				while (row != NULL)
				{
					AESIT aesit = findAES (row[0], false);
					
					if (aesit != AdminExecutorServices.end())
					{
						addRequestWaitingNb (rid);
						(*aesit).WaitingRequestId.push_back (rid);
						
						CMessage msgout("AES_GET_VIEW");
						msgout.serial (rid);
						msgout.serial (subvarpath.Destination[j].second);
						CUnifiedNetwork::getInstance ()->send ((*aesit).SId, msgout);
						nlinfo ("REQUEST: Sent view '%s' to shard name %s 'AES-%hu'", subvarpath.Destination[j].second.c_str(), (*aesit).Name.c_str(), (*aesit).SId);
						
					}
					else if (server == "#")
					{
						vector<string> vara, vala;
						
						// adding default row
						vara.push_back ("shard");
						vala.push_back (row[1]);

						vara.push_back ("server");
						vala.push_back (row[0]);

						vara.push_back ("service");
						vala.push_back ("AES");
						
						vara.push_back ("State");
						vala.push_back ("Offline");
						
						addRequestAnswer (rid, vara, vala);
					}
					row = sqlNextRow ();
				}
			}
			else if (server == "*" || server == "#")
			{
				// Send the request to all online server of a specific shard

				MYSQL_ROW row = sqlQuery ("select distinct server from service where shard='%s'", shard.c_str ());

				while (row != NULL)
				{
					AESIT aesit = findAES (row[0], false);

					if (aesit != AdminExecutorServices.end())
					{
						addRequestWaitingNb (rid);
						(*aesit).WaitingRequestId.push_back (rid);

						CMessage msgout("AES_GET_VIEW");
						msgout.serial (rid);
						msgout.serial (subvarpath.Destination[j].second);
						CUnifiedNetwork::getInstance ()->send ((*aesit).SId, msgout);
						nlinfo ("REQUEST: Sent view '%s' to shard name %s 'AES-%hu'", subvarpath.Destination[j].second.c_str(), (*aesit).Name.c_str(), (*aesit).SId);

					}
					else if (server == "#")
					{
						vector<string> vara, vala;
						
						// adding default row
						vara.push_back ("shard");
						vala.push_back (shard);

						vara.push_back ("server");
						vala.push_back (row[0]);
						
						vara.push_back ("service");
						vala.push_back ("AES");
						
						vara.push_back ("State");
						vala.push_back ("Offline");

						addRequestAnswer (rid, vara, vala);
					}
					row = sqlNextRow ();
				}
			}
			else
			{
				AESIT aesit = findAES (server, false);

				if (aesit != AdminExecutorServices.end())
				{
					addRequestWaitingNb (rid);
					(*aesit).WaitingRequestId.push_back (rid);

					CMessage msgout("AES_GET_VIEW");
					msgout.serial (rid);
					msgout.serial (subvarpath.Destination[j].second);
					CUnifiedNetwork::getInstance ()->send ((*aesit).SId, msgout);
					nlinfo ("REQUEST: Sent view '%s' to shard name %s 'AES-%hu'", subvarpath.Destination[j].second.c_str(), (*aesit).Name.c_str(), (*aesit).SId);
				}
				else
				{
					nlwarning ("Server %s is not found in the list", server.c_str ());
				}
			}
		}
	}
}

static void varRequestTimeout(CConfigFile::CVar &var)
{
	RequestTimeout = var.asInt();
	nlinfo ("Request timeout is now after %d seconds", RequestTimeout);
}

static void varAdminAlertAccumlationTime (CConfigFile::CVar &var)
{
	AdminAlertAccumlationTime = var.asInt();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////// SERVICE IMPLEMENTATION //////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

class CAdminService : public IService
{
public:

	/// Init the service, load the universal time.
	void		init ()
	{
		setDefaultEmailParams (ConfigFile.getVar ("SMTPServer").asString (), "", "");

		sqlInit ();

		connectionWebInit ();

		//CVarPath toto ("[toto");

		//CVarPath toto ("*.*.*.*");
		//CVarPath toto ("[srv1,srv2].*.*.*");
		//CVarPath toto ("[svr1.svc1,srv2.svc2].*.*");
		//CVarPath toto ("[svr1.[svc1,svc2].*.var1,srv2.svc2.fe*.var2].toto");
		//CVarPath toto ("[svr1.svc1.*.toto,srv2.svc2.*.tata]");

//		CNetManager::setConnectionCallback ("AS", clientConnection, NULL);

		CUnifiedNetwork::getInstance ()->setServiceUpCallback ("AES", cbNewAESConnection);
		CUnifiedNetwork::getInstance ()->setServiceDownCallback ("AES", cbNewAESDisconnection);

		varRequestTimeout (ConfigFile.getVar ("RequestTimeout"));
		ConfigFile.setCallback("RequestTimeout", &varRequestTimeout);
		
		varAdminAlertAccumlationTime (ConfigFile.getVar ("AdminAlertAccumlationTime"));
		ConfigFile.setCallback("AdmimAlertAccumlationTime", &varAdminAlertAccumlationTime);
		
		//
		// Get the list of AESHosts, add in the structures and create connection to all AES
		//

		/*CConfigFile::CVar &host = ConfigFile.getVar ("AESHosts");
		sint i;
		for (i = 0 ; i < host.size (); i+=2)
		{
			string serverAlias = host.asString(i);
			string serverAddr = host.asString(i+1);

			// add to the list
			CAdminExecutorService aes;
			aes.ServerAlias = serverAlias;
			aes.ServerAddr = serverAddr;
			AdminExecutorServices.push_back (aes);

			// connect to the AES
			CNetManager::setConnectionCallback (serverAlias, cbAESConnection, NULL);
			CNetManager::setDisconnectionCallback (serverAlias, cbAESDisconnection, NULL);
			CNetManager::addClient (serverAlias, serverAddr+":49996");
			CNetManager::addCallbackArray (serverAlias, AESCallbackArray, sizeof (AESCallbackArray)/sizeof(AESCallbackArray[0]));
		}*/

		//
		// Get the list of services in the shard
		//
/*
		CConfigFile::CVar &serv = ConfigFile.getVar ("Services");
		for (i = 0 ; i < serv.size (); i+=2)
		{
			string serverAlias = serv.asString(i);
			string serviceAlias = serv.asString(i+1);

			AESIT aesit = findAdminExecutorService (serverAlias);

			// add new AES in the list
			CService s;
			s.AliasName = serviceAlias;
			s.InConfig = true;
			(*aesit).Services.push_back (s);
		}
		displayServices ();
*/	}

	bool update ()
	{
		cleanRequest ();
		connectionWebUpdate ();
		
		updateSendAdminAlert ();
		return true;
	}

	void release ()
	{
		connectionWebRelease ();
	}
};


/// Naming Service
NLNET_SERVICE_MAIN (CAdminService, "AS", "admin_service", 49996, CallbackArray, NELNS_CONFIG, NELNS_LOGS);


NLMISC_COMMAND (getViewAS, "send a view and receive an array as result", "<varpath>")
{
//	if(args.size() != 1) return false;

	string cmd;
	for (uint i = 0; i < args.size(); i++)
	{
		if (i != 0) cmd += " ";
		cmd += args[i];
	}

	addRequest (cmd, NULL);

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
		log.displayNL ("id: %d wait: %d recv: %d from: %s nbrow: %d", Requests[i].Id, Requests[i].NbWaiting, Requests[i].NbReceived, Requests[i].From->asString ().c_str (), Requests[i].NbRow);
	}
	log.displayNL ("End of display pending requests");

	return true;
}

NLMISC_COMMAND (generateAlert, "generate an alert", "<text>")
{
	if(args.size() != 1) return false;
	
	sendAdminAlert (args[0].c_str());

	return true;
}
