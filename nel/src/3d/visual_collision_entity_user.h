/** \file visual_collision_entity_user.h
 * UVisualCollisionEntity implementation.
 *
 * $Id: visual_collision_entity_user.h,v 1.7 2002/05/23 14:40:18 berenguier Exp $
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

#ifndef NL_VISUAL_COLLISION_ENTITY_USER_H
#define NL_VISUAL_COLLISION_ENTITY_USER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/u_visual_collision_entity.h"
#include "3d/visual_collision_entity.h"
#include "3d/visual_collision_manager.h"


namespace NL3D 
{


/**
 * UVisualCollisionEntity implementation.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CVisualCollisionEntityUser : public UVisualCollisionEntity
{
public:

	/// Constructor. create entity.
	CVisualCollisionEntityUser(CVisualCollisionManager *manager)
	{
		_Manager= manager;
		_Entity= _Manager->createEntity();
	}
	/// dtor, delete the entity.
	~CVisualCollisionEntityUser()
	{
		_Manager->deleteEntity(_Entity);
	}


	virtual bool	snapToGround(CVector &pos)
	{
		return _Entity->snapToGround(pos);
	}
	virtual bool	snapToGround(CVector &pos, CVector &normal)
	{
		return _Entity->snapToGround(pos, normal);
	}



	virtual void	setGroundMode(bool groundMode)
	{
		_Entity->setGroundMode(groundMode);
	}
	virtual void	setCeilMode(bool ceilMode)
	{
		_Entity->setCeilMode(ceilMode);
	}
	virtual bool	getGroundMode() const
	{
		return _Entity->getGroundMode();
	}
	virtual bool	getCeilMode() const
	{
		return _Entity->getCeilMode();
	}


	virtual void	setSnapToRenderedTesselation(bool snapMode)
	{
		_Entity->setSnapToRenderedTesselation(snapMode);
	}
	virtual bool	getSnapToRenderedTesselation() const
	{
		return _Entity->getSnapToRenderedTesselation();
	}


	virtual bool	getStaticLightSetup(const CVector &pos, std::vector<CPointLightInfluence> &pointLightList, 
		uint8 &sunContribution, NLMISC::CRGBA &localAmbient)
	{
		return _Entity->getStaticLightSetup(pos, pointLightList, sunContribution, localAmbient);
	}


	virtual void	displayDebugGrid(UDriver &drv) const;


private:
	CVisualCollisionManager		*_Manager;
	CVisualCollisionEntity		*_Entity;

};


} // NL3D


#endif // NL_VISUAL_COLLISION_ENTITY_USER_H

/* End of visual_collision_entity_user.h */
