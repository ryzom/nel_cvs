/** \file driver_direct3d_index.cpp
 * Direct 3d driver implementation
 *
 * $Id: driver_direct3d_index.cpp,v 1.2 2004/03/23 16:32:27 corvazier Exp $
 *
 * \todo manage better the init/release system (if a throw occurs in the init, we must release correctly the driver)
 */

/* Copyright, 2000 Nevrax Ltd.
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

#include "stddirect3d.h"

#include "3d/index_buffer.h"
#include "3d/light.h"
#include "nel/misc/rect.h"
#include "nel/misc/di_event_emitter.h"
#include "nel/misc/mouse_device.h"
#include "nel/3d/viewport.h"
#include "nel/3d/scissor.h"
#include "nel/3d/u_driver.h"

#include "driver_direct3d.h"

using namespace std;
using namespace NLMISC;

namespace NL3D 
{

// ***************************************************************************

CIBDrvInfosD3D::CIBDrvInfosD3D(IDriver *drv, ItIBDrvInfoPtrList it, CIndexBuffer *ib) : IIBDrvInfos(drv, it, ib)
{
	IndexBuffer = NULL;
}

// ***************************************************************************

extern uint indexCount=0;

CIBDrvInfosD3D::~CIBDrvInfosD3D()
{
	if (IndexBuffer)
	{
		indexCount--;
		IndexBuffer->Release();
	}
}

// ***************************************************************************

uint32 *CIBDrvInfosD3D::lock (uint first, uint last, bool readOnly)
{
	nlassert (IndexBuffer);
	
	void *pbData;
	if (IndexBuffer->Lock ( first*sizeof(uint32), (last-first)*sizeof(uint32), &pbData, readOnly?D3DLOCK_READONLY:0) == D3D_OK)
		return (uint32*)pbData;
	return NULL;
}

// ***************************************************************************

void	CIBDrvInfosD3D::unlock (uint first, uint last)
{
	IndexBuffer->Unlock ();
}

// ***************************************************************************

DWORD RemapIndexBufferUsage[CIndexBuffer::PreferredCount]=
{
	D3DUSAGE_DYNAMIC,						// RAMPreferred
	D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY,	// AGPPreferred
	D3DUSAGE_WRITEONLY,						// VRAMPreferred
};

// ***************************************************************************

D3DPOOL RemapIndexBufferPool[CIndexBuffer::PreferredCount]=
{
	D3DPOOL_SYSTEMMEM,	// RAMPreferred
	D3DPOOL_DEFAULT,	// AGPPreferred
	D3DPOOL_DEFAULT,	// VRAMPreferred
};

// ***************************************************************************

bool CDriverD3D::activeIndexBuffer(CIndexBuffer& IB)
{
	// Must not be locked
	nlassert (!IB.isLocked());

	// Must not be empty
	if (IB.capacity() == 0)
		return false;

	const bool touched = (IB.getTouchFlags() & (CIndexBuffer::TouchedReserve|CIndexBuffer::TouchedIndexFormat)) != 0;
	if ((_IndexBuffer.IndexBuffer != &IB) || touched)
	{
		// Build the driver info
		if (touched)
		{
			// Delete previous index buffer info
			if (IB.DrvInfos)
			{
				delete IB.DrvInfos;
				nlassert (IB.DrvInfos == NULL);
			}

			// Rebuild it
			_IBDrvInfos.push_front (NULL);
			ItIBDrvInfoPtrList ite = _IBDrvInfos.begin();
			CIBDrvInfosD3D *info = new CIBDrvInfosD3D(this, ite, &IB);
			*ite = info;

			// Create the index buffer
			const uint size = (uint)IB.capacity();
			uint preferredMemory = _DisableHardwareIndexArrayAGP?CIndexBuffer::RAMPreferred:IB.getPreferredMemory ();
			bool sucess;
			do
			{
				if (sucess =(_DeviceInterface->CreateIndexBuffer(size*sizeof(uint32), 
					RemapIndexBufferUsage[preferredMemory],
					D3DFMT_INDEX32, RemapIndexBufferPool[preferredMemory], &(info->IndexBuffer), NULL) == D3D_OK))
					break;
			}
			while (preferredMemory--);
			if (!sucess)
				return false;

			indexCount++;

			// Release the local index buffer
			IB.DrvInfos = info;
			IB.setLocation((CIndexBuffer::TLocation)preferredMemory);
		}

		// Set the current index buffer
		nlassert (IB.DrvInfos);
		_IndexBuffer.IndexBuffer = &IB;
		touchRenderVariable (&_IndexBuffer);
	}

	return true;
}

// ***************************************************************************

bool CDriverD3D::supportIndexBufferHard() const 
{
	return !_DisableHardwareIndexArrayAGP;
}

// ***************************************************************************

void CDriverD3D::disableHardwareIndexArrayAGP()
{
	_DisableHardwareIndexArrayAGP = true;
}

// ***************************************************************************

void CDriverD3D::restaureIndexBuffer (CIBDrvInfosD3D &indexBuffer)
{
	/* Restaure a resident index buffer to system memory
	 * 
	 * We use a tricky code here. We read "writeonly" index buffer data.
	 * We lock the whole buffer and read with the pointer returned.
	 */
	/* Hulud test : read in a "write only" index buffer. Seams to work well. */

	CIndexBuffer *ib = indexBuffer.IndexBufferPtr;
	const uint size = ib->capacity()*sizeof(uint32);
	
	// No special flag for the lock, the driver should return a valid index buffer pointer with previous values.
	uint32 *out = indexBuffer.lock (0, 0, false);
	nlassert (out);
	{
		if (ib->getLocation () == CIndexBuffer::RAMResident)
		{
			// Realloc local memory
			ib->setLocation (CIndexBuffer::NotResident);	// If resident in RAM, content backuped by setLocation
		}
		else
		{
			// Realloc local memory
			ib->setLocation (CIndexBuffer::NotResident);

			// Lock the index buffer
			CIndexBufferReadWrite iba;
			ib->lock (iba);

			// Copy the index data
			memcpy (iba.getPtr(), out, size);
		}
	}
	indexBuffer.unlock (0, 0);
}

// ***************************************************************************

} // NL3D
