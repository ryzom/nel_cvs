/** \file mesh_skin_manager.cpp
 * <File description>
 *
 * $Id: mesh_skin_manager.cpp,v 1.2 2002/08/30 11:59:42 berenguier Exp $
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

#include "std3d.h"

#include "3d/mesh_skin_manager.h"


namespace NL3D 
{


// ***************************************************************************
CMeshSkinManager::CMeshSkinManager()
{
	_Enabled= false;
	_VertexFormat= 0;
	_VertexSize= 0;
	_MaxVertices= 0;
	_CurentVBHard= 0;
}
// ***************************************************************************
CMeshSkinManager::~CMeshSkinManager()
{
	release();
}

// ***************************************************************************
void			CMeshSkinManager::init(IDriver *driver, uint vertexFormat, uint maxVertices)
{
	nlassert(driver);
	// clean before.
	release();

	// setup, => correct for possible release below
	_Driver= driver;

	// a dummy VB, for easy setup
	CVertexBuffer	vb;
	vb.setVertexFormat(vertexFormat);

	// create the VBHard, if possible
	for(uint i=0;i<NumVBHard;i++)
	{
		_VBHard[i]= _Driver->createVertexBufferHard(vb.getVertexFormat(), vb.getValueTypePointer(), maxVertices, IDriver::VBHardAGP);
		// if filas, release all, and quit
		if(_VBHard[i]==NULL)
		{
			release();
			return;
		}
	}

	// init misc
	_Enabled= true;
	_VertexFormat= vb.getVertexFormat();
	_VertexSize= vb.getVertexSize();
	_MaxVertices= maxVertices;
	_CurentVBHard= 0;
}
// ***************************************************************************
void			CMeshSkinManager::release()
{
	// release driver/VBHard
	if(_Driver)
	{
		for(uint i=0;i<NumVBHard;i++)
		{
			if(_VBHard[i])
				_Driver->deleteVertexBufferHard(_VBHard[i]);
			_VBHard[i]= NULL;
		}
		_Driver= NULL;
	}

	// misc
	_Enabled= false;
	_VertexFormat= 0;
	_VertexSize= 0;
	_MaxVertices= 0;
	_CurentVBHard= 0;
}
// ***************************************************************************
uint8			*CMeshSkinManager::lock()
{
	return	(uint8*)_VBHard[_CurentVBHard]->lock();
}
// ***************************************************************************
void			CMeshSkinManager::unlock(uint numVertices)
{
	// ATI: release only vertices used.
	_VBHard[_CurentVBHard]->unlock(0, numVertices);
}
// ***************************************************************************
void			CMeshSkinManager::activate()
{
	_Driver->activeVertexBufferHard(_VBHard[_CurentVBHard]);
}
// ***************************************************************************
void			CMeshSkinManager::swapVBHard()
{
	_CurentVBHard++;
	_CurentVBHard= _CurentVBHard%NumVBHard;
}


} // NL3D
