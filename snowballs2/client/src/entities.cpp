/** \file commands.cpp
 * commands management with user interface
 *
 * $Id: entities.cpp,v 1.16 2001/07/17 13:57:34 lecroart Exp $
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
#include <nel/misc/time_nl.h>
#include <nel/misc/displayer.h>
#include <nel/misc/vector.h>
#include <nel/misc/vectord.h>
#include <nel/misc/time_nl.h>

#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_text_context.h>
#include <nel/3d/u_instance.h>
#include <nel/3d/u_scene.h>
#include <nel/3d/u_material.h>
#include <nel/3d/u_landscape.h>
#include <nel/3d/u_skeleton.h>

#include <nel/3d/u_visual_collision_manager.h>
#include <nel/3d/u_visual_collision_entity.h>

#include <nel/pacs/u_move_container.h>
#include <nel/pacs/u_move_primitive.h>
#include <nel/pacs/u_global_retriever.h>
#include <nel/pacs/u_global_position.h>


#include "client.h"
#include "entities.h"
#include "pacs.h"
#include "animation.h"
#include "camera.h"
#include "sound.h"
#include "mouse_listener.h"
#include "landscape.h"

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

float			WorldWidth = 20*160;
float			WorldHeight = 6*160;

uint32			NextEID = 0;
float			PlayerSpeed = 3.0f;	// 6 km/h
float			SnowballSpeed = 10.0f;	// 6 km/h

// these variables are set with the config file

float RadarPosX, RadarPosY, RadarWidth, RadarHeight, RadarBorder;
CRGBA RadarBackColor, RadarFrontColor, RadarBorderColor, RadarSelfColor;
float RadarEntitySize;
float RadarZoom;

float		EntityNameSize;
CRGBA		EntityNameColor;

CEntity		*Self = NULL;
UInstance	*AimingInstance = NULL;

void CEntity::setState (TState state)
{
	State = state;
	StateStartTime = CTime::getLocalTime ();
}



EIT findEntity (uint32 eid, bool needAssert = true)
{
	EIT entity = Entities.find (eid);
	if (entity == Entities.end () && needAssert)
	{
		nlerror ("Entity %u not found", eid);
	}
	return entity;
}


void addEntity (uint32 eid, CEntity::TType type, const CVector &startPosition, const CVector &serverPosition)
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
	entity.Angle = 0.0f;
	entity.ServerPosition = serverPosition;
	entity.VisualCollisionEntity = VisualCollisionManager->createEntity();

	// setup the move primitive and the mesh instance depending on the type of entity
	switch (type)
	{
	case CEntity::Self:
		entity.MovePrimitive = MoveContainer->addCollisionablePrimitive(0, 1);
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
		entity.Skeleton = Scene->createSkeleton ("fy_hom.skel");
		entity.Skeleton->bindSkin (entity.Instance);
		entity.Instance->hide ();
		entity.Speed = PlayerSpeed;

		entity.Particule = Scene->createInstance("appear.ps");

		entity.setState (CEntity::Appear);

		playAnimation (entity, IdleAnimId);

		break;
	case CEntity::Other:
		entity.MovePrimitive = MoveContainer->addCollisionablePrimitive(0, 1);
		entity.MovePrimitive->setPrimitiveType(UMovePrimitive::_2DOrientedCylinder);
		entity.MovePrimitive->setReactionType(UMovePrimitive::DoNothing);
		entity.MovePrimitive->setTriggerType(UMovePrimitive::NotATrigger);
		entity.MovePrimitive->setCollisionMask(OtherCollisionBit+SelfCollisionBit+SnowballCollisionBit);
		entity.MovePrimitive->setOcclusionMask(OtherCollisionBit);
		entity.MovePrimitive->setObstacle(true);
		entity.MovePrimitive->setRadius(0.5f);
		entity.MovePrimitive->setHeight(1.8f);
		entity.MovePrimitive->insertInWorldImage(0);
		entity.MovePrimitive->setGlobalPosition(CVectorD(startPosition.x, startPosition.y, startPosition.z), 0);

		entity.Instance = Scene->createInstance("barman.shape");
		entity.Skeleton = Scene->createSkeleton ("fy_hom.skel");
		entity.Skeleton->bindSkin (entity.Instance);
		entity.Instance->hide ();
		entity.Speed = PlayerSpeed;

		entity.Particule = Scene->createInstance("appear.ps");

		entity.setState (CEntity::Appear);

		playAnimation (entity, IdleAnimId);

		break;
	case CEntity::Snowball:
		// snowballs don't use pacs right now
/*
		entity.MovePrimitive = MoveContainer->addCollisionablePrimitive(0, 1);
		entity.MovePrimitive->setPrimitiveType(UMovePrimitive::_2DOrientedCylinder);
		entity.MovePrimitive->setReactionType(UMovePrimitive::Slide);
		entity.MovePrimitive->setTriggerType(UMovePrimitive::EnterTrigger);
		entity.MovePrimitive->setCollisionMask(SelfCollisionBit+OtherCollisionBit);
		entity.MovePrimitive->setOcclusionMask(SnowballCollisionBit);
		entity.MovePrimitive->setObstacle(false);
		entity.MovePrimitive->setRadius(0.2f);
		entity.MovePrimitive->setHeight(0.4f);
		entity.MovePrimitive->insertInWorldImage(0);
		entity.MovePrimitive->setGlobalPosition(CVectorD(startPosition.x, startPosition.y, startPosition.z), 0);
*/
		entity.MovePrimitive = NULL;

		// allows collision snapping to the ceiling
		entity.VisualCollisionEntity->setCeilMode(true);

