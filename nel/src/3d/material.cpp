/** \file material.cpp
 * CMaterial implementation
 *
 * $Id: material.cpp,v 1.9 2000/12/12 10:04:48 berenguier Exp $
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

#include "nel/3d/driver.h"

using std::vector;

namespace NL3D
{


// ***************************************************************************

void			CMaterial::initUnlit()
{
	setShader(normal);
	setLighting(false);
	setColor(CRGBA(255,255,255,255));
	for(sint i=0;i<IDRV_MAT_MAXTEXTURES;i++)
		setTexture(NULL, i);
	setZBias(0);
	setZFunction(lessequal);
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
		_Textures[i]= mat._Textures[i];

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
	sint	ver= f.serialVersion(0);
	// For the version 0:
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
	}

	if(f.isReading())
		// All states of material are modified.
		_Touched= IDRV_TOUCHED_ALL;

}


}

