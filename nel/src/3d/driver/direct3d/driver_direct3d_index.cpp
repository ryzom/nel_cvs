/** \file driver_direct3d_index.cpp
 * Direct 3d driver implementation
 *
 * $Id: driver_direct3d_index.cpp,v 1.8.4.1 2004/09/14 15:33:43 vizerie Exp $
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

CIBDrvInfosD3D::CIBDrvInfosD3D(CDriverD3D *drv, ItIBDrvInfoPtrList it, CIndexBuffer *ib) : IIBDrvInfos(drv, it, ib)
{
	H_AUTO_D3D(CIBDrvInfosD3D_CIBDrvInfosD3D)
	Driver = drv;
	IndexBuffer = NULL;
	VolatileIndexBuffer = NULL;
}

// ***************************************************************************

extern uint indexCount=0;

CIBDrvInfosD3D::~CIBDrvInfosD3D()
{
	H_AUTO_D3D(CIBDrvInfosD3D_CIBDrvInfosD3DDtor);
	CDriverD3D *driver = static_cast<CDriverD3D*>(_Driver);
	// Restaure non resident memory
	if (IndexBufferPtr)
	{
		IndexBufferPtr->setLocation(CIndexBuffer::NotResident);
		IndexBufferPtr= NULL;
	}
		
	// release index buffer
	if (IndexBuffer && !Volatile)
	{
		if (Driver)
		{
			if (Driver->_IndexBufferCache.IndexBuffer == IndexBuffer)
			{
				Driver->_IndexBufferCache.IndexBuffer = NULL;
				Driver->touchRenderVariable(&Driver->_IndexBufferCache);
			}
		}
		indexCount--;
		IndexBuffer->Release();
	}
}

// ***************************************************************************

uint32 *CIBDrvInfosD3D::lock (uint first, uint last, bool readOnly)
{
	H_AUTO_D3D(CIBDrvInfosD3D_lock);
	nlassert (first != last);
	CDriverD3D *driver = static_cast<CDriverD3D*>(_Driver);
	
	if (Volatile)
	{
		// Lock the good buffer
		CVolatileIndexBuffer *&buffer = VolatileRAM ? (driver->_VolatileIndexBufferRAM[driver->_CurrentRenderPass&1]):
			(driver->_VolatileIndexBufferAGP[driver->_CurrentRenderPass&1]);
		uint32 *ptr = (uint32*)buffer->lock ((last-first)*sizeof(uint32), Offset);
		if (!ptr)
		{
			// buffer full, swap them			
			CVolatileIndexBuffer *&bufferOther = VolatileRAM ? (driver->_VolatileIndexBufferRAM[(driver->_CurrentRenderPass + 1) &1]):
			(driver->_VolatileIndexBufferAGP[(driver->_CurrentRenderPass + 1 ) &1]);			
			std::swap(buffer, bufferOther);
			buffer->reset();
			ptr = (uint32*)buffer->lock ((last-first)*sizeof(uint32), Offset);
			nlassert(ptr);			
		}
		nlassert(!VolatileIndexBuffer);
		VolatileIndexBuffer = buffer;
		IndexBuffer = buffer->IndexBuffer;
		ptr -= first;

		// Current lock time
		VolatileLockTime = driver->_CurrentRenderPass;

		// Touch the index buffer
		driver->touchRenderVariable (&driver->_IndexBufferCache);
		
		return ptr;
	}
	else
	{
		nlassert (IndexBuffer);
		// Lock Profile?
		TTicks	beforeLock;
		if(driver->_IBProfiling /*&& Hardware*/)
		{
			beforeLock= CTime::getPerformanceTime();
		}		
		void *pbData;
		HRESULT result = IndexBuffer->Lock ( first*sizeof(uint32), (last-first)*sizeof(uint32), &pbData, readOnly?D3DLOCK_READONLY:0);
		// Lock Profile?
		if(driver->_IBProfiling /*&& Hardware*/)
		{
			TTicks	afterLock;
			afterLock= CTime::getPerformanceTime();
			driver->appendIBLockProfile(afterLock-beforeLock, IndexBufferPtr);
		}
		if (result == D3D_OK) return (uint32*)pbData;
	}
	return NULL;
}

// ***************************************************************************

