/** \file welcome_service.cpp
 * Welcome Service (WS)
 *
 * $Id: welcome_service.cpp,v 1.4 2001/09/05 17:19:29 lecroart Exp $
 *
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX D.T.C. SYSTEM.
 * NEVRAX D.T.C. SYSTEM is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * NEVRAX D.T.C. SYSTEM is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with NEVRAX D.T.C. SYSTEM; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include <stdio.h>
#include <ctype.h>
#include <math.h>

#include <list>

#include "nel/misc/types_nl.h"

#include "nel/misc/debug.h"
#include "nel/misc/config_file.h"
#include "nel/misc/displayer.h"
#include "nel/misc/command.h"
#include "nel/misc/log.h"

#include "nel/net/service.h"
#include "nel/net/net_manager.h"
#include "nel/net/login_cookie.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

/// Set the version of the shard. you have to increase it each time the client-server protocol changes.
/// You have to increment the client too (the server and client version must be the same to run correctly)
static const uint32 ServerVersion = 1;


/// Contains the correspondance between userid and the FES connection where the userid is connected.
map<uint32, TSockId> UserIdSockAssociations;

/// \todo ace: code a better heuristic to distribute user on FES (using NbUser and not only NbEstimatedUser)

struct CFES
{
	CFES (TSockId sockid) : SockId(sockid), NbUser(0), NbEstimatedUser(0) { }

	TSockId	SockId;					// Connection to the front end
	uint32	NbEstimatedUser;		// Number of user that already routed to this FES. This number could be different with the NbUser if
									// some users are not yet connected on the FES (used to equilibrate connection to all front end).
									// This number *never* decrease, it's just to fairly distribute user.
	uint32	NbUser;					// Number of user currently connected on this front end
};

list<CFES> FESList;

// find the best front end service
CFES *findBestFES ()
{
	if (FESList.empty ()) return NULL;
	list<CFES>::iterator best = FESList.begin();
	for (list<CFES>::iterator it = best; it != FESList.end(); it++)
	{
		if ((*it).NbEstimatedUser < (*best).NbEstimatedUser)
		{
			best = it;
		}
	}
	return &(*best);
}

void displayFES ()
{
	nlinfo ("There's %d FES in the list:", FESList.size());
	for (list<CFES>::iterator it = FESList.begin(); it != FESList.end(); it++)
	{
		nlinfo(" > %s NbUser:%d NbEstUser:%d", (*it).SockId->asString().c_str(), (*it).NbUser, (*it).NbEstimatedUser);
	}
	nlinfo ("End of the list");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// CONNECTION TO THE FRONT END SERVICE ///////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void cbFESShardChooseShard (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	// the WS answer a user authorize
	string reason;
	CLoginCookie cookie;
	string addr;

	//
	// S09: receive "SCS" message from FES and send the "SCS" message to the LS
	//
	
	CMessage msgout (CNetManager::getSIDA ("LS"), "SCS");

	msgin.serial (reason);
	msgout.serial (reason);

	msgin.serial (cookie);
	msgout.serial (cookie);
	
	if (reason.empty())
	{
		msgin.serial (addr);
		msgout.serial (addr);
	}
	
	CNetManager::send ("LS", msgout);
}

// This function is call when a FES accepted a new client or lost a connection to a client
void cbFESClientConnected (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	//
	// S15: receive "CC" message from FES and send "CC" message to the "WS"
	//

	CMessage msgout (CNetManager::getSIDA ("LS"), "CC");
	
	uint32 userid;
	msgin.serial (userid);
	msgout.serial (userid);

	uint8 con;
	msgin.serial (con);
	msgout.serial (con);

	CNetManager::send ("LS", msgout);

	// add or remove the user number really connected on this shard
	for (list<CFES>::iterator it = FESList.begin(); it != FESList.end(); it++)
	{
		if ((*it).SockId == from)
		{
			if (con) (*it).NbUser++;
			else (*it).NbUser--;
			break;
		}
	}

	if (con)
	{
		// we know that this user is on this FES
		UserIdSockAssociations.insert (make_pair (userid, from));

	}
	else
	{
		// remove the user
		UserIdSockAssociations.erase (userid);
	}
}

// a new front end connecting to me, add it
void cbFESConnection (const string &serviceName, TSockId from, void *arg)
{
	CCallbackNetBase *cnb = CNetManager::getNetBase("WS");
	const CInetAddress &ia = cnb->hostAddress (from);

	FESList.push_back (CFES (from));

	nldebug("new FES connection: %s", ia.asString ().c_str ());

	displayFES ();
}

// a front end closes the connection, deconnect him
void cbFESDisconnection (const string &serviceName, TSockId from, void *arg)
{
	CCallbackNetBase *cnb = CNetManager::getNetBase("WS");
	const CInetAddress &ia = cnb->hostAddress (from);

	for (list<CFES>::iterator it = FESList.begin(); it != FESList.end(); it++)
	{
		if ((*it).SockId == from)
		{
			map<uint32, TSockId>::iterator nitc;
			// send a message to the LS to say that all players from this FES are offline
			for (map<uint32, TSockId>::iterator itc = UserIdSockAssociations.begin(); itc != UserIdSockAssociations.end();)
			{
				nitc = itc;
				nitc++;
				if ((*itc).second == from)
				{
					// bye bye little player
					CMessage msgout (CNetManager::getSIDA ("LS"), "CC");
					uint32 userid = (*itc).first;
					msgout.serial (userid);
					uint8 con = 0;
					msgout.serial (con);
					CNetManager::send ("LS", msgout);

					UserIdSockAssociations.erase (itc);
				}
				itc = nitc;
			}

			// remove the FES
			FESList.erase (it);

			break;
		}
	}

	nldebug("new FES disconnection: %s", ia.asString ().c_str ());

	displayFES ();
}

// Callback Array for message from FES
TCallbackItem FESCallbackArray[] =
{
	{ "SCS", cbFESShardChooseShard },
	{ "CC", cbFESClientConnected },
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// CONNECTION TO THE LOGIN SERVICE ///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void cbLSChooseShard (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	// the LS warns me that a new client want to come in my shard

	//
	// S07: receive the "CS" message from LS and send the "CS" message to the selected FES
	//

	CLoginCookie cookie;
	msgin.serial (cookie);

	CFES *best = findBestFES();
	if (best == NULL)
	{
		// answer to the LS that we can't accept the user
		CMessage msgout (netbase.getSIDA (), "SCS");
		string reason = "No front end service available";
		msgout.serial (reason);
		msgout.serial (cookie);
		netbase.send (msgout);
		return;
	}

	CMessage msgout (CNetManager::getNetBase ("WS")->getSIDA (), "CS");
	msgout.serial (cookie);
	CNetManager::send ("WS", msgout, best->SockId);
	best->NbEstimatedUser++;
}


void cbLSDisconnectClient (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	// the LS tells me that i have to disconnect a client

	uint32 userid;
	msgin.serial (userid);

	map<uint32, TSockId>::iterator it = UserIdSockAssociations.find (userid);
	if (it == UserIdSockAssociations.end ())
	{
		nlwarning ("Can't disconnect the user %d, he is not found", userid);
	}
	else
	{
		CMessage msgout (CNetManager::getNetBase ("WS")->getSIDA (), "DC");
		msgout.serial (userid);
		CNetManager::send ("WS", msgout, (*it).second);
	}
}




// Callback Array for message from LS
TCallbackItem LSCallbackArray[] =
{
	{ "CS", cbLSChooseShard },
	
	{ "DC", cbLSDisconnectClient },
};

class CWelcomeService : public NLNET::IService
{

public:

	/// Init the service, load the universal time.
	void init ()
	{
		CNetManager::setConnectionCallback ("WS", cbFESConnection, NULL);
		CNetManager::setDisconnectionCallback ("WS", cbFESDisconnection, NULL);

		// add a connection to the LS
		string LSAddr = ConfigFile.getVar("LSHost").asString() + ":49998";
		CNetManager::addClient ("LS", LSAddr);
		CNetManager::addCallbackArray ("LS", LSCallbackArray, sizeof(LSCallbackArray)/sizeof(LSCallbackArray[0]));
	}
};


/// Welcome Service
NLNET_SERVICE_MAIN (CWelcomeService, "WS", "welcome_service", 0, FESCallbackArray);

//
// Commands
//


NLMISC_COMMAND (frontends, "displays the list of all registered front ends", "")
{
	if(args.size() != 0) return false;

	log.displayNL ("Display the %d registered front end :", FESList.size());
	for (list<CFES>::iterator it = FESList.begin(); it != FESList.end (); it++)
	{
		log.displayNL ("> %s '%s' nb estimated users: %u nb users: %u", (*it).SockId->asString().c_str(), CNetManager::getNetBase ("WS")->hostAddress((*it).SockId).asString().c_str(), (*it).NbEstimatedUser, (*it).NbUser );
	}
	log.displayNL ("End ot the list");

	return true;
}

NLMISC_COMMAND (users, "displays the list of all registered users", "")
{
	if(args.size() != 0) return false;

	log.displayNL ("Display the %d registered users :", UserIdSockAssociations.size());
	for (map<uint32, TSockId>::iterator it = UserIdSockAssociations.begin(); it != UserIdSockAssociations.end (); it++)
	{
		log.displayNL ("> %u %s '%s'", (*it).first, (*it).second->asString().c_str(), CNetManager::getNetBase ("WS")->hostAddress((*it).second).asString().c_str());
	}
	log.displayNL ("End ot the list");

	return true;
}

