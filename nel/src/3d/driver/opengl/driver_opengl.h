/** \file driver_opengl.h
 * OpenGL driver implementation
 *
 * $Id: driver_opengl.h,v 1.61 2001/04/19 12:49:28 puzin Exp $
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

#else // NL_OS_UNIX

#define	GL_GLEXT_PROTOTYPES
#include <GL/glx.h>

#ifdef XF86VIDMODE
#include <X11/extensions/xf86vmode.h>
#endif //XF86VIDMODE

#endif // NL_OS_UNIX

#include <GL/gl.h>
#include "driver_opengl_extension.h"

#include "nel/3d/driver.h"
#include "nel/3d/material.h"
#include "nel/3d/shader.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/misc/matrix.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/rgba.h"
#include "nel/misc/event_emitter.h"
#include "nel/misc/bit_set.h"

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
	// The GL Id.
	GLuint					ID;
	// Is the internal format of the texture is a compressed one?
	bool					Compressed;

	// The current wrap modes assigned to the texture.
	ITexture::TWrapMode		WrapS;
	ITexture::TWrapMode		WrapT;
	ITexture::TMagFilter	MagFilter;
	ITexture::TMinFilter	MinFilter;

	// The gl id is auto created here.
	CTextureDrvInfosGL();
	// The gl id is auto deleted here.
	~CTextureDrvInfosGL();
};



// --------------------------------------------------

class CVBDrvInfosGL : public IVBDrvInfos
{
public:
	// Software Skinning: post-rendered vertices/normales.
	std::vector<CVector>	SoftSkinVertices;
	std::vector<CVector>	SoftSkinNormals;
	// Software Skinning: flags to know what vertex must be computed.
	std::vector<uint8>		SoftSkinFlags;

	CVBDrvInfosGL() {}
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

	// Some constants
	enum { MaxLight=8 };

	// Acces
	uint32					getHwnd ()
	{
#ifdef NL_OS_WINDOWS
		return (uint32)_hWnd;
#else // NL_OS_WINDOWS
		return 0;
#endif // NL_OS_WINDOWS
	}

							CDriverGL();
	virtual					~CDriverGL() { release(); };

	virtual bool			init();

	virtual ModeList		enumModes();

	virtual bool			setDisplay(void* wnd, const GfxMode& mode) throw(EBadDisplay);

	virtual void*			getDisplay()
	{
#ifdef NL_OS_WINDOWS
		return (void*)_hWnd;
#else // NL_OS_WINDOWS
		return NULL;
#endif // NL_OS_WINDOWS
	}

	virtual bool			activate();

	virtual	sint			getNbTextureStages() {return _Extensions.NbTextureStages;}

	virtual NLMISC::IEventEmitter	*getEventEmitter() { return&_EventEmitter; };

	virtual bool			clear2D(CRGBA rgba);

	virtual bool			clearZBuffer(float zval=1);

	virtual bool			setupTexture(ITexture& tex);

	virtual bool			setupMaterial(CMaterial& mat);

	virtual void			setFrustum(float left, float right, float bottom, float top, float znear, float zfar, bool perspective = true);

	virtual void			setupViewMatrix(const CMatrix& mtx);

	virtual void			setupModelMatrix(const CMatrix& mtx, uint8 n=0);

	virtual CMatrix			getViewMatrix() const;

	virtual	void			setupVertexMode(uint vmode)
	{
		_VertexMode= vmode;
	}

	virtual bool			activeVertexBuffer(CVertexBuffer& VB);

	virtual bool			activeVertexBuffer(CVertexBuffer& VB, uint first, uint end);

	virtual bool			render(CPrimitiveBlock& PB, CMaterial& Mat);

	virtual void			renderTriangles(CMaterial& Mat, uint32 *tri, uint32 ntris);

	virtual bool			swapBuffers();

	virtual uint			getNumMatrix();

	virtual bool			supportPaletteSkinning();

	virtual bool			release();

	virtual TMessageBoxId	systemMessageBox (const char* message, const char* title, TMessageBoxType type=okType, TMessageBoxIcon icon=noIcon);

	virtual void			setupScissor (const class CViewport& viewport);

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

	virtual void			getBuffer (CBitmap &bitmap);

	virtual void			getZBuffer (std::vector<float>  &zbuffer);

	virtual void			getBufferPart (CBitmap &bitmap, NLMISC::CRect &rect);

	virtual void			getZBufferPart (std::vector<float>  &zbuffer, NLMISC::CRect &rect);

	virtual void			setPolygonMode (TPolygonMode mode);

	virtual void			setLight (uint8 num, const CLight& light);

	virtual void			enableLight (uint8 num, bool enable=true);

	virtual void			setAmbientColor (CRGBA color);

	/// \name Fog support.
	// @{
	virtual	bool			fogEnabled();
	virtual	void			enableFog(bool enable);
	/// setup fog parameters. fog must enabled to see result. start and end are in [0,1] range.
	virtual	void			setupFog(float start, float end, CRGBA color);
	// @}


private:

	// For fast vector/point multiplication.
	struct	CMatrix3x4
	{
		// Order them in memory line first, for faster memory access.
		float	a11, a12, a13, a14;
		float	a21, a22, a23, a24;
		float	a31, a32, a33, a34;

		// Copy from a matrix.
		void	set(const CMatrix &mat);
		// mulAddvector. NB: in should be different as v!! (else don't work).
		void	mulAddVector(const CVector &in, float scale, CVector &out)
		{
			out.x+= (a11*in.x + a12*in.y + a13*in.z) * scale;
			out.y+= (a21*in.x + a22*in.y + a23*in.z) * scale;
			out.z+= (a31*in.x + a32*in.y + a33*in.z) * scale;
		}
		// mulAddpoint. NB: in should be different as v!! (else don't work).
		void	mulAddPoint(const CVector &in, float scale, CVector &out)
		{
			out.x+= (a11*in.x + a12*in.y + a13*in.z + a14) * scale;
			out.y+= (a21*in.x + a22*in.y + a23*in.z + a24) * scale;
			out.z+= (a31*in.x + a32*in.y + a33*in.z + a34) * scale;
		}
	};


private:
	// Version of the driver. Not the interface version!! Increment when implementation of the driver change.
	static const uint32		ReleaseVersion;

	bool					_FullScreen;

#ifdef NL_OS_WINDOWS

	friend static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	
	HWND						_hWnd;
	HDC							_hDC;
	PIXELFORMATDESCRIPTOR		_pfd;
    HGLRC						_hRC;
	static uint					_Registered;
	DEVMODE						_OldScreenMode;
	NLMISC::CWinEventEmitter	_EventEmitter;

#elif defined (NL_OS_UNIX)

	Display						*dpy;
	GLXContext					ctx;
	Window						win;
	Cursor						cursor;
	NLMISC::CUnixEventEmitter	_EventEmitter;

#ifdef XF86VIDMODE
	int						_OldDotClock;   // old dotclock
	XF86VidModeModeLine		_OldScreenMode;	// old modeline
	int						_OldX, _OldY;   //Viewport settings
#endif //XF86VIDMODE

#endif // NL_OS_UNIX

	bool					_Initialized;

	/// \name Driver Caps.
	// @{
	// OpenGL extensions Extensions.
	CGlExtensions			_Extensions;
	// Say if palette skinning can be done in Hardware
	bool					_PaletteSkinHard;
	// @}


	// The vertex transform mode.
	uint					_VertexMode;

	// To know if matrix setup has been changed from last activeVertexBuffer() (any call to setupViewMatrix() / setupModelMatrix()).
	bool					_MatrixSetupDirty;

	// To know if view matrix setup has been changed from last activeVertexBuffer() (any call to setupViewMatrix()).
	bool					_ViewMatrixSetupDirty;

	// for each model matrix, a flag to know if setuped.
	NLMISC::CBitSet			_ModelViewMatrixDirty;
	// same flag, but for palette Skinning (because they don't share same setup).
	NLMISC::CBitSet			_ModelViewMatrixDirtyPaletteSkin;


	// Current (OpenGL basis) View matrix.
	CMatrix					_ViewMtx;

	// Current computed (OpenGL basis) ModelView matrix.
	CMatrix					_ModelViewMatrix[MaxModelMatrix];
	// For software skinning.
	CMatrix					_ModelViewMatrixNormal[MaxModelMatrix];
	CMatrix3x4				_ModelViewMatrix3x4[MaxModelMatrix];
	CMatrix3x4				_ModelViewMatrixNormal3x4[MaxModelMatrix];


	// Sofware Skinning.
	uint8					*_CurrentSoftSkinFlags;
	uint8					*_CurrentSoftSkinSrc;
	uint					_CurrentSoftSkinNormalOff;
	uint					_CurrentSoftSkinPaletteSkinOff;
	uint					_CurrentSoftSkinWeightOff;
	uint					_CurrentSoftSkinSrcStride;
	uint					_CurrentSoftSkinFirst;
	uint					_CurrentSoftSkinEnd;
	CVector					*_CurrentSoftSkinVectorDst;
	CVector					*_CurrentSoftSkinNormalDst;

	// Fog.
	bool					_FogEnabled;

	// Num lights return by GL_MAX_LIGHTS
	uint						_MaxDriverLight;
	bool						_LightEnable[MaxLight];				// Light enable.
	uint						_LightMode[MaxLight];				// Light mode.
	CVector						_WorldLightPos[MaxLight];			// World position of the lights.
	CVector						_WorldLightDirection[MaxLight];		// World direction of the lights.

	// Prec settings, for optimisation.
	ITexture*				_CurrentTexture[IDRV_MAT_MAXTEXTURES];
	CMaterial*				_CurrentMaterial;
	CMaterial::CTexEnv		_CurrentTexEnv[IDRV_MAT_MAXTEXTURES];
	bool					_CurrentNormalize;

private:
	bool					setupVertexBuffer(CVertexBuffer& VB);
	bool					activateTexture(uint stage, ITexture *tex);
	void					activateTexEnvMode(uint stage, const CMaterial::CTexEnv  &env);
	void					activateTexEnvColor(uint stage, const CMaterial::CTexEnv  &env);

	// Called by activeVertexBuffer when _ViewMatrixSetupDirty is true to clean the view matrix.
	// set _ViewMatrixSetupDirty to false;
	void					cleanViewMatrix ();

	// According to extensions, retrieve GL tex format of the texture.
	GLint					getGlTextureFormat(ITexture& tex, bool &compressed);


	// Clip the wanted rectangle with window. return true if rect is not NULL.
	bool					clipRect(NLMISC::CRect &rect);


	// software skinning. Use _CurrentSoftSkin* global setup.
	void			computeSoftwareVertexSkinning(uint8 *pSrc, CVector *pVertexDst);
	void			computeSoftwareNormalSkinning(uint8 *pSrc, CVector *pNormalDst);
	void			refreshSoftwareSkinning();

};

} // NL3D

#endif // NL_OPENGL_H
