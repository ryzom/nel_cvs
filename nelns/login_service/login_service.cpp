/** \file login_service.cpp
 * Login Service (LS)
 *
 * $Id: login_service.cpp,v 1.4 2001/05/18 16:51:01 lecroart Exp $
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
#include "nel/misc/log.h"

#include "nel/net/service.h"
#include "nel/net/net_manager.h"
#include "nel/net/login_cookie.h"

#include "login_service.h"
#include "connection_client.h"
#include "connection_ws.h"


using namespace std;
using namespace NLMISC;
using namespace NLNET;


// store specific user information
NLMISC::CFileDisplayer Fd ("ls.log");
NLMISC::CLog Output;

uint32 CUser::NextUserId = 1;	// 0 is reserved

// Variables

vector<CUser>	Users;
vector<CShard>	Shards;

// config file name that load and save the universal time
const char		*PlayerDatabaseName = "login_service_database.cfg";
CConfigFile		*PlayerDatabase;

// Functions

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
	nlinfo ("disconnectClient (): %s %d %d %d", user.Login.c_str(), user.State, disconnectClient, disconnectShard);

	displayUsers ();

	switch (user.State)
	{
	case CUser::Offline:
		break;

	case CUser::Authorized:
		if (disconnectClient)
		{
			CNetManager::getNetBase("LS")->disconnect(user.SockId);
		}
		user.SockId = NULL;
		user.Cookie.clear ();
		break;

	case CUser::Awaiting:
		user.ShardId = NULL;
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

	user.State = CUser::Offline;

	displayUsers ();
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

bool CUser::Authorize (TSockId sender, CCallbackNetBase &netbase)
{
	bool ok = false;

	displayUsers ();

	switch (State)
	{
	case Offline:
		State = Authorized;
		SockId = sender;
		Cookie = CLoginCookie(netbase.hostAddress(sender).internalIPAddress(), Id);
		ok = true;
		break;

	case Authorized:
		nlwarning ("user %d already authorized! disconnect him and the other one");
		disconnectClient (*this, true, true);
		disconnectClient (Users[findUser(Id)], true, true);
		break;

	case Awaiting:
		nlwarning ("user %d already awaiting! disconnect the new user and the other one", Id);
		disconnectClient (*this, true, true);
		disconnectClient (Users[findUser(Id)], true, true);
		break;

	case Online:
		nlwarning ("user %d already online! disconnect the new user and the other one", Id);
		disconnectClient (*this, true, true);
		disconnectClient (Users[findUser(Id)], true, true);
		break;

	default:
		nlstop;
		break;
	}
	displayUsers ();
	return ok;
}

void displayShards ()
{
	nlinfo ("There's %d shards in the list:", Shards.size());
	for (uint i = 0; i < Shards.size(); i++)
	{
		nlinfo(" > %s %d %d %s %s", Shards[i].Name.c_str(), Shards[i].Online, Shards[i].NbPlayers, Shards[i].SockId->asString().c_str(), Shards[i].WSAddr.c_str());
	}
	nlinfo ("End of the list");

	checkClients ();
}

void displayUsers ()
{
	nlinfo ("There's %d users in the list:", Users.size());
	for (uint i = 0; i < Users.size(); i++)
	{
		nlinfo(" > %d %d %s %s '%s' '%s'", Users[i].Id, Users[i].State, Users[i].Login.c_str(), Users[i].Cookie.toString().c_str(), Users[i].SockId->asString().c_str(), Users[i].ShardId->asString().c_str());
	}
	nlinfo ("End of the list");

	checkClients ();
}

void readPlayerDatabase ()
{
	nlinfo("Loading the player database...");

	// load users
	sint i, k;
	for (i = 0; i < (sint)Users.size (); i++)
		Users[i].Loaded = false;

	const CConfigFile::CVar &v = PlayerDatabase->getVar ("Users");
	for (i = 0; i < v.size(); i+=3)
	{
		for (k = 0; k < (sint)Users.size (); k++)
		{
			if (Users[k].Login == v.asString(i))
			{
				nldebug("Update user '%s' from '%s' '%d' to '%s' '%s'", Users[k].Login.c_str(), Users[k].Password.c_str(), Users[k].Id, v.asString(i+1).c_str(), v.asString(i+2).c_str());
				Users[k].Loaded = true;
				Users[k].Password = v.asString(i+1);
				Users[k].Id = atoi(v.asString(i+2).c_str());
				if (Users[k].Id >= CUser::NextUserId) CUser::NextUserId = Users[k].Id + 1;
				break;
			}
		}
		if (k == (sint)Users.size())
		{
			// new user
			nldebug("New user '%s' '%s' '%s'", v.asString(i).c_str(), v.asString(i+1).c_str(), v.asString(i+2).c_str());
			Users.push_back (CUser(v.asString(i), v.asString(i+1), atoi(v.asString(i+2).c_str())));
		}
	}

	// delete not loaded user
	for (vector<CUser>::iterator uit = Users.begin(); uit < Users.end(); )
	{
		if (!(*uit).Loaded)
		{
			nldebug("Deleting user '%s' '%s' '%d'", (*uit).Login.c_str(), (*uit).Password.c_str(), (*uit).Id);
			uit = Users.erase (uit);
		}
		else
		{
			uit++;
		}
	}
	
	nlinfo ("%d users in the database, next user id is %d", Users.size (), CUser::NextUserId);

	// load shards
	for (i = 0; i < (sint)Shards.size (); i++)
		Shards[i].Loaded = false;

	const CConfigFile::CVar &v2 = PlayerDatabase->getVar ("Shards");
	for (i = 0; i < v2.size(); i+=2)
	{
		try
		{
			for (k = 0; k < (sint)Shards.size (); k++)
			{
				if (Shards[k].WSAddr == v2.asString(i))
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
		catch(ESocket)
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
	if (fp != NULL)
	{
		sint i;

		fprintf (fp, "Users = {\n");
		for (i = 0; i < (sint) Users.size (); i++)
		{
			fprintf (fp, " \"%s\",\"%s\",\"%d\"%c\n", Users[i].Login.c_str(), Users[i].Password.c_str(), Users[i].Id, (i==(sint)Users.size()-1)?' ':',');
		}
		fprintf (fp, "};\n");

		nlinfo ("%d users saved", Users.size ());


		fprintf (fp, "\nShards = {\n");
		for (i = 0; i < (sint) Shards.size (); i++)
		{
			fprintf (fp, " \"%s\", \"%s\"%c\n", Shards[i].WSAddr.c_str (), Shards[i].Name.c_str (), (i==(sint)Shards.size()-1)?' ':',');
		}
		fprintf (fp, "};\n");

		nlinfo ("%d shards saved", Shards.size ());

		fclose (fp);

		PlayerDatabase->setLastModifiedNow ();
	}

	displayShards ();
}


////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
/////////////// SERVICE IMPLEMENTATION /////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

class CLoginService : public NLNET::IService
{
	bool Init;

public:

	CLoginService () : Init(false) {};

	/// Init the service, load the universal time.
	void init ()
	{
		FILE *fp = fopen (PlayerDatabaseName, "rt");
		if (fp == NULL)
		{
			// the file not exist, create it and init the universal time to 0
			nlwarning("'%s' not found, create it", PlayerDatabaseName);
			writePlayerDatabase ();
		}
		else fclose (fp);

		// load config file
		PlayerDatabase = new CConfigFile;
		PlayerDatabase->load (PlayerDatabaseName);
		PlayerDatabase->setCallback (readPlayerDatabase);
		readPlayerDatabase ();

		Output.addDisplayer (&Fd);

		connectionClientInit ();
		
		connectionWSInit ();

		Init = true;
	}

	/// release the service, save the universal time
	void release ()
	{
		if (PlayerDatabase != NULL && Init)
		{
			writePlayerDatabase ();
			delete PlayerDatabase;
		}
	}
};


/// Naming Service
NLNET_SERVICE_MAIN (CLoginService, "LS", "login_service", 49999, EmptyCallbackArray);
