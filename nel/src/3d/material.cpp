/** \file 3d/material.cpp
 * CMaterial implementation
 *
 * $Id: material.cpp,v 1.36 2002/08/09 09:31:13 berenguier Exp $
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

#include "std3d.h"

#include "3d/material.h"
#include "3d/texture.h"
#include "3d/shader.h"
#include "3d/driver.h"
#include "nel/misc/stream.h"

using namespace std;
using namespace NLMISC;

namespace NL3D
{

// ***************************************************************************
CMaterial::CMaterial()
{
	_Touched= 0;
	_Flags= IDRV_MAT_ZWRITE;
	// Must init All the flags by default.
	_ShaderType= Normal;
	_SrcBlend= srcalpha;
	_DstBlend= invsrcalpha;
	_ZFunction= lessequal;
	_ZBias= 0;
	_Color.set(255,255,255,255);
	_StainedGlassWindow = false;
	_AlphaTestThreshold= 0.5f;	
}

// ***************************************************************************
void			CMaterial::initUnlit()
{
	setShader(Normal);
	setLighting(false, false);
	setColor(CRGBA(255,255,255,255));
	for(sint i=0;i<IDRV_MAT_MAXTEXTURES;i++)
		setTexture(i ,NULL);
	setZBias(0);
	setZFunc(lessequal);
	setZWrite(true);
	setBlend(false);
	setAlphaTestThreshold(0.5f);
}

// ***************************************************************************

void			CMaterial::initLighted()
{
	initUnlit();
	setLighting(true, false);
}


// ***************************************************************************
CMaterial		&CMaterial::operator=(const CMaterial &mat)
{
	_ShaderType= mat._ShaderType;
	_Flags= mat._Flags;
	_SrcBlend= mat._SrcBlend;
	_DstBlend= mat._DstBlend;
	_ZFunction= mat._ZFunction;
	_ZBias= mat._ZBias;
	_Color= mat._Color;
	_Emissive= mat._Emissive;
	_Ambient= mat._Ambient;
	_Diffuse= mat._Diffuse;
	_Specular= mat._Specular;
	_Shininess= mat._Shininess;
	_AlphaTestThreshold= mat._AlphaTestThreshold;

	for(sint i=0;i<IDRV_MAT_MAXTEXTURES;i++)
	{
		_Textures[i]= mat._Textures[i];
		_TexEnvs[i]= mat._TexEnvs[i];
		_TexAddrMode[i] = mat._TexAddrMode[i];
	}

	// copy lightmaps.
	_LightMaps= mat._LightMaps;

	// copy texture matrix if there.
	if (mat._TexUserMat.get())
	{
		std::auto_ptr<CUserTexMat> texMatClone( new CUserTexMat(*(mat._TexUserMat))); // make cpy
		std::swap(texMatClone, _TexUserMat); // swap with old
	}
	else
	{
		_TexUserMat.reset();	
	}

	// Must do not copy drv info.

	// All states of material is modified.
	_Touched= IDRV_TOUCHED_ALL;

	return *this;
}


// ***************************************************************************
CMaterial::~CMaterial()
{
	// Must kill the drv mirror of this material.
	pShader.kill();	
}


// ***************************************************************************
void		CMaterial::serial(NLMISC::IStream &f)
{
	/*
	Version 6:
		- Texture matrix animation
	Version 5:
		- AlphaTest threshold
	Version 4:
		- Texture Addressing modes
	Version 3:
		- LightMaps.
	Version 2:
		- Shininess.
	Version 1:
		- texture environement.
	Version 0:
		- base version.
	*/

	sint	ver= f.serialVersion(6);
	// For the version <=1:
	nlassert(IDRV_MAT_MAXTEXTURES==4);

	f.serialEnum(_ShaderType);
	f.serial(_Flags);
	f.serialEnum(_SrcBlend);
	f.serialEnum(_DstBlend);
	f.serialEnum(_ZFunction);
	f.serial(_ZBias);
	f.serial(_Color);
	f.serial(_Emissive, _Ambient, _Diffuse, _Specular);
	if(ver>=2)
	{
		f.serial(_Shininess);
	}
	if(ver>=5)
	{
		f.serial(_AlphaTestThreshold);
	}


	for(sint i=0;i<IDRV_MAT_MAXTEXTURES;i++)
	{
		// Serial texture descriptor.
		ITexture*	text;
		if(f.isReading())
		{
			f.serialPolyPtr(text);
			_Textures[i]= text;
		}
		else
		{
			text= _Textures[i];
			f.serialPolyPtr(text);
		}

		// Read texture environnement, or setup them.
		if(ver>=1)
		{
			f.serial(_TexEnvs[i]);
		}
		else
		{
			// Else setup as default behavior, like before...
			if(f.isReading())
				_TexEnvs[i].setDefault();
		}
	}

	if(ver>=3)
	{
		f.serialCont(_LightMaps);
	}

	if (ver >= 4)
	{
		if (_Flags & IDRV_MAT_TEX_ADDR)
		{
			for(sint i=0;i<IDRV_MAT_MAXTEXTURES;i++)
			{
				f.serial(_TexAddrMode[i]);
			}
		}
	}

	if(f.isReading())
	{
		// All states of material are modified.
		_Touched= IDRV_TOUCHED_ALL;

		if ((_Flags & IDRV_MAT_USER_TEX_MAT_ALL)) // are there user textrue coordinates matrix ?
		{
			std::auto_ptr<CUserTexMat> newPtr(new CUserTexMat); // create new			
			std::swap(_TexUserMat, newPtr); // replace old
		}
	}

	if (ver >= 6)
	{
		for(uint i=0; i < IDRV_MAT_MAXTEXTURES; ++i)
		{
			if (isUserTexMatEnabled(i))
			{				
				f.serial(_TexUserMat->TexMat[i]);				
			}			
		}
	}

}


