/** \file driver_opengl.cpp
 * OpenGL driver implementation
 *
 * $Id: driver_opengl.cpp,v 1.31 2000/12/18 09:30:42 lecroart Exp $
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

#include "nel/misc/types_nl.h"

#ifdef NL_OS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#endif // NL_OS_WINDOWS

#include <GL/gl.h>
#include "driver_opengl.h"
#include "nel/3d/viewport.h"

using namespace NLMISC;

namespace NL3D
{

#ifdef NL_OS_WINDOWS
uint CDriverGL::_Registered=0;
#endif // NL_OS_WINDOWS

// Version of the driver. Not the interface version!! Increment when implementation of the driver change.
const uint32		CDriverGL::ReleaseVersion = 0x0;

#ifdef NL_OS_WINDOWS
__declspec(dllexport) IDriver* NL3D_createIDriverInstance ()
{
	return new CDriverGL;
}

__declspec(dllexport) uint32 NL3D_interfaceVersion ()
{
	return IDriver::InterfaceVersion;
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Get the driver pointer..
	CDriverGL *pDriver=(CDriverGL*)GetWindowLong (hWnd, GWL_USERDATA);
	if (pDriver != NULL)
	{
		// Process the message by the emitter
		pDriver->_EventEmitter.processMessage ((uint32)hWnd, message, wParam, lParam);
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}
#endif // NL_OS_WINDOWS

CDriverGL::CDriverGL()
{
#ifdef NL_OS_WINDOWS
	_FullScreen= false;
	_hWnd = NULL;
	_hRC = NULL;
	_hDC = NULL;
#endif // NL_OS_WINDOWS

	_CurrentMaterial=NULL;
	_CurrentTexture[0]= NULL;
	_CurrentTexture[1]= NULL;
	_CurrentTexture[2]= NULL;
	_CurrentTexture[3]= NULL;
}


// --------------------------------------------------

bool CDriverGL::init()
{
#ifdef WIN32
	WNDCLASS		wc;

	if (!_Registered)
	{
		memset(&wc,0,sizeof(wc));
		wc.style			= CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc		= (WNDPROC)WndProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= GetModuleHandle(NULL);
		wc.hIcon			= NULL;
		wc.hCursor			= NULL;
		wc.hbrBackground	= WHITE_BRUSH;
		wc.lpszClassName	= "NLClass";
		wc.lpszMenuName		= NULL;
		if ( !RegisterClass(&wc) ) 
		{
			return false;
		}
		_Registered=1;
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

bool CDriverGL::setDisplay(void* wnd, const GfxMode& mode)
{
#ifdef NL_OS_WINDOWS
	uint8					Depth;
	int						pf;

	if (wnd)
	{
		_hWnd=(HWND)wnd;
	}
	else
	{
		ULONG	WndFlags;
		RECT	WndRect;

		_FullScreen= false;
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
			devMode.dmFields= DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
			devMode.dmPelsWidth= mode.Width;
			devMode.dmPelsHeight= mode.Height;
			devMode.dmBitsPerPel= mode.Depth;
			ChangeDisplaySettings(&devMode, 0);
		}
		WndRect.left=0;
		WndRect.top=0;
		WndRect.right=mode.Width;
		WndRect.bottom=mode.Height;
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
		SetRect (&rc, 0, 0, mode.Width, mode.Height);
		AdjustWindowRectEx (&rc, GetWindowStyle (_hWnd), GetMenu (_hWnd) != NULL, GetWindowExStyle (_hWnd));
		SetWindowPos (_hWnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );

		ShowWindow(_hWnd,SW_SHOW);
	}
	_EventEmitter.setHWnd((uint32)_hWnd);
	_hDC=GetDC(_hWnd);
	Depth=GetDeviceCaps(_hDC,BITSPIXEL);
	// ---
	memset(&_pfd,0,sizeof(_pfd));
	_pfd.nSize        = sizeof(_pfd);
	_pfd.nVersion     = 1;
	_pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	_pfd.iPixelType   = PFD_TYPE_RGBA;
	_pfd.cColorBits   = (char)Depth;
	_pfd.cDepthBits   = 16;
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
#endif // NL_OS_WINDOWS
	glViewport(0,0,mode.Width,mode.Height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,mode.Width,mode.Height,0,-1.0f,1.0f);	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_AUTO_NORMAL);
	glDisable(GL_BLEND);
	glDisable(GL_COLOR_MATERIAL);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DITHER);
	glDisable(GL_FOG);
	glDisable(GL_LIGHTING);
	glDisable(GL_LINE_SMOOTH);
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glDepthFunc(GL_LEQUAL);
	return true;
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
#endif // NL_OS_WINDOWS
	return true;
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
	glClear(GL_DEPTH_BUFFER_BIT);
	return true;
}


// --------------------------------------------------

bool CDriverGL::setupVertexBuffer(CVertexBuffer& VB)
{
	// Do not create any drv infos for now...
	return true;
}

bool CDriverGL::activeVertexBuffer(CVertexBuffer& VB)
{
	uint32	flags;

	if (VB.DrvInfos==NULL && !setupVertexBuffer(VB))
		return false;

	glEnable(GL_VERTEX_ARRAY);
	glVertexPointer(3,GL_FLOAT,VB.getVertexSize(),VB.getVertexCoordPointer());

	flags=VB.getVertexFormat();

	if (flags & IDRV_VF_COLOR)
	{
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4,GL_UNSIGNED_BYTE,VB.getVertexSize(),VB.getColorPointer());
	}
	else
		glDisableClientState(GL_COLOR_ARRAY);

	if (flags & IDRV_VF_NORMAL)
	{
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT,VB.getVertexSize(),VB.getNormalCoordPointer());
	}
	else
		glDisableClientState(GL_NORMAL_ARRAY);

	if (flags & IDRV_VF_UV[0])
	{
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2,GL_FLOAT,VB.getVertexSize(),VB.getTexCoordPointer(0,0));
	}
	else
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	return true;
}

// --------------------------------------------------

bool CDriverGL::render(CPrimitiveBlock& PB, CMaterial& Mat)
{
	if ( !setupMaterial(Mat) )
		return false;
	
	if(PB.getNumTri()!=0)
		glDrawElements(GL_TRIANGLES,3*PB.getNumTri(),GL_UNSIGNED_INT,PB.getTriPointer());
	if(PB.getNumQuad()!=0)
		glDrawElements(GL_QUADS,4*PB.getNumQuad(),GL_UNSIGNED_INT,PB.getQuadPointer());
	if(PB.getNumLine()!=0)
		glDrawElements(GL_LINES,2*PB.getNumLine(),GL_UNSIGNED_INT,PB.getLinePointer());

	return true;
}


// --------------------------------------------------

bool CDriverGL::swapBuffers()
{
#ifdef NL_OS_WINDOWS
	return SwapBuffers(_hDC) == TRUE;
#else // NL_OS_WINDOWS
	return true;
#endif // NL_OS_WINDOWS
}

// --------------------------------------------------

bool CDriverGL::release()
{
	// Call IDriver::release() before, to destroy textures, shaders and VBs...
	IDriver::release();

#ifdef NL_OS_WINDOWS
	// Then delete.
	wglMakeCurrent(NULL,NULL);
	if (_hRC)
		wglDeleteContext(_hRC);
	if (_hWnd&&_hDC)
	{
		ReleaseDC(_hWnd,_hDC);
		DestroyWindow (_hWnd);
	}

	if(_FullScreen)
	{
		ChangeDisplaySettings(&_OldScreenMode, 0);
		_FullScreen= false;
	}

	_hRC=NULL;
	_hDC=NULL;
	_hWnd=NULL;
#endif // NL_OS_WINDOWS

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
	if (_hWnd)
	{
		// Get window rect
		RECT rect;
		GetClientRect (_hWnd, &rect);

		// Get viewport
		float x;
		float y;
		float width;
		float height;
		viewport.getValues (x, y, width, height);

		// Setup gl viewport
		int clientWidth=rect.right-rect.left;
		int clientHeight=rect.bottom-rect.top;
		int ix=(int)((float)clientWidth*x);
		clamp (ix, 0, clientWidth);
		int iy=(int)((float)clientHeight*y);
		clamp (iy, 0, clientHeight);
		int iwidth=(int)((float)clientWidth*width);
		clamp (iwidth, 0, clientWidth-ix);
		int iheight=(int)((float)clientHeight*height);
		clamp (iheight, 0, clientHeight-iy);
		glViewport (ix, iy, iwidth, iheight);
	}
#endif // NL_OS_WINDOWS
}


// --------------------------------------------------

void CDriverGL::showCursor(bool b)
{
#ifdef NL_OS_WINDOWS
	ShowCursor(b);
#endif // NL_OS_WINDOWS
}


// --------------------------------------------------

void CDriverGL::setMousePos(float x, float y)
{
#ifdef NL_OS_WINDOWS
	// NeL window coordinate to MSWindows coordinates
	RECT client;
	GetClientRect (_hWnd, &client);
	POINT pt;
	pt.x = (int)((float)(client.right-client.left)*x);
	pt.y = (int)((float)(client.bottom-client.top)*(1.0f-y));

	ClientToScreen (_hWnd, &pt);
	
	SetCursorPos(pt.x, pt.y);
#endif // NL_OS_WINDOWS
}

}
