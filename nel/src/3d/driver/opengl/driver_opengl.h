/** \file driver_opengl.h
 * OpenGL driver implementation
 *
 * $Id: driver_opengl.h,v 1.79 2001/08/07 15:01:47 vizerie Exp $
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

#include "3d/driver.h"
#include "3d/material.h"
#include "3d/shader.h"
#include "3d/vertex_buffer.h"
#include "3d/vertex_buffer_hard.h"
#include "nel/misc/matrix.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/rgba.h"
#include "nel/misc/event_emitter.h"
#include "nel/misc/bit_set.h"
#include "3d/ptr_set.h"
#include "nel/misc/heap_memory.h"



#ifdef NL_OS_WINDOWS
#include "nel/misc/win_event_emitter.h"
#elif defined (NL_OS_UNIX)
#include "unix_event_emitter.h"
#endif // NL_OS_UNIX


namespace NL3D {

using NLMISC::CMatrix;
using NLMISC::CVector;


class	CDriverGL;


// --------------------------------------------------

class CTextureDrvInfosGL : public ITextureDrvInfos
{
public:
	// The GL Id.
	GLuint					ID;
	// Is the internal format of the texture is a compressed one?
	bool					Compressed;

	// This is the computed size of what memory this texture take.
	uint32					TextureMemory;
	// This is the owner driver.
	CDriverGL				*_Driver;


	// The current wrap modes assigned to the texture.
	ITexture::TWrapMode		WrapS;
	ITexture::TWrapMode		WrapT;
	ITexture::TMagFilter	MagFilter;
	ITexture::TMinFilter	MinFilter;

	// The gl id is auto created here.
	CTextureDrvInfosGL(IDriver *drv, ItTexDrvInfoPtrMap it, CDriverGL *drvGl);
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

	CVBDrvInfosGL(IDriver *drv, ItVBDrvInfoPtrList it) : IVBDrvInfos(drv, it) {}
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

	CShaderGL(IDriver *drv, ItShaderPtrList it) : IShader(drv, it) {}
};


// --------------------------------------------------


/** Work only if ARRAY_RANGE_NV is enabled. else, only call to ctor/dtor/free() is valid.
 *	any call to allocateVB() will return NULL.
 */
class CVertexArrayRange
{
public:
	CVertexArrayRange(CDriverGL *drv)
	{
		_Driver= drv;
		_VertexArrayPtr= NULL;
		_VertexArraySize= 0;
	}


	bool			allocated() const {return _VertexArrayPtr!=NULL;}


	/// allocate a vertex array sapce. false if error. must free before re-allocate.
	bool			allocate(uint32 size, IDriver::TVBHardType vbType);
	/// free this space.
	void			free();


	// Those methods read/write in _Driver->_CurrentVertexArrayRange.
	/// active this VertexArrayRange as the current vertex array range used. no-op if already setup.
	void			enable();
	/// disable this VertexArrayRange. _Driver->_CurrentVertexArrayRange= NULL;
	void			disable();


	/// Allocate a small subset of the memory. NULL if not enough mem.
	void			*allocateVB(uint32 size);
	/// free a VB allocated with allocateVB. No-op if NULL.
	void			freeVB(void	*ptr);


private:
	CDriverGL	*_Driver;
	void		*_VertexArrayPtr;
	uint32		_VertexArraySize;

	// Allocator.
	NLMISC::CHeapMemory		_HeapMemory;

};



/// Work only if ARRAY_RANGE_NV is enabled.
class CVertexBufferHardGL : public IVertexBufferHard
{
public:

	CVertexBufferHardGL();
	virtual	~CVertexBufferHardGL();

	bool				init(CDriverGL *drv, uint32 vertexFormat, uint32 numVertices, IDriver::TVBHardType vbType);

	virtual	void		*lock();
	virtual	void		unlock();


