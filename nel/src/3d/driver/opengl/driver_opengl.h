/** \file driver_opengl.h
 * OpenGL driver implementation
 *
 * $Id: driver_opengl.h,v 1.128 2002/09/04 12:41:36 berenguier Exp $
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
#  ifdef min
#    undef min
#  endif
#  ifdef max
#    undef max
#  endif

#else // NL_OS_UNIX

#define GLX_GLXEXT_PROTOTYPES

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
#include "nel/misc/matrix.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/rgba.h"
#include "nel/misc/event_emitter.h"
#include "nel/misc/bit_set.h"
#include "3d/ptr_set.h"
#include "nel/misc/heap_memory.h"
#include "nel/misc/event_emitter_multi.h"
#include "driver_opengl_states.h"
#include "3d/texture_cube.h"


#ifdef NL_OS_WINDOWS
#include "nel/misc/win_event_emitter.h"
#elif defined (NL_OS_UNIX)
#include "unix_event_emitter.h"
#endif // NL_OS_UNIX


// For optimisation consideration, allow 256 lightmaps at max.
#define	NL3D_DRV_MAX_LIGHTMAP		256


namespace NL3D {

using NLMISC::CMatrix;
using NLMISC::CVector;


class	CDriverGL;
class	IVertexArrayRange;
class	IVertexBufferHardGL;


// ***************************************************************************
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


// ***************************************************************************
class CVBDrvInfosGL : public IVBDrvInfos
{
public:
	CVBDrvInfosGL(IDriver *drv, ItVBDrvInfoPtrList it) : IVBDrvInfos(drv, it) {}
};


// ***************************************************************************
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
	// For fast comp.
	uint32		PackedEmissive;
	uint32		PackedAmbient;
	uint32		PackedDiffuse;
	uint32		PackedSpecular;

	// The supported Shader type.
	CMaterial::TShader	SupportedShader;

	CShaderGL(IDriver *drv, ItShaderPtrList it) : IShader(drv, it) {}
};


// ***************************************************************************
/// Info for the last VertexBuffer setuped (iether normal or hard).
class	CVertexBufferInfo
{
public:
	uint16					VertexFormat;
	uint16					VertexSize;
	uint32					NumVertices;
	uint32					NumWeight;	
	CVertexBuffer::TType	Type[CVertexBuffer::NumValue];

	// NB: ptrs are invalid if VertexFormat does not support the compoennt. must test VertexFormat, not the ptr.
	void					*ValuePtr[CVertexBuffer::NumValue];


	// ATI Special setup ?
	bool					ATIVBHardMode;
	// the handle of ATI memory
	uint					ATIVertexObjectId;
	// This is the offset relative to the ATIVertexObjectId start mem.
	uint					ATIValueOffset[CVertexBuffer::NumValue];


	void		setupVertexBuffer(CVertexBuffer &vb);
	void		setupVertexBufferHard(IVertexBufferHardGL &vb);
};


// ***************************************************************************
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
	virtual					~CDriverGL();

	virtual bool			init();

	virtual ModeList		enumModes();

	virtual void			disableHardwareVertexProgram();
	virtual void			disableHardwareVertexArrayAGP();
	virtual void			disableHardwareTextureShader();

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

	virtual	sint			getNbTextureStages() const;

	virtual bool			isTextureExist(const ITexture&tex);

	virtual NLMISC::IEventEmitter	*getEventEmitter() { return&_EventEmitter; };

	virtual bool			clear2D(CRGBA rgba);

	virtual bool			clearZBuffer(float zval=1);

	virtual bool			setupTexture (ITexture& tex);
	
	virtual bool			setupTextureEx (ITexture& tex, bool bUpload, bool &bAllUploaded);
	virtual bool			uploadTexture (ITexture& tex, NLMISC::CRect& rect, uint8 nNumMipMap);
	virtual bool			uploadTextureCube (ITexture& tex, NLMISC::CRect& rect, uint8 nNumMipMap, uint8 nNumFace);

	virtual void			forceDXTCCompression(bool dxtcComp);

	virtual void			forceTextureResize(uint divisor);

	/// Setup texture env functions. Used by setupMaterial
	void				setTextureEnvFunction(uint stage, CMaterial& mat);

	/// setup the texture matrix for a given number of stages (starting from 0)
	void      setupUserTextureMatrix(uint numStages, CMaterial& mat);

	/// disable all texture matrix 
	void      disableUserTextureMatrix();

	/// For objects with caustics, setup the first texture (which actually is the one from the material)
	/*static inline void		setupCausticsFirstTex(const CMaterial &mat);

	/// For objects with caustics, setup the caustic texture itself
	static inline void		setupCausticsSecondTex(uint stage);*/

	virtual bool			setupMaterial(CMaterial& mat);

	virtual void			setFrustum(float left, float right, float bottom, float top, float znear, float zfar, bool perspective = true);

	virtual void			setupViewMatrix(const CMatrix& mtx);

	virtual void			setupViewMatrixEx(const CMatrix& mtx, const CVector &cameraPos);

	virtual void			setupModelMatrix(const CMatrix& mtx);

	virtual void			multiplyModelMatrix(const CMatrix& mtx);

	virtual CMatrix			getViewMatrix() const;

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

	virtual	bool			slowUnlockVertexBufferHard() const;

	virtual	uint			getMaxVerticesByVertexBufferHard() const;

	virtual	bool			initVertexArrayRange(uint agpMem, uint vramMem);

	virtual	IVertexBufferHard	*createVertexBufferHard(uint16 vertexFormat, const uint8 *typeArray, uint32 numVertices, 
														IDriver::TVBHardType vbType);

	virtual	void			deleteVertexBufferHard(IVertexBufferHard *VB);

	virtual void			activeVertexBufferHard(IVertexBufferHard *VB);


	virtual bool			activeVertexBuffer(CVertexBuffer& VB);

	virtual bool			activeVertexBuffer(CVertexBuffer& VB, uint first, uint end);

	virtual	void			mapTextureStageToUV(uint stage, uint uv);

	virtual bool			render(CPrimitiveBlock& PB, CMaterial& Mat);

	virtual void			renderTriangles(CMaterial& Mat, uint32 *tri, uint32 ntris);

	virtual void			renderSimpleTriangles(uint32 *tri, uint32 ntris);

	virtual void			renderPoints(CMaterial& Mat, uint32 numPoints) ;
	
	virtual void			renderQuads(CMaterial& Mat, uint32 startIndex, uint32 numQuads) ;

	virtual bool			swapBuffers();

	virtual	void			profileRenderedPrimitives(CPrimitiveProfile &pIn, CPrimitiveProfile &pOut);

	virtual	uint32			profileAllocatedTextureMemory();

	virtual	uint32			profileSetupedMaterials() const;

	virtual	uint32			profileSetupedModelMatrix() const;

	void					enableUsedTextureMemorySum (bool enable);
	
	uint32					getUsedTextureMemory() const;

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

	virtual uint8			getBitPerPixel ();

	virtual void			showCursor (bool b);

	// between 0.0 and 1.0
	virtual void			setMousePos(float x, float y);

	virtual void			setCapture (bool b);

	virtual NLMISC::IMouseDevice			*enableLowLevelMouse(bool enable);
		
	virtual NLMISC::IKeyboardDevice			*enableLowLevelKeyboard(bool enable);

	virtual NLMISC::IInputDeviceManager		*getLowLevelInputDeviceManager();

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

	virtual bool			fillBuffer (CBitmap &bitmap);

	virtual void			setPolygonMode (TPolygonMode mode);

	virtual uint			getMaxLight () const;

	virtual void			setLight (uint8 num, const CLight& light);

	virtual void			enableLight (uint8 num, bool enable=true);

	virtual void			setPerPixelLightingLight(CRGBA diffuse, CRGBA specular, float shininess);

	virtual void			setAmbientColor (CRGBA color);

	/// \name Fog support.
	// @{
	virtual	bool			fogEnabled();
	virtual	void			enableFog(bool enable);
	/// setup fog parameters. fog must enabled to see result. start and end are in [0,1] range.
	virtual	void			setupFog(float start, float end, CRGBA color);
	// @}

	/// \name texture addressing modes
	// @{
	virtual bool supportTextureShaders() const;

	virtual bool isTextureAddrModeSupported(CMaterial::TTexAddressingMode mode) const;

	virtual void setMatrix2DForTextureOffsetAddrMode(const uint stage, const float mat[4]);
	// @}

	virtual bool supportPerPixelLighting(bool specular) const;


	/// \name Misc
	// @{
	virtual	bool			supportBlendConstantColor() const;
	virtual	void			setBlendConstantColor(NLMISC::CRGBA col);
	virtual	NLMISC::CRGBA	getBlendConstantColor() const;
	// @}

