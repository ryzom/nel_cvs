/** \file network.cpp
 * Animation interface between the game and NeL
 *
 * $Id: network.cpp,v 1.12 2001/07/24 17:29:23 lecroart Exp $
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

//
// Includes
//

#include <nel/misc/types_nl.h>
#include <nel/misc/event_listener.h>
#include <nel/misc/command.h>
#include <nel/misc/log.h>
#include <nel/misc/displayer.h>

#include <nel/net/login_client.h>

#include <nel/3d/u_text_context.h>

#include "client.h"
#include "commands.h"
#include "network.h"
#include "entities.h"
#include "interface.h"
#include "graph.h"

//
// Namespaces
//

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace NL3D;

//
// Variables
//

CCallbackClient *Connection = NULL;

//
// Functions
//

static void cbClientDisconnected (TSockId from, void *arg)
{
	nlwarning ("You lost the connection to the server");

	askString ("You are offline!!!", "", 2, CRGBA(64,0,0,128));
}

static void cbAddEntity (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	uint32 id;
	string name;
	uint8 race;
	CVector startPosition;

	msgin.serial (id, name, race, startPosition);

	nlinfo ("receive add entity %u '%s' %s (%f,%f,%f)", id, name.c_str(), race==0?"penguin":"gnu", startPosition.x, startPosition.y, startPosition.z);

	if (id != Self->Id)
	{
		addEntity(id, CEntity::Other, startPosition, startPosition);
	}
	else
	{
		nlinfo ("receive my add entity");
	}
}

static void cbRemoveEntity (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	uint32 id;

	msgin.serial (id);

	nlinfo ("Receive remove entity %u", id);

	removeEntity (id);
}

static void cbEntityPos (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	uint32 id;
	CVector position;
	float angle;
	uint32 state;

	msgin.serial (id, position, angle, state);

	nlinfo ("Receive entity pos %u (%f,%f,%f) %f, %u", id, position.x, position.y, position.z, angle, state);

	if (Self->Id = id)
	{
		// receive my info, ignore them, i know where i am
		return;
	}

	EIT eit = findEntity (id, false);
	if (eit == Entities.end ())
	{
		nlwarning ("can't find entity %u", id);
	}
	else
	{
		CEntity	&entity = (*eit).second;

		entity.ServerPosition = position;
		entity.AuxiliaryAngle = angle;
		if (state&1)
		{
			entity.IsAiming = true;
			entity.IsWalking = false;
		}
	}
}

static void cbSBHit(CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	nlinfo ("Receive hit msg");
}

static void cbSnowball (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	uint32 eid;
	CVector position, target;
	float speed;
	TTime startTime;

	msgin.serial (eid, position, target, speed, startTime);
	
	nlinfo ("Receive a snowball message");

	shotSnowball (eid, position, target, speed, startTime);
}

static void cbChat (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	string line;
	msgin.serial (line);
	addLine (line);
}

static void cbIdentification (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	uint32 id;
	msgin.serial (id);
	
	if (Self == NULL)
		nlerror ("Self is NULL");

	// copy my old entity
	CEntity me = *Self;
	
	// set my new online id
	me.Id = id;

	// add my new entity in the array
	EIT eit = (Entities.insert (make_pair (id, me))).first;

	// remove my old entity
	Entities.erase (Self->Id);

	// remap Self
	Self = &((*eit).second);
	
	nlinfo ("my online id is %u", id);

	// send to the network my entity					
	sendAddEntity (Self->Id, Self->Name, 1, Self->Position);
}

/*static void cbDummy (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
}*/

// Array that contains all callback that could comes from the server
static TCallbackItem ClientCallbackArray[] =
{
	{ "ADD_ENTITY", cbAddEntity },
	{ "REMOVE_ENTITY", cbRemoveEntity },
	{ "ENTITY_POS", cbEntityPos },
	{ "SB_HIT", cbSBHit },
	{ "CHAT", cbChat },
	{ "SNOWBALL", cbSnowball },
	{ "IDENTIFICATION", cbIdentification },
//	{ "", cbDummy },
};


bool	isOnline ()
{
	return Connection != NULL && Connection->connected ();
}

void	sendAddEntity (uint32 id, string &name, uint8 race, CVector &startPosition)
{
	if (!isOnline ()) return;

	CMessage msgout (Connection->getSIDA(), "ADD_ENTITY");
	msgout.serial (id, name, race, startPosition);
	Connection->send (msgout);
}

void	sendChatLine (string Line)
{
	if (!isOnline ()) return;

	CMessage msgout (Connection->getSIDA(), "CHAT");
	msgout.serial (Line);
	Connection->send (msgout);
}

void	sendEntityPos (CEntity &entity)
{
	if (!isOnline ()) return;

	// is aiming? is launching etc...
	uint32 state = 0;
	state |= (entity.IsAiming?1:0);

	CMessage msgout (Connection->getSIDA(), "ENTITY_POS");
	msgout.serial (entity.Id, entity.Position, entity.Angle, state);

	UploadGraph.addValue ((float)msgout.length ());
	
	Connection->send (msgout);
	
	nlinfo("sending pos to network (%f,%f,%f, %f), %u", entity.Position.x, entity.Position.y, entity.Position.z, entity.Angle, state);
}

void	sendSnowBall (uint32 eid, const NLMISC::CVector &position, const NLMISC::CVector &target, float speed, NLMISC::TTime startTime)
{
	if (!isOnline ()) return;

	CMessage msgout (Connection->getSIDA(), "SNOWBALL");
	msgout.serial (eid, const_cast<CVector &>(position), const_cast<CVector &>(target), speed, startTime);
	Connection->send (msgout);

	nlinfo("sending snowball to network (%f,%f,%f) to (%f,%f,%f) with %f %"NL_I64"d", position.x, position.y, position.z, target.x, target.y, target.z, speed, startTime);
}


TTime LastPosSended;

void	initNetwork()
{
	Connection = new CCallbackClient;
	Connection->addCallbackArray (ClientCallbackArray, sizeof (ClientCallbackArray) / sizeof (ClientCallbackArray[0]));
	Connection->setDisconnectionCallback (cbClientDisconnected, NULL);

	LastPosSended = 0;
}

void	updateNetwork()
{
	if (Connection != NULL)
	{
		sint64 newBytesDownloaded = (sint64) Connection->newBytesDownloaded ();
		sint64 newBytesUploaded = (sint64) Connection->newBytesUploaded ();

		TextContext->setHotSpot (UTextContext::MiddleTop);
		TextContext->setColor (CRGBA(255, 255, 255, 255));
		TextContext->setFontSize (14);
		TextContext->printfAt (0.5f, 0.99f, "d:%"NL_I64"u u:%"NL_I64"u / d:%"NL_I64"u u:%"NL_I64"u / d:%"NL_I64"u u:%"NL_I64"u",
			Connection->bytesDownloaded (), Connection->bytesUploaded (),
			Connection->getBytesReceived (),Connection->getBytesSended (),
			newBytesDownloaded, newBytesUploaded);

		DownloadGraph.addValue ((float)newBytesDownloaded);
		UploadGraph.addValue ((float)newBytesUploaded);

		Connection->update ();

		if (isOnline () && Self != NULL)
		{
			if (CTime::getLocalTime () > LastPosSended + 100)
			{
				sendEntityPos (*Self);

				LastPosSended = CTime::getLocalTime ();
			}
		}
	}
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

	string LoginSystemAddress = ConfigFile.getVar("LoginSystemAddress").asString ();

	string res = CLoginClient::authenticate (LoginSystemAddress+":49999", args[0], args[1], 1);
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
