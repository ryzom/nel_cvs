/** \file driver_direct3d.cpp
 * Direct 3d driver implementation
 *
 * $Id: driver_direct3d.cpp,v 1.16 2004/08/13 15:25:25 vizerie Exp $
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


#define RASTERIZER D3DDEVTYPE_HAL
//#define RASTERIZER D3DDEVTYPE_REF

#define D3D_WINDOWED_STYLE (WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_THICKFRAME|WS_MINIMIZEBOX|WS_MAXIMIZEBOX)
#define D3D_FULLSCREEN_STYLE (WS_POPUP)

HINSTANCE HInstDLL = NULL;

// ***************************************************************************

// Try to allocate 16Mo by default of AGP Ram.
#define	NL3D_DRV_VERTEXARRAY_AGP_INIT_SIZE		(16384*1024)

// Initial volatile vertex buffer size
#define NL_VOLATILE_RAM_VB_SIZE	512*1024
#define NL_VOLATILE_AGP_VB_SIZE	128*1024
#define NL_VOLATILE_RAM_IB_SIZE	64*1024
#define NL_VOLATILE_AGP_IB_SIZE	1024
//
#define NL_VOLATILE_RAM_VB_MAXSIZE	512*1024
#define NL_VOLATILE_AGP_VB_MAXSIZE	2500*1024
#define NL_VOLATILE_RAM_IB_MAXSIZE	256*1024
#define NL_VOLATILE_AGP_IB_MAXSIZE	256*1024


// ***************************************************************************

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
	HInstDLL = hinstDLL;
	return true;
}

// ***************************************************************************

namespace NL3D
{

// ***************************************************************************

// Version of the driver. Not the interface version!! Increment when implementation of the driver change.
const uint32		CDriverD3D::ReleaseVersion = 0xa;

// ***************************************************************************

__declspec(dllexport) IDriver* NL3D_createIDriverInstance ()
{
	return new CDriverD3D;
}

// ***************************************************************************

__declspec(dllexport) uint32 NL3D_interfaceVersion ()
{
	return IDriver::InterfaceVersion;
}

// ***************************************************************************

CDriverD3D::CDriverD3D()
{
	_SwapBufferCounter = 0;
	_CurrentOcclusionQuery = NULL;
	_D3D = NULL;
	_HWnd = NULL;
	_DeviceInterface = NULL;
	_DestroyWindow = false;
	_CurrentMode.Width = 640;
	_CurrentMode.Height = 480;
	_WindowX = 0;
	_WindowY = 0;
	_UserViewMtx.identity();
	_UserModelMtx.identity();
	_PZBCameraPos = CVector::Null;
	_CurrentMaterial = NULL;
	_CurrentMaterialInfo = NULL;
	_CurrentShader = NULL;
	_BackBuffer = NULL;
	_Maximized = false;
	_HandlePossibleSizeChangeNextSize = false;
	_Interval = 1;
	_AGPMemoryAllocated = 0;
	_VRAMMemoryAllocated = 0;
#ifdef NL_DISABLE_HARDWARE_VERTEX_PROGAM
	_DisableHardwareVertexProgram = true;
#else // NL_DISABLE_HARDWARE_VERTEX_PROGAM
	_DisableHardwareVertexProgram = false;
#endif // NL_DISABLE_HARDWARE_VERTEX_PROGAM
#ifdef NL_DISABLE_HARDWARE_VERTEX_ARRAY_AGP
	_DisableHardwareVertexArrayAGP = true;
#else // NL_DISABLE_HARDWARE_VERTEX_ARRAY_AGP
	_DisableHardwareVertexArrayAGP = false;
#endif // NL_DISABLE_HARDWARE_VERTEX_ARRAY_AGP
#ifdef NL_DISABLE_HARDWARE_INDEX_ARRAY_AGP
	_DisableHardwareIndexArrayAGP = true;
#else // NL_DISABLE_HARDWARE_INDEX_ARRAY_AGP
	_DisableHardwareIndexArrayAGP = false;
#endif // NL_DISABLE_HARDWARE_INDEX_ARRAY_AGP
#ifdef NL_DISABLE_HARDWARE_PIXEL_SHADER
	_DisableHardwarePixelShader = true;
#else // NL_DISABLE_HARDWARE_PIXEL_SHADER
	_DisableHardwarePixelShader = false;
#endif // NL_DISABLE_HARDWARE_PIXEL_SHADER

	// Compute the Flag which say if one texture has been changed in CMaterial.
	_MaterialAllTextureTouchedFlag= 0;
	uint i;
	for(i=0; i < IDRV_MAT_MAXTEXTURES; i++)
	{
		_MaterialAllTextureTouchedFlag|= IDRV_TOUCHED_TEX[i];
	}

	// Default adapter
	_Adapter = 0xffffffff;
	_ModifiedRenderState = NULL;

	// Create a Direct 3d object
    if ( NULL == (_D3D = Direct3DCreate9(D3D_SDK_VERSION)))
	{
		nlwarning ("CDriverD3D::setDisplay: Can't create the direct 3d 9 object.");
	}
	_DepthRangeNear = 0.f;
	_DepthRangeNear = 1.f;
	// default for lightmap
	_LightMapDynamicLightDirty= false;
	_LightMapDynamicLightEnabled= false;
	_CurrentMaterialSupportedShader= CMaterial::Normal;
	// to avoid any problem if light0 never setuped, and ligthmap rendered
	_UserLight0.setupDirectional(CRGBA::Black, CRGBA::White, CRGBA::White, CVector::K);
	// All User Light are disabled by Default
	for(i=0;i<MaxLight;i++)
		_UserLightEnable[i]= false;	
	_CullMode = CCW;
	_ScissorTouched = true;
	_Scissor.X = -1;
	_Scissor.Y = -1;
	_Scissor.Width = -1;
	_Scissor.Height = -1;
	for(uint k = 0; k < MaxTexture; ++k)
	{
		_CurrentUVRouting[k] = (uint8) k;
	}
	_VolatileVertexBufferRAM[0]	= new CVolatileVertexBuffer;
	_VolatileVertexBufferRAM[1]	= new CVolatileVertexBuffer;
	_VolatileVertexBufferAGP[0]	= new CVolatileVertexBuffer;
	_VolatileVertexBufferAGP[1]	= new CVolatileVertexBuffer;
	_VolatileIndexBufferRAM[0]= new CVolatileIndexBuffer;
	_VolatileIndexBufferRAM[1]= new CVolatileIndexBuffer;
	_VolatileIndexBufferAGP[0]= new CVolatileIndexBuffer;
	_VolatileIndexBufferAGP[1]= new CVolatileIndexBuffer;
}

// ***************************************************************************

CDriverD3D::~CDriverD3D()
{
	release();

    if(_D3D != NULL)
	{
        _D3D->Release();
		_D3D = NULL;
	}
	delete _VolatileVertexBufferRAM[0];
	delete _VolatileVertexBufferRAM[1];
	delete _VolatileVertexBufferAGP[0];
	delete _VolatileVertexBufferAGP[1];
	delete _VolatileIndexBufferRAM[0];
	delete _VolatileIndexBufferRAM[1];
	delete _VolatileIndexBufferAGP[0];
	delete _VolatileIndexBufferAGP[1];
}

// ***************************************************************************

void CDriverD3D::resetRenderVariables()
{
	H_AUTO_D3D(CDriver3D_resetRenderVariables);
	
	uint i;
	for (i=0; i<MaxRenderState; i++)
	{
		if (_RenderStateCache[i].Value != 0xcccccccc)
			touchRenderVariable (&(_RenderStateCache[i]));
	}
	for (i=0; i<MaxTexture; i++)
	{
		uint j;
		for (j=0; j<MaxTextureState; j++)
		{
			if (_TextureStateCache[i][j].Value != 0xcccccccc)
				touchRenderVariable (&(_TextureStateCache[i][j]));
		}
	}
	for (i=0; i<MaxTexture; i++)
	{
		if (_TextureIndexStateCache[i].TexGenMode != 0xcccccccc)
			touchRenderVariable (&(_TextureIndexStateCache[i]));
	}
	for (i=0; i<MaxTexture; i++)
	{
		if ((uint32)(_TexturePtrStateCache[i].Texture) != 0xcccccccc)
			touchRenderVariable (&(_TexturePtrStateCache[i]));
	}
	for (i=0; i<MaxSampler; i++)
	{
		uint j;
		for (j=0; j<MaxSamplerState; j++)
		{
			if (_SamplerStateCache[i][j].Value != 0xcccccccc)
				touchRenderVariable (&(_SamplerStateCache[i][j]));
		}
	}
	touchRenderVariable (&(_MatrixCache[remapMatrixIndex (D3DTS_VIEW)]));
	touchRenderVariable (&(_MatrixCache[remapMatrixIndex (D3DTS_PROJECTION)]));
	touchRenderVariable (&(_MatrixCache[remapMatrixIndex (D3DTS_TEXTURE0)]));
	touchRenderVariable (&(_MatrixCache[remapMatrixIndex (D3DTS_TEXTURE1)]));
	touchRenderVariable (&(_MatrixCache[remapMatrixIndex (D3DTS_TEXTURE2)]));
	touchRenderVariable (&(_MatrixCache[remapMatrixIndex (D3DTS_TEXTURE3)]));
	touchRenderVariable (&(_MatrixCache[remapMatrixIndex (D3DTS_TEXTURE4)]));
	touchRenderVariable (&(_MatrixCache[remapMatrixIndex (D3DTS_TEXTURE5)]));
	touchRenderVariable (&(_MatrixCache[remapMatrixIndex (D3DTS_TEXTURE6)]));
	touchRenderVariable (&(_MatrixCache[remapMatrixIndex (D3DTS_TEXTURE7)]));

	// Vertices and indexes are not valid anymore
	_VertexBufferCache.VertexBuffer = NULL;
	_IndexBufferCache.IndexBuffer = NULL;
	_VertexDeclCache.Decl = NULL;

	touchRenderVariable (&(_VertexBufferCache));
	touchRenderVariable (&(_IndexBufferCache));
	touchRenderVariable (&(_VertexDeclCache));

	for (i=0; i<MaxLight; i++)
	{
		if (*(uint32*)(&(_LightCache[i].Light)) != 0xcccccccc)
		{
			_LightCache[i].EnabledTouched = true;
			touchRenderVariable (&(_LightCache[i]));
		}
	}
	
	// Vertices and indexes are not valid anymore
	_VertexProgramCache.VertexProgram = NULL;
	touchRenderVariable (&(_VertexProgramCache));
	_PixelShaderCache.PixelShader = NULL;
	touchRenderVariable (&(_PixelShaderCache));

	for (i=0; i<MaxVertexProgramConstantState; i++)
	{
		touchRenderVariable (&(_VertexProgramConstantCache[i]));
	}
	for (i=0; i<MaxPixelShaderConstantState; i++)
	{
		touchRenderVariable (&(_PixelShaderConstantCache[i]));
	}
	setRenderTarget (NULL, 0, 0, 0, 0, 0, 0);

	CVertexBuffer::TLocation vertexAgpLocation = _DisableHardwareVertexArrayAGP ? CVertexBuffer::RAMResident : CVertexBuffer::AGPResident;
	CIndexBuffer::TLocation indexAgpLocation = _DisableHardwareIndexArrayAGP ? CIndexBuffer::RAMResident : CIndexBuffer::AGPResident;
	

	// Init volatile vertex buffers
	_VolatileVertexBufferRAM[0]->init (CVertexBuffer::RAMResident, _VolatileVertexBufferRAM[0]->Size, _VolatileVertexBufferRAM[0]->MaxSize, this);
	_VolatileVertexBufferRAM[0]->reset ();
	_VolatileVertexBufferRAM[1]->init (CVertexBuffer::RAMResident, _VolatileVertexBufferRAM[1]->Size, _VolatileVertexBufferRAM[1]->MaxSize, this);
	_VolatileVertexBufferRAM[1]->reset ();
	_VolatileVertexBufferAGP[0]->init (vertexAgpLocation, _VolatileVertexBufferAGP[0]->Size, _VolatileVertexBufferAGP[0]->MaxSize, this);
	_VolatileVertexBufferAGP[0]->reset ();
	_VolatileVertexBufferAGP[1]->init (vertexAgpLocation, _VolatileVertexBufferAGP[1]->Size, _VolatileVertexBufferAGP[1]->MaxSize, this);
	_VolatileVertexBufferAGP[1]->reset ();
	_VolatileIndexBufferRAM[0]->init (CIndexBuffer::RAMResident, _VolatileIndexBufferRAM[0]->Size, _VolatileIndexBufferRAM[0]->MaxSize, this);
	_VolatileIndexBufferRAM[0]->reset ();
	_VolatileIndexBufferRAM[1]->init (CIndexBuffer::RAMResident, _VolatileIndexBufferRAM[1]->Size, _VolatileIndexBufferRAM[1]->MaxSize, this);
	_VolatileIndexBufferRAM[1]->reset ();
	_VolatileIndexBufferAGP[0]->init (indexAgpLocation, _VolatileIndexBufferAGP[0]->Size, _VolatileIndexBufferAGP[0]->MaxSize, this);
	_VolatileIndexBufferAGP[0]->reset ();
	_VolatileIndexBufferAGP[1]->init (indexAgpLocation, _VolatileIndexBufferAGP[1]->Size, _VolatileIndexBufferAGP[1]->MaxSize, this);
	_VolatileIndexBufferAGP[1]->reset ();

	_ScissorTouched = true;
}

// ***************************************************************************

void CDriverD3D::initRenderVariables()
{
	H_AUTO_D3D(CDriver3D_initRenderVariables);
	uint i;
	for (i=0; i<MaxRenderState; i++)
	{
		_RenderStateCache[i].StateID = (D3DRENDERSTATETYPE)i;
		_RenderStateCache[i].Value = 0xcccccccc;
		_RenderStateCache[i].Modified = false;
	}
	for (i=0; i<MaxTexture; i++)
	{
		uint j;
		for (j=0; j<MaxTextureState; j++)
		{
			_TextureStateCache[i][j].StageID = i;
			_TextureStateCache[i][j].StateID = (D3DTEXTURESTAGESTATETYPE)j;
			_TextureStateCache[i][j].Value = 0xcccccccc;
			_TextureStateCache[i][j].Modified = false;
		}
	}
	for (i=0; i<MaxTexture; i++)
	{
		_TextureIndexStateCache[i].StageID = i;
		_TextureIndexStateCache[i].TexGenMode = 0xcccccccc;
		_TextureIndexStateCache[i].Modified = false;
	}
	for (i=0; i<MaxTexture; i++)
	{
		_TexturePtrStateCache[i].StageID = i;
		*(uint32*)&(_TexturePtrStateCache[i].Texture) = 0xcccccccc;
		_TexturePtrStateCache[i].Modified = false;
	}
	for (i=0; i<MaxSampler; i++)
	{
		uint j;
		for (j=0; j<MaxSamplerState; j++)
		{
			_SamplerStateCache[i][j].SamplerID = i;
			_SamplerStateCache[i][j].StateID = (D3DSAMPLERSTATETYPE)j;
			_SamplerStateCache[i][j].Value = 0xcccccccc;
			_SamplerStateCache[i][j].Modified = false;
		}
	}
	for (i=0; i<MaxMatrixState; i++)
	{
		_MatrixCache[i].TransformType = (D3DTRANSFORMSTATETYPE)((i>=MatrixStateRemap)?i-MatrixStateRemap+256:i);
		memset (&(_MatrixCache[i].Matrix), 0xcc, sizeof(D3DXMATRIX));
		_MatrixCache[i].Modified = false;
	}
	_VertexBufferCache.Modified = false;
	_VertexBufferCache.VertexBuffer = NULL;
	_IndexBufferCache.Modified = false;
	_IndexBufferCache.IndexBuffer = NULL;
	_VertexDeclCache.Modified = false;
	_VertexDeclCache.Decl = NULL;
	for (i=0; i<MaxLight; i++)
	{
		_LightCache[i].LightIndex = i;
		*(uint32*)&(_LightCache[i].Light) = 0xcccccccc;
		_LightCache[i].Modified = false;
	}
	_VertexProgramCache.Modified = false;
	_VertexProgramCache.VertexProgram = NULL;
	_PixelShaderCache.Modified = false;
	_PixelShaderCache.PixelShader = NULL;
	for (i=0; i<MaxVertexProgramConstantState; i++)
	{
		_VertexProgramConstantCache[i].StateID = i;
		_VertexProgramConstantCache[i].Modified = false;
		_VertexProgramConstantCache[i].ValueType = CVertexProgramConstantState::Undef;
	}
	for (i=0; i<MaxPixelShaderConstantState; i++)
	{
		_PixelShaderConstantCache[i].StateID = i;
		_PixelShaderConstantCache[i].Modified = false;
		_PixelShaderConstantCache[i].ValueType = CPixelShaderConstantState::Undef;
	}
	_RenderTarget.Modified = false;

	// Set the render states cache to its default values
	setRenderState (D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA|D3DCOLORWRITEENABLE_RED|D3DCOLORWRITEENABLE_GREEN|D3DCOLORWRITEENABLE_BLUE);
	setRenderState (D3DRS_CULLMODE, D3DCULL_CW);
	setRenderState (D3DRS_FOGENABLE, FALSE);

	// Force normalize
	_ForceNormalize = false;
	_UseVertexColor = false;

	// Material
	_CurrentMaterial = NULL;
	_CurrentMaterialInfo = NULL;
	_CurrentShader = NULL;

	// Shaders
	initInternalShaders();

	// Fog default values
	_FogStart = 0;
	_FogEnd = 1;
	setRenderState (D3DRS_FOGSTART, *((DWORD*) (&_FogStart)));
	setRenderState (D3DRS_FOGEND, *((DWORD*) (&_FogEnd)));
	setRenderState (D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);

	// Alpha render states
	setRenderState (D3DRS_ALPHATESTENABLE, FALSE);
	setRenderState (D3DRS_ALPHAREF, 128);
	setRenderState (D3DRS_ALPHAFUNC, D3DCMP_GREATER);

	// Cull mode
	_InvertCullMode = false;
	_DoubleSided = false;

	// Depth range
	_DepthRangeNear = 0.f;
	_DepthRangeFar = 1.f;

	// Flush caches
	updateRenderVariablesInternal();
}

// ***************************************************************************

#define NL_SRC_OPERATORS_COUNT 6

// ***************************************************************************

static const D3DTEXTURESTAGESTATETYPE SrcOperators[NL_SRC_OPERATORS_COUNT]=
{
	D3DTSS_COLORARG0,
	D3DTSS_COLORARG1,
	D3DTSS_COLORARG2,
	D3DTSS_ALPHAARG0,
	D3DTSS_ALPHAARG1,
	D3DTSS_ALPHAARG2,
};

// ***************************************************************************

void CDriverD3D::updateRenderVariables()
{
	H_AUTO_D3D(CDriver3D_updateRenderVariables);
	_CurrentMaterial = NULL;
	_CurrentMaterialInfo = NULL;

	updateRenderVariablesInternal();
}

// ***************************************************************************

void CDriverD3D::updateRenderVariablesInternal()
{
	H_AUTO_D3D(CDriver3D_updateRenderVariablesInternal);
	nlassert (_DeviceInterface);

	/* The "unlighted without vertex color" trick */
	if (_CurrentMaterialInfo && _CurrentMaterialInfo->NeedsConstantForDiffuse)
	{
		// No pixel shader ?
		if (_CurrentMaterialInfo->PixelShader)
		{
			/*
			 * We have to set the pixel shader now, because we have to choose between normal pixel shader and pixel shader without vertex color */

			// Must have two pixel shader
			nlassert (_CurrentMaterialInfo->PixelShaderUnlightedNoVertexColor);

			if (!_UseVertexColor && (_VertexProgramCache.VertexProgram == NULL))
			{
				setPixelShader (_CurrentMaterialInfo->PixelShaderUnlightedNoVertexColor);
			}
			else
			{
				setPixelShader (_CurrentMaterialInfo->PixelShader);
			}
		}
		else
		{
			setPixelShader (NULL);

			/*
			 * We have to change all texture state setuped to D3DTA_DIFFUSE into D3DTA_TFACTOR
			 * if we use a vertex buffer with diffuse color vertex with an unlighted material and a vertex program */
			if (!_UseVertexColor && (_VertexProgramCache.VertexProgram == NULL))
			{
				// Max texture
				const uint maxTexture = inlGetNumTextStages();

				// Look for texture state
				uint i;
				for (i=0; i<maxTexture; i++)
				{
					uint j;
					for (j=0; j<NL_SRC_OPERATORS_COUNT; j++)
					{
						D3DTEXTURESTAGESTATETYPE state = SrcOperators[j];
						DWORD value = _TextureStateCache[i][state].Value;
						if ((value & D3DTA_SELECTMASK) == D3DTA_DIFFUSE)
						{
							setTextureState (i, state, (value&~D3DTA_SELECTMASK)|D3DTA_TFACTOR);
						}
					}

					// Operator is D3DTOP_BLENDDIFFUSEALPHA ?
					if (_TextureStateCache[i][D3DTSS_COLOROP].Value == D3DTOP_BLENDDIFFUSEALPHA)
					{
						setTextureState (i, D3DTSS_COLOROP, D3DTOP_LERP);
						setTextureState (i, D3DTSS_COLORARG0, D3DTA_TFACTOR|D3DTA_ALPHAREPLICATE);
					}
					if (_TextureStateCache[i][D3DTSS_ALPHAOP].Value == D3DTOP_BLENDDIFFUSEALPHA)
					{
						setTextureState (i, D3DTSS_ALPHAOP, D3DTOP_LERP);
						setTextureState (i, D3DTSS_ALPHAARG0, D3DTA_TFACTOR);
					}
				}
			}
		}
	}


	// Flush all the modified render states
	while (_ModifiedRenderState)
	{
		// Current variable
		CRenderVariable	*currentRenderState = _ModifiedRenderState;

		// It is clean now
		currentRenderState->Modified = false;

		// Unlinked
		_ModifiedRenderState = currentRenderState->NextModified;

		switch (currentRenderState->Type)
		{
		case CRenderVariable::RenderState:
			{
				CRenderState *renderState = static_cast<CRenderState*>(currentRenderState);
				_DeviceInterface->SetRenderState (renderState->StateID, renderState->Value);
			}
			break;
		case CRenderVariable::TextureState:
			{
				CTextureState *textureState = static_cast<CTextureState*>(currentRenderState);
				_DeviceInterface->SetTextureStageState (textureState->StageID, textureState->StateID, textureState->Value);
			}
			break;
		case CRenderVariable::TextureIndexState:
			{
				CTextureIndexState *textureState = static_cast<CTextureIndexState*>(currentRenderState);
				if (textureState->TexGen)
					setTextureState (textureState->StageID, D3DTSS_TEXCOORDINDEX, textureState->TexGenMode);
				else
					setTextureState (textureState->StageID, D3DTSS_TEXCOORDINDEX, textureState->UVChannel);
			}
			break;
		case CRenderVariable::TexturePtrState:
			{
				CTexturePtrState *textureState = static_cast<CTexturePtrState*>(currentRenderState);
				_DeviceInterface->SetTexture (textureState->StageID, textureState->Texture);
			}
			break;
		case CRenderVariable::VertexProgramPtrState:
			{
				CVertexProgramPtrState *vertexProgram = static_cast<CVertexProgramPtrState*>(currentRenderState);
				_DeviceInterface->SetVertexShader(vertexProgram->VertexProgram);
			}
			break;
		case CRenderVariable::PixelShaderPtrState:
			{
				CPixelShaderPtrState *pixelShader = static_cast<CPixelShaderPtrState*>(currentRenderState);
				_DeviceInterface->SetPixelShader(pixelShader->PixelShader);
			}
			break;
		case CRenderVariable::VertexProgramConstantState:
			{
				CVertexProgramConstantState *vertexProgramConstant = static_cast<CVertexProgramConstantState*>(currentRenderState);
				switch (vertexProgramConstant->ValueType)
				{
				case CVertexProgramConstantState::Float:
					_DeviceInterface->SetVertexShaderConstantF (vertexProgramConstant->StateID, (float*)vertexProgramConstant->Values, 1);
					break;
				case CVertexProgramConstantState::Int:
					_DeviceInterface->SetVertexShaderConstantI (vertexProgramConstant->StateID, (int*)vertexProgramConstant->Values, 1);
					break;
				}
			}
			break;
		case CRenderVariable::PixelShaderConstantState:
			{
				CPixelShaderConstantState *pixelShaderConstant = static_cast<CPixelShaderConstantState*>(currentRenderState);
				switch (pixelShaderConstant->ValueType)
				{
				case CPixelShaderConstantState::Float:
					_DeviceInterface->SetPixelShaderConstantF (pixelShaderConstant->StateID, (float*)pixelShaderConstant->Values, 1);
					break;
				case CPixelShaderConstantState::Int:
					_DeviceInterface->SetPixelShaderConstantI (pixelShaderConstant->StateID, (int*)pixelShaderConstant->Values, 1);
					break;
				}
			}
			break;
		case CRenderVariable::SamplerState:
			{
				CSamplerState *samplerState = static_cast<CSamplerState*>(currentRenderState);
				_DeviceInterface->SetSamplerState (samplerState->SamplerID, samplerState->StateID, samplerState->Value);
			}
			break;
		case CRenderVariable::MatrixState:
			{
				CMatrixState *renderMatrix = static_cast<CMatrixState*>(currentRenderState);
				_DeviceInterface->SetTransform (renderMatrix->TransformType, &(renderMatrix->Matrix));
			}
			break;
		case CRenderVariable::VBState:
			{
				CVBState *renderVB = static_cast<CVBState*>(currentRenderState);
				if (renderVB->VertexBuffer)
				{
					_DeviceInterface->SetStreamSource (0, renderVB->VertexBuffer, renderVB->Offset, renderVB->Stride);
				}
			}
			break;
		case CRenderVariable::IBState:
			{
				CIBState *renderIB = static_cast<CIBState*>(currentRenderState);
				if (renderIB->IndexBuffer)
				{
					_DeviceInterface->SetIndices (renderIB->IndexBuffer);
				}
			}
			break;
		case CRenderVariable::VertexDecl:
			{
				CVertexDeclState *renderVB = static_cast<CVertexDeclState*>(currentRenderState);
				if (renderVB->Decl)
				{
					_DeviceInterface->SetVertexDeclaration (renderVB->Decl);
				}
			}
			break;
		case CRenderVariable::LightState:
			{
				CLightState *renderLight = static_cast<CLightState*>(currentRenderState);
				
				// Enabel state modified ?
				if (renderLight->EnabledTouched)
					_DeviceInterface->LightEnable (renderLight->LightIndex, renderLight->Enabled);

				// Light enabled ?
				if (renderLight->Enabled)
				{
					if (renderLight->SettingsTouched)
					{
						// New position
						renderLight->Light.Position.x -= _PZBCameraPos.x;
						renderLight->Light.Position.y -= _PZBCameraPos.y;
						renderLight->Light.Position.z -= _PZBCameraPos.z;
						_DeviceInterface->SetLight (renderLight->LightIndex, &(renderLight->Light));
						renderLight->SettingsTouched = false;
					}
				}

				// Clean
				renderLight->EnabledTouched = false;
			}
			break;
		case CRenderVariable::RenderTargetState:
			{
				CRenderTargetState *renderTarget = static_cast<CRenderTargetState*>(currentRenderState);
				_DeviceInterface->SetRenderTarget (0, renderTarget->Target);				
				setupViewport (_Viewport);				
				setupScissor (_Scissor);				
			}
			break;
		}
	}
}

