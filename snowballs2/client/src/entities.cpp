/** \file commands.cpp
 * commands management with user interface
 *
 * $Id: entities.cpp,v 1.10 2001/07/13 09:58:06 lecroart Exp $
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

#include <math.h>
#include <map>

#include <nel/misc/types_nl.h>
#include <nel/misc/event_listener.h>
#include <nel/misc/command.h>
#include <nel/misc/log.h>
#include <nel/misc/displayer.h>
#include <nel/misc/vector.h>
#include <nel/misc/vectord.h>
#include <nel/misc/time_nl.h>

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
#include <nel/pacs/u_global_retriever.h>
#include <nel/pacs/u_global_position.h>

#include <nel/3d/u_visual_collision_manager.h>
#include <nel/3d/u_visual_collision_entity.h>

#include "client.h"
#include "entities.h"
#include "pacs.h"
#include "camera.h"

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

float WorldWidth = 20*160;
float WorldHeight = 6*160;

// these variables are set with the config file

float RadarPosX, RadarPosY, RadarWidth, RadarHeight, RadarBorder;
CRGBA RadarBackColor, RadarFrontColor, RadarBorderColor, RadarSelfColor;
float RadarEntitySize;
float RadarZoom;

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
//	entity.ServerPosition = startPosition;
	entity.MovePrimitive = MoveContainer->addCollisionablePrimitive(0, 1);
	entity.VisualCollisionEntity = VisualCollisionManager->createEntity();

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
		entity.Instance = Scene->createInstance("barman.shape");
		entity.Instance->hide ();

		entity.Particule = Scene->createInstance("appear.ps");
		entity.Particule->setPos (startPosition);
		entity.State = CEntity::Appear;
		entity.StateStartTime = CTime::getLocalTime ();

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
		entity.Instance->hide ();

		entity.Particule = Scene->createInstance("appear.ps");
		entity.Particule->setPos (startPosition);
		entity.State = CEntity::Appear;
		entity.StateStartTime = CTime::getLocalTime ();

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

		entity.State = CEntity::Normal;
		entity.StateStartTime = CTime::getLocalTime ();
		break;
	}
	entity.MovePrimitive->insertInWorldImage(0);
//	entity.MovePrimitive->setGlobalPosition(CVectorD(startPosition.x, startPosition.y, startPosition.z), 0);
	entity.MovePrimitive->setGlobalPosition(GlobalRetriever->retrievePosition(CVectorD(startPosition.x, startPosition.y, startPosition.z)), 0);

	entity.Instance->setPos (startPosition);
}

void removeEntity (uint32 eid)
{
	nlinfo ("removing entity %u", eid);

	EIT eit = findEntity (eid);

	CEntity	&entity = (*eit).second;

	entity.Particule = Scene->createInstance("disappear.ps");
	entity.Particule->setPos (entity.Position);

	entity.State = CEntity::Disappear;
	entity.StateStartTime = CTime::getLocalTime ();
}

void stateAppear (CEntity &entity)
{
	if (CTime::getLocalTime () > entity.StateStartTime + 1000)
	{
		if (entity.Instance->getVisibility () != UTransform::Show)
			entity.Instance->show ();
	}

	if (CTime::getLocalTime () > entity.StateStartTime + 5000)
	{
		Scene->deleteInstance (entity.Particule);
		entity.Particule = NULL;

		entity.State = CEntity::Normal;
		entity.StateStartTime = CTime::getLocalTime ();
	}

	entity.MovePrimitive->move(CVector(0,0,0), 0);
}

void stateDisappear (CEntity &entity)
{
	// after 1 second, remove the mesh and all collision stuff
	if (CTime::getLocalTime () > entity.StateStartTime + 1000)
	{
		if (entity.Instance->getVisibility () != UTransform::Hide)
		{
			if (entity.Type == CEntity::Self)
			{
				if (Self == NULL)
					nlerror("Self entity doesn't exist");
				Self = NULL;
			}

			entity.Instance->hide ();
		}
	}

	// after 5 seconds, remove the particule system and the entity entry
	if (CTime::getLocalTime () > entity.StateStartTime + 5000)
	{
		Scene->deleteInstance (entity.Particule);
		entity.Particule = NULL;

		Scene->deleteInstance (entity.Instance);
		entity.Instance = NULL;

		VisualCollisionManager->deleteEntity (entity.VisualCollisionEntity);
		entity.VisualCollisionEntity = NULL;

		MoveContainer->removePrimitive(entity.MovePrimitive);
		entity.MovePrimitive = NULL;

		nlinfo ("Remove the entity %u from the Entities list", entity.Id);
		EIT eit = findEntity (entity.Id);
		Entities.erase (eit);
	}
	else
	{
		entity.MovePrimitive->move(CVector(0,0,0), 0);
	}
}

void stateNormal (CEntity &entity)
{
	double	dt = (double)(NewTime-LastTime) / 1000.0;
	CVector	oldPos;
	CVector	newPos;

	oldPos = entity.Position;

/// \todo remove when server entities will work
	// find a new random server position
	if (entity.AutoMove /*&& entity.ServerPosition == entity.Position*/)
	{
		float EntityMaxSpeed = 1.0f;
		entity.Position = entity.Position + CVector(frand (2.0f*EntityMaxSpeed) - EntityMaxSpeed,
										   frand (2.0f*EntityMaxSpeed) - EntityMaxSpeed,
										   0.0f);
	}
