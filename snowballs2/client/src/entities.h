/** \file entities.h
 * 
 *
 * $Id: entities.h,v 1.1 2001/07/12 12:54:15 lecroart Exp $
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

class CEntity
{
public:
	
	uint32 Id;

	std::string Name;

	float x, y, z, speed, angle;

	enum TType { Self, Other, Snowball };
	
	TType Type;
};



void addEntity (uint32 eid);
void removeEntity (uint32 eid);

void updateEntities ();


void initRadar ();
void updateRadar ();


#endif // ENTITIES_H

/* End of entities.h */
