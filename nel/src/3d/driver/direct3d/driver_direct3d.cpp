/** \file driver_direct3d.cpp
 * Direct 3d driver implementation
 *
 * $Id: driver_direct3d.cpp,v 1.4 2004/03/23 16:32:27 corvazier Exp $
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

HINSTANCE HInstDLL = NULL;

// ***************************************************************************

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
	HInstDLL = hinstDLL;
	return true;
}

// ***************************************************************************

namespace NL3D
{

// todo hulud beginscene
extern bool begun = false;

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
}

// ***************************************************************************

void CDriverD3D::resetRenderVariables()
{
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
		_TextureIndexStateCache[i].Modified = false;
	}
	for (i=0; i<MaxTexture; i++)
	{
		_TexturePtrStateCache[i].StageID = i;
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
	_VertexBuffer.Modified = false;
	_VertexBuffer.VertexBuffer = NULL;
	_IndexBuffer.Modified = false;
	_IndexBuffer.IndexBuffer = NULL;
	for (i=0; i<MaxLight; i++)
	{
		_LightCache[i].LightIndex = i;
		_LightCache[i].Modified = false;
	}
	_VertexProgramCache.Modified = false;
	_VertexProgramCache.VertexProgram = NULL;
	_PixelShaderCache.Modified = false;
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

	// Flush caches
	updateRenderVariables();
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
					CVBDrvInfosD3D *info = static_cast<CVBDrvInfosD3D*>(static_cast<IVBDrvInfos*>(renderVB->VertexBuffer->DrvInfos));
					nlassert (info);	// Must be setuped

					_DeviceInterface->SetVertexDeclaration (info->VertexDecl);
					_DeviceInterface->SetStreamSource (0, info->VertexBuffer, 0, info->Stride);
				}
			}
			break;
		case CRenderVariable::IBState:
			{
				CIBState *renderIB = static_cast<CIBState*>(currentRenderState);
				if (renderIB->IndexBuffer)
				{
					CIBDrvInfosD3D *info = static_cast<CIBDrvInfosD3D*>(static_cast<IIBDrvInfos*>(renderIB->IndexBuffer->DrvInfos));
					nlassert (info);	// Must be setuped

					_DeviceInterface->SetIndices (info->IndexBuffer);
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
						_DeviceInterface->SetLight (renderLight->LightIndex, &(renderLight->Light));
					renderLight->SettingsTouched = false;
				}

				// Clean
				renderLight->EnabledTouched = false;
			}
			break;
		case CRenderVariable::RenderTargetState:
			{
				CRenderTargetState *renderTarget = static_cast<CRenderTargetState*>(currentRenderState);
				_DeviceInterface->SetRenderTarget (0, renderTarget->Target);

				// Reset the viewport and the scissor to fit the new render target
				setupViewport (_Viewport);
				setupScissor (_Scissor);
			}
			break;
		}
	}

	if (_VertexBuffer.VertexBuffer)
	{
		CVBDrvInfosD3D *info = static_cast<CVBDrvInfosD3D*>(static_cast<IVBDrvInfos*>(_VertexBuffer.VertexBuffer->DrvInfos));
		nlassert (info);	// Must be setuped

		_DeviceInterface->SetVertexDeclaration (info->VertexDecl);
		_DeviceInterface->SetStreamSource (0, info->VertexBuffer, 0, info->Stride);
	}
}

// ***************************************************************************

static void D3DWndProc(CDriverD3D *driver, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if(message == WM_SIZE)
	{
		if (driver != NULL)
		{
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
	
	if (driver->_EventEmitter.getNumEmitters() > 0)
	{
		CWinEventEmitter *we = NLMISC::safe_cast<CWinEventEmitter *>(driver->_EventEmitter.getEmitter(0));
		// Process the message by the emitter
		we->setHWnd((uint32)hWnd);
		we->processMessage ((uint32)hWnd, message, wParam, lParam);
	}
}

// ***************************************************************************

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
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
	// Register a window class
	WNDCLASS		wc;

	memset(&wc,0,sizeof(wc));
	wc.style			= CS_HREDRAW | CS_VREDRAW ;//| CS_DBLCLKS;
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
		ULONG WndFlags=(WS_OVERLAPPEDWINDOW+WS_CLIPCHILDREN+WS_CLIPSIBLINGS)&~WS_VISIBLE;

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
		// _NbTextureStages = caps.MaxSimultaneousTextures;
	}
	else
	{
		_TextureCubeSupported = false;
		_NbNeLTextureStages = 1;
		_MADOperatorSupported = false;
		// _NbTextureStages = 1;
	}
#ifdef NL_FORCE_TEXTURE_STAGE_COUNT
	_NbNeLTextureStages = min ((uint)NL_FORCE_TEXTURE_STAGE_COUNT, (uint)IDRV_MAT_MAXTEXTURES);
	// _NbTextureStages = NL_FORCE_TEXTURE_STAGE_COUNT;
#endif // NL_FORCE_TEXTURE_STAGE_COUNT

	_VertexProgram = !_DisableHardwareVertexProgram && ((caps.VertexShaderVersion&0xffff) >= 0x0100);
#ifdef NL_DISABLE_HARDWARE_PIXEL_SHADER
	_PixelShader = false;
#else // NL_DISABLE_HARDWARE_PIXEL_SHADER
	_PixelShader = (caps.PixelShaderVersion&0xffff) >= 0x0101;
#endif // NL_DISABLE_HARDWARE_PIXEL_SHADER
	_MaxVerticesByVertexBufferHard = caps.MaxVertexIndex;

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

	/* todo hulud beginscene
	// Begin now
	if (_DeviceInterface->BeginScene() != D3D_OK)
	{
		nlwarning ("CDriverD3D::setDisplay: BeginScene fails.");
		release();
		return false;
	}*/

	// Reset render state cache
	resetRenderVariables();

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
	_AllocatedTextureMemory = 0;
	_FogEnabled = false;

	// No back buffer backuped for the moment
	_BackBuffer = NULL;

	// Done
	return true;
}

