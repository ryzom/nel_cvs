/** \file driver_opengl.cpp
 * OpenGL driver implementation
 *
 * $Id: driver_opengl.cpp,v 1.193 2003/09/12 15:42:04 lecroart Exp $
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

#include "stdopengl.h"

#ifdef NL_OS_WINDOWS

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <string>


#else // NL_OS_UNIX

#include <GL/glx.h>

#endif // NL_OS_UNIX

#include <vector>
#include <GL/gl.h>

#include "nel/3d/viewport.h"
#include "nel/3d/scissor.h"
#include "nel/3d/u_driver.h"
#include "3d/vertex_buffer.h"
#include "3d/light.h"
#include "3d/primitive_block.h"
#include "nel/misc/rect.h"
#include "nel/misc/di_event_emitter.h"
#include "nel/misc/mouse_device.h"
#include "driver_opengl_vertex_buffer_hard.h"


using namespace std;
using namespace NLMISC;






// ***************************************************************************
// try to allocate 16Mo by default of AGP Ram.
#define	NL3D_DRV_VERTEXARRAY_AGP_INIT_SIZE		(16384*1024)



// ***************************************************************************
#ifdef NL_OS_WINDOWS
// dllmain::
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		// Yoyo: Vianney change: don't need to call initDebug() anymore.
		// initDebug();
	}
	return true;
}
#endif


namespace NL3D
{

#ifdef NL_OS_WINDOWS
uint CDriverGL::_Registered=0;
#endif // NL_OS_WINDOWS

// Version of the driver. Not the interface version!! Increment when implementation of the driver change.
const uint32		CDriverGL::ReleaseVersion = 0xa;

// Number of register to allocate for the EXTVertexShader extension
const uint CDriverGL::_EVSNumConstant = 97;

#ifdef NL_OS_WINDOWS

__declspec(dllexport) IDriver* NL3D_createIDriverInstance ()
{
	return new CDriverGL;
}

__declspec(dllexport) uint32 NL3D_interfaceVersion ()
{
	return IDriver::InterfaceVersion;
}

static void GlWndProc(CDriverGL *driver, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if(message == WM_SIZE)
	{
		if (driver != NULL)
		{
			RECT rect;
			GetClientRect (driver->_hWnd, &rect);

			// Setup gl viewport
			driver->_WindowWidth = rect.right-rect.left;
			driver->_WindowHeight = rect.bottom-rect.top;
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

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Get the driver pointer..
	CDriverGL *pDriver=(CDriverGL*)GetWindowLong (hWnd, GWL_USERDATA);
	if (pDriver != NULL)
	{
		GlWndProc (pDriver, hWnd, message, wParam, lParam);
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

#elif defined (NL_OS_UNIX)

extern "C"
{
IDriver* NL3D_createIDriverInstance ()
{
	return new CDriverGL;
}

uint32 NL3D_interfaceVersion ()
{
	return IDriver::InterfaceVersion;
}
}
/*
static Bool WndProc(Display *d, XEvent *e, char *arg)
{
  nlinfo("glop %d %d", e->type, e->xmap.window);
  CDriverGL *pDriver = (CDriverGL*)arg;
  if (pDriver != NULL)
    {
      // Process the message by the emitter
      pDriver->_EventEmitter.processMessage();
    }
  // TODO i'don t know what to return exactly
  return (e->type == MapNotify) && (e->xmap.window == (Window) arg);
}
*/
#endif // NL_OS_UNIX


// ***************************************************************************
CDriverGL::CDriverGL()
{	
	_OffScreen = false;

#ifdef NL_OS_WINDOWS
	_PBuffer = NULL;
	_hWnd = NULL;
	_hRC = NULL;
	_hDC = NULL;
	_NeedToRestaureGammaRamp = false;
#elif defined (NL_OS_UNIX) // NL_OS_WINDOWS

	cursor = None;
	
#ifdef XF86VIDMODE
	// zero the old screen mode
	memset(&_OldScreenMode, 0, sizeof(_OldScreenMode));
#endif //XF86VIDMODE

#endif // NL_OS_UNIX

	_FullScreen= false;

	_CurrentMaterial=NULL;
	_Initialized = false;

	_FogEnabled= false;
	_FogEnd = _FogStart = 0.f;
	_CurrentFogColor[0]= 0;
	_CurrentFogColor[1]= 0;
	_CurrentFogColor[2]= 0;
	_CurrentFogColor[3]= 0;


	_LightSetupDirty= false;
	_ModelViewMatrixDirty= false;
	_RenderSetupDirty= false;
	// All lights default pos.
	uint i;
	for(i=0;i<MaxLight;i++)
		_LightDirty[i]= false;

	

	_CurrentGlNormalize= false;
	_ForceNormalize= false;

	_AGPVertexArrayRange= NULL;
	_VRAMVertexArrayRange= NULL;
	_CurrentVertexArrayRange= NULL;
	_CurrentVertexBufferHard= NULL;
	_NVCurrentVARPtr= NULL;
	_NVCurrentVARSize= 0;
	_SupportVBHard= false;
	_SlowUnlockVBHard= false;
	_MaxVerticesByVBHard= 0;

	_AllocatedTextureMemory= 0;

	_ForceDXTCCompression= false;
	_ForceTextureResizePower= 0;

	_SumTextureMemoryUsed = false;

	_NVTextureShaderEnabled = false;


	// Compute the Flag which say if one texture has been changed in CMaterial.
	_MaterialAllTextureTouchedFlag= 0;
	for(i=0; i < IDRV_MAT_MAXTEXTURES; i++)
	{
		_MaterialAllTextureTouchedFlag|= IDRV_TOUCHED_TEX[i];
		_CurrentTexAddrMode[i] = GL_NONE;
	}


	_UserTexMatEnabled = 0;
	
	// Ligtmap preca.
	_LastVertexSetupIsLightMap= false;
	for(i=0; i < IDRV_MAT_MAXTEXTURES; i++)
		_LightMapUVMap[i]= -1;
	// reserve enough space to never reallocate, nor test for reallocation.
	_LightMapLUT.resize(NL3D_DRV_MAX_LIGHTMAP);
	// must set replace for alpha part.
	_LightMapLastStageEnv.Env.OpAlpha= CMaterial::Replace;
	_LightMapLastStageEnv.Env.SrcArg0Alpha= CMaterial::Texture;
	_LightMapLastStageEnv.Env.OpArg0Alpha= CMaterial::SrcAlpha;

	_ProjMatDirty = true;

	std::fill(_StageSupportEMBM, _StageSupportEMBM + IDRV_MAT_MAXTEXTURES, false);

	ATIWaterShaderHandleNoDiffuseMap = 0;
	ATIWaterShaderHandle = 0;

	std::fill(ARBWaterShader, ARBWaterShader + 4, 0);

///	buildCausticCubeMapTex();

	_SpecularBatchOn= false;

	_PolygonSmooth= false;

	_VBHardProfiling= false;
	_CurVBHardLockCount= 0;
	_NumVBHardProfileFrame= 0;
}


// ***************************************************************************
CDriverGL::~CDriverGL()
{
	release();
}

// ***************************************************************************
bool CDriverGL::init (uint windowIcon)
{
#ifdef WIN32
	WNDCLASS		wc;

	if (!_Registered)
	{
		memset(&wc,0,sizeof(wc));
		wc.style			= CS_HREDRAW | CS_VREDRAW ;//| CS_DBLCLKS;
		wc.lpfnWndProc		= (WNDPROC)WndProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= GetModuleHandle(NULL);
		wc.hIcon			= (HICON)windowIcon;
		wc.hCursor			= LoadCursor(NULL,IDC_ARROW);
		wc.hbrBackground	= WHITE_BRUSH;
		wc.lpszClassName	= "NLClass";
		wc.lpszMenuName		= NULL;
		if ( !RegisterClass(&wc) ) 
		{
			return false;
		}
		_Registered=1;
	}

	// Backup monitor color parameters
	HDC dc = CreateDC ("DISPLAY", NULL, NULL, NULL);
	if (dc)
	{
		_NeedToRestaureGammaRamp = GetDeviceGammaRamp (dc, _GammaRampBackuped) != FALSE;

		// Release the DC
		ReleaseDC (NULL, dc);
	}
	else
	{
		nlwarning ("(CDriverGL::init): can't create DC");
	}

#endif
	return true;
}

// --------------------------------------------------

ModeList CDriverGL::enumModes()
{
	ModeList	ML;
#ifdef NL_OS_WINDOWS
	DEVMODE		devmode;
	sint		n;
	GfxMode		Mode;

	n=0;
	while( EnumDisplaySettings(NULL, n, &devmode) )
	{
		Mode.Windowed=false;
		Mode.Width=(uint16)devmode.dmPelsWidth;
		Mode.Height=(uint16)devmode.dmPelsHeight;	
		Mode.Depth=(uint8)devmode.dmBitsPerPel;	
		ML.push_back(Mode);
		n++;	
	}
#endif // NL_OS_WINDOWS
	return ML;
}

// --------------------------------------------------

void CDriverGL::disableHardwareVertexProgram()
{
	_Extensions.DisableHardwareVertexProgram= true;
}

void CDriverGL::disableHardwareVertexArrayAGP()
{
	_Extensions.DisableHardwareVertexArrayAGP= true;
}

void CDriverGL::disableHardwareTextureShader()
{
	_Extensions.DisableHardwareTextureShader= true;
}

// --------------------------------------------------