// ***************************************************************************

static void D3DWndProc(CDriverD3D *driver, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	H_AUTO_D3D(D3DWndProc);
	// Check this message in parents
	if ((message == WM_SIZE) || (message == WM_EXITSIZEMOVE) || (message == WM_MOVE))
	{
		if (driver != NULL)
		{
			// *** Check the hwnd is the root parent of driver->_HWnd

			// hwnd must be a _HWnd parent 
			bool sameWindow = hWnd == driver->_HWnd;
			bool rootWindow = false;
			HWND tmp = driver->_HWnd;
			while (tmp)
			{
				if (tmp == hWnd)
				{
					rootWindow = true;
					break;
				}
				tmp = GetParent (tmp);
			}

			// hwnd must be a top window
			rootWindow &= (GetParent (hWnd) == NULL);

			// This is a root parent, not _HWnd
			if (rootWindow)
			{
				if (message == WM_SIZE)
				{
					if( SIZE_MAXIMIZED == wParam)
					{
						driver->_Maximized = true;
						if (sameWindow)
							driver->handlePossibleSizeChange();
						else
							driver->_HandlePossibleSizeChangeNextSize = true;
					}
					else if( SIZE_RESTORED == wParam)
					{
						if (driver->_Maximized)
						{
							driver->_Maximized = false;
							if (sameWindow)
								driver->handlePossibleSizeChange();
							else
								driver->_HandlePossibleSizeChangeNextSize = true;
						}
					}
				}
				else if(message == WM_EXITSIZEMOVE)
				{
					if (driver != NULL)
					{
						driver->handlePossibleSizeChange();
					}
				}
				else if(message == WM_MOVE)
				{
					if (driver != NULL)
					{
						RECT rect;
						GetWindowRect (hWnd, &rect);
						driver->_WindowX = rect.left;
						driver->_WindowY = rect.top;
					}
				}
			}
			// This is the window itself
			else if (sameWindow)
			{
				if (message == WM_SIZE)
				{
					if (driver->_HandlePossibleSizeChangeNextSize)
					{
						driver->handlePossibleSizeChange();
						driver->_HandlePossibleSizeChangeNextSize = false;
					}
				}
			}

		}
	}
	
	if (driver->_EventEmitter.getNumEmitters() > 0)
	{
		CWinEventEmitter *we = NLMISC::safe_cast<CWinEventEmitter *>(driver->_EventEmitter.getEmitter(0));
		// Process the message by the emitter
		we->setHWnd((uint32)hWnd);
		we->processMessage ((uint32)hWnd, message, wParam, lParam);
	}
}

