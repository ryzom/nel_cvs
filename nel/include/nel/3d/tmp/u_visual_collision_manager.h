/** \file u_visual_collision_manager.h
 * Interface to visual collision manager.
 *
 * $Id: u_visual_collision_manager.h,v 1.1 2001/06/12 11:49:39 berenguier Exp $
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

#ifndef NL_U_VISUAL_COLLISION_MANAGER_H
#define NL_U_VISUAL_COLLISION_MANAGER_H

#include "nel/misc/types_nl.h"


namespace NL3D
{


class	ULandscape;
class	UVisualCollisionEntity;


/**
 * Interface to visual collision manager.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class UVisualCollisionManager
{
protected:
	UVisualCollisionManager() {}
	virtual ~UVisualCollisionManager() {}


public:

	/** setup the landscape used for this collision manager. ptr is kept, and manager must be cleared
	 * when the landscape is deleted.
	 */
	virtual void					setLandscape(ULandscape *landscape) =0;


	/** create an entity. NB: CVisualCollisionManager owns this ptr, and you must delete it with deleteEntity().
	 * NB: all CVisualCollisionEntity are deleted when this manager is deleted.
	 */
	virtual UVisualCollisionEntity	*createEntity() =0;

	/** delete an entity.
	 */
	virtual void					deleteEntity(UVisualCollisionEntity	*entity) =0;


};


} // NL3D


#endif // NL_U_VISUAL_COLLISION_MANAGER_H

/* End of u_visual_collision_manager.h */
