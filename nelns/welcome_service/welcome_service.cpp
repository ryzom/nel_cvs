/** \file welcome_service.cpp
 * Welcome Service (WS)
 *
 * $Id: welcome_service.cpp,v 1.36 2004/07/08 08:55:25 distrib Exp $
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

#include <stdio.h>
#include <ctype.h>
#include <math.h>

#include <list>

#include "nel/misc/debug.h"
#include "nel/misc/config_file.h"
#include "nel/misc/displayer.h"
#include "nel/misc/command.h"
#include "nel/misc/variable.h"
#include "nel/misc/log.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"

#include "nel/net/service.h"
#include "nel/net/unified_network.h"
#include "nel/net/login_cookie.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;


CVariable<sint> PlayerLimit(
	"PlayerLimit", "Rough max number of players accepted on this shard (-1 for Unlimited)",
	5000,
	0, true );

// Forward declaration of callback cbShardOpenStateFile (see ShardOpenStateFile variable)
void	cbShardOpenStateFile(IVariable &var);

// Types of open state
enum TShardOpenState
{
	ClosedForAll = 0,
	OpenOnlyForAllowed = 1,
	OpenForAll = 2
};

/**
 * ShardOpen
 * true if shard is open to public
 * 0 means closed for all but :DEV:
 * 1 means open only for groups in config file (see OpenGroups variable) and :DEV:
 * 2 means open for all
 */
CVariable<uint>		ShardOpen("ShardOpen", "Indicates if shard is open to public (0 closed for all but :DEV:, 1 open only for groups in cfg, 2 open for all)", 2, 0, true);

/**
 * ShardOpenStateFile
 * true if shard is open to public
 */
CVariable<string>	ShardOpenStateFile("ShardOpenStateFile", "Name of the file that contains ShardOpen state", "", 0, true, cbShardOpenStateFile);

/**
 * OpenGroups
 */
CVariable<string>	OpenGroups("OpenGroups", "list of groups allowed at ShardOpen Level 1", "", 0, true);


/**
 * Using expected services and current running service instances, this class
 * reports a main "online status".
 */
class COnlineServices
{
public:

	/// Set expected instances. Ex: { "TICKS", "FS", "FS", "FS" }
	void		setExpectedInstances( CConfigFile::CVar& var )
	{
		// Reset "expected" counters (but don't clear the map, keep the running instances)
		CInstances::iterator ici;
		for ( ici=_Instances.begin(); ici!=_Instances.end(); ++ici )
		{
			(*ici).second.Expected = 0;
		}
		// Rebuild "expected" counters
		for ( sint i=0; i!=var.size(); ++i )
		{
			++_Instances[var.asString(i)].Expected;
		}
	}
	
	/// Add a service instance
	void		addInstance( const std::string& serviceName )
	{
		++_Instances[serviceName].Running;
	}

	/// Remove a service instance
	void		removeInstance( const std::string& serviceName ) 
	{
		CInstances::iterator ici = _Instances.find( serviceName );
		if ( ici != _Instances.end() )
		{
			--(*ici).second.Running;

			// Remove from the map only if not part of the expected list
			if ( ((*ici).second.Expected == 0) && ((*ici).second.Running == 0) )
			{
				_Instances.erase( ici );
			}
		}
		else
		{
			nlwarning( "Can't remove instance of %s", serviceName.c_str() );
		}
	}

	/// Check if all expected instances are online
	bool		getOnlineStatus() const
	{
		CInstances::const_iterator ici;
		for ( ici=_Instances.begin(); ici!=_Instances.end(); ++ici )
		{
			if ( ! (*ici).second.isOnlineAsExpected() )
				return false;
		}
		return true;
	}

	/// Display contents
	void		display( NLMISC::CLog& log = *NLMISC::DebugLog )
	{
		CInstances::const_iterator ici;
		for ( ici=_Instances.begin(); ici!=_Instances.end(); ++ici )
		{
			log.displayNL( "%s: %s (%u expected, %u running)",
				(*ici).first.c_str(),
				(*ici).second.Expected ? ((*ici).second.isOnlineAsExpected() ? "ONLINE" : "MISSING") : "OPTIONAL",
				(*ici).second.Expected, (*ici).second.Running );
		}
	}

private:

	struct TInstanceCounters
	{
		TInstanceCounters() : Expected(0), Running(0) {}

		// If not expected, count as online as well
		bool isOnlineAsExpected() const { return Running >= Expected; }

		uint	Expected;
		uint	Running;
	};

