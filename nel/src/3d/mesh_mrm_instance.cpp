/** \file mesh_mrm_instance.cpp
 * <File description>
 *
 * $Id: mesh_mrm_instance.cpp,v 1.3 2002/03/06 10:24:47 corvazier Exp $
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

#include "std3d.h"

#include "3d/mesh_mrm_instance.h"
#include "3d/mesh_mrm.h"


namespace NL3D 
{


// ***************************************************************************
void		CMeshMRMInstance::registerBasic()
{
	CMOT::registerModel(MeshMRMInstanceId, MeshBaseInstanceId, CMeshMRMInstance::creator);
}

// ***************************************************************************
void		CMeshMRMInstance::setApplySkin(bool state)
{
	// Call parents method
	CMeshBaseInstance::setApplySkin (state);

	// Recompute the id
	if (state)
	{
		// Get a pointer on the shape
		CMeshMRM *pMesh = NLMISC::safe_cast<CMeshMRM *>((IShape*)Shape);

		pMesh->computeBonesId (_FatherSkeletonModel);
	}
}


} // NL3D