void	CIBDrvInfosD3D::unlock (uint first, uint last)
{
	H_AUTO_D3D(CIBDrvInfosD3D_unlock)
	if (Volatile)
	{		
		nlassert(VolatileIndexBuffer);
		VolatileIndexBuffer->unlock ();
		VolatileIndexBuffer = NULL;
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
	H_AUTO_D3D(CDriverD3D_activeIndexBuffer)
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
		{
			preferredMemory = CIndexBuffer::RAMResident;
			info->Volatile = false;
		}
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

			indexCount++;
		}


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
	H_AUTO_D3D(CDriverD3D_supportIndexBufferHard);
	return !_DisableHardwareIndexArrayAGP;
}

// ***************************************************************************

void CDriverD3D::disableHardwareIndexArrayAGP()
{
	H_AUTO_D3D(CDriverD3D_disableHardwareIndexArrayAGP)
	_DisableHardwareIndexArrayAGP = true;
}

// ***************************************************************************
// CVolatileIndexBuffer
// ***************************************************************************

CVolatileIndexBuffer::CVolatileIndexBuffer()
{
	H_AUTO_D3D(CVolatileIndexBuffer_CVolatileIndexBuffer);
	IndexBuffer = NULL;
	Locked = false;
}

// ***************************************************************************

CVolatileIndexBuffer::~CVolatileIndexBuffer()
{
	H_AUTO_D3D(CVolatileIndexBuffer_CVolatileIndexBufferDtor);
	release ();
}

// ***************************************************************************

void CVolatileIndexBuffer::release ()
{
	H_AUTO_D3D(CVolatileIndexBuffer_release);
	if (IndexBuffer)
		IndexBuffer->Release();
	IndexBuffer = NULL;
}

// ***************************************************************************

void CVolatileIndexBuffer::init (CIndexBuffer::TLocation	location, uint size, uint maxSize, CDriverD3D *driver)
{
	H_AUTO_D3D(CVolatileIndexBuffer_init);
	release();

	// Init the buffer
	Location = location;
	Size = size;
	MaxSize = maxSize;
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
		Location = CIndexBuffer::RAMResident;
	}
}

// ***************************************************************************

void *CVolatileIndexBuffer::lock (uint size, uint &offset)
{
	nlassertex(!Locked, ("Volatile buffer usage should follow an atomic lock/unlock/render sequence"));
	H_AUTO_D3D(CVolatileIndexBuffer_lock);
	/* If not enough room to allocate this buffer, resise the buffer to Size+Size/2 but do not reset CurrentIndex
	 * to be sure the buffer will be large enough next pass. */
	
	// Enough room for this index ?
	if (CurrentIndex+size > Size)
	{		
		if (CurrentIndex+size > MaxSize && CurrentIndex != 0)
		{
			return NULL;  // max size exceeded -> can reallocate only if we are at start of block
		}		
		// No, reallocate
		init (Location, std::max (std::min(Size+Size/2, MaxSize),  CurrentIndex+size), MaxSize, Driver);
	}
	// Lock Profile?
	TTicks	beforeLock;
	if(Driver->_IBProfiling /*&& Hardware*/)
	{
		beforeLock= CTime::getPerformanceTime();
	}
	// Lock the buffer, noblocking lock here if not the first allocation since a reset
	VOID *pbData;
	if (CurrentIndex==0)
	{
		nlverify (IndexBuffer->Lock (0, size, &pbData, 0) == D3D_OK);
	}
	else
	{
		nlverify (IndexBuffer->Lock (CurrentIndex, size, &pbData, D3DLOCK_NOOVERWRITE) == D3D_OK);
	}
	if(Driver->_IBProfiling /*&& Hardware*/)
	{
		TTicks	afterLock;
		afterLock= CTime::getPerformanceTime();
		Driver->_VolatileIBLockTime += afterLock - beforeLock;
	}		

	// Old buffer position
	offset = CurrentIndex/sizeof(uint32);

	// New buffer position
	CurrentIndex += size;
	Locked = true;
	return pbData;
}

// ***************************************************************************

void CVolatileIndexBuffer::unlock ()
{
	H_AUTO_D3D(CVolatileIndexBuffer_unlock);
	nlassertex(Locked, ("Volatile buffer usage should follow an atomic lock/unlock/render sequence"));
	nlverify (IndexBuffer->Unlock () == D3D_OK);
	Locked = false;
}

// ***************************************************************************

void CVolatileIndexBuffer::reset ()
{
	H_AUTO_D3D(CVolatileIndexBuffer_reset);
	CurrentIndex = 0;
}

// ***************************************************************************

} // NL3D