bool CDriverGL::setDisplay(void *wnd, const GfxMode &mode, bool show) throw(EBadDisplay)
{
	
	uint width = mode.Width;
	uint height = mode.Height;


#ifdef NL_OS_WINDOWS
	
	// Driver caps.
	//=============
	// Retrieve the WGL extensions before init the driver.
	int						pf;

	_OffScreen = mode.OffScreen;

	// Init pointers
	_PBuffer = NULL;
	_hWnd = NULL;
	_WindowWidth = _WindowHeight = _WindowX = _WindowY = 0;
	_hRC = NULL;
	_hDC = NULL;

	// Offscreen mode ?
	if (_OffScreen)
	{
		// Get a hdc

		ULONG WndFlags=WS_OVERLAPPEDWINDOW+WS_CLIPCHILDREN+WS_CLIPSIBLINGS;
		WndFlags&=~WS_VISIBLE;
		RECT	WndRect;
		WndRect.left=0;
		WndRect.top=0;
		WndRect.right=width;
		WndRect.bottom=height;
		AdjustWindowRect(&WndRect,WndFlags,FALSE);
		HWND tmpHWND = CreateWindow(	"NLClass",
									"",
									WndFlags,
									CW_USEDEFAULT,CW_USEDEFAULT,
									WndRect.right,WndRect.bottom,
									NULL,
									NULL,
									GetModuleHandle(NULL),
									NULL);
		if (!tmpHWND) 
		{
			nlwarning ("CDriverGL::setDisplay: CreateWindow failed");
			return false;
		}

		// resize the window
		RECT rc;
		SetRect (&rc, 0, 0, width, height);
		_WindowWidth = width;
		_WindowHeight = height;
		AdjustWindowRectEx (&rc, GetWindowStyle (_hWnd), GetMenu (_hWnd) != NULL, GetWindowExStyle (_hWnd));
		SetWindowPos (_hWnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
		
		// Get the 
		HDC tempHDC = GetDC(tmpHWND);

		_Depth=GetDeviceCaps(tempHDC,BITSPIXEL);

		// ---
		memset(&_pfd,0,sizeof(_pfd));
		_pfd.nSize        = sizeof(_pfd);
		_pfd.nVersion     = 1;
		_pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		_pfd.iPixelType   = PFD_TYPE_RGBA;
		_pfd.cColorBits   = (char)_Depth;

		// Choose best suited Depth Buffer.
		if(_Depth<=16)
		{
			_pfd.cDepthBits   = 16;
		}
		else
		{
			_pfd.cDepthBits = 24;
			_pfd.cAlphaBits	= 8;
		}
		_pfd.iLayerType	  = PFD_MAIN_PLANE;
		pf=ChoosePixelFormat(tempHDC,&_pfd);
		if (!pf) 
		{
			nlwarning ("CDriverGL::setDisplay: ChoosePixelFormat failed");
			DestroyWindow (tmpHWND);
			return false;
		} 
		if ( !SetPixelFormat(tempHDC,pf,&_pfd) ) 
		{
			nlwarning ("CDriverGL::setDisplay: SetPixelFormat failed");
			DestroyWindow (tmpHWND);
			return false;
		} 

		// Create gl context
		HGLRC tempGLRC = wglCreateContext(tempHDC);
		if (tempGLRC == NULL)
		{
			DWORD error = GetLastError ();
			nlwarning ("CDriverGL::setDisplay: wglCreateContext failed: 0x%x", error);
			DestroyWindow (tmpHWND);
			_PBuffer = NULL;
			_hWnd = NULL;
			_hRC = NULL;
			_hDC = NULL;
			return false;
		}

		// Make the context current
		if (!wglMakeCurrent(tempHDC,tempGLRC))
		{
			DWORD error = GetLastError ();
			nlwarning ("CDriverGL::setDisplay: wglMakeCurrent failed: 0x%x", error);
			wglDeleteContext (tempGLRC);
			DestroyWindow (tmpHWND);
			_PBuffer = NULL;
			_hWnd = NULL;
			_hRC = NULL;
			_hDC = NULL;
			return false;
		}

		// Register WGL functions
		registerWGlExtensions (_Extensions, tempHDC);

		HDC hdc = wglGetCurrentDC ();
		if (hdc == NULL)
		{
			DWORD error = GetLastError ();
			nlwarning ("CDriverGL::setDisplay: wglGetCurrentDC failed: 0x%x", error);
			DestroyWindow (tmpHWND);
			_PBuffer = NULL;
			_hWnd = NULL;
			_hRC = NULL;
			_hDC = NULL;
			return false;
		}

		// Get ready to query for a suitable pixel format that meets our
		// minimum requirements.
		int iattributes[2*20];
		float fattributes[2*20];
		int nfattribs = 0;
		int niattribs = 0;

		// Attribute arrays must be “0” terminated – for simplicity, first
		// just zero-out the array then fill from left to right.
		for ( int a = 0; a < 2*20; a++ )
		{
			iattributes[a] = 0;
			fattributes[a] = 0;
		}
		
		// Since we are trying to create a pbuffer, the pixel format we
		// request (and subsequently use) must be “p-buffer capable”.
		iattributes[2*niattribs ] = WGL_DRAW_TO_PBUFFER_ARB;
		iattributes[2*niattribs+1] = true;
		niattribs++;
		
		// We require a minimum of 24-bit depth.
		iattributes[2*niattribs ] = WGL_DEPTH_BITS_ARB;
		iattributes[2*niattribs+1] = 24;
		niattribs++;
		
		// We require a minimum of 8-bits for each R, G, B, and A.
		iattributes[2*niattribs ] = WGL_RED_BITS_ARB;
		iattributes[2*niattribs+1] = 8;
		niattribs++;
		iattributes[2*niattribs ] = WGL_GREEN_BITS_ARB;
		iattributes[2*niattribs+1] = 8;
		niattribs++;
		iattributes[2*niattribs ] = WGL_BLUE_BITS_ARB;
		iattributes[2*niattribs+1] = 8;
		niattribs++;
		iattributes[2*niattribs ] = WGL_ALPHA_BITS_ARB;
		iattributes[2*niattribs+1] = 8;
		niattribs++;
		
		// Now obtain a list of pixel formats that meet these minimum
		// requirements.
		int pformat[20];
		unsigned int nformats;
		if ( !wglChoosePixelFormatARB ( hdc, iattributes, fattributes,
			20, pformat, &nformats ) )
		{
			nlwarning ( "pbuffer creation error: Couldn't find a suitable pixel format.\n" );
			wglDeleteContext (tempGLRC);
			DestroyWindow (tmpHWND);
			return false;
		}

		/* After determining a compatible pixel format, the next step is to create a pbuffer of the
			chosen format. Fortunately this step is fairly easy, as you merely select one of the formats
			returned in the list in step #2 and call the function: */
		int iattributes2[1] = {0};
		// int iattributes2[] = {WGL_PBUFFER_LARGEST_ARB, 1, 0};
		_PBuffer = wglCreatePbufferARB( hdc, pformat[0], width, height, iattributes2 );
		if (_PBuffer == NULL)
		{
			DWORD error = GetLastError ();
			nlwarning ("CDriverGL::setDisplay: wglCreatePbufferARB failed: 0x%x", error);
			wglDeleteContext (tempGLRC);
			DestroyWindow (tmpHWND);
			_PBuffer = NULL;
			_hWnd = NULL;
			_hRC = NULL;
			_hDC = NULL;
			return false;
		}

		/* After creating a pbuffer, you may use this functions to determine the dimensions of the pbuffer actually created. */
		if ( !wglQueryPbufferARB( _PBuffer, WGL_PBUFFER_WIDTH_ARB, (int*)&width ) )
		{
			DWORD error = GetLastError ();
			nlwarning ("CDriverGL::setDisplay: wglQueryPbufferARB failed: 0x%x", error);
			wglDeleteContext (tempGLRC);
			DestroyWindow (tmpHWND);
			_PBuffer = NULL;
			_hWnd = NULL;
			_hRC = NULL;
			_hDC = NULL;
			return false;
		}
		if ( !wglQueryPbufferARB( _PBuffer, WGL_PBUFFER_HEIGHT_ARB, (int*)&height ) )
		{
			DWORD error = GetLastError ();
			nlwarning ("CDriverGL::setDisplay: wglQueryPbufferARB failed: 0x%x", error);
			wglDeleteContext (tempGLRC);
			DestroyWindow (tmpHWND);
			_PBuffer = NULL;
			_hWnd = NULL;
			_hRC = NULL;
			_hDC = NULL;
			return false;
		}
		_WindowWidth = width;
		_WindowHeight = height;
		
		/* The next step is to create a device context for the newly created pbuffer. To do this,
			call the the function: */
		_hDC = wglGetPbufferDCARB( _PBuffer );
		if (_hDC == NULL)
		{
			DWORD error = GetLastError ();
			nlwarning ("CDriverGL::setDisplay: wglGetPbufferDCARB failed: 0x%x", error);
			wglDestroyPbufferARB( _PBuffer );
			wglDeleteContext (tempGLRC);
			DestroyWindow (tmpHWND);
			_PBuffer = NULL;
			_hWnd = NULL;
			_hRC = NULL;
			_hDC = NULL;
			return false;
		}


		/* The final step of pbuffer creation is to create an OpenGL rendering context and
			associate it with the handle for the pbuffer’s device context created in step #4. This is done as follows */
		_hRC = wglCreateContext( _hDC );
		if (_hRC == NULL)
		{
			DWORD error = GetLastError ();
			nlwarning ("CDriverGL::setDisplay: wglCreateContext failed: 0x%x", error);
			wglReleasePbufferDCARB( _PBuffer, _hDC );
			wglDestroyPbufferARB( _PBuffer );
			wglDeleteContext (tempGLRC);
			DestroyWindow (tmpHWND);
			_PBuffer = NULL;
			_hWnd = NULL;
			_hRC = NULL;
			_hDC = NULL;
			return false;
		}

		// Get the depth
		_Depth = GetDeviceCaps (_hDC, BITSPIXEL);

		// Destroy the temp gl context
		if (!wglDeleteContext (tempGLRC))
		{
			DWORD error = GetLastError ();
			nlwarning ("CDriverGL::setDisplay: wglDeleteContext failed: 0x%x", error);
		}

		// Destroy the temp windows
		if (!DestroyWindow (tmpHWND))
			nlwarning ("CDriverGL::setDisplay: DestroyWindow failed");

		/* After a pbuffer has been successfully created you can use it for off-screen rendering. To do
			so, you’ll first need to bind the pbuffer, or more precisely, make its GL rendering context
			the current context that will interpret all OpenGL commands and state changes. */
		if (!wglMakeCurrent(_hDC,_hRC))
		{
			DWORD error = GetLastError ();
			nlwarning ("CDriverGL::setDisplay: wglMakeCurrent failed: 0x%x", error);
			wglDeleteContext (_hRC);
			wglReleasePbufferDCARB( _PBuffer, _hDC );
			wglDestroyPbufferARB( _PBuffer );
			DestroyWindow (tmpHWND);
			_PBuffer = NULL;
			_hWnd = NULL;
			_hRC = NULL;
			_hDC = NULL;
			return false;
		}
 	}
	else
	{
		_FullScreen= false;
		if (wnd)
		{
			_hWnd=(HWND)wnd;
			_DestroyWindow=false;
		}
		else
		{
			ULONG	WndFlags;
			RECT	WndRect;

			// Must destroy this window
			_DestroyWindow=true;

			if(mode.Windowed)
				WndFlags=WS_OVERLAPPEDWINDOW+WS_CLIPCHILDREN+WS_CLIPSIBLINGS;
			else
			{
				WndFlags=WS_POPUP;

				_FullScreen= true;
				DEVMODE		devMode;
				_OldScreenMode.dmSize= sizeof(DEVMODE);
				_OldScreenMode.dmDriverExtra= 0;
				EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &_OldScreenMode);
				_OldScreenMode.dmFields= DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY ;

				devMode.dmSize= sizeof(DEVMODE);
				devMode.dmDriverExtra= 0;
				devMode.dmFields= DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;
				devMode.dmPelsWidth= width;
				devMode.dmPelsHeight= height;
				devMode.dmBitsPerPel= mode.Depth;
				devMode.dmDisplayFrequency= mode.Frequency;
				if (ChangeDisplaySettings(&devMode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
					return false;
			}
			WndRect.left=0;
			WndRect.top=0;
			WndRect.right=width;
			WndRect.bottom=height;
			AdjustWindowRect(&WndRect,WndFlags,FALSE);
			_hWnd = CreateWindow(	"NLClass",
									"",
									WndFlags,
									CW_USEDEFAULT,CW_USEDEFAULT,
									WndRect.right,WndRect.bottom,
									NULL,
									NULL,
									GetModuleHandle(NULL),
									NULL);
			if (!_hWnd) 
			{
				return false;
			}

			SetWindowLong (_hWnd, GWL_USERDATA, (LONG)this);

			// resize the window
			RECT rc;
			SetRect (&rc, 0, 0, width, height);
			AdjustWindowRectEx (&rc, GetWindowStyle (_hWnd), GetMenu (_hWnd) != NULL, GetWindowExStyle (_hWnd));
			SetWindowPos (_hWnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );

			if (show || _FullScreen)
				ShowWindow(_hWnd,SW_SHOW);
		}

		// Init Window Width and Height
		RECT clientRect;
		GetClientRect (_hWnd, &clientRect);
		_WindowWidth = clientRect.right-clientRect.left;
		_WindowHeight = clientRect.bottom-clientRect.top;
		_WindowX = clientRect.left;
		_WindowY = clientRect.top;

		_hDC=GetDC(_hWnd);
		wglMakeCurrent(_hDC,NULL);
		_Depth=GetDeviceCaps(_hDC,BITSPIXEL);
		// ---
		memset(&_pfd,0,sizeof(_pfd));
		_pfd.nSize        = sizeof(_pfd);
		_pfd.nVersion     = 1;
		_pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		_pfd.iPixelType   = PFD_TYPE_RGBA;
		_pfd.cColorBits   = (char)_Depth;
		// Choose best suited Depth Buffer.
		if(_Depth<=16)
		{
			_pfd.cDepthBits   = 16;
		}
		else
		{
			_pfd.cDepthBits = 24;
			_pfd.cAlphaBits	= 8;
		}
		_pfd.iLayerType	  = PFD_MAIN_PLANE;
		pf=ChoosePixelFormat(_hDC,&_pfd);
		if (!pf) 
		{
			return false;
		}

		if ( !SetPixelFormat(_hDC,pf,&_pfd) ) 
		{
			return false;
		} 
		_hRC=wglCreateContext(_hDC);
		wglMakeCurrent(_hDC,_hRC);
	}

	/// release old emitter
	while (_EventEmitter.getNumEmitters() != 0)
	{
		_EventEmitter.removeEmitter(_EventEmitter.getEmitter(_EventEmitter.getNumEmitters() - 1));
	}
	NLMISC::CWinEventEmitter *we = new NLMISC::CWinEventEmitter;
	// setup the event emitter, and try to retrieve a direct input interface
	_EventEmitter.addEmitter(we, true /*must delete*/); // the main emitter
	/// try to get direct input
	try
	{
		NLMISC::CDIEventEmitter *diee = NLMISC::CDIEventEmitter::create(GetModuleHandle(NULL), _hWnd, we);
		if (diee)
		{
			_EventEmitter.addEmitter(diee, true);
		}
	}
	catch(EDirectInput &e)
	{
		nlinfo(e.what());
	}

#elif defined(NL_OS_UNIX) // NL_OS_WINDOWS

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL)
        {
	  nlerror ("XOpenDisplay failed on '%s'",getenv("DISPLAY"));
	}
	else
	  {
	    nldebug("XOpenDisplay on '%s' OK", getenv("DISPLAY"));
	  }

	int sAttribList[] =
	{
	  GLX_RGBA,
	  GLX_DOUBLEBUFFER,
	  //GLX_BUFFER_SIZE, 16,
	  GLX_DEPTH_SIZE, 16,
	  GLX_RED_SIZE, 4,
	  GLX_GREEN_SIZE, 4,
	  GLX_BLUE_SIZE, 4,
	  //GLX_ALPHA_SIZE, 8,
	  None
	};
	/*
	int sAttribList[] =
	{
	  GLX_RGBA,
	  GLX_DOUBLEBUFFER,
	  //GLX_BUFFER_SIZE, 32,
	  GLX_DEPTH_SIZE, 32,
	  GLX_RED_SIZE, 8,
	  GLX_GREEN_SIZE, 8,
	  GLX_BLUE_SIZE, 8,
	  GLX_ALPHA_SIZE, 8,
	  None
	  };
	*/
	XVisualInfo *visual_info = glXChooseVisual (dpy, DefaultScreen(dpy), sAttribList);

	if(visual_info == NULL)
	  {
	    nlerror("glXChooseVisual() failed");
	  }
	else
	  {
	    nldebug("glXChooseVisual OK");
	  }

	ctx = glXCreateContext (dpy, visual_info, None, GL_TRUE);

	if(ctx == NULL)
	  {
	    nlerror("glXCreateContext() failed");
	  }
	else
	  {
	    nldebug("glXCreateContext() OK");
	  }

	Colormap cmap = XCreateColormap (dpy, RootWindow(dpy, DefaultScreen(dpy)), visual_info->visual, AllocNone);

	XSetWindowAttributes attr;
	attr.colormap = cmap;
	attr.background_pixel = BlackPixel(dpy, DefaultScreen(dpy));
	
#ifdef XF86VIDMODE
	// If we're going to attempt fullscreen, we need to set redirect to True,
	// This basically places the window with no borders in the top left 
	// corner of the screen.
	if (mode.Windowed)
	{
		attr.override_redirect = False;
	}
	else
	{
		attr.override_redirect = True;
	}
#else
	attr.override_redirect = False;
#endif

	int attr_flags = CWOverrideRedirect | CWColormap | CWBackPixel;

	win = XCreateWindow (dpy, RootWindow(dpy, DefaultScreen(dpy)), 0, 0, width, height, 0, visual_info->depth, InputOutput, visual_info->visual, attr_flags, &attr);	

	if(!win)
	  {
	    nlerror("XCreateWindow() failed");
	  }
	else
	  {
	    nldebug("XCreateWindow() OK");
	  }

	XSizeHints size_hints;
	size_hints.x = 0;
	size_hints.y = 0;
	size_hints.width = width;
	size_hints.height = height;
	size_hints.flags = PSize | PMinSize | PMaxSize;
	size_hints.min_width = width;
	size_hints.min_height = height;
	size_hints.max_width = width;
	size_hints.max_height = height;

	XTextProperty text_property;
	char *title="NeL window";
	XStringListToTextProperty(&title, 1, &text_property);

	XSetWMProperties (dpy, win, &text_property, &text_property,  0, 0, &size_hints, 0, 0);
	glXMakeCurrent (dpy, win, ctx);
	XMapRaised (dpy, win);

	XSelectInput (dpy, win,
		      KeyPressMask|
		      KeyReleaseMask|
		      ButtonPressMask|
		      ButtonReleaseMask|
		      PointerMotionMask
		      );

	XMapWindow(dpy, win);

	_EventEmitter.init (dpy, win);

//	XEvent event;
//	XIfEvent(dpy, &event, WaitForNotify, (char *)this);

#ifdef XF86VIDMODE
	if (!mode.Windowed)
	{

		// Set window to the right size, map it to the display, and raise it
		// to the front
		XResizeWindow(dpy,win,width,height);
		XMapRaised(dpy,win);
		XRaiseWindow(dpy, win);

		// grab the mouse and keyboard on the fullscreen window 
		if ((XGrabPointer(dpy, win, True, 0,
						  GrabModeAsync, GrabModeAsync,
						  win, None, CurrentTime) != GrabSuccess) ||
			(XGrabKeyboard(dpy, win, True,
						   GrabModeAsync, GrabModeAsync, CurrentTime) != 0) )
		{
			// Until I work out how to deal with this nicely, it just gives
			// an error and exits the prorgam.
			nlerror("Unable to grab keyboard and mouse\n");
		}
		else
		{
			// Save the old screen mode and dotclock
			memset(&_OldScreenMode, 0, sizeof(_OldScreenMode));
			XF86VidModeGetModeLine(dpy, 
								   DefaultScreen(dpy),
								   &_OldDotClock,
								   &_OldScreenMode);
			// Save the old viewport
			XF86VidModeGetViewPort(dpy, 
								   DefaultScreen(dpy),
								   &_OldX,
								   &_OldY);
    
			// get a list of modes, search for an appropriate one.
			XF86VidModeModeInfo **modes;
			int nmodes;
			if (XF86VidModeGetAllModeLines(dpy,
										   DefaultScreen(dpy),
										   &nmodes,&modes))
			{
				int mode_index = -1; // Gah, magic numbers all bad. 
				for (int i = 0; i < nmodes; i++)
				{
					nldebug("Available mode - %dx%d\n",width,height);
					if( (modes[i]->hdisplay == width) &&
						(modes[i]->vdisplay == height))
					{
						mode_index = i;
					}
				}
				// Switch to the mode
				if (mode_index != -1)
				{
					if(XF86VidModeSwitchToMode(dpy,
											   DefaultScreen(dpy), 
											   modes[mode_index]))
					{
						nlinfo("Switching to mode %dx%d,\n",width, 
							   height);
						XF86VidModeSetViewPort(dpy,DefaultScreen(dpy),0, 0);
						_FullScreen = true;
					}
				}
				else
				{
					// This is a problem, since we've nuked the border from 
					// window in the setup stage, until I work out how
					// to get it back (recreate window? seems excessive)
					nlerror("Couldn't find an appropriate mode %dx%d\n",
							width,
							height);
				}
			}
		}
	}

#endif // XF86VIDMODE

#endif // NL_OS_UNIX


	// Driver caps.
	//=============
	// Retrieve the extensions for the current context.
	NL3D::registerGlExtensions (_Extensions);	
	//
#ifdef NL_OS_WINDOWS
	NL3D::registerWGlExtensions (_Extensions, _hDC);
#endif // ifdef NL_OS_WINDOWS

	// Check required extensions!!
	// ARBMultiTexture is a opengl 1.2 required extension.
	if(!_Extensions.ARBMultiTexture)
	{
		nlwarning("Missing Required GL extension: GL_ARB_multitexture. Update your driver");
		throw EBadDisplay("Missing Required GL extension: GL_ARB_multitexture. Update your driver");
	}
	if(!_Extensions.EXTTextureEnvCombine)
	{
		nlwarning("Missing Important GL extension: GL_EXT_texture_env_combine => All envcombine are setup to GL_MODULATE!!!");
	}


	// init _DriverGLStates
	_DriverGLStates.init(_Extensions.ARBTextureCubeMap);


	// Init OpenGL/Driver defaults.
	//=============================
	glViewport(0,0,width,height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,width,height,0,-1.0f,1.0f);	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_AUTO_NORMAL);
	glDisable(GL_COLOR_MATERIAL);
	glEnable(GL_DITHER);
	glDisable(GL_FOG);
	glDisable(GL_LINE_SMOOTH);
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_NORMALIZE);
	_CurrViewport.init(0.f, 0.f, 1.f, 1.f);
	_CurrentGlNormalize= false;
	_ForceNormalize= false;
	// Setup defaults for blend, lighting ...
	_DriverGLStates.forceDefaults(inlGetNumTextStages());
	// Default delta camera pos.
	_PZBCameraPos= CVector::Null;

	if (_NVTextureShaderEnabled)
	{
		enableNVTextureShader(false);		
	}

	// Be always in EXTSeparateSpecularColor.	
	if(_Extensions.EXTSeparateSpecularColor)
	{
		glLightModeli((GLenum)GL_LIGHT_MODEL_COLOR_CONTROL_EXT, GL_SEPARATE_SPECULAR_COLOR_EXT);
	}

	_VertexProgramEnabled= false;
	_LastSetupGLArrayVertexProgram= false;


	// Init VertexArrayRange according to supported extenstion.
	_SupportVBHard= false;
	_SlowUnlockVBHard= false;
	_MaxVerticesByVBHard= 0;
	// Try with NVidia ext first.
	if(_Extensions.NVVertexArrayRange)
	{
		_AGPVertexArrayRange= new CVertexArrayRangeNVidia(this);
		_VRAMVertexArrayRange= new CVertexArrayRangeNVidia(this);
		_SupportVBHard= true;
		_MaxVerticesByVBHard= _Extensions.NVVertexArrayRangeMaxVertex;
	}
	// Else, try with ATI ext
	else if(_Extensions.ATIVertexArrayObject)
	{
		if (!_Extensions.ATIMapObjectBuffer)
		{		
			_AGPVertexArrayRange= new CVertexArrayRangeATI(this);
			_VRAMVertexArrayRange= new CVertexArrayRangeATI(this);			
			// BAD ATI extension scheme.
			_SlowUnlockVBHard= true;		
		}
		else
		{
			_AGPVertexArrayRange= new CVertexArrayRangeMapObjectATI(this);
			_VRAMVertexArrayRange= new CVertexArrayRangeMapObjectATI(this);			
		}
		_SupportVBHard= true;
		_MaxVerticesByVBHard= 65535; // should always work with recent drivers.		
	}

	// Reset VertexArrayRange.
	_CurrentVertexArrayRange= NULL;
	_CurrentVertexBufferHard= NULL;
	_NVCurrentVARPtr= NULL;
	_NVCurrentVARSize= 0;
	if(_SupportVBHard)
	{
		// try to allocate 16Mo by default of AGP Ram.
		initVertexArrayRange(NL3D_DRV_VERTEXARRAY_AGP_INIT_SIZE, 0);

		// If not success to allocate at least a minimum space in AGP, then disable completely VBHard feature
		if( _AGPVertexArrayRange->sizeAllocated()==0 )
		{
			// reset any allocated VRAM space.
			resetVertexArrayRange();

			// delete containers
			delete _AGPVertexArrayRange;
			delete _VRAMVertexArrayRange;
			_AGPVertexArrayRange= NULL;
			_VRAMVertexArrayRange= NULL;

			// disable.
			_SupportVBHard= false;
			_SlowUnlockVBHard= false;
			_MaxVerticesByVBHard= 0;
		}
	}

	// Init embm if present
	//===========================================================
	initEMBM();

	// Init fragment shaders if present
	//===========================================================
	initFragmentShaders();



	// Activate the default texture environnments for all stages.
	//===========================================================
	for(sint stage=0;stage<inlGetNumTextStages(); stage++)
	{
		// init no texture.
		_CurrentTexture[stage]= NULL;
		_CurrentTextureInfoGL[stage]= NULL;
		// texture are disabled in DriverGLStates.forceDefaults().
		
		// init default env.
		CMaterial::CTexEnv	env;	// envmode init to default.
		env.ConstantColor.set(255,255,255,255);
		forceActivateTexEnvMode(stage, env);
		forceActivateTexEnvColor(stage, env);

		// Not special TexEnv.
		_CurrentTexEnvSpecial[stage]= TexEnvSpecialDisabled;
				
		// set All TexGen by default to identity matrix (prefer use the textureMatrix scheme)
		_DriverGLStates.activeTextureARB(stage);
		GLfloat		params[4];
		params[0]=1; params[1]=0; params[2]=0; params[3]=0;
		glTexGenfv(GL_S, GL_OBJECT_PLANE, params);
		glTexGenfv(GL_S, GL_EYE_PLANE, params);
		params[0]=0; params[1]=1; params[2]=0; params[3]=0;
		glTexGenfv(GL_T, GL_OBJECT_PLANE, params);
		glTexGenfv(GL_T, GL_EYE_PLANE, params);
		params[0]=0; params[1]=0; params[2]=1; params[3]=0;
		glTexGenfv(GL_R, GL_OBJECT_PLANE, params);
		glTexGenfv(GL_R, GL_EYE_PLANE, params);
		params[0]=0; params[1]=0; params[2]=0; params[3]=1;
		glTexGenfv(GL_Q, GL_OBJECT_PLANE, params);
		glTexGenfv(GL_Q, GL_EYE_PLANE, params);
	}
	resetTextureShaders();

	// Get num of light for this driver
	int numLight;
	glGetIntegerv (GL_MAX_LIGHTS, &numLight);
	_MaxDriverLight=(uint)numLight;
	if (_MaxDriverLight>MaxLight)
		_MaxDriverLight=MaxLight;

	// Reset the lights position flags
	for (uint i=0; i<MaxLight; i++)
		_LightEnable[i]=false;


	_PPLExponent = 1.f;
	_PPLightDiffuseColor = NLMISC::CRGBA::White;
	_PPLightSpecularColor = NLMISC::CRGBA::White;

	// Backward compatibility: default lighting is Light0 default openGL
	// meaning that light direction is always (0,1,0) in eye-space
	// use enableLighting(0....), to get normal behaviour
	glEnable(GL_LIGHT0);

	_Initialized = true;

	_ForceDXTCCompression= false;
	_ForceTextureResizePower= 0;

	// Reset profiling.
	_AllocatedTextureMemory= 0;
	_TextureUsed.clear();
	_PrimitiveProfileIn.reset();
	_PrimitiveProfileOut.reset();
	_NbSetupMaterialCall= 0;
	_NbSetupModelMatrixCall= 0;

	// check wether per pixel lighting shader is supported
	checkForPerPixelLightingSupport();

	// if EXTVertexShader is used, bind  the standard GL arrays, and allocate constant
	if (!_Extensions.NVVertexProgram && _Extensions.EXTVertexShader)
	{
			_EVSPositionHandle = nglBindParameterEXT(GL_CURRENT_VERTEX_EXT);
			_EVSNormalHandle   = nglBindParameterEXT(GL_CURRENT_NORMAL);
			_EVSColorHandle    = nglBindParameterEXT(GL_CURRENT_COLOR);
			if (!_EVSPositionHandle || !_EVSNormalHandle || !_EVSColorHandle)
			{
				nlwarning("Unable to bind input parameters for use with EXT_vertex_shader, vertex program support is disabled");
				_Extensions.EXTVertexShader = false;
			}
			else
			{			
				// bind texture units
				for(uint k = 0; k < 8; ++k)
				{				
					_EVSTexHandle[k] = nglBindTextureUnitParameterEXT(GL_TEXTURE0_ARB + k, GL_CURRENT_TEXTURE_COORDS);
				}
				// Other attributes are managed using variant pointers :
				// Secondary color
				// Fog Coords
				// Skin Weight
				// Skin palette
				// This mean that they must have 4 components

				// Allocate invariants. One assitionnal variant is needed for fog coordinate
				_EVSConstantHandle = nglGenSymbolsEXT(GL_VECTOR_EXT, GL_INVARIANT_EXT, GL_FULL_RANGE_EXT, _EVSNumConstant + 1);

				if (_EVSConstantHandle == 0)
				{
					nlwarning("Unable to allocate constants for EXT_vertex_shader, vertex program support is disabled");
					_Extensions.EXTVertexShader = false;
				}
			}
	}

	return true;
}

