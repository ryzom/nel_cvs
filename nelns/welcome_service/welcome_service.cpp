/** \file welcome_service.cpp
 * Welcome Service (WS)
 *
 * $Id: welcome_service.cpp,v 1.11 2002/03/04 15:33:43 lecroart Exp $
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

#include "nel/misc/types_nl.h"

#include <stdio.h>
#include <ctype.h>
#include <math.h>

#include <list>

#include "nel/misc/debug.h"
#include "nel/misc/config_file.h"
#include "nel/misc/displayer.h"
#include "nel/misc/command.h"
#include "nel/misc/log.h"

#include "nel/net/service_5.h"
#include "nel/net/unified_network.h"
#include "nel/net/login_cookie.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

/// Set the version of the shard. you have to increase it each time the client-server protocol changes.
/// You have to increment the client too (the server and client version must be the same to run correctly)
static const uint32 ServerVersion = 1;

IService5 *ServiceInstance = NULL;

/// Contains the correspondance between userid and the FES connection where the userid is connected.
map<uint32, TServiceId> UserIdSockAssociations;

// ubi hack
string FrontEndAddress;

/// \todo ace: code a better heuristic to distribute user on FES (using NbUser and not only NbEstimatedUser)

struct CFES
{
	CFES (TServiceId sid) : SId(sid), NbUser(0), NbEstimatedUser(0) { }

	TServiceId	SId;				// Connection to the front end
	uint32		NbEstimatedUser;	// Number of user that already routed to this FES. This number could be different with the NbUser if
									// some users are not yet connected on the FES (used to equilibrate connection to all front end).
									// This number *never* decrease, it's just to fairly distribute user.
	uint32		NbUser;				// Number of user currently connected on this front end
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
		nlinfo(" > %u NbUser:%d NbEstUser:%d", (*it).SId, (*it).NbUser, (*it).NbEstimatedUser);
	}
	nlinfo ("End of the list");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// CONNECTION TO THE FRONT END SERVICE ///////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void cbFESShardChooseShard (CMessage &msgin, const std::string &serviceName, uint16 sid)
{
	// the WS answer a user authorize
	string reason;
	CLoginCookie cookie;
	string addr;

	//
	// S09: receive "SCS" message from FES and send the "SCS" message to the LS
	//
	
	CMessage msgout ("SCS");

	msgin.serial (reason);
	msgout.serial (reason);

	msgin.serial (cookie);
	msgout.serial (cookie);
	
	if (reason.empty())
	{
		msgin.serial (addr);

		// if we set the FontEndAddress in the welcome_service.cfg we use this address
		if (FrontEndAddress.empty())
		{
			msgout.serial (addr);
		}
		else
		{
			msgout.serial (FrontEndAddress);
		}
	}
	
	CUnifiedNetwork::getInstance()->send ("LS", msgout);
}

// This function is call when a FES accepted a new client or lost a connection to a client
void cbFESClientConnected (CMessage &msgin, const std::string &serviceName, uint16 sid)
{
	//
	// S15: receive "CC" message from FES and send "CC" message to the "LS"
	//

	CMessage msgout ("CC");
	
	uint32 userid;
	msgin.serial (userid);
	msgout.serial (userid);

	uint8 con;
	msgin.serial (con);
	msgout.serial (con);

	CUnifiedNetwork::getInstance()->send ("LS", msgout);

	// add or remove the user number really connected on this shard
	for (list<CFES>::iterator it = FESList.begin(); it != FESList.end(); it++)
	{
		if ((*it).SId == sid)
		{
			if (con) (*it).NbUser++;
			else (*it).NbUser--;
			break;
		}
	}

	if (con)
	{
		// we know that this user is on this FES
		UserIdSockAssociations.insert (make_pair (userid, sid));

	}
	else
	{
		// remove the user
		UserIdSockAssociations.erase (userid);
	}
}

// a new front end connecting to me, add it
void cbFESConnection (const std::string &serviceName, uint16 sid, void *arg)
{
	FESList.push_back (CFES ((TServiceId)sid));
	nldebug("new FES connection: sid %u", sid);
	displayFES ();
}

// a front end closes the connection, deconnect him
void cbFESDisconnection (const std::string &serviceName, uint16 sid, void *arg)
{
	nldebug("new FES disconnection: sid %u", sid);

	for (list<CFES>::iterator it = FESList.begin(); it != FESList.end(); it++)
	{
		if ((*it).SId == sid)
		{
			// send a message to the LS to say that all players from this FES are offline
			map<uint32, TServiceId>::iterator itc = UserIdSockAssociations.begin();
			map<uint32, TServiceId>::iterator nitc = itc;
			while (itc != UserIdSockAssociations.end())
			{
				nitc++;
				if ((*itc).second == sid)
				{
					// bye bye little player
					uint32 userid = (*itc).first;
					nldebug ("due to a frontend crash, removed the player %d", userid);
					CMessage msgout ("CC");
					msgout.serial (userid);
					uint8 con = 0;
					msgout.serial (con);
					CUnifiedNetwork::getInstance()->send ("LS", msgout);
					UserIdSockAssociations.erase (itc);
				}
				itc = nitc;
			}

			// remove the FES
			FESList.erase (it);

			break;
		}
	}

	displayFES ();
}

// Callback Array for message from FES
TUnifiedCallbackItem FESCallbackArray[] =
{
	{ "SCS", cbFESShardChooseShard },
	{ "CC", cbFESClientConnected },
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// CONNECTION TO THE LOGIN SERVICE ///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void cbLSChooseShard (CMessage &msgin, const std::string &serviceName, uint16 sid)
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
		CMessage msgout ("SCS");
		string reason = "No front end service available";
		msgout.serial (reason);
		msgout.serial (cookie);
		CUnifiedNetwork::getInstance()->send(sid, msgout);
		return;
	}

	CMessage msgout ("CS");
	msgout.serial (cookie);
	CUnifiedNetwork::getInstance()->send (best->SId, msgout);
	best->NbEstimatedUser++;
}


void cbLSDisconnectClient (CMessage &msgin, const std::string &serviceName, uint16 sid)
{
	// the LS tells me that i have to disconnect a client

	uint32 userid;
	msgin.serial (userid);

	map<uint32, TServiceId>::iterator it = UserIdSockAssociations.find (userid);
	if (it == UserIdSockAssociations.end ())
	{
		nlwarning ("Can't disconnect the user %d, he is not found", userid);
	}
	else
	{
		CMessage msgout ("DC");
		msgout.serial (userid);
		CUnifiedNetwork::getInstance()->send ((*it).second, msgout);
	}
}

// connection to the LS, send the identification message
void cbLSConnection (const std::string &serviceName, uint16 sid, void *arg)
{
	CMessage msgout ("WS_IDENT");
	string shardName;
	
	try
	{
		shardName = ServiceInstance->ConfigFile.getVar ("ShardName").asString();
	}
	catch(Exception &)
	{
		shardName = "::";
	}

	if (shardName.empty())
	{
		nlerror ("ShardName variable in the config file must not be empty, set it to \"::\" for the default behavior");
	}

	msgout.serial (shardName);
	CUnifiedNetwork::getInstance()->send (serviceName, msgout);
}





// Callback Array for message from LS
TUnifiedCallbackItem LSCallbackArray[] =
{
	{ "CS", cbLSChooseShard },
	{ "DC", cbLSDisconnectClient },
};

class CWelcomeService : public NLNET::IService5
{

public:

	/// Init the service, load the universal time.
	void init ()
	{
		ServiceInstance = this;

		string FrontendServiceName = ConfigFile.getVar ("FrontendServiceName").asString();

		try { FrontEndAddress = ConfigFile.getVar ("FrontEndAddress").asString(); } catch(Exception &) { }

		nlinfo ("Waiting frontend services named '%s'", FrontendServiceName.c_str());

		CUnifiedNetwork::getInstance()->setServiceUpCallback(FrontendServiceName, cbFESConnection, NULL);
		CUnifiedNetwork::getInstance()->setServiceDownCallback(FrontendServiceName, cbFESDisconnection, NULL);

		// add a connection to the LS
		string LSAddr = ConfigFile.getVar("LSHost").asString();
		
		// add default port if not set by the config file
		if (LSAddr.find (":") == string::npos)
			LSAddr + ":49998";

		CUnifiedNetwork::getInstance()->addCallbackArray(LSCallbackArray, sizeof(LSCallbackArray)/sizeof(LSCallbackArray[0]));
		CUnifiedNetwork::getInstance()->setServiceUpCallback("LS", cbLSConnection, NULL);
		CUnifiedNetwork::getInstance()->addService("LS", LSAddr, false);
	}
};


// Service instanciation
NLNET_SERVICE_MAIN (CWelcomeService, "WS", "welcome_service", 0, FESCallbackArray);


//
// Variables
//

NLMISC_DYNVARIABLE(uint32, online_users_nb, "number of connected users on this shard")
{
	// we can only read the value
	if (get)
	{
		uint32 nbusers = 0;
		for (list<CFES>::iterator it = FESList.begin(); it != FESList.end (); it++)
		{
			nbusers += (*it).NbUser;
		}
		*pointer = nbusers;
	}
}


//
// Commands
//


NLMISC_COMMAND (frontends, "displays the list of all registered front ends", "")
{
	if(args.size() != 0) return false;

	log.displayNL ("Display the %d registered front end :", FESList.size());
	for (list<CFES>::iterator it = FESList.begin(); it != FESList.end (); it++)
	{
		log.displayNL ("> FE %u: nb estimated users: %u nb users: %u", (*it).SId, (*it).NbEstimatedUser, (*it).NbUser );
	}
	log.displayNL ("End ot the list");

	return true;
}

NLMISC_COMMAND (users, "displays the list of all registered users", "")
{
	if(args.size() != 0) return false;

	log.displayNL ("Display the %d registered users :", UserIdSockAssociations.size());
	for (map<uint32, TServiceId>::iterator it = UserIdSockAssociations.begin(); it != UserIdSockAssociations.end (); it++)
	{
		log.displayNL ("> %u SId=%u", (*it).first, (*it).second);
	}
	log.displayNL ("End ot the list");

	return true;
}
