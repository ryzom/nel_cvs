/** \file driver.h
 * Generic driver header.
 * Low level HW classes : ITexture, Cmaterial, CVertexBuffer, CPrimitiveBlock, IDriver
 *
 * \todo yoyo: garbage collector system, to remove NULL _Shaders, _TexDrvInfos and _VBDrvInfos entries.
 *
 * $Id: driver.h,v 1.33 2000/12/05 17:04:48 berenguier Exp $
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

#ifndef NL_DRV_H
#define NL_DRV_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/rgba.h"
#include "nel/misc/matrix.h"
#include "nel/3d/texture.h"

#include <vector>
#include <list>

namespace NLMISC
{
class IEventEmitter;
};

namespace NL3D
{

using NLMISC::CRefPtr;
using NLMISC::CRefCount;
using NLMISC::CSmartPtr;
using NLMISC::CRGBA;
using NLMISC::CVector;
using NLMISC::CMatrix;

// --------------------------------------------------

class IShader : public CRefCount
{
protected:
public:
	// The virtual dtor is important.
	virtual ~IShader() {};

};



const uint32 IDRV_MAT_MAXTEXTURES	=	4;

const uint32 IDRV_TOUCHED_BLENDFUNC	=	0x00000001;
const uint32 IDRV_TOUCHED_BLEND		=	0x00000002;
const uint32 IDRV_TOUCHED_OPACITY	=	0x00000004;
const uint32 IDRV_TOUCHED_SHADER	=	0x00000008;
const uint32 IDRV_TOUCHED_ZFUNC		=	0x00000010;
const uint32 IDRV_TOUCHED_ZBIAS		=	0x00000020;
const uint32 IDRV_TOUCHED_COLOR		=	0x00000040;
const uint32 IDRV_TOUCHED_LIGHTING	=	0x00000080;
const uint32 IDRV_TOUCHED_DEFMAT	=	0x00000100;
const uint32 IDRV_TOUCHED_ALPHA		=	0x00000200;

// Start texture touch at 0x10000.
const uint32 IDRV_TOUCHED_TEX[IDRV_MAT_MAXTEXTURES]		=
	{0x00010000, 0x00020000, 0x00040000, 0x00080000};
const uint32 IDRV_TOUCHED_ALL		=	0xFFFFFFFF;


const uint32 IDRV_MAT_HIDE			=	0x00000001;
const uint32 IDRV_MAT_TSP			=	0x00000002;
const uint32 IDRV_MAT_ZWRITE		=	0x00000004;
const uint32 IDRV_MAT_ZLIST			=	0x00000008;
const uint32 IDRV_MAT_LIGHTING		=	0x00000010;
const uint32 IDRV_MAT_SPECULAR		=	0x00000020;
const uint32 IDRV_MAT_DEFMAT		=	0x00000040;
const uint32 IDRV_MAT_BLEND			=	0x00000080;

class CMaterial : public CRefCount
{
public:

	enum ZFunc				{ always,never,equal,notequal,less,lessequal,greater,greaterequal };
	enum TBlend				{ one, zero, srcalpha, invsrcalpha };
	enum TShader			{ normal, user_color, envmap, bump};

private:

	TShader					_ShaderType;
	float					_Opacity;
	uint32					_Flags;
	TBlend					_SrcBlend,_DstBlend;
	ZFunc					_ZFunction;
	float					_ZBias;
	CRGBA					_Color;
	CRGBA					_Emissive,_Ambient,_Diffuse,_Specular;
	float					_Alpha;
	uint32					_Touched;

	CSmartPtr<ITexture>		_Textures[IDRV_MAT_MAXTEXTURES];

public:
	// Private. For Driver only.
	CRefPtr<IShader>		pShader;

	uint32					getFlags() const {return _Flags;}

public:
	// Object.
	CMaterial() {_Touched= 0;_Flags=0;}
	// see operator=.
	CMaterial(const CMaterial &mat) {_Touched= 0;_Flags=0; operator=(mat);}
	~CMaterial();
	// Do not copy DrvInfos, copy all infos and set IDRV_TOUCHED_ALL.
	CMaterial				&operator=(const CMaterial &mat);

	uint32					getTouched(void) { return(_Touched); }
	void					clearTouched(uint32 flag) { _Touched&=~flag; }

	bool					texturePresent(uint8 n);
	ITexture*				getTexture(uint8 n);
	void 					setTexture(ITexture* ptex, uint8 n=0);

	void					setShader(TShader val);

	void					setOpacity(float val);

	TBlend					getSrcBlend(void) { return(_SrcBlend); }
	void					setSrcBlend(TBlend val);

	TBlend					getDstBlend(void) { return(_DstBlend); }
	void					setDstBlend(TBlend val);
	void					setBlendFunc(TBlend src, TBlend dst);


	ZFunc					getZFunc(void) { return(_ZFunction); }		
	void					setZFunction(ZFunc val);

	float					getZBias(void) { return(_ZBias); }
	void					setZBias(float val);

	CRGBA					getColor(void) { return(_Color); }
	void					setColor(CRGBA rgba);

	void					setBlend(bool active);

	void					setLighting(	bool active, bool DefMat=true,
											CRGBA emissive=CRGBA(0,0,0), 
											CRGBA ambient=CRGBA(0,0,0), 
											CRGBA diffuse=CRGBA(0,0,0), 
											CRGBA specular=CRGBA(0,0,0) );

	void					setAlpha(float val);

	/** Init the material as unlit. normal shader, no lighting ....
	 * Default to: normal shader, no lighting, color to White(1,1,1,1), no texture, ZBias=0, ZFunc= lessequal, no blend.
	 * All other states are undefined (such as blend function, since blend is disabled).
	 */
	void					initUnlit();
	/** Init the material as default white lighted material. normal shader, lighting ....
	 * Default to: normal shader, lighting to default material, no texture, ZBias=0, ZFunc= lessequal, no blend.
	 * All other states are undefined (such as blend function, since blend is disabled).
	 */
	void					initLighted();
};

