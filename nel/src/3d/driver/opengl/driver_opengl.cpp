/** \file driver_opengl.cpp
 * OpenGL driver implementation
 *
 * $Id: driver_opengl.cpp,v 1.17 2000/11/23 11:18:52 coutelas Exp $
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

#ifndef NL_OS_WINDOWS
#include <stdio.h>
#endif // NL_OS_WINDOWS
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
	// Get the driver pointer..
	CDriverGL *pDriver=(CDriverGL*)GetWindowLong (hWnd, GWL_USERDATA);
	if (pDriver)
	{
		// Process the message by the emitter
		pDriver->_EventEmitter.processMessage ((uint32)hWnd, message, wParam, lParam);
	}

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

bool CDriverGL::setDisplay(void* wnd, const GfxMode& mode)
{
	uint8					Depth;
#ifdef WIN32
	int						pf;

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
		SetWindowLong (_hWnd, GWL_USERDATA, (LONG)this);
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
		return(false);
	} 
	if ( !SetPixelFormat(_hDC,pf,&_pfd) ) 
	{
		return(false);
	} 
    _hRC=wglCreateContext(_hDC);
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
	glDisable(GL_LINE_SMOOTH);
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
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

bool CDriverGL::clear2D(CRGBA rgba)
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

	flags=VB.getVertexFormat();

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
	if ( !setupMaterial(Mat) )
	{
		return(false);
	}
	
	if(PB.getNumTri()!=0)
		glDrawElements(GL_TRIANGLES,3*PB.getNumTri(),GL_UNSIGNED_INT,PB.getTriPointer());
	if(PB.getNumQuad()!=0)
		glDrawElements(GL_QUADS,4*PB.getNumQuad(),GL_UNSIGNED_INT,PB.getQuadPointer());
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
	wglDeleteContext(_hRC);
	ReleaseDC(_hWnd,_hDC);
	return(true);
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
	nlassert (0);		// no!
#else
	static const char* icons[iconCount]=
	{
		"",
		"WAIT:\n",
		"QUESTION:\n",
		"HEY!\n",
		"",
		"WARNING!\n",
		"ERROR!\n",
		"INFORMATION:\n",
		"STOP:\n"
	};
	static const char* messages[typeCount]=
	{
		"Press any key...",
		"(O)k or (C)ancel ?",
		"(Y)es or (N)o ?",
		"(A)bort (R)etry (I)gnore ?",
		"(Y)es (N)o (C)ancel ?",
		"(R)etry (C)ancel ?"
	};
	printf ("%s%s\n%s", icons[icon], title, message);
	while (1)
	{
		printf ("\n%s", messages[type]);
		int c=getchar();
		if (type==okType)
			return okId;
		switch (c)
		{
		case 'O':
		case 'o':
			if ((type==okType)||(type==okCancelType))
				return okId;
			break;
		case 'C':
		case 'c':
			if ((type==yesNoCancelType)||(type==okCancelType)||(type==retryCancelType))
				return cancelId;
			break;
		case 'Y':
		case 'y':
			if ((type==yesNoCancelType)||(type==yesNoType))
				return yesId;
			break;
		case 'N':
		case 'n':
			if ((type==yesNoCancelType)||(type==yesNoType))
				return noId;
			break;
		case 'A':
		case 'a':
			if (type==abortRetryIgnoreType)
				return abortId;
			break;
		case 'R':
		case 'r':
			if (type==abortRetryIgnoreType)
				return retryId;
			break;
		case 'I':
		case 'i':
			if (type==abortRetryIgnoreType)
				return ignoreId;
			break;
		}
	}

#endif
	return okId;
}

// --------------------------------------------------
}
