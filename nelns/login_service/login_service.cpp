/** \file login_service.cpp
 * Login Service (LS)
 *
 * $Id: login_service.cpp,v 1.1 2001/04/18 13:54:25 valignat Exp $
 *
 * \warning If a player starts a connection from a shard ip and the shard is offline, login_service will think that the the player is the shard, so you can't launch a client from a shard ip.
 * \todo manage a hello message from the shard to identify him, can t use the connection message because can t distinguish client or shard at the connection taime
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

#include "nel/net/msg_socket.h"
#include "nel/net/service.h"

#include "nel/net/unitime.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

#ifdef NL_OS_UNIX
extern "C" char *crypt (const char *__key, const char *__salt);
#endif

/// Set the version of the server. you have to increase it each time the client-server protocol changes.
/// You have to increment the client too (the server and client version must be the same)
static const uint32 ServerVersion = 1;


// 1 if you want to add new player, 0 if you want to eject new player
#define ACCEPT_NEW_USER 1
#define ACCEPT_EXTERNAL_SHARD 1

#define CRYPT_PASSWORD 1

// store specific user information
NLMISC::CFileDisplayer Fd ("ls.log");
NLMISC::CLog Output;


// Structures

struct CUser
{
	// next user id for new users
	static uint32	NextUserId;

	// next key for new user connection
	static uint32	NextKey;

	string		Login;
	string		Password;
	uint32		Id;				// User ID, it's a unique number linked to a player for all his live

	bool		Authorized;		// true if the login password verification granted
	uint32		Key;			// Temp key for the connection
	TSenderId	SenderId;		// Connection to the user

	bool		Online;			// true if the user is on a shard
	TSenderId	ShardId;		// the shard that the user connected to
	
	CUser (string login) : Login(login) { Authorized = Online = false;}
	CUser (string login, string password) : Login(login), Password(password) { Id = NextUserId++; Authorized = Online = false; Loaded = true; }
	CUser (string login, string password, uint32 id) : Login(login), Password(password), Id(id), Authorized(false), Loaded(true), Online(false)
	{
		if (id >= CUser::NextUserId)
			CUser::NextUserId = id + 1;
	}

	void Authorize (TSenderId sender)
	{
		if (Authorized)
		{
			nlwarning ("user %d already authorized! disconnect the new user", Id);
			// disconnect him!!!
			CMsgSocket::close (sender);
			return;
		}
		else if (Online)
		{
			nlwarning ("user %d already online! disconnect the new user", Id);
			// disconnect him!!!
			CMsgSocket::close (sender);
			return;
		}
		Authorized = true;
		SenderId = sender;
		Key = NextKey++;
	}


	// use when reloading config file
	bool Loaded;
};

uint32 CUser::NextKey = 0;
uint32 CUser::NextUserId = 1;	// 0 is reserved

struct CShard
{
	string			Name;
	CInetAddress	Address;
	bool			Online;
	TSenderId		SenderId;	// Connection of to the shard
	sint			NbPlayers;

	CShard (const CInetAddress &ia)
	{
		Name = "External Shard '"+ia.hostName()+"'";
		Address = ia;
		Online = false;
		Loaded = true;
		NbPlayers = 0;
	}

	CShard (string address)
	{
		Name = "<Unknown>";
		Address = CInetAddress (address, 0);
		Online = false;
		Loaded = true;
		NbPlayers = 0;
	}

	CShard (string address, string name)
	{
		Name = name;
		Address = CInetAddress (address, 0);
		Online = false;
		Loaded = true;
		NbPlayers = 0;
	}


	// use when reloading config file
	bool Loaded;
};

// Variables

vector<CUser>	Users;
vector<CShard>	Shards;

// config file name that load and save the universal time
const char		*ConfigFileName = "ls.txt";
CConfigFile		*ConfigFile;

// Functions

// These functions enable crypting password, work only on unix

const uint32 EncryptedSize = 13;

// Get a number between 0 and 64, used by cryptPassword
static uint32 rand64 ()
{
	return (uint32) floor(64.0*(double)rand()/((double)RAND_MAX+1.0));
}

// Crypt a password
string cryptPassword (const string &password)
{
#if defined(NL_OS_UNIX) && CRYPT_PASSWORD
	char Salt[3];
	static char SaltString[65] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";

	Salt[0] = SaltString[rand64()];
	Salt[1] = SaltString[rand64()];
	Salt[2] = '\0';

	return string (crypt (password.c_str(), Salt));
#else
	return password;
#endif
}

// Check if a password is valid
bool checkPassword (const string &password, const string &encrypted)
{
#if defined(NL_OS_UNIX) && CRYPT_PASSWORD
	char Salt[3];

	if (encrypted.size() != EncryptedSize)
	{
		nlwarning ("checkPassword(): \"%s\" is not a valid encrypted password", encrypted.c_str());
		return false;
	}

	Salt[0] = encrypted[0];
	Salt[1] = encrypted[1];
	Salt[2] = '\0';

	return encrypted == crypt (password.c_str(), Salt);
#else
	return encrypted == password;
#endif
}

void loadConfigFile ()
{
	nlinfo("Loading the config file...");

	// load users
	sint i, k;
	for (i = 0; i < (sint)Users.size (); i++)
		Users[i].Loaded = false;

	const CConfigFile::CVar &v = ConfigFile->getVar ("Users");
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

	const CConfigFile::CVar &v2 = ConfigFile->getVar ("Shards");
	for (i = 0; i < v2.size(); i+=2)
	{
	  try
	    {
	      CInetAddress newshard(v2.asString(i));
	      
	      for (k = 0; k < (sint)Shards.size (); k++)
		{
		  if (Shards[k].Address.ipAddress() == newshard.ipAddress())
		    {
		      nldebug("Update shard '%s' from '%s' to '%s'", Shards[k].Address.hostName().c_str (), Shards[k].Name.c_str(), v2.asString(i+1).c_str());
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
			nldebug("Deleting shard '%s' '%s'", (*sit).Address.hostName().c_str (), (*sit).Name.c_str());
			sit = Shards.erase (sit);
		}
		else
		{
			sit++;
		}
	}
	
	nlinfo ("%d Shards in the database", Shards.size ());
}

void writeConfigFile ()
{
	nlinfo("Writing the config file...");

	FILE *fp = fopen (ConfigFileName, "wt");
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
			fprintf (fp, " \"%s\", \"%s\"%c\n", Shards[i].Address.hostName().c_str (), Shards[i].Name.c_str (), (i==(sint)Shards.size()-1)?' ':',');
		}
		fprintf (fp, "};\n");

		nlinfo ("%d shards saved", Shards.size ());

		fclose (fp);

		if (ConfigFile != NULL)
			ConfigFile->setLastModifiedNow ();
	}
}

sint findShard (TSenderId senderId)
{
	for (sint i = 0; i < (sint) Shards.size (); i++)
	{
		if (Shards[i].SenderId == senderId)
		{
			return i;
		}
	}
	// shard not found
	return -1;
}

sint findUser (string &login)
{
	for (sint i = 0; i < (sint) Users.size (); i++)
	{
		if (Users[i].Login == login)
		{
			return i;
		}
	}
	// user not found
	return -1;
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

void addUser (string &login, string &password)
{
	if (findUser (login) == -1)
	{
		Users.push_back (CUser (login, cryptPassword(password)));
		writeConfigFile ();
	}
	else
	{
		nlwarning ("user '%s' already exists in the base", login.c_str ());
	}
}

class CLoginService : public NLNET::IService
{
	bool Init;

public:

	CLoginService () : Init(false) {};

	/// Init the service, load the universal time.
	void init ()
	{
		FILE *fp = fopen (ConfigFileName, "rt");
		if (fp == NULL)
		{
			// the file not exist, create it and init the universal time to 0
			nlwarning("'%s' not found, create it", ConfigFileName);
			writeConfigFile ();
		}
		else fclose (fp);

		// load config file
		ConfigFile = new CConfigFile;
		ConfigFile->load (ConfigFileName);
		ConfigFile->setCallback (loadConfigFile);
		loadConfigFile ();

		Output.addDisplayer (&Fd);

		Init = true;
	}

	/// release the service, save the universal time
	void release ()
	{
		if (ConfigFile != NULL && Init)
		{
			writeConfigFile ();
			delete ConfigFile;
		}
	}
};

bool stringIsStandard(const string &str)
{
	for (sint i = 0; i < (sint) str.size(); i++)
	{
		if (!isalnum (str[i])) return false;
	}
	return true;
}

sint userToLog(sint userPos)
{
	if (userPos == -1) return userPos;
	else return Users[userPos].Id;
}

/**
 * Callback for service unregistration.
 *
 * Message expected :
 * - nothing
 */
