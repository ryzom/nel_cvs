/** \file commands.cpp
 * Snowballs 2 specific code for managing the command interface
 *
 * $Id: entities.cpp,v 1.29 2001/07/20 09:55:49 lecroart Exp $
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

//
// Namespaces
//

using namespace std;
using namespace NLMISC;
using namespace NL3D;
using namespace NLPACS;

//
// Variables
//

// The collision bits used by pacs (dynamic collisions)
enum
{
	SelfCollisionBit = 1,
	OtherCollisionBit = 2,
	SnowballCollisionBit = 4
};

// A map of entities. All entities are later reffered by their unique id
map<uint32, CEntity>	Entities;

CEntity					*Self = NULL;

UInstance				*AimingInstance = NULL;
TTime					LastAimingUpdate = 0;
TTime					RefreshRate = 100;

// The size of the world, in meter
float					WorldWidth = 20*160;
float					WorldHeight = 6*160;

// Entity Id, only used offline
uint32					NextEID = 0;

// The speed settings
float					PlayerSpeed = 1.8f;		// 6.5 km/h
float					SnowballSpeed = 10.0f;	// 36 km/h

// these variables are set with the config file

// Setup for the name up the character
float					EntityNameSize;
CRGBA					EntityNameColor;

//
// Functions
//

// Set the state of the entity (Appear, Normal, Disappear)
void CEntity::setState (TState state)
{
	State = state;
	StateStartTime = CTime::getLocalTime ();
}


// Get an map iterator on a entity, specified by its id
EIT findEntity (uint32 eid, bool needAssert = true)
{
	EIT entity = Entities.find (eid);
	if (entity == Entities.end () && needAssert)
	{
		nlerror ("Entity %u not found", eid);
	}
	return entity;
}


// Creates an entity, given its id, its type (Self, Other, Snowball), its start and server positions.
void addEntity (uint32 eid, CEntity::TType type, const CVector &startPosition, const CVector &serverPosition)
{
	nlinfo ("adding entity %u", eid);

	// Check that the entity doesn't exist yet
	EIT eit = findEntity (eid, false);
	if (eit != Entities.end ())
	{
		nlerror ("Entity %d already exist", eid);
	}

	// Create a new entity
	eit = (Entities.insert (make_pair (eid, CEntity()))).first;
	CEntity	&entity = (*eit).second;

	// Check that in the case the entity newly created is a Self, ther isn't a Self yet.
	if (type == CEntity::Self)
	{
		if (Self != NULL)
			nlerror("Self entity already created");

		Self = &entity;
	}

	// Set the entity up
	entity.Id = eid;
	entity.Type = type;
	entity.Name = "Entity"+toString(eid);
	entity.Position = startPosition;
	entity.Angle = 0.0f;
	entity.ServerPosition = serverPosition;
	entity.VisualCollisionEntity = VisualCollisionManager->createEntity();

	// setup the move primitive and the mesh instance depending on the type of entity
	switch (type)
	{
	case CEntity::Self:
		// create a move primitive associated to the entity
		entity.MovePrimitive = MoveContainer->addCollisionablePrimitive(0, 1);
		// it's a cylinder
		entity.MovePrimitive->setPrimitiveType(UMovePrimitive::_2DOrientedCylinder);
		// the entity should slide against obstacles
		entity.MovePrimitive->setReactionType(UMovePrimitive::Slide);
		// do not generate event if there is a collision
		entity.MovePrimitive->setTriggerType(UMovePrimitive::NotATrigger);
		// which entity should collide against me
		entity.MovePrimitive->setCollisionMask(OtherCollisionBit+SnowballCollisionBit);
		// the self collision bit
		entity.MovePrimitive->setOcclusionMask(SelfCollisionBit);
		// the self is an obstacle
		entity.MovePrimitive->setObstacle(true);
		// the size of the cylinder
		entity.MovePrimitive->setRadius(0.5f);
		entity.MovePrimitive->setHeight(1.8f);
		// only use one world image, so use insert in world image 0
		entity.MovePrimitive->insertInWorldImage(0);
		// retreive the start position of the entity
		entity.MovePrimitive->setGlobalPosition(CVectorD(startPosition.x, startPosition.y, startPosition.z), 0);

		// create instance of the mesh character
		entity.Instance = Scene->createInstance("gnu.shape");
		entity.Skeleton = Scene->createSkeleton ("gnu.skel");
		// use the instance on the skeleton
		entity.Skeleton->bindSkin (entity.Instance);
		entity.Instance->hide ();

		// setup final parameters
		entity.Speed = PlayerSpeed;
		entity.Particule = Scene->createInstance("appear.ps");
		entity.setState (CEntity::Appear);
		playAnimation (entity, IdleAnim);

		break;
	case CEntity::Other:
		entity.MovePrimitive = MoveContainer->addCollisionablePrimitive(0, 1);
		entity.MovePrimitive->setPrimitiveType(UMovePrimitive::_2DOrientedCylinder);
		entity.MovePrimitive->setReactionType(UMovePrimitive::Slide);
		entity.MovePrimitive->setTriggerType(UMovePrimitive::NotATrigger);
		entity.MovePrimitive->setCollisionMask(OtherCollisionBit+SelfCollisionBit+SnowballCollisionBit);
		entity.MovePrimitive->setOcclusionMask(OtherCollisionBit);
		entity.MovePrimitive->setObstacle(true);
		entity.MovePrimitive->setRadius(0.5f);
		entity.MovePrimitive->setHeight(1.8f);
		entity.MovePrimitive->insertInWorldImage(0);
		entity.MovePrimitive->setGlobalPosition(CVectorD(startPosition.x, startPosition.y, startPosition.z), 0);

		entity.Instance = Scene->createInstance("gnu.shape");
		entity.Skeleton = Scene->createSkeleton ("gnu.skel");
		entity.Skeleton->bindSkin (entity.Instance);
		entity.Instance->hide ();

		entity.Speed = PlayerSpeed;
		entity.Particule = Scene->createInstance("appear.ps");
		entity.setState (CEntity::Appear);
		playAnimation (entity, IdleAnim);

		break;
	case CEntity::Snowball:
		entity.MovePrimitive = NULL;

		// allows collision snapping to the ceiling
		entity.VisualCollisionEntity->setCeilMode(true);

		entity.Instance = Scene->createInstance("snowball.ps");
		entity.Skeleton = NULL;
		entity.Speed = SnowballSpeed;

		playSound (entity, SoundId);

		entity.setState (CEntity::Normal);
		break;
	}

	if (entity.Skeleton != NULL)
		entity.Skeleton->setPos (startPosition);
	else
		entity.Instance->setPos (startPosition);

// todo sound
//	if (entity.Source != NULL)
//		entity.Source->setPosition (startPosition);

	if (entity.Particule != NULL)
		entity.Particule->setPos (startPosition);

}

// Remove an entity specified by its id
// The entity passes into the Disappear state
void removeEntity (uint32 eid)
{
	nlinfo ("removing entity %u", eid);

	// look for the entity
	EIT eit = findEntity (eid);
	CEntity	&entity = (*eit).second;

	// if there is a particule system linked, delete it
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

// What to do when the entity appears
void stateAppear (CEntity &entity)
{
	// after 1 second, show the instance
	if (CTime::getLocalTime () > entity.StateStartTime + 1000)
	{
		if (entity.Instance->getVisibility () != UTransform::Show)
			entity.Instance->show ();
	}

	// after 5 seconds, delete the particle system (if any)
	// and passe the entity into the Normal state
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

// What to do when the entity disappears
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

	// find a new random server position
	if (entity.Type == CEntity::Other && entity.AutoMove && pDelta.norm() < 0.1f)
	{
		float EntityMaxSpeed = 10.0f;
		entity.AuxiliaryAngle += frand(1.5f)-0.75f;
		entity.ServerPosition += CVector((float)cos(entity.AuxiliaryAngle),
										 (float)sin(entity.AuxiliaryAngle),
										 0.0f)*EntityMaxSpeed;

		playAnimation (entity, WalkAnim);
	}
	else if (entity.Type == CEntity::Snowball && entity.AutoMove && pDelta.norm() < 0.1f)
	{
		removeEntity(entity.Id);
	}

	if (entity.Type == CEntity::Self)
	{
		// the self entity
		// get new position
		newPos = MouseListener->getPosition();
		// get new orientation
		entity.Angle = MouseListener->getOrientation();

		bool	isAiming = MouseListener->getAimingState();

		// modify the orientation depending on the straff
		// The straff is determined by the keys that are down simultaneously
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
			playAnimation (*Self, isAiming ? PrepareSnowBall : WalkAnim);
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
			playAnimation (*Self, isAiming ? PrepareSnowBall : WalkAnim);
		}
		else if (Driver->AsyncListener.isKeyDown (KeyLEFT))
		{
			entity.AuxiliaryAngle = (float)Pi/2.0f;
			playAnimation (*Self, isAiming ? PrepareSnowBall : WalkAnim);
		}
		else if (Driver->AsyncListener.isKeyDown (KeyRIGHT))
		{
			entity.AuxiliaryAngle = -(float)Pi/2.0f;
			playAnimation (*Self, isAiming ? PrepareSnowBall : WalkAnim);
		}
		else
		{
			playAnimation (*Self, isAiming ? PrepareSnowBall : IdleAnim);
		}

		if (isAiming)
			entity.AuxiliaryAngle = 0.0f;

		// Interpolate the character orientation towards the server angle
		// for smoother movements
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

		// tell the move container how much the entity should move
		entity.MovePrimitive->move((newPos-oldPos)/(float)dt, 0);

		// If the player is aiming (left mouse button down), show the target
		if (MouseListener->getAimingState() && Self != NULL && AimingInstance != NULL)
		{
			// We only refresh the target at predefined rate to avoid to much cpu consuming
			// If it's time to update the target
			if (CTime::getLocalTime() - LastAimingUpdate > RefreshRate)
			{
				LastAimingUpdate = CTime::getLocalTime();
				// get the start of the snowball
				CVector start = MouseListener->getPosition()+CVector(0.0f, 0.0f, 1.3f);
				// get the direction of aiming
				CVector direction = MouseListener->getViewDirection().normed();
				// and compute the target
				CVector	target = getTarget(start, direction, 100);
				// Eventually, setup the aiming mesh
				AimingInstance->lookAt(target, Camera->getMatrix().getPos());
				AimingInstance->show();
			}
			float	scale = MouseListener->getDamage();
			AimingInstance->setScale(scale, scale, scale);
		}
		else
		{
			// If the player is not aiming, just hide the target
			LastAimingUpdate = 0;
			AimingInstance->hide();
		}
	}
	else if (entity.Type == CEntity::Other && pDelta.norm()>0.1f)
	{
		// go to the server position with linear interpolation

		// Interpolate orientation for smoother motions
		// AuxiliaryAngle -> the server imposed angle
		// InterpolatedAuxiliaryAngle -> the angle showed by the entity
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

		entity.Angle = entity.InterpolatedAuxiliaryAngle;

		pDelta.normalize();
		pDelta *= -entity.Speed;
		entity.MovePrimitive->move(pDelta, 0);

	}
	else if (entity.Type == CEntity::Snowball && pDeltaOri.norm()>0.1f)
	{
		// go to the server position with linear interpolation

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
		newPos = oldPos;
	}
}

void updateEntities ()
{
	// compute the delta t that has elapsed since the last update (in seconds)
	double	dt = (double)(NewTime-LastTime) / 1000.0;
	EIT		eit, nexteit;

	// move entities
	for (eit = Entities.begin (); eit != Entities.end (); )
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

		if (entity.Instance != NULL)
		{
			CVector	jdir;
			switch (entity.Type)
			{
			case CEntity::Self:
				jdir = CVector(-(float)cos(entity.Angle), -(float)sin(entity.Angle), 0.0f);
				break;
			case CEntity::Other:
				jdir = CVector(-(float)cos(entity.Angle), -(float)sin(entity.Angle), 0.0f);
				break;
			case CEntity::Snowball:
				jdir = CVector(-(float)cos(entity.Angle), -(float)sin(entity.Angle), 0.0f);
				break;
			}

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

// todo sound
//		if (entity.Source != NULL)
//			entity.Source->setPosition (entity.Position);

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

// Draw entities names up the characters
void renderEntitiesNames ()
{
	// Setup the driver in matrix mode
	Driver->setMatrixMode3D (*Camera);
	// Setup the drawing context
	TextContext->setHotSpot (UTextContext::MiddleTop);
	TextContext->setColor (EntityNameColor);
	TextContext->setFontSize ((uint32)EntityNameSize);
	//
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


// The entities preferences callback
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

	AimingInstance = Scene->createInstance("box.shape");
	AimingInstance->setTransformMode(UTransformable::RotQuat);
}

void releaseEntities()
{
	Scene->deleteInstance(AimingInstance);
	AimingInstance = NULL;
}


// Reset the pacs position of an entity, in case pacs went wrong
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


void	shotSnowball(uint32 eid, const CVector &start, const CVector &target)
{
	uint32 sbid = NextEID++;
	EIT eit = findEntity (eid);

	CEntity	&launcher = (*eit).second;

	// get direction
	CVector direction = (target-start).normed();

	// create a new snowball entity
	addEntity(sbid, CEntity::Snowball, start, target);
	eit = findEntity (sbid);
	CEntity	&snowball = (*eit).second;
	snowball.AutoMove = 1;

	if (launcher.Type == CEntity::Self)
	{
		/// \todo Ben inform the server the player is shooting a snowball
		snowball.ServerPosition = getTarget(start, direction, 200);
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

NLMISC_COMMAND(set_speed,"set the speed of an identity","<eid> <speed>")
{
	// check args, if there s not the right number of parameter, return bad
	if(args.size() != 2) return false;

	uint eid = (uint)atoi(args[0].c_str());
	EIT eit = findEntity (eid);
	CEntity	&entity = (*eit).second;

	entity.Speed = (float)atof(args[1].c_str());
	if (entity.Type == CEntity::Self)
	{
		MouseListener->setSpeed(entity.Speed);
	}
	return true;
}

NLMISC_COMMAND(speed,"set the player speed","<speed in km/h>")
{
	// check args, if there s not the right number of parameter, return bad
	if(args.size() != 1) return false;
	float speed = min( max( (float)atof(args[0].c_str()), 0.1f ), 200.0f ); // speed range in km/h
	if (Self != NULL)
	{
		MouseListener->setSpeed( speed / 3.6f );
	}
	return true;
}

NLMISC_COMMAND(goto, "go to a given position", "<x> <y>")
{
	// check args, if there s not the right number of parameter, return bad
	if(args.size() != 2) return false;

	if (Self == NULL)
	{
		return false;
	}

	CEntity	&entity = *Self;

	float	x, y;

	x = (float)atof(args[1].c_str());
	y = (float)atof(args[2].c_str());

	//
	if (entity.MovePrimitive != NULL && entity.VisualCollisionEntity != NULL)
	{
		UGlobalPosition	gPos;
		entity.MovePrimitive->setGlobalPosition(CVectorD(x, y, 0.0f), 0);
		// get the global position in pacs coordinates system
		entity.MovePrimitive->getGlobalPosition(gPos, 0);
		// convert it in a vector 3d
		entity.Position = GlobalRetriever->getGlobalPosition(gPos);
		// get the good z position
		gPos.LocalPosition.Estimation.z = 0.0f;
		entity.Position.z = GlobalRetriever->getMeanHeight(gPos);
		// snap to the ground
		entity.VisualCollisionEntity->snapToGround(entity.Position);

		if (entity.Type == CEntity::Self)
		{
			MouseListener->setPosition(entity.Position);
		}
	}

	return true;
}