//		entity.Instance = Scene->createInstance("box.shape");
		entity.Instance = Scene->createInstance("snowball.ps");
		entity.Speed = SnowballSpeed;

		playSound (entity, SoundId);

		entity.setState (CEntity::Normal);
		break;
	}

	if (entity.Skeleton != NULL)
		entity.Skeleton->setPos (startPosition);
	else
		entity.Instance->setPos (startPosition);

	if (entity.Source != NULL)
		entity.Source->setPosition (startPosition);

	if (entity.Particule != NULL)
		entity.Particule->setPos (startPosition);

}

void removeEntity (uint32 eid)
{
	nlinfo ("removing entity %u", eid);

	EIT eit = findEntity (eid);
	CEntity	&entity = (*eit).second;

	if (entity.Particule != NULL)
	{
		Scene->deleteInstance (entity.Particule);
		entity.Particule = NULL;
	}

//	if (entity.Type == CEntity::Other)
	{

		entity.Particule = Scene->createInstance("disappear.ps");
		entity.Particule->setPos (entity.Position);
	}

	entity.setState (CEntity::Disappear);
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
		if (entity.Particule != NULL)
		{
			Scene->deleteInstance (entity.Particule);
			entity.Particule = NULL;
		}

		entity.setState (CEntity::Normal);
	}

	if (entity.MovePrimitive != NULL)
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
		if (entity.Particule != NULL)
		{
			Scene->deleteInstance (entity.Particule);
			entity.Particule = NULL;
		}

		deleteAnimation (entity);

		if (entity.Skeleton != NULL)
		{
			entity.Skeleton->detachSkeletonSon (entity.Instance);
			Scene->deleteSkeleton (entity.Skeleton);
			entity.Skeleton = NULL;
		}

		if (entity.Instance != NULL)
		{
			Scene->deleteInstance (entity.Instance);
			entity.Instance = NULL;
		}

		if (entity.VisualCollisionEntity != NULL)
		{
			VisualCollisionManager->deleteEntity (entity.VisualCollisionEntity);
			entity.VisualCollisionEntity = NULL;
		}

		if (entity.MovePrimitive != NULL)
		{
			MoveContainer->removePrimitive(entity.MovePrimitive);
			entity.MovePrimitive = NULL;
		}

		deleteSound (entity);

		nlinfo ("Remove the entity %u from the Entities list", entity.Id);
		EIT eit = findEntity (entity.Id);
		Entities.erase (eit);
	}
	else
	{
		if (entity.MovePrimitive != NULL)
			entity.MovePrimitive->move(CVector(0,0,0), 0);
	}
}