// --------------------------------------------------

class IVBDrvInfos : public CRefCount
{
private:
public:
	// The virtual dtor is important.
	virtual ~IVBDrvInfos() {};
};

// All these flags are similar to DX8
const uint32	IDRV_VF_MAXW		=	4;
const uint32	IDRV_VF_MAXSTAGES	=	8;
const uint32	IDRV_VF_XYZ			=	0x00000001;
const uint32	IDRV_VF_W[IDRV_VF_MAXW]			= { 0x00000002,0x00000004,0x00000008,0x00000010 };
const uint32	IDRV_VF_NORMAL		=	0x00000020;
const uint32	IDRV_VF_RGBA		=	0x00000040;
const uint32	IDRV_VF_SPECULAR	=	0x00000080;
const uint32	IDRV_VF_UV[IDRV_VF_MAXSTAGES]	= { 0x00000100,0x00000200,0x00000400,0x00000800,0x00001000,0x00002000,0x00004000,0x00008000 };

class CVertexBuffer : public CRefCount
{
private:
	uint32					_Flags;
	uint8					_VertexSize;
	uint32					_NbVerts;
	uint32					_Capacity;
	std::vector<uint8>		_Verts;

	uint					_WOff[IDRV_VF_MAXW];
	uint					_NormalOff;
	uint					_RGBAOff;
	uint					_SpecularOff;
	uint					_UVOff[IDRV_VF_MAXSTAGES];

public:
	// Private. For Driver only.
	CRefPtr<IVBDrvInfos>	DrvInfos;


public:
	CVertexBuffer(void);
	// see operator=.
	CVertexBuffer(const CVertexBuffer &vb);
	~CVertexBuffer(void);
	// Do not copy DrvInfos, copy all infos and set IDRV_TOUCHED_ALL.
	CVertexBuffer			&operator=(const CVertexBuffer &vb);


	/// Setup the vertex format. Do it before any other method.
	bool					setVertexFormat(uint32 Flags);
	uint32					getVertexFormat(void) { return(_Flags); };
	uint8					getVertexSize(void) {return(_VertexSize);}

	/// reserve space for nVerts vertices. You are allowed to write your vertices on this space.
	void					reserve(uint32 nVerts);
	/// Return the number of vertices reserved.
	uint32					capacity() {return _Capacity;}
	/// Set the number of active vertices. It enlarge capacity, if needed.
	void					setNumVertices(uint32 n);
	/// Get the number of active vertices.
	uint32					getNumVertices(void) {return(_NbVerts);}


