/** \file mesh_vertex_program.h
 * <File description>
 *
 * $Id: mesh_vertex_program.h,v 1.1 2002/02/26 14:17:55 berenguier Exp $
 */

/* Copyright, 2000-2002 Nevrax Ltd.
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

#ifndef NL_MESH_VERTEX_PROGRAM_H
#define NL_MESH_VERTEX_PROGRAM_H

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include "nel/misc/smart_ptr.h"


namespace NL3D 
{


class	IDriver;
class	CScene;
class	CMeshBaseInstance;


/**
 * Interface to a CMeshGeom/CMeshMRMGeom Special VertexProgram Effect.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class IMeshVertexProgram : public NLMISC::IStreamable, public NLMISC::CRefCount
{
public:

	/// Constructor
	IMeshVertexProgram() {}
	virtual ~IMeshVertexProgram() {}

	/** Called at creation of an instance, to setup some information directly in the CMeshBaseInstance
	 */
	virtual	void	initInstance(CMeshBaseInstance *mbi) =0;


	/** Called to setup constant / activate VertexProgram. (called before activate of the VB)
	 *	\param drv driver where to setup VP.
	 *	\param scene retrieve some useFull scene info (windPower, time ...)
	 *	\param mbi the mesh instance to retrieve some instance setup
	 */
	virtual	void	begin(IDriver *drv, CScene *scene, CMeshBaseInstance *mbi) =0;
	/** Typically disable the VertexProgram, or do some uninit.
	 */
	virtual	void	end(IDriver *drv) =0;

	/** true if support VertexProgram Scene Lighting scheme.
	 *	NB: it is user of IMeshVertexProgram which call 
	 *	CRenderTrav::beginVPLightSetup() and CRenderTrav::changeVPLightSetupMaterial().
	 */
	virtual bool	useSceneVPLightSetup(bool &supportSpecular, uint &lightCteStart) const =0;
};


} // NL3D


#endif // NL_MESH_VERTEX_PROGRAM_H

/* End of mesh_vertex_program.h */