// ***************************************************************************

bool CDriverD3D::handlePossibleSizeChange()
{
	H_AUTO_D3D(CDriver3D_handlePossibleSizeChange);
	// If windowed, check if the size as changed
	if (_CurrentMode.Windowed)
	{
		RECT rect;
		GetClientRect (_HWnd, &rect);

		// Setup d3d resolution
		uint newWidth = rect.right-rect.left;
		uint newHeight = rect.bottom-rect.top;

		// Set the new mode. Only change the size, keep the last setDisplay/setMode settings
		GfxMode mode = _CurrentMode;
		mode.Width = newWidth;
		mode.Height = newHeight;
		if ( ( (mode.Width != _CurrentMode.Width) || (mode.Height != _CurrentMode.Height) ) &&
			( mode.Width != 0 ) &&
			( mode.Height != 0 ) )
			return reset (mode);
	}
	return false;
}

// ***************************************************************************

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	H_AUTO_D3D(WndProc);
	// Get the driver pointer..
	CDriverD3D *pDriver=(CDriverD3D*)GetWindowLong (hWnd, GWL_USERDATA);
	if (pDriver != NULL)
	{
		D3DWndProc (pDriver, hWnd, message, wParam, lParam);
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

// ***************************************************************************

bool CDriverD3D::init (uint windowIcon)
{
	H_AUTO_D3D(CDriver3D_init );
	// Register a window class
	WNDCLASS		wc;

	memset(&wc,0,sizeof(wc));
	wc.style			= 0; // CS_HREDRAW | CS_VREDRAW ;//| CS_DBLCLKS;
	wc.lpfnWndProc		= (WNDPROC)WndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= GetModuleHandle(NULL);
	wc.hIcon			= (HICON)windowIcon;
	wc.hCursor			= LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground	= WHITE_BRUSH;
	_WindowClass = "NLD3D" + toString(windowIcon);
	wc.lpszClassName	= _WindowClass.c_str();
	wc.lpszMenuName		= NULL;
	if ( !RegisterClass(&wc) )
		nlwarning ("CDriverD3D::init: Can't register windows class %s", wc.lpszClassName);

	return true;
}

// ***************************************************************************

// From the SDK
bool CDriverD3D::isDepthFormatOk(UINT adapter, D3DFORMAT DepthFormat, 
                      D3DFORMAT AdapterFormat, 
                      D3DFORMAT BackBufferFormat)
{
	H_AUTO_D3D(CDriverD3D_isDepthFormatOk);
    // Verify that the depth format exists
    HRESULT hr = _D3D->CheckDeviceFormat(adapter,
                                         RASTERIZER,
                                         AdapterFormat,
                                         D3DUSAGE_DEPTHSTENCIL,
                                         D3DRTYPE_SURFACE,
                                         DepthFormat);

    if(FAILED(hr)) return FALSE;

    // Verify that the depth format is compatible
    hr = _D3D->CheckDepthStencilMatch(D3DADAPTER_DEFAULT,
                                      RASTERIZER,
                                      AdapterFormat,
                                      BackBufferFormat,
                                      DepthFormat);

    return SUCCEEDED(hr);

}

// ***************************************************************************

const D3DFORMAT FinalPixelFormat[ITexture::UploadFormatCount][CDriverD3D::FinalPixelFormatChoice]=
{
	{ D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8 },	// Auto
	{ D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8 },	// RGBA8888
	{ D3DFMT_A4R4G4B4,	D3DFMT_A1R5G5B5,	D3DFMT_A8R3G3B2,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8 },	// RGBA4444
	{ D3DFMT_A1R5G5B5,	D3DFMT_A4R4G4B4,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8 },	// RGBA5551
	{ D3DFMT_R8G8B8,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8 },	// RGB888
	{ D3DFMT_R5G6B5,	D3DFMT_A1R5G5B5,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8 },	// RGB565
	{ D3DFMT_DXT1,		D3DFMT_R3G3B2,		D3DFMT_A1R5G5B5,	D3DFMT_R5G6B5,		D3DFMT_A8R8G8B8 },	// DXTC1
	{ D3DFMT_DXT1,		D3DFMT_R3G3B2,		D3DFMT_A1R5G5B5,	D3DFMT_R5G6B5,		D3DFMT_A8R8G8B8 },	// DXTC1Alpha
	{ D3DFMT_DXT3,		D3DFMT_A4R4G4B4,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8 },	// DXTC3
	{ D3DFMT_DXT5,		D3DFMT_A4R4G4B4,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8 },	// DXTC5
	{ D3DFMT_L8,		D3DFMT_A8L8,		D3DFMT_R5G6B5,		D3DFMT_A1R5G5B5,	D3DFMT_A8R8G8B8 },	// Luminance
	{ D3DFMT_A8,		D3DFMT_A8L8,		D3DFMT_A8R3G3B2,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8 },	// Alpha
	{ D3DFMT_A8L8,		D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8 },	// AlphaLuminance
	{ D3DFMT_V8U8,		D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8 },	// DsDt
};

// ***************************************************************************

bool CDriverD3D::setDisplay(void* wnd, const GfxMode& mode, bool show) throw(EBadDisplay)
{
	H_AUTO_D3D(CDriver3D_setDisplay);
	if (!_D3D)
		return false;
	// Release the driver if already setuped
	release ();

	// Sould be initialized
	nlassert (!_WindowClass.empty());

	// Should be released
	nlassert (_DeviceInterface == NULL);
	nlassert (_HWnd == NULL);

	// Create a window
	_HWnd = (HWND)wnd;

	// Reset window state
	_Maximized = false;
	_HandlePossibleSizeChangeNextSize = false;

	if (_HWnd)
	{
		// We don't have to destroy this window
		_DestroyWindow = false;

		// Init Window Width and Height
		RECT clientRect;
		GetClientRect (_HWnd, &clientRect);
		_CurrentMode.OffScreen = false;
		_CurrentMode.Width = (uint16)(clientRect.right-clientRect.left);
		_CurrentMode.Height = (uint16)(clientRect.bottom-clientRect.top);
		_CurrentMode.Frequency = 0;
		_CurrentMode.Windowed = true;
		_CurrentMode.Depth = 32;
	}
	else
	{
		_CurrentMode = mode;

		// We have to destroy this window
		_DestroyWindow = true;

		// Window flags
		ULONG WndFlags=(mode.Windowed?D3D_WINDOWED_STYLE:D3D_FULLSCREEN_STYLE)&~WS_VISIBLE;

		// Window rect
		RECT	WndRect;
		WndRect.left=0;
		WndRect.top=0;
		WndRect.right=_CurrentMode.Width;
		WndRect.bottom=_CurrentMode.Height;
		AdjustWindowRect(&WndRect,WndFlags,FALSE);

		// Create
		_HWnd = CreateWindow(_WindowClass.c_str(), "", WndFlags, CW_USEDEFAULT,CW_USEDEFAULT, WndRect.right-WndRect.left,WndRect.bottom-WndRect.top, NULL, NULL, 
			GetModuleHandle(NULL), NULL);
		if (!_HWnd) 
		{
			nlwarning ("CDriverD3D::setDisplay: CreateWindow failed");
			release();
			return false;
		}

		// Set the window long integer
		SetWindowLong (_HWnd, GWL_USERDATA, (LONG)this);

		// Show the window
		if (show || !_CurrentMode.Windowed)
			ShowWindow(_HWnd,SW_SHOW);
	}

	// Choose an adapter
	UINT adapter = (_Adapter==0xffffffff)?D3DADAPTER_DEFAULT:(UINT)_Adapter;

	// Get adapter format
    D3DDISPLAYMODE adapterMode;
	if (_D3D->GetAdapterDisplayMode (adapter, &adapterMode) != D3D_OK)
	{
		nlwarning ("CDriverD3D::setDisplay: GetAdapterDisplayMode failed");
		release();
		return false;
	}

	// Create device options
	D3DPRESENT_PARAMETERS parameters;
	D3DFORMAT adapterFormat;
	if (!fillPresentParameter (parameters, adapterFormat, _CurrentMode, adapter, adapterMode))
	{
		release();
		return false;
	}

	// Create the D3D device
	HRESULT result = _D3D->CreateDevice (adapter, RASTERIZER, (HWND)_HWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING|D3DCREATE_PUREDEVICE, 
		&parameters, &_DeviceInterface);
	if (result != D3D_OK)
	{
		nlwarning ("CDriverD3D::setDisplay: Can't create device.");
		release();
		return false;
	}

	// Check some caps
	D3DCAPS9 caps;
	if (_DeviceInterface->GetDeviceCaps(&caps) == D3D_OK)
	{
		_TextureCubeSupported = (caps.TextureCaps & D3DPTEXTURECAPS_CUBEMAP) != 0;
		_NbNeLTextureStages = (caps.MaxSimultaneousTextures<IDRV_MAT_MAXTEXTURES)?caps.MaxSimultaneousTextures:IDRV_MAT_MAXTEXTURES;
		_MADOperatorSupported = (caps.TextureOpCaps & D3DTEXOPCAPS_MULTIPLYADD) != 0;
		_EMBMSupported = (caps.TextureOpCaps &  D3DTOP_BUMPENVMAP) != 0;
		_PixelShaderVersion = caps.PixelShaderVersion;
	}
	else
	{
		_TextureCubeSupported = false;
		_NbNeLTextureStages = 1;
		_MADOperatorSupported = false;
		_EMBMSupported = false;
		_PixelShaderVersion = 0;
	}
	// test for occlusion query support
	IDirect3DQuery9 *dummyQuery = NULL;
	if (_DeviceInterface->CreateQuery(D3DQUERYTYPE_OCCLUSION, &dummyQuery) == D3DERR_NOTAVAILABLE)
	{
		_OcclusionQuerySupported = false;
	}
	else
	{
		_OcclusionQuerySupported = true;
		if (dummyQuery) dummyQuery->Release();
	}

	
#ifdef NL_FORCE_TEXTURE_STAGE_COUNT
	_NbNeLTextureStages = min ((uint)NL_FORCE_TEXTURE_STAGE_COUNT, (uint)IDRV_MAT_MAXTEXTURES);
#endif // NL_FORCE_TEXTURE_STAGE_COUNT

	_VertexProgram = !_DisableHardwareVertexProgram && ((caps.VertexShaderVersion&0xffff) >= 0x0100);
	_PixelShader = !_DisableHardwarePixelShader && (caps.PixelShaderVersion&0xffff) >= 0x0101;
	_MaxVerticesByVertexBufferHard = caps.MaxVertexIndex;
	_MaxLight = caps.MaxActiveLights;

	// *** Check textures caps
	uint j;
	uint i;
	for (i=0; i<ITexture::UploadFormatCount; i++)
	{
		// Default format : D3DFMT_A8R8G8B8
		_PreferedTextureFormat[i] = D3DFMT_A8R8G8B8;

		for (j=0; j<CDriverD3D::FinalPixelFormatChoice; j++)
		{
			if (isTextureFormatOk(adapter, FinalPixelFormat[i][j], adapterFormat))
				break;
		}

		// Set the prefered pixel format
		if (j<CDriverD3D::FinalPixelFormatChoice)
			_PreferedTextureFormat[i] = FinalPixelFormat[i][j];
	}

	// Reset render state cache
	initRenderVariables();

	// *** Event init

	// Release old emitter
	while (_EventEmitter.getNumEmitters() != 0)
	{
		_EventEmitter.removeEmitter(_EventEmitter.getEmitter(_EventEmitter.getNumEmitters() - 1));
	}
	NLMISC::CWinEventEmitter *we = new NLMISC::CWinEventEmitter;

	// Setup the event emitter, and try to retrieve a direct input interface
	_EventEmitter.addEmitter(we, true /*must delete*/); // the main emitter

	// Try to get direct input
	try
	{
		NLMISC::CDIEventEmitter *diee = NLMISC::CDIEventEmitter::create(GetModuleHandle(NULL), _HWnd, we);
		if (diee)
		{
			_EventEmitter.addEmitter(diee, true);
		}
	}
	catch(EDirectInput &e)
	{
		nlinfo(e.what());
	}

	// Init some variables
	_ForceDXTCCompression = false;
	_ForceTextureResizePower = 0;
	_FogEnabled = false;

	// No back buffer backuped for the moment
	_BackBuffer = NULL;

	// Reset profiling.
	_AllocatedTextureMemory= 0;
	_TextureUsed.clear();
	_PrimitiveProfileIn.reset();
	_PrimitiveProfileOut.reset();
	_NbSetupMaterialCall= 0;
	_NbSetupModelMatrixCall= 0;
	_VBHardProfiling= false;
	_CurVBHardLockCount= 0;
	_NumVBHardProfileFrame= 0;
	_IBProfiling= false;
	_CurIBLockCount= 0;
	_NumIBProfileFrame= 0;

	// try to allocate 16Mo by default of AGP Ram.
	initVertexBufferHard(NL3D_DRV_VERTEXARRAY_AGP_INIT_SIZE, 0);

	// Init volatile vertex buffers
	_CurrentRenderPass = 0;
	_VolatileVertexBufferRAM[0]->init (CVertexBuffer::RAMResident, NL_VOLATILE_RAM_VB_SIZE, NL_VOLATILE_RAM_VB_MAXSIZE, this);
	_VolatileVertexBufferRAM[0]->reset ();
	_VolatileVertexBufferRAM[1]->init (CVertexBuffer::RAMResident, NL_VOLATILE_RAM_VB_SIZE, NL_VOLATILE_RAM_VB_MAXSIZE, this);
	_VolatileVertexBufferRAM[1]->reset ();
	_VolatileVertexBufferAGP[0]->init (CVertexBuffer::AGPResident, NL_VOLATILE_AGP_VB_SIZE, NL_VOLATILE_AGP_VB_MAXSIZE, this);
	_VolatileVertexBufferAGP[0]->reset ();
	_VolatileVertexBufferAGP[1]->init (CVertexBuffer::AGPResident, NL_VOLATILE_AGP_VB_SIZE, NL_VOLATILE_AGP_VB_MAXSIZE, this);
	_VolatileVertexBufferAGP[1]->reset ();
	_VolatileIndexBufferRAM[0]->init (CIndexBuffer::RAMResident, NL_VOLATILE_RAM_IB_SIZE, NL_VOLATILE_RAM_IB_MAXSIZE, this);
	_VolatileIndexBufferRAM[0]->reset ();
	_VolatileIndexBufferRAM[1]->init (CIndexBuffer::RAMResident, NL_VOLATILE_RAM_IB_SIZE, NL_VOLATILE_RAM_IB_MAXSIZE, this);
	_VolatileIndexBufferRAM[1]->reset ();
	_VolatileIndexBufferAGP[0]->init (CIndexBuffer::AGPResident, NL_VOLATILE_AGP_IB_SIZE, NL_VOLATILE_AGP_IB_MAXSIZE, this);
	_VolatileIndexBufferAGP[0]->reset ();
	_VolatileIndexBufferAGP[1]->init (CIndexBuffer::AGPResident, NL_VOLATILE_AGP_IB_SIZE, NL_VOLATILE_AGP_IB_MAXSIZE, this);
	_VolatileIndexBufferAGP[1]->reset ();

	setupViewport (CViewport());

	// Begin now
	if (_DeviceInterface->BeginScene() != D3D_OK)
		return false;

	// Done
	return true;
}

// ***************************************************************************
extern uint indexCount;
extern uint vertexCount;

bool CDriverD3D::release() 
{
	H_AUTO_D3D(CDriver3D_release);
	// Call IDriver::release() before, to destroy textures, shaders and VBs...
	IDriver::release();

	_SwapBufferCounter = 0;

	// delete querries
	while (!_OcclusionQueryList.empty())
	{
		deleteOcclusionQuery(_OcclusionQueryList.front());
	}

	// Back buffer ref
	if (_BackBuffer)
		_BackBuffer->Release();
	_BackBuffer = NULL;

	// Release all the vertex declaration
	std::list<CVertexDeclaration>::iterator ite = _VertexDeclarationList.begin();
	while (ite != _VertexDeclarationList.end())
	{
		ite->VertexDecl->Release();
		ite++;
	}
	_VertexDeclarationList.clear ();

	// Release pixel shaders
	_NormalPixelShaders[0].clear ();
	_NormalPixelShaders[1].clear ();

    if( _DeviceInterface != NULL)
	{
        _DeviceInterface->Release();
		_DeviceInterface = NULL;
	}

	if (_HWnd)
	{
		if (_DestroyWindow)
			DestroyWindow (_HWnd);
		_HWnd = NULL;
	}


	nlassert (indexCount == 0);
	nlassert (vertexCount == 0);
	return true;
};

// ***************************************************************************

emptyProc CDriverD3D::getWindowProc() 
{
	return (emptyProc)D3DWndProc;
};

// ***************************************************************************

IDriver::TMessageBoxId CDriverD3D::systemMessageBox (const char* message, const char* title, TMessageBoxType type, TMessageBoxIcon icon)
{
	switch (::MessageBox (_HWnd, message, title, ((type==retryCancelType)?MB_RETRYCANCEL:
		(type==yesNoCancelType)?MB_YESNOCANCEL:
		(type==okCancelType)?MB_OKCANCEL:
		(type==abortRetryIgnoreType)?MB_ABORTRETRYIGNORE:
		(type==yesNoType)?MB_YESNO|MB_ICONQUESTION:MB_OK)|
		
		((icon==handIcon)?MB_ICONHAND:
		(icon==questionIcon)?MB_ICONQUESTION:
		(icon==exclamationIcon)?MB_ICONEXCLAMATION:
		(icon==asteriskIcon)?MB_ICONASTERISK:
		(icon==warningIcon)?MB_ICONWARNING:
		(icon==errorIcon)?MB_ICONERROR:
		(icon==informationIcon)?MB_ICONINFORMATION:
		(icon==stopIcon)?MB_ICONSTOP:0)))
	{
		case IDOK:
			return okId;
		case IDCANCEL:
			return cancelId;
		case IDABORT:
			return abortId;
		case IDRETRY:
			return retryId;
		case IDIGNORE:
			return ignoreId;
		case IDYES:
			return yesId;
		case IDNO:
			return noId;
	}
	return okId;
}

// ***************************************************************************

bool CDriverD3D::activate() 
{
	return true;
}

// ***************************************************************************

bool CDriverD3D::isActive () 
{
	return (IsWindow(_HWnd) != 0);
}

// ***************************************************************************

void* CDriverD3D::getDisplay()
{
	return (void*)_HWnd;
}

// ***************************************************************************

NLMISC::IEventEmitter	*CDriverD3D::getEventEmitter() 
{ 
	return &_EventEmitter;
}

// ***************************************************************************

void CDriverD3D::getWindowSize (uint32 &width, uint32 &height)
{
	H_AUTO_D3D(CDriverD3D_getWindowSize);
	width = _CurrentMode.Width;
	height = _CurrentMode.Height;
}

// ***************************************************************************

void CDriverD3D::getWindowPos (uint32 &x, uint32 &y)
{
	H_AUTO_D3D(CDriverD3D_getWindowPos);
	x = _WindowX;
	y = _WindowY;
}

// ***************************************************************************

uint32 CDriverD3D::getImplementationVersion () const
{
	H_AUTO_D3D(CDriverD3D_getImplementationVersion);
	return ReleaseVersion;
}

// ***************************************************************************

const char *CDriverD3D::getDriverInformation ()
{
	return "Directx 9 NeL Driver";
}

// ***************************************************************************

uint8 CDriverD3D::getBitPerPixel ()
{
	return _CurrentMode.Depth;
}

// ***************************************************************************

bool CDriverD3D::clear2D(CRGBA rgba) 
{
	H_AUTO_D3D(CDriverD3D_clear2D);
	nlassert (_DeviceInterface);
	
	// Backup viewport
	CViewport oldViewport = _Viewport;
	setupViewport (CViewport());
	updateRenderVariables ();

	bool result = _DeviceInterface->Clear( 0, NULL, D3DCLEAR_TARGET, NL_D3DCOLOR_RGBA(rgba), 1.0f, 0 ) == D3D_OK;

	// Restaure the old viewport
	setupViewport (oldViewport);
	return result;
}

// ***************************************************************************

bool CDriverD3D::clearZBuffer(float zval)
{
	H_AUTO_D3D(CDriverD3D_clearZBuffer);
	nlassert (_DeviceInterface);

	// Backup viewport
	CViewport oldViewport = _Viewport;
	setupViewport (CViewport());
	updateRenderVariables ();

	bool result = _DeviceInterface->Clear( 0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0,0,0,0), zval, 0 ) == D3D_OK;

	// Restaure the old viewport
	setupViewport (oldViewport);

	// NVidia driver 56.72 needs to reset the vertex buffer after a clear Z
	touchRenderVariable (&_VertexBufferCache);

	return result;
}