// ***************************************************************************
void		CMaterial::setShader(TShader val)
{
	// First, reset all textures.
	uint	nTexts= IDRV_MAT_MAXTEXTURES;
	// If user color or lightmap, set only the 1st.
	if(_ShaderType==LightMap || _ShaderType==UserColor)
		nTexts=1;
	// reset all needed
	for(sint i=0;i<nTexts;i++)
		setTexture(i ,NULL);

	// If userColor, use TexEnv caps (we got it, so use it :) ).
	if(val== CMaterial::UserColor)
	{
		// force normal, to setup TexEnvMode correclty. 
		_ShaderType=CMaterial::Normal;

		// First stage, interpolate Constant and texture with Alpha of texture.
		texEnvOpRGB(0, InterpolateTexture);
		texEnvArg0RGB(0, Texture, SrcColor);
		texEnvArg1RGB(0, Constant, SrcColor);
		// And just use Alpha Diffuse.
		texEnvOpAlpha(0, Replace);
		texEnvArg0Alpha(0, Previous, SrcAlpha);

		// Second stage, modulate result with diffuse color.
		texEnvOpRGB(1, Modulate);
		texEnvArg0RGB(1, Previous, SrcColor);
		texEnvArg1RGB(1, Diffuse, SrcColor);
		// And just use Alpha Diffuse.
		texEnvOpAlpha(1, Replace);
		texEnvArg0Alpha(1, Previous, SrcAlpha);
	}

	_ShaderType= val;
	_Touched|=IDRV_TOUCHED_SHADER;
}


// ***************************************************************************
void CMaterial::setTexture(uint8 n, ITexture* ptex)
{
	nlassert(n<IDRV_MAT_MAXTEXTURES);

	// User Color material?
	if( _ShaderType== CMaterial::UserColor)
	{
		// user color. Only texture 0 can be set.
		nlassert( n==0 );

		// Affect the 2 first textures.
		_Textures[0]=ptex;
		_Textures[1]=ptex;
		_Touched|=IDRV_TOUCHED_TEX[0];
		_Touched|=IDRV_TOUCHED_TEX[1];
	}
	else if( _ShaderType== CMaterial::LightMap)
	{
		// Only texture 0 can be set.
		nlassert( n==0 );
		_Textures[n]=ptex;
		_Touched|=IDRV_TOUCHED_TEX[n];
	}
	// Normal material?
	else
	{
		_Textures[n]=ptex;
		_Touched|=IDRV_TOUCHED_TEX[n];
	}
}


// ***************************************************************************
void			CMaterial::flushTextures (IDriver &driver)
{
	// For each textures
	for (uint tex=0; tex<IDRV_MAT_MAXTEXTURES; tex++)
	{
		// Texture exist ?
		if (_Textures[tex])
		{
			// Force setup texture
			driver.setupTexture (*_Textures[tex]);
		}
	}

	// If Lightmap material
	if(_ShaderType==LightMap)
	{
		// For each lightmap
		for (uint lmap=0; lmap<_LightMaps.size(); lmap++)
		{
			// Texture exist?
			if(_LightMaps[lmap].Texture)
			{
				// Force setup texture
				driver.setupTexture (*_LightMaps[lmap].Texture);
			}
		}
	}

}


