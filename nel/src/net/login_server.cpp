/** \file login_server.cpp
 * CLoginServer is the interface used by the front end to accepts authenticate users.
 *
 * $Id: login_server.cpp,v 1.8 2001/10/26 13:51:56 lecroart Exp $
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

#include <string>
#include <vector>

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/command.h"

#include "nel/net/callback_client.h"
#include "nel/net/unified_network.h"

#include "nel/net/login_cookie.h"
#include "nel/net/login_server.h"

using namespace std;
using namespace NLMISC;

namespace NLNET {

struct CPendingUser
{
	CPendingUser (const CLoginCookie &cookie) : Cookie (cookie) { }
	CLoginCookie Cookie;
};

static list<CPendingUser> PendingUsers;

static CCallbackServer *Server;
static string ListenAddr;

/// contains the correspondance between userid and the sockid
map<uint32, TSockId> UserIdSockAssociations;

TNewClientCallback NewClientCallback;

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
///////////// CONNECTION TO THE WELCOME SERVICE //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

void cbWSChooseShard (CMessage &msgin, const std::string &serviceName, uint16 sid)
{
	// the WS call me that a new client want to come in my shard
	string reason;
	CLoginCookie cookie;
	
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
	list<CPendingUser>::iterator it;
	for (it = PendingUsers.begin(); it != PendingUsers.end (); it++)
	{
		if ((*it).Cookie == cookie)
		{
			// ok, it was validate, remove it
			break;
		}
	}

	if (it == PendingUsers.end ())
	{
		reason = "I didn't receive the cookie from WS";
	}
	else
	{
		PendingUsers.erase (it);
		reason = "";
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
		// identification OK, let's call the user callback
		if (NewClientCallback != NULL)
			NewClientCallback (from, cookie);

		// warn the WS that the client effectively connected
		uint8 con = 1;
		CMessage msgout ("CC");
		uint32 userid = cookie.getUserId();
		msgout.serial (userid);
		msgout.serial (con);
		CUnifiedNetwork::getInstance()->send("WS", msgout);

		// add the user association
		UserIdSockAssociations.insert (make_pair(userid, from));

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


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
///////////// CONNECTION TO THE WELCOME SERVICE //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

void CLoginServer::init (CCallbackServer &server, TNewClientCallback ncl)
{
	// connect to the welcome service
	CUnifiedNetwork::getInstance()->addCallbackArray(WSCallbackArray, sizeof(WSCallbackArray)/sizeof(WSCallbackArray[0]));

	// add callback to the server
	server.addCallbackArray (ClientCallbackArray, sizeof (ClientCallbackArray) / sizeof (ClientCallbackArray[0]));
	server.setConnectionCallback (ClientConnection, NULL);

	ListenAddr = server.listenAddress ().asIPString();
	nlinfo("Listen Addresss trapped %s", ListenAddr.c_str());

	NewClientCallback = ncl;
	Server = &server;
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

NLMISC_COMMAND (ls_users, "displays the list of all connected users", "")
{
	if(args.size() != 0) return false;

	log.displayNL ("Display the %d connected users :", UserIdSockAssociations.size());
	for (map<uint32, TSockId>::iterator it = UserIdSockAssociations.begin(); it != UserIdSockAssociations.end (); it++)
	{
		log.displayNL ("> %u %s", (*it).first, (*it).second->asString().c_str());
	}
	log.displayNL ("End ot the list");

	return true;
}

NLMISC_COMMAND (ls_pending, "displays the list of all pending users", "")
{
	if(args.size() != 0) return false;

	log.displayNL ("Display the %d pending users :", PendingUsers.size());
	for (list<CPendingUser>::iterator it = PendingUsers.begin(); it != PendingUsers.end (); it++)
	{
		log.displayNL ("> %s", (*it).Cookie.toString().c_str());
	}
	log.displayNL ("End ot the list");

	return true;
}

} // NLNET


/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/*
#include "v2/service.h"

using namespace std;
using namespace NLNET;

void ClientConnection (TSockId from, const CLoginCookie &cookie)
{
	nlinfo("player (%d) comes in", cookie.getUserId());
	from->setAppId (cookie.getUserId());
}

void ClientDisconnection (TSockId from, void *arg)
{
	nlinfo("player (%d) leaves", from->appId());
	CLoginServer::clientDisconnected (from->appId());

}

class CFrontEndService : public NLNET::IService
{
public:

	/// Init the service, load the universal time.
	void init ()
	{
		CLoginServer::init (dynamic_cast<CCallbackServer&>(*CNetManager::getNetBase ("FES")), ClientConnection);
		CNetManager::getNetBase ("FES")->setDisconnectionCallback (ClientDisconnection, NULL);
	}
};


/// Naming Service
NLNET_SERVICE_MAIN (CFrontEndService, "FES", "front_end_service", 0);
*/
