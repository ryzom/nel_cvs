/** \file mesh_mrm_instance.h
 * <File description>
 *
 * $Id: mesh_mrm_instance.h,v 1.7 2002/06/10 14:02:47 berenguier Exp $
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

#ifndef NL_MESH_MRM_INSTANCE_H
#define NL_MESH_MRM_INSTANCE_H

#include "nel/misc/types_nl.h"
#include "3d/mesh_base_instance.h"
#include "3d/material.h"
#include "3d/animated_material.h"


namespace NL3D
{


class CMeshMRM;


// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		MeshMRMInstanceId=NLMISC::CClassId(0xec608f3, 0x1111c33);


// ***************************************************************************
/**
 * An instance of CMeshMRM.
 * NB: no observers are needed, since same functionnality as CMeshBaseInstance.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CMeshMRMInstance : public CMeshBaseInstance
{
public:
	/// Call at the begining of the program, to register the model, and the basic observers.
	static	void	registerBasic();

protected:
	/// Constructor
	CMeshMRMInstance()
	{
	}
	/// Destructor
	virtual ~CMeshMRMInstance();


	/// \name Skinning Behavior.
	// @{
	/// I can be skinned if the mesh is.
	virtual	bool	isSkinnable() const;

	/// Called when the skin is applied on the skeleton
	virtual	void	setApplySkin(bool state);

	/// Called for lod character coloring.
	virtual const std::vector<sint32>	*getSkinBoneUsage() const;
	// @}


	/** Change MRM Distance setup. See CMeshBaseInstance::changeMRMDistanceSetup()
	 */
	virtual void		changeMRMDistanceSetup(float distanceFinest, float distanceMiddle, float distanceCoarsest);


private:
	static IModel	*creator() {return new CMeshMRMInstance;}
	friend	class CMeshMRM;

};



} // NL3D


#endif // NL_MESH_MRM_INSTANCE_H

/* End of mesh_mrm_instance.h */
