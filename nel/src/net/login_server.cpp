/** \file login_server.cpp
 * CLoginServer is the interface used by the front end to *s authenticate users.
 *
 * $Id: login_server.cpp,v 1.24 2003/01/13 14:06:25 lecroart Exp $
 *
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "stdnet.h"

#include "nel/net/callback_client.h"
#include "nel/net/service.h"

#include "nel/net/login_cookie.h"
#include "nel/net/login_server.h"

#include "nel/net/udp_sock.h"

using namespace std;
using namespace NLMISC;

namespace NLNET {

struct CPendingUser
{
	CPendingUser (const CLoginCookie &cookie) : Cookie (cookie) { Time = CTime::getSecondsSince1970(); }
	CLoginCookie Cookie;
	uint32 Time;	// when the cookie is inserted in pending list
};

static list<CPendingUser> PendingUsers;

static CCallbackServer *Server;
static string ListenAddr;

static bool AcceptInvalidCookie = false;

static TDisconnectClientCallback DisconnectClientCallback = NULL;

// default value is 2 minutes
static uint TimeBeforeEraseCookie = 120;

/// contains the correspondance between userid and the sockid
map<uint32, TSockId> UserIdSockAssociations;

TNewClientCallback NewClientCallback = NULL;

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
///////////// CONNECTION TO THE WELCOME SERVICE //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

void refreshPendingList ()
{
	// delete too old cookie

	list<CPendingUser>::iterator it = PendingUsers.begin();
	uint32 Time = CTime::getSecondsSince1970();
	while (it != PendingUsers.end ())
	{
		if ((*it).Time < Time - TimeBeforeEraseCookie)
		{
			nlinfo("Removing cookie '%s' because too old", (*it).Cookie.toString().c_str());
			it = PendingUsers.erase (it);
		}
		else
		{
			it++;
		}
	}
}


void cbWSChooseShard (CMessage &msgin, const std::string &serviceName, uint16 sid)
{
	// the WS call me that a new client want to come in my shard
	string reason;
	CLoginCookie cookie;

	refreshPendingList ();

	//
	// S08: receive "CS" message from WS and send "SCS" message to WS
	//

	msgin.serial (cookie);

	list<CPendingUser>::iterator it;
	for (it = PendingUsers.begin(); it != PendingUsers.end (); it++)
	{
		if ((*it).Cookie == cookie)
		{
			// the cookie already exists, erase it and return false
			nlwarning ("cookie %s is already in the pending user list", cookie.toString().c_str());
			PendingUsers.erase (it);
			reason = "cookie already exists";
			break;
		}
	}
	if (it == PendingUsers.end ())
	{
		// add it to the awaiting client
		nlinfo ("New cookie %s inserted in the pending user list (awaiting new client)", cookie.toString().c_str());
		PendingUsers.push_back (CPendingUser (cookie));
		reason = "";
	}

	CMessage msgout ("SCS");
	msgout.serial (reason);
	msgout.serial (cookie);
	msgout.serial (ListenAddr);
	CUnifiedNetwork::getInstance()->send ("WS", msgout);
}

void cbWSDisconnectClient (CMessage &msgin, const std::string &serviceName, uint16 sid)
{
	// the WS tells me that i have to disconnect a client

	uint32 userid;
	msgin.serial (userid);

	map<uint32, TSockId>::iterator it = UserIdSockAssociations.find (userid);
	if (it == UserIdSockAssociations.end ())
	{
		nlwarning ("Can't disconnect the user %d, he is not found", userid);
	}
	else
	{
		nlinfo ("Disconnect the user %d", userid);
		Server->disconnect ((*it).second);
	}

	if (DisconnectClientCallback != NULL)
	{
		DisconnectClientCallback (userid);
	}
}

static TUnifiedCallbackItem WSCallbackArray[] =
{
	{ "CS", cbWSChooseShard },
	{ "DC", cbWSDisconnectClient },
};

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
///////////// CONNECTION TO THE CLIENT ///////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

void cbShardValidation (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	//
	// S13: receive "SV" message from client
	//

	// the client send me a cookie
	CLoginCookie cookie;
	string reason;
	msgin.serial (cookie);

	// verify that the user was pending
	reason = CLoginServer::isValidCookie (cookie);

	// if the cookie is not valid and we accept them, clear the error
	if(AcceptInvalidCookie && !reason.empty())
	{
		reason = "";
		cookie.set (rand(), rand(), rand());
	}

	CMessage msgout2 (netbase.getSIDA (), "SV");
	msgout2.serial (reason);
	netbase.send (msgout2, from);
	
	if (!reason.empty())
	{
		nlwarning ("User (%s) is not in the pending user list (cookie:%s)", netbase.hostAddress(from).asString().c_str(), cookie.toString().c_str());
		// deconnect him
		netbase.disconnect (from);
	}
	else
	{
		// add the user association
		uint32 userid = cookie.getUserId();
		UserIdSockAssociations.insert (make_pair(userid, from));

		// identification OK, let's call the user callback
		if (NewClientCallback != NULL)
			NewClientCallback (from, cookie);

		// ok, now, he can call all callback
		Server->authorizeOnly (NULL, from);
	}
}

void ClientConnection (TSockId from, void *arg)
{
	nldebug("new client connection: %s", from->asString ().c_str ());

	// the client could only call "SV" message
	Server->authorizeOnly ("SV", from);
}


static const TCallbackItem ClientCallbackArray[] =
{
	{ "SV", cbShardValidation },
};

void cfcbListenAddress (CConfigFile::CVar &var)
{
	// set the new ListenAddr
	ListenAddr = var.asString();
	
	// is the var is empty or not found, take it from the listenAddress()
	if (ListenAddr.empty())
	{
		ListenAddr = Server->listenAddress ().asIPString();
	}

	nlinfo("Listen Address trapped '%s'", ListenAddr.c_str());
}

void cfcbAcceptInvalidCookie(CConfigFile::CVar &var)
{
	// set the new ListenAddr
	AcceptInvalidCookie = var.asInt() == 1;
	
	nlinfo("This service %saccept invalid cookie", AcceptInvalidCookie?"":"doesn't ");
}

void cfcbTimeBeforeEraseCookie(CConfigFile::CVar &var)
{
	// set the new ListenAddr
	TimeBeforeEraseCookie = var.asInt();
	
	nlinfo("This service will remove cookie after %d seconds", TimeBeforeEraseCookie);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
///////////// CONNECTION TO THE WELCOME SERVICE //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

void CLoginServer::init (string &listenAddress)
{
	// connect to the welcome service
	connectToWS ();
	
	try {
		cfcbListenAddress (IService::getInstance()->ConfigFile.getVar("ListenAddress"));
		IService::getInstance()->ConfigFile.setCallback("ListenAddress", cfcbListenAddress);
		
	} catch(Exception &) { }
	
	try {
		cfcbAcceptInvalidCookie (IService::getInstance()->ConfigFile.getVar("AcceptInvalidCookie"));
		IService::getInstance()->ConfigFile.setCallback("AcceptInvalidCookie", cfcbAcceptInvalidCookie);
	} catch(Exception &) { }
	
	try {
		cfcbTimeBeforeEraseCookie (IService::getInstance()->ConfigFile.getVar("TimeBeforeEraseCookie"));
		IService::getInstance()->ConfigFile.setCallback("TimeBeforeEraseCookie", cfcbTimeBeforeEraseCookie);
	} catch(Exception &) { }
	
	// if the listen addr is not in the config file, try to find it dynamically
	if (ListenAddr.empty())
	{
		ListenAddr = listenAddress;
	}

	nlinfo("Listen Address trapped '%s'", ListenAddr.c_str());
}

void CLoginServer::init (CCallbackServer &server, TNewClientCallback ncl)
{
	init (server.listenAddress ().asIPString());
	
	// add callback to the server
	server.addCallbackArray (ClientCallbackArray, sizeof (ClientCallbackArray) / sizeof (ClientCallbackArray[0]));
	server.setConnectionCallback (ClientConnection, NULL);

	NewClientCallback = ncl;
	Server = &server;
}

void CLoginServer::init (CUdpSock &server, TDisconnectClientCallback dc)
{
	init (server.localAddr ().asIPString());

	DisconnectClientCallback = dc;
}

string CLoginServer::isValidCookie (const CLoginCookie &lc)
{
	if (AcceptInvalidCookie)
		return "";

	if (!lc.isValid())
		return "The cookie is invalid";

	// verify that the user was pending
	list<CPendingUser>::iterator it;
	for (it = PendingUsers.begin(); it != PendingUsers.end (); it++)
	{
		if ((*it).Cookie == lc)
		{
			// ok, it was validate, remove it
			PendingUsers.erase (it);

			nlinfo ("Cookie '%s' is valid and pending, send the client connection to the WS", lc.toString ().c_str ());

			// warn the WS that the client effectively connected
			uint8 con = 1;
			CMessage msgout ("CC");
			uint32 userid = lc.getUserId();
			msgout.serial (userid);
			msgout.serial (con);

			CUnifiedNetwork::getInstance()->send("WS", msgout);

			return "";
		}
	}

	return "I didn't receive the cookie from WS";
}

void CLoginServer::connectToWS ()
{
	CUnifiedNetwork::getInstance()->addCallbackArray(WSCallbackArray, sizeof(WSCallbackArray)/sizeof(WSCallbackArray[0]));
}

void CLoginServer::clientDisconnected (uint32 userId)
{
	uint8 con = 0;
	CMessage msgout ("CC");
	msgout.serial (userId);
	msgout.serial (con);

	CUnifiedNetwork::getInstance()->send("WS", msgout);

	// remove the user association
	UserIdSockAssociations.erase (userId);
}

//
// Commands
//

NLMISC_COMMAND (lsUsers, "displays the list of all connected users", "")
{
	if(args.size() != 0) return false;

	log.displayNL ("Display the %d connected users :", UserIdSockAssociations.size());
	for (map<uint32, TSockId>::iterator it = UserIdSockAssociations.begin(); it != UserIdSockAssociations.end (); it++)
	{
		log.displayNL ("> %u %s", (*it).first, (*it).second->asString().c_str());
	}
	log.displayNL ("End of the list");

	return true;
}

NLMISC_COMMAND (lsPending, "displays the list of all pending users", "")
{
	if(args.size() != 0) return false;

	log.displayNL ("Display the %d pending users :", PendingUsers.size());
	for (list<CPendingUser>::iterator it = PendingUsers.begin(); it != PendingUsers.end (); it++)
	{
		log.displayNL ("> %s", (*it).Cookie.toString().c_str());
	}
	log.displayNL ("End of the list");

	return true;
}


NLMISC_DYNVARIABLE(string, LSListenAddress, "the listen address sended to the client to connect on this front_end")
{
	if (get)
	{
		*pointer = ListenAddr;
	}
	else
	{
		if ((*pointer).find (":") == string::npos)
		{
			log.displayNL ("You must set the address + port (ie: \"itsalive.nevrax.org:38000\")");
			return;
		}
		else if ((*pointer).empty())
		{
			ListenAddr = Server->listenAddress ().asIPString();
		}
		else
		{
			ListenAddr = *pointer;
		}
		log.displayNL ("Listen Address trapped '%s'", ListenAddr.c_str());
	}
}


} // NLNET