#ifdef NL_OS_WINDOWS
// --------------------------------------------------
// This code comes from MFC
static void modifyStyle (HWND hWnd, int nStyleOffset, DWORD dwRemove, DWORD dwAdd)
{
	DWORD dwStyle = ::GetWindowLong(hWnd, nStyleOffset);
	DWORD dwNewStyle = (dwStyle & ~dwRemove) | dwAdd;
	if (dwStyle == dwNewStyle)
		return;

	::SetWindowLong(hWnd, nStyleOffset, dwNewStyle);
}
#endif

// --------------------------------------------------
bool CDriverGL::setMode(const GfxMode& mode)
{
#ifdef NL_OS_WINDOWS
	if (mode.Windowed)
	{
		if (_FullScreen)
		{
			ChangeDisplaySettings (NULL,0);
			modifyStyle(_hWnd, GWL_STYLE, WS_POPUP, WS_OVERLAPPEDWINDOW+WS_CLIPCHILDREN+WS_CLIPSIBLINGS);
		}
		_WindowWidth  = mode.Width;
		_WindowHeight = mode.Height;
	}
	else
	{
		_WindowWidth  = mode.Width;
		_WindowHeight = mode.Height;

		DEVMODE		devMode;
		if (!_FullScreen)
		{
			_OldScreenMode.dmSize= sizeof(DEVMODE);
			_OldScreenMode.dmDriverExtra= 0;
			EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &_OldScreenMode);
			_OldScreenMode.dmFields= DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY ;
		}

		devMode.dmSize= sizeof(DEVMODE);
		devMode.dmDriverExtra= 0;
		devMode.dmFields= DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY ;
		devMode.dmPelsWidth  = _WindowWidth;
		devMode.dmPelsHeight = _WindowHeight;
		devMode.dmBitsPerPel = _Depth;
		devMode.dmDisplayFrequency = mode.Frequency;
		if (ChangeDisplaySettings(&devMode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
			return false;
		
		if (!_FullScreen)
		{
			modifyStyle(_hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW+WS_CLIPCHILDREN+WS_CLIPSIBLINGS, WS_POPUP);
		}
#endif
	}
	_FullScreen = !mode.Windowed;

#ifdef NL_OS_WINDOWS
	// Resize the window
	RECT rc;
	SetRect (&rc, 0, 0, _WindowWidth, _WindowHeight);
	AdjustWindowRectEx (&rc, GetWindowStyle (_hWnd), false, GetWindowExStyle (_hWnd));
	SetWindowPos (_hWnd, NULL, 0, 0, rc.right-rc.left, rc.bottom-rc.top, SWP_NOZORDER | SWP_NOACTIVATE );

	ShowWindow(_hWnd, SW_SHOW);

	// Init Window Width and Height
	RECT clientRect;
	GetClientRect (_hWnd, &clientRect);
	_WindowWidth = clientRect.right-clientRect.left;
	_WindowHeight = clientRect.bottom-clientRect.top;
	_WindowX = clientRect.left;
	_WindowY = clientRect.top;	return true;
#endif
}

// --------------------------------------------------
bool CDriverGL::getModes(std::vector<GfxMode> &modes)
{
#ifdef NL_OS_WINDOWS
	sint modeIndex = 0;
	DEVMODE devMode;
	while (EnumDisplaySettings (NULL, modeIndex, &devMode))
	{
		// Keep only 16 and 32 bits
		if ((devMode.dmBitsPerPel == 16 ) || (devMode.dmBitsPerPel == 32))
		{
			// Add this mode
			GfxMode mode;
			mode.Width = devMode.dmPelsWidth;
			mode.Height = devMode.dmPelsHeight;
			mode.Depth = devMode.dmBitsPerPel;
			mode.Frequency = devMode.dmDisplayFrequency;
			modes.push_back (mode);
		}

		// Mode index
		modeIndex++;
	}
#endif
	return true;
}

// --------------------------------------------------

void CDriverGL::resetTextureShaders()
{	
	if (_Extensions.NVTextureShader)
	{
		glEnable(GL_TEXTURE_SHADER_NV);
		for (uint stage = 0; stage < (uint) inlGetNumTextStages(); ++stage)
		{		
			_DriverGLStates.activeTextureARB(stage);
			if (stage != 0)
			{
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV, GL_TEXTURE0_ARB + stage - 1);	
			}
			glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_NONE);	
			_CurrentTexAddrMode[stage] = GL_NONE;			
		}
		glDisable(GL_TEXTURE_SHADER_NV);
		_NVTextureShaderEnabled = false;
	}
}