void stateNormal (CEntity &entity)
{
	double	dt = (double)(NewTime-LastTime) / 1000.0;
	CVector	oldPos;
	CVector	newPos;

	oldPos = entity.Position;
	CVector	pDelta = entity.Position - entity.ServerPosition;
	CVector	pDeltaOri = pDelta;
	pDelta.z = 0.0f;

/// \todo remove when server entities will work
	// find a new random server position
	if (entity.Type == CEntity::Other && entity.AutoMove && pDelta.norm() < 0.1f)
	{
		float EntityMaxSpeed = 10.0f;
		entity.Angle += frand(1.5f)-0.75f;
		entity.ServerPosition += CVector((float)cos(entity.Angle),
										 (float)sin(entity.Angle),
										 0.0f)*EntityMaxSpeed;

		playAnimation (entity, WalkAnimId);
	}
	else if (entity.Type == CEntity::Snowball && entity.AutoMove && pDelta.norm() < 0.1f)
	{
		removeEntity(entity.Id);

	}
/// \todo end of the remove block when server entities will work


	if (entity.Type == CEntity::Self)
	{
		// the self entity
		// get new position
		newPos = MouseListener->getPosition();
		// get new orientation
		entity.Angle = MouseListener->getOrientation();

		if (Driver->AsyncListener.isKeyDown (KeyUP))
		{
			if (Driver->AsyncListener.isKeyDown (KeyLEFT))
			{
				entity.AuxiliaryAngle = (float)Pi/4.0f;
			}
			else if (Driver->AsyncListener.isKeyDown (KeyRIGHT))
			{
				entity.AuxiliaryAngle = -(float)Pi/4.0f;
			}
			else
			{
				entity.AuxiliaryAngle = 0;
			}
			playAnimation (*Self, WalkAnimId);
		}
		else if (Driver->AsyncListener.isKeyDown (KeyDOWN))
		{
			if (Driver->AsyncListener.isKeyDown (KeyLEFT))
			{
				entity.AuxiliaryAngle = (float)Pi-(float)Pi/4.0f;
			}
			else if (Driver->AsyncListener.isKeyDown (KeyRIGHT))
			{
				entity.AuxiliaryAngle = -(float)Pi+(float)Pi/4.0f;
			}
			else
			{
				entity.AuxiliaryAngle = (float)Pi;
			}
			playAnimation (*Self, WalkAnimId);
		}
		else if (Driver->AsyncListener.isKeyDown (KeyLEFT))
		{
			entity.AuxiliaryAngle = (float)Pi/2.0f;
			playAnimation (*Self, WalkAnimId);
		}
		else if (Driver->AsyncListener.isKeyDown (KeyRIGHT))
		{
			entity.AuxiliaryAngle = -(float)Pi/2.0f;
			playAnimation (*Self, WalkAnimId);
		}
		else
		{
			playAnimation (*Self, IdleAnimId);
		}

		float	sweepDistance = entity.AuxiliaryAngle-entity.InterpolatedAuxiliaryAngle;
		if (sweepDistance > (float)Pi)
		{
			sweepDistance -= (float)Pi*2.0f;
			entity.InterpolatedAuxiliaryAngle += (float)Pi*2.0f;
		}
		if (sweepDistance < -(float)Pi)
		{
			sweepDistance += (float)Pi*2.0f;
			entity.InterpolatedAuxiliaryAngle -= (float)Pi*2.0f;
		}
		float	sweepAngle = 4.0f*sweepDistance;
		entity.InterpolatedAuxiliaryAngle += (float)(sweepAngle*dt);
		if ((entity.AuxiliaryAngle-entity.InterpolatedAuxiliaryAngle)*sweepAngle <= 0.0)
			entity.InterpolatedAuxiliaryAngle = entity.AuxiliaryAngle;
		entity.Angle += entity.InterpolatedAuxiliaryAngle;

		entity.MovePrimitive->move((newPos-oldPos)/(float)dt, 0);
	}
	else if (entity.Type == CEntity::Other && pDelta.norm()>0.1f)
	{
		// go to the server position with linear interpolation
		/// \todo compute the linear interpolation

		pDelta.normalize();
		entity.Angle = (float)atan2(pDelta.y, pDelta.x);
		pDelta *= -entity.Speed;
		entity.MovePrimitive->move(pDelta, 0);

	}
	else if (entity.Type == CEntity::Snowball && pDeltaOri.norm()>0.1f)
	{
		// go to the server position with linear interpolation
		/// \todo compute the linear interpolation

		pDelta.normalize();
		pDeltaOri.normalize();
		entity.Angle = (float)atan2(pDelta.y, pDelta.x);
		pDeltaOri *= -entity.Speed;
		entity.Position += pDeltaOri*(float)dt;
		if ((entity.ServerPosition-entity.Position)*pDeltaOri < 0.0f)
		{
			entity.Position = entity.ServerPosition;
		}
	}
	else
	{
		// automatic speed
		/// \todo compute new entity position
		newPos = oldPos;
	}
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

/*
	if (Driver->AsyncListener.isKeyDown (KeyUP))
	{
		if (Self != NULL)
			playAnimation (*Self, WalkAnimId);
	}
	else
	{
		if (Self != NULL)
			playAnimation (*Self, IdleAnimId);
	}
*/	
	
	// evaluate collisions
	MoveContainer->evalCollision(dt, 0);

	// snap entities to the ground
	for (eit = Entities.begin (); eit != Entities.end (); eit++)
	{
		CEntity	&entity = (*eit).second;
		UGlobalPosition	gPos;

		if (entity.MovePrimitive != NULL)
		{
			// get the global position in pacs coordinates system
			entity.MovePrimitive->getGlobalPosition(gPos, 0);
			// convert it in a vector 3d
			entity.Position = GlobalRetriever->getGlobalPosition(gPos);
			// get the good z position
			gPos.LocalPosition.Estimation.z = 0.0f;
			entity.Position.z = GlobalRetriever->getMeanHeight(gPos);

			// check position retrieving
/*
			UGlobalPosition gPosCheck;
			gPosCheck = GlobalRetriever->retrievePosition(entity.Position);
			if (gPos.InstanceId != gPosCheck.InstanceId ||
				gPos.LocalPosition.Surface != gPosCheck.LocalPosition.Surface)
			{
				nlwarning("Checked UGlobalPosition differs from store");
//				gPos.InstanceId = gPosCheck.InstanceId;
//				gPos.LocalPosition.Surface = gPosCheck.LocalPosition.Surface;
			}
*/
			// snap to the ground
			entity.VisualCollisionEntity->snapToGround(entity.Position);
		}

		if (entity.Type == CEntity::Snowball && SnapSnowballs)
		{
			CVector	snapPos = entity.Position;
			entity.VisualCollisionEntity->snapToGround(snapPos);
			entity.Instance->setPos(snapPos);
			CVector	jdir = CVector((float)cos(entity.Angle), (float)sin(entity.Angle), 0.0f);
			entity.Instance->setRotQuat(jdir);
		}
		else if (entity.Instance != NULL)
		{
			CVector	jdir = CVector(-(float)cos(entity.Angle), -(float)sin(entity.Angle), 0.0f);

			if (entity.Skeleton != NULL)
			{
				entity.Skeleton->setPos(entity.Position);
				entity.Skeleton->setRotQuat(jdir);
			}
			else
			{
				entity.Instance->setPos(entity.Position);
				entity.Instance->setRotQuat(jdir);
			}
		}

		if (entity.Source != NULL)
			entity.Source->setPosition (entity.Position);

		if (entity.Particule != NULL)
		{
			entity.Particule->setPos(entity.Position);
		}

		if (entity.Type == CEntity::Self)
		{
			MouseListener->setPosition(entity.Position);
		}
	}
}