// ***************************************************************************
extern uint indexCount;
extern uint textureCount;
extern uint vertexCount;

bool CDriverD3D::release() 
{
	// Call IDriver::release() before, to destroy textures, shaders and VBs...
	IDriver::release();

	// Back buffer ref
	if (_BackBuffer)
		_BackBuffer->Release();

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
	nlassert (textureCount == 0);
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
	width = _CurrentMode.Width;
	height = _CurrentMode.Height;
}

// ***************************************************************************

void CDriverD3D::getWindowPos (uint32 &x, uint32 &y)
{
	x = _WindowX;
	y = _WindowY;
}

// ***************************************************************************

uint32 CDriverD3D::getImplementationVersion () const
{
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
	nlassert (_DeviceInterface);

	// Backup viewport
	CViewport oldViewport = _Viewport;
	setupViewport (CViewport());
	updateRenderVariables ();

	bool result = _DeviceInterface->Clear( 0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0,0,0,0), zval, 0 ) == D3D_OK;

	// Restaure the old viewport
	setupViewport (oldViewport);
	return result;
}

// ***************************************************************************

void CDriverD3D::setColorMask (bool bRed, bool bGreen, bool bBlue, bool bAlpha) 
{
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
	nlassert (_DeviceInterface);

	// Is direct input running ?
	if (_EventEmitter.getNumEmitters() > 1) 
	{
		// flush direct input messages if any
		NLMISC::safe_cast<NLMISC::CDIEventEmitter *>(_EventEmitter.getEmitter(1))->poll();
	}

	// End now
	if (_DeviceInterface->EndScene() != D3D_OK)
		return false;

	if (_DeviceInterface->Present( NULL, NULL, NULL, NULL) != D3D_OK)
		return false;

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
		setMode (mode);
	}

	/* todo hulud beginscene
	// Begin now
	return _DeviceInterface->BeginScene() == D3D_OK; */
	begun = false;
	_DeviceInterface->SetVertexShader(NULL);

	return true;
};

// ***************************************************************************

void CDriverD3D::setPolygonMode (TPolygonMode mode)
{
	IDriver::setPolygonMode (mode);

	setRenderState (D3DRS_FILLMODE, mode==Point?D3DFILL_POINT:mode==Line?D3DFILL_WIREFRAME:D3DFILL_SOLID);
}

// ***************************************************************************

bool CDriverD3D::isTextureFormatOk(UINT adapter, D3DFORMAT TextureFormat, D3DFORMAT AdapterFormat)
{
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
	_ForceDXTCCompression = dxtcComp;
}

// ***************************************************************************

void CDriverD3D::forceTextureResize(uint divisor)
{
	clamp(divisor, 1U, 256U);

	// 16 -> 4.
	_ForceTextureResizePower= getPowerOf2(divisor);
}

// ***************************************************************************

bool CDriverD3D::fogEnabled()
{
	return _RenderStateCache[D3DRS_FOGENABLE].Value == TRUE;
}

// ***************************************************************************

void CDriverD3D::enableFog(bool enable)
{
	_FogEnabled = enable;
	setRenderState (D3DRS_FOGENABLE, enable?TRUE:FALSE);
}

// ***************************************************************************