// --------------------------------------------------

emptyProc CDriverGL::getWindowProc()
{
#ifdef NL_OS_WINDOWS
	return (emptyProc)GlWndProc;
#else // NL_OS_WINDOWS
	return NULL;
#endif // NL_OS_WINDOWS
}

// --------------------------------------------------

bool CDriverGL::activate()
{
#ifdef NL_OS_WINDOWS
	HGLRC hglrc=wglGetCurrentContext();
	if (hglrc!=_hRC)
	{
		wglMakeCurrent(_hDC,_hRC);
	}
#elif defined (NL_OS_UNIX)
	GLXContext nctx=glXGetCurrentContext();
	if (nctx != NULL && nctx!=ctx)
	{
		glXMakeCurrent(dpy, win,ctx);
	}
#endif // NL_OS_WINDOWS
	return true;
}

// --------------------------------------------------

bool CDriverGL::isTextureExist(const ITexture&tex)
{
	bool result;

	// Create the shared Name.
	std::string	name;
	getTextureShareName (tex, name);

	{
		CSynchronized<TTexDrvInfoPtrMap>::CAccessor access(&_SyncTexDrvInfos);
		TTexDrvInfoPtrMap &rTexDrvInfos = access.value();
		result = (rTexDrvInfos.find(name) != rTexDrvInfos.end());
	}
	return result;
}

// --------------------------------------------------