/// \todo end of the remove block when server entities will work


	if (entity.Type == CEntity::Self)
	{
		// the self entity
		// get new position
		newPos = MouseListener->getViewMatrix().getPos();
		// get new orientation
		CVector	j = MouseListener->getViewMatrix().getJ();
		ViewHeight -= j.z;
		j.z = 0.0f;
		j.normalize();
		entity.Angle = (float)atan2(j.y, j.x);
	}
	else if (entity.ServerPosition != entity.Position)
	{
		// go to the server position with linear interpolation
		/// \todo compute the linear interpolation
		newPos = oldPos;
	}
	else
	{
		// automatic speed
		/// \todo compute new entity position
		newPos = oldPos;
	}

	entity.MovePrimitive->move((newPos-oldPos)/(float)dt, 0);
}

void updateEntities ()
{
	// compute the delta t that has elapsed since the last update (in seconds)
	double	dt = (double)(NewTime-LastTime) / 1000.0;
	EIT		eit, nexteit;

	// move entities
	for (eit = Entities.begin (); eit != Entities.end ();)
	{
		nexteit = eit; nexteit++;

		CEntity	&entity = (*eit).second;

		switch (entity.State)
		{
		case CEntity::Appear:
			stateAppear (entity);
			break;
		case CEntity::Normal:
			stateNormal (entity);
			break;
		case CEntity::Disappear:
			stateDisappear (entity);
			break;
		default:
			nlstop;
			break;
		}

		eit = nexteit;
	}

	// evaluate collisions
	MoveContainer->evalCollision(dt, 0);

	// snap entities to the ground
	for (eit = Entities.begin (); eit != Entities.end (); eit++)
	{
		CEntity	&entity = (*eit).second;
		UGlobalPosition	gPos;

		if (entity.Instance != NULL)
		{
			// get the global position in pacs coordinates system
			entity.MovePrimitive->getGlobalPosition(gPos, 0);
			// convert it in a vector 3d
			entity.Position = GlobalRetriever->getGlobalPosition(gPos);
			// get the good z position
			gPos.LocalPosition.Estimation.z = 0.0f;
			entity.Position.z = GlobalRetriever->getMeanHeight(gPos);
			// snap to the ground
			entity.VisualCollisionEntity->snapToGround(entity.Position);

			// check position retrieving
			UGlobalPosition gPosCheck;
			gPosCheck = GlobalRetriever->retrievePosition(entity.Position);
			if (gPos.InstanceId != gPosCheck.InstanceId ||
				gPos.LocalPosition.Surface != gPosCheck.LocalPosition.Surface)
			{
	//			nlwarning("Checked UGlobalPosition differs from store");
	//			gPos.InstanceId = gPosCheck.InstanceId;
	//			gPos.LocalPosition.Surface = gPosCheck.LocalPosition.Surface;
			}

			entity.Instance->setPos(entity.Position);
			CVector	jdir = CVector((float)cos(entity.Angle), (float)sin(entity.Angle), 0.0f);
			entity.Instance->setRotQuat(jdir);
		}

		if (entity.Particule != NULL)
			entity.Particule->setPos(entity.Position);
	}
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
	else if (var.Name == "RadarSelfColor") RadarSelfColor.set (var.asInt(0), var.asInt(1), var.asInt(2), var.asInt(3));
	else if (var.Name == "RadarEntitySize") RadarEntitySize = var.asFloat ();
	else if (var.Name == "RadarBorder") RadarBorder = var.asFloat ();
	else if (var.Name == "RadarZoom") RadarZoom = var.asFloat ();
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
	ConfigFile.setCallback ("RadarSelfColor", cbUpdateRadar);
	ConfigFile.setCallback ("RadarEntitySize", cbUpdateRadar);
	ConfigFile.setCallback ("RadarBorder", cbUpdateRadar);
	ConfigFile.setCallback ("RadarZoom", cbUpdateRadar);

	cbUpdateRadar (ConfigFile.getVar ("RadarPosX"));
	cbUpdateRadar (ConfigFile.getVar ("RadarPosY"));
	cbUpdateRadar (ConfigFile.getVar ("RadarWidth"));
	cbUpdateRadar (ConfigFile.getVar ("RadarHeight"));
	cbUpdateRadar (ConfigFile.getVar ("RadarFrontColor"));
	cbUpdateRadar (ConfigFile.getVar ("RadarBackColor"));
	cbUpdateRadar (ConfigFile.getVar ("RadarBorderColor"));
	cbUpdateRadar (ConfigFile.getVar ("RadarSelfColor"));
	cbUpdateRadar (ConfigFile.getVar ("RadarEntitySize"));
	cbUpdateRadar (ConfigFile.getVar ("RadarBorder"));
	cbUpdateRadar (ConfigFile.getVar ("RadarZoom"));
}

