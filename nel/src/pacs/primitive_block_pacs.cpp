/** \file primitive_block_pacs.cpp
 * Block of PACS primitives
 *
 * $Id: primitive_block_pacs.cpp,v 1.1 2002/03/26 10:11:43 corvazier Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "stdpacs.h"
#include "pacs/primitive_block.h"


namespace NLPACS 
{

// ***************************************************************************

CPrimitiveDesc::CPrimitiveDesc ()
{
	Attenuation = 1;
	Type = UMovePrimitive::_2DOrientedBox;
	Reaction = UMovePrimitive::DoNothing;
	Trigger = UMovePrimitive::NotATrigger;
	Obstacle = true;
	OcclusionMask = 0xffffffff;
	CollisionMask = 0xffffffff;
}

// ***************************************************************************

void CPrimitiveDesc::serial (NLMISC::IStream &s)
{
	// Serial the version
	int ver = s.serialVersion (0);
	
	s.xmlPush ("LENGTH");
	s.serial (Length[0]);
	s.serial (Length[1]);
	s.xmlPop ();

	s.xmlSerial (Height, "HEIGHT");
	s.xmlSerial (Attenuation, "ATTENUATION");

	s.xmlPush ("TYPE");
	s.serialEnum (Type);
	s.xmlPop ();

	s.xmlPush ("REACTION");
	s.serialEnum (Reaction);
	s.xmlPop ();

	s.xmlPush ("TRIGGER");
	s.serialEnum (Trigger);
	s.xmlPop ();

	s.xmlSerial (Obstacle, "OBSTACLE");
	s.xmlSerial (OcclusionMask, "OCCLUSION_MASK");
	s.xmlSerial (CollisionMask, "COLLISION_MASK");
	s.xmlSerial (Position, "POSITION");
	s.xmlSerial (Orientation, "ORIENTATION");
}

// ***************************************************************************

void CPrimitiveBlock::serial (NLMISC::IStream &s)
{
	s.xmlPush ("PRIMITIVE_BLOCK");

	// Serial checks
	s.serialCheck ((uint32)'KBRP');

	// Serial the version
	int ver = s.serialVersion (0);

	s.xmlPush ("PRIMITIVES");
	s.serialCont (Primitives);
	s.xmlPop ();

	s.xmlPop ();
}

// ***************************************************************************

} // NLPACS