	// It is an error (assert) to set a vertex component if not setuped in setVertexFormat().
	void					setVertexCoord(uint idx, float x, float y, float z);
	void					setVertexCoord(uint idx, const CVector &v);
	void					setNormalCoord(uint idx, const CVector &v);
	void					setTexCoord(uint idx, uint8 stage, float u, float v);
	void					setRGBA(uint idx, CRGBA rgba);


	void*					getVertexCoordPointer(uint idx=0);
	void*					getNormalCoordPointer(uint idx=0);
	void*					getColorPointer(uint idx=0);
	void*					getTexCoordPointer(uint idx=0, uint8 stage=0);
};

// --------------------------------------------------

class CPrimitiveBlock
{
private:
	// Triangles.
	uint32				_NbTris;
	uint32				_TriCapacity;
	std::vector<uint32>	_Tri;

	// Quads
	uint32				_NbQuads;
	uint32				_QuadCapacity;
	std::vector<uint32>	_Quad;

	// Lines
	uint32				_NbLines;
	uint32				_LineCapacity;
	std::vector<uint32>	_Line;

	// Strip/Fans (todo later).
	uint32				_StripIdx;
	uint32*				_Strip;
	uint32				_FanIdx;
	uint32*				_Fan;
public:
						CPrimitiveBlock(void) 
						{_TriCapacity=_NbTris= _NbQuads=_QuadCapacity=_NbLines=_LineCapacity= 0;};
						~CPrimitiveBlock(void) {}; 
	
	
	// Lines. A line is 2 uint32.
	
	/// reserve space for nLines Line. You are allowed to write your Line indices on this space.
	void				reserveLine(uint32 n);
	/// Return the number of Line reserved.
	uint32				capacityLine() {return _LineCapacity;}
	/// Set the number of active Line. It enlarge Line capacity, if needed.
	void				setNumLine(uint32 n);
	/// Get the number of active Lineangles.
	uint32				getNumLine(void) {return _NbLines;}

	/// Build a Lineangle.
	void				setLine(uint lineIdx, uint32 vidx0, uint32 vidx1);

	uint32*				getLinePointer(void);



	// Triangles. A triangle is 3 uint32.
	
	/// reserve space for nTris triangles. You are allowed to write your triangles indices on this space.
	void				reserveTri(uint32 n);
	/// Return the number of triangles reserved.
	uint32				capacityTri() {return _TriCapacity;}
	/// Set the number of active triangles. It enlarge Tri capacity, if needed.
	void				setNumTri(uint32 n);
	/// Get the number of active triangles.
	uint32				getNumTri(void) {return _NbTris;}

	/// Build a triangle.
	void				setTri(uint triIdx, uint32 vidx0, uint32 vidx1, uint32 vidx2);

	uint32*				getTriPointer(void);



	// Quads (a quad is 4 uint32)

	/**
	 *	reserve space for quads. 
	 */
	void reserveQuad(uint32 n);
	
	/**
	 * Return the number of triangles reserved.
	 */
	uint32 capacityQuad() { return _QuadCapacity; }
	
	/**
	 * Set the number of active quads. It enlarges Quad capacity, if needed.
	 */
	void setNumQuad(uint32 n);
	
	/**
	 * Get the number of active quads.
	 */
	uint32 getNumQuad(void) { return _NbQuads; }

	/**
	 * Build a quad.
	 */
	void setQuad(uint quadIdx, uint32 vidx0, uint32 vidx1, uint32 vidx2, uint32 vidx3);

	/**
	 * Return the Quad buffer
	 */
	uint32*	getQuadPointer(void);
};

// --------------------------------------------------

class GfxMode 
{
public:
	bool				Windowed;
	uint16				Width;
	uint16				Height;
	uint8				Depth;

						GfxMode(void) 
						{ 
							Windowed=false;
							Width=0;
							Height=0;
							Depth=0;
						}
						GfxMode(uint16 w, uint16 h, uint8 d, bool windowed= true);
};

typedef std::vector<GfxMode> ModeList;

// --------------------------------------------------

