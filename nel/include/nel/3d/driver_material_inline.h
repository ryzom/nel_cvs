/** \file driver_material_inline.h
 * 
 * 
 *
 * $Id: driver_material_inline.h,v 1.6 2000/12/04 16:58:20 berenguier Exp $
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

#ifndef NL_DRIVER_MATERIAL_INLINE_H
#define NL_DRIVER_MATERIAL_INLINE_H

namespace NL3D
{

// --------------------------------------------------

inline bool CMaterial::texturePresent(uint8 n)
{
	nlassert(n<IDRV_MAT_MAXTEXTURES);
	if (_Textures[n])
	{
		return(true);
	}
	return(false);
}

inline ITexture*		CMaterial::getTexture(uint8 n)
{ 
	nlassert(n<IDRV_MAT_MAXTEXTURES);
	return(_Textures[n]); 
}

inline void CMaterial::setTexture(ITexture* ptex, uint8 n)
{
	nlassert(n<IDRV_MAT_MAXTEXTURES);
	_Textures[n]=ptex;
	_Touched|=IDRV_TOUCHED_TEX[n];
}

inline void CMaterial::setShader(TShader val)
{
	_ShaderType=val;
	_Touched|=IDRV_TOUCHED_SHADER;
}

inline void CMaterial::setOpacity(float val)
{
	_Opacity=val;
	_Touched|=IDRV_TOUCHED_OPACITY;
}

inline void CMaterial::setSrcBlend(TBlend val)
{
	_SrcBlend=val;
	_Touched|=IDRV_TOUCHED_BLENDFUNC;
}

inline void CMaterial::setDstBlend(TBlend val)
{
	_DstBlend=val;
	_Touched|=IDRV_TOUCHED_BLENDFUNC;
}

inline void	CMaterial::setBlendFunc(TBlend src, TBlend dst)
{
	_SrcBlend=src;
	_DstBlend=dst;
	_Touched|=IDRV_TOUCHED_BLENDFUNC;
}


inline void CMaterial::setZFunction(ZFunc val)
{
	_ZFunction=val;
	_Touched|=IDRV_TOUCHED_ZFUNC;
}

inline void CMaterial::setZBias(float val)
{
	_ZBias=val;
	_Touched|=IDRV_TOUCHED_ZBIAS;
}

inline void CMaterial::setColor(CRGBA rgba)
{
	_Color=rgba;
	_Touched|=IDRV_TOUCHED_COLOR;
}

inline void CMaterial::setBlend(bool active)
{
	if (active)	_Flags|=IDRV_MAT_BLEND;
	else		_Flags&=~IDRV_MAT_BLEND;
	_Touched|=IDRV_TOUCHED_BLEND;
}

inline void CMaterial::setLighting(	bool active, bool DefMat,
									CRGBA emissive, 
									CRGBA ambient, 
									CRGBA diffuse, 
									CRGBA specular )
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

inline void CMaterial::setAlpha(float val)
{
	_Alpha=val;
	_Touched|=IDRV_TOUCHED_ALPHA;
}

// --------------------------------------------------

}

#endif