// ***************************************************************************

void CDriverD3D::setColorMask (bool bRed, bool bGreen, bool bBlue, bool bAlpha) 
{
	H_AUTO_D3D(CDriverD3D_setColorMask);
	setRenderState (D3DRS_COLORWRITEENABLE, 
		(bAlpha?D3DCOLORWRITEENABLE_ALPHA:0)|
		(bRed?D3DCOLORWRITEENABLE_RED:0)|
		(bGreen?D3DCOLORWRITEENABLE_GREEN:0)|
		(bBlue?D3DCOLORWRITEENABLE_BLUE:0)
		);
}

// ***************************************************************************

bool CDriverD3D::swapBuffers() 
{		
	H_AUTO_D3D(CDriverD3D_swapBuffers);
	nlassert (_DeviceInterface);

	++ _SwapBufferCounter;
	// Swap & reset volatile buffers
	_CurrentRenderPass++;
	_VolatileVertexBufferRAM[_CurrentRenderPass&1]->reset ();
	_VolatileVertexBufferAGP[_CurrentRenderPass&1]->reset ();
	_VolatileIndexBufferRAM[_CurrentRenderPass&1]->reset ();
	_VolatileIndexBufferAGP[_CurrentRenderPass&1]->reset ();

	// todo hulud volatile
	_DeviceInterface->SetStreamSource(0, _VolatileVertexBufferRAM[1]->VertexBuffer, 0, 12);

	// Is direct input running ?
	if (_EventEmitter.getNumEmitters() > 1) 
	{
		// flush direct input messages if any
		NLMISC::safe_cast<NLMISC::CDIEventEmitter *>(_EventEmitter.getEmitter(1))->poll();
	}

	// End now
	if (_DeviceInterface->EndScene() != D3D_OK)
		return false;

	HRESULT result;
	if ((result=_DeviceInterface->Present( NULL, NULL, NULL, NULL)) != D3D_OK)
	{
		// Device lost ?
		if (result == D3DERR_DEVICELOST)
		{
			// Reset the driver
			reset (_CurrentMode);
			_DeviceInterface->EndScene();
		}
	}
	
	// Check window size
	handlePossibleSizeChange ();

	// Reset the profiling counter.
	_PrimitiveProfileIn.reset();
	_PrimitiveProfileOut.reset();
	_NbSetupMaterialCall= 0;
	_NbSetupModelMatrixCall= 0;

	// Reset the texture set
	_TextureUsed.clear();

	// Reset vertex program
	setVertexProgram (NULL);

	// Begin now
	return _DeviceInterface->BeginScene() == D3D_OK;
};

