/** \file driver_direct3d_vertex.cpp
 * Direct 3d driver implementation
 *
 * $Id: driver_direct3d_vertex.cpp,v 1.2 2004/03/23 16:32:27 corvazier Exp $
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

#include "3d/vertex_buffer.h"
#include "3d/light.h"
#include "3d/index_buffer.h"
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

CVBDrvInfosD3D::CVBDrvInfosD3D(IDriver *drv, ItVBDrvInfoPtrList it, CVertexBuffer *vb) : IVBDrvInfos(drv, it, vb)
{
	VertexDecl = NULL;
	VertexBuffer = NULL;
}

// ***************************************************************************

extern uint vertexCount=0;

CVBDrvInfosD3D::~CVBDrvInfosD3D()
{
	// Restaure non resident memory
	if (VertexBufferPtr)
	{
		VertexBufferPtr->setLocation(CVertexBuffer::NotResident);
		VertexBufferPtr = NULL;
	}

	// Don't release VertexDecl, it is release by the driver
	if (VertexBuffer)
	{
		vertexCount--;
		VertexBuffer->Release();
	}
}

// ***************************************************************************

uint8	*CVBDrvInfosD3D::lock (uint first, uint last, bool readOnly)
{
	nlassert (VertexBuffer);
	
	void *pbData;
	if (VertexBuffer->Lock ( first*Stride, (last-first)*Stride, &pbData, readOnly?D3DLOCK_READONLY:0) == D3D_OK)
		return (uint8*)pbData;
	return NULL;
}

// ***************************************************************************

void	CVBDrvInfosD3D::unlock (uint first, uint last)
{
	VertexBuffer->Unlock ();
}

// ***************************************************************************

const D3DDECLTYPE RemapVertexBufferTypeNeL2D3D[CVertexBuffer::NumType]=
{
	D3DDECLTYPE_UNUSED, // Double1,
	D3DDECLTYPE_FLOAT1,	// Float1,
	D3DDECLTYPE_UNUSED, // Short1,
	D3DDECLTYPE_UNUSED, // Double2,
	D3DDECLTYPE_FLOAT2, // Float2,
	D3DDECLTYPE_SHORT2, // Short2,
	D3DDECLTYPE_UNUSED, // Double3,
	D3DDECLTYPE_FLOAT3, // Float3,
	D3DDECLTYPE_UNUSED, // Short3,
	D3DDECLTYPE_UNUSED, // Double4,
	D3DDECLTYPE_FLOAT4, // Float4,
	D3DDECLTYPE_SHORT4, // Short4,
	D3DDECLTYPE_D3DCOLOR, // UChar4,
};

// ***************************************************************************

const D3DDECLUSAGE RemapVertexBufferUsageNeL2D3D[CVertexBuffer::NumValue]=
{
	D3DDECLUSAGE_POSITION,	// Position
	D3DDECLUSAGE_NORMAL,	// Normal
	D3DDECLUSAGE_TEXCOORD,	// TexCoord0
	D3DDECLUSAGE_TEXCOORD,	// TexCoord1
	D3DDECLUSAGE_TEXCOORD,	// TexCoord2
	D3DDECLUSAGE_TEXCOORD,	// TexCoord3
	D3DDECLUSAGE_TEXCOORD,	// TexCoord4
	D3DDECLUSAGE_TEXCOORD,	// TexCoord5
	D3DDECLUSAGE_TEXCOORD,	// TexCoord6
	D3DDECLUSAGE_TEXCOORD,	// TexCoord7
	D3DDECLUSAGE_COLOR,		// PrimaryColor
	D3DDECLUSAGE_COLOR,		// SecondaryColor
	D3DDECLUSAGE_BLENDWEIGHT,	// Weight
	D3DDECLUSAGE_BLENDINDICES,	// PaletteSkin
	D3DDECLUSAGE_FOG,		// Fog
};

// ***************************************************************************

const uint RemapVertexBufferIndexNeL2D3D[CVertexBuffer::NumValue]=
{
	0,	// Position
	0,	// Normal
	0,	// TexCoord0
	1,	// TexCoord1
	2,	// TexCoord2
	3,	// TexCoord3
	4,	// TexCoord4
	5,	// TexCoord5
	6,	// TexCoord6
	7,	// TexCoord7
	0,	// PrimaryColor
	1,	// SecondaryColor
	0,	// Weight
	0,	// PaletteSkin
	0,	// Fog
};

// ***************************************************************************

DWORD RemapVertexBufferUsage[CVertexBuffer::PreferredCount]=
{
	D3DUSAGE_DYNAMIC,						// RAMPreferred
	D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY,	// AGPPreferred
	D3DUSAGE_WRITEONLY,						// VRAMPreferred
};

// ***************************************************************************

D3DPOOL RemapVertexBufferPool[CVertexBuffer::PreferredCount]=
{
	D3DPOOL_SYSTEMMEM,	// RAMPreferred
	D3DPOOL_DEFAULT,	// AGPPreferred
	D3DPOOL_DEFAULT,	// VRAMPreferred
};

// ***************************************************************************

bool CDriverD3D::activeVertexBuffer(CVertexBuffer& VB)
{
	// Must not be locked
	nlassert (!VB.isLocked());

	// Must not be empty
	if (VB.capacity() == 0)
		return false;

	const bool touched = (VB.getTouchFlags() & (CVertexBuffer::TouchedReserve|CVertexBuffer::TouchedVertexFormat)) != 0;
	if ((_VertexBuffer.VertexBuffer != &VB) || touched)
	{
		// Build the driver info
		if (touched)
		{
			// Delete previous vertex buffer info
			if (VB.DrvInfos)
			{
				delete VB.DrvInfos;
				nlassert (VB.DrvInfos == NULL);
			}

			// Force the vertex color format to BGRA
			VB.setVertexColorFormat (CVertexBuffer::TBGRA);

			// Rebuild it
			_VBDrvInfos.push_front (NULL);
			ItVBDrvInfoPtrList ite = _VBDrvInfos.begin();
			CVBDrvInfosD3D *info = new CVBDrvInfosD3D(this, ite, &VB);
			*ite = info;

			// Use vertex color ?
			info->UseVertexColor = (VB.getVertexFormat()&CVertexBuffer::PrimaryColorFlag) != 0;
			info->Stride = (uint8)VB.getVertexSize();

			// Create the vertex declaration
			if (!createVertexDeclaration (VB.getVertexFormat(), VB.getValueTypePointer(), &(info->VertexDecl)))
				return false;

			// Create the vertex buffer
			const uint size = VB.capacity()*VB.getVertexSize();
			uint preferredMemory = _DisableHardwareVertexArrayAGP?CVertexBuffer::RAMPreferred:VB.getPreferredMemory ();
			bool sucess;
			do
			{
				if (sucess =(_DeviceInterface->CreateVertexBuffer(size, RemapVertexBufferUsage[preferredMemory],
					0, RemapVertexBufferPool[preferredMemory], &(info->VertexBuffer), NULL) == D3D_OK))
					break;
			}
			while (preferredMemory--);
			if (!sucess)
				return false;

			vertexCount++;

			// Release the local vertex buffer
			VB.DrvInfos = info;
			VB.setLocation ((CVertexBuffer::TLocation)preferredMemory);
		}

		// Set the current vertex buffer
		nlassert (VB.DrvInfos);
		_VertexBuffer.VertexBuffer = &VB;
		_UseVertexColor = static_cast<CVBDrvInfosD3D*>(static_cast<IVBDrvInfos*>(VB.DrvInfos))->UseVertexColor;
		touchRenderVariable (&_VertexBuffer);

		// Set UVRouting
		const uint8 *uvRouting = VB.getUVRouting();
		uint i;
		for (i=0; i<MaxTexture; i++)
			setTextureIndexUV (i, uvRouting[i]);
	}

	/* Hulud test : read in a "write only" vertex buffer. Seams to work well. */
