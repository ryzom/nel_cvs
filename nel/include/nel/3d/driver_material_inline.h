/** \file driver_material_inline.h
 * <File description>
 *
 * $Id: driver_material_inline.h,v 1.12 2001/01/11 13:53:29 lecroart Exp $
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

#include "nel/misc/debug.h"

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

inline void CMaterial::setTexture(uint8 n, ITexture* ptex)
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

inline void CMaterial::setBlend(bool active)
{
	if (active)	_Flags|=IDRV_MAT_BLEND;
	else		_Flags&=~IDRV_MAT_BLEND;
	_Touched|=IDRV_TOUCHED_BLEND;
}

inline void	CMaterial::setBlendFunc(TBlend src, TBlend dst)
{
	_SrcBlend=src;
	_DstBlend=dst;
	_Touched|=IDRV_TOUCHED_BLENDFUNC;
}


inline void CMaterial::setZFunc(ZFunc val)
{
	_ZFunction=val;
	_Touched|=IDRV_TOUCHED_ZFUNC;
}

inline void	CMaterial::setZWrite(bool active)
{
	if (active)	_Flags|=IDRV_MAT_ZWRITE;
	else		_Flags&=~IDRV_MAT_ZWRITE;
	_Touched|=IDRV_TOUCHED_ZWRITE;
}

inline void CMaterial::setZBias(float val)
{
	_ZBias=val;
	_Touched|=IDRV_TOUCHED_ZBIAS;
}

inline void CMaterial::setColor(NLMISC::CRGBA rgba)
{
	_Color=rgba;
	_Touched|=IDRV_TOUCHED_COLOR;
}

inline void CMaterial::setLighting(	bool active, bool DefMat,
									NLMISC::CRGBA emissive, 
									NLMISC::CRGBA ambient, 
									NLMISC::CRGBA diffuse, 
									NLMISC::CRGBA specular )
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


// ***************************************************************************
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
inline void					CMaterial::texEnvOpRGB(uint stage, TTexOperator ope)
{
	nlassert(stage>=0 && stage<IDRV_MAT_MAXTEXTURES);
	_TexEnvs[stage].Env.OpRGB= ope;
}
// ***************************************************************************
inline void					CMaterial::texEnvArg0RGB(uint stage, TTexSource src, TTexOperand oper)
{
	nlassert(stage>=0 && stage<IDRV_MAT_MAXTEXTURES);
	_TexEnvs[stage].Env.SrcArg0RGB= src;
	_TexEnvs[stage].Env.OpArg0RGB= oper;
}
// ***************************************************************************
inline void					CMaterial::texEnvArg1RGB(uint stage, TTexSource src, TTexOperand oper)
{
	nlassert(stage>=0 && stage<IDRV_MAT_MAXTEXTURES);
	nlassert(src!=Texture);
	_TexEnvs[stage].Env.SrcArg1RGB= src;
	_TexEnvs[stage].Env.OpArg1RGB= oper;
}


// ***************************************************************************
inline void					CMaterial::texEnvOpAlpha(uint stage, TTexOperator ope)
{
	nlassert(stage>=0 && stage<IDRV_MAT_MAXTEXTURES);
	_TexEnvs[stage].Env.OpAlpha= ope;
}
// ***************************************************************************
inline void					CMaterial::texEnvArg0Alpha(uint stage, TTexSource src, TTexOperand oper)
{
	nlassert(stage>=0 && stage<IDRV_MAT_MAXTEXTURES);
	nlassert(oper==SrcAlpha || oper==InvSrcAlpha);
	_TexEnvs[stage].Env.SrcArg0Alpha= src;
	_TexEnvs[stage].Env.OpArg0Alpha= oper;
}
// ***************************************************************************
inline void					CMaterial::texEnvArg1Alpha(uint stage, TTexSource src, TTexOperand oper)
{
	nlassert(stage>=0 && stage<IDRV_MAT_MAXTEXTURES);
	nlassert(oper==SrcAlpha || oper==InvSrcAlpha);
	nlassert(src!=Texture);
	_TexEnvs[stage].Env.SrcArg1Alpha= src;
	_TexEnvs[stage].Env.OpArg1Alpha= oper;
}


// ***************************************************************************
inline void					CMaterial::texConstantColor(uint stage, CRGBA color)
{
	nlassert(stage>=0 && stage<IDRV_MAT_MAXTEXTURES);
	_TexEnvs[stage].ConstantColor= color;
}


// ***************************************************************************
inline uint32				CMaterial::getTexEnvMode(uint stage)
{
	nlassert(stage>=0 && stage<IDRV_MAT_MAXTEXTURES);
	return _TexEnvs[stage].Env.Packed;
}
// ***************************************************************************
inline void					CMaterial::setTexEnvMode(uint stage, uint32 packed)
{
	nlassert(stage>=0 && stage<IDRV_MAT_MAXTEXTURES);
	_TexEnvs[stage].Env.Packed= packed;
}
// ***************************************************************************
inline CRGBA				CMaterial::getTexConstantColor(uint stage)
{
	nlassert(stage>=0 && stage<IDRV_MAT_MAXTEXTURES);
	return _TexEnvs[stage].ConstantColor;
}


}

#endif