// ***************************************************************************

void CDriverD3D::setPolygonMode (TPolygonMode mode)
{
	H_AUTO_D3D(CDriverD3D_setPolygonMode);
	IDriver::setPolygonMode (mode);

	setRenderState (D3DRS_FILLMODE, mode==Point?D3DFILL_POINT:mode==Line?D3DFILL_WIREFRAME:D3DFILL_SOLID);
}

// ***************************************************************************

bool CDriverD3D::isTextureFormatOk(UINT adapter, D3DFORMAT TextureFormat, D3DFORMAT AdapterFormat)
{
	H_AUTO_D3D(CDriverD3D_isTextureFormatOk);
    HRESULT hr = _D3D->CheckDeviceFormat( adapter,
                                          RASTERIZER,
                                          AdapterFormat,
                                          0,
                                          D3DRTYPE_TEXTURE,
                                          TextureFormat);

    return SUCCEEDED( hr ) != FALSE;
}

// ***************************************************************************

void CDriverD3D::forceDXTCCompression(bool dxtcComp)
{
	H_AUTO_D3D(CDriverD3D_forceDXTCCompression);
	_ForceDXTCCompression = dxtcComp;
}

// ***************************************************************************

void CDriverD3D::forceTextureResize(uint divisor)
{
	H_AUTO_D3D(CDriverD3D_forceTextureResize);
	clamp(divisor, 1U, 256U);

	// 16 -> 4.
	_ForceTextureResizePower= getPowerOf2(divisor);
}

