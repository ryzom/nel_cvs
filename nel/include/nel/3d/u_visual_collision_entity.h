/** \file u_visual_collision_entity.h
 * <File description>
 *
 * $Id: u_visual_collision_entity.h,v 1.2 2001/07/13 16:08:13 berenguier Exp $
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

#ifndef NL_U_VISUAL_COLLISION_ENTITY_H
#define NL_U_VISUAL_COLLISION_ENTITY_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"


namespace NL3D 
{


using	NLMISC::CVector;


/**
 * Interface to visual collision entity.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class UVisualCollisionEntity
{
protected:
	UVisualCollisionEntity() {}
	virtual ~UVisualCollisionEntity() {}

public:


	/** Snap the entity onto the ground. pos.z is modified so that it lies on the ground, according to rendered landscapes
	 * and meshes setuped into the visual collision manager.
	 * \return true if pos.z has been modified (sometimes it may not find a solution).
	 */
	virtual bool	snapToGround(CVector &pos) =0;


	/** Snap the entity onto the ground. pos.z is modified so that it lies on the ground, according to rendered landscapes
	 * and meshes setuped into the visual collision manager.
	 * \param normal the ret normal of where it is snapped. NB: if return false, not modified.
	 * \return true if pos.z has been modified (sometimes it may not find a solution).
	 */
	virtual bool	snapToGround(CVector &pos, CVector &normal) =0;


};


} // NL3D


#endif // NL_U_VISUAL_COLLISION_ENTITY_H

/* End of u_visual_collision_entity.h */