void cbVerifyLoginPassword( CMessage& msgin, TSenderId from )
{
	uint8 ok;
	string err;

	string Login, Password;
	msgin.serial (Login);
	msgin.serial (Password);

	sint userPos = findUser (Login);
	const CInetAddress *ia = CMsgSocket::addressFromId (from);

	Output.displayNL ("***: %3d Login '%s' Ip '%s'", userToLog(userPos), Login.c_str(), ia->asString().c_str());

	// recv the client version and check it

	uint32 ClientVersion;
	msgin.serial (ClientVersion);
	if (ClientVersion < ServerVersion || ClientVersion > ServerVersion)
	{
		// reject the use, bad version
		ok = false;
		CMessage msgout( "ALP" );
		msgout.serial (ok);
		if (ClientVersion < ServerVersion)
			err = "Your client is out of date. You have to download the last version.";
		else
			err = "Your client is too new compare to the server. You have to get an older version of the client.";
		msgout.serial (err);
		CMsgSocket::send (msgout, from);
		Output.displayNL ("---: %3d Bad Version, ClientVersion: %d ServerVersion: %d", userToLog(userPos), ClientVersion, ServerVersion);
		return;
	}

	// recv client hardware info

	string OS, Proc, Mem, Gfx;

	msgin.serial (OS);
	msgin.serial (Proc);
	msgin.serial (Mem);
	msgin.serial (Gfx);

	Output.displayNL ("OS : %3d %s", userToLog(userPos), OS.c_str());
	Output.displayNL ("PRC: %3d %s", userToLog(userPos), Proc.c_str());
	Output.displayNL ("MEM: %3d %s", userToLog(userPos), Mem.c_str());
	Output.displayNL ("GFX: %3d %s", userToLog(userPos), Gfx.c_str());

	// check the login & pass

	if (!stringIsStandard (Login))
	{
		// reject the new user, bad login format
		ok = false;
		CMessage msgout( "ALP" );
		msgout.serial (ok);
		err = "Bad login format, only alphanumeric character";
		msgout.serial (err);
		CMsgSocket::send (msgout, from);
		Output.displayNL ("---: %3d Bad Login Format", userToLog(userPos));
		return;
	}

	if (!stringIsStandard (Password))
	{
		// reject the new user, bad password format
		ok = false;
		CMessage msgout( "ALP" );
		msgout.serial (ok);
		err = "Bad password format, only alphanumeric character";
		msgout.serial (err);
		CMsgSocket::send (msgout, from);
		Output.displayNL ("---: %3d Bad Password Format", userToLog(userPos));
		return;
	}

	if (userPos == -1)
	{
		// unknown user
#if ACCEPT_NEW_USER
		// add the new user
		ok = true;
		addUser (Login, Password);
		// take the new user entry
		userPos = findUser (Login);
		Output.displayNL ("---: %3d New User (new id:%d)", -1, userToLog(userPos));
#else
		// reject the new user
		ok = false;
		CMessage msgout( "ALP" );
		msgout.serial (ok);
		err = "Bad login";
		msgout.serial (err);
		CMsgSocket::send (msgout, from);
		Output.displayNL ("---: %3d Bad Login", userToLog(userPos));
		return;
#endif
	}
	else
	{
		ok = checkPassword (Password, Users[userPos].Password);
		if (ok == 0)
		{
			// error reason
			CMessage msgout( "ALP" );
			msgout.serial (ok);
			err = "Bad password";
			msgout.serial (err);
			CMsgSocket::send (msgout, from);
			Output.displayNL ("---: %3d Bad Password", userToLog(userPos));
			return;
		}
		else
		{
			Output.displayNL ("---: %3d Ok", userToLog(userPos));
		}
	}

	// count online shards
	uint32 nbshard = 0;
	sint32 i;
	for (i = 0; i < (sint32) Shards.size (); i++)
	{
		if (Shards[i].Online)
		{
			nbshard++;
		}
	}
	if (nbshard==0)
	{
		ok = false;
		err = "No shards available";
	}
	
	CMessage msgout( "ALP" );
	msgout.serial (ok);
	if (ok)
	{
		// send number of online shard
		msgout.serial (nbshard);

		// send address and name of all online shards
		for (i = 0; i < (sint32) Shards.size (); i++)
		{
			if (Shards[i].Online)
			{
			  string str = Shards[i].Address.ipAddress ();
				msgout.serial (str);
				string shardname;
				shardname = Shards[i].Name;

				if (Shards[i].NbPlayers == 0)
				{
					shardname += " (no users)";
				}
				else
				{
					char num[1024];
					smprintf(num, 1024, "%d", Shards[i].NbPlayers);
					shardname += " (";
					shardname += num;
					if (Shards[i].NbPlayers == 1)
						shardname += " user)";
					else
						shardname += " users)";
				}
				msgout.serial (shardname);
			}
		}

		Users[userPos].Authorize (from);
	}
	else
	{
		// put the error message
		msgout.serial (err);
	}
	CMsgSocket::send (msgout, from);
}

