/** \file instance_group_user.h
 * Implementation of the user interface managing instance groups.
 *
 * $Id: instance_group_user.h,v 1.1 2001/06/15 16:24:43 corvazier Exp $
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

#ifndef NL_INSTANCE_GROUP_USER_H
#define NL_INSTANCE_GROUP_USER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/u_instance_group.h"
#include "3d/scene_group.h"

namespace NLMISC
{
	class CVector;
	class CQuat;
}

namespace NL3D 
{


/**
 * Implementation of the user interface managing instance groups.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CInstanceGroupUser : public UInstanceGroup
{
public:
	// Init with a scene.
	//bool load (const std::string &instanceGroup);

	// Init without a scene
	bool init (const std::string &instanceGroup);
	bool init (const std::string &instanceGroup, CScene &scene);

private:
	// From UInstanceGroup
	void addToScene (class UScene& scene);
	void removeFromScene (class UScene& scene);
	uint getNumInstance () const;
	const std::string& getInstanceName (uint instanceNb) const;
	const NLMISC::CVector& getInstancePos (uint instanceNb) const;
	const NLMISC::CQuat& getInstanceRot (uint instanceNb) const;
	const NLMISC::CVector& getInstanceScale (uint instanceNb) const;

	// The real instance group
	CInstanceGroup	_InstanceGroup;
};


} // NL3D


#endif // NL_INSTANCE_GROUP_USER_H

/* End of instance_group_user.h */
