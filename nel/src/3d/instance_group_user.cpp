/** \file instance_group_user.cpp
 * Implementation of the user interface managing instance groups.
 *
 * $Id: instance_group_user.cpp,v 1.1 2001/04/09 14:26:49 corvazier Exp $
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

#include "nel/3d/instance_group_user.h"
#include "nel/misc/file.h"

using namespace NLMISC;

namespace NL3D 
{

// ***************************************************************************

bool CInstanceGroupUser::init (const std::string &instanceGroup, CScene& scene)
{
	// Create a file
	CIFile file;
	if (file.open (instanceGroup))
	{
		// Serialize this class
		try
		{
			// Read the class
			_InstanceGroup.serial (file);

			// Add to the scene
			_InstanceGroup.addToScene (scene);
		}
		catch (EStream& e)
		{
			// Avoid visual warning
			EStream ee=e;

			// Serial problem
			return false;
		}
	}
	else
	{
		// Failed.
		return false;
	}

	// Ok
	return true;
}

// ***************************************************************************

void CInstanceGroupUser::release (CScene& scene)
{
	// Remove from the scene
	_InstanceGroup.removeFromScene (scene);
}

// ***************************************************************************

uint CInstanceGroupUser::getNumInstance () const
{
	return _InstanceGroup.getNumInstance ();
}

// ***************************************************************************

const std::string& CInstanceGroupUser::getInstanceName (uint instanceNb) const
{
	// Check args
	if (instanceNb>=_InstanceGroup.getNumInstance ())
		nlerror("getInstanceName*(): bad instance Id");
	
	return _InstanceGroup.getInstanceName (instanceNb);
}

// ***************************************************************************

const NLMISC::CVector& CInstanceGroupUser::getInstancePos (uint instanceNb) const
{
	// Check args
	if (instanceNb>=_InstanceGroup.getNumInstance ())
		nlerror("getInstancePos*(): bad instance Id");
	
	return _InstanceGroup.getInstancePos (instanceNb);
}

// ***************************************************************************

const NLMISC::CQuat& CInstanceGroupUser::getInstanceRot (uint instanceNb) const
{
	// Check args
	if (instanceNb>=_InstanceGroup.getNumInstance ())
		nlerror("getInstanceRot*(): bad instance Id");
	
	return _InstanceGroup.getInstanceRot (instanceNb);
}

// ***************************************************************************

const NLMISC::CVector& CInstanceGroupUser::getInstanceScale (uint instanceNb) const
{
	// Check args
	if (instanceNb>=_InstanceGroup.getNumInstance ())
		nlerror("getInstanceScale*(): bad instance Id");
	
	return _InstanceGroup.getInstanceScale (instanceNb);
}

// ***************************************************************************

} // NL3D