void cbChooseShard ( CMessage& msgin, TSenderId from )
{
	// first find if the user is authorized
	for (vector<CUser>::iterator it = Users.begin (); it != Users.end (); it++)
	{
		if ((*it).Authorized && (*it).SenderId == from)
		{
			// it's ok, so we found the wanted shard
			string ShardIP;
			msgin.serial (ShardIP);

			for (sint32 i = 0; i < (sint32) Shards.size (); i++)
			{
				if (Shards[i].Online && Shards[i].Address.ipAddress () == ShardIP)
				{
					// send a message to the shard to warn a new user will come
					CMessage msgout( "ACC" );

					// send the ip in string of the user
					const CInetAddress *ia = CMsgSocket::addressFromId ((*it).SenderId);
					string str = ia->ipAddress ();
					msgout.serial (str);

					// send the uniq key
					msgout.serial ((*it).Key);

					// send the user id
					msgout.serial ((*it).Id);

					CMsgSocket::send (msgout, Shards[i].SenderId);
					return;
				}
			}
			// the shard is not available, denied the user
			CMessage msgout( "ACC" );
			uint8 res = false;
			string err = "Selected shard is not available";
			msgout.serial (res);
			msgout.serial (err);
			CMsgSocket::send (msgout, from);
			return;
		}
	}

	// the user isn t authorized
	nlwarning("User try to choose a shard without authorization");
	// disconnect him!!!
	CMsgSocket::close (from);
}