/*
	// Read the vertex buffer
	CVBDrvInfosD3D *info = static_cast<CVBDrvInfosD3D*>(static_cast<IVBDrvInfos*>(VB.DrvInfos));
	static vector<uint8> temp;
	uint size = VB.capacity()*VB.getVertexSize();
	
	// No special flag for the lock, the driver should return a valid vertex buffer pointer with previous values.
	uint8 *out = info->lock (0, 0, false);
	nlassert (out);
	{
		temp.resize (size);
		memcpy (&(temp[0]), out, size);
	}
	info->unlock (0, 0);
	out = info->lock (0, 0, false);
	nlassert (out);
	{
		memcpy (out, &(temp[0]), size);
	}
	info->unlock (0, 0);
*/	

	return true;
}

// ***************************************************************************

bool CDriverD3D::createVertexDeclaration (uint16 vertexFormat, const uint8 *typeArray,
										IDirect3DVertexDeclaration9 **vertexDecl, 
										uint *stride)
{
	CVertexDeclaration declaration;

	// Set the vertex format
	uint i;
	uint j = 0;
	uint offset = 0;
	for (i=0; i<CVertexBuffer::NumValue; i++)
	{
		// Slot used ?
		if (vertexFormat & (1<<i))
		{
			D3DVERTEXELEMENT9 &vertexElement = declaration.VertexElements[j];
			vertexElement.Stream = 0;
			vertexElement.Type = RemapVertexBufferTypeNeL2D3D[(uint)typeArray[i]];
			vertexElement.Offset = offset;
			vertexElement.Method = D3DDECLMETHOD_DEFAULT;
			vertexElement.Usage = RemapVertexBufferUsageNeL2D3D[(uint)i];
			vertexElement.UsageIndex = RemapVertexBufferIndexNeL2D3D[(uint)i];
			
			offset += CVertexBuffer::SizeType[typeArray[i]];
			j++;
		}
	}

	// Set the stride ?
	if (stride)
		*stride = offset;

	// End
	D3DVERTEXELEMENT9 end = D3DDECL_END();
	declaration.VertexElements[j] = end;

	// Look for the same vertex declaration
	std::list<CVertexDeclaration>::iterator ite = _VertexDeclarationList.begin();
	while (ite != _VertexDeclarationList.end())
	{
		for (i=0; i<=j; i++)
		{
			const D3DVERTEXELEMENT9 &vertexElementNew = declaration.VertexElements[i];
			const D3DVERTEXELEMENT9 &vertexElementOld = ite->VertexElements[i];
			if ( (vertexElementNew.Stream != vertexElementOld.Stream) ||
				(vertexElementNew.Type != vertexElementOld.Type) ||
				(vertexElementNew.Offset != vertexElementOld.Offset) ||
				(vertexElementNew.Method != vertexElementOld.Method) ||
				(vertexElementNew.Usage != vertexElementOld.Usage) ||
				(vertexElementNew.UsageIndex != vertexElementOld.UsageIndex))
			{

				break;
			}
		}

		// All is good ?
		if (i == (j+1))
		{
			// It is the same vertex declaration
			*vertexDecl = ite->VertexDecl;
			return true;
		}

		ite++;
	}

	// Not found, create the vertex declaration
	if (_DeviceInterface->CreateVertexDeclaration (declaration.VertexElements, &(declaration.VertexDecl)) != D3D_OK)
	{
		return false;
	}

	// Add the vertex declaration
	_VertexDeclarationList.push_back (declaration);

	// Set the final declaration pointer
	*vertexDecl = declaration.VertexDecl;

	return true;
}

