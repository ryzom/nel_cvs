/** \file vertex_stream_manager.cpp
 * <File description>
 *
 * $Id: vertex_stream_manager.cpp,v 1.3.4.1 2004/09/14 17:12:27 vizerie Exp $
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
	_SupportVolatileVB = false;
	_VertexFormat= 0;
	_VertexSize= 0;
	_MaxVertices= 0;
	_CurentVB= 0;
	_NumVB= 0;
	_LockDone = false;
}
// ***************************************************************************
CVertexStreamManager::~CVertexStreamManager()
{
	release();
}

// ***************************************************************************
void			CVertexStreamManager::init(IDriver *driver, uint vertexFormat, uint maxVertices, uint numVBHard, const std::string &vbName, bool allowVolatileVertexBuffer /*= false*/)
{
	nlassert(driver);
	// clean before.
	release();

	// Create VBHard placeholder
	if(numVBHard==0 || maxVertices==0)
		return;

	_NumVB= numVBHard;
	_VB.resize(_NumVB);

	// setup, => correct for possible release below
	_Driver= driver;

	// create the VBHard, if possible
	uint i;
	for(i=0;i<_NumVB;i++)
	{
		_VB[i].setVertexFormat(vertexFormat);

		// For the moment, all UV channel are routed to UV0
		uint j;
		for (j=0; j<CVertexBuffer::MaxStage; j++)
			_VB[i].setUVRouting (j, 0);

		_VB[i].setNumVertices (maxVertices);
		_VB[i].setPreferredMemory (CVertexBuffer::AGPPreferred, false);
		_VB[i].setName(vbName + NLMISC::toString(i));
	}

	// init misc
	_InitOk= true;
	_VertexFormat= _VB[0].getVertexFormat();
	_VertexSize= _VB[0].getVertexSize();
	_MaxVertices= maxVertices;
	_CurentVB= 0;
	if (driver->supportVolatileVertexBuffer() && allowVolatileVertexBuffer)
	{
		_VBVolatile.setVertexFormat(vertexFormat);
		// For the moment, all UV channel are routed to UV0
		uint j;
		for (j=0; j<CVertexBuffer::MaxStage; j++)
			_VBVolatile.setUVRouting (j, 0);
		_VBVolatile.setNumVertices (maxVertices);
		_VBVolatile.setPreferredMemory (CVertexBuffer::AGPVolatile, false);
		_VBVolatile.setName(vbName + "Volatile");
		_SupportVolatileVB = true;
	}
	else
	{
		_SupportVolatileVB = false;
	}
}
// ***************************************************************************
void			CVertexStreamManager::release()
{
	// release driver/VBHard
	if(_Driver)
		_Driver= NULL;

	_VB.clear();

	// misc
	_InitOk= false;
	_VertexFormat= 0;
	_VertexSize= 0;
	_MaxVertices= 0;
	_CurentVB= 0;
	_NumVB= 0;
}
// ***************************************************************************
uint8			*CVertexStreamManager::lock()
{
	H_AUTO( NL3D_VertexStreamManager_lock )
	nlassert(_InitOk);
	nlassert(!_LockDone);
	_LockDone = true;	
	if (_SupportVolatileVB)
	{	
		_VBVolatile.lock (_VBA);
		return (uint8*)_VBA.getVertexCoordPointer();
	}
	else
	{
		_VB[_CurentVB].lock (_VBA);
		return 	(uint8*)_VBA.getVertexCoordPointer();		
	}
}
// ***************************************************************************
void			CVertexStreamManager::unlock(uint numVertices)
{
	H_AUTO( NL3D_VertexStreamManager_unlock )
	nlassert(_InitOk);
	nlassert(_LockDone);	
	_VBA.touchVertices (0, numVertices);
	_VBA.unlock ();
}
// ***************************************************************************
void			CVertexStreamManager::activate()
{
	H_AUTO( NL3D_VertexStreamManager_activate )
	nlassert(_InitOk);
	if (_SupportVolatileVB)
	{
		_Driver->activeVertexBuffer(_VBVolatile);
	}
	else
	{	
		_Driver->activeVertexBuffer(_VB[_CurentVB]);
	}
}
// ***************************************************************************
void			CVertexStreamManager::swapVBHard()
{
	nlassert(_InitOk);
	_LockDone = false;
	if (!_SupportVolatileVB)
	{	
	_CurentVB++;
	_CurentVB= _CurentVB%_NumVB;
}
}


} // NL3D