/** Shard accepted the new user, so warn the user that he could connect to the shard now */
void cbShardAcceptedTheUser ( CMessage& msgin, TSenderId from )
{
	uint8 res;
	string IP;
	uint32 Key, Id;
	msgin.serial (IP);
	msgin.serial (Key);
	msgin.serial (Id);
	msgin.serial (res);

	CMessage msgout( "ACC" );
	msgout.serial (res);

	if(res)
	{
		// the shard accept the user
		msgout.serial (IP);
		msgout.serial (Key);
	}
	else
	{
		// the shard don't want him!
		string reason;
		msgin.serial (reason);
		msgout.serial (reason);
	}

	// find the user
	for (vector<CUser>::iterator it = Users.begin (); it != Users.end (); it++)
	{
		if ((*it).Authorized && (*it).Key == Key)
		{
			// send the answer to the user
			CMsgSocket::send (msgout, (*it).SenderId);

			(*it).Authorized = false;
			return;
		}
	}
}

void cbShardComesIn( CMessage& msgin, TSenderId from )
{
	const CInetAddress *ia = CMsgSocket::addressFromId (from);

	// at this time, it could be a new shard or a ne client.

	nldebug("new potential shard: %s", ia->asString ().c_str ());

	// first, check if it an authorized shard
	for (sint32 i = 0; i < (sint32) Shards.size (); i++)
	{
		if (Shards[i].Address.ipAddress () == ia->ipAddress ())
		{
			if (Shards[i].Online)
			{
				nlwarning("Shard with ip '%s' is already online! Disconnect the new one", ia->asString().c_str ());
				CMsgSocket::close (from);
			}
			else
			{
				// new shard connected
				Shards[i].Address = *ia;
				Shards[i].Online = true;
				Shards[i].SenderId = from;
				nlinfo("Shard with ip '%s' is online!", Shards[i].Address.asString().c_str ());
			}
			return;
		}
	}
#if ACCEPT_EXTERNAL_SHARD
	// New externam shard connected, add it in the file
	Shards.push_back (CShard(*ia));
	sint32 pos = Shards.size()-1;
	Shards[pos].Online = true;
	Shards[pos].SenderId = from;
	nlinfo("External shard with ip '%s' is online!", Shards[pos].Address.asString().c_str ());
	writeConfigFile ();
#else
	nlwarning("It's not a authorized shard, disconnect it");
	CMsgSocket::close (from);
#endif
}