	typedef std::map< std::string, TInstanceCounters > CInstances;

	CInstances	_Instances;
};

/// Online services
COnlineServices OnlineServices;


/// Main online status
bool OnlineStatus;

/// Send changes of status to the LS
void reportOnlineStatus( bool newStatus )
{
	if ( newStatus != OnlineStatus )
	{
		CMessage msgout( "OL_ST" );
		msgout.serial( newStatus );
		CUnifiedNetwork::getInstance()->send( "LS", msgout );
		OnlineStatus = newStatus;
	}
}



/// Set the version of the shard. you have to increase it each time the client-server protocol changes.
/// You have to increment the client too (the server and client version must be the same to run correctly)
static const uint32 ServerVersion = 1;

/// Contains the correspondance between userid and the FES connection where the userid is connected.
map<uint32, TServiceId> UserIdSockAssociations;

// ubi hack
string FrontEndAddress;

/// \todo ace: code a better heuristic to distribute user on FES (using NbUser and not only NbEstimatedUser)

struct CFES
{
	CFES (TServiceId sid) : SId(sid), NbEstimatedUser(0), NbUser(0) { }

	TServiceId	SId;				// Connection to the front end
	uint32		NbEstimatedUser;	// Number of user that already routed to this FES. This number could be different with the NbUser if
									// some users are not yet connected on the FES (used to equilibrate connection to all front end).
									// This number *never* decrease, it's just to fairly distribute user.
	uint32		NbUser;				// Number of user currently connected on this front end
};

list<CFES> FESList;

/*
 * Find the best front end service for a new connecting user (return NULL if there is no suitable FES).
 * Additionally, calculate totalNbUsers.
 */