	void			enable();
	void			disable();



public:
	// NB: do not check if format is OK. return invalid result if format is KO.
	void				*getVertexCoordPointer()
	{
		nlassert(_VertexPtr);
		return _VertexPtr;
	}
	void				*getNormalCoordPointer()
	{
		nlassert(_VertexPtr);
		return (uint8*)_VertexPtr + getNormalOff();
	}
	void				*getTexCoordPointer(uint stageid)
	{
		nlassert(_VertexPtr);
		return (uint8*)_VertexPtr + getTexCoordOff(stageid);
	}
	void				*getColorPointer()
	{
		nlassert(_VertexPtr);
		return (uint8*)_VertexPtr + getColorOff();
	}
	void				*getSpecularPointer()
	{
		nlassert(_VertexPtr);
		return (uint8*)_VertexPtr + getSpecularOff();
	}
	void				*getWeightPointer(uint wid)
	{
		nlassert(_VertexPtr);
		return (uint8*)_VertexPtr + getWeightOff(wid);
	}
	void				*getPaletteSkinPointer()
	{
		nlassert(_VertexPtr);
		return (uint8*)_VertexPtr + getPaletteSkinOff();
	}


private:
	CDriverGL			*_Driver;
	CVertexArrayRange	*_VertexArrayRange;
	void				*_VertexPtr;

};



// --------------------------------------------------
/// Info for the last VertexBuffer setuped (iether normal or hard).
class	CVertexBufferInfo
{
public:
	uint32		VertexFormat;
	uint32		VertexSize;
	uint32		NumVertices;
	// NB: ptrs are invalid if VertexFormat does not support the compoennt. must test VertexFormat, not the ptr.
	void		*VertexCoordPointer;
	void		*NormalCoordPointer;
	void		*TexCoordPointer[IDRV_VF_MAXSTAGES];
	void		*ColorPointer;
	void		*SpecularPointer;
	void		*WeightPointer[IDRV_VF_MAXW];
	void		*PaletteSkinPointer;

	void		setupVertexBuffer(CVertexBuffer &vb);
	void		setupVertexBufferHard(CVertexBufferHardGL &vb);
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

	virtual emptyProc		getWindowProc();

	virtual bool			activate();

	virtual	sint			getNbTextureStages() const {return _Extensions.NbTextureStages;}

	virtual bool			isTextureExist(const ITexture&tex);

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


	virtual	void			forceNormalize(bool normalize)
	{
		_ForceNormalize= normalize;
		// if ForceNormalize, must enable GLNormalize now.
		if(normalize)
			enableGlNormalize(true);
	}

	virtual	bool			isForceNormalize() const
	{
		return _ForceNormalize;
	}


	virtual	bool			supportVertexBufferHard() const;

	virtual	bool			initVertexArrayRange(uint agpMem, uint vramMem);

	virtual	IVertexBufferHard	*createVertexBufferHard(uint32 vertexFormat, uint32 numVertices, IDriver::TVBHardType vbType);

	virtual	void			deleteVertexBufferHard(IVertexBufferHard *VB);

	virtual void			activeVertexBufferHard(IVertexBufferHard *VB);


	virtual bool			activeVertexBuffer(CVertexBuffer& VB);

	virtual bool			activeVertexBuffer(CVertexBuffer& VB, uint first, uint end);

	virtual bool			render(CPrimitiveBlock& PB, CMaterial& Mat);

	virtual void			renderTriangles(CMaterial& Mat, uint32 *tri, uint32 ntris);

	virtual void			renderPoints(CMaterial& Mat, uint32 numPoints) ;
	
	virtual void			renderQuads(CMaterial& Mat, uint32 startIndex, uint32 numQuads) ;

	virtual bool			swapBuffers();

	virtual	void			profileRenderedPrimitives(CPrimitiveProfile &pIn, CPrimitiveProfile &pOut);

	virtual	uint32			profileAllocatedTextureMemory();

	virtual uint			getNumMatrix();

	virtual bool			supportPaletteSkinning();

	virtual bool			release();

	virtual TMessageBoxId	systemMessageBox (const char* message, const char* title, TMessageBoxType type=okType, TMessageBoxIcon icon=noIcon);

	virtual void			setupScissor (const class CScissor& scissor);

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
		