void cbConnection( CMessage& msgin, TSenderId from )
{
	const CInetAddress *ia = CMsgSocket::addressFromId (from);

	// at this time, it could be a new shard or a ne client.

	nldebug("new connection: %s", ia->asString ().c_str ());
}

void cbDisconnection( CMessage& msgin, TSenderId from )
{
	const CInetAddress *ia = CMsgSocket::addressFromId (from);

	// at this time, it could be a new shard or a ne client.

	nldebug("disconnection: %s", ia->asString ().c_str ());

	for (sint32 i = 0; i < (sint32) Shards.size (); i++)
	{
		if (Shards[i].Address == *ia)
		{
			if (Shards[i].Online)
			{
				// shard disconnected
				Shards[i].Online = false;
				nlinfo("Shard with ip '%s' is offline!", Shards[i].Address.asString().c_str());

				// put users connected on this shard offline
				for (sint32 u = 0; u < (sint32) Users.size (); u++)
				{
					if (Users[u].ShardId == from)
						Users[u].Online = false;
				}
			}
			else
			{
				nlwarning("Shard with ip '%s' goes offline but wasn't online!", Shards[i].Address.asString().c_str ());
			}
			return;
		}
	}

	// it's not a shard, remove the user if necessary
	for (vector<CUser>::iterator it = Users.begin (); it != Users.end (); it++)
	{
		if ((*it).SenderId == from)
		{
			// remove the authorized user because he's not here anymore
			(*it).Authorized = false;
			return;
		}
	}
}

void cbPlayerConnectedToShard( CMessage& msgin, TSenderId from )
{
	// a shard tell me that a player is connected
	// find the user
	uint32 Id;
	msgin.serial (Id);

	sint pos = findUser (Id);
	if (pos == -1)
	{
		nlwarning ("Id %d doesn't exist", Id);
		Output.displayNL ("###: %3d UId doesn't exist", Id);
	}
	else if (Users[pos].Online)
	{
		nlwarning ("Id %d already on a shard", Id);
		Output.displayNL ("###: %3d User already connected on a shard", Id);
	}
	else
	{
		Users[pos].Online = true;
		Users[pos].ShardId = from;

		sint ShardPos = findShard(from);
		if (ShardPos != -1)
			Shards[ShardPos].NbPlayers++;
		else
			nlwarning ("user connected shard isn't in the shard list");
		
		nldebug ("Id %d is connected on the shard", Id);
		Output.displayNL ("###: %3d User connected to the shard", Id);
	}
}

void cbPlayerDisconnectedToShard( CMessage& msgin, TSenderId from )
{
	// a shard tell me that a player is connected
	// find the user
	uint32 Id;
	msgin.serial (Id);

	sint pos = findUser (Id);
	if (pos == -1)
	{
		nlwarning ("Id %d doesn't exist", Id);
		Output.displayNL ("###: %3d UId doesn't exist", Id);
	}
	else if (!Users[pos].Online)
	{
		nlwarning ("Id %d wasn't connected on a shard", Id);
		Output.displayNL ("###: %3d User wasn't connected on a shard", Id);
	}
	else
	{
		Users[pos].Online = false;

		sint ShardPos = findShard(from);
		if (ShardPos != -1)
			Shards[ShardPos].NbPlayers--;
		else
			nlwarning ("user disconnected shard isn't in the shard list");
		
		nldebug ("Id %d is discconnected from the shard", Id);
		Output.displayNL ("###: %3d User disconnected from the shard", Id);
	}
}


/** Callback Array
 */
TCallbackItem CallbackArray[] =
{
	{ "VLP", cbVerifyLoginPassword },
	{ "CS", cbChooseShard },
	{ "ACC", cbShardAcceptedTheUser },
	{ "SHARD", cbShardComesIn },

	{ "C", cbConnection },
	{ "D", cbDisconnection },

	{ "CON", cbPlayerConnectedToShard},
	{ "DIS", cbPlayerDisconnectedToShard},
};


/// Naming Service
NLNET_SERVICE_MAIN( CLoginService, "LS", 49999 );