void renderEntitiesNames ()
{
	Driver->setMatrixMode3D (*Camera);
	TextContext->setHotSpot (UTextContext::MiddleTop);
	TextContext->setColor (EntityNameColor);
	TextContext->setFontSize ((uint32)EntityNameSize);
	for (EIT eit = Entities.begin (); eit != Entities.end (); eit++)
	{
		CEntity	&entity = (*eit).second;
		if (entity.Instance != NULL && entity.Type == CEntity::Other)
		{
			CMatrix		mat = Camera->getMatrix();
			mat.setPos(entity.Position + CVector(0.0f, 0.0f, 2.0f));
			mat.scale(4.0f);
			TextContext->render3D(mat, entity.Name);
		}
	}
}

void cbUpdateEntities (CConfigFile::CVar &var)
{
	if (var.Name == "EntityNameColor") EntityNameColor.set (var.asInt(0), var.asInt(1), var.asInt(2), var.asInt(3));
	else if (var.Name == "EntityNameSize") EntityNameSize = var.asFloat ();
	else nlwarning ("Unknown variable update %s", var.Name.c_str());
}

void initEntities()
{
	ConfigFile.setCallback ("EntityNameColor", cbUpdateEntities);
	ConfigFile.setCallback ("EntityNameSize", cbUpdateEntities);

	cbUpdateEntities (ConfigFile.getVar ("EntityNameColor"));
	cbUpdateEntities (ConfigFile.getVar ("EntityNameSize"));
}

void releaseEntities()
{
}

//
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



void displayRadarPoint (const CVector &Position, const CVector &Center, float Size, const CRGBA &Color)
{
	float userPosX, userPosY;

	// convert from world coords to radar coords (0.0 -> 1.0)
	userPosX = (Position.x - Center.x) / (2.0f*WorldWidth);
	userPosY = (Position.y - Center.y) / (2.0f*WorldHeight);
	// userpos is between -0.5 -> +0.5
	userPosX *= RadarZoom;
	userPosY *= RadarZoom;
/*
	/// \todo virer kan ca marchera
	string str = toString(userPosX) + " / " + toString(userPosY) + " ** ";
	str += toString(Center.x) + " / " + toString(Center.y) + " *** ";
	str += toString(Position.x) + " / " + toString(Position.y);
	TextContext->printfAt (0.1f, 0.5f, str.c_str());
*/
	if (userPosX > 0.5f || userPosX < -0.5f || userPosY > 0.5f || userPosY < -0.5f)
		return;
	userPosX += 0.5f;
	userPosY += 0.5f;
	// userpos is between 0.0 -> 1.0

	userPosX *= RadarWidth;
	userPosY *= RadarHeight;

	userPosX += RadarPosX;
	userPosY += RadarPosY;

	Driver->drawQuad (userPosX, userPosY, Size, Color);
}