void CDriverD3D::setupFog(float start, float end, CRGBA color)
{
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
	static const D3DFORMAT format[]=
	{
		D3DFMT_A8R8G8B8,
		D3DFMT_A1R5G5B5,
		D3DFMT_R5G6B5,
	};
	static const uint8 depth[]=
	{
		32,
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
	UINT adapter = (_Adapter==0xffffffff)?D3DADAPTER_DEFAULT:(UINT)_Adapter;
	D3DDISPLAYMODE mode;
	_D3D->GetAdapterDisplayMode(adapter, &mode);
	gfxMode.Windowed=false;
	gfxMode.Width=(uint16)mode.Width;
	gfxMode.Height=(uint16)mode.Height;
	// \todo yoyo may be false, but don't care
	gfxMode.Depth= mode.Format==D3DFMT_A8R8G8B8?32:16;
	gfxMode.Frequency=(uint8)mode.RefreshRate;

	return true;
}

// ***************************************************************************

uint CDriverD3D::getNumAdapter() const
{
	if (_D3D)
		return _D3D->GetAdapterCount();
	else
		return 0;
}

// ***************************************************************************

bool CDriverD3D::getAdapter(uint adapter, IDriver::CAdapter &desc) const
{
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

uint32 CDriverD3D::getAvailableVertexAGPMemory ()
{
	if (_DeviceInterface)
	{
		/* todo hulud remove 
		uint allocated = 0;		
		uint size = 1024*1024;
		std::vector<IDirect3DVertexBuffer9*> vertexBuffers;
		while (size)
		{
			IDirect3DVertexBuffer9 *vb;
			if (_DeviceInterface->CreateVertexBuffer (size, D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC, 0, 
				D3DPOOL_DEFAULT, &vb, NULL) == D3D_OK)
			{
				allocated += size;
				vertexBuffers.push_back (vb);
			}
			else
			{
				size /= 2;
			}
		}

		// Free all
		while (!vertexBuffers.empty())
		{
			IDirect3DVertexBuffer9 *vb = vertexBuffers.back();
			vertexBuffers.pop_back();
			vb->Release();
		}*/
		return 1024*1024*64;
		//return allocated;
	}
	return 0;
}

// ***************************************************************************

uint32 CDriverD3D::getAvailableVertexVRAMMemory ()
{
	if (_DeviceInterface)
	{
		uint allocated = 0;		
		uint size = 1024*1024;
		std::vector<IDirect3DVertexBuffer9*> vertexBuffers;
		while (size)
		{
			IDirect3DVertexBuffer9 *vb;
			if (_DeviceInterface->CreateVertexBuffer (size, D3DUSAGE_WRITEONLY, 0, 
				D3DPOOL_DEFAULT, &vb, NULL) == D3D_OK)
			{
				allocated += size;
				vertexBuffers.push_back (vb);
			}
			else
			{
				size /= 2;
			}
		}

		// Free all
		while (!vertexBuffers.empty())
		{
			IDirect3DVertexBuffer9 *vb = vertexBuffers.back();
			vertexBuffers.pop_back();
			vb->Release();
		}
		return allocated;
	}
	return 0;
}

// ***************************************************************************
bool CDriverD3D::supportMADOperator() const
{
	if (_DeviceInterface)
	{
		return _MADOperatorSupported;
	}
	return false; // don't know..
}


bool CDriverD3D::setMode (const GfxMode& mode)
{
	// Mode as change ?
	if ((mode.Depth != _CurrentMode.Depth) ||
		(mode.Width != _CurrentMode.Width) ||
		(mode.Height != _CurrentMode.Height) ||
		(mode.OffScreen != _CurrentMode.OffScreen) ||
		(mode.Windowed != _CurrentMode.Windowed) ||
		(mode.Frequency != _CurrentMode.Frequency))
	{
		// Current mode
		_CurrentMode = mode;

		// Restaure non managed vertex buffer in system memory
		while (!_VBDrvInfos.empty())
		{
			CVBDrvInfosD3D *vertexBuffer = static_cast<CVBDrvInfosD3D*>(_VBDrvInfos.back());
			restaureVertexBuffer (*vertexBuffer);
			delete vertexBuffer;
		}

		// Restaure non managed index buffer in system memory
		while (!_IBDrvInfos.empty())
		{
			CIBDrvInfosD3D *indexBuffer = static_cast<CIBDrvInfosD3D*>(_IBDrvInfos.back());
			restaureIndexBuffer (*indexBuffer);
			delete indexBuffer;
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

		// Make the reset

		D3DPRESENT_PARAMETERS parameters;
		D3DFORMAT adapterFormat;
		if (!fillPresentParameter (parameters, adapterFormat, mode, adapter, adapterMode))
			return false;
		if (_DeviceInterface->Reset (&parameters) != D3D_OK)
			return false;

		// Reset internal caches
		resetRenderVariables();
	}

	return true;
}

// ***************************************************************************

bool CDriverD3D::fillPresentParameter (D3DPRESENT_PARAMETERS &parameters, D3DFORMAT &adapterFormat, const GfxMode& mode, UINT adapter, const D3DDISPLAYMODE &adapterMode)
{
	memset (&parameters, 0, sizeof(D3DPRESENT_PARAMETERS));
	parameters.BackBufferWidth = mode.Width;
	parameters.BackBufferHeight = mode.Height;
	parameters.BackBufferFormat = (mode.Depth==16)?D3DFMT_R5G6B5:(mode.Depth==24)?D3DFMT_R8G8B8:D3DFMT_A8R8G8B8;
	parameters.BackBufferCount = 1;
	parameters.MultiSampleType = D3DMULTISAMPLE_NONE;
	parameters.MultiSampleQuality = 0;
	parameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	parameters.hDeviceWindow = NULL;
	parameters.Windowed = mode.Windowed;
	parameters.EnableAutoDepthStencil = TRUE;
	parameters.FullScreen_RefreshRateInHz = mode.Windowed?0:mode.Frequency;
	parameters.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

	adapterFormat = mode.Windowed?adapterMode.Format:parameters.BackBufferFormat;

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

} // NL3D