/** \file driver.h
 * Generic driver header.
 * Low level HW classes : CTexture, Cmaterial, CVertexBuffer, CPrimitiveBlock, IDriver
 *
 * $Id: driver.h,v 1.9 2000/11/07 15:35:11 berenguier Exp $
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

#ifndef NL_IDRV_H
#define NL_IDRV_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/rgba.h"
#include "nel/misc/matrix.h"

#include <vector>
#include <list>

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
};

// --------------------------------------------------

class ITextureDrvInfos : public CRefCount
{
private:
public:
			ITextureDrvInfos() {};
			ITextureDrvInfos(class IDriver& driver);
			virtual ~ITextureDrvInfos(void){ };
};

class CTexture : public CRefCount
{
private:
	uint16					_Width;
	uint16					_Height;
	std::vector<CRGBA>		_Data;

public:
	CRefPtr<ITextureDrvInfos> DrvInfos;

public:
							CTexture(void);
							CTexture(uint16 width, uint16 height);

	void					resize(uint16 width, uint16 height);

	uint16					getWidth() const { return(_Width); }
	uint16					getHeight() const { return(_Height); }

	bool					fillData(const void* data);
	bool					fillData(const std::vector<CRGBA>& data);
	void*					getDataPointer() {return( &(*_Data.begin()) );}
};

// --------------------------------------------------

const uint32 IDRV_TOUCHED_SRCBLEND	=	0x00000001;
const uint32 IDRV_TOUCHED_DSTBLEND	=	0x00000002;
const uint32 IDRV_TOUCHED_OPACITY	=	0x00000004;
const uint32 IDRV_TOUCHED_SHADER	=	0x00000008;
const uint32 IDRV_TOUCHED_ZFUNC		=	0x00000010;
const uint32 IDRV_TOUCHED_ZBIAS		=	0x00000020;
const uint32 IDRV_TOUCHED_COLOR		=	0x00000040;
const uint32 IDRV_TOUCHED_LIGHTING	=	0x00000080;
const uint32 IDRV_TOUCHED_DEFMAT	=	0x00000100;
const uint32 IDRV_TOUCHED_ALPHA		=	0x00000200;
const uint32 IDRV_TOUCHED_TEX0		=	0x00000400;
const uint32 IDRV_TOUCHED_TEX1		=	0x00000800;
const uint32 IDRV_TOUCHED_TEX2		=	0x00001000;
const uint32 IDRV_TOUCHED_TEX3		=	0x00002000;

const uint32	IDRV_MAT_HIDE		= 0x00000001;
const uint32	IDRV_MAT_TSP		= 0x00000002;
const uint32	IDRV_MAT_ZWRITE		= 0x00000004;
const uint32	IDRV_MAT_ZLIST		= 0x00000008;
const uint32	IDRV_MAT_LIGHTING	= 0x00000010;
const uint32	IDRV_MAT_SPECULAR	= 0x00000020;
const uint32	IDRV_MAT_DEFMAT		= 0x00000040;
const uint32	IDRV_MAT_BLEND		= 0x00000080;


class CMaterial : public CRefCount
{
public:
	enum ZFunc		{ always,never,equal,notequal,less,lessequal,greater,greaterequal };
	enum TBlend		{ one, zero, srcalpha, invsrcalpha };
	enum TShader	{ normal, user_color, envmap, bump};


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

	CSmartPtr<CTexture>		pTex[4];

public:
	// Private. For Driver only.
	CRefPtr<IShader>		pShader;


public:
	CMaterial() {_Touched= 0;}

	uint32					getTouched(void) { return(_Touched); }

	void					clearTouched(void) { _Touched=0; }

	bool					texturePresent(uint8 n)
	{
		if (pTex[n])
		{
			return(true);
		}
		return(false);
	}

	CTexture&				getTexture(uint8 n)
	{
		return(*pTex[n]);
	}

	void 					setTexture(CTexture* ptex, uint8 n=0)
	{
		pTex[n]=ptex;
		switch(n)
		{
		case 0:
			_Touched|=IDRV_TOUCHED_TEX0;
		case 1:
			_Touched|=IDRV_TOUCHED_TEX1;
		case 2:
			_Touched|=IDRV_TOUCHED_TEX2;
		case 3:
			_Touched|=IDRV_TOUCHED_TEX3;
		default:
			break;
		}
	}

	void					setShader(TShader val)
	{
		_ShaderType=val;
		_Touched|=IDRV_TOUCHED_SHADER;
	}

	void					setOpacity(float val)
	{
		_Opacity=val;
		_Touched|=IDRV_TOUCHED_OPACITY;
	}

	TBlend					getSrcBlend(void) { return(_SrcBlend); }
	void					setSrcBlend(TBlend val)
	{
		_SrcBlend=val;
		_Touched|=IDRV_TOUCHED_SRCBLEND;
	}

	TBlend					getDstBlend(void) { return(_DstBlend); }
	void					setDstBlend(TBlend val)
	{
		_DstBlend=val;
		_Touched|=IDRV_TOUCHED_DSTBLEND;
	}

	ZFunc					getZFunc(void) { return(_ZFunction); }		
	void					setZFunction(ZFunc val)
	{
		_ZFunction=val;
		_Touched|=IDRV_TOUCHED_ZFUNC;
	}

	float					getZBias(void) { return(_ZBias); }
	void					setZBias(float val)
	{
		_ZBias=val;
		_Touched|=IDRV_TOUCHED_ZBIAS;
	}

	CRGBA					getColor(void) { return(_Color); }
	void					setColor(CRGBA& rgba)
	{
		_Color=rgba;
		_Touched|=IDRV_TOUCHED_COLOR;
	}

	void					setBlend(bool active)
	{
		if (active)	_Flags|=IDRV_MAT_BLEND;
		else		_Flags&=~IDRV_MAT_BLEND;
	}

	void					setLighting(	bool active, bool DefMat=true,
											CRGBA& emissive=CRGBA(0,0,0), 
											CRGBA& ambient=CRGBA(0,0,0), 
											CRGBA& diffuse=CRGBA(0,0,0), 
											CRGBA& specular=CRGBA(0,0,0) )
	{
		if (active)
		{
			_Flags|=IDRV_MAT_LIGHTING;
			if (DefMat)
			{
				_Flags|=IDRV_MAT_DEFMAT;
			}
			else
			{
				_Flags&=~IDRV_MAT_DEFMAT;
			}
		}
		else
		{
			_Flags&=~IDRV_MAT_LIGHTING;
		}
		_Emissive=emissive;
		_Ambient=ambient;
		_Diffuse=diffuse;
		_Specular=specular;
		_Touched|=IDRV_TOUCHED_LIGHTING;
	}

	void					setAlpha(float val)
	{
		_Alpha=val;
		_Touched|=IDRV_TOUCHED_ALPHA;
	}

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

class IVBDrvInfos
{
private:
public:
};
// All these flags are similar to DX8
const uint8		IDRV_VF_MAXW		=	4;
const uint8		IDRV_VF_MAXSTAGES	=	8;
const uint32	IDRV_VF_XYZ			=	0x00000001;
const uint32	IDRV_VF_W[4]		= { 0x00000002,0x00000004,0x00000008,0x00000010 };
const uint32	IDRV_VF_NORMAL		=	0x00000020;
const uint32	IDRV_VF_RGBA		=	0x00000040;
const uint32	IDRV_VF_SPECULAR	=	0x00000080;
const uint32	IDRV_VF_UV[8]		= { 0x00000100,0x00000200,0x00000400,0x00000800,0x00001000,0x00002000,0x00004000,0x00008000 };

class CVertexBuffer : public NLMISC::CRefCount
{
private:
	uint32					_Flags;
	uint8					_VertexSize;
	uint16					_NbVerts;
	std::vector<uint8>		_Verts;

	uint					_WOff[4];
	uint					_NormalOff;
	uint					_RGBAOff;
	uint					_SpecularOff;
	uint					_UVOff[8];

public:
							CVertexBuffer(void);

	CRefPtr<IVBDrvInfos>	DrvInfos;
	uint32					getFlags(void) { return(_Flags); };

	bool					setVertexFormat(uint32 Flags);
	bool					reserve(uint16 n);
	sint					capacity() {return _Verts.size();}
	bool					setNumVertices(uint16 n);

	bool					setVertexCoord(uint idx, float x, float y, float z);
	bool					setVertexCoord(uint idx, const CVector &v);
	bool					setNormalCoord(uint idx, const CVector &v);
	bool					setRGBA(uint idx, CRGBA& rgba);
	bool					setTexCoord(uint idx, uint8 stage, float u, float v);
							~CVertexBuffer(void);

	uint8					getVertexSize(void)
	{
		return(_VertexSize);
	}
	uint16					getNumVertices(void)
	{
		return(_NbVerts);
	}

	void*					getVertexCoordPointer(uint idx=0);
	void*					getNormalCoordPointer(uint idx=0);
	void*					getColorPointer(uint idx=0);
	void*					getTexCoordPointer(uint idx=0, uint8 stage=0);
};

// --------------------------------------------------

class CPrimitiveBlock
{
private:
	uint16				_TriIdx;
	std::vector<uint32>	_Tri;
	uint16				_StripIdx;
	uint16*				_Strip;
	uint16				_FanIdx;
	uint16*				_Fan;
public:
						CPrimitiveBlock(void) {};
						~CPrimitiveBlock(void) {}; 
	bool				setNumTri(uint16 n);
	bool				addTri(uint16 idx1, uint16 idx2, uint16 idx3);
	uint16				getNumTri(void);
	void*				getTriPointer(void);
};

// --------------------------------------------------

class GfxMode 
{
public:
	bool			Windowed;
	uint16			Width;
	uint16			Height;
	uint8			Depth;

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
friend class ITextureDrvInfos;
private:
	std::list< CRefPtr<ITextureDrvInfos> >	_pTexDrvInfos;
public:
							IDriver(void) { };
	virtual					~IDriver(void) 
	{ 
		std::list< CRefPtr<ITextureDrvInfos> >::iterator it = _pTexDrvInfos.begin();
		while( it!=_pTexDrvInfos.end() )
		{
			it->kill();
			it++;
		}
	};

	virtual bool			init(void)=0;

	virtual ModeList		enumModes()=0;

	// first param is the associated window. 
	// Must be a HWND for Windows (WIN32).
	virtual bool			setDisplay(void* wnd, const GfxMode& mode)=0;

	virtual bool			processMessages(void)=0;

	/// Before rendering via a driver in a thread, must activate() (per thread).
	virtual bool			activate(void)=0;

	virtual bool			clear2D(CRGBA& rgba)=0;

	virtual bool			clearZBuffer(float zval=1)=0;

	virtual bool			setupTexture(CTexture& tex)=0;

	virtual bool			setupMaterial(CMaterial& mat)=0;

	/// Setup the camera mode as a perspective/ortho camera. NB: znear and zfar must be >0 (if perspective).
	virtual void			setFrustum(float left, float right, float bottom, float top, float znear, float zfar, bool perspective= true)=0;

	virtual void			setupViewMatrix(const CMatrix& mtx)=0;

	virtual void			setupModelMatrix(const CMatrix& mtx, uint8 n=0)=0;

	virtual CMatrix			getViewMatrix(void) const=0;

	virtual bool			activeVertexBuffer(CVertexBuffer& VB)=0;

	virtual bool			render(CPrimitiveBlock& PB, CMaterial& Mat)=0;

	virtual bool			swapBuffers(void)=0;

	virtual bool			release(void)=0;


};

// --------------------------------------------------

}

#endif // NL_IDRV_H