bool CDriverGL::clear2D(CRGBA rgba)
{
	glClearColor((float)rgba.R/255.0f,(float)rgba.G/255.0f,(float)rgba.B/255.0f,(float)rgba.A/255.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	return true;
}

// --------------------------------------------------

bool CDriverGL::clearZBuffer(float zval)
{
	glClearDepth(zval);
	_DriverGLStates.enableZWrite(true);
	glClear(GL_DEPTH_BUFFER_BIT);
	return true;
}

// --------------------------------------------------

void CDriverGL::setColorMask (bool bRed, bool bGreen, bool bBlue, bool bAlpha)
{
	glColorMask (bRed, bGreen, bBlue, bAlpha);
}

// --------------------------------------------------
bool CDriverGL::swapBuffers()
{	
	// Reset texture shaders
	//resetTextureShaders();


	/* Yoyo: must do this (GeForce bug ??) esle weird results if end render with a VBHard.
		Setup a std vertex buffer to ensure NVidia synchronisation.
	*/
	static	CVertexBuffer	dummyVB;
	static	bool			dummyVBinit= false; 
	if(!dummyVBinit)
	{
		dummyVBinit= true;
		// setup a full feature VB (maybe not usefull ... :( ).
		dummyVB.setVertexFormat(CVertexBuffer::PositionFlag|CVertexBuffer::NormalFlag|
			CVertexBuffer::PrimaryColorFlag|CVertexBuffer::SecondaryColorFlag|
			CVertexBuffer::TexCoord0Flag|CVertexBuffer::TexCoord1Flag|
			CVertexBuffer::TexCoord2Flag|CVertexBuffer::TexCoord3Flag
			);
		// some vertices.
		dummyVB.setNumVertices(10);
	}
	/* activate each frame to close VBHard rendering. 
		NVidia: This also force a SetFence on if last VB was a VBHard, "closing" it before swap.
	*/
	activeVertexBuffer(dummyVB);
	nlassert(_CurrentVertexBufferHard==NULL);


	/* PATCH For Possible NVidia Synchronisation.
	/*/
	// Because of Bug with GeForce, must finishFence() for all VBHard.
	/*set<IVertexBufferHardGL*>::iterator		itVBHard= _VertexBufferHardSet.Set.begin();
	while(itVBHard != _VertexBufferHardSet.Set.end() )
	{
		// Need only to do it for NVidia VB ones.
		if((*itVBHard)->NVidiaVertexBufferHard)
		{
			CVertexBufferHardGLNVidia	*vbHardNV= static_cast<CVertexBufferHardGLNVidia*>(*itVBHard);
			// If needed, "flush" these VB.
			vbHardNV->finishFence();
		}
		itVBHard++;
	}*/
	/* Need to Do this code only if Synchronisation PATCH before not done!
		AS NV_Fence GeForce Implementation says. Test each frame the NVFence, until completion. 
		NB: finish is not required here. Just test. This is like a "non block synchronisation"
	 */
	set<IVertexBufferHardGL*>::iterator		itVBHard= _VertexBufferHardSet.Set.begin();
	while(itVBHard != _VertexBufferHardSet.Set.end() )
	{
		if((*itVBHard)->VBType == IVertexBufferHardGL::NVidiaVB)
		{
			CVertexBufferHardGLNVidia	*vbHardNV= static_cast<CVertexBufferHardGLNVidia*>(*itVBHard);
			if(vbHardNV->isFenceSet())
			{
				// update Fence Cache.
				vbHardNV->testFence();
			}
		}
		itVBHard++;
	}


#ifdef NL_OS_WINDOWS
	if (_EventEmitter.getNumEmitters() > 1) // is direct input running ?
	{
		// flush direct input messages if any
		NLMISC::safe_cast<NLMISC::CDIEventEmitter *>(_EventEmitter.getEmitter(1))->poll();
	}
#endif


#ifdef NL_OS_WINDOWS
	SwapBuffers(_hDC);
#else // NL_OS_WINDOWS
	glXSwapBuffers(dpy, win);
#endif // NL_OS_WINDOWS
	
	// Activate the default texture environnments for all stages.
	//===========================================================
	// This is not a requirement, but it ensure a more stable state each frame.
	// (well, maybe the good reason is "it hides much more the bugs"  :o) ).
	for(sint stage=0;stage<inlGetNumTextStages(); stage++)
	{
		// init no texture.
		_CurrentTexture[stage]= NULL;
		_CurrentTextureInfoGL[stage]= NULL;
		// texture are disabled in DriverGLStates.forceDefaults().
		
		// init default env.
		CMaterial::CTexEnv	env;	// envmode init to default.
		env.ConstantColor.set(255,255,255,255);
		forceActivateTexEnvMode(stage, env);
		forceActivateTexEnvColor(stage, env);
	}
	

	// Activate the default material.
	//===========================================================
	// Same reasoning as textures :)
	_DriverGLStates.forceDefaults(inlGetNumTextStages());
	if (_NVTextureShaderEnabled)
	{
		glDisable(GL_TEXTURE_SHADER_NV);
		_NVTextureShaderEnabled = false;
	}
	_CurrentMaterial= NULL;

	// Reset the profiling counter.
	_PrimitiveProfileIn.reset();
	_PrimitiveProfileOut.reset();
	_NbSetupMaterialCall= 0;
	_NbSetupModelMatrixCall= 0;

	// Reset the texture set
	_TextureUsed.clear();

	// Reset Profile VBHardLock
	if(_VBHardProfiling)
	{
		_CurVBHardLockCount= 0;
		_NumVBHardProfileFrame++;
	}

	return true;
}

// --------------------------------------------------

bool CDriverGL::release()
{
	// release only if the driver was initialized
	if (!_Initialized) return true;

	// Call IDriver::release() before, to destroy textures, shaders and VBs...
	IDriver::release();


	deleteFragmentShaders();	

	// release caustic cube map
//	_CauticCubeMap = NULL;

	// Reset VertexArrayRange.
	resetVertexArrayRange();

	// delete containers
	delete _AGPVertexArrayRange;
	delete _VRAMVertexArrayRange;
	_AGPVertexArrayRange= NULL;
	_VRAMVertexArrayRange= NULL;

#ifdef NL_OS_WINDOWS
	// Then delete.
	// wglMakeCurrent(NULL,NULL);

	// Off-screen rendering ?
	if (_OffScreen)
	{
		if (_PBuffer)
		{
			wglDeleteContext( _hRC );
			wglReleasePbufferDCARB( _PBuffer, _hDC );
			wglDestroyPbufferARB( _PBuffer );
		}
	}
	else
	{
		if (_hRC)
			wglDeleteContext(_hRC);
		if (_hWnd&&_hDC)
		{
			ReleaseDC(_hWnd,_hDC);
			if (_DestroyWindow)
				DestroyWindow (_hWnd);
		}

		if(_FullScreen)
		{
			ChangeDisplaySettings(&_OldScreenMode, 0);
			_FullScreen= false;
		}
	}

	_hRC=NULL;
	_hDC=NULL;
	_hWnd=NULL;
	_PBuffer = NULL;

	// Restaure monitor color parameters
	if (_NeedToRestaureGammaRamp)
	{
		HDC dc = CreateDC ("DISPLAY", NULL, NULL, NULL);
		if (dc)
		{
			if (!SetDeviceGammaRamp (dc, _GammaRampBackuped))
				nlwarning ("(CDriverGL::release): SetDeviceGammaRamp failed");

			// Release the DC
			ReleaseDC (NULL, dc);
		}
		else
		{
			nlwarning ("(CDriverGL::release): can't create DC");
		}
	}

#elif defined (NL_OS_UNIX)// NL_OS_WINDOWS
 
#ifdef XF86VIDMODE
	if(_FullScreen)
	{
		XF86VidModeModeInfo info;
		nlinfo("Switching back to original mode \n");
 
		// This is a bit ugly - a quick hack to copy the ModeLine structure 
		// into the modeInfo structure.
		memcpy((XF86VidModeModeLine *)((char *)&info + sizeof(info.dotclock)),&_OldScreenMode, sizeof(XF86VidModeModeLine));
		info.dotclock = _OldDotClock;
 
		nlinfo("Mode is %dx%d,\n",info.hdisplay,info.vdisplay);
		XF86VidModeSwitchToMode(dpy,DefaultScreen(dpy),&info);
		nlinfo("Switching viewporr to %d,%d,\n",_OldX, _OldY);
		XF86VidModeSetViewPort(dpy,DefaultScreen(dpy),_OldX,_OldY);
		// Ungrab the keyboard (probably not necessary);
		XUngrabKeyboard(dpy, CurrentTime);
	}
#endif // XF86VIDMODE

#endif // NL_OS_UNIX


	// released
	_Initialized= false;

	return true;
}

// --------------------------------------------------

IDriver::TMessageBoxId	CDriverGL::systemMessageBox (const char* message, const char* title, IDriver::TMessageBoxType type, TMessageBoxIcon icon)
{
#ifdef NL_OS_WINDOWS
	switch (::MessageBox (NULL, message, title, ((type==retryCancelType)?MB_RETRYCANCEL:
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
	nlstop;
#else // NL_OS_WINDOWS
	// Call the console version!
	IDriver::systemMessageBox (message, title, type, icon);
#endif // NL_OS_WINDOWS
	return okId;
}

// --------------------------------------------------

void CDriverGL::setupViewport (const class CViewport& viewport)
{
#ifdef NL_OS_WINDOWS
	if (_hWnd == NULL) return;

	// Setup gl viewport
	int clientWidth = _WindowWidth;
	int clientHeight = _WindowHeight;

#else // NL_OS_WINDOWS

	XWindowAttributes win_attributes;
	if (!XGetWindowAttributes(dpy, win, &win_attributes))
		throw EBadDisplay("Can't get window attributes.");

	// Setup gl viewport
	int clientWidth=win_attributes.width;
	int clientHeight=win_attributes.height;

#endif // NL_OS_WINDOWS

	// Get viewport
	float x;
	float y;
	float width;
	float height;
	viewport.getValues (x, y, width, height);

	// Setup gl viewport
	int ix=(int)((float)clientWidth*x);
	clamp (ix, 0, clientWidth);
	int iy=(int)((float)clientHeight*y);
	clamp (iy, 0, clientHeight);
	int iwidth=(int)((float)clientWidth*width);
	clamp (iwidth, 0, clientWidth-ix);
	int iheight=(int)((float)clientHeight*height);
	clamp (iheight, 0, clientHeight-iy);
	glViewport (ix, iy, iwidth, iheight);

	_CurrViewport.init(ix / (float) clientWidth, iy / (float) clientHeight,
					   iwidth / (float) clientWidth, iheight / (float) clientHeight);
		
}

// --------------------------------------------------
void CDriverGL::getViewport(CViewport &viewport)
{
	viewport = _CurrViewport;	
}



// --------------------------------------------------
void	CDriverGL::setupScissor (const class CScissor& scissor)
{
	// Get viewport
	float x= scissor.X;
	float width= scissor.Width;
	float height= scissor.Height;

	if(x==0 && x==0 && width==1 && height==1)
	{
		glDisable(GL_SCISSOR_TEST);
	}
	else
	{
#ifdef NL_OS_WINDOWS

		float y= scissor.Y;

		if (_hWnd)
		{
			// Get window rect
			int clientWidth = _WindowWidth;
			int clientHeight = _WindowHeight;

			// Setup gl scissor
			int ix0=(int)floor((float)clientWidth * x + 0.5f);
			clamp (ix0, 0, clientWidth);
			int iy0=(int)floor((float)clientHeight* y + 0.5f);
			clamp (iy0, 0, clientHeight);

			int ix1=(int)floor((float)clientWidth * (x+width) + 0.5f );
			clamp (ix1, 0, clientWidth);
			int iy1=(int)floor((float)clientHeight* (y+height) + 0.5f );
			clamp (iy1, 0, clientHeight);


			int iwidth= ix1 - ix0;
			clamp (iwidth, 0, clientWidth);
			int iheight= iy1 - iy0;
			clamp (iheight, 0, clientHeight);

			glScissor (ix0, iy0, iwidth, iheight);
			glEnable(GL_SCISSOR_TEST);
		}
#endif // NL_OS_WINDOWS
	}
}



// --------------------------------------------------

void CDriverGL::showCursor(bool b)
{
#ifdef NL_OS_WINDOWS
	if (b)
	{
		while (ShowCursor(b) < 0) {};
	}
	else
	{
		while (ShowCursor(b) >= 0) {};
	}
#elif defined (NL_OS_UNIX)

	if (b)
	{
		if (cursor != None)
		{
			XFreeCursor(dpy, cursor);
			cursor = None;
		}
		XUndefineCursor(dpy, win);
	}
	else
	{
		if (cursor == None)
		{
			char bm_no_data[] = { 0,0,0,0, 0,0,0,0 };
			Pixmap pixmap_no_data = XCreateBitmapFromData (dpy, win, bm_no_data, 8, 8);
			XColor black;
			memset(&black, 0, sizeof (XColor));
			black.flags = DoRed | DoGreen | DoBlue;
			cursor = XCreatePixmapCursor (dpy, pixmap_no_data, pixmap_no_data, &black, &black, 0, 0);
			XFreePixmap(dpy, pixmap_no_data);
		}
		XDefineCursor(dpy, win, cursor);
	}
#endif // NL_OS_UNIX
}


// --------------------------------------------------

void CDriverGL::setMousePos(float x, float y)
{
#ifdef NL_OS_WINDOWS
	if (_hWnd)
	{
		// NeL window coordinate to MSWindows coordinates
		POINT pt;
		pt.x = (int)((float)(_WindowWidth)*x);
		pt.y = (int)((float)(_WindowHeight)*(1.0f-y));
		ClientToScreen (_hWnd, &pt);
		SetCursorPos(pt.x, pt.y);
	}
#elif defined (NL_OS_UNIX)
	XWindowAttributes xwa;
	XGetWindowAttributes (dpy, win, &xwa);
	int x1 = (int)(x * (float) xwa.width);
	int y1 = (int)((1.0f - y) * (float) xwa.height);
	XWarpPointer (dpy, None, win, None, None, None, None, x1, y1);
#endif // NL_OS_UNIX
}


void CDriverGL::getWindowSize(uint32 &width, uint32 &height)
{
#ifdef NL_OS_WINDOWS
	// Off-srceen rendering ?
	if (_OffScreen)
	{
		if (_PBuffer)
		{
			wglQueryPbufferARB( _PBuffer, WGL_PBUFFER_WIDTH_ARB, (int*)&width );
			wglQueryPbufferARB( _PBuffer, WGL_PBUFFER_HEIGHT_ARB, (int*)&height );
		}
	}
	else
	{
		if (_hWnd)
		{
			width = (uint32)(_WindowWidth);
			height = (uint32)(_WindowHeight);
		}
	}
#elif defined (NL_OS_UNIX)
	XWindowAttributes xwa;
	XGetWindowAttributes (dpy, win, &xwa);
	width = (uint32) xwa.width;
	height = (uint32) xwa.height;
#endif // NL_OS_UNIX
}

void CDriverGL::getWindowPos(uint32 &x, uint32 &y)
{
#ifdef NL_OS_WINDOWS
	// Off-srceen rendering ?
	if (_OffScreen)
	{
		if (_PBuffer)
		{
			x = y = 0;
		}
	}
	else
	{
		if (_hWnd)
		{
			x = (uint32)(_WindowX);
			y = (uint32)(_WindowY);
		}
	}
#elif defined (NL_OS_UNIX)
	x = y = 0;
#endif // NL_OS_UNIX
}



// --------------------------------------------------

bool CDriverGL::isActive()
{
#ifdef NL_OS_WINDOWS
	return (IsWindow(_hWnd) != 0);
#elif defined (NL_OS_UNIX)
	return true;
#endif // NL_OS_UNIX
}

uint8 CDriverGL::getBitPerPixel ()
{
	return _Depth;
}

const char *CDriverGL::getVideocardInformation ()
{
	static char name[1024];

	if (!_Initialized) return "OpenGL isn't initialized";

	const char *vendor = (const char *) glGetString (GL_VENDOR);
	const char *renderer = (const char *) glGetString (GL_RENDERER);
	const char *version = (const char *) glGetString (GL_VERSION);

	smprintf(name, 1024, "%s / %s / %s", vendor, renderer, version);
	return name;
}


void CDriverGL::setCapture (bool b)
{
#ifdef NL_OS_WINDOWS

	if (b)
	{
		RECT client;
		GetClientRect (_hWnd, &client);
		POINT pt1,pt2;
		pt1.x = client.left;
		pt1.y = client.top;
		ClientToScreen (_hWnd, &pt1);
		pt2.x = client.right;
		pt2.y = client.bottom;
		ClientToScreen (_hWnd, &pt2);
		client.bottom = pt2.y;
		client.top = pt1.y;
		client.left = pt1.x;
		client.right = pt2.x;
		ClipCursor (&client);
	}
	else
		ClipCursor (NULL);

	/*
	if (b)
		SetCapture (_hWnd);
	else
		ReleaseCapture ();
	*/

#elif defined (NL_OS_UNIX)

#endif // NL_OS_UNIX
}


bool			CDriverGL::clipRect(NLMISC::CRect &rect)
{
	// Clip the wanted rectangle with window.
	uint32 width, height;
	getWindowSize(width, height);

	sint32	xr=rect.right() ,yr=rect.bottom();
	
	clamp((sint32&)rect.X, (sint32)0, (sint32)width);
	clamp((sint32&)rect.Y, (sint32)0, (sint32)height);
	clamp((sint32&)xr, (sint32)rect.X, (sint32)width);
	clamp((sint32&)yr, (sint32)rect.Y, (sint32)height);
	rect.Width= xr-rect.X;
	rect.Height= yr-rect.Y;

	return rect.Width>0 && rect.Height>0;
}



void			CDriverGL::getBufferPart (CBitmap &bitmap, NLMISC::CRect &rect)
{
	bitmap.reset();

	if(clipRect(rect))
	{
		bitmap.resize(rect.Width, rect.Height, CBitmap::RGBA);
		glReadPixels (rect.X, rect.Y, rect.Width, rect.Height, GL_RGBA, GL_UNSIGNED_BYTE, bitmap.getPixels ().getPtr());
	}
}


void			CDriverGL::getZBufferPart (std::vector<float>  &zbuffer, NLMISC::CRect &rect)
{
	zbuffer.clear();

	if(clipRect(rect))
	{
		zbuffer.resize(rect.Width*rect.Height);
		glPixelTransferf(GL_DEPTH_SCALE, 1.0f) ;
		glPixelTransferf(GL_DEPTH_BIAS, 0.f) ;
		glReadPixels (rect.X, rect.Y, rect.Width, rect.Height, GL_DEPTH_COMPONENT , GL_FLOAT, &(zbuffer[0]));
	}
}


void			CDriverGL::getZBuffer (std::vector<float>  &zbuffer)
{
	CRect	rect(0,0);
	getWindowSize(rect.Width, rect.Height);
	getZBufferPart(zbuffer, rect);
}

void CDriverGL::getBuffer (CBitmap &bitmap)
{
	CRect	rect(0,0);
	getWindowSize(rect.Width, rect.Height);
	getBufferPart(bitmap, rect);
}

bool CDriverGL::fillBuffer (CBitmap &bitmap)
{
	CRect	rect(0,0);
	getWindowSize(rect.Width, rect.Height);
	if( rect.Width!=bitmap.getWidth() || rect.Height!=bitmap.getHeight() || bitmap.getPixelFormat()!=CBitmap::RGBA )
		return false;

	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glDrawPixels (rect.Width, rect.Height, GL_RGBA, GL_UNSIGNED_BYTE, &(bitmap.getPixels()[0]) );

	return true;
}


void CDriverGL::copyFrameBufferToTexture(ITexture *tex,
										 uint32 level,
										 uint32 offsetx,
										 uint32 offsety,
										 uint32 x,
										 uint32 y,
										 uint32 width,
										 uint32 height														
										)
{
	nlassert(!tex->isTextureCube());
	bool compressed = false;
	getGlTextureFormat(*tex, compressed);
	nlassert(!compressed);	
	// first, mark the texture as valid, and make sure there is a corresponding texture in the device memory	
	setupTexture(*tex);	
	CTextureDrvInfosGL*	gltext = (CTextureDrvInfosGL*)(ITextureDrvInfos*)(tex->TextureDrvShare->DrvTexture);
	_DriverGLStates.activeTextureARB(0);
	// setup texture mode, after activeTextureARB()
	_DriverGLStates.setTextureMode(CDriverGLStates::Texture2D);
	glBindTexture(GL_TEXTURE_2D, gltext->ID);	
	glCopyTexSubImage2D(GL_TEXTURE_2D, level, offsetx, offsety, x, y, width, height);	
	// disable texturing.
	_DriverGLStates.setTextureMode(CDriverGLStates::TextureDisabled);
	_CurrentTexture[0] = NULL;
	_CurrentTextureInfoGL[0] = NULL;
}


void CDriverGL::setPolygonMode (TPolygonMode mode)
{
	IDriver::setPolygonMode (mode);

	// Set the polygon mode
	switch (_PolygonMode)
	{
	case Filled:
		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
		break;
	case Line:
		glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
		break;
	case Point:
		glPolygonMode (GL_FRONT_AND_BACK, GL_POINT);
		break;
	}
}


bool			CDriverGL::fogEnabled()
{
	return _FogEnabled;
}

void			CDriverGL::enableFog(bool enable)
{
	_DriverGLStates.enableFog(enable);
	_FogEnabled= enable;
}

void			CDriverGL::setupFog(float start, float end, CRGBA color)
{
	glFogf(GL_FOG_MODE, GL_LINEAR);
	glFogf(GL_FOG_START, start);
	glFogf(GL_FOG_END, end);

	_CurrentFogColor[0]= color.R/255.0f;
	_CurrentFogColor[1]= color.G/255.0f;
	_CurrentFogColor[2]= color.B/255.0f;
	_CurrentFogColor[3]= color.A/255.0f;

	glFogfv(GL_FOG_COLOR, _CurrentFogColor);

	/** Special : with vertex program, using the extension EXT_vertex_shader, fog is emulated using 1 more constant to scale result to [0, 1]
	  */
	if (_Extensions.EXTVertexShader && !_Extensions.NVVertexProgram)
	{
		// register 96 is used to store fog informations
		if (start != end)
		{		
			setConstant(_EVSNumConstant, 1.f / (start - end), - end / (start - end), 0, 0);			
		}
		else
		{
			setConstant(_EVSNumConstant, 0.f, 0, 0, 0);
		}
	}
	_FogStart = start;
	_FogEnd = end;
}


// ***************************************************************************
float			CDriverGL::getFogStart() const
{
	return _FogStart;
}

// ***************************************************************************
float			CDriverGL::getFogEnd() const
{
	return _FogEnd;
}

// ***************************************************************************
CRGBA			CDriverGL::getFogColor() const
{
	CRGBA	ret;
	ret.R= (uint8)(_CurrentFogColor[0]*255);
	ret.G= (uint8)(_CurrentFogColor[1]*255);
	ret.B= (uint8)(_CurrentFogColor[2]*255);
	ret.A= (uint8)(_CurrentFogColor[3]*255);
	return ret;
}


// ***************************************************************************
void			CDriverGL::profileRenderedPrimitives(CPrimitiveProfile &pIn, CPrimitiveProfile &pOut)
{
	pIn= _PrimitiveProfileIn;
	pOut= _PrimitiveProfileOut;
}


// ***************************************************************************
uint32			CDriverGL::profileAllocatedTextureMemory()
{
	return _AllocatedTextureMemory;
}


// ***************************************************************************
uint32			CDriverGL::profileSetupedMaterials() const
{
	return _NbSetupMaterialCall;
}


// ***************************************************************************
uint32			CDriverGL::profileSetupedModelMatrix() const
{
	return _NbSetupModelMatrixCall;
}


// ***************************************************************************
void			CDriverGL::enableUsedTextureMemorySum (bool enable)
{
	if (enable)
		nlinfo ("PERFORMANCE INFO: enableUsedTextureMemorySum has been set to true in CDriverGL\n");
	_SumTextureMemoryUsed=enable;
}


// ***************************************************************************
uint32			CDriverGL::getUsedTextureMemory() const
{
	// Sum memory used
	uint32 memory=0;

	// For each texture used
	set<CTextureDrvInfosGL*>::iterator ite=_TextureUsed.begin();
	while (ite!=_TextureUsed.end())
	{
		// Get the gl texture
		CTextureDrvInfosGL*	gltext;
		gltext= (*ite);

		// Sum the memory used by this texture
		memory+=gltext->TextureMemory;

		// Next texture
		ite++;
	}

	// Return the count
	return memory;
}


// ***************************************************************************
bool CDriverGL::supportTextureShaders() const
{
	// fully supported by NV_TEXTURE_SHADER	
	return _Extensions.NVTextureShader;
}

// ***************************************************************************
bool CDriverGL::isWaterShaderSupported() const
{
	if (!_Extensions.EXTVertexShader && !_Extensions.NVVertexProgram) return false; // should support vertex programms
	if (!_Extensions.NVTextureShader && !_Extensions.ATIFragmentShader && !_Extensions.ARBFragmentProgram) return false;
	return true;
}

// ***************************************************************************
bool CDriverGL::isTextureAddrModeSupported(CMaterial::TTexAddressingMode mode) const
{
	if (_Extensions.NVTextureShader)
	{
		// all the given addessing mode are supported with this extension
		return true;
	}
	else
	{			
		return false;
	}
}

// ***************************************************************************
void CDriverGL::setMatrix2DForTextureOffsetAddrMode(const uint stage, const float mat[4])
{
	if (!supportTextureShaders()) return;
	//nlassert(supportTextureShaders());
	nlassert(stage < (uint) inlGetNumTextStages() );
	_DriverGLStates.activeTextureARB(stage);
	glTexEnvfv(GL_TEXTURE_SHADER_NV, GL_OFFSET_TEXTURE_MATRIX_NV, mat);
}


// ***************************************************************************
void      CDriverGL::enableNVTextureShader(bool enabled)
{		
	if (enabled != _NVTextureShaderEnabled)
	{

		if (enabled)
		{							
			glEnable(GL_TEXTURE_SHADER_NV);			
		}
		else
		{						
			glDisable(GL_TEXTURE_SHADER_NV);		
		}
		_NVTextureShaderEnabled = enabled;
	}	
}

// ***************************************************************************
void CDriverGL::checkForPerPixelLightingSupport()
{
	// we need at least 3 texture stages and cube map support + EnvCombine4 or 3 support	
	// TODO : support for EnvCombine3
	// TODO : support for less than 3 stages

	_SupportPerPixelShaderNoSpec = (_Extensions.NVTextureEnvCombine4 || _Extensions.ATIXTextureEnvCombine3)
								   && _Extensions.ARBTextureCubeMap
								   && _Extensions.NbTextureStages >= 3
								   && (_Extensions.NVVertexProgram || _Extensions.EXTVertexShader);
	
	_SupportPerPixelShader = (_Extensions.NVTextureEnvCombine4 || _Extensions.ATIXTextureEnvCombine3) 
							 && _Extensions.ARBTextureCubeMap
							 && _Extensions.NbTextureStages >= 2
							 && (_Extensions.NVVertexProgram || _Extensions.EXTVertexShader);	
}

// ***************************************************************************
bool CDriverGL::supportPerPixelLighting(bool specular) const
{
	return specular ? _SupportPerPixelShader : _SupportPerPixelShaderNoSpec;	
}

// ***************************************************************************
void	CDriverGL::setPerPixelLightingLight(CRGBA diffuse, CRGBA specular, float shininess)
{
	_PPLExponent = shininess;
	_PPLightDiffuseColor = diffuse;
	_PPLightSpecularColor = specular;
}

// ***************************************************************************
NLMISC::IMouseDevice	*CDriverGL::enableLowLevelMouse(bool enable, bool exclusive)
{
#ifdef NL_OS_WINDOWS
		if (_EventEmitter.getNumEmitters() < 2) return NULL;
		NLMISC::CDIEventEmitter *diee = NLMISC::safe_cast<CDIEventEmitter *>(_EventEmitter.getEmitter(1));		
		if (enable)
		{
			try
			{
				NLMISC::IMouseDevice *md = diee->getMouseDevice(exclusive);
				return md;
			}
			catch (EDirectInput &)
			{
				return NULL;
			}
		}
		else
		{
			diee->releaseMouse();			
			return NULL;
		}
#else
		return NULL;
#endif
}
		
// ***************************************************************************
NLMISC::IKeyboardDevice		*CDriverGL::enableLowLevelKeyboard(bool enable)
{
#ifdef NL_OS_WINDOWS
		if (_EventEmitter.getNumEmitters() < 2) return NULL;
		NLMISC::CDIEventEmitter *diee = NLMISC::safe_cast<NLMISC::CDIEventEmitter *>(_EventEmitter.getEmitter(1));
		if (enable)
		{
			try
			{
				NLMISC::IKeyboardDevice *md = diee->getKeyboardDevice();
				return md;
			}
			catch (EDirectInput &)
			{
				return NULL;
			}
		}
		else
		{
			diee->releaseKeyboard();
			return NULL;
		}
#else
		return NULL;
#endif
}

// ***************************************************************************
NLMISC::IInputDeviceManager		*CDriverGL::getLowLevelInputDeviceManager()
{
#ifdef NL_OS_WINDOWS
		if (_EventEmitter.getNumEmitters() < 2) return NULL;
		NLMISC::CDIEventEmitter *diee = NLMISC::safe_cast<NLMISC::CDIEventEmitter *>(_EventEmitter.getEmitter(1));
		return diee;
#else
		return NULL;
#endif
}

// ***************************************************************************
uint CDriverGL::getDoubleClickDelay(bool hardwareMouse)
{
#ifdef NL_OS_WINDOWS
		NLMISC::IMouseDevice *md = NULL;
		if (_EventEmitter.getNumEmitters() >= 2)
		{		
			NLMISC::CDIEventEmitter *diee = NLMISC::safe_cast<CDIEventEmitter *>(_EventEmitter.getEmitter(1));					
			if (diee->isMouseCreated())
			{			
				try
				{
					md = diee->getMouseDevice(hardwareMouse);					
				}
				catch (EDirectInput &)
				{
					// could not get device ..					
				}			
			}
		}
		if (md)
		{
			return md->getDoubleClickDelay();
		}
		// try to read the good value from windows
		return ::GetDoubleClickTime();
#else
		// FIXME: FAKE FIX
		return 250;
#endif
}

// ***************************************************************************
bool			CDriverGL::supportBlendConstantColor() const
{
	return _Extensions.EXTBlendColor;
}
// ***************************************************************************
void			CDriverGL::setBlendConstantColor(NLMISC::CRGBA col)
{
	// bkup
	_CurrentBlendConstantColor= col;

	// update GL
	if(!_Extensions.EXTBlendColor)
		return;
	static const	float	OO255= 1.0f/255;
	nglBlendColorEXT(col.R*OO255, col.G*OO255, col.B*OO255, col.A*OO255);
}
// ***************************************************************************
NLMISC::CRGBA	CDriverGL::getBlendConstantColor() const
{
	return	_CurrentBlendConstantColor;
}

// ***************************************************************************
sint			CDriverGL::getNbTextureStages() const
{
	return inlGetNumTextStages();
}


// ***************************************************************************
void CDriverGL::refreshProjMatrixFromGL()
{
	if (!_ProjMatDirty) return;	
	float mat[16];
	glGetFloatv(GL_PROJECTION_MATRIX, mat);
	_GLProjMat.set(mat);	
	_ProjMatDirty = false;
}


// ***************************************************************************
bool			CDriverGL::setMonitorColorProperties (const CMonitorColorProperties &properties)
{
#ifdef NL_OS_WINDOWS
	
	// Get a DC
	HDC dc = CreateDC ("DISPLAY", NULL, NULL, NULL);
	if (dc)
	{
		// The ramp
		WORD ramp[256*3];

		// For each composant
		uint c;
		for( c=0; c<3; c++ )
		{
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
				ramp[i+(c<<8)] = min (65535, max (0, (int)(value * 65535)));
			}
		}

		// Set the ramp
		bool result = SetDeviceGammaRamp (dc, ramp) != FALSE;
		
		// Release the DC
		ReleaseDC (NULL, dc);

		// Returns result
		return result;
	}
	else
	{
		nlwarning ("(CDriverGL::setMonitorColorProperties): can't create DC");
		return false;
	}

#else

	nlwarning ("CDriverGL::setMonitorColorProperties not implemented");
	return false;

#endif
}

// ***************************************************************************
bool CDriverGL::supportEMBM() const
{
	// For now, supported via ATI extension
	return _Extensions.ATIEnvMapBumpMap;
}

// ***************************************************************************
bool CDriverGL::isEMBMSupportedAtStage(uint stage) const
{
	nlassert(supportEMBM());
	nlassert(stage < IDRV_MAT_MAXTEXTURES);
	return _StageSupportEMBM[stage];

}

// ***************************************************************************
void CDriverGL::setEMBMMatrix(const uint stage,const float mat[4])
{
	nlassert(supportEMBM());
	nlassert(stage < IDRV_MAT_MAXTEXTURES);
	// 
	if (_Extensions.ATIEnvMapBumpMap)
	{
		_DriverGLStates.activeTextureARB(stage);
		nglTexBumpParameterfvATI(GL_BUMP_ROT_MATRIX_ATI, const_cast<float *>(mat));
	}
}

// ***************************************************************************
void CDriverGL::initEMBM()
{
	if (supportEMBM())
	{	
		std::fill(_StageSupportEMBM, _StageSupportEMBM + IDRV_MAT_MAXTEXTURES, false);
		if (_Extensions.ATIEnvMapBumpMap)
		{		
			// Test which stage support EMBM
			GLint numEMBMUnits;
			nglGetTexBumpParameterivATI(GL_BUMP_NUM_TEX_UNITS_ATI, &numEMBMUnits);
			std::vector<GLint> EMBMUnits(numEMBMUnits);
			// get array of units that supports EMBM
			nglGetTexBumpParameterivATI(GL_BUMP_TEX_UNITS_ATI, &EMBMUnits[0]);
			uint k;
			for(k = 0; k < (EMBMUnits.size() - 1); ++k)
			{
				uint stage = EMBMUnits[k] - GL_TEXTURE0_ARB;
				if (stage < (IDRV_MAT_MAXTEXTURES - 1))
				{
					_StageSupportEMBM[k] = true;
				}		
			} 
			// setup each stage to apply the bump map to the next stage
			for(k = 0; k < IDRV_MAT_MAXTEXTURES - 1; ++k)
			{
				if (_StageSupportEMBM[k])
				{	
					// setup each stage so that it apply EMBM on the next stage
					_DriverGLStates.activeTextureARB(k);
					glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);	  
					glTexEnvi(GL_TEXTURE_ENV, GL_BUMP_TARGET_ATI, GL_TEXTURE0_ARB + k + 1);				
				}
			}
			_DriverGLStates.activeTextureARB(0);
		}
	}
}




// ***************************************************************************
/** Water fragment program with extension ARB_fragment_program
  */
static const char *WaterCodeNoDiffuseForARBFragmentProgram =
"!!ARBfp1.0																			\n\
OPTION ARB_precision_hint_nicest;													\n\
PARAM  bump0ScaleBias = program.env[0];												\n\
PARAM  bump1ScaleBias = program.env[1];												\n\
ATTRIB bump0TexCoord  = fragment.texcoord[0];										\n\
ATTRIB bump1TexCoord  = fragment.texcoord[1];										\n\
ATTRIB envMapTexCoord = fragment.texcoord[2];										\n\
OUTPUT oCol  = result.color;														\n\
TEMP   bmValue;                                                                     \n\
#read bump map 0																	\n\
TEX    bmValue, bump0TexCoord, texture[0], 2D;										\n\
#bias result (include scaling)														\n\
MAD    bmValue, bmValue, bump0ScaleBias.xxxx, bump0ScaleBias.yyzz;					\n\
ADD    bmValue, bmValue, bump1TexCoord;                                             \n\
#read bump map 1																	\n\
TEX    bmValue, bmValue, texture[1], 2D;											\n\
#bias result (include scaling)														\n\
MAD    bmValue, bmValue, bump1ScaleBias.xxxx, bump1ScaleBias.yyzz;					\n\
#add envmap coord																	\n\
ADD	   bmValue, bmValue, envMapTexCoord;											\n\
#read envmap																		\n\
TEX    oCol, bmValue, texture[2], 2D;												\n\
END ";

static const char *WaterCodeNoDiffuseWithFogForARBFragmentProgram = 
"!!ARBfp1.0																			\n\
OPTION ARB_precision_hint_nicest;													\n\
PARAM  bump0ScaleBias = program.env[0];												\n\
PARAM  bump1ScaleBias = program.env[1];												\n\
PARAM  fogColor       = state.fog.color;											\n\
PARAM  fogFactor      = program.env[2];												\n\
ATTRIB bump0TexCoord  = fragment.texcoord[0];										\n\
ATTRIB bump1TexCoord  = fragment.texcoord[1];										\n\
ATTRIB envMapTexCoord = fragment.texcoord[2];										\n\
ATTRIB fogValue		  = fragment.fogcoord;											\n\
OUTPUT oCol  = result.color;														\n\
TEMP   bmValue;                                                                     \n\
TEMP   envMap;																		\n\
TEMP   tmpFog;																		\n\
#read bump map 0																	\n\
TEX    bmValue, bump0TexCoord, texture[0], 2D;										\n\
#bias result (include scaling)														\n\
MAD    bmValue, bmValue, bump0ScaleBias.xxxx, bump0ScaleBias.yyzz;					\n\
ADD    bmValue, bmValue, bump1TexCoord;                                             \n\
#read bump map 1																	\n\
TEX    bmValue, bmValue, texture[1], 2D;											\n\
#bias result (include scaling)														\n\
MAD    bmValue, bmValue, bump1ScaleBias.xxxx, bump1ScaleBias.yyzz;					\n\
#add envmap coord																	\n\
ADD	   bmValue, bmValue, envMapTexCoord;											\n\
#read envmap																		\n\
TEX    envMap, bmValue, texture[2], 2D;												\n\
#compute fog																		\n\
MAD_SAT tmpFog, fogValue.x, fogFactor.x, fogFactor.y;								\n\
LRP    oCol, tmpFog.x, envMap, fogColor;											\n\
END ";



// **************************************************************************************
/** Water fragment program with extension ARB_fragment_program and a diffuse map applied
  */
static const char *WaterCodeForARBFragmentProgram =
"!!ARBfp1.0																			\n\
OPTION ARB_precision_hint_nicest;													\n\
PARAM  bump0ScaleBias = program.env[0];												\n\
PARAM  bump1ScaleBias = program.env[1];												\n\
ATTRIB bump0TexCoord  = fragment.texcoord[0];										\n\
ATTRIB bump1TexCoord  = fragment.texcoord[1];										\n\
ATTRIB envMapTexCoord = fragment.texcoord[2];										\n\
ATTRIB diffuseTexCoord = fragment.texcoord[3];										\n\
OUTPUT oCol  = result.color;														\n\
TEMP   bmValue;                                                                     \n\
TEMP   diffuse;																		\n\
TEMP   envMap;																		\n\
#read bump map 0																	\n\
TEX    bmValue, bump0TexCoord, texture[0], 2D;										\n\
#bias result (include scaling)														\n\
MAD    bmValue, bmValue, bump0ScaleBias.xxxx, bump0ScaleBias.yyzz;					\n\
ADD    bmValue, bmValue, bump1TexCoord;                                             \n\
#read bump map 1																	\n\
TEX    bmValue, bmValue, texture[1], 2D;											\n\
#bias result (include scaling)														\n\
MAD    bmValue, bmValue, bump1ScaleBias.xxxx, bump1ScaleBias.yyzz;					\n\
#add envmap coord																	\n\
ADD	   bmValue, bmValue, envMapTexCoord;											\n\
#read envmap																		\n\
TEX    envMap, bmValue, texture[2], 2D;												\n\
#read diffuse																		\n\
TEX    diffuse, diffuseTexCoord, texture[3], 2D;									\n\
#modulate diffuse and envmap to get result											\n\
MUL    oCol, diffuse, envMap;														\n\
END ";

static const char *WaterCodeWithFogForARBFragmentProgram =
"!!ARBfp1.0																			\n\
OPTION ARB_precision_hint_nicest;													\n\
PARAM  bump0ScaleBias = program.env[0];												\n\
PARAM  bump1ScaleBias = program.env[1];												\n\
PARAM  fogColor       = state.fog.color;											\n\
PARAM  fogFactor      = program.env[2];												\n\
ATTRIB bump0TexCoord  = fragment.texcoord[0];										\n\
ATTRIB bump1TexCoord  = fragment.texcoord[1];										\n\
ATTRIB envMapTexCoord = fragment.texcoord[2];										\n\
ATTRIB diffuseTexCoord = fragment.texcoord[3];										\n\
ATTRIB fogValue		   = fragment.fogcoord;											\n\
OUTPUT oCol  = result.color;														\n\
TEMP   bmValue;                                                                     \n\
TEMP   diffuse;																		\n\
TEMP   envMap;																		\n\
TEMP   tmpFog;																		\n\
#read bump map 0																	\n\
TEX    bmValue, bump0TexCoord, texture[0], 2D;										\n\
#bias result (include scaling)														\n\
MAD    bmValue, bmValue, bump0ScaleBias.xxxx, bump0ScaleBias.yyzz;					\n\
ADD    bmValue, bmValue, bump1TexCoord;                                             \n\
#read bump map 1																	\n\
TEX    bmValue, bmValue, texture[1], 2D;											\n\
#bias result (include scaling)														\n\
MAD    bmValue, bmValue, bump1ScaleBias.xxxx, bump1ScaleBias.yyzz;					\n\
#add envmap coord																	\n\
ADD	   bmValue, bmValue, envMapTexCoord;											\n\
TEX    envMap, bmValue, texture[2], 2D;												\n\
TEX    diffuse, diffuseTexCoord, texture[3], 2D;									\n\
MAD_SAT tmpFog, fogValue, fogFactor.x, fogFactor.y;									\n\
#modulate diffuse and envmap to get result											\n\
MUL    diffuse, diffuse, envMap;													\n\
LRP    oCol, tmpFog, diffuse, fogColor;												\n\
END ";


// ***************************************************************************
/** Load a ARB_fragment_program_code, and ensure it is loaded natively
  */
uint loadARBFragmentProgramStringNative(const char *prog)
{
	if (!prog) return 0;
	GLuint progID;
	nglGenProgramsARB(1, &progID);
	if (!progID) return false;
	nglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, progID);
	GLint errorPos, isNative;
	nglProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, strlen(prog), prog);
	nglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, 0);
	glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
	nglGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &isNative);
	if ((errorPos == -1) && (isNative == 1))
	{	
		return progID;
	}
	else
	{
		if (errorPos == -1)
		{
			nglDeleteProgramsARB(1, &progID);			
		}
		return 0;
	}
}


