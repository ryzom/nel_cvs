/** \file login_service.cpp
 * Login Service (LS)
 *
 * $Id: login_service.cpp,v 1.15 2002/03/04 18:13:15 lecroart Exp $
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

#include <stdio.h>
#include <ctype.h>
#include <math.h>

#include <vector>
#include <map>

#include "nel/misc/types_nl.h"

#include "nel/misc/debug.h"
#include "nel/misc/config_file.h"
#include "nel/misc/displayer.h"
#include "nel/misc/command.h"
#include "nel/misc/log.h"
#include "nel/misc/window_displayer.h"

#include "nel/net/service.h"
#include "nel/net/net_manager.h"
#include "nel/net/login_cookie.h"

#include "login_service.h"
#include "connection_client.h"
#include "connection_ws.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

/* PlayerDatabaseName must contains an account per line separated by space or tab, each line should look like that: (without "")
 * "1 10 login password priv"
 * first param is 1 or 0. If 0, the account is disable, 1 is enable.
 * second param is the unic user id
 * login is the login, password is the passord...
 * priv is the privilege for a account. Some shard could need priviliege to have access on it. priv look like ":BETA:" for example
 */

//
// Variables
//

uint32 ServerVersion = 1;
bool AcceptNewUser = true;
bool AcceptExternalShard = true;
bool CryptPassword = true;

// store specific user information
NLMISC::CFileDisplayer Fd ("login_service.stat");
NLMISC::CStdDisplayer Sd;
NLMISC::CLog Output;

uint32 CUser::NextUserId = 1;	// 0 is reserved

// Variables

vector<CUser>	Users;
vector<CShard>	Shards;

IService *ServiceInstance = NULL;

const char		*PlayerDatabaseName = "login_service.udb";

//
// Functions
//


// transform "192.168.1.1:80" into "192.168.1.1"
string removePort (const string &addr)
{
	return addr.substr (0, addr.find (":"));
}

void checkClients ()
{
	nldebug ("checkClients ()");
	for (uint i = 0; i < Users.size (); i++)
	{
		switch (Users[i].State)
		{
		case CUser::Offline:
			nlassert (Users[i].SockId == NULL);
			nlassert (!Users[i].Cookie.isValid());
			nlassert (Users[i].ShardId == NULL);
			break;
		case CUser::Authorized:
			nlassert (Users[i].SockId != NULL);
			nlassert (Users[i].Cookie.isValid());
			nlassert (Users[i].ShardId == NULL);
			break;
		case CUser::Awaiting:
			nlassert (Users[i].SockId == NULL);
			nlassert (!Users[i].Cookie.isValid());
			nlassert (Users[i].ShardId == NULL);
			break;
		case CUser::Online:
			nlassert (Users[i].SockId == NULL);
			nlassert (!Users[i].Cookie.isValid());
			nlassert (Users[i].ShardId != NULL);
			break;
		default:
			nlstop;
			break;
		}
	}
}

void disconnectClient (CUser &user, bool disconnectClient, bool disconnectShard)
{
	nlinfo ("User %d '%s' need to be disconnect (%d %d %d)", user.Id, user.Login.c_str(), user.State, disconnectClient, disconnectShard);

	switch (user.State)
	{
	case CUser::Offline:
		break;

	case CUser::Authorized:
		if (disconnectClient)
		{
			CNetManager::getNetBase("LS")->disconnect(user.SockId);
		}
		user.Cookie.clear ();
		break;

	case CUser::Awaiting:
		break;

	case CUser::Online:
		if (disconnectShard)
		{
			// ask the WS to disconnect the player from the shard
			CMessage msgout (CNetManager::getNetBase("WSLS")->getSIDA (), "DC");
			msgout.serial (user.Id);
			CNetManager::send ("WSLS", msgout, user.ShardId);
		}
		user.ShardId = NULL;
		break;

	default:
		nlstop;
		break;
	}

	user.SockId = NULL;
	user.State = CUser::Offline;
}

