/** \file material.cpp
 * CMaterial implementation
 *
 * $Id: material.cpp,v 1.13 2001/01/08 18:20:28 berenguier Exp $
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

#include "nel/3d/material.h"
#include "nel/3d/texture.h"
#include "nel/3d/shader.h"
#include "nel/misc/stream.h"

using namespace std;
using namespace NLMISC;

namespace NL3D
{

// ***************************************************************************
CMaterial::CMaterial()
{
	_Touched= 0;
	_Flags= IDRV_MAT_ZWRITE ;
	// Must init All the flags by default.
	_ShaderType= Normal;
	_SrcBlend= srcalpha;
	_DstBlend= invsrcalpha;
	_ZFunction= lessequal;
	_ZBias= 0;
	_Color.set(255,255,255,255);

}

// ***************************************************************************
void			CMaterial::initUnlit()
{
	setShader(Normal);
	setLighting(false);
	setColor(CRGBA(255,255,255,255));
	for(sint i=0;i<IDRV_MAT_MAXTEXTURES;i++)
		setTexture(NULL, i);
	setZBias(0);
	setZFunc(lessequal);
	setZWrite(true);
	setBlend(false);
}

// ***************************************************************************

void			CMaterial::initLighted()
{
	initUnlit();
	setLighting(true);
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

	for(sint i=0;i<IDRV_MAT_MAXTEXTURES;i++)
	{
		_Textures[i]= mat._Textures[i];
		_TexEnvs[i]= mat._TexEnvs[i];
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
	Version 1:
		- texture environement.
	Version 0:
		- base version.
	*/

	sint	ver= f.serialVersion(1);
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
			f.serial(_TexEnvs[i].Env.Packed);
			f.serial(_TexEnvs[i].ConstantColor);
		}
		else
		{
			// Else setup as default behavior, like before...
			if(f.isReading())
				_TexEnvs[i].setDefault();
		}
	}

	if(f.isReading())
		// All states of material are modified.
		_Touched= IDRV_TOUCHED_ALL;

}


}

