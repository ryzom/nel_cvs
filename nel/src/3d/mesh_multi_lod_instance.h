/** \file mesh_multi_lod_instance.h
 * An instance of CMeshMulitLod
 *
 * $Id: mesh_multi_lod_instance.h,v 1.3 2001/07/06 12:51:23 corvazier Exp $
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

#ifndef NL_MESH_MULTI_LOD_INSTANCE_H
#define NL_MESH_MULTI_LOD_INSTANCE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/class_id.h"

#include "3d/mesh_instance.h"


namespace NL3D 
{

// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		MeshMultiLodInstanceId=NLMISC::CClassId(0x1ade6ef8, 0x75c5a84);


// ***************************************************************************
/**
 * An instance of CMeshMulitLod
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CMeshMultiLodInstance : public CMeshInstance
{
public:
	/// Call at the begining of the program, to register the model, and the basic observers.
	static	void	registerBasic();

	/// Pointer on the scene.
	CScene			*Scene;

	/// Last Matrix date
	uint64			_LastLodMatrixDate;

private:
	static IModel	*creator() {return new CMeshMultiLodInstance;}
	friend	class CMeshMultiLod;
};


} // NL3D


#endif // NL_MESH_MULTI_LOD_INSTANCE_H

/* End of mesh_multi_lod_instance.h */
