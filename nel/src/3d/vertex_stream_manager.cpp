/** \file vertex_stream_manager.cpp
 * <File description>
 *
 * $Id: vertex_stream_manager.cpp,v 1.1 2003/11/26 13:44:17 berenguier Exp $
 */

/* Copyright, 2000-2003 Nevrax Ltd.
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

#include "3d/vertex_stream_manager.h"
#include "nel/misc/hierarchical_timer.h"


using namespace	NLMISC;

namespace NL3D 
{


// ***************************************************************************
CVertexStreamManager::CVertexStreamManager()
{
	_InitOk= false;
	_VBHardMode= false;
	_VertexFormat= 0;
	_VertexSize= 0;
	_MaxVertices= 0;
	_CurentVBHard= 0;
	_NumVBHard= 0;
}
// ***************************************************************************
CVertexStreamManager::~CVertexStreamManager()
{
	release();
}

// ***************************************************************************
void			CVertexStreamManager::init(IDriver *driver, uint vertexFormat, uint maxVertices, uint numVBHard, const std::string &vbName)
{
	nlassert(driver);
	// clean before.
	release();

	// Create VBHard placeholder
	if(numVBHard==0 || maxVertices==0)
		return;
	_NumVBHard= numVBHard;
	_VBHard.resize(_NumVBHard, NULL);

	// setup, => correct for possible release below
	_Driver= driver;

	// setup the VB soft, for easy setup
	_VBSoft.setVertexFormat(vertexFormat);

	// For the moment, all UV channel are routed to UV0
	uint i;
	for (i=0; i<CVertexBuffer::MaxStage; i++)
		_VBSoft.setUVRouting (i, 0);

	// create the VBHard, if possible
	_VBHardMode= true;
	for(i=0;i<_NumVBHard;i++)
	{
		_VBHard[i]= _Driver->createVertexBufferHard(_VBSoft.getVertexFormat(), _VBSoft.getValueTypePointer(), maxVertices, IDriver::VBHardAGP, _VBSoft.getUVRouting());
		// if filas, release all, and quit
		if(_VBHard[i]==NULL)
		{
			_VBHardMode= false;
			break;
		}
		// ok, set name for lock profiling
		else
		{
			_VBHard[i]->setName(vbName + NLMISC::toString(i));
		}
	}

	// if fails to create vbHard, abort, and create only one vbSoft
	if(!_VBHardMode)
	{
		// release all vbhard created
		for(uint i=0;i<_NumVBHard;i++)
		{
			if(_VBHard[i])
				_Driver->deleteVertexBufferHard(_VBHard[i]);
		}
		_VBHard.clear();

		// create the Soft One
		_VBSoft.setNumVertices(maxVertices);
	}

	// init misc
	_InitOk= true;
	_VertexFormat= _VBSoft.getVertexFormat();
	_VertexSize= _VBSoft.getVertexSize();
	_MaxVertices= maxVertices;
	_CurentVBHard= 0;

	// release the VBsoft if in vbHardMode (no more used)
	if(_VBHardMode)
	{
		contReset(_VBSoft);
	}
}
// ***************************************************************************
void			CVertexStreamManager::release()
{
	// release driver/VBHard
	if(_Driver)
	{
		for(uint i=0;i<_NumVBHard;i++)
		{
			if(_VBHard[i])
				_Driver->deleteVertexBufferHard(_VBHard[i]);
			_VBHard[i]= NULL;
		}
		_Driver= NULL;
	}

	_VBHard.clear();

	// release VBSoft
	contReset(_VBSoft);

	// misc
	_InitOk= false;
	_VBHardMode= false;
	_VertexFormat= 0;
	_VertexSize= 0;
	_MaxVertices= 0;
	_CurentVBHard= 0;
	_NumVBHard= 0;
}
// ***************************************************************************
uint8			*CVertexStreamManager::lock()
{
	H_AUTO( NL3D_VertexStreamManager_lock )
	nlassert(_InitOk);

	if(_VBHardMode)
		return	(uint8*)_VBHard[_CurentVBHard]->lock();
	else
		return 	(uint8*)_VBSoft.getVertexCoordPointer();
}
// ***************************************************************************
void			CVertexStreamManager::unlock(uint numVertices)
{
	H_AUTO( NL3D_VertexStreamManager_unlock )
	nlassert(_InitOk);
	
	if(_VBHardMode)
	{
		// ATI: release only vertices used.
		_VBHard[_CurentVBHard]->unlock(0, numVertices);
	}
}
// ***************************************************************************
void			CVertexStreamManager::activate()
{
	H_AUTO( NL3D_VertexStreamManager_activate )
	nlassert(_InitOk);
	
	if(_VBHardMode)
		_Driver->activeVertexBufferHard(_VBHard[_CurentVBHard]);
	else
		_Driver->activeVertexBuffer(_VBSoft);
}
// ***************************************************************************
void			CVertexStreamManager::swapVBHard()
{
	nlassert(_InitOk);
	
	if(_VBHardMode)
	{
		_CurentVBHard++;
		_CurentVBHard= _CurentVBHard%_NumVBHard;
	}
}


} // NL3D