	virtual void			copyFrameBufferToTexture(ITexture *tex, uint32 level
														, uint32 offsetx, uint32 offsety
													    , uint32 x, uint32 y
														, uint32 width, uint32 height														
													)  ;
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
	friend class					CTextureDrvInfosGL;


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

	friend static void GlWndProc(CDriverGL *driver, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	
	HWND						_hWnd;
	HDC							_hDC;
	PIXELFORMATDESCRIPTOR		_pfd;
    HGLRC						_hRC;
	static uint					_Registered;
	DEVMODE						_OldScreenMode;
	NLMISC::CWinEventEmitter	_EventEmitter;
	bool						_DestroyWindow;

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


	// The forceNormalize() state.
	bool					_ForceNormalize;


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
	CMatrix					_TexMtx;

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
	// NB: CRefPtr are not used for mem/spped optimisation. setupMaterial() and setupTexture() reset those states.
	ITexture*				_CurrentTexture[IDRV_MAT_MAXTEXTURES];
	CMaterial*				_CurrentMaterial;
	CMaterial::CTexEnv		_CurrentTexEnv[IDRV_MAT_MAXTEXTURES];
	bool					_CurrentGlNormalize;

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


	/// \name Material multipass.
	// @{
	/// init multipass for this material. return number of pass required to render this material.
	sint			beginMultiPass(const CMaterial &mat);
	/// active the ith pass of this material.
	void			setupPass(const CMaterial &mat, uint pass);
	/// end multipass for this material.
	void			endMultiPass(const CMaterial &mat);
	/// LastVB for UV setup.
	CVertexBufferInfo	_LastVB;
	// @}


	/// setup a texture stage with an UV from VB.
	void			setupUVPtr(uint stage, CVertexBufferInfo &VB, uint uvId);


	/// \name Lightmap.
	// @{
	void			computeLightMapInfos(const CMaterial &mat, uint &nLMaps, uint &nLMapPerPass, uint &nPass) const;
	sint			beginLightMapMultiPass(const CMaterial &mat);
	void			setupLightMapPass(const CMaterial &mat, uint pass);
	void			endLightMapMultiPass(const CMaterial &mat);
	// @}

	/// \name Specular.
	// @{
	sint			beginSpecularMultiPass(const CMaterial &mat);
	void			setupSpecularPass(const CMaterial &mat, uint pass);
	void			endSpecularMultiPass(const CMaterial &mat);
	// @}


	/// setup GL arrays, with a vb info.
	void			setupGlArrays(CVertexBufferInfo &vb, CVBDrvInfosGL *vbInf, bool skinning, bool paletteSkinning);


	/// Test/activate normalisation of normal.
	void			enableGlNormalize(bool normalize)
	{
		if(_CurrentGlNormalize!=normalize)
		{
			_CurrentGlNormalize= normalize;
			if(normalize)
				glEnable(GL_NORMALIZE);
			else
				glDisable(GL_NORMALIZE);
		}
	}


	/// \name VertexBufferHard 
	// @{
	CPtrSet<CVertexBufferHardGL>	_VertexBufferHardSet;
	friend class					CVertexArrayRange;
	friend class					CVertexBufferHardGL;
	// The CVertexArrayRange activated.
	CVertexArrayRange				*_CurrentVertexArrayRange;
	// The CVertexBufferHardGL activated.
	CVertexBufferHardGL				*_CurrentVertexBufferHard;

	// The AGP VertexArrayRange.
	CVertexArrayRange				_AGPVertexArrayRange;
	// The VRAM VertexArrayRange.
	CVertexArrayRange				_VRAMVertexArrayRange;


	void							resetVertexArrayRange();

	// @}


	/// \name Profiling
	// @{
	CPrimitiveProfile				_PrimitiveProfileIn;
	CPrimitiveProfile				_PrimitiveProfileOut;
	uint32							_AllocatedTextureMemory;
	uint							computeMipMapMemoryUsage(uint w, uint h, GLint glfmt) const;
	// @}

};

} // NL3D

#endif // NL_OPENGL_H
