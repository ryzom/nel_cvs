/** \file driver_opengl.cpp
 * OpenGL driver implementation
 *
 * $Id: driver_opengl.cpp,v 1.5 2000/11/07 15:34:14 berenguier Exp $
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

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <gl/gl.h>
#include "driver_opengl.h"

// --------------------------------------------------

namespace NL3D
{

	uint CDriverGL::_Registered=0;

// --------------------------------------------------

__declspec(dllexport) IDriver* NL3D_createIDriverInstance(void)
{
	return( new CDriverGL );
}

// --------------------------------------------------

#ifdef WIN32
static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	// ---
	case WM_DESTROY:
		break;
	// ---
	case WM_KEYDOWN:
		if (wParam==VK_ESCAPE)
		{
			PostQuitMessage(0x01);
		}
		break;
	// ---
	case WM_COMMAND:
		break;
	// ---
	default:
		break;
	// ---
	}
	return( DefWindowProc(hWnd, message, wParam, lParam) );
}
#endif

// --------------------------------------------------

bool CDriverGL::init(void)
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
			return(false);
		}
		_Registered=1;
	}
#endif
	return(true);
}

// --------------------------------------------------

ModeList CDriverGL::enumModes()
{
	sint		n;
	DEVMODE		devmode;
	ModeList	ML;
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
	return(ML);
}

// --------------------------------------------------

bool CDriverGL::processMessages(void)
{
#ifdef WIN32
	MSG	msg;

	while ( PeekMessage(&msg,NULL,0,0,PM_NOREMOVE) )
	{
		if ( !GetMessage(&msg,NULL,0,0) )
		{ 
			return(false);
		}
		if (msg.message==WM_QUIT)
		{
			return(false);
		}
		TranslateMessage(&msg); 
		DispatchMessage(&msg);
	}
#endif
	return(true);
}

// --------------------------------------------------

bool CDriverGL::setDisplay(void* wnd, const GfxMode& mode)
{
	uint8					Depth;
#ifdef WIN32
	int						pf;

//	OutputDebugString("----- SETDISPLAY");
	if (wnd)
	{
		_hWnd=(HWND)wnd;
	}
	else
	{
		ULONG	WndFlags;
		RECT	WndRect;

		WndFlags=WS_OVERLAPPEDWINDOW+WS_CLIPCHILDREN+WS_CLIPSIBLINGS;
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
			return(false);
		}
		ShowWindow(_hWnd,SW_SHOW);
	}
	_hDC=GetDC(_hWnd);
	Depth=24;//GetDeviceCaps(_hDC,BITSPIXEL);
	// ---
	memset(&_pfd,0,sizeof(_pfd));
	_pfd.nSize        = sizeof(_pfd);
	_pfd.nVersion     = 1;
	_pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	_pfd.iPixelType   = PFD_TYPE_RGBA;
	_pfd.cColorBits   = (char)Depth;
	_pfd.cDepthBits   = 16;
	_pfd.iLayerType	  = PFD_MAIN_PLANE;
//	OutputDebugString("----- Chosse pixel format");
	pf=ChoosePixelFormat(_hDC,&_pfd);
	if (!pf) 
	{
		return(false);
	} 
//	OutputDebugString("----- set pixel format");
	if ( !SetPixelFormat(_hDC,pf,&_pfd) ) 
	{
		return(false);
	} 
//	OutputDebugString("----- create context");
    _hRC=wglCreateContext(_hDC);
//	OutputDebugString("----- make current");
    wglMakeCurrent(_hDC,_hRC);
#endif
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
	glDisable(GL_CULL_FACE);
	glEnable(GL_DITHER);
	glDisable(GL_FOG);
	glDisable(GL_LIGHTING);
	glEnable(GL_LINE_SMOOTH);
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	return(true);
}

// --------------------------------------------------

bool CDriverGL::activate(void)
{
	HGLRC hglrc=wglGetCurrentContext();
	if (hglrc!=_hRC)
	{
		wglMakeCurrent(_hDC,_hRC);
	}
	return(true);
}

// --------------------------------------------------

bool CDriverGL::clear2D(CRGBA& rgba)
{
	glClearColor((float)rgba.R/255.0f,(float)rgba.G/255.0f,(float)rgba.B/255.0f,(float)rgba.A/255.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	return(true);
}

// --------------------------------------------------

bool CDriverGL::clearZBuffer(float zval)
{
	glClearDepth(zval);
	glClear(GL_DEPTH_BUFFER_BIT);
	return(true);
}

// --------------------------------------------------

bool CDriverGL::setupTexture(CTexture& tex)
{
	if (tex.DrvInfos==NULL)
	{
// C++ Hardcore style :
//		tex.DrvInfos=new CTextureDrvInfosGL;
//		ITextureDrvInfos* iinfos=static_cast<ITextureDrvInfos*>(tex.DrvInfos);
//		CTextureDrvInfosGL* infos=static_cast<CTextureDrvInfosGL*>(iinfos);

// C++ standard/intelligent style :
		CTextureDrvInfosGL* infos = new CTextureDrvInfosGL;
		tex.DrvInfos=infos;

// C style :
//		tex.DrvInfos= static_cast<CTextureDrvInfosGL*>(tex.DrvInfos);
//		tex.DrvInfos=new CTextureDrvInfosGL;
//		CTextureDrvInfosGL* infos=(CTextureDrvInfosGL*)((ITextureDrvInfos*)(tex.DrvInfos));
		glGenTextures(1,&infos->ID);
		glBindTexture(GL_TEXTURE_2D,infos->ID);
		glPixelStorei(GL_UNPACK_ALIGNMENT,1);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
		glTexImage2D(GL_TEXTURE_2D,0,4,tex.getWidth(),tex.getHeight(),0,GL_RGBA,GL_UNSIGNED_BYTE,tex.getDataPointer());
	}
	return(true);
}

// --------------------------------------------------

bool CDriverGL::_setupVertexBuffer(CVertexBuffer& VB)
{
	return(true);
}

bool CDriverGL::activeVertexBuffer(CVertexBuffer& VB)
{
	uint32	flags;

	if (VB.DrvInfos==NULL)
	{
		if ( !_setupVertexBuffer(VB) )
		{
			return(false);
		}
	}

	glEnable(GL_VERTEX_ARRAY);
	glVertexPointer(3,GL_FLOAT,VB.getVertexSize(),VB.getVertexCoordPointer());

	flags=VB.getFlags();

	if (flags & IDRV_VF_RGBA)
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

	return(true);
}

// --------------------------------------------------

bool CDriverGL::render(CPrimitiveBlock& PB, CMaterial& Mat)
{
	// Temp Antoine.
	/*ITextureDrvInfos*	iinfos;
	CTextureDrvInfosGL*	infosgl;

	iinfos=(Mat.getTexture(0)).DrvInfos;
	infosgl=static_cast<CTextureDrvInfosGL*>(iinfos);
	glBindTexture(GL_TEXTURE_2D,infosgl->ID);
	*/

	// Temp YOYO.
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glDisable(GL_TEXTURE_2D);

	glDrawElements(GL_TRIANGLES,3*PB.getNumTri(),GL_UNSIGNED_INT,PB.getTriPointer());
	return(true);
}

// --------------------------------------------------

bool CDriverGL::swapBuffers(void)
{
	if ( !SwapBuffers(_hDC) )	
	{
		return(false);
	}
	return(true);
}

// --------------------------------------------------

bool CDriverGL::release(void)
{
	wglMakeCurrent(NULL,NULL);
	ReleaseDC(_hWnd,_hDC);
	wglDeleteContext(_hRC);
	return(true);
}

// --------------------------------------------------

}
