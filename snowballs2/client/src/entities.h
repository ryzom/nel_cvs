/** \file entities.h
 * 
 *
 * $Id: entities.h,v 1.8 2001/07/13 16:17:41 legros Exp $
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
};

class CEntity
{
public:
	
	CEntity () : Id(0xffffffff), Name("<Unknown>"), AutoMove(false), Instance(NULL), Particule(NULL) { }

	uint32							Id;

	std::string						Name;

	// Contain the target position for this entity
	NLMISC::CVector					ServerPosition;

	// Contain the current position of the entity
	NLMISC::CVector					Position;
	float							Speed, 
									Angle;

	enum TType { Self, Other, Snowball };
	
	TType							Type;

	enum TState { Appear, Normal, Disappear };
	
	TState							State;

	NLMISC::TTime					StateStartTime;

	bool							AutoMove;

	NLPACS::UMovePrimitive			*MovePrimitive;
	NL3D::UVisualCollisionEntity	*VisualCollisionEntity;
	NL3D::UInstance					*Instance;
	NL3D::UInstance					*Particule;
};

extern CEntity	*Self;

extern float	EntitySpeed;

void	addEntity (uint32 eid, CEntity::TType type, const NLMISC::CVector &startPosition, const NLMISC::CVector &serverPosition);
void	removeEntity (uint32 eid);

void	updateEntities ();

void	shotSnowball(uint32 eid, const NLMISC::CVector &target);

void	initRadar ();
void	updateRadar ();


#endif // ENTITIES_H

/* End of entities.h */