class IDriver
{
public:
	/// Version of the driver interface. To increment when the interface change.
	static const uint32						InterfaceVersion;

private:
	static IDriver*							_Current;

protected:
	typedef	std::list< CRefPtr<ITextureDrvInfos> >	TTexDrvInfoPtrList;
	typedef	std::list< CRefPtr<IShader> >			TShaderPtrList;
	typedef	std::list< CRefPtr<IVBDrvInfos> >		TVBDrvInfoPtrList;
	typedef	TTexDrvInfoPtrList::iterator			ItTexDrvInfoPtrList;
	typedef	TShaderPtrList::iterator				ItShaderPtrList;
	typedef	TVBDrvInfoPtrList::iterator				ItVBDrvInfoPtrList;


protected:
	TTexDrvInfoPtrList		_TexDrvInfos;
	TShaderPtrList			_Shaders;
	TVBDrvInfoPtrList		_VBDrvInfos;

public:
	enum TMessageBoxId { okId=0, yesId, noId, abortId, retryId, cancelId, ignoreId };
	enum TMessageBoxType { okType=0, okCancelType, yesNoType, abortRetryIgnoreType, yesNoCancelType, retryCancelType, typeCount };
	enum TMessageBoxIcon { noIcon=0, handIcon, questionIcon, exclamationIcon, asteriskIcon, warningIcon, errorIcon, informationIcon, stopIcon, iconCount };

							IDriver(void);
	virtual					~IDriver(void);

	virtual bool			init(void)=0;

	virtual ModeList		enumModes()=0;

	// first param is the associated window. 
	// Must be a HWND for Windows (WIN32).
	virtual bool			setDisplay(void* wnd, const GfxMode& mode)=0;

	/// Before rendering via a driver in a thread, must activate() (per thread).
	virtual bool			activate(void)=0;

	virtual NLMISC::IEventEmitter*	getEventEmitter(void)=0;

	virtual bool			clear2D(CRGBA rgba)=0;

	virtual bool			clearZBuffer(float zval=1)=0;

	virtual bool			setupTexture(ITexture& tex)=0;

	virtual bool			setupMaterial(CMaterial& mat)=0;

	// Setup the camera mode as a perspective/ortho camera. NB: znear and zfar must be >0 (if perspective).
	virtual void			setFrustum(float left, float right, float bottom, float top, float znear, float zfar, bool perspective=true)=0;

	virtual void			setupViewMatrix(const CMatrix& mtx)=0;

	virtual void			setupModelMatrix(const CMatrix& mtx, uint8 n=0)=0;

	virtual CMatrix			getViewMatrix(void)const=0;

	virtual bool			activeVertexBuffer(CVertexBuffer& VB)=0;

	virtual bool			render(CPrimitiveBlock& PB, CMaterial& Mat)=0;

	virtual bool			swapBuffers(void)=0;

	/// Deriver should calls IDriver::release() first, to destroy all driver components (textures, shaders, VBuffers).
	virtual bool			release(void);


	/** Output a system message box and print a message with an icon. This method can be call even if the driver is not initialized.
	  * This method is used to return internal driver problem when string can't be displayed in the driver window.
	  * If the driver can't open a messageBox, it should not override this method and let the IDriver class manage it with the ASCII console.
	  *
	  * \param message This is the message to display in the message box.
	  * \param title This is the title of the message box.
	  * \param type This is the type of the message box, ie number of button and label of buttons.
	  * \param icon This is the icon of the message box should use like warning, error etc...
	  */
	virtual TMessageBoxId	systemMessageBox (const char* message, const char* title, TMessageBoxType type=okType, TMessageBoxIcon icon=noIcon);

	/** Set the current viewport
	  *
	  * \param viewport is a viewport to setup as current viewport.
	  */
	virtual void			setupViewport (const class CViewport& viewport)=0;

	/**
	  * Get the driver version. Not the same than interface version. Incremented at each implementation change.
	  *
	  * \see InterfaceVersion
	  */
	virtual uint32			getImplementationVersion () const =0;

	/**
	  * Get driver informations.
	  *
	  */
	virtual const char*		getDriverInformation ()=0;

	virtual void showCursor(bool b) = 0;

	virtual void setMousePos(float x, float y) = 0;

};

// --------------------------------------------------

}

#include "nel/3d/driver_material_inline.h"

#endif // NL_DRV_H

