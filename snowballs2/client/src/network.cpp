/** \file network.cpp
 * manage the connection to the shard and messages
 *
 * $Id: network.cpp,v 1.4 2001/07/17 13:57:34 lecroart Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX SNOWBALLS.
 * NEVRAX SNOWBALLS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX SNOWBALLS is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX SNOWBALLS; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include <list>

#include <nel/misc/types_nl.h>
#include <nel/misc/event_listener.h>
#include <nel/misc/command.h>
#include <nel/misc/log.h>
#include <nel/misc/displayer.h>

#include <nel/net/login_client.h>

#include "client.h"
#include "commands.h"
#include "network.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

CCallbackClient *Connection = NULL;

static void cbClientDisconnected (TSockId from, void *arg)
{
	nlwarning ("You lost the connection to the server");
}

static void cbAddEntity (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	nlinfo ("Receive add entity");
}

static void cbRemoveEntity (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	nlinfo ("Receive remove entity");
}

static void cbEntityPos (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	nlinfo ("Receive entity pos");
}

static void cbSBHit(CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	nlinfo ("Receive hit msg");
}

static void cbChat (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	string line;
	addLine (line);
}

/*static void cbDummy (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
}*/

static TCallbackItem ClientCallbackArray[] =
{
	{ "ADD_ENTITY", cbAddEntity },
	{ "REMOVE_ENTITY", cbRemoveEntity },
	{ "ENTITY_POS", cbEntityPos },
	{ "SB_HIT", cbSBHit },
	{ "CHAT", cbChat },
//	{ "", cbDummy },
};


bool	isOnline ()
{
	return Connection != NULL && Connection->connected ();
}

void	sendChatLine (string Line)
{
	if (!isOnline ()) return;

	/// \todo ace: send the message

//	CMessage msgout (Connection.getSIDA(), "CHAT");
//	msgout.serial (Line);
//	Connection.send (msgout);
}

void	sendEntityPos (const CEntity &entity)
{
	if (!isOnline ()) return;

	/// \todo ace: send the message

//	CMessage msgout (Connection.getSIDA(), "ENTITY_POS");
//	msgout.serial ();
//	Connection.send (msgout);
}


void	initNetwork()
{
	Connection = new CCallbackClient;
	Connection->addCallbackArray (ClientCallbackArray, sizeof (ClientCallbackArray) / sizeof (ClientCallbackArray[0]));
	Connection->setDisconnectionCallback (cbClientDisconnected, NULL);
}

void	updateNetwork()
{
	Connection->update ();
}

void	releaseNetwork()
{
	if (Connection != NULL)
	{
		if (Connection->connected ())
			Connection->disconnect ();

		delete Connection;
		Connection = NULL;
	}
}


NLMISC_COMMAND(connect,"connect to the login system","<login> <password>")
{
	// check args, if there s not the right number of parameter, return bad
	if (args.size() != 2) return false;

	string LoginSystemHost = ConfigFile.getVar("LoginSystemHost").asString ();

	string res = CLoginClient::authenticate (LoginSystemHost+":49999", args[0], args[1], 1);
	if (!res.empty ()) log.displayNL ("Authentification failed: %s", res.c_str());

	log.displayNL ("Please select a shard in the list using \"/select <num>\" where <num> is the shard number");
	for (uint32 i = 0; i < CLoginClient::ShardList.size (); i++)
	{
		log.displayNL ("Shard %d: %s (%s)", i, CLoginClient::ShardList[i].ShardName.c_str(), CLoginClient::ShardList[i].WSAddr.c_str());
	}

	return true;
}

NLMISC_COMMAND(disconnect,"disconnect from the shard","")
{
	// check args, if there s not the right number of parameter, return bad
	if (args.size() != 0) return false;

	if (!isOnline()) log.displayNL ("You already are offline");

	Connection->disconnect ();

	return true;
}

NLMISC_COMMAND(select,"select a shard using his number","<shard_number>")
{
	// check args, if there s not the right number of parameter, return bad
	if (args.size() != 1) return false;

	uint32 num = (uint32) atoi (args[0].c_str());

	string res = CLoginClient::connectToShard (num, *Connection);
	if (!res.empty ()) log.displayNL ("Connection failed: %s", res.c_str());

	log.displayNL ("You are online!!!");

	return true;
}
