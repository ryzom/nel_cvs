/** \file driver_direct3d_vertex.cpp
 * Direct 3d driver implementation
 *
 * $Id: driver_direct3d_render.cpp,v 1.1 2004/03/19 10:11:36 corvazier Exp $
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

// todo hulud beginscene
extern bool begun;

// ***************************************************************************

bool CDriverD3D::renderLines(CMaterial& mat, uint32 firstIndex, uint32 nlines)
{
	if (!begun)
	{
		_DeviceInterface->BeginScene();
		begun = true;
	}

	// Setup material
	if ( !setupMaterial(mat) )
		return false;

	if (nlines == 0)
		return false;

	if (_VertexBuffer.VertexBuffer && _IndexBuffer.IndexBuffer)
	{
		uint pass;
		beginMultiPass ();
		for (pass=0; pass<_CurrentShaderPassCount; pass++)
		{
			// Active the pass
			activePass (pass);

			_DeviceInterface->DrawIndexedPrimitive (D3DPT_LINELIST, 0, 0, _VertexBuffer.VertexBuffer->getNumVertices(), 
				firstIndex, nlines);
		}
		endMultiPass ();
	}
	return true;
}

// ***************************************************************************

bool CDriverD3D::renderTriangles(CMaterial& mat, uint32 firstIndex, uint32 ntris)
{
	if (!begun)
	{
		_DeviceInterface->BeginScene();
		begun = true;
	}

	// Setup material
	if ( !setupMaterial(mat) )
		return false;

	if (ntris == 0)
		return false;

	if (_VertexBuffer.VertexBuffer && _IndexBuffer.IndexBuffer)
	{
		uint pass;
		beginMultiPass ();
		for (pass=0; pass<_CurrentShaderPassCount; pass++)
		{
			// Active the pass
			activePass (pass);

			_DeviceInterface->DrawIndexedPrimitive (D3DPT_TRIANGLELIST, 0, 0, _VertexBuffer.VertexBuffer->getNumVertices(), 
				firstIndex, ntris);
		}
		endMultiPass ();
	}
	return true;
}

// ***************************************************************************

bool CDriverD3D::renderSimpleTriangles(uint32 firstIndex, uint32 ntris)
{
	if (!begun)
	{
		_DeviceInterface->BeginScene();
		begun = true;
	}

	nlassert (ntris != 0);
	nlassert (_VertexBuffer.VertexBuffer);
	nlassert (_IndexBuffer.IndexBuffer);

	// Update matrix and Light if needed
	updateRenderVariables();

	_DeviceInterface->DrawIndexedPrimitive (D3DPT_TRIANGLELIST, 0, 0, _VertexBuffer.VertexBuffer->getNumVertices(), 
		firstIndex, ntris);
	return true;
}

// ***************************************************************************

bool CDriverD3D::renderRawPoints(CMaterial& mat, uint32 firstIndex, uint32 numPoints)
{
	// todo hulud beginscene
	if (!begun)
	{
		_DeviceInterface->BeginScene();
		begun = true;
	}

	// Setup material
	if ( !setupMaterial(mat) )
		return false;

	if (_VertexBuffer.VertexBuffer)
	{
		uint pass;
		beginMultiPass ();
		for (pass=0; pass<_CurrentShaderPassCount; pass++)
		{
			// Active the pass
			activePass (pass);

			_DeviceInterface->DrawPrimitive (D3DPT_POINTLIST, firstIndex, numPoints);
		}
		endMultiPass ();
	}
	return true;
}

// ***************************************************************************

bool CDriverD3D::renderRawLines(CMaterial& mat, uint32 firstIndex, uint32 numPoints)
{
	// todo hulud beginscene
	if (!begun)
	{
		_DeviceInterface->BeginScene();
		begun = true;
	}

	// Setup material
	if ( !setupMaterial(mat) )
		return false;

	if (_VertexBuffer.VertexBuffer)
	{
		uint pass;
		beginMultiPass ();
		for (pass=0; pass<_CurrentShaderPassCount; pass++)
		{
			// Active the pass
			activePass (pass);

			_DeviceInterface->DrawPrimitive (D3DPT_LINELIST, firstIndex, numPoints);
		}
		endMultiPass ();
	}
	return true;
}

// ***************************************************************************

bool CDriverD3D::renderRawTriangles(CMaterial& mat, uint32 firstIndex, uint32 numPoints)
{
	// todo hulud beginscene
	if (!begun)
	{
		_DeviceInterface->BeginScene();
		begun = true;
	}

	// Setup material
	if ( !setupMaterial(mat) )
		return false;

	if (_VertexBuffer.VertexBuffer)
	{
		uint pass;
		beginMultiPass ();
		for (pass=0; pass<_CurrentShaderPassCount; pass++)
		{
			// Active the pass
			activePass (pass);

			_DeviceInterface->DrawPrimitive (D3DPT_TRIANGLELIST, firstIndex, numPoints);
		}
		endMultiPass ();
	}
	return true;
}

// ***************************************************************************

void fillQuadIndexes (uint32 *indexes, uint first, uint last)
{
	uint firstQuad = (first / 6) * 4;
	for (;first<last; first+=6, firstQuad+=4)
	{
		indexes[first+0] = firstQuad+0;
		indexes[first+1] = firstQuad+1;
		indexes[first+2] = firstQuad+2;
		indexes[first+3] = firstQuad+0;
		indexes[first+4] = firstQuad+2;
		indexes[first+5] = firstQuad+3;
	}
}

// ***************************************************************************

void CDriverD3D::setDebugMaterial()
{
	_DeviceInterface->SetRenderState (D3DRS_ALPHABLENDENABLE, FALSE);
	_DeviceInterface->SetRenderState (D3DRS_SRCBLEND, D3DBLEND_ONE);
	_DeviceInterface->SetRenderState (D3DRS_DESTBLEND, D3DBLEND_ZERO);
	_DeviceInterface->SetRenderState (D3DRS_ALPHATESTENABLE, FALSE);
	_DeviceInterface->SetRenderState (D3DRS_ALPHAREF, 128);
	_DeviceInterface->SetRenderState (D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	_DeviceInterface->SetRenderState (D3DRS_LIGHTING, FALSE);
	_DeviceInterface->SetRenderState (D3DRS_TEXTUREFACTOR, NL_D3DCOLOR_RGBA(CRGBA(255,0,255,255)));
	_DeviceInterface->SetRenderState (D3DRS_CULLMODE, D3DCULL_CW);
	_DeviceInterface->SetRenderState (D3DRS_COLORVERTEX, FALSE);
	_DeviceInterface->SetRenderState (D3DRS_ZWRITEENABLE, TRUE);
	_DeviceInterface->SetRenderState (D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	_DeviceInterface->SetRenderState (D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
	_DeviceInterface->SetRenderState (D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL);
	_DeviceInterface->SetRenderState (D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);
	_DeviceInterface->SetRenderState (D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);
	_DeviceInterface->SetTexture (0, NULL);
	_DeviceInterface->SetTexture (1, NULL);
	_DeviceInterface->SetTexture (2, NULL);
	_DeviceInterface->SetTexture (3, NULL);
	_DeviceInterface->SetTexture (4, NULL);
	_DeviceInterface->SetTexture (5, NULL);
	_DeviceInterface->SetTexture (6, NULL);
	_DeviceInterface->SetTexture (7, NULL);
	_DeviceInterface->SetTextureStageState (0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	_DeviceInterface->SetTextureStageState (0, D3DTSS_COLORARG0, D3DTA_TFACTOR);
	_DeviceInterface->SetTextureStageState (0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	_DeviceInterface->SetTextureStageState (0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
	_DeviceInterface->SetTextureStageState (0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	_DeviceInterface->SetTextureStageState (0, D3DTSS_ALPHAARG0, D3DTA_TFACTOR);
	_DeviceInterface->SetTextureStageState (0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
	_DeviceInterface->SetTextureStageState (0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
	// _DeviceInterface->SetTextureStageState (0, D3DTSS_CONSTANT, 0x0);
	uint i;
	for (i=1; i<8; i++)
	{
		_DeviceInterface->SetTextureStageState (i, D3DTSS_COLOROP, D3DTOP_DISABLE);
		_DeviceInterface->SetTextureStageState (i, D3DTSS_COLORARG0, D3DTA_TFACTOR);
		_DeviceInterface->SetTextureStageState (i, D3DTSS_COLORARG1, D3DTA_TFACTOR);
		_DeviceInterface->SetTextureStageState (i, D3DTSS_COLORARG2, D3DTA_TFACTOR);
		_DeviceInterface->SetTextureStageState (i, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		_DeviceInterface->SetTextureStageState (i, D3DTSS_ALPHAARG0, D3DTA_TFACTOR);
		_DeviceInterface->SetTextureStageState (i, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
		_DeviceInterface->SetTextureStageState (i, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
		// _DeviceInterface->SetTextureStageState (i, D3DTSS_CONSTANT, 0x0);
	}
}

// ***************************************************************************

bool CDriverD3D::renderRawQuads(CMaterial& mat, uint32 startIndex, uint32 numQuads)
{
	// todo hulud beginscene
	if (!begun)
	{
		_DeviceInterface->BeginScene();
		begun = true;
	}

	if (numQuads == 0)
		return false;

	// Num of indexes needed
	const uint numQuadsNeeded = numQuads*6;

	// Need to resize the quad indexes array ?
	if (_QuadIndexes.getNumIndexes() < numQuadsNeeded)
	{
		// Resize it
		_QuadIndexes.setNumIndexes(numQuadsNeeded);
		_QuadIndexes.setPreferredMemory (CIndexBuffer::VRAMPreferred);

		// Fill the index buffer in VRAM
		CIndexBufferReadWrite iba;
		_QuadIndexes.lock (iba);
		fillQuadIndexes (iba.getPtr(), 0, numQuadsNeeded); 
	}

	activeIndexBuffer (_QuadIndexes);

	// Setup material
	if ( !setupMaterial(mat) )
		return false;

	if (_VertexBuffer.VertexBuffer)
	{
		uint pass;
		beginMultiPass ();
		for (pass=0; pass<_CurrentShaderPassCount; pass++)
		{
			// Active the pass
			activePass (pass);

			_DeviceInterface->DrawIndexedPrimitive (D3DPT_TRIANGLELIST, startIndex, 0, _VertexBuffer.VertexBuffer->getNumVertices()-startIndex,
				0, numQuads*2);
		}
		endMultiPass ();
	}
	return true;
}

// ***************************************************************************

} // NL3D