sint findUser (uint32 Id)
{
	for (sint i = 0; i < (sint) Users.size (); i++)
	{
		if (Users[i].Id == Id)
		{
			return i;
		}
	}
	// user not found
	return -1;
}

string CUser::Authorize (TSockId sender, CCallbackNetBase &netbase)
{
	string reason;

	switch (State)
	{
	case Offline:
		State = Authorized;
		SockId = sender;
		Cookie = CLoginCookie(netbase.hostAddress(sender).internalIPAddress(), Id);
		break;

	case Authorized:
		nlwarning ("user %d already authorized! disconnect him and the other one", Id);
		reason = "You are already authorized (another user uses your account?)";
		disconnectClient (*this, true, true);
		disconnectClient (Users[findUser(Id)], true, true);
		break;

	case Awaiting:
		nlwarning ("user %d already awaiting! disconnect the new user and the other one", Id);
		reason = "You are already awaiting (another user uses your account?)";
		disconnectClient (*this, true, true);
		disconnectClient (Users[findUser(Id)], true, true);
		break;

	case Online:
		nlwarning ("user %d already online! disconnect the new user and the other one", Id);
		reason = "You are already online (another user uses your account?)";
		disconnectClient (*this, true, true);
		disconnectClient (Users[findUser(Id)], true, true);
		break;

	default:
		reason = "default case should never occurs, there's a bug in the login_service.cpp";
		nlstop;
		break;
	}
	return reason;
}

void displayShards ()
{
	ICommand::execute ("shards", *InfoLog);
}

void displayUsers ()
{
	ICommand::execute ("users", *InfoLog);
}

void readPlayerDatabase ()
{
	nlinfo("Loading the player database...");

	// load users
	sint i, k;
	for (i = 0; i < (sint)Users.size (); i++)
		Users[i].Loaded = false;

	FILE *fp = fopen (PlayerDatabaseName, "rt");
	if (fp == NULL)
	{
		nlwarning ("Can't read the user data base (%s)", PlayerDatabaseName);
	}
	else
	{
		vector<string> userStr;
		while (!feof (fp))
		{
			char activated[1024];
			char uid[1024];
			char login[1024];
			char password[1024];
			char perm[1024];
			if (fscanf (fp, "%s %s %s %s %s", activated, uid, login, password, perm) != 5)
			{
				nlwarning ("bad user data base format should be: uid login pass perm");
				break;
			}
			userStr.push_back (activated);
			userStr.push_back (uid);
			userStr.push_back (login);
			userStr.push_back (password);
			userStr.push_back (perm);
		}
		fclose (fp);

		for (uint i = 0; i < userStr.size(); i+=5)
		{
			for (k = 0; k < (sint)Users.size (); k++)
			{
				if (Users[k].Login == userStr[i+1])
				{
					nldebug("Update user '%d' from '%s' '%s' '%s' to '%s' '%s' '%s'", Users[k].Id, Users[k].Login.c_str(), Users[k].Password.c_str(), Users[k].ShardPrivilege.c_str(), userStr[i+1].c_str(), userStr[i+2].c_str(), userStr[i+3].c_str());
					Users[k].Loaded = true;
					Users[k].Active = atoi(userStr[i].c_str()) == 1;
					Users[k].Id = atoi(userStr[i+1].c_str());
					Users[k].Password = userStr[i+3];
					Users[k].ShardPrivilege = userStr[i+4];
					if (Users[k].Id >= CUser::NextUserId) CUser::NextUserId = Users[k].Id + 1;
					break;
				}
			}
			if (k == (sint)Users.size())
			{
				// new user
				nldebug("New user '%s' '%s' '%s' '%s'", userStr[i+1].c_str(), userStr[i+2].c_str(), userStr[i+3].c_str(), userStr[i+4].c_str());
				Users.push_back (CUser(atoi(userStr[i].c_str()) == 1, atoi(userStr[i+1].c_str()), userStr[i+2], userStr[i+3], userStr[i+4]));
			}
		}

		// delete not loaded user
		for (vector<CUser>::iterator uit = Users.begin(); uit < Users.end(); )
		{
			if (!(*uit).Loaded)
			{
				nldebug("Deleting user '%d' '%s'", (*uit).Id, (*uit).Login.c_str());
				uit = Users.erase (uit);
			}
			else
			{
				uit++;
			}
		}
		
		nlinfo ("%d users in the database, next user id is %d", Users.size (), CUser::NextUserId);
	}

	// load shards
	for (i = 0; i < (sint)Shards.size (); i++)
		Shards[i].Loaded = false;

	const CConfigFile::CVar &v2 = IService::Instance->ConfigFile.getVar ("Shards");
	for (i = 0; i < v2.size(); i+=2)
	{
		try
		{
			for (k = 0; k < (sint)Shards.size (); k++)
			{
				if (removePort(Shards[k].WSAddr) == v2.asString(i) && !Shards[k].Loaded)
				{
					nldebug("Update shard '%s' from '%s' to '%s'", Shards[k].WSAddr.c_str (), Shards[k].Name.c_str(), v2.asString(i+1).c_str());
					Shards[k].Loaded = true;
					Shards[k].Name = v2.asString(i+1);
					break;
				}
			}
			if (k == (sint)Shards.size())
			{
				// new shard
				nldebug("New shard '%s' '%s'", v2.asString(i).c_str(), v2.asString(i+1).c_str());
				Shards.push_back (CShard(v2.asString(i), v2.asString(i+1)));
			}
		}
		catch(ESocket &)
		{
			nlwarning("Couldn't resolve the address '%s', remove it from the config file", v2.asString(i).c_str());
		}
	}
	
	// delete not loaded shard
	for (vector<CShard>::iterator sit = Shards.begin(); sit < Shards.end(); )
	{
		if (!(*sit).Loaded)
		{
			nldebug("Deleting shard '%s' '%s'", (*sit).WSAddr.c_str (), (*sit).Name.c_str());
			sit = Shards.erase (sit);
		}
		else
		{
			sit++;
		}
	}
	
	nlinfo ("%d Shards in the database", Shards.size ());

	displayUsers ();
	displayShards ();
}

