/** \file driver_opengl_material.cpp
 * OpenGL driver implementation : setupMaterial
 *
 * $Id: driver_opengl_material.cpp,v 1.12 2000/12/04 16:58:59 berenguier Exp $
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

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <gl/gl.h>
#include "driver_opengl.h"

// --------------------------------------------------

namespace NL3D
{

// --------------------------------------------------

static bool convBlend(CMaterial::TBlend blend, GLenum& glenum)
{
	switch(blend)
	{
	case CMaterial::one:
		glenum=GL_ONE;
		return(true);
	case CMaterial::zero:
		glenum=GL_ZERO;
		return(true);
	case CMaterial::srcalpha:
		glenum=GL_SRC_ALPHA;
		return(true);
	case CMaterial::invsrcalpha:
		glenum=GL_ONE_MINUS_SRC_ALPHA;
		return(true);
	default:
		break;
	}
	return(false);
}

static bool convZFunction(CMaterial::ZFunc zfunc, GLenum& glenum)
{
	switch(zfunc)
	{
	case CMaterial::always:
		glenum=GL_ALWAYS;
		return(true);
	case CMaterial::never:
		glenum=GL_NEVER;
		return(true);
	case CMaterial::equal:
		glenum=GL_EQUAL;
		return(true);
	case CMaterial::notequal:
		glenum=GL_NOTEQUAL;
		return(true);
	case CMaterial::less:
		glenum=GL_LESS;
		return(true);
	case CMaterial::lessequal:
		glenum=GL_LEQUAL;
		return(true);
	case CMaterial::greater:
		glenum=GL_GREATER;
		return(true);
	case CMaterial::greaterequal:
		glenum=GL_GEQUAL;
		return(true);
	default:
		return(false);
	}
	return(false);
}

// --------------------------------------------------

bool CDriverGL::setupMaterial(CMaterial& mat)
{
	CShaderGL*	pShader;
	GLenum		glenum;
	uint		i;
	uint32		touched=mat.getTouched();


	// 0. Setup / Bind Textures.
	//==========================
	// Must setup textures each frame. (need to test if touched).
	for(i=0 ; i<IDRV_MAT_MAXTEXTURES ; i++)
	{
		ITexture	*text= mat.getTexture(i);
		if ( text )
		{
			if ( !setupTexture(*text) )
			{
				return(false);
			}
		}
		activateTexture(i,text);
	}

	
	// 1. Retrieve/Create driver shader.
	//==================================
	if (!mat.pShader)
	{
		mat.pShader=new CShaderGL;
		// insert into driver list.
		_Shaders.push_back(mat.pShader);

		// Must create all OpenGL shader states.
		touched= 0xFFFFFFFF;
	}
	pShader=static_cast<CShaderGL*>((IShader*)(mat.pShader));


	// 2. Setup modified flags of material.
	//=====================================
	if(touched)
	{
		// Convert Material to driver shader.
		if (touched & IDRV_TOUCHED_BLENDFUNC)
		{
			convBlend( mat.getSrcBlend(),glenum );
			pShader->SrcBlend=glenum;
			convBlend( mat.getDstBlend(),glenum );
			pShader->DstBlend=glenum;
		}

		// Optimize: reset all flags at the end.
		mat.clearTouched(0xFFFFFFFF);

		// Since modified, must rebind all openGL states.
		_CurrentMaterial= NULL;
	}


	// 3. Bind OpenGL States.
	//=======================
	if (_CurrentMaterial!=&mat)
	{
		// \todo: yoyo: optimize with precedent material flags test.
		// => must change the way it works with _CurrentMaterial.

		// Bind Blend Part.
		if(mat.getFlags()&IDRV_MAT_BLEND)
		{
			glEnable(GL_BLEND);
			glBlendFunc(pShader->SrcBlend, pShader->DstBlend);
		}
		else
		{
			glDisable(GL_BLEND);
		}
		// Color unlit part.
		CRGBA	col= mat.getColor();
		glColor4ub(col.R, col.G, col.B, col.A);


		_CurrentMaterial=&mat;
	}


	// TempYoyo.
    //glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	
	return(true);
}


// --------------------------------------------------

}