void updateRadar ()
{
	// Display the back of the radar
	Driver->setMatrixMode2D11 ();
	Driver->drawQuad (RadarPosX-RadarBorder, RadarPosY-RadarBorder, RadarPosX+RadarWidth+RadarBorder, RadarPosY+RadarHeight+RadarBorder, RadarBackColor);
	Driver->drawWiredQuad (RadarPosX, RadarPosY, RadarPosX+RadarWidth, RadarPosY+RadarHeight, RadarBorderColor);

	CVector Center;
	Center.x = WorldWidth/2.0f;
	Center.y = -WorldHeight/2.0f;

	// the center of the radar is the player
	if (Self != NULL)
	{
		Center.x = Self->Position.x;
		Center.y = Self->Position.y;
	}

	float entitySize = RadarEntitySize * ((RadarZoom <= 1.0f) ? 1.0f : RadarZoom);
	if (entitySize > RadarBorder) entitySize = RadarBorder;
	if (entitySize < RadarEntitySize * 16) entitySize = RadarEntitySize * 16;

	for (EIT eit = Entities.begin (); eit != Entities.end (); eit++)
	{

		CRGBA entityColor = ((*eit).second.Type == CEntity::Self) ? RadarSelfColor : RadarFrontColor;

		displayRadarPoint ((*eit).second.Position, Center, entitySize, entityColor);

		displayRadarPoint ((*eit).second.ServerPosition, Center, entitySize, CRGBA (255,255,255,255));
	}
}

//
void	resetEntityPosition(uint32 eid)
{
	uint32 sbid = NextEID++;
	EIT eit = findEntity (eid);

	CEntity	&entity = (*eit).second;

	UGlobalPosition	gPos;
	// get the global position
	gPos = GlobalRetriever->retrievePosition(entity.Position);
	// convert it in a vector 3d
	entity.Position = GlobalRetriever->getGlobalPosition(gPos);
	// get the good z position
	gPos.LocalPosition.Estimation.z = 0.0f;
	entity.Position.z = GlobalRetriever->getMeanHeight(gPos);

	// snap to the ground
	if (entity.VisualCollisionEntity != NULL)
		entity.VisualCollisionEntity->snapToGround(entity.Position);

	if (entity.MovePrimitive != NULL)
		entity.MovePrimitive->setGlobalPosition(CVector(entity.Position.x, entity.Position.y, entity.Position.z), 0);
}

void	shotSnowball(uint32 eid, const CVector &target)
{
	uint32 sbid = NextEID++;
	EIT eit = findEntity (eid);

	CEntity	&launcher = (*eit).second;
	CVector	start = launcher.Position;
	start.z += 1.3f;
	CVector direction = (target-start).normed();
	start += direction*1.0f;

	addEntity(sbid, CEntity::Snowball, start, target);
	eit = findEntity (sbid);
	CEntity	&snowball = (*eit).second;
	snowball.AutoMove = 1;


	if (launcher.Type == CEntity::Self)
	{
		snowball.ServerPosition = getTarget(start, direction, 100);
/*
		const uint	numTestStep = 100;
		CVector	testPos = start;
		CVector	step = (target-start)/numTestStep;

		uint	i;
		for (i=0; i<numTestStep; ++i)
		{
			CVector	snapped = testPos;
			CVector	normal;
			// here use normal to check if we have collision
			if (snowball.VisualCollisionEntity->snapToGround(snapped, normal) && (testPos.z-snapped.z)*normal.z < 0.0f)
			{
				testPos -= step*0.5f;
				break;
			}
			testPos += step;
		}

		snowball.ServerPosition = testPos;
*/
  }
}


//
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

	for (uint i = 0; i < nb ; i++)
	{
		uint32 eid = NextEID++;
		float x = 1000.0f; ///frand(1000.0f);
		float y = -1000.0f; ///-frand(1000.0f);
		float z = 2.0f;
		addEntity (eid, CEntity::Other, CVector(x, y, z), CVector(x, y, z));
		EIT eit = findEntity (eid);
		(*eit).second.AutoMove = atoi(args[1].c_str()) == 1;
	}

	return true;
}
