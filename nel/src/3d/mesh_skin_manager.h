/** \file mesh_skin_manager.h
 * <File description>
 *
 * $Id: mesh_skin_manager.h,v 1.3 2003/08/07 08:49:13 berenguier Exp $
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

#ifndef NL_MESH_SKIN_MANAGER_H
#define NL_MESH_SKIN_MANAGER_H

#include "nel/misc/types_nl.h"
#include "3d/driver.h"


namespace NL3D
{


// ***************************************************************************
/**
 * A class used to render Skins in a VBHard, grouped by their skeleton
 *	This allow optimisation because less VBuffer swap is needed
 *	Also, only 2 Big VBHard is created for all the skins, which improves use of AGP space
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CMeshSkinManager
{
public:

	/// Constructor
	CMeshSkinManager();
	~CMeshSkinManager();

	/// \name Init/Setup
	// @{

	/** init the manager with a driver, allocate the VBHards, and setup the vertexFormat. 
	 *	Must call each time the drive changes
	 *	\param numVBHard the number of VBHard to create. The max you set, the lower lock you'll have.
	 *	\param vbName base, for Lock profiling
	 */
	void			init(IDriver *driver, uint vertexFormat, uint maxVertices, uint numVBHard=2, const std::string &vbName="MeshSkinVB");
	/// release the VBHard. init() can be called after this.
	void			release();

	/// false if error at init, eg if driver do no support VBHard, or if no more AGP memory
	bool			enabled() const {return _Enabled;}
	/// return the driver used.
	IDriver			*getDriver() const {return _Driver;}
	/// get the vertexFormat
	uint			getVertexFormat() const {return _VertexFormat;}
	/// get the vertexSize
	uint			getVertexSize() const {return _VertexSize;}
	/// get max vertices the Buffer allows.
	uint			getMaxVertices() const {return _MaxVertices;}

	// @}

	/// \name Rendering. Those methods must be called only if enabled(), else crash
	// @{

	/// lock the currently activated VBHard, for future filling
	uint8			*lock();
	/// unlock the currently activated VBHard. Tell how many vertices have changed.
	void			unlock(uint numVertices);

	/// activate the currentVBhard as the current VB in the driver, for future rendering
	void			activate();

	/// Swap to the next VBHard. This allow some parralelism, since CPU fill one VBHard while the other is rendered
	void			swapVBHard();

	// @}

// ********************
private:
	uint			_NumVBHard;

	NLMISC::CRefPtr<IDriver>			_Driver;
	std::vector<NLMISC::CRefPtr<IVertexBufferHard> >	_VBHard;
	bool			_Enabled;
	uint			_VertexFormat;
	uint			_VertexSize;
	uint			_MaxVertices;

	uint			_CurentVBHard;

};


} // NL3D


#endif // NL_MESH_SKIN_MANAGER_H

/* End of mesh_skin_manager.h */
