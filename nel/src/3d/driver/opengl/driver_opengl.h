/** \file driver_opengl.h
 * OpenGL driver implementation
 *
 * $Id: driver_opengl.h,v 1.38 2001/01/09 17:02:17 corvazier Exp $
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

#ifndef NL_OPENGL_H
#define NL_OPENGL_H


#include "nel/misc/types_nl.h"

#ifdef NL_OS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <GL/glx.h>
#endif // NL_OS_WINDOWS

#include <GL/gl.h>
#include "driver_opengl_extension.h"

#include "nel/3d/driver.h"
#include "nel/3d/material.h"
#include "nel/3d/shader.h"
#include "nel/misc/matrix.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/rgba.h"
#include "nel/misc/event_emitter.h"

#ifdef NL_OS_WINDOWS
#include "nel/misc/win_event_emitter.h"
#elif defined (NL_OS_UNIX)
#include "unix_event_emitter.h"
#endif // NL_OS_UNIX

namespace NL3D {

using NLMISC::CMatrix;
using NLMISC::CVector;

// --------------------------------------------------

class CTextureDrvInfosGL : public ITextureDrvInfos
{
public:
		GLuint					ID;
		// The current wrap modes assigned to the texture.
		ITexture::TWrapMode		WrapS;
		ITexture::TWrapMode		WrapT;
		ITexture::TMagFilter	MagFilter;
		ITexture::TMinFilter	MinFilter;

		CTextureDrvInfosGL()
		{
			// The id is auto created here.
			glGenTextures(1,&ID);
		}
		~CTextureDrvInfosGL()
		{
			// The id is auto deleted here.
			glDeleteTextures(1,&ID);
		}
};


// --------------------------------------------------

class CShaderGL : public IShader
{
public:
	GLenum		SrcBlend;
	GLenum		DstBlend;
	GLenum		ZComp;

	GLfloat		Emissive[4];
	GLfloat		Ambient[4];
	GLfloat		Diffuse[4];
	GLfloat		Specular[4];
};

// --------------------------------------------------

class CDriverGL : public IDriver
{
public:

#ifdef NL_OS_WINDOWS
	// Acces
	uint32					getHwnd ()
	{
		return (uint32)_hWnd;
	}
#endif // NL_OS_WINDOWS

							CDriverGL();
	virtual					~CDriverGL() { release(); };

	virtual bool			init();

	virtual ModeList		enumModes();

	virtual bool			setDisplay(void* wnd, const GfxMode& mode) throw(EBadDisplay);

	virtual bool			activate();

	virtual NLMISC::IEventEmitter	*getEventEmitter() { return&_EventEmitter; };

	virtual bool			clear2D(CRGBA rgba);

	virtual bool			clearZBuffer(float zval=1);

	virtual bool			setupTexture(ITexture& tex);

	virtual bool			setupMaterial(CMaterial& mat);

	virtual void			setFrustum(float left, float right, float bottom, float top, float znear, float zfar, bool perspective = true);

	virtual void			setupViewMatrix(const CMatrix& mtx);

	virtual void			setupModelMatrix(const CMatrix& mtx, uint8 n=0);

	virtual CMatrix			getViewMatrix() const;

	virtual bool			activeVertexBuffer(CVertexBuffer& VB);

	virtual bool			render(CPrimitiveBlock& PB, CMaterial& Mat);

	virtual bool			swapBuffers();

	virtual bool			release();

	virtual TMessageBoxId	systemMessageBox (const char* message, const char* title, TMessageBoxType type=okType, TMessageBoxIcon icon=noIcon);

	virtual void			setupViewport (const class CViewport& viewport);

	virtual uint32			getImplementationVersion () const
	{
		return ReleaseVersion;
	}

	virtual const char*		getDriverInformation ()
	{
		return "Opengl 1.2 NeL Driver";
	}

	virtual const char*		getVideocardInformation ();

	virtual bool			isActive ();

	virtual void			showCursor (bool b);

	// between 0.0 and 1.0
	virtual void setMousePos(float x, float y);

	virtual void			setCapture (bool b);

	virtual void			getWindowSize (uint32 &width, uint32 &height);

private:
	// Version of the driver. Not the interface version!! Increment when implementation of the driver change.
	static const uint32		ReleaseVersion;

#ifdef NL_OS_WINDOWS

	friend static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	
	HWND						_hWnd;
	HDC							_hDC;
	PIXELFORMATDESCRIPTOR		_pfd;
    HGLRC						_hRC;
	NLMISC::CWinEventEmitter	_EventEmitter;
	static uint					_Registered;
	DEVMODE						_OldScreenMode;
	bool						_FullScreen;

#elif defined (NL_OS_UNIX)
	Display *dpy;
	GLXContext ctx;
	Window win;
	Cursor cursor;
	NLMISC::CUnixEventEmitter	_EventEmitter;
#endif // NL_OS_UNIX

	bool					_Initialized;

	// OpenGL extensions Extensions.
	CGlExtensions			_Extensions;

	// Current View matrix.
	CMatrix					_ViewMtx;


	// Prec settings, for optimisation.
	ITexture*				_CurrentTexture[IDRV_MAT_MAXTEXTURES];
	CMaterial*				_CurrentMaterial;
	CMaterial::CTexEnv		_CurrentTexEnv[IDRV_MAT_MAXTEXTURES];

private:
	bool					setupVertexBuffer(CVertexBuffer& VB);
	bool					activateTexture(uint stage, ITexture *tex);
	void					activateTexEnvMode(uint stage, const CMaterial::CTexEnv  &env);
	void					activateTexEnvColor(uint stage, const CMaterial::CTexEnv  &env);


};

} // NL3D

#endif // NL_OPENGL_H
