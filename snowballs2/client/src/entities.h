/** \file entities.h
 * 
 *
 * $Id: entities.h,v 1.11 2001/07/17 12:27:42 legros Exp $
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

#include <nel/misc/vector.h>
#include <nel/misc/time_nl.h>

namespace NLPACS
{
	class UMovePrimitive;
};

namespace NL3D
{
	class UInstance;
	class UVisualCollisionEntity;
	class UPlayList;
	class USkeleton;
};

class CEntity
{
public:
	
	CEntity () : Id(0xffffffff), Name("<Unknown>"), AutoMove(false), Instance(NULL), Skeleton(NULL), Particule(NULL), PlayList(NULL), CurrentAnimId(0xffffffff) { }

	uint32							Id;

	std::string						Name;

	// Contain the target position for this entity
	NLMISC::CVector					ServerPosition;

	// Contain the current position of the entity
	NLMISC::CVector					Position;
	float							Speed, 
									Angle,
									AuxiliaryAngle, InterpolatedAuxiliaryAngle;

	enum TState	{ Appear, Normal, Disappear };

	TState							State;
	NLMISC::TTime					StateStartTime;

	enum TType	{ Self, Other, Snowball };

	TType							Type;

	bool							AutoMove;

	NLPACS::UMovePrimitive			*MovePrimitive;
	NL3D::UVisualCollisionEntity	*VisualCollisionEntity;
	NL3D::UInstance					*Instance;
	NL3D::USkeleton					*Skeleton;
	NL3D::UInstance					*Particule;

	NL3D::UPlayList					*PlayList;

	uint							 CurrentAnimId;

	void	setState (TState state);

};

extern CEntity	*Self;

extern float	 PlayerSpeed;

void	addEntity (uint32 eid, CEntity::TType type, const NLMISC::CVector &startPosition, const NLMISC::CVector &serverPosition);
void	removeEntity (uint32 eid);

void	updateEntities ();
void	initEntities();

void	resetEntityPosition(uint32 eid);
void	shotSnowball(uint32 eid, const NLMISC::CVector &target);

void	initRadar ();
void	updateRadar ();
void	renderEntitiesNames ();


#endif // ENTITIES_H

/* End of entities.h */