// ***************************************************************************
/** R200 Fragment Shader :
  * Send fragment shader to fetch a perturbed envmap from the addition of 2 bumpmap
  * The result is in R2 after the 2nd pass
  */
static void fetchPerturbedEnvMapR200()
{
	////////////
	// PASS 1 //
	////////////

	nglSampleMapATI(GL_REG_0_ATI, GL_TEXTURE0_ARB, GL_SWIZZLE_STR_ATI); // sample bump map 0
	nglSampleMapATI(GL_REG_1_ATI, GL_TEXTURE1_ARB, GL_SWIZZLE_STR_ATI); // sample bump map 1
	nglPassTexCoordATI(GL_REG_2_ATI, GL_TEXTURE2_ARB, GL_SWIZZLE_STR_ATI);	// get texcoord for envmap
	
	nglColorFragmentOp3ATI(GL_MAD_ATI, GL_REG_2_ATI, GL_NONE, GL_NONE, GL_REG_0_ATI, GL_NONE, GL_BIAS_BIT_ATI|GL_2X_BIT_ATI, GL_CON_0_ATI, GL_NONE, GL_NONE, GL_REG_2_ATI, GL_NONE, GL_NONE); // scale bumpmap 1 & add envmap coords	
	nglColorFragmentOp3ATI(GL_MAD_ATI, GL_REG_2_ATI, GL_NONE, GL_NONE, GL_REG_1_ATI, GL_NONE, GL_BIAS_BIT_ATI|GL_2X_BIT_ATI, GL_CON_1_ATI, GL_NONE, GL_NONE, GL_REG_2_ATI, GL_NONE, GL_NONE); // scale bumpmap 2 & add to bump map 1
	
	

	////////////
	// PASS 2 //
	////////////
	nglSampleMapATI(GL_REG_2_ATI, GL_REG_2_ATI, GL_SWIZZLE_STR_ATI); // fetch envmap at perturbed texcoords	

}

