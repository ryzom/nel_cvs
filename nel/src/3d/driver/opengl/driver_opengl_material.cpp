/** \file driver_opengl_material.cpp
 * OpenGL driver implementation : setupMaterial
 *
 * $Id: driver_opengl_material.cpp,v 1.2 2000/11/06 14:34:14 viau Exp $
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

static bool convBlend(TBlend blend, GLenum& glenum)
{
	switch(blend)
	{
	case one:
		glenum=GL_ONE;
		return(true);
	case zero:
		glenum=GL_ZERO;
		return(true);
	case srcalpha:
		glenum=GL_SRC_ALPHA;
		return(true);
	case invsrcalpha:
		glenum=GL_ONE_MINUS_SRC_ALPHA;
		return(true);
	default:
		break;
	}
	return(false);
}

static bool convZFunction(ZFunc zfunc, GLenum& glenum)
{
	switch(zfunc)
	{
	case always:
		glenum=GL_ALWAYS;
		return(true);
	case never:
		glenum=GL_NEVER;
		return(true);
	case equal:
		glenum=GL_EQUAL;
		return(true);
	case notequal:
		glenum=GL_NOTEQUAL;
		return(true);
	case less:
		glenum=GL_LESS;
		return(true);
	case lessequal:
		glenum=GL_LEQUAL;
		return(true);
	case greater:
		glenum=GL_GREATER;
		return(true);
	case greaterequal:
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

	if (!mat.pShader)
	{
		mat.pShader=new CShaderGL;
	}
	pShader=static_cast<CShaderGL*>((IShader*)(mat.pShader));

	convBlend( mat.getSrcBlend(),glenum );
	pShader->SrcBlend=glenum;
	convBlend( mat.getDstBlend(),glenum );
	pShader->DstBlend=glenum;

	for(i=0 ; i<4 ; i++)
	{
		if ( mat.texturePresent(i) )
		{
			if ( mat.getTouched() & (IDRV_TOUCHED_TEX0<<i) )
			{
				if ( !setupTexture(mat.getTexture(i)) )
				{
					return(false);
				}
				mat.clearTouched();
			}
		}
	}
	mat.clearTouched();
	return(true);
}

// --------------------------------------------------

}
