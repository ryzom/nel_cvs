/** \file entities.h
 * 
 *
 * $Id: entities.h,v 1.17 2001/07/18 17:14:21 legros Exp $
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

#ifndef ENTITIES_H
#define ENTITIES_H

#include <string>
#include <map>

#include <nel/misc/vector.h>
#include <nel/misc/time_nl.h>

namespace NLSOUND
{
	class USource;
}

namespace NLPACS
{
	class UMovePrimitive;
}

namespace NL3D
{
	class UInstance;
	class UVisualCollisionEntity;
	class UPlayList;
	class USkeleton;
}

// An entity that will move through the landscape.
// The possible entities are the Self (the player's avatar), the Other and the Snowball
class CEntity
{
public:
	
	// Create a default entity
	CEntity () :
	  Id(0xffffffff), Name("<Unknown>"), AutoMove(false), Instance(NULL), Skeleton(NULL),
	  Particule(NULL), PlayList(NULL), CurrentAnimId(0xffffffff), NextEmptySlot(0), Source (NULL),
	  Angle(0.0f), AuxiliaryAngle(0.0f), InterpolatedAuxiliaryAngle(0.0f) { }


	// The id of the entity
	uint32							Id;

	// The name of the entity
	std::string						Name;

	// Contain the target position for this entity
	NLMISC::CVector					ServerPosition;

	// Contain the current position of the entity
	NLMISC::CVector					Position;
	// The maximum speed of the entity
	float							Speed, 
	// The angle of the entity
									Angle,
	// Various angle controls for the interpolation
									AuxiliaryAngle, InterpolatedAuxiliaryAngle;

	// The state enum of the entity
	enum TState	{ Appear, Normal, Disappear };

	// The state of this entity
	TState							State;
	// The date of the beginning of this state
	NLMISC::TTime					StateStartTime;

	// The type enum of the entity
	enum TType	{ Self, Other, Snowball };

	// The type of this entity
	TType							Type;

	// Is it an auto-moving entity
	bool							AutoMove;

	// The PACS move primitive
	NLPACS::UMovePrimitive			*MovePrimitive;
	// The collision entity (for ground snapping)
	NL3D::UVisualCollisionEntity	*VisualCollisionEntity;
	// The mesh instance associated to this entity
	NL3D::UInstance					*Instance;
	// The skeleton binded to the instance
	NL3D::USkeleton					*Skeleton;
	// The particle system (for appear and disappear effects)
	NL3D::UInstance					*Particule;

	//
	NL3D::UPlayList					*PlayList;

	// The sound source associated to the entity
	NLSOUND::USource				*Source;

	uint							 CurrentAnimId;
	uint							 NextEmptySlot;

	void	setState (TState state);

};

// The entity representing the player avatar
extern CEntity								*Self;

// The speed of the player
extern float								PlayerSpeed;

// The entities storage
extern std::map<uint32, CEntity>			Entities;
typedef std::map<uint32, CEntity>::iterator	EIT;


void	addEntity (uint32 eid, CEntity::TType type, const NLMISC::CVector &startPosition, const NLMISC::CVector &serverPosition);
void	removeEntity (uint32 eid);

void	initEntities();
void	updateEntities ();

// Reset the pacs position of an entity (in case pacs went wrong)
void	resetEntityPosition(uint32 eid);
// Process the event when an entity shoots a snowball
void	shotSnowball(uint32 eid, const NLMISC::CVector &startPos, const NLMISC::CVector &target);

void	renderEntitiesNames ();


#endif // ENTITIES_H

/* End of entities.h */