void updateRadar ()
{
	// Display the back of the radar
	Driver->setMatrixMode2D11 ();
	Driver->drawQuad (RadarPosX-RadarBorder, RadarPosY-RadarBorder, RadarPosX+RadarWidth+RadarBorder, RadarPosY+RadarHeight+RadarBorder, RadarBackColor);
	Driver->drawWiredQuad (RadarPosX, RadarPosY, RadarPosX+RadarWidth, RadarPosY+RadarHeight, RadarBorderColor);

	float CenterX = WorldWidth/2.0f;
	float CenterY = -WorldHeight/2.0f;
/* /// \todo remettre kan on aura des entities
	if (Self != NULL)
	{
		CenterX = Self->Position.x;
		CenterY = Self->Position.y;
	}
*/	
	for (EIT eit = Entities.begin (); eit != Entities.end (); eit++)
	{
		float userPosX, userPosY;

		// convert from world coords to radar coords (0.0 -> 1.0)

		userPosX = ((*eit).second.Position.x - CenterX) / (2.0f*WorldWidth);
		userPosY = ((*eit).second.Position.y - CenterY) / (2.0f*WorldHeight);

		// userpos is between -0.5 -> +0.5

		userPosX *= RadarZoom;
		userPosY *= RadarZoom;

		/// \todo virer kan ca marchera
		string str = toString(userPosX) + " / " + toString(userPosY) + " ** ";
		str += toString(CenterX) + " / " + toString(CenterY);
		TextContext->printfAt (0.5f, 0.5f, str.c_str());

		if (userPosX > 0.5f || userPosX < -0.5f || userPosY > 0.5f || userPosY < -0.5f)
			continue;

		userPosX += 0.5f;
		userPosY += 0.5f;

		// userpos is between 0.0 -> 1.0

		userPosX *= RadarWidth;
		userPosY *= RadarHeight;

		userPosX += RadarPosX;
		userPosY += RadarPosY;

		float entitySize = RadarEntitySize * ((RadarZoom <= 1.0f) ? 1.0f : RadarZoom);
		CRGBA entityColor = ((*eit).second.Type == CEntity::Self) ? RadarSelfColor : RadarFrontColor;

		Driver->drawQuad (userPosX, userPosY, entitySize, entityColor);
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