private:
	friend class					CTextureDrvInfosGL;
	friend class					CVertexProgamDrvInfosGL;


private:
	// Version of the driver. Not the interface version!! Increment when implementation of the driver change.
	static const uint32		ReleaseVersion;

	bool					_FullScreen;
	bool						_OffScreen;

#ifdef NL_OS_WINDOWS

	friend static void GlWndProc(CDriverGL *driver, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	
	HWND						_hWnd;
	HDC							_hDC;
	PIXELFORMATDESCRIPTOR		_pfd;
    HGLRC						_hRC;
	static uint					_Registered;
	DEVMODE						_OldScreenMode;
	NLMISC::CEventEmitterMulti	_EventEmitter; // this can contains a win emitter and eventually a direct input emitter	
	bool						_DestroyWindow;

	// Off-screen rendering in Dib section
	HPBUFFERARB					_PBuffer;

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
	// @}


	// Depth of the driver in Bit Per Pixel
	uint8					_Depth;

	// The forceNormalize() state.
	bool					_ForceNormalize;

	// To know if light setup has been changed from last render() ( any call to setupViewMatrix() or setLight() ).
	bool					_LightSetupDirty;

	// To know if the modelview matrix setup has been changed from last render() (any call to setupViewMatrix() / setupModelMatrix() ).
	bool					_ModelViewMatrixDirty;

	// Ored of _LightSetupDirty and _ModelViewMatrixDirty
	bool					_RenderSetupDirty;

	// Backup znear and zfar
	float					_OODeltaZ;

	// Current View matrix, in NEL basis. This is the parameter passed in setupViewMatrix*().
	CMatrix					_UserViewMtx;
	// Current (OpenGL basis) View matrix.
	// NB: if setuped with setupViewMatrixEx(), _ViewMtx.Pos()==(0,0,0)
	CMatrix					_ViewMtx;
	// Matrix used for specular
	CMatrix					_TexMtx;
	// Precision ZBuffer: The Current cameraPosition, to remove from each model Position.
	CVector					_PZBCameraPos;


	// Current computed (OpenGL basis) ModelView matrix.
	// NB: This matrix have already substracted the _PZBCameraPos
	// Hence this matrix represent the Exact eye-space basis (only _ViewMtx is a bit tricky).
	CMatrix					_ModelViewMatrix;

	// Fog.
	bool					_FogEnabled;
	GLfloat					_CurrentFogColor[4];


	// Num lights return by GL_MAX_LIGHTS
	uint						_MaxDriverLight;
	bool						_LightEnable[MaxLight];				// Light enable.
	uint						_LightMode[MaxLight];				// Light mode.
	CVector						_WorldLightPos[MaxLight];			// World position of the lights.
	CVector						_WorldLightDirection[MaxLight];		// World direction of the lights.

	//\name description of the per pixel light
	// @{
		void checkForPerPixelLightingSupport();
		bool						_SupportPerPixelShader;
		bool						_SupportPerPixelShaderNoSpec;
		float						_PPLExponent;
		NLMISC::CRGBA				_PPLightDiffuseColor;
		NLMISC::CRGBA				_PPLightSpecularColor;
	// @}

	

	/// \name Prec settings, for optimisation.
	// @{

	// Special Texture environnements.
	enum	CTexEnvSpecial {
		TexEnvSpecialDisabled= 0, 
		TexEnvSpecialLightMapNV4, 
		TexEnvSpecialSpecularStage0NV4,
		TexEnvSpecialSpecularStage1NV4,
		TexEnvSpecialSpecularStage1NoTextNV4,
		TexEnvSpecialPPLStage0,
		TexEnvSpecialPPLStage2,
	};

	// NB: CRefPtr are not used for mem/spped optimisation. setupMaterial() and setupTexture() reset those states.
	CMaterial*				_CurrentMaterial;
	CMaterial::TShader		_CurrentMaterialSupportedShader;
	ITexture*				_CurrentTexture[IDRV_MAT_MAXTEXTURES];
	CTextureDrvInfosGL*		_CurrentTextureInfoGL[IDRV_MAT_MAXTEXTURES];
	CMaterial::CTexEnv		_CurrentTexEnv[IDRV_MAT_MAXTEXTURES];
	// Special Texture Environnement.
	CTexEnvSpecial			_CurrentTexEnvSpecial[IDRV_MAT_MAXTEXTURES];
	// Texture addressing mode
	GLenum					_CurrentTexAddrMode[IDRV_MAT_MAXTEXTURES];
	// Reset texture shaders to their initial state if they are used
	void					resetTextureShaders();
	// activation of texture shaders
	bool					_NVTextureShaderEnabled;

	// Prec settings for material.
	CDriverGLStates			_DriverGLStates;
	// Optim: To not test change in Materials states if just texture has changed. Very usefull for landscape.
	uint32					_MaterialAllTextureTouchedFlag;

	// @}

	bool					_CurrentGlNormalize;

private:	

	bool					setupVertexBuffer(CVertexBuffer& VB);
	// Activate Texture Environnement. Do it with caching.
	bool					activateTexture(uint stage, ITexture *tex);
	// NB: this test _CurrentTexEnv[] and _CurrentTexEnvSpecial[].
	void					activateTexEnvMode(uint stage, const CMaterial::CTexEnv  &env);
	void					activateTexEnvColor(uint stage, const CMaterial::CTexEnv  &env);
	// Force Activate Texture Environnement. no caching here. TexEnvSpecial is disabled.
	void					forceActivateTexEnvMode(uint stage, const CMaterial::CTexEnv  &env);
	void					activateTexEnvColor(uint stage, NLMISC::CRGBA col);
	void					forceActivateTexEnvColor(uint stage, NLMISC::CRGBA col)
	{
		static	const float	OO255= 1.0f/255;	
		_CurrentTexEnv[stage].ConstantColor= col;
		// Setup the gl cte color.
		_DriverGLStates.activeTextureARB(stage);
		GLfloat		glcol[4];
		glcol[0]= col.R*OO255;
		glcol[1]= col.G*OO255;
		glcol[2]= col.B*OO255;
		glcol[3]= col.A*OO255;
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, glcol);	
	}
	void					forceActivateTexEnvColor(uint stage, const CMaterial::CTexEnv  &env)
	{	
		forceActivateTexEnvColor(stage, env.ConstantColor);	
	}


	/// nv texture shaders. Should be used only if this caps is present!
	void					enableNVTextureShader(bool enabled);
	// check nv texture shader consistency
	void			verifyNVTextureShaderConfig();

	// Called by doRefreshRenderSetup(). set _LightSetupDirty to false
	void					cleanLightSetup ();

	// According to extensions, retrieve GL tex format of the texture.
	GLint					getGlTextureFormat(ITexture& tex, bool &compressed);


	// Clip the wanted rectangle with window. return true if rect is not NULL.
	bool					clipRect(NLMISC::CRect &rect);


	/// \name Material multipass.
	/**	NB: setupMaterial() must be called before thoses methods.
	 */
	// @{
	/// init multipass for _CurrentMaterial. return number of pass required to render this material.
	sint			beginMultiPass();
	/// active the ith pass of this material.
	void			setupPass(uint pass);
	/// end multipass for this material.
	void			endMultiPass();
	/// LastVB for UV setup.
	CVertexBufferInfo	_LastVB;
	// @}


	/// setup a texture stage with an UV from VB.
	void			setupUVPtr(uint stage, CVertexBufferInfo &VB, uint uvId);


	/// \name Lightmap.
	// @{
	void			computeLightMapInfos(const CMaterial &mat);
	sint			beginLightMapMultiPass();
	void			setupLightMapPass(uint pass);
	void			endLightMapMultiPass();

	/// Temp Variables computed in beginLightMapMultiPass(). Reused in setupLightMapPass().
	uint			_NLightMaps;
	uint			_NLightMapPerPass;
	uint			_NLightMapPass;
	// This array is the LUT from lmapId in [0, _NLightMaps[, to original lightmap id in material.
	std::vector<uint>		_LightMapLUT;

	// last stage env.
	CMaterial::CTexEnv	_LightMapLastStageEnv;

	// Caching
	bool			_LastVertexSetupIsLightMap;
	sint8			_LightMapUVMap[IDRV_MAT_MAXTEXTURES];

	// restore std vertex Setup.
	void			resetLightMapVertexSetup();

	// @}

	/// \name Specular.
	// @{
	sint			beginSpecularMultiPass();
	void			setupSpecularPass(uint pass);
	void			endSpecularMultiPass();
	// @}


	/// \name Per pixel lighting
	// @{
	// per pixel lighting with specular
	sint			beginPPLMultiPass();
	void			setupPPLPass(uint pass);
	void			endPPLMultiPass();

	// per pixel lighting, no specular
	sint			beginPPLNoSpecMultiPass();
	void			setupPPLNoSpecPass(uint pass);
	void			endPPLNoSpecMultiPass();

	typedef NLMISC::CSmartPtr<CTextureCube> TSPTextureCube;	
	typedef std::vector<TSPTextureCube> TTexCubeVect;	
	TTexCubeVect   _SpecularTextureCubes; // the cube maps used to compute the specular lighting.
		

	/// get (and if necessary, build) a cube map used for specular lighting. The range for exponent is limited, and only the best fit is used
	CTextureCube   *getSpecularCubeMap(uint exp);

	// get (and if necessary, build) the cube map used for diffuse lighting
	CTextureCube   *getDiffuseCubeMap() { return getSpecularCubeMap(1); }



	// @}

	/// \name Caustics
	// @{
	/*sint			beginCausticsMultiPass(const CMaterial &mat);
	void			setupCausticsPass(const CMaterial &mat, uint pass);
	void			endCausticsMultiPass(const CMaterial &mat);*/
	// @}



	/// setup GL arrays, with a vb info.
	void			setupGlArrays(CVertexBufferInfo &vb);


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

	// refresh matrixes and lights.
	void			refreshRenderSetup()
	{
		// check if something to change.
		if(_RenderSetupDirty)
		{
			doRefreshRenderSetup();
		}
	}
	void			doRefreshRenderSetup();


	/// \name VertexBufferHard 
	// @{
	CPtrSet<IVertexBufferHardGL>	_VertexBufferHardSet;
	friend class					CVertexArrayRangeNVidia;
	friend class					CVertexBufferHardGLNVidia;
	friend class					CVertexArrayRangeATI;
	friend class					CVertexBufferHardGLATI;
	// The VertexArrayRange activated.
	IVertexArrayRange				*_CurrentVertexArrayRange;
	// The VertexBufferHardGL activated.
	IVertexBufferHardGL				*_CurrentVertexBufferHard;
	// current setup passed to nglVertexArrayRangeNV()
	void							*_NVCurrentVARPtr;
	uint32							_NVCurrentVARSize;

	// Info got from ATI or NVidia extension.
	bool							_SupportVBHard;
	bool							_SlowUnlockVBHard;
	uint32							_MaxVerticesByVBHard;

	// The AGP VertexArrayRange.
	IVertexArrayRange				*_AGPVertexArrayRange;
	// The VRAM VertexArrayRange.
	IVertexArrayRange				*_VRAMVertexArrayRange;


	void							resetVertexArrayRange();

	void							fenceOnCurVBHardIfNeeded(IVertexBufferHardGL *newVBHard);


	// @}


	/// \name Profiling
	// @{
	CPrimitiveProfile									_PrimitiveProfileIn;
	CPrimitiveProfile									_PrimitiveProfileOut;
	uint32												_AllocatedTextureMemory;
	uint32												_NbSetupMaterialCall;
	uint32												_NbSetupModelMatrixCall;
	bool												_SumTextureMemoryUsed;
	std::set<NLMISC::CSmartPtr<ITexture> >				_TextureUsed;
	uint							computeMipMapMemoryUsage(uint w, uint h, GLint glfmt) const;
	// @}

	/// \name Vertex program interface
	// @{

	bool			isVertexProgramSupported () const;
	bool			isVertexProgramEmulated () const;
	bool			activeVertexProgram (CVertexProgram *program);
	void			setConstant (uint index, float, float, float, float);
	void			setConstant (uint index, double, double, double, double);
	void			setConstant (uint indexStart, const NLMISC::CVector& value);
	void			setConstant (uint indexStart, const NLMISC::CVectorD& value);	
	void			setConstant (uint index, uint num, const float *src);	
	void			setConstant (uint index, uint num, const double *src);
	void			setConstantMatrix (uint index, IDriver::TMatrix matrix, IDriver::TTransform transform);	
	void			enableVertexProgramDoubleSidedColor(bool doubleSided);
	
	// @}


	/// \fallback for material shaders
	// @{
		/// test wether the given shader is supported, and gives back a supported shader
		CMaterial::TShader	getSupportedShader(CMaterial::TShader shader);
	// @}

	bool			isVertexProgramEnabled () const
	{
		// Don't use glIsEnabled, too slow.
		return _VertexProgramEnabled;
	}

	// Track state of activeVertexProgram()
	bool							_VertexProgramEnabled;
	// Say if last setupGlArrays() was a VertexProgram setup.
	bool							_LastSetupGLArrayVertexProgram;


	bool							_ForceDXTCCompression;
	/// Divisor for textureResize (power).
	uint							_ForceTextureResizePower;


	// user texture matrix
	NLMISC::CMatrix		_UserTexMat[IDRV_MAT_MAXTEXTURES];
	uint				_UserTexMatEnabled; // bitm ask for user texture coords
	//NLMISC::CMatrix		_UserTexMat[IDRV_MAT_MAXTEXTURES];

	// Static const
	static const uint NumCoordinatesType[CVertexBuffer::NumType];
	static const uint GLType[CVertexBuffer::NumType];
	static const uint GLVertexAttribIndex[CVertexBuffer::NumValue];
	static const uint GLMatrix[IDriver::NumMatrix];
	static const uint GLTransform[IDriver::NumTransform];

	/// \name Caustics shaders
	// @{
		NLMISC::CSmartPtr<CTextureCube>	_CausticCubeMap; // a cube map used for the rendering of caustics
		static void initCausticCubeMap();
	// @}

	/// Same as getNbTextureStages(), but faster because inline, and not virtual!!
	sint			inlGetNumTextStages() const {return _Extensions.NbTextureStages;}


	NLMISC::CRGBA					_CurrentBlendConstantColor;
};

} // NL3D

#endif // NL_OPENGL_H