void writePlayerDatabase ()
{
	nlinfo("Writing the player database...");

	FILE *fp = fopen (PlayerDatabaseName, "wt");
	if (fp == NULL)
	{
		nlwarning ("Can't write the user data base! (%s)", PlayerDatabaseName);
		return;
	}

	for (sint i = 0; i < (sint) Users.size (); i++)
	{
		fprintf (fp, "%d %6d %-20s %-20s %-20s\n", Users[i].Active?1:0, Users[i].Id, Users[i].Login.c_str(), Users[i].Password.c_str(), Users[i].ShardPrivilege.c_str());
	}

	fclose (fp);

	nlinfo ("%d users saved", Users.size ());
}


////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
/////////////// SERVICE IMPLEMENTATION /////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

void beep (uint freq, uint nb, uint beepDuration, uint pauseDuration)
{
#ifdef NL_OS_WINDOWS
	if (ServiceInstance == NULL)
		return;

	try
	{
		if (ServiceInstance->ConfigFile.getVar ("Beep").asInt() == 1)
		{
			for (uint i = 0; i < nb; i++)
			{
				Beep (freq, beepDuration);
				nlSleep (pauseDuration);
			}
		}
	}
	catch (Exception &)
	{
	}
#endif // NL_OS_WINDOWS
}

void cbVar (CConfigFile::CVar &var)
{
	if (var.Name == "ServerVersion") ServerVersion = var.asInt();
	else if (var.Name == "AcceptNewUser") AcceptNewUser = var.asInt() == 1;
	else if (var.Name == "AcceptExternalShard") AcceptExternalShard = var.asInt() == 1;
	else if (var.Name == "CryptPassword") CryptPassword = var.asInt() == 1;
	else nlstop;
}

class CLoginService : public NLNET::IService
{
	bool Init;

public:

	CLoginService () : Init(false) {};