// ***************************************************************************
void CDriverGL::initFragmentShaders()
{
	///////////////////
	// WATER SHADERS //
	///////////////////

	// the ARB_fragment_program is prioritary over other extensions when present
	if (_Extensions.ARBFragmentProgram)
	{
		
		ARBWaterShader[0] = loadARBFragmentProgramStringNative(WaterCodeNoDiffuseForARBFragmentProgram);
		ARBWaterShader[1] = loadARBFragmentProgramStringNative(WaterCodeNoDiffuseWithFogForARBFragmentProgram);
		ARBWaterShader[2] = loadARBFragmentProgramStringNative(WaterCodeForARBFragmentProgram);
		ARBWaterShader[3] = loadARBFragmentProgramStringNative(WaterCodeWithFogForARBFragmentProgram);
		bool ok = true;
		for(uint k = 0; k < 4; ++k)
		{
			if (!ARBWaterShader[k])
			{
				ok = false;
				deleteARBFragmentPrograms();
				break;
			}
		}	
		if (ok) return;
	}
	
	if (_Extensions.ATIFragmentShader)
	{
		ATIWaterShaderHandleNoDiffuseMap = nglGenFragmentShadersATI(1);
		ATIWaterShaderHandle = nglGenFragmentShadersATI(1);
		if (!ATIWaterShaderHandle || !ATIWaterShaderHandleNoDiffuseMap)
		{
			ATIWaterShaderHandleNoDiffuseMap = ATIWaterShaderHandle = 0;
			nlwarning("Couldn't generate water shader using ATI_fragment_shader !");
			return;
		}
		else
		{

			glGetError();
			// Water shader for R200 : we just add the 2 bump map contributions (du, dv). We then use this contribution to perturbate the envmap
			nglBindFragmentShaderATI(ATIWaterShaderHandleNoDiffuseMap);
			nglBeginFragmentShaderATI();			
			//
			fetchPerturbedEnvMapR200();
			nglColorFragmentOp1ATI(GL_MOV_ATI, GL_REG_0_ATI, GL_NONE, GL_NONE, GL_REG_2_ATI, GL_NONE, GL_NONE);
			nglAlphaFragmentOp1ATI(GL_MOV_ATI, GL_REG_0_ATI, GL_NONE, GL_REG_2_ATI, GL_NONE, GL_NONE);
			//			
			nglEndFragmentShaderATI();
			GLenum error = glGetError();
		    nlassert(error == GL_NONE);

			// The same but with a diffuse map added
			nglBindFragmentShaderATI(ATIWaterShaderHandle);
			nglBeginFragmentShaderATI();
			//
			fetchPerturbedEnvMapR200();

			nglSampleMapATI(GL_REG_3_ATI, GL_TEXTURE3_ARB, GL_SWIZZLE_STR_ATI); // fetch envmap at perturbed texcoords
			nglColorFragmentOp2ATI(GL_MUL_ATI, GL_REG_0_ATI, GL_NONE, GL_NONE, GL_REG_3_ATI, GL_NONE, GL_NONE, GL_REG_2_ATI, GL_NONE, GL_NONE); // scale bumpmap 1 & add envmap coords
			nglAlphaFragmentOp2ATI(GL_MUL_ATI, GL_REG_0_ATI, GL_NONE, GL_REG_3_ATI, GL_NONE, GL_NONE, GL_REG_2_ATI, GL_NONE, GL_NONE);


			nglEndFragmentShaderATI();
			error = glGetError();
		    nlassert(error == GL_NONE);
			nglBindFragmentShaderATI(0);			
		}
	}	

	// if none of the previous programs worked, fallback on NV_texture_shader, or (todo) simpler shader
	
}