// ***************************************************************************

bool CDriverD3D::fogEnabled()
{
	H_AUTO_D3D(CDriverD3D_fogEnabled);
	return _RenderStateCache[D3DRS_FOGENABLE].Value == TRUE;
}

// ***************************************************************************

void CDriverD3D::enableFog(bool enable)
{
	H_AUTO_D3D(CDriverD3D_enableFog);
	_FogEnabled = enable;
	setRenderState (D3DRS_FOGENABLE, enable?TRUE:FALSE);
}

// ***************************************************************************

void CDriverD3D::setupFog(float start, float end, CRGBA color)
{
	H_AUTO_D3D(CDriverD3D_setupFog);
	// Remember fog start and end
	_FogStart = start;
	_FogEnd = end;

	// Set the fog
	setRenderState (D3DRS_FOGCOLOR, NL_D3DCOLOR_RGBA(color));
	setRenderState (D3DRS_FOGSTART, *((DWORD*) (&_FogStart)));
	setRenderState (D3DRS_FOGEND, *((DWORD*) (&_FogEnd)));
}

// ***************************************************************************

float CDriverD3D::getFogStart() const
{
	return _FogStart;
}

// ***************************************************************************

float CDriverD3D::getFogEnd() const
{
	return _FogEnd;
}

// ***************************************************************************

CRGBA CDriverD3D::getFogColor() const
{
	return D3DCOLOR_NL_RGBA(_RenderStateCache[D3DRS_FOGCOLOR].Value);
}

// ***************************************************************************

CVertexBuffer::TVertexColorType CDriverD3D::getVertexColorFormat() const
{
	return CVertexBuffer::TBGRA;
}

// ***************************************************************************

sint CDriverD3D::getNbTextureStages() const
{
	return _NbNeLTextureStages;
}

// ***************************************************************************

bool CDriverD3D::getModes(std::vector<GfxMode> &modes)
{
	H_AUTO_D3D(CDriverD3D_getModes);
	static const D3DFORMAT format[]=
	{
		D3DFMT_A8R8G8B8,
		D3DFMT_X8R8G8B8,
		D3DFMT_R8G8B8,
		D3DFMT_A1R5G5B5,
		D3DFMT_R5G6B5,
	};
	static const uint8 depth[]=
	{
		32,
		32,
		24,
		16,
		16,
	};
	uint f;
	for (f=0; f<sizeof(format)/sizeof(uint); f++)
	{
		UINT adapter = (_Adapter==0xffffffff)?D3DADAPTER_DEFAULT:(UINT)_Adapter;
		const uint count = _D3D->GetAdapterModeCount(adapter, format[f]);
		uint i;
		for (i=0; i<count; i++)
		{
			D3DDISPLAYMODE mode;
			if (_D3D->EnumAdapterModes( adapter, format[f], i, &mode) == D3D_OK)
			{
				GfxMode gfxMode;
				gfxMode.Windowed=false;
				gfxMode.Width=(uint16)mode.Width;
				gfxMode.Height=(uint16)mode.Height;
				gfxMode.Depth=depth[f];
				gfxMode.Frequency=(uint8)mode.RefreshRate;
				modes.push_back (gfxMode);
			}
		}
	}	
	return true;
}

// ***************************************************************************
bool CDriverD3D::getCurrentScreenMode(GfxMode &gfxMode)
{
	H_AUTO_D3D(CDriverD3D_getCurrentScreenMode);
	UINT adapter = (_Adapter==0xffffffff)?D3DADAPTER_DEFAULT:(UINT)_Adapter;
	D3DDISPLAYMODE mode;
	_D3D->GetAdapterDisplayMode(adapter, &mode);
	gfxMode.Windowed=false;
	gfxMode.Width=(uint16)mode.Width;
	gfxMode.Height=(uint16)mode.Height;
	gfxMode.Depth= ((mode.Format==D3DFMT_A8R8G8B8)||(mode.Format==D3DFMT_X8R8G8B8))?32:16;
	gfxMode.Frequency=(uint8)mode.RefreshRate;

	return true;
}

// ***************************************************************************

uint CDriverD3D::getNumAdapter() const
{
H_AUTO_D3D(CDriverD3D_getNumAdapter);
	if (_D3D)
		return _D3D->GetAdapterCount();
	else
		return 0;
}

// ***************************************************************************

bool CDriverD3D::getAdapter(uint adapter, IDriver::CAdapter &desc) const
{
	H_AUTO_D3D(CDriverD3D_getAdapter);
	if (_D3D)
	{
		D3DADAPTER_IDENTIFIER9 identifier;
		UINT _adapter = (adapter==0xffffffff)?D3DADAPTER_DEFAULT:(UINT)adapter;
		if (_D3D->GetAdapterIdentifier(_adapter, 0, &identifier) == D3D_OK)
		{
			desc.Description = identifier.Description;
			desc.DeviceId = identifier.DeviceId;
			desc.DeviceName = identifier.DeviceName;
			desc.Driver = identifier.Driver;
			desc.DriverVersion = (((sint64)identifier.DriverVersion.HighPart<<32))|(sint64)identifier.DriverVersion.LowPart;
			desc.Revision = identifier.Revision;
			desc.SubSysId = identifier.SubSysId;
			desc.VendorId = identifier.VendorId;
			return true;
		}
	}
	return false;
}

// ***************************************************************************

bool CDriverD3D::setAdapter(uint adapter)
{
	H_AUTO_D3D(CDriverD3D_setAdapter);
	if (_D3D)
	{
		D3DDISPLAYMODE adapterMode;
		UINT _adapter = (adapter==0xffffffff)?D3DADAPTER_DEFAULT:(UINT)adapter;
		if (_D3D->GetAdapterDisplayMode (_adapter, &adapterMode) != D3D_OK)
		{
			nlwarning ("CDriverD3D::setDisplay: GetAdapterDisplayMode failed");
			release();
			return false;
		}
		_Adapter = adapter;
		return true;
	}
	return false;
}

// ***************************************************************************

bool CDriverD3D::supportMADOperator() const
{
	H_AUTO_D3D(CDriverD3D_supportMADOperator);
	if (_DeviceInterface)
	{
		return _MADOperatorSupported;
	}
	return false; // don't know..
}

// ***************************************************************************

bool CDriverD3D::setMode (const GfxMode& mode)
{
	H_AUTO_D3D(CDriverD3D_setMode);
    if( mode.Windowed )
    {
        // Set windowed-mode style
        SetWindowLong( _HWnd, GWL_STYLE, D3D_WINDOWED_STYLE|WS_VISIBLE);
    }
    else
    {
        // Set fullscreen-mode style
        SetWindowLong( _HWnd, GWL_STYLE, D3D_FULLSCREEN_STYLE|WS_VISIBLE);
    }

	// Reset the driver
	if (reset (mode))
	{
		// Reajust window
		if( _CurrentMode.Windowed )
		{
			// Window rect
			RECT	WndRect;
			WndRect.left=_WindowX;
			WndRect.top=_WindowY;
			WndRect.right=_WindowX+_CurrentMode.Width;
			WndRect.bottom=_WindowY+_CurrentMode.Height;
			AdjustWindowRect(&WndRect, GetWindowLong (_HWnd, GWL_STYLE), FALSE);

			SetWindowPos( _HWnd, HWND_NOTOPMOST,
				std::max ((int)WndRect.left, 0), std::max ((int)WndRect.top, 0),
						  ( WndRect.right - WndRect.left ),
						  ( WndRect.bottom - WndRect.top ),
						  SWP_SHOWWINDOW );
		}
		return true;
	}
	return false;
}


// ***************************************************************************
void CDriverD3D::deleteIndexBuffer(CIBDrvInfosD3D *indexBuffer)
{
	if (!indexBuffer) return;
	CIndexBuffer *ib = indexBuffer->IndexBufferPtr;	
	// If resident in RAM, content has not been lost
	if (ib->getLocation () != CIndexBuffer::RAMResident)
	{
		// Realloc local memory
		ib->setLocation (CIndexBuffer::NotResident);
		delete indexBuffer;
	}
}


