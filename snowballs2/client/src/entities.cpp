/** \file commands.cpp
 * commands management with user interface
 *
 * $Id: entities.cpp,v 1.1 2001/07/12 12:54:15 lecroart Exp $
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

#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_text_context.h>
#include <nel/3d/u_instance.h>
#include <nel/3d/u_scene.h>
#include <nel/3d/u_3d_mouse_listener.h>
#include <nel/3d/u_material.h>
#include <nel/3d/u_landscape.h>

#include "client.h"
#include "entities.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;


map<uint32, CEntity> Entities;
typedef map<uint32, CEntity>::iterator EIT;


// these variables are set with the config file

float RadarPosX, RadarPosY, RadarWidth, RadarHeight, RadarBorder;
CRGBA RadarBackColor, RadarFrontColor, RadarBorderColor;
float RadarEntitySize;

EIT findEntity (uint32 eid, bool needAssert = true)
{
	EIT entity = Entities.find (eid);
	if (entity == Entities.end () && needAssert)
	{
		nlerror ("Entity %d not found", eid);
	}
	return entity;
}


void addEntity (uint32 eid, CEntity::TType type)
{
	nlinfo ("adding entity %d", eid);

	EIT eit = findEntity (eid, false);
	if (eit != Entities.end ())
	{
		nlerror ("Entity %d already exist", eid);
	}
	CEntity newEntity;
	newEntity.Id = eid;
	newEntity.Type = type;
	newEntity.Name = "Entity"+toString(rand());
	Entities.insert (make_pair (eid, newEntity));
}

void removeEntity (uint32 eid)
{
	nlinfo ("removing entity %d", eid);

	EIT eit = findEntity (eid);
	Entities.erase (eit);
}

void updateEntities ()
{
	/// \todo
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
		userPosX = (*eit).second.x;
		userPosY = (*eit).second.y;

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

	uint32 eid = (float)atoi(args[0].c_str());
	removeEntity (eid);
}


NLMISC_COMMAND(add_entity,"add a local entity","<x> <y>")
{
	// check args, if there s not the right number of parameter, return bad
	if(args.size() != 2) return false;

	float x = (float)atof(args[0].c_str());
	float y = (float)atof(args[1].c_str());

	static uint32 nextEID = 0;
	uint32 eid = nextEID++;
	addEntity (eid, CEntity::Other);
	EIT eit = findEntity (eid);
	(*eit).second.x = x;
	(*eit).second.y = y;
	return true;
}