// ***************************************************************************

bool CDriverD3D::supportVertexBufferHard() const 
{
	return !_DisableHardwareVertexArrayAGP;
}

// ***************************************************************************

void CDriverD3D::disableHardwareVertexArrayAGP()
{
	_DisableHardwareVertexArrayAGP = true;
}

// ***************************************************************************

uint CDriverD3D::getMaxVerticesByVertexBufferHard() const
{
	return _MaxVerticesByVertexBufferHard;
}

// ***************************************************************************

void CDriverD3D::restaureVertexBuffer (CVBDrvInfosD3D &vertexBuffer)
{
	/* Restaure a resident vertex buffer to system memory
	 * 
	 * We use a tricky code here. We read "writeonly" vertex buffer data.
	 * We lock the whole buffer and read with the pointer returned.
	 */
	/* Hulud test : read in a "write only" vertex buffer. Seams to work well. */

	CVertexBuffer *vb = vertexBuffer.VertexBufferPtr;
	const uint size = vb->capacity()*vb->getVertexSize();
	
	// No special flag for the lock, the driver should return a valid vertex buffer pointer with previous values.
	uint8 *out = vertexBuffer.lock (0, 0, false);
	nlassert (out);
	{
		if (vb->getLocation () == CVertexBuffer::RAMResident)
		{
			// Realloc local memory
			vb->setLocation (CVertexBuffer::NotResident);	// If resident in RAM, content backuped by setLocation
		}
		else
		{
			// Realloc local memory
			vb->setLocation (CVertexBuffer::NotResident);

			// Lock the vertex buffer
			CVertexBufferReadWrite vba;
			vb->lock (vba);

			// Copy the vertex data
			memcpy (vba.getVertexCoordPointer(), out, size);
		}
	}
	vertexBuffer.unlock (0, 0);
}

// ***************************************************************************

} // NL3D