// ***************************************************************************
bool CDriverD3D::reset (const GfxMode& mode)
{
	H_AUTO_D3D(CDriverD3D_reset);
	// Current mode
	_CurrentMode = mode;

	// Restaure non managed vertex buffer in system memory
	ItVBDrvInfoPtrList iteVb = _VBDrvInfos.begin();
	while (iteVb != _VBDrvInfos.end())
	{
		ItVBDrvInfoPtrList iteVbNext = iteVb;
		iteVbNext++;

		CVBDrvInfosD3D *vertexBuffer = static_cast<CVBDrvInfosD3D*>(*iteVb);
		CVertexBuffer *vb = vertexBuffer->VertexBufferPtr;

		// If resident in RAM, content has not been lost
		if (vb->getLocation () != CVertexBuffer::RAMResident)
		{
			// Realloc local memory
			vb->setLocation (CVertexBuffer::NotResident);	// If resident in RAM, content backuped by setLocation
			delete vertexBuffer;
		}

		iteVb = iteVbNext;
	}

	// Restaure non managed index buffer in system memory
	ItIBDrvInfoPtrList iteIb = _IBDrvInfos.begin();
	while (iteIb != _IBDrvInfos.end())
	{
		ItIBDrvInfoPtrList iteIbNext = iteIb;
		iteIbNext++;		
		deleteIndexBuffer(static_cast<CIBDrvInfosD3D*>(*iteIb));
		iteIb = iteIbNext;		
	}
	deleteIndexBuffer(static_cast<CIBDrvInfosD3D*>((IIBDrvInfos *) _QuadIndexesAGP.DrvInfos));

	// Remove render targets
	ItTexDrvSharePtrList ite = _TexDrvShares.begin();
	while (ite != _TexDrvShares.end())
	{
		ItTexDrvSharePtrList iteNext = ite;
		iteNext++;

		// Render target ?
		nlassert ((*ite)->DrvTexture != NULL);
		CTextureDrvInfosD3D *info = static_cast<CTextureDrvInfosD3D*>(static_cast<ITextureDrvInfos*>((*ite)->DrvTexture));
		if (info->RenderTarget)
		{
			// Remove it
			delete *ite;
		}

		ite = iteNext;
	}

	// Free volatile buffers
	_VolatileVertexBufferRAM[0]->release ();
	_VolatileVertexBufferRAM[1]->release ();
	_VolatileVertexBufferAGP[0]->release ();
	_VolatileVertexBufferAGP[1]->release ();
	_VolatileIndexBufferRAM[0]->release ();
	_VolatileIndexBufferRAM[1]->release ();
	_VolatileIndexBufferAGP[0]->release ();
	_VolatileIndexBufferAGP[1]->release ();

	// Back buffer ref
	if (_BackBuffer)
		_BackBuffer->Release();
	_BackBuffer = NULL;

	// Release internal shaders
	releaseInternalShaders();

	// Choose an adapter
	UINT adapter = (_Adapter==0xffffffff)?D3DADAPTER_DEFAULT:(UINT)_Adapter;

	// Get adapter format
	D3DDISPLAYMODE adapterMode;
	if (_D3D->GetAdapterDisplayMode (adapter, &adapterMode) != D3D_OK)
	{
		nlwarning ("CDriverD3D::setDisplay: GetAdapterDisplayMode failed");
		release();
		return false;
	}

	// Do the reset

	// Increment the IDriver reset counter

	D3DPRESENT_PARAMETERS parameters;
	D3DFORMAT adapterFormat;
	if (!fillPresentParameter (parameters, adapterFormat, mode, adapter, adapterMode))
		return false;

	/* Do not reset if reset will fail */
	if (_DeviceInterface->TestCooperativeLevel() != D3DERR_DEVICELOST)
	{
		_ResetCounter++;
		if (_DeviceInterface->Reset (&parameters) != D3D_OK)
			return false;

		// BeginScene now
		_DeviceInterface->BeginScene();
	}

	// Reset internal caches
	resetRenderVariables();

	// Init shaders
	initInternalShaders();

	return true;
}

// ***************************************************************************

bool CDriverD3D::fillPresentParameter (D3DPRESENT_PARAMETERS &parameters, D3DFORMAT &adapterFormat, const GfxMode& mode, UINT adapter, const D3DDISPLAYMODE &adapterMode)
{
	H_AUTO_D3D(CDriverD3D_fillPresentParameter);
	memset (&parameters, 0, sizeof(D3DPRESENT_PARAMETERS));
	parameters.BackBufferWidth = mode.Width;
	parameters.BackBufferHeight = mode.Height;
	parameters.BackBufferCount = 1;
	parameters.MultiSampleType = D3DMULTISAMPLE_NONE;
	parameters.MultiSampleQuality = 0;
	parameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	parameters.hDeviceWindow = NULL;
	parameters.Windowed = mode.Windowed;
	parameters.EnableAutoDepthStencil = TRUE;
	parameters.FullScreen_RefreshRateInHz = mode.Windowed?0:mode.Frequency;
	switch (_Interval)
	{
	case 0:
		parameters.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
		break;
	case 1:
		parameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
		break;
	case 2:
		parameters.PresentationInterval = D3DPRESENT_INTERVAL_TWO;
		break;
	case 3:
		parameters.PresentationInterval = D3DPRESENT_INTERVAL_THREE;
		break;
	default:
		parameters.PresentationInterval = D3DPRESENT_INTERVAL_FOUR;
		break;
	}
	parameters.Flags |= D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

	// Builda depth index
	const uint depthIndex = (mode.Depth==16)?0:(mode.Depth==24)?1:2;
	
	// Choose a back buffer format
	const uint numFormat = 4;
	D3DFORMAT backBufferFormats[3][numFormat]=
	{
		{D3DFMT_R5G6B5,		D3DFMT_A1R5G5B5, D3DFMT_X1R5G5B5, D3DFMT_A4R4G4B4},
		{D3DFMT_R8G8B8,		D3DFMT_A8R8G8B8, D3DFMT_X8R8G8B8, D3DFMT_X8R8G8B8},
		{D3DFMT_A8R8G8B8,	D3DFMT_X8R8G8B8, D3DFMT_X8R8G8B8, D3DFMT_X8R8G8B8},
	};
	bool found = false;
	if (mode.Windowed)
	{
		uint backBuffer;
		for (backBuffer=0; backBuffer<numFormat; backBuffer++)
		{
			if (_D3D->CheckDeviceType(adapter, RASTERIZER, adapterMode.Format, backBufferFormats[depthIndex][backBuffer], FALSE) == D3D_OK)
			{
				parameters.BackBufferFormat = backBufferFormats[depthIndex][backBuffer];
				adapterFormat = adapterMode.Format;
				found = true;
				break;
			}
		}
	}
	else
	{
		// Found a pair display and back buffer format
		uint backBuffer;
		for (backBuffer=0; backBuffer<numFormat; backBuffer++)
		{
			uint display;
			for (display=0; display<numFormat; display++)
			{
				if (_D3D->CheckDeviceType(adapter, RASTERIZER, backBufferFormats[depthIndex][display], backBufferFormats[depthIndex][display], FALSE) == D3D_OK)
				{
					parameters.BackBufferFormat = backBufferFormats[depthIndex][backBuffer];
					adapterFormat = backBufferFormats[depthIndex][display];
					found = true;
					break;
				}
			}
			if (found)
				break;
		}
	}

	// Not found ?
	if (!found)
	{
		nlwarning ("CDriverD3D::setDisplay: Can't create backbuffer");
		return false;
	}

	// Choose a zbuffer format
	D3DFORMAT zbufferFormats[]=
	{
		D3DFMT_D32,
		D3DFMT_D24X8,
		D3DFMT_D24S8,
		D3DFMT_D24X4S4,
		D3DFMT_D24FS8,
		D3DFMT_D16,
	};
	const uint zbufferFormatCount = sizeof(zbufferFormats)/sizeof(D3DFORMAT);
	uint i;
	for (i=0; i<zbufferFormatCount; i++)
	{
		if (isDepthFormatOk (adapter, zbufferFormats[i], adapterFormat, parameters.BackBufferFormat))
			break;
	}

	if (i>=zbufferFormatCount)
	{
		nlwarning ("CDriverD3D::setDisplay: Can't create zbuffer");
		return false;
	}

	// Set the zbuffer format
	parameters.AutoDepthStencilFormat = zbufferFormats[i];

	return true;
}

// ***************************************************************************

const char *CDriverD3D::getVideocardInformation ()
{
	H_AUTO_D3D(CDriverD3D_getVideocardInformation);
	static char name[1024];

	if (!_DeviceInterface) 
		return "Direct3d isn't initialized";

	D3DADAPTER_IDENTIFIER9 identifier;
	UINT adapter = (_Adapter==0xffffffff)?D3DADAPTER_DEFAULT:(UINT)_Adapter;
	if (_D3D->GetAdapterIdentifier(adapter, 0, &identifier) == D3D_OK)
	{
		uint64 version = ((uint64)identifier.DriverVersion.HighPart) << 32;
		version |= identifier.DriverVersion.LowPart;
		smprintf (name, 1024, "Direct3d / %s / %s / %s / driver version : %d.%d.%d.%d", identifier.Driver, identifier.Description, identifier.DeviceName, 
			(uint16)(version>>48),(uint16)(version>>32),(uint16)(version>>16),(uint16)(version&0xffff));
		return name;
	}
	else
		return "Can't get video card informations";
}

// ***************************************************************************

void CDriverD3D::getBuffer (CBitmap &bitmap)
{
	H_AUTO_D3D(CDriverD3D_getBuffer);
	NLMISC::CRect rect;
	rect.setWH (0, 0, _CurrentMode.Width, _CurrentMode.Height);
	getBufferPart (bitmap, rect);
}

// ***************************************************************************

void CDriverD3D::getBufferPart (CBitmap &bitmap, NLMISC::CRect &rect)
{
	H_AUTO_D3D(CDriverD3D_getBufferPart);
	if (_DeviceInterface)
	{
		// Resize the bitmap
		const uint lineCount = rect.Height;
		const uint width = rect.Width;
		bitmap.resize (width, lineCount, CBitmap::RGBA);

		// Lock the back buffer
		IDirect3DSurface9 *surface;
		if (_DeviceInterface->GetBackBuffer (0, 0, D3DBACKBUFFER_TYPE_MONO, &surface) == D3D_OK)
		{
			// Surface desc
			D3DSURFACE_DESC desc;
			if (surface->GetDesc(&desc) == D3D_OK)
			{
				// 32 bits format supported
				if ((desc.Format == D3DFMT_R8G8B8) ||
					(desc.Format == D3DFMT_A8R8G8B8) ||
					(desc.Format == D3DFMT_X8R8G8B8) ||
					(desc.Format == D3DFMT_A8B8G8R8) ||
					(desc.Format == D3DFMT_X8B8G8R8))
				{
					// Invert RGBA ?
					bool invertRGBA = (desc.Format == D3DFMT_R8G8B8) || (desc.Format == D3DFMT_A8R8G8B8) || (desc.Format == D3DFMT_X8R8G8B8);

					// Lock the surface
					D3DLOCKED_RECT lock;
					::RECT winRect;
					winRect.left = rect.left();
					winRect.right = rect.right();
					winRect.top = rect.top();
					winRect.bottom = rect.bottom();
					if (surface->LockRect (&lock, &winRect, D3DLOCK_READONLY) == D3D_OK)
					{
						// Line count
						uint8 *dest = &(bitmap.getPixels ()[0]);
						uint i;
						for (i=0; i<lineCount; i++)
						{
							if (invertRGBA)
								copyRGBA2BGRA ((uint32*)dest+(i*width), (uint32*)((uint8*)lock.pBits+(i*lock.Pitch)), width);
							else
								memcpy (dest+(4*i*width), ((uint8*)lock.pBits)+(i*lock.Pitch), width*4);
						}

						surface->UnlockRect ();
					}
				}
			}

			surface->Release();
		}
	}
}

