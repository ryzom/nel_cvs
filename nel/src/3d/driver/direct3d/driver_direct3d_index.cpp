/** \file driver_direct3d_index.cpp
 * Direct 3d driver implementation
 *
 * $Id: driver_direct3d_index.cpp,v 1.1 2004/03/19 10:11:36 corvazier Exp $
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
	D3DUSAGE_DYNAMIC,	// RAMPreferred
	D3DUSAGE_DYNAMIC,	// AGPPreferred
	0,					// VRAMPreferred
};

// ***************************************************************************

D3DPOOL RemapIndexBufferPool[CIndexBuffer::PreferredCount]=
{
	D3DPOOL_SYSTEMMEM,	// RAMPreferred
	D3DPOOL_DEFAULT,	// AGPPreferred
	D3DPOOL_MANAGED,	// VRAMPreferred
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
					(IB.getWriteOnly((CIndexBuffer::TPreferredMemory)preferredMemory)?D3DUSAGE_WRITEONLY:0)|RemapIndexBufferUsage[preferredMemory],
					D3DFMT_INDEX32, RemapIndexBufferPool[preferredMemory], &(info->IndexBuffer), NULL) == D3D_OK))
					break;
			}
			while (preferredMemory--);
			if (!sucess)
				return false;

			indexCount++;

			// Copy the index buffer
			uint32 *out = info->lock (0, 0, false);
			nlassert (out);
			{
				CIndexBufferRead iba;
				IB.lock(iba);
				memcpy (out, iba.getPtr(), size*sizeof(uint32));
			}
			info->unlock (0, 0);

			// Set the new location
			switch (preferredMemory)
			{
			case CIndexBuffer::RAMPreferred:
				IB.Location = CIndexBuffer::RAMResident;
				break;
			case CIndexBuffer::AGPPreferred:
				IB.Location = CIndexBuffer::AGPResident;
				break;
			case CIndexBuffer::VRAMPreferred:
				IB.Location = CIndexBuffer::VRAMResident;
				break;
			}

			// Release the local index buffer
			IB.DrvInfos = info;
			IB.releaseNonResidentIndexes();
		}

		// Reset touch flags
		IB.resetTouchFlags ();

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

} // NL3D
