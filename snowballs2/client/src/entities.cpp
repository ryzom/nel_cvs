/** \file commands.cpp
 * commands management with user interface
 *
 * $Id: entities.cpp,v 1.6 2001/07/12 15:43:05 lecroart Exp $
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

#include <map>

#include <nel/misc/types_nl.h>
#include <nel/misc/event_listener.h>
#include <nel/misc/command.h>
#include <nel/misc/log.h>
#include <nel/misc/displayer.h>
#include <nel/misc/vector.h>
#include <nel/misc/vectord.h>

#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_text_context.h>
#include <nel/3d/u_instance.h>
#include <nel/3d/u_scene.h>
#include <nel/3d/u_3d_mouse_listener.h>
#include <nel/3d/u_material.h>
#include <nel/3d/u_landscape.h>

#include <nel/pacs/u_move_container.h>
#include <nel/pacs/u_move_primitive.h>

#include "client.h"
#include "entities.h"
#include "pacs.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;
using namespace NLPACS;


map<uint32, CEntity> Entities;
typedef map<uint32, CEntity>::iterator EIT;

enum
{
	SelfCollisionBit = 1,
	OtherCollisionBit = 2,
	SnowballCollisionBit = 4
};

// these variables are set with the config file

float RadarPosX, RadarPosY, RadarWidth, RadarHeight, RadarBorder;
CRGBA RadarBackColor, RadarFrontColor, RadarBorderColor;
float RadarEntitySize;

CEntity		*Self = NULL;


EIT findEntity (uint32 eid, bool needAssert = true)
{
	EIT entity = Entities.find (eid);
	if (entity == Entities.end () && needAssert)
	{
		nlerror ("Entity %u not found", eid);
	}
	return entity;
}


void addEntity (uint32 eid, CEntity::TType type, CVector startPosition)
{
	nlinfo ("adding entity %u", eid);

	EIT eit = findEntity (eid, false);
	if (eit != Entities.end ())
	{
		nlerror ("Entity %d already exist", eid);
	}

	eit = (Entities.insert (make_pair (eid, CEntity()))).first;
	CEntity	&entity = (*eit).second;

	if (type == CEntity::Self)
	{
		if (Self != NULL)
			nlerror("Self entity already created");

		Self = &entity;
	}

	entity.Id = eid;
	entity.Type = type;
	entity.Name = "Entity"+toString(rand());
	entity.Position = startPosition;
	entity.MovePrimitive = MoveContainer->addCollisionablePrimitive(0, 1);

	// setup the move primitive and the mesh instance depending on the type of entity
	switch (type)
	{
	case CEntity::Self:
		entity.MovePrimitive->setPrimitiveType(UMovePrimitive::_2DOrientedCylinder);
		entity.MovePrimitive->setReactionType(UMovePrimitive::Slide);
		entity.MovePrimitive->setTriggerType(UMovePrimitive::NotATrigger);
		entity.MovePrimitive->setCollisionMask(OtherCollisionBit+SnowballCollisionBit);
		entity.MovePrimitive->setOcclusionMask(SelfCollisionBit);
		entity.MovePrimitive->setObstacle(true);
		entity.MovePrimitive->setRadius(0.5f);
		entity.MovePrimitive->setHeight(1.8f);
		entity.MovePrimitive->insertInWorldImage(0);
		entity.MovePrimitive->setGlobalPosition(CVectorD(startPosition.x, startPosition.y, startPosition.z), 0);
		entity.Instance = Scene->createInstance("barman.shape");
		break;
	case CEntity::Other:
		entity.MovePrimitive->setPrimitiveType(UMovePrimitive::_2DOrientedCylinder);
		entity.MovePrimitive->setReactionType(UMovePrimitive::Slide);
		entity.MovePrimitive->setTriggerType(UMovePrimitive::NotATrigger);
		entity.MovePrimitive->setCollisionMask(SelfCollisionBit+SnowballCollisionBit);
		entity.MovePrimitive->setOcclusionMask(OtherCollisionBit);
		entity.MovePrimitive->setObstacle(true);
		entity.MovePrimitive->setRadius(0.5f);
		entity.MovePrimitive->setHeight(1.8f);
		entity.Instance = Scene->createInstance("barman.shape");
		break;
	case CEntity::Snowball:
		entity.MovePrimitive->setPrimitiveType(UMovePrimitive::_2DOrientedCylinder);
		entity.MovePrimitive->setReactionType(UMovePrimitive::Slide);
		entity.MovePrimitive->setTriggerType(UMovePrimitive::EnterTrigger);
		entity.MovePrimitive->setCollisionMask(SelfCollisionBit+OtherCollisionBit);
		entity.MovePrimitive->setOcclusionMask(SnowballCollisionBit);
		entity.MovePrimitive->setObstacle(false);
		entity.MovePrimitive->setRadius(0.2f);
		entity.MovePrimitive->setHeight(0.4f);
		entity.Instance = Scene->createInstance("barman.shape");
		break;
	}

	entity.Instance->setPos (startPosition);
}

void removeEntity (uint32 eid)
{
	nlinfo ("removing entity %u", eid);

	EIT eit = findEntity (eid);

	if ((*eit).second.Type == CEntity::Self)
	{
		if (Self == NULL)
			nlerror("Self entity doesn't exist");
		Self = NULL;
	}

	MoveContainer->removePrimitive((*eit).second.MovePrimitive);

	Entities.erase (eit);
}

float EntityMaxSpeed = 1.0f;

void updateEntities ()
{
	// move auto move entity
	for (EIT eit = Entities.begin (); eit != Entities.end (); eit++)
	{
		if ((*eit).second.AutoMove)
		{
			(*eit).second.Position.x += frand (2*EntityMaxSpeed) - EntityMaxSpeed;
			(*eit).second.Position.y += frand (2*EntityMaxSpeed) - EntityMaxSpeed;
			(*eit).second.Instance->setPos ((*eit).second.Position);
		}
	}

	// compute the collision for the primitives inserted in the move container	
	double	dt = (double)(NewTime-LastTime) / 1000.0;
	MoveContainer->evalCollision(dt, 0);
}

void cbUpdateRadar (CConfigFile::CVar &var)
{
	if (var.Name == "RadarPosX") RadarPosX = var.asFloat ();
	else if (var.Name == "RadarPosY") RadarPosY = var.asFloat ();
	else if (var.Name == "RadarWidth") RadarWidth = var.asFloat ();
	else if (var.Name == "RadarHeight") RadarHeight = var.asFloat ();
	else if (var.Name == "RadarBackColor") RadarBackColor.set (var.asInt(0), var.asInt(1), var.asInt(2), var.asInt(3));
	else if (var.Name == "RadarFrontColor") RadarFrontColor.set (var.asInt(0), var.asInt(1), var.asInt(2), var.asInt(3));
	else if (var.Name == "RadarBorderColor") RadarBorderColor.set (var.asInt(0), var.asInt(1), var.asInt(2), var.asInt(3));
	else if (var.Name == "RadarEntitySize") RadarEntitySize = var.asFloat ();
	else if (var.Name == "RadarBorder") RadarBorder = var.asFloat ();
	else nlwarning ("Unknown variable update %s", var.Name.c_str());
}

void initRadar ()
{
	ConfigFile.setCallback ("RadarPosX", cbUpdateRadar);
	ConfigFile.setCallback ("RadarPosY", cbUpdateRadar);
	ConfigFile.setCallback ("RadarWidth", cbUpdateRadar);
	ConfigFile.setCallback ("RadarHeight", cbUpdateRadar);
	ConfigFile.setCallback ("RadarBackColor", cbUpdateRadar);
	ConfigFile.setCallback ("RadarFrontColor", cbUpdateRadar);
	ConfigFile.setCallback ("RadarBorderColor", cbUpdateRadar);
	ConfigFile.setCallback ("RadarEntitySize", cbUpdateRadar);
	ConfigFile.setCallback ("RadarBorder", cbUpdateRadar);

	cbUpdateRadar (ConfigFile.getVar ("RadarPosX"));
	cbUpdateRadar (ConfigFile.getVar ("RadarPosY"));
	cbUpdateRadar (ConfigFile.getVar ("RadarWidth"));
	cbUpdateRadar (ConfigFile.getVar ("RadarHeight"));
	cbUpdateRadar (ConfigFile.getVar ("RadarFrontColor"));
	cbUpdateRadar (ConfigFile.getVar ("RadarBackColor"));
	cbUpdateRadar (ConfigFile.getVar ("RadarBorderColor"));
	cbUpdateRadar (ConfigFile.getVar ("RadarEntitySize"));
	cbUpdateRadar (ConfigFile.getVar ("RadarBorder"));
}

void updateRadar ()
{
	// Display the back of the radar
	Driver->setMatrixMode2D11 ();
	Driver->drawQuad (RadarPosX-RadarBorder, RadarPosY-RadarBorder, RadarPosX+RadarWidth+RadarBorder, RadarPosY+RadarHeight+RadarBorder, RadarBackColor);
	Driver->drawWiredQuad (RadarPosX, RadarPosY, RadarPosX+RadarWidth, RadarPosY+RadarHeight, RadarBorderColor);

	for (EIT eit = Entities.begin (); eit != Entities.end (); eit++)
	{
		float userPosX, userPosY;

		// convert from world coords to radar coords (0.0 -> 1.0)
		userPosX = (*eit).second.Position.x / 1000.0f;
		userPosY = -(*eit).second.Position.y / 1000.0f;

		// userPosX and userPosY must be between 0.0 -> 1.0

		userPosX *= RadarWidth;
		userPosY *= RadarHeight;

		userPosX += RadarPosX;
		userPosY += RadarPosY;

		Driver->drawQuad (userPosX, userPosY, RadarEntitySize, RadarFrontColor);
	}
}


NLMISC_COMMAND(remove_entity,"remove a local entity","<eid>")
{
	// check args, if there s not the right number of parameter, return bad
	if(args.size() != 1) return false;

	uint32 eid = (uint32)atoi(args[0].c_str());
	removeEntity (eid);

	return true;
}


NLMISC_COMMAND(add_entity,"add a local entity","<nb_entities> <auto_update>")
{
	// check args, if there s not the right number of parameter, return bad
	if(args.size() != 2) return false;

	uint nb = (uint)atoi(args[0].c_str());

	static uint32 nextEID = 0;

	for (uint i = 0; i < nb ; i++)
	{
		uint32 eid = nextEID++;
		float x = 1000.0f; ///frand(1000.0f);
		float y = -1000.0f; ///-frand(1000.0f);
		float z = 2.0f;
		addEntity (eid, CEntity::Other, CVector(x, y, z));
		EIT eit = findEntity (eid);
		(*eit).second.AutoMove = atoi(args[1].c_str()) == 1;
	}

	return true;
}