// ***************************************************************************
void CDriverGL::deleteARBFragmentPrograms()
{
	for(uint k = 0; k < 4; ++k)
	{
		if (ARBWaterShader[k])
		{
			GLuint progId = (GLuint) ARBWaterShader[k];
			nglDeleteProgramsARB(1, &progId);
			ARBWaterShader[k] = 0;
		}
	}

}

// ***************************************************************************
void CDriverGL::deleteFragmentShaders()
{	
	deleteARBFragmentPrograms();
	if (ATIWaterShaderHandleNoDiffuseMap)
	{		
		nglDeleteFragmentShaderATI((GLuint) ATIWaterShaderHandleNoDiffuseMap);		
		ATIWaterShaderHandleNoDiffuseMap = 0;
	}
	if (ATIWaterShaderHandle)
	{		
		nglDeleteFragmentShaderATI((GLuint) ATIWaterShaderHandle);		
		ATIWaterShaderHandle = 0;
	}
}


// ***************************************************************************
void CDriverGL::finish()
{
	glFinish();
}


// ***************************************************************************
void	CDriverGL::setSwapVBLInterval(uint interval)
{
#ifdef NL_OS_WINDOWS
	if(_Extensions.WGLEXTSwapControl)
	{
		wglSwapIntervalEXT(interval);
	}
#endif
}

// ***************************************************************************
uint	CDriverGL::getSwapVBLInterval()
{
#ifdef NL_OS_WINDOWS
	if(_Extensions.WGLEXTSwapControl)
	{
		return wglGetSwapIntervalEXT();
	}
	else
		return 0;
#else
	return 0;
#endif
}

// ***************************************************************************
void	CDriverGL::enablePolygonSmoothing(bool smooth)
{
	if(smooth)
		glEnable(GL_POLYGON_SMOOTH);
	else
		glDisable(GL_POLYGON_SMOOTH);
	_PolygonSmooth= smooth;
}

// ***************************************************************************
bool	CDriverGL::isPolygonSmoothingEnabled() const
{
	return _PolygonSmooth;
}



// ***************************************************************************
void	CDriverGL::startProfileVBHardLock()
{
	if(_VBHardProfiling)
		return;

	// start
	_VBHardProfiles.clear();
	_VBHardProfiles.reserve(50);
	_VBHardProfiling= true;
	_CurVBHardLockCount= 0;
	_NumVBHardProfileFrame= 0;
}

// ***************************************************************************
void	CDriverGL::endProfileVBHardLock(vector<std::string> &result)
{
	if(!_VBHardProfiling)
		return;

	// Fill infos.
	result.clear();
	result.resize(_VBHardProfiles.size());
	for(uint i=0;i<_VBHardProfiles.size();i++)
	{
		const	uint tmpSize= 256;
		char	tmp[tmpSize];
		CVBHardProfile	&vbProf= _VBHardProfiles[i];
		const char	*vbName;
		if(vbProf.VBHard && !vbProf.VBHard->getName().empty())
		{
			vbName= vbProf.VBHard->getName().c_str();
		}
		else
		{
			vbName= "????";
		}
		// Display in ms.
		smprintf(tmp, tmpSize, "%16s%c: %2.3f ms", vbName, vbProf.Change?'*':' ', 
			CTime::ticksToSecond(vbProf.AccumTime)*1000 / _NumVBHardProfileFrame);

		result[i]= tmp;
	}

	// clear.
	_VBHardProfiling= false;
	contReset(_VBHardProfiles);
}

// ***************************************************************************
void	CDriverGL::appendVBHardLockProfile(NLMISC::TTicks time, IVertexBufferHard *vb)
{
	// must allocate a new place?
	if(_CurVBHardLockCount>=_VBHardProfiles.size())
	{
		_VBHardProfiles.resize(_VBHardProfiles.size()+1);
		// set the original VBHard
		_VBHardProfiles[_CurVBHardLockCount].VBHard= vb;
	}

	// Accumulate.
	_VBHardProfiles[_CurVBHardLockCount].AccumTime+= time;
	// if change of VBHard for this chrono place
	if(_VBHardProfiles[_CurVBHardLockCount].VBHard != vb)
	{
		// flag, and set new
		_VBHardProfiles[_CurVBHardLockCount].VBHard= vb;
		_VBHardProfiles[_CurVBHardLockCount].Change= true;
	}

	// next!
	_CurVBHardLockCount++;
}


} // NL3D