	/// Init the service, load the universal time.
	void init ()
	{
		ServiceInstance = this;

		beep ();

		readPlayerDatabase ();

		Output.addDisplayer (&Fd);
		if (_WindowDisplayer != NULL)
			Output.addDisplayer (_WindowDisplayer);

		connectionClientInit ();
		
		//
		// Get Config file variables
		//

		uint16 port = 49998;
		try
		{
			port = ConfigFile.getVar("WSPort").asInt();
		}
		catch (Exception &)
		{
		}
		connectionWSInit (port);

		ConfigFile.setCallback ("ServerVersion", cbVar);
		cbVar (ConfigFile.getVar ("ServerVersion"));
		ConfigFile.setCallback ("AcceptNewUser", cbVar);
		cbVar (ConfigFile.getVar ("AcceptNewUser"));
		ConfigFile.setCallback ("AcceptExternalShard", cbVar);
		cbVar (ConfigFile.getVar ("AcceptExternalShard"));
		ConfigFile.setCallback ("CryptPassword", cbVar);
		cbVar (ConfigFile.getVar ("CryptPassword"));

		Init = true;
		
		Output.displayNL ("Login Service initialised");
	}

	/// release the service, save the universal time
	void release ()
	{
		if (Init)
		{
			writePlayerDatabase ();
		}

		Output.displayNL ("Login Service released");
	}
};


// Service instanciation
NLNET_SERVICE_MAIN (CLoginService, "LS", "login_service", 49999, EmptyCallbackArray);


//
// Variables
//

NLMISC_DYNVARIABLE(uint32, online_users_nb, "number of connected users")
{
	// we can only read the value
	if (get)
	{
		uint32 nbusers = 0;
		for (uint i = 0; i < Users.size(); i++)
		{
			if (Users[i].State == CUser::Online)
				nbusers++;
		}
		*pointer = nbusers;
	}
}


//
// Commands
//

NLMISC_COMMAND (shards, "displays the list of all registered shards", "")
{
	if(args.size() != 0) return false;


	log.displayNL ("Display the %d registered shards :", Shards.size());
	for (uint i = 0; i < Shards.size(); i++)
	{
		log.displayNL ("> '%s' %d %d '%s' '%s' '%s'", Shards[i].Name.c_str(), Shards[i].Online, Shards[i].NbPlayers, Shards[i].SockId->asString().c_str(), Shards[i].WSAddr.c_str(), Shards[i].ShardName.c_str());
	}
	log.displayNL ("End ot the list");

	checkClients ();

	return true;
}

NLMISC_COMMAND (registered_users, "displays the list of all registered users", "")
{
	if(args.size() != 0) return false;

	log.displayNL ("Display the %d registered users :", Users.size());
	for (uint i = 0; i < Users.size(); i++)
	{
		log.displayNL ("> %d %d '%s' '%s' '%s' '%s' '%s'", Users[i].Id, Users[i].State, Users[i].Login.c_str(), Users[i].Cookie.toString().c_str(), Users[i].SockId->asString().c_str(), Users[i].ShardId->asString().c_str(), Users[i].ShardPrivilege.c_str());
	}
	log.displayNL ("End ot the list");

	checkClients ();

	return true;
}

NLMISC_COMMAND (online_users, "displays the list of online users", "")
{
	if(args.size() != 0) return false;

	uint32 nbusers = 0, nbauth = 0, nbwait = 0;
	log.displayNL ("Display the online users :", Users.size());
	for (uint i = 0; i < Users.size(); i++)
	{
		if (Users[i].State == CUser::Online)
		{
			log.displayNL ("> %d '%s' '%s' '%s' '%s' '%s'", Users[i].Id, Users[i].Login.c_str(), Users[i].Cookie.toString().c_str(), Users[i].SockId->asString().c_str(), Users[i].ShardId->asString().c_str(), Users[i].ShardPrivilege.c_str());
			nbusers++;
		}
		else if (Users[i].State == CUser::Awaiting) nbwait++;
		else if (Users[i].State == CUser::Authorized) nbauth++;
	}
	log.displayNL ("End ot the list (%d online users, %d authorized, %d awaiting)", nbusers, nbauth, nbwait);

	checkClients ();

	return true;
}
