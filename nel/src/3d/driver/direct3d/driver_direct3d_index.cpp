/** \file driver_direct3d_index.cpp
 * Direct 3d driver implementation
 *
 * $Id: driver_direct3d_index.cpp,v 1.4 2004/04/26 13:48:23 corvazier Exp $
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
	nlassert (first != last);

	if (Volatile)
	{
		CDriverD3D *driver = static_cast<CDriverD3D*>(_Driver);
		// Lock the good buffer
	/* todo hulud volatile
		CVolatileIndexBuffer *buffer = VolatileRAM ? &(driver->_VolatileIndexBufferRAM[driver->_CurrentRenderPass&1]):
			&(driver->_VolatileIndexBufferAGP[driver->_CurrentRenderPass&1]);
			*/
		CVolatileIndexBuffer *buffer = VolatileRAM ? &(driver->_VolatileIndexBufferRAM[0]):
			&(driver->_VolatileIndexBufferAGP[0]);
		uint32 *ptr = (uint32*)buffer->lock ((last-first)*sizeof(uint32), Offset);
		IndexBuffer = buffer->IndexBuffer;
		ptr -= first;

		// Current lock time
		VolatileLockTime = driver->_CurrentRenderPass;

		return ptr;
	}
	else
	{
		nlassert (IndexBuffer);
		
		void *pbData;
		if (IndexBuffer->Lock ( first*sizeof(uint32), (last-first)*sizeof(uint32), &pbData, readOnly?D3DLOCK_READONLY:0) == D3D_OK)
			return (uint32*)pbData;
	}
	return NULL;
}

// ***************************************************************************

void	CIBDrvInfosD3D::unlock (uint first, uint last)
{
	if (Volatile)
	{
		CDriverD3D *driver = static_cast<CDriverD3D*>(_Driver);
		// Unlock the good buffer
		/* todo hulud volatile
		CVolatileIndexBuffer *buffer = VolatileRAM ? &(driver->_VolatileIndexBufferRAM[driver->_CurrentRenderPass&1]):
			&(driver->_VolatileIndexBufferAGP[driver->_CurrentRenderPass&1]);*/
		CVolatileIndexBuffer *buffer = VolatileRAM ? &(driver->_VolatileIndexBufferRAM[0]):
			&(driver->_VolatileIndexBufferAGP[0]);
		buffer->unlock ();
	}
	else
		IndexBuffer->Unlock ();
}

// ***************************************************************************

DWORD RemapIndexBufferUsage[CIndexBuffer::LocationCount]=
{
	D3DUSAGE_DYNAMIC,						// RAMResident
	D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY,	// AGPResident
	D3DUSAGE_WRITEONLY,						// VRAMResident
	0,										// Not used
};

// ***************************************************************************

