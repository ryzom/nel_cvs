/** \file driver_opengl_states.cpp
 * <File description>
 *
 * $Id: driver_opengl_states.cpp,v 1.1 2001/09/20 16:43:10 berenguier Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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


// Must include "driver_opengl.h" for good GL.h definition
#include "driver_opengl.h"


// ***************************************************************************
// define it For Debug purpose only. Normal use is to hide this line
//#define		NL3D_GLSTATE_DISABLE_CACHE

namespace NL3D 
{

// ***************************************************************************
CDriverGLStates::CDriverGLStates()
{
}

// ***************************************************************************
void			CDriverGLStates::forceDefaults()
{
	// Enable / disable.
	_CurBlend= false;
	_CurCullFace= true;
	_CurAlphaTest= false;
	_CurLighting= false;
	_CurZWrite= true;
	// setup GLStates.
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_LIGHTING);
	glDepthMask(GL_TRUE);

	// Func.
	_CurBlendSrc= GL_SRC_ALPHA;
	_CurBlendDst= GL_ONE_MINUS_SRC_ALPHA;
	_CurDepthFunc= GL_LEQUAL;
	// setup GLStates.
	glBlendFunc(_CurBlendSrc, _CurBlendDst);
	glDepthFunc(_CurDepthFunc);

	// Materials.
	uint32			packedOne= (CRGBA(255,255,255,255)).getPacked();
	uint32			packedZero= (CRGBA(0,0,0,255)).getPacked();
	_CurEmissive= packedZero;
	_CurAmbient= packedOne;
	_CurDiffuse= packedOne;
	_CurSpecular= packedZero;
	_CurShininess= 1;
	// setup GLStates.
	static const GLfloat		one[4]= {1,1,1,1};
	static const GLfloat		zero[4]= {0,0,0,1};
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zero);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, one);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, one);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, zero);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, _CurShininess);

}



// ***************************************************************************
void			CDriverGLStates::enableBlend(uint enable)
{
	// If different from current setup, update.
	bool	enabled= (enable!=0);
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( enabled != _CurBlend )
#endif
	{
		// new state.
		_CurBlend= enabled;
		// Setup GLState.
		if(_CurBlend)
			glEnable(GL_BLEND);
		else
			glDisable(GL_BLEND);
	}
}

// ***************************************************************************
void			CDriverGLStates::enableCullFace(uint enable)
{
	// If different from current setup, update.
	bool	enabled= (enable!=0);
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( enabled != _CurCullFace )
#endif
	{
		// new state.
		_CurCullFace= enabled;
		// Setup GLState.
		if(_CurCullFace)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);
	}
}

// ***************************************************************************
void			CDriverGLStates::enableAlphaTest(uint enable)
{
	// If different from current setup, update.
	bool	enabled= (enable!=0);
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( enabled != _CurAlphaTest )
#endif
	{
		// new state.
		_CurAlphaTest= enabled;
		// Setup GLState.
		if(_CurAlphaTest)
		{
			glEnable(GL_ALPHA_TEST);
			glAlphaFunc(GL_GREATER, 0.5f);
		}
		else
		{
			glDisable(GL_ALPHA_TEST);
			glAlphaFunc(GL_ALWAYS, 0.0f);
		}
	}
}

// ***************************************************************************
void			CDriverGLStates::enableLighting(uint enable)
{
	// If different from current setup, update.
	bool	enabled= (enable!=0);
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( enabled != _CurLighting )
#endif
	{
		// new state.
		_CurLighting= enabled;
		// Setup GLState.
		if(_CurLighting)
			glEnable(GL_LIGHTING);
		else
			glDisable(GL_LIGHTING);
	}
}

// ***************************************************************************
void			CDriverGLStates::enableZWrite(uint enable)
{
	// If different from current setup, update.
	bool	enabled= (enable!=0);
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( enabled != _CurZWrite )
#endif
	{
		// new state.
		_CurZWrite= enabled;
		// Setup GLState.
		if(_CurZWrite)
			glDepthMask(GL_TRUE);
		else
			glDepthMask(GL_FALSE);
	}
}



// ***************************************************************************
void			CDriverGLStates::blendFunc(GLenum src, GLenum dst)
{
	// If different from current setup, update.
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( src!= _CurBlendSrc || dst!=_CurBlendDst )
#endif
	{
		// new state.
		_CurBlendSrc= src;
		_CurBlendDst= dst;
		// Setup GLState.
		glBlendFunc(_CurBlendSrc, _CurBlendDst);
	}
}

// ***************************************************************************
void			CDriverGLStates::depthFunc(GLenum zcomp)
{
	// If different from current setup, update.
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( zcomp != _CurDepthFunc )
#endif
	{
		// new state.
		_CurDepthFunc= zcomp;
		// Setup GLState.
		glDepthFunc(_CurDepthFunc);
	}
}



// ***************************************************************************
void			CDriverGLStates::setEmissive(uint32 packedColor, const GLfloat color[4])
{
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( packedColor!=_CurEmissive )
#endif
	{
		_CurEmissive= packedColor;
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, color);
	}
}

// ***************************************************************************
void			CDriverGLStates::setAmbient(uint32 packedColor, const GLfloat color[4])
{
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( packedColor!=_CurAmbient )
#endif
	{
		_CurAmbient= packedColor;
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, color);
	}
}

// ***************************************************************************
void			CDriverGLStates::setDiffuse(uint32 packedColor, const GLfloat color[4])
{
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( packedColor!=_CurDiffuse )
#endif
	{
		_CurDiffuse= packedColor;
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
	}
}

// ***************************************************************************
void			CDriverGLStates::setSpecular(uint32 packedColor, const GLfloat color[4])
{
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( packedColor!=_CurSpecular )
#endif
	{
		_CurSpecular= packedColor;
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, color);
	}
}

// ***************************************************************************
void			CDriverGLStates::setShininess(float shin)
{
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( shin != _CurShininess )
#endif
	{
		_CurShininess= shin;
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shin);
	}
}



} // NL3D
