/** \file driver_opengl.cpp
 * OpenGL driver implementation
 *
 * $Id: driver_opengl.cpp,v 1.86 2001/04/06 14:54:10 corvazier Exp $
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

#include "nel/misc/types_nl.h"

#ifdef NL_OS_WINDOWS

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

#include <string>

#else // NL_OS_UNIX

#define	GL_GLEXT_PROTOTYPES
#include <GL/glx.h>

#endif // NL_OS_UNIX

#include <vector>
#include <GL/gl.h>

#include "driver_opengl.h"
#include "nel/3d/tmp/viewport.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/light.h"
#include "nel/3d/primitive_block.h"
#include "nel/misc/rect.h"

using namespace std;
using namespace NLMISC;



// ***************************************************************************
// Flags for software vertex skinning.
#define	NL3D_DRV_SOFTSKIN_VNEEDCOMPUTE	3
#define	NL3D_DRV_SOFTSKIN_VMUSTCOMPUTE	1
#define	NL3D_DRV_SOFTSKIN_VCOMPUTED		0
// 3 means "vertex may need compute".
// 1 means "Primitive say vertex must be computed".
// 0 means "vertex is computed".



// ***************************************************************************
#ifdef NL_OS_WINDOWS
// dllmain::
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		initDebug();
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
const uint32		CDriverGL::ReleaseVersion = 0x5;

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

CDriverGL::CDriverGL()
{
#ifdef NL_OS_WINDOWS
	_FullScreen= false;
	_hWnd = NULL;
	_hRC = NULL;
	_hDC = NULL;
#endif // NL_OS_WINDOWS

	_CurrentMaterial=NULL;
	_Initialized = false;

	_FogEnabled= false;


	_MatrixSetupDirty= false;
	_ViewMatrixSetupDirty= false;
	_ModelViewMatrixDirty.resize(MaxModelMatrix);
	_ModelViewMatrixDirty.clearAll();
	_ModelViewMatrixDirtyPaletteSkin.resize(MaxModelMatrix);
	_ModelViewMatrixDirtyPaletteSkin.clearAll();
	

	_PaletteSkinHard= false;
	_CurrentNormalize= false;
}


// --------------------------------------------------

bool CDriverGL::init()
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
		wc.hIcon			= NULL;
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

bool CDriverGL::setDisplay(void *wnd, const GfxMode &mode) throw(EBadDisplay)
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
			ChangeDisplaySettings(&devMode, CDS_FULLSCREEN);
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
    wglMakeCurrent(_hDC,NULL);
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


#elif defined(NL_OS_UNIX)

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
	  GLX_BUFFER_SIZE, 16,
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
	attr.override_redirect = False;
	int attr_flags = CWOverrideRedirect | CWColormap | CWBackPixel;

	win = XCreateWindow (dpy, RootWindow(dpy, DefaultScreen(dpy)), 0, 0, mode.Width, mode.Height, 0, visual_info->depth, InputOutput, visual_info->visual, attr_flags, &attr);	

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
	size_hints.width = mode.Width;
	size_hints.height = mode.Height;
	size_hints.flags = PSize | PMinSize | PMaxSize;
	size_hints.min_width = mode.Width;
	size_hints.min_height = mode.Height;
	size_hints.max_width = mode.Width;
	size_hints.max_height = mode.Height;

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

#endif // NL_OS_WINDOWS


	// Driver caps.
	//=============

	// Retrieve the extensions for the current context.
	NL3D::registerGlExtensions(_Extensions);
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

	// skinning in hard??
	// TODO_HARDWARE_SKINNIG:
	_PaletteSkinHard= false;


	// Init OpenGL/Driver defaults.
	//=============================
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
	glDisable(GL_NORMALIZE);
	_CurrentNormalize= false;


	// Be always in EXTSeparateSpecularColor.
	if(_Extensions.EXTSeparateSpecularColor)
	{
		glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL_EXT, GL_SEPARATE_SPECULAR_COLOR_EXT);
	}



	// Activate the default texture environnments for all stages.
	//===========================================================
	for(sint stage=0;stage<getNbTextureStages(); stage++)
	{
		// init no texture.
		_CurrentTexture[stage]= NULL;
		glActiveTextureARB(GL_TEXTURE0_ARB+stage);
		glDisable(GL_TEXTURE_2D);
		
		// init default env.
		CMaterial::CTexEnv	env;	// envmode init to default.
		env.ConstantColor.set(255,255,255,255);
		activateTexEnvMode(stage, env);
		activateTexEnvColor(stage, env);
	}

	// Get num of light for this driver
	int numLight;
	glGetIntegerv (GL_MAX_LIGHTS, &numLight);
	_MaxDriverLight=(uint)numLight;
	if (_MaxDriverLight>MaxLight)
		_MaxDriverLight=MaxLight;

	// Reset the lights position flags
	for (uint i=0; i<MaxLight; i++)
		_LightEnable[i]=false;

	_Initialized = true;

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
	// 1. Retrieve/Create driver shader.
	//==================================
	if (!VB.DrvInfos)
	{
		VB.DrvInfos=new CVBDrvInfosGL;
		// insert into driver list. (so it is deleted when driver is deleted).
		_VBDrvInfos.push_back(VB.DrvInfos);
	}
	CVBDrvInfosGL	*vbInf= static_cast<CVBDrvInfosGL*>((IVBDrvInfos*)(VB.DrvInfos));

	// 2. If necessary, do modifications.
	//==================================
	if( VB.getTouchFlags()!=0 )
	{
		// Software and Skinning: must allocate PostRender Vertices and normals.
		if(!_PaletteSkinHard && (VB.getVertexFormat() & IDRV_VF_PALETTE_SKIN)==IDRV_VF_PALETTE_SKIN )
		{
			// Must reallocate post-rendered vertices/normals Flags.
			vbInf->SoftSkinFlags.resize(VB.getNumVertices());
			// Must reallocate post-rendered vertices/normals.
			vbInf->SoftSkinVertices.resize(VB.getNumVertices());
			// Normals onli if nomal enabled.
			if(VB.getVertexFormat() & IDRV_VF_NORMAL)
				vbInf->SoftSkinNormals.resize(VB.getNumVertices());
		}

		// OK!
		VB.resetTouchFlags();
	}


	return true;
}

bool CDriverGL::activeVertexBuffer(CVertexBuffer& VB, uint first, uint end)
{
	uint32	flags;

	if (VB.DrvInfos==NULL && !setupVertexBuffer(VB))
		return false;

	if (VB.getNumVertices()==0)
		return true;

	nlassert(end<=VB.getNumVertices());
	nlassert(first<=end);

	// Get VB flags, to setup matrixes and arrays.
	flags=VB.getVertexFormat();
	// Get VB drv infos.
	CVBDrvInfosGL	*vbInf= static_cast<CVBDrvInfosGL*>((IVBDrvInfos*)(VB.DrvInfos));


	// Skin mode.
	// NB: this test either if palette skin is enabled, or normal skinning is enabled.
	bool	skinning= (flags & IDRV_VF_PALETTE_SKIN)!=0;
	// NB: this test if palette skin is enabled.
	bool	paletteSkinning= (flags & IDRV_VF_PALETTE_SKIN)==IDRV_VF_PALETTE_SKIN;


	// 0. Setup Matrixes.
	//===================

	// Check if view matrix has been modified
	if (_ViewMatrixSetupDirty)
		// Recompute stuff touched by the view matrix
		cleanViewMatrix ();

	// Check view matrix is good
	nlassert (_ViewMatrixSetupDirty==false);

	// Just to inform render*() that Matrix mode is OK.
	_MatrixSetupDirty= false;

	// General case: no software skinning.
	_CurrentSoftSkinFlags= NULL;

	// No Skinning at all??. 
	//==============
	if ( !skinning )
	{
		if(_ModelViewMatrixDirty[0])
		{
			_ModelViewMatrixDirty.clear(0);
			// By default, the first model matrix is active
			glLoadMatrixf( _ModelViewMatrix[0].get() );
		}
	}
	// Palette Skinning??
	//==============
	else if ( paletteSkinning )
	{
		if(_PaletteSkinHard)
		{
			// TODO_HARDWARE_SKINNIG: setup vertex program.
			// NB: must test _ModelViewMatrixDirtyPaletteSkin...
		}
		else
		{
			// setup identity
			glLoadMatrixf(CMatrix::Identity.get());
			
			// in software, we must test Model View Matrixes flags (_ModelViewMatrixDirtyPaletteSkin), to
			// compute M-1t matrices fro normals.
			for(sint i=0;i<IDriver::MaxModelMatrix;i++)
			{
				if(_ModelViewMatrixDirtyPaletteSkin[i])
				{
					_ModelViewMatrixDirtyPaletteSkin.clear(i);

					CMatrix		&mview= _ModelViewMatrix[i];
					CMatrix		&mviewNormal= _ModelViewMatrixNormal[i];
					// copy only the rot matrix.
					mviewNormal.setRot(mview);
					// If matrix has scale...
					if(mviewNormal.hasScalePart())
					{
						// Must compute the transpose of the invert matrix. (10 times slower if not uniform scale!!)
						mviewNormal.invert();
						mviewNormal.transpose3x3();
					}
					// else, no need to do it since transpose==inverse.
				}
			}


			// Compute vertices/normales.
			// NB: in software, skinning compute is done at render, since we don't know, at activeVB(), what
			// vertex will be used.

			// reset flags, only for given vertex range.
			// NL3D_DRV_SOFTSKIN_VNEEDCOMPUTE==3.
			memset(&vbInf->SoftSkinFlags[first], NL3D_DRV_SOFTSKIN_VNEEDCOMPUTE, end-first);

			// For render*() to know how they must skin...
			_CurrentSoftSkinFlags= &vbInf->SoftSkinFlags[0];
			_CurrentSoftSkinSrc= (uint8*) VB.getVertexCoordPointer();
			_CurrentSoftSkinSrcStride= VB.getVertexSize();
			_CurrentSoftSkinFirst= first;
			_CurrentSoftSkinEnd= end;
			_CurrentSoftSkinPaletteSkinOff= VB.getPaletteSkinOff();
			_CurrentSoftSkinWeightOff= VB.getWeightOff(0);
			_CurrentSoftSkinVectorDst= &(vbInf->SoftSkinVertices[0]);
			// Normal ptr.
			if(flags & IDRV_VF_NORMAL)
			{
				_CurrentSoftSkinNormalDst= &(vbInf->SoftSkinNormals[0]);
				_CurrentSoftSkinNormalOff= VB.getNormalOff();
			}
			else
			{
				_CurrentSoftSkinNormalDst= NULL;
			}
		}
	}
	// Non Paletted skinning.
	//==============
	else
	{
		// TODO_SOFTWARE_SKINNIG: We must make the skinning by software. (maybe one day :) ).
		// TODO_HARDWARE_SKINNIG: we must make the skinning by hardware (Radeon, NV20 vertexprogram).

		// For now, even if weight number is better than 2, do the skinning in EXTVertexWeighting 2 matrix (if possible)

		// Choose an extension to setup a second matrix
		if (_Extensions.EXTVertexWeighting)
		{
			if(_ModelViewMatrixDirty[0])
			{
				_ModelViewMatrixDirty.clear(0);
				// By default, the first model matrix is active
				glLoadMatrixf( _ModelViewMatrix[0].get() );
			}

			if(_ModelViewMatrixDirty[1])
			{
				_ModelViewMatrixDirty.clear(1);
				// Active the second model matrix
				glMatrixMode(GL_MODELVIEW1_EXT);
				// Set it
				glLoadMatrixf( _ModelViewMatrix[1].get() );
				// Active first model matrix
				glMatrixMode(GL_MODELVIEW);
			}
		}
		// else nothing!!!
	}



	// 1. Special Normalize.
	//======================
	// NB: must enable GL_NORMALIZE when skinning is enabled or when ModelView has scale.
	if(skinning || _ModelViewMatrix[0].hasScalePart())
	{
		if(!_CurrentNormalize)
		{
			_CurrentNormalize= true;
			glEnable(GL_NORMALIZE);
		}
	}
	else
	{
		if(_CurrentNormalize)
		{
			_CurrentNormalize= false;
			glDisable(GL_NORMALIZE);
		}
	}



	// 2. Setup Arrays.
	//===================


	// Setup Vertex / Normal.
	//=======================
	// if software palette skinning: setup correct Vertex/Normal array.
	if(paletteSkinning && !_PaletteSkinHard)
	{
		// Must point on computed Vertex/Normal array.
		glEnable(GL_VERTEX_ARRAY);
		// array is compacted.
		glVertexPointer(3,GL_FLOAT,0,&(*vbInf->SoftSkinVertices.begin()));

		// Check for normal param in vertex buffer
		if (flags & IDRV_VF_NORMAL)
		{
			glEnableClientState(GL_NORMAL_ARRAY);
			// array is compacted.
			glNormalPointer(GL_FLOAT,0,&(*vbInf->SoftSkinNormals.begin()));
		}
		else
			glDisableClientState(GL_NORMAL_ARRAY);
	}
	else
	{
		glEnable(GL_VERTEX_ARRAY);
		glVertexPointer(3,GL_FLOAT,VB.getVertexSize(),VB.getVertexCoordPointer());

		// Check for normal param in vertex buffer
		if (flags & IDRV_VF_NORMAL)
		{
			glEnableClientState(GL_NORMAL_ARRAY);
			glNormalPointer(GL_FLOAT,VB.getVertexSize(),VB.getNormalCoordPointer());
		}
		else
			glDisableClientState(GL_NORMAL_ARRAY);
	}


	// Setup Vertex Weight.
	//=======================
	// disable vertex weight
	bool disableVertexWeight=_Extensions.EXTVertexWeighting;

	// if skinning not paletted...
	if ( skinning && !paletteSkinning)
	{
		// 4 weights ?
		if (flags & IDRV_VF_W[3])
		{
			// TODO_HARDWARE_SKINNIG: we must implement the 4 matrices mode by hardware (Radeon, NV20).
			// TODO_SOFTWARE_SKINNIG: we must implement the 4 matrices mode by software.
		}
		// 3 weights ?
		else if (flags & IDRV_VF_W[2])
		{
			// TODO_HARDWARE_SKINNIG: we must implement the 3 matrices mode by hardware (Radeon, NV20).
			// TODO_SOFTWARE_SKINNIG: we must implement the 3 matrices mode by software.
		}
		// 2 weights ?
		else
		{
			// Check if vertex weighting extension is available
			if (_Extensions.EXTVertexWeighting)
			{
				// Active skinning
				glEnable (GL_VERTEX_WEIGHTING_EXT);

				// Don't disable it
				disableVertexWeight=false;

				// Setup
				glEnableClientState(GL_VERTEX_WEIGHT_ARRAY_EXT);
				glVertexWeightPointerEXT(1,GL_FLOAT,VB.getVertexSize(),VB.getWeightPointer());
			}
			else
			{
				// TODO_HARDWARE_SKINNIG: we must implement the 2 matrices mode by hardware (Radeon, NV20).
				// TODO_SOFTWARE_SKINNIG: we must implement the 2 matrices mode by software.
			}
		}
	}

	// Disable vertex weight
	if (disableVertexWeight)
		glDisable (GL_VERTEX_WEIGHTING_EXT);


	// Setup Color / UV.
	//==================
	// Check for color param in vertex buffer
	if (flags & IDRV_VF_COLOR)
	{
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4,GL_UNSIGNED_BYTE,VB.getVertexSize(),VB.getColorPointer());
	}
	else
		glDisableClientState(GL_COLOR_ARRAY);

	// Active UVs.
	for(sint i=0; i<getNbTextureStages(); i++)
	{
		glClientActiveTextureARB(GL_TEXTURE0_ARB+i);
		if (flags & IDRV_VF_UV[i])
		{
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2,GL_FLOAT,VB.getVertexSize(),VB.getTexCoordPointer(0,i));
		}
		else
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}


	return true;
}


// --------------------------------------------------
bool		CDriverGL::activeVertexBuffer(CVertexBuffer& VB)
{
	return activeVertexBuffer(VB, 0, VB.getNumVertices());
}


// --------------------------------------------------

bool CDriverGL::render(CPrimitiveBlock& PB, CMaterial& Mat)
{
	// Check user code :)
	nlassert(!_MatrixSetupDirty);

	if ( !setupMaterial(Mat) )
		return false;


	// test if VB software skinning.
	//==============================
	if(_CurrentSoftSkinFlags)
	{
		uint32	*pIndex;
		uint	nIndex;

		// This may be better to flags in 2 pass (first traverse primitives, then test vertices).
		// Better sol for BTB..., because number of tests are divided by 6 (for triangles).

		// First, for all prims, indicate which vertex we must compute.
		// nothing if not already computed (ie 0), because 0&1==0.
		// Lines.
		pIndex= (uint32*)PB.getLinePointer();
		nIndex= PB.getNumLine()*2;
		for(;nIndex>0;nIndex--, pIndex++)
			_CurrentSoftSkinFlags[*pIndex]&= NL3D_DRV_SOFTSKIN_VMUSTCOMPUTE;
		// Tris.
		pIndex= (uint32*)PB.getTriPointer();
		nIndex= PB.getNumTri()*3;
		for(;nIndex>0;nIndex--, pIndex++)
			_CurrentSoftSkinFlags[*pIndex]&= NL3D_DRV_SOFTSKIN_VMUSTCOMPUTE;
		// Quads.
		pIndex= (uint32*)PB.getQuadPointer();
		nIndex= PB.getNumQuad()*4;
		for(;nIndex>0;nIndex--, pIndex++)
			_CurrentSoftSkinFlags[*pIndex]&= NL3D_DRV_SOFTSKIN_VMUSTCOMPUTE;


		// Second, traverse All vertices in range, testing if we must compute those vertices.
		refreshSoftwareSkinning();
	}


	// render primitives.
	//==============================

	if(PB.getNumTri()!=0)
		glDrawElements(GL_TRIANGLES,3*PB.getNumTri(),GL_UNSIGNED_INT,PB.getTriPointer());
	if(PB.getNumQuad()!=0)
		glDrawElements(GL_QUADS,4*PB.getNumQuad(),GL_UNSIGNED_INT,PB.getQuadPointer());
	if(PB.getNumLine()!=0)
		glDrawElements(GL_LINES,2*PB.getNumLine(),GL_UNSIGNED_INT,PB.getLinePointer());

	return true;
}


// --------------------------------------------------
void	CDriverGL::renderTriangles(CMaterial& Mat, uint32 *tri, uint32 ntris)
{
	// Check user code :)
	nlassert(!_MatrixSetupDirty);

	if ( !setupMaterial(Mat) )
		return;

	
	// test if VB software skinning.
	//==============================
	if(_CurrentSoftSkinFlags)
	{
		uint32	*pIndex;
		uint	nIndex;

		// see render() for explanation.
		// First, for all prims, indicate which vertex we must compute.
		// nothing if not already computed (ie 0), because 0&1==0.
		// Tris.
		pIndex= tri;
		nIndex= ntris*3;
		for(;nIndex>0;nIndex--, pIndex++)
			_CurrentSoftSkinFlags[*pIndex]&= NL3D_DRV_SOFTSKIN_VMUSTCOMPUTE;

		// Second, traverse All vertices in range, testing if we must compute those vertices.
		refreshSoftwareSkinning();
	}


	// render primitives.
	//==============================
	if(ntris!=0)
		glDrawElements(GL_TRIANGLES,3*ntris,GL_UNSIGNED_INT, tri);
}


// --------------------------------------------------

bool CDriverGL::swapBuffers()
{
#ifdef NL_OS_WINDOWS
	SwapBuffers(_hDC);
#else // NL_OS_WINDOWS
	glXSwapBuffers(dpy, win);
#endif // NL_OS_WINDOWS

	// Activate the default texture environnments for all stages.
	//===========================================================
	// This is not a requirement, but it ensure a more stable state each frame.
	// (well, maybe the good reason is "it hides much more the bugs"  :o) ).
	for(sint stage=0;stage<getNbTextureStages(); stage++)
	{
		// init no texture.
		_CurrentTexture[stage]= NULL;
		glActiveTextureARB(GL_TEXTURE0_ARB+stage);
		glDisable(GL_TEXTURE_2D);
		
		// init default env.
		CMaterial::CTexEnv	env;	// envmode init to default.
		env.ConstantColor.set(255,255,255,255);
		activateTexEnvMode(stage, env);
		activateTexEnvColor(stage, env);
	}
	glActiveTextureARB(GL_TEXTURE0_ARB);


	return true;
}

// --------------------------------------------------

uint CDriverGL::getNumMatrix()
{
	// TODO_HARDWARE_SKINNIG: we must implement the 4 matrices mode by hardware (Radeon, NV20).
	// TODO_SOFTWARE_SKINNIG: we must implement the 4 matrices mode by software.

	// If GL_Wertex_weighting_EXT is available..
	if (_Extensions.EXTVertexWeighting)
		return 2;
	// Else, software..
	else
		return 1;
}

// --------------------------------------------------

bool	CDriverGL::supportPaletteSkinning()
{
	return _PaletteSkinHard;
}


// --------------------------------------------------

bool CDriverGL::release()
{
	// release only if the driver was initialized
	if (!_Initialized) return true;

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
#elif defined (NL_OS_UNIX)
	return;
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
	// NeL window coordinate to MSWindows coordinates
	RECT client;
	GetClientRect (_hWnd, &client);
	POINT pt;
	pt.x = (int)((float)(client.right-client.left)*x);
	pt.y = (int)((float)(client.bottom-client.top)*(1.0f-y));
	ClientToScreen (_hWnd, &pt);
	SetCursorPos(pt.x, pt.y);
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
	RECT client;
	GetClientRect (_hWnd, &client);
	width = (uint32)(client.right-client.left);
	height = (uint32)(client.bottom-client.top);
#elif defined (NL_OS_UNIX)
	XWindowAttributes xwa;
	XGetWindowAttributes (dpy, win, &xwa);
	width = (uint32) xwa.width;
	height = (uint32) xwa.height;
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
		vector<uint8> &d = bitmap.getPixels ();
		glReadPixels (rect.X, rect.Y, rect.Width, rect.Height, GL_RGBA, GL_UNSIGNED_BYTE, &(d[0]));
	}
}


void			CDriverGL::getZBufferPart (std::vector<float>  &zbuffer, NLMISC::CRect &rect)
{
	zbuffer.clear();

	if(clipRect(rect))
	{
		zbuffer.resize(rect.Width*rect.Height);
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


void CDriverGL::setPolygonMode (TPolygonMode mode)
{
	IDriver::setPolygonMode (mode);

	// Set the polygon mode
	switch (_PolygonMode)
	{
	case Filled:
		glPolygonMode (GL_FRONT, GL_FILL);
		break;
	case Line:
		glPolygonMode (GL_FRONT, GL_LINE);
		break;
	case Point:
		glPolygonMode (GL_FRONT, GL_POINT);
		break;
	}
}


bool			CDriverGL::fogEnabled()
{
	return _FogEnabled;
}

void			CDriverGL::enableFog(bool enable)
{
	_FogEnabled= enable;
	if(enable)
		glEnable(GL_FOG);
	else
		glDisable(GL_FOG);
}

void			CDriverGL::setupFog(float start, float end, CRGBA color)
{
	glFogf(GL_FOG_MODE, GL_LINEAR);
	glFogf(GL_FOG_START, start);
	glFogf(GL_FOG_END, end);

	GLfloat		col[4];
	col[0]= color.R/255.0f;
	col[1]= color.G/255.0f;
	col[2]= color.B/255.0f;
	col[3]= color.A/255.0f;

	glFogfv(GL_FOG_COLOR, col);
}

// ***************************************************************************

void			CDriverGL::computeSoftwareVertexSkinning(uint8 *pSrc, CVector *pDst)
{
	CMatrix		*pMat;

	// TODO_OPTIMIZE: SSE verion...

	CVector			*srcVec= (CVector*)pSrc;
	CPaletteSkin	*srcPal= (CPaletteSkin*)(pSrc + _CurrentSoftSkinPaletteSkinOff);
	float			*srcWgt= (float*)(pSrc + _CurrentSoftSkinWeightOff);

	// checks indices.
	nlassert(srcPal->MatrixId[0]<IDriver::MaxModelMatrix);
	nlassert(srcPal->MatrixId[1]<IDriver::MaxModelMatrix);
	nlassert(srcPal->MatrixId[2]<IDriver::MaxModelMatrix);
	nlassert(srcPal->MatrixId[3]<IDriver::MaxModelMatrix);


	// Sum influences.
	pDst->set(0,0,0);

	// 0th matrix influence.
	pMat= _ModelViewMatrix + srcPal->MatrixId[0];
	*pDst+= pMat->mulPoint(*srcVec) * srcWgt[0];
	// 1th matrix influence.
	pMat= _ModelViewMatrix + srcPal->MatrixId[1];
	*pDst+= pMat->mulPoint(*srcVec) * srcWgt[1];
	// 2th matrix influence.
	pMat= _ModelViewMatrix + srcPal->MatrixId[2];
	*pDst+= pMat->mulPoint(*srcVec) * srcWgt[2];
	// 3th matrix influence.
	pMat= _ModelViewMatrix + srcPal->MatrixId[3];
	*pDst+= pMat->mulPoint(*srcVec) * srcWgt[3];

}

// ***************************************************************************

void			CDriverGL::computeSoftwareNormalSkinning(uint8 *pSrc, CVector *pDst)
{
	CMatrix		*pMat;

	// TODO_OPTIMIZE: SSE verion...

	CVector			*srcNormal= (CVector*)(pSrc + _CurrentSoftSkinNormalOff);
	CPaletteSkin	*srcPal= (CPaletteSkin*)(pSrc + _CurrentSoftSkinPaletteSkinOff);
	float			*srcWgt= (float*)(pSrc + _CurrentSoftSkinWeightOff);

	// checks indices.
	nlassert(srcPal->MatrixId[0]<IDriver::MaxModelMatrix);
	nlassert(srcPal->MatrixId[1]<IDriver::MaxModelMatrix);
	nlassert(srcPal->MatrixId[2]<IDriver::MaxModelMatrix);
	nlassert(srcPal->MatrixId[3]<IDriver::MaxModelMatrix);

	
	// Sum influences.
	pDst->set(0,0,0);

	// 0th matrix influence.
	pMat= _ModelViewMatrixNormal + srcPal->MatrixId[0];
	*pDst+= pMat->mulVector(*srcNormal) * srcWgt[0];
	// 1th matrix influence.
	pMat= _ModelViewMatrixNormal + srcPal->MatrixId[1];
	*pDst+= pMat->mulVector(*srcNormal) * srcWgt[1];
	// 2th matrix influence.
	pMat= _ModelViewMatrixNormal + srcPal->MatrixId[2];
	*pDst+= pMat->mulVector(*srcNormal) * srcWgt[2];
	// 3th matrix influence.
	pMat= _ModelViewMatrixNormal + srcPal->MatrixId[3];
	*pDst+= pMat->mulVector(*srcNormal) * srcWgt[3];
}

// ***************************************************************************

void	CDriverGL::refreshSoftwareSkinning()
{
	// traverse All vertices in range, testing if we must compute those vertices.
	uint8		*pSrc= _CurrentSoftSkinSrc + _CurrentSoftSkinFirst*_CurrentSoftSkinSrcStride;
	uint8		*pFlag= _CurrentSoftSkinFlags + _CurrentSoftSkinFirst;
	CVector		*pVectorDst= _CurrentSoftSkinVectorDst + _CurrentSoftSkinFirst;
	CVector		*pNormalDst= _CurrentSoftSkinNormalDst + _CurrentSoftSkinFirst;
	uint		size= _CurrentSoftSkinEnd - _CurrentSoftSkinFirst;
	for(;size>0;size--)
	{
		// If we must compute this vertex.
		if(*pFlag==NL3D_DRV_SOFTSKIN_VMUSTCOMPUTE)
		{
			// Flag this vertex as computed.
			*pFlag=NL3D_DRV_SOFTSKIN_VCOMPUTED;

			// compute vertex part.
			computeSoftwareVertexSkinning(pSrc, pVectorDst);

			// compute normal part.
			if(_CurrentSoftSkinNormalDst)
			{
				computeSoftwareNormalSkinning(pSrc, pNormalDst);
			}
		}

		pSrc+= _CurrentSoftSkinSrcStride;
		pFlag++;
		pVectorDst++;
		pNormalDst++;
	}
}

// ***************************************************************************

void				CDriverGL::cleanViewMatrix ()
{
	// Should be dirty
	nlassert (_ViewMatrixSetupDirty);
	
	// First light
	bool first=true;

	// For each lights
	for (uint i=0; i<_MaxDriverLight; i++)
	{
		// Is this light enabled ?
		if (_LightEnable[i])
		{
			// If first light
			if (first)
			{
				first=false;

				// Push the matrix
				glPushMatrix ();

				// Load the view matrix
				glLoadMatrixf (_ViewMtx.get());
			}

			// Light is directionnal ?
			if (_LightMode[i]==(uint)CLight::DirectionalLight)
			{
				// GL vector
				GLfloat vectorGL[4];

				// Set the GL array
				vectorGL[0]=-_WorldLightDirection[i].x;
				vectorGL[1]=-_WorldLightDirection[i].y;
				vectorGL[2]=-_WorldLightDirection[i].z;
				vectorGL[3]=0.f;
	
				// Set it
				glLightfv (GL_LIGHT0+i, GL_POSITION, vectorGL);
			}

			// Spotlight ?
			if (_LightMode[i]==(uint)CLight::SpotLight)
			{
				// GL vector
				GLfloat vectorGL[4];

				// Set the GL array
				vectorGL[0]=_WorldLightDirection[i].x;
				vectorGL[1]=_WorldLightDirection[i].y;
				vectorGL[2]=_WorldLightDirection[i].z;
	
				// Set it
				glLightfv (GL_LIGHT0+i, GL_SPOT_DIRECTION, vectorGL);
			}

			// Position 
			if (_LightMode[i]!=(uint)CLight::DirectionalLight)
			{
				// GL vector
				GLfloat vectorGL[4];

				// Set the GL array
				vectorGL[0]=_WorldLightPos[i].x;
				vectorGL[1]=_WorldLightPos[i].y;
				vectorGL[2]=_WorldLightPos[i].z;
				vectorGL[3]=1.f;
	
				// Set it
				glLightfv (GL_LIGHT0+i, GL_POSITION, vectorGL);
			}
		}
	}

	// Pop old matrix
	if (!first)
		glPopMatrix ();

	// Clean flag
	_ViewMatrixSetupDirty=false;
}

} // NL3D