// ***************************************************************************

bool CDriverD3D::fillBuffer (NLMISC::CBitmap &bitmap)
{
	H_AUTO_D3D(CDriverD3D_fillBuffer);
	bool result = false;
	if (_DeviceInterface)
	{
		// Resize the bitmap
		const uint lineCount = _CurrentMode.Height;
		const uint width = _CurrentMode.Width;
		if ((bitmap.getWidth() == width) && (bitmap.getHeight() == lineCount))
		{
			// Lock the back buffer
			IDirect3DSurface9 *surface;
			if (_DeviceInterface->GetBackBuffer (0, 0, D3DBACKBUFFER_TYPE_MONO, &surface) == D3D_OK)
			{
				// Surface desc
				D3DSURFACE_DESC desc;
				if (surface->GetDesc(&desc) == D3D_OK)
				{
					// 32 bits format supported
					if ((desc.Format == D3DFMT_R8G8B8) ||
						(desc.Format == D3DFMT_A8R8G8B8) ||
						(desc.Format == D3DFMT_X8R8G8B8) ||
						(desc.Format == D3DFMT_A8B8G8R8) ||
						(desc.Format == D3DFMT_X8B8G8R8))
					{
						// Invert RGBA ?
						bool invertRGBA = (desc.Format == D3DFMT_R8G8B8) || (desc.Format == D3DFMT_A8R8G8B8) || (desc.Format == D3DFMT_X8R8G8B8);

						// Lock the surface
						D3DLOCKED_RECT lock;
						::RECT winRect;
						winRect.left = 0;
						winRect.right = width;
						winRect.top = 0;
						winRect.bottom = lineCount;
						if (surface->LockRect (&lock, &winRect, 0) == D3D_OK)
						{
							// Line count
							uint8 *dest = &(bitmap.getPixels ()[0]);
							uint i;
							for (i=0; i<lineCount; i++)
							{
								if (invertRGBA)
									copyRGBA2BGRA ((uint32*)((uint8*)lock.pBits+(i*lock.Pitch)), (uint32*)dest+(i*width), width);
								else
									memcpy (((uint8*)lock.pBits)+(i*lock.Pitch), dest+(4*i*width), width*4);
							}

							surface->UnlockRect ();
							result = true;
						}
					}
				}

				surface->Release();
			}
		}
	}
	return result;
}

// ***************************************************************************

void CDriverD3D::setSwapVBLInterval(uint interval)
{
	H_AUTO_D3D(CDriverD3D_setSwapVBLInterval);
	if (_Interval != interval)
	{
		_Interval = interval;

		// Must do a reset because the vsync parameter is choosed in the D3DPRESENT structure
		if (_DeviceInterface)
			reset (_CurrentMode);
	}
}

// ***************************************************************************

uint CDriverD3D::getSwapVBLInterval()
{
	H_AUTO_D3D(CDriverD3D_getSwapVBLInterval);	
	return _Interval;
}

// ***************************************************************************

void CDriverD3D::finish()
{
	H_AUTO_D3D(CDriverD3D_finish);	
	// Flush now
	_DeviceInterface->EndScene();
	_DeviceInterface->BeginScene();
}

// ***************************************************************************

bool CDriverD3D::setMonitorColorProperties (const CMonitorColorProperties &properties)
{
	H_AUTO_D3D(CDriverD3D_setMonitorColorProperties);	
	// The ramp
	D3DGAMMARAMP ramp;

	// For each composant
	uint c;
	for( c=0; c<3; c++ )
	{
		WORD *table = (c==0)?ramp.red:(c==1)?ramp.green:ramp.blue;
		uint i;
		for( i=0; i<256; i++ )
		{
			// Floating value
			float value = (float)i / 256;

			// Contrast
			value = (float) max (0.0f, (value-0.5f) * (float) pow (3.f, properties.Contrast[c]) + 0.5f );

			// Gamma
			value = (float) pow (value, (properties.Gamma[c]>0) ? 1 - 3 * properties.Gamma[c] / 4 : 1 - properties.Gamma[c] );

			// Luminosity
			value = value + properties.Luminosity[c] / 2.f;
			table[i] = min (65535, max (0, (int)(value * 65535)));
		}
	}

	// Set the ramp
	_DeviceInterface->SetGammaRamp (0, D3DSGR_NO_CALIBRATION, &ramp);
	return true;
}
// ***************************************************************************

//****************************************************************************
bool CDriverD3D::supportEMBM() const
{
	H_AUTO_D3D(CDriverD3D_supportEMBM);	
	return _EMBMSupported;
}

//****************************************************************************
bool CDriverD3D::isEMBMSupportedAtStage(uint stage) const
{
	H_AUTO_D3D(CDriverD3D_isEMBMSupportedAtStage);	
	// we assume EMBM is supported at all stages except the last one
	return stage < (uint) _NbNeLTextureStages - 1;
}

//****************************************************************************
void CDriverD3D::setEMBMMatrix(const uint stage, const float mat[4])
{
	H_AUTO_D3D(CDriverD3D_setEMBMMatrix);
	nlassert(stage < (uint) _NbNeLTextureStages - 1);
	SetTextureStageState(stage, D3DTSS_BUMPENVMAT00, (DWORD &) mat[0]);
	SetTextureStageState(stage, D3DTSS_BUMPENVMAT01, (DWORD &) mat[1]);
	SetTextureStageState(stage, D3DTSS_BUMPENVMAT10, (DWORD &) mat[2]);
	SetTextureStageState(stage, D3DTSS_BUMPENVMAT11, (DWORD &) mat[3]);
}

// ***************************************************************************
bool CDriverD3D::supportOcclusionQuery() const
{
	H_AUTO_D3D(CDriverD3D_supportOcclusionQuery);	
	return _OcclusionQuerySupported;
}

// ***************************************************************************
IOcclusionQuery *CDriverD3D::createOcclusionQuery()
{
	H_AUTO_D3D(CDriverD3D_createOcclusionQuery);	
	nlassert(_OcclusionQuerySupported);
	nlassert(_DeviceInterface);
	IDirect3DQuery9 *query;
	if (_DeviceInterface->CreateQuery(D3DQUERYTYPE_OCCLUSION, &query) != D3D_OK) return false;	
	COcclusionQueryD3D *oqd3d = new COcclusionQueryD3D;
	oqd3d->Driver = this;
	oqd3d->Query = query;
	oqd3d->VisibleCount = 0;
	oqd3d->OcclusionType = IOcclusionQuery::NotAvailable;
	_OcclusionQueryList.push_front(oqd3d);
	oqd3d->Iterator = _OcclusionQueryList.begin();
	return oqd3d;
}

// ***************************************************************************
void CDriverD3D::deleteOcclusionQuery(IOcclusionQuery *oq)
{
	H_AUTO_D3D(CDriverD3D_deleteOcclusionQuery);	
	if (!oq) return;
	COcclusionQueryD3D *oqd3d = NLMISC::safe_cast<COcclusionQueryD3D *>(oq);
	nlassert((CDriverD3D *) oqd3d->Driver == this); // should come from the same driver
	oqd3d->Driver = NULL;
	nlassert(oqd3d->Query);
	oqd3d->Query->Release();	
	_OcclusionQueryList.erase(oqd3d->Iterator);
	if (oqd3d == _CurrentOcclusionQuery)
	{
		_CurrentOcclusionQuery = NULL;
	}
	delete oqd3d;
}

// ***************************************************************************
void COcclusionQueryD3D::begin()
{	
	H_AUTO_D3D(COcclusionQueryD3D_begin);	
	nlassert(Driver);
	nlassert(Driver->_CurrentOcclusionQuery == NULL); // only one query at a time
	nlassert(Query);
	Query->Issue(D3DISSUE_BEGIN);
	Driver->_CurrentOcclusionQuery = this;
	OcclusionType = NotAvailable;
}

// ***************************************************************************
void COcclusionQueryD3D::end()
{
	H_AUTO_D3D(COcclusionQueryD3D_end);
	nlassert(Driver);	
	nlassert(Driver->_CurrentOcclusionQuery == this); // only one query at a time
	nlassert(Query);
	Query->Issue(D3DISSUE_END);
	Driver->_CurrentOcclusionQuery = NULL;
}

// ***************************************************************************
IOcclusionQuery::TOcclusionType COcclusionQueryD3D::getOcclusionType()
{
	H_AUTO_D3D(COcclusionQueryD3D_getOcclusionType);
	nlassert(Driver);
	nlassert(Query);
	nlassert(Driver->_CurrentOcclusionQuery != this) // can't query result between a begin/end pair!
	if (OcclusionType == NotAvailable)
	{
		DWORD numPix;
		if (Query->GetData(&numPix, sizeof(DWORD), 0) == S_OK)
		{
			OcclusionType = numPix != 0 ? NotOccluded : Occluded;
			VisibleCount = (uint) numPix;
		}		
	}
	return OcclusionType;
}

// ***************************************************************************
uint COcclusionQueryD3D::getVisibleCount()
{
	H_AUTO_D3D(COcclusionQueryD3D_getVisibleCount);	
	nlassert(Driver);
	nlassert(Query);
	nlassert(Driver->_CurrentOcclusionQuery != this) // can't query result between a begin/end pair!
	if (getOcclusionType() == NotAvailable) return 0;
	return VisibleCount;
}

// ***************************************************************************
bool CDriverD3D::isWaterShaderSupported() const
{			
	H_AUTO_D3D(CDriverD3D_isWaterShaderSupported);	
	return _PixelShaderVersion >= D3DPS_VERSION(1, 1);
}

// ***************************************************************************
void CDriverD3D::setCullMode(TCullMode cullMode)
{
	H_AUTO_D3D(CDriver3D_cullMode);
#ifdef NL_D3D_USE_RENDER_STATE_CACHE
	if (cullMode != _CullMode)
#endif
	{
		if (_InvertCullMode)
		{
			setRenderState(D3DRS_CULLMODE, _CullMode == CCW ? D3DCULL_CW : D3DCULL_CCW);
		}
		else
		{
			setRenderState(D3DRS_CULLMODE, _CullMode == CCW ? D3DCULL_CCW : D3DCULL_CW);
		}
		_CullMode = cullMode;
	}
}

// ***************************************************************************
IDriver::TCullMode CDriverD3D::getCullMode() const
{
	H_AUTO_D3D(CDriver3D_CDriverD3D);
	return _CullMode;
}



} // NL3D

