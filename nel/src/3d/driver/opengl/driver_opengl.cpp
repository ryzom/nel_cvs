/** \file driver_opengl.cpp
 * OpenGL driver implementation
 *
 * $Id: driver_opengl.cpp,v 1.113 2001/08/07 15:01:47 vizerie Exp $
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
#include "nel/3d/viewport.h"
#include "nel/3d/scissor.h"
#include "3d/vertex_buffer.h"
#include "3d/light.h"
#include "3d/primitive_block.h"
#include "nel/misc/rect.h"

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
const uint32		CDriverGL::ReleaseVersion = 0x6;

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
	// Process the message by the emitter
	driver->_EventEmitter.setHWnd((uint32)hWnd);
	driver->_EventEmitter.processMessage ((uint32)hWnd, message, wParam, lParam);
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

CDriverGL::CDriverGL() : _AGPVertexArrayRange(this), _VRAMVertexArrayRange(this)
{

#ifdef NL_OS_WINDOWS
	_hWnd = NULL;
	_hRC = NULL;
	_hDC = NULL;

#elif defined (NL_OS_UNIX) // NL_OS_WINDOWS

#ifdef XF86VIDMODE
	// zero the old screen mode
	memset(&_OldScreenMode, 0, sizeof(_OldScreenMode));
#endif //XF86VIDMODE

#endif // NL_OS_UNIX

	_FullScreen= false;

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
	_CurrentGlNormalize= false;
	_ForceNormalize= false;

	_VertexMode= NL3D_VERTEX_MODE_NORMAL;

	_CurrentVertexArrayRange= NULL;
	_CurrentVertexBufferHard= NULL;

	_AllocatedTextureMemory= 0;
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

#ifdef XF86VIDMODE
	if (!mode.Windowed)
	{

		// Set window to the right size, map it to the display, and raise it
		// to the front
		XResizeWindow(dpy,win,mode.Width,mode.Height);
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
					nldebug("Available mode - %dx%d\n",mode.Width,mode.Height);
					if( (modes[i]->hdisplay == mode.Width) &&
						(modes[i]->vdisplay == mode.Height))
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
						nlinfo("Switching to mode %dx%d,\n",mode.Width, 
							   mode.Height);
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
							mode.Width,
							mode.Height);
				}
			}
		}
	}

#endif // XF86VIDMODE

#endif // NL_OS_UNIX


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
	// \todo yoyo: TODO_HARDWARE_SKINNIG:
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
	_CurrentGlNormalize= false;
	_ForceNormalize= false;

	// Be always in EXTSeparateSpecularColor.
	if(_Extensions.EXTSeparateSpecularColor)
	{
		glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL_EXT, GL_SEPARATE_SPECULAR_COLOR_EXT);
	}


	// Reset VertexArrayRange.
	_CurrentVertexArrayRange= NULL;
	_CurrentVertexBufferHard= NULL;
	if(_Extensions.NVVertexArrayRange)
	{
		// try to allocate 16Mo by default of AGP Ram.
		initVertexArrayRange(NL3D_DRV_VERTEXARRAY_AGP_INIT_SIZE, 0);
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


	// Reset profiling.
	_AllocatedTextureMemory= 0;
	_PrimitiveProfileIn.reset();
	_PrimitiveProfileOut.reset();

	return true;
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
	std::string	name= tex.getShareName();
	// append format Id of the texture.
	static char	fmt[256];
	smprintf(fmt, 256, "@Fmt:%d", (uint32)tex.getUploadFormat());
	name+= fmt;
	// append mipmap info
	if(tex.mipMapOn())
		name+= "@MMp:On";
	else
		name+= "@MMp:Off";
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
	glDepthMask(GL_TRUE);
	glClear(GL_DEPTH_BUFFER_BIT);
	return true;
}



// --------------------------------------------------

bool CDriverGL::swapBuffers()
{
	// Reset VertexArrayRange.
	if(_CurrentVertexArrayRange)
	{
		_CurrentVertexArrayRange->disable();
		_CurrentVertexArrayRange= NULL;
	}


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
		glDisable(GL_TEXTURE_CUBE_MAP_ARB);
		
		// init default env.
		CMaterial::CTexEnv	env;	// envmode init to default.
		env.ConstantColor.set(255,255,255,255);
		activateTexEnvMode(stage, env);
		activateTexEnvColor(stage, env);
	}
	glActiveTextureARB(GL_TEXTURE0_ARB);


	// Reset the profiling counter.
	_PrimitiveProfileIn.reset();
	_PrimitiveProfileOut.reset();


	// Reset VertexArrayRange.
	if(_CurrentVertexBufferHard)
	{
		_CurrentVertexBufferHard->disable();
	}


	return true;
}

// --------------------------------------------------

uint CDriverGL::getNumMatrix()
{
	// \todo yoyo: TODO_HARDWARE_SKINNIG: we must implement the 4 matrices mode by hardware (Radeon, NV20).
	// \todo yoyo: TODO_SOFTWARE_SKINNIG: we must implement the 4 matrices mode by software.

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


	// Reset VertexArrayRange.
	resetVertexArrayRange();


#ifdef NL_OS_WINDOWS
	// Then delete.
	wglMakeCurrent(NULL,NULL);
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

	_hRC=NULL;
	_hDC=NULL;
	_hWnd=NULL;

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
void	CDriverGL::setupScissor (const class CScissor& scissor)
{
	// Get viewport
	float x= scissor.X;
	float y= scissor.Y;
	float width= scissor.Width;
	float height= scissor.Height;

	if(x==0 && x==0 && width==1 && height==1)
	{
		glDisable(GL_SCISSOR_TEST);
	}
	else
	{
#ifdef NL_OS_WINDOWS
		if (_hWnd)
		{
			// Get window rect
			RECT rect;
			GetClientRect (_hWnd, &rect);
			int clientWidth=rect.right-rect.left;
			int clientHeight=rect.bottom-rect.top;

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


void CDriverGL::copyFrameBufferToTexture(ITexture *tex, uint32 level
														, uint32 offsetx, uint32 offsety
													    , uint32 x, uint32 y
														, uint32 width, uint32 height														
													)
{
	nlassert(!tex->isTextureCube()) ;

	bool compressed = false ;
	GLint tfm = getGlTextureFormat(*tex, compressed);
	nlassert(!compressed) ;

	
	// first, mark the texture as valid, and make sure there is a corresponding texture in the device memory	
	setupTexture(*tex) ;	

	CTextureDrvInfosGL*	gltext = (CTextureDrvInfosGL*)(ITextureDrvInfos*)(tex->TextureDrvShare->DrvTexture) ;
	
	
	
	

	

	glActiveTextureARB(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_2D) ; 
	glBindTexture(GL_TEXTURE_2D, gltext->ID);	


	glCopyTexSubImage2D(GL_TEXTURE_2D, level, offsetx, offsety, x, y, width, height);
	

	glDisable(GL_TEXTURE_2D) ;


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


} // NL3D