// ***************************************************************************
void					CMaterial::setLightMap(uint lmapId, ITexture *lmap)
{
	nlassert(_ShaderType==CMaterial::LightMap);
	if(lmapId>=_LightMaps.size())
		_LightMaps.resize(lmapId+1);
	_LightMaps[lmapId].Texture= lmap;

	_Touched|=IDRV_TOUCHED_LIGHTMAP;
}

// ***************************************************************************
ITexture				*CMaterial::getLightMap(uint lmapId) const
{
	nlassert(_ShaderType==CMaterial::LightMap);
	if(lmapId<_LightMaps.size())
		return _LightMaps[lmapId].Texture;
	else
		return NULL;	
}

// ***************************************************************************
void					CMaterial::setLightMapFactor(uint lmapId, CRGBA factor)
{
	if (_ShaderType==CMaterial::LightMap)
	{
		if(lmapId>=_LightMaps.size())
			_LightMaps.resize(lmapId+1);
		_LightMaps[lmapId].Factor= factor;

		_Touched|=IDRV_TOUCHED_LIGHTMAP;
	}
}


// ***************************************************************************
void			CMaterial::CLightMap::serial(NLMISC::IStream &f)
{
	f.serial(Factor);
	// Serial texture descriptor.
	ITexture*	text= NULL;
	if(f.isReading())
	{
		f.serialPolyPtr(text);
		Texture= text;
	}
	else
	{
		text= Texture;
		f.serialPolyPtr(text);
	}
}



// ***************************************************************************
void				CMaterial::enableTexAddrMode(bool enable /*= true*/)
{
	if (enable)
	{
		if (!(_Flags & IDRV_MAT_TEX_ADDR))
		{
			_Flags |= IDRV_MAT_TEX_ADDR;
			for (sint k = 0; k < IDRV_MAT_MAXTEXTURES; ++k)
			{
				_TexAddrMode[k] = (uint8) FetchTexture;
			}			
		}		
	}
	else
	{
		_Flags &= ~IDRV_MAT_TEX_ADDR;			
	}
}

// ***************************************************************************
bool			    CMaterial::texAddrEnabled() const
{
	return( _Flags & IDRV_MAT_TEX_ADDR) != 0;
}

// ***************************************************************************
void				CMaterial::setTexAddressingMode(uint8 stage, TTexAddressingMode mode)
{
	nlassert(_Flags & IDRV_MAT_TEX_ADDR);
	nlassert(stage < IDRV_MAT_MAXTEXTURES);
	nlassert(mode < TexAddrCount);
	_TexAddrMode[stage] = (uint8) mode;	
}


// ***************************************************************************
CMaterial::TTexAddressingMode	CMaterial::getTexAddressingMode(uint8 stage)
{
	nlassert(_Flags & IDRV_MAT_TEX_ADDR);
	nlassert(stage < IDRV_MAT_MAXTEXTURES);
	return (TTexAddressingMode) _TexAddrMode[stage];
}

// ***************************************************************************
void					CMaterial::decompUserTexMat(uint stage, float &uTrans, float &vTrans, float &wRot, float &uScale, float &vScale)
{
	nlassert(stage < IDRV_MAT_MAXTEXTURES);
	nlassert(isUserTexMatEnabled(stage)); // must activate animated texture matrix for this stage
	const NLMISC::CMatrix texMat = _TexUserMat->TexMat[stage];
	uTrans = texMat.getPos().x;
	vTrans = texMat.getPos().y;
	/// find the rotation around w
	NLMISC::CVector i = texMat.getI();
	NLMISC::CVector j = texMat.getJ();
	float  normI = i.norm();
	float  normJ = i.norm();
	i /= normI;
	j /= normJ;	
	float angle = ::acosf(i.x);
	if (i.y < 0)
	{
		angle = 2.f * (float) NLMISC::Pi - angle;
	}
	wRot   = angle;	
	uScale = normI;
	vScale = normJ;
}

// ***************************************************************************
void		CMaterial::selectTextureSet(uint index)
{
	for (uint k = 0; k < IDRV_MAT_MAXTEXTURES; ++k)
	{
		if (_Textures[k] != NULL) _Textures[k]->selectTexture(index);
	}
}

}