CFES *findBestFES ( uint& totalNbUsers )
{
	totalNbUsers = 0;
	if (FESList.empty ()) return NULL;
	list<CFES>::iterator best = FESList.begin();
	for (list<CFES>::iterator it = best; it != FESList.end(); it++)
	{
		if ((*it).NbEstimatedUser < (*best).NbEstimatedUser)
		{
			best = it;
		}
		totalNbUsers += (*it).NbUser;
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
			else
			{
				if ( (*it).NbUser != 0 )
					(*it).NbUser--;
			}
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


/*
 * Set Shard Open State
 * uint8	Open State (0 closed for all, 1 open for groups in cfg, 2 open for all)
 */
void cbSetShardOpen(CMessage &msgin, const std::string &serviceName, uint16 sid)
{
	uint8 shardOpenState;
	msgin.serial (shardOpenState);

	if (shardOpenState > OpenForAll)
	{
		shardOpenState = OpenForAll;
	}

	ShardOpen = shardOpenState;
}

// forward declaration to callback
void	cbShardOpenStateFile(IVariable &var);

/*
 * Restore Shard Open state from config file or from file if found
 */
void cbRestoreShardOpen(CMessage &msgin, const std::string &serviceName, uint16 sid)
{
	// first restore state from config file
	CConfigFile::CVar*	var = IService::getInstance()->ConfigFile.getVarPtr("ShardOpen");
	if (var != NULL)
	{
		ShardOpen = var->asInt();
	}

	// then restore state from state file, if it exists
	cbShardOpenStateFile(ShardOpenStateFile);
}





// a new front end connecting to me, add it
void cbFESConnection (const std::string &serviceName, uint16 sid, void *arg)
{
	FESList.push_back (CFES ((TServiceId)sid));
	nldebug("new FES connection: sid %u", sid);
	displayFES ();

	// Reset NbEstimatedUser to NbUser for all front-ends
	for (list<CFES>::iterator it = FESList.begin(); it != FESList.end(); it++)
	{
		(*it).NbEstimatedUser = (*it).NbUser;
	}
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
					nlinfo ("Due to a frontend crash, removed the player %d", userid);
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


//
void cbServiceUp (const std::string &serviceName, uint16 sid, void *arg)
{
	OnlineServices.addInstance( serviceName );
	reportOnlineStatus( OnlineServices.getOnlineStatus() );

	// send shard id to service
	sint32 shardId;
	if (IService::getInstance()->haveArg('S'))
	{
		// use the command line param if set
		shardId = atoi(IService::getInstance()->getArg('S').c_str());
	}
	else if (IService::getInstance()->ConfigFile.exists ("ShardId"))
	{
		// use the config file param if set
		shardId = IService::getInstance()->ConfigFile.getVar ("ShardId").asInt();
	}
	else
	{
		shardId = -1;
	}

	if (shardId == -1)
	{
		nlerror ("ShardId variable must be valid (>0)");
	}

	CMessage	msgout("R_SH_ID");
	msgout.serial(shardId);
	CUnifiedNetwork::getInstance()->send (sid, msgout);
}


//
void cbServiceDown (const std::string &serviceName, uint16 sid, void *arg)
{
	OnlineServices.removeInstance( serviceName );
	reportOnlineStatus( OnlineServices.getOnlineStatus() );
}


// Callback Array for message from FES
TUnifiedCallbackItem FESCallbackArray[] =
{
	{ "SCS", cbFESShardChooseShard },
	{ "CC", cbFESClientConnected },

	{ "SET_SHARD_OPEN",		cbSetShardOpen },
	{ "RESTORE_SHARD_OPEN",	cbRestoreShardOpen },
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
	string userName, userPriv;
	msgin.serial (userName);

	try
	{
		msgin.serial (userPriv);
	}
	catch (Exception &)
	{
		nlwarning ("LS didn't give me the user privilege for user '%s', set to empty", userName.c_str());
	}

	uint totalNbUsers;
	CFES *best = findBestFES( totalNbUsers );
	if (best == NULL)
	{
		// answer the LS that we can't accept the user
		CMessage msgout ("SCS");
		string reason = "No front-end server available";
		msgout.serial (reason);
		msgout.serial (cookie);
		CUnifiedNetwork::getInstance()->send(sid, msgout);
		return;
	}



	bool	authorizeUser = false;
	bool	forceAuthorize = false;

	if (userPriv == ":DEV:")
	{
		// devs have all privileges
		authorizeUser = true;
		forceAuthorize = true;
	}
	else if (ShardOpen != ClosedForAll)
	{
		const std::string&	allowedGroups = OpenGroups;
		bool				userInOpenGroups = (allowedGroups.find(userPriv) != std::string::npos);

		// open for all or user is privileged
		authorizeUser = (ShardOpen == OpenForAll || userInOpenGroups);
		// let authorized users to force access even if limit is reached
		forceAuthorize = userInOpenGroups;
	}

	bool	shardLimitReached = ( (PlayerLimit.get() != -1) && (totalNbUsers >= (uint)PlayerLimit.get()) );

	if (!forceAuthorize && (!authorizeUser || shardLimitReached))
	{
		// answer the LS that we can't accept the user
		CMessage msgout ("SCS");
		string reason;
		if (shardLimitReached)
			reason = "The shard is currently full, please try again in 5 minutes.";
		else
			reason = "The shard is closed.";
		msgout.serial (reason);
		msgout.serial (cookie);
		CUnifiedNetwork::getInstance()->send(sid, msgout);
		return;
	}


	CMessage msgout ("CS");
	msgout.serial (cookie);
	msgout.serial (userName, userPriv);

	CUnifiedNetwork::getInstance()->send (best->SId, msgout);
	best->NbEstimatedUser++;
}

void cbFailed (CMessage &msgin, const std::string &serviceName, uint16 sid)
{
	// I can't connect to the Login Service, just nlerror ();
	string reason;
	msgin.serial (reason);
	nlerror (reason.c_str());
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
	sint32 shardId;
	
	if (IService::getInstance()->haveArg('S'))
	{
		// use the command line param if set
		shardId = atoi(IService::getInstance()->getArg('S').c_str());
	}
	else if (IService::getInstance()->ConfigFile.exists ("ShardId"))
	{
		// use the config file param if set
		shardId = IService::getInstance()->ConfigFile.getVar ("ShardId").asInt();
	}
	else
	{
		shardId = -1;
	}

	if (shardId == -1)
	{
		nlerror ("ShardId variable must be valid (>0)");
	}

	CMessage msgout ("WS_IDENT");
	msgout.serial (shardId);
	CUnifiedNetwork::getInstance()->send (sid, msgout);

	nlinfo ("Connected to %s-%hu and sent identification with shardId '%d'", serviceName.c_str(), sid, shardId);
}


// Callback for detection of config file change about "ExpectedServices"
void cbUpdateExpectedServices( CConfigFile::CVar& var )
{
	OnlineServices.setExpectedInstances( var );
}


/*
 * ShardOpen update functions/callbacks etc.
 */

/**
 * updateShardOpenFromFile()
 * Update ShardOpen from a file.
 * Read a line of text in the file, converts it to int (atoi), then casts into bool for ShardOpen.
 */
void	updateShardOpenFromFile(const std::string& filename)
{
	CIFile	f;

	if (!f.open(filename))
	{
		nlwarning("Failed to update ShardOpen from file '%s', couldn't open file", filename.c_str());
		return;
	}

	try
	{
		char	readBuffer[256];
		f.getline(readBuffer, 256);
		ShardOpen = atoi(readBuffer);

		nlinfo("Updated ShardOpen state to '%u' from file '%s'", ShardOpen.get(), filename.c_str());
	}
	catch (Exception& e)
	{
		nlwarning("Failed to update ShardOpen from file '%s', exception raised while getline() '%s'", filename.c_str(), e.what());
	}
}

std::string	ShardOpenStateFileName;

/**
 * cbShardOpenStateFile()
 * Callback for ShardOpenStateFile
 */
void	cbShardOpenStateFile(IVariable &var)
{
	// remove previous file change callback
	if (!ShardOpenStateFileName.empty())
	{
		CFile::removeFileChangeCallback(ShardOpenStateFileName);
		nlinfo("Removed callback for ShardOpenStateFileName file '%s'", ShardOpenStateFileName.c_str());
	}

	ShardOpenStateFileName = var.toString();

	if (!ShardOpenStateFileName.empty())
	{
		// set new callback for the file
		CFile::addFileChangeCallback(ShardOpenStateFileName, updateShardOpenFromFile);
		nlinfo("Set callback for ShardOpenStateFileName file '%s'", ShardOpenStateFileName.c_str());

		// and update state from file...
		updateShardOpenFromFile(ShardOpenStateFileName);
	}
}



// Callback Array for message from LS
TUnifiedCallbackItem LSCallbackArray[] =
{
	{ "CS", cbLSChooseShard },
	{ "DC", cbLSDisconnectClient },
	{ "FAILED", cbFailed },
};

class CWelcomeService : public IService
{

public:

	/// Init the service, load the universal time.
	void init ()
	{
		string FrontendServiceName = ConfigFile.getVar ("FrontendServiceName").asString();

		try { FrontEndAddress = ConfigFile.getVar ("FrontEndAddress").asString(); } catch(Exception &) { }

		nlinfo ("Waiting frontend services named '%s'", FrontendServiceName.c_str());

		CUnifiedNetwork::getInstance()->setServiceUpCallback(FrontendServiceName, cbFESConnection, NULL);
		CUnifiedNetwork::getInstance()->setServiceDownCallback(FrontendServiceName, cbFESDisconnection, NULL);
		CUnifiedNetwork::getInstance()->setServiceUpCallback("*", cbServiceUp, NULL);
		CUnifiedNetwork::getInstance()->setServiceDownCallback("*", cbServiceDown, NULL);

		// add a connection to the LS
		string LSAddr;
		if (haveArg('T'))
		{
			// use the command line param if set
			LSAddr = getArg('T');
		}
		else if (ConfigFile.exists ("LSHost"))
		{
			// use the config file param if set
			LSAddr = ConfigFile.getVar("LSHost").asString();
		}

		// the config file must have a valid address where the login service is
		nlassert(!LSAddr.empty());

		// add default port if not set by the config file
		if (LSAddr.find (":") == string::npos)
			LSAddr += ":49999";

		CUnifiedNetwork::getInstance()->addCallbackArray(LSCallbackArray, sizeof(LSCallbackArray)/sizeof(LSCallbackArray[0]));
		CUnifiedNetwork::getInstance()->setServiceUpCallback("LS", cbLSConnection, NULL);
		CUnifiedNetwork::getInstance()->addService("LS", LSAddr);

		// List of expected service instances
		ConfigFile.setCallback( "ExpectedServices", cbUpdateExpectedServices );
		cbUpdateExpectedServices( ConfigFile.getVar( "ExpectedServices" ) );


		/*
		 * read config variable ShardOpenStateFile to update
		 * 
		 */
		cbShardOpenStateFile(ShardOpenStateFile);
	}
};


// Service instantiation
NLNET_SERVICE_MAIN (CWelcomeService, "WS", "welcome_service", 0, FESCallbackArray, NELNS_CONFIG, NELNS_LOGS);


//
// Variables
//

NLMISC_DYNVARIABLE(uint32, OnlineUsersNumber, "number of connected users on this shard")
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

NLMISC_COMMAND( displayOnlineServices, "Display the online service instances", "" )
{
	OnlineServices.display( log );
	return true;
}

NLMISC_VARIABLE( bool, OnlineStatus, "Main online status of the shard" );