D3DPOOL RemapIndexBufferPool[CIndexBuffer::LocationCount]=
{
	D3DPOOL_SYSTEMMEM,	// RAMResident
	D3DPOOL_DEFAULT,	// AGPResident
	D3DPOOL_DEFAULT,	// VRAMResident
	D3DPOOL_DEFAULT,	// Not used
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
	CIBDrvInfosD3D *info = static_cast<CIBDrvInfosD3D*>(static_cast<IIBDrvInfos*>(IB.DrvInfos));

	// Volatile buffers must be filled at each pass
	nlassertex (!info || !info->Volatile || IB.getKeepLocalMemory() || (info->VolatileLockTime == _CurrentRenderPass), ("Volatile buffers must be filled at each pass"));

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
		info = new CIBDrvInfosD3D(this, ite, &IB);
		*ite = info;

		// Create the index buffer
		const uint size = (uint)IB.capacity();
		uint preferredMemory;
		if (_DisableHardwareIndexArrayAGP)
			preferredMemory = CIndexBuffer::RAMResident;
		else
		{
			switch (IB.getPreferredMemory ())
			{
			case CIndexBuffer::RAMPreferred:
				preferredMemory = CIndexBuffer::RAMResident;
				info->Volatile = false;
				break;
			case CIndexBuffer::AGPPreferred:
				preferredMemory = CIndexBuffer::AGPResident;
				info->Volatile = false;
				break;
			case CIndexBuffer::StaticPreferred:
				if (getStaticMemoryToVRAM())
					preferredMemory = CIndexBuffer::VRAMResident;
				else
					preferredMemory = CIndexBuffer::AGPResident;
				info->Volatile = false;
				break;
			case CIndexBuffer::RAMVolatile:
				preferredMemory = CIndexBuffer::RAMResident;
				info->Volatile = true;
				break;
			case CIndexBuffer::AGPVolatile:
				preferredMemory = CIndexBuffer::AGPResident;
				info->Volatile = true;
				break;
			}
		}

		// Volatile index buffer
		if (info->Volatile)
		{
			nlassert (info->IndexBuffer == NULL);
			info->VolatileRAM = preferredMemory == CIndexBuffer::RAMResident;
		}
		else
		{
			// Offset will be 0
			info->Offset = 0;

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
		}

		indexCount++;

		// Release the local index buffer
		IB.DrvInfos = info;
		IB.setLocation((CIndexBuffer::TLocation)preferredMemory);

		// Force the vertex buffer update
		touchRenderVariable (&_IndexBufferCache);
	}

	// Set the current index buffer
	nlassert (info);

	// Fill the buffer if in local memory
	IB.fillBuffer ();

	// Set the vertex buffer
	setIndexBuffer (info->IndexBuffer, info->Offset);

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
// CVolatileIndexBuffer
// ***************************************************************************

CVolatileIndexBuffer::CVolatileIndexBuffer()
{
	IndexBuffer = NULL;
}

// ***************************************************************************

CVolatileIndexBuffer::~CVolatileIndexBuffer()
{
	release ();
}

// ***************************************************************************

void CVolatileIndexBuffer::release ()
{
	if (IndexBuffer)
		IndexBuffer->Release();
	IndexBuffer = NULL;
}

// ***************************************************************************

void CVolatileIndexBuffer::init (CIndexBuffer::TLocation	location, uint size, CDriverD3D *driver)
{
	release();

	// Init the buffer
	Location = location;
	Size = size;
	Driver = driver;

	// Allocate the vertex buffer
	if (Driver->_DeviceInterface->CreateIndexBuffer(size, RemapIndexBufferUsage[location],
		D3DFMT_INDEX32, RemapIndexBufferPool[location], &IndexBuffer, NULL) != D3D_OK)
	{
		// Location in RAM must not failed
		nlassert (location != CIndexBuffer::RAMResident);

		// Allocate in RAM
		nlverify (Driver->_DeviceInterface->CreateIndexBuffer(size, RemapIndexBufferUsage[CIndexBuffer::RAMResident],
				D3DFMT_INDEX32, RemapIndexBufferPool[CIndexBuffer::RAMResident], &IndexBuffer, NULL) != D3D_OK);
	}
}

// ***************************************************************************

void *CVolatileIndexBuffer::lock (uint size, uint &offset)
{
	/* If not enough room to allocate this buffer, resise the buffer to Size+Size/2 but do not reset CurrentIndex
	 * to be sure the buffer will be large enough next pass. */

	// Enough room for this vertex ?
	if (CurrentIndex+size > Size)
	{
		// No, reallocate
		init (Location, std::max (Size+Size/2,  CurrentIndex+size), Driver);
	}

	// Lock the buffer, noblocking lock here if not the first allocation since a reset
	VOID *pbData;
	nlverify (IndexBuffer->Lock (CurrentIndex, size, &pbData, (CurrentIndex!=0)?D3DLOCK_NOOVERWRITE:0) == D3D_OK);

	// Old buffer position
	offset = CurrentIndex;

	// New buffer position
	CurrentIndex += size;
	return pbData;
}

// ***************************************************************************

void CVolatileIndexBuffer::unlock ()
{
	nlverify (IndexBuffer->Unlock () == D3D_OK);
}

// ***************************************************************************

void CVolatileIndexBuffer::reset ()
{
	CurrentIndex = 0;
}

// ***************************************************************************

} // NL3D
