/** \file driver_opengl_states.h
 * <File description>
 *
 * $Id: driver_opengl_states.h,v 1.1 2001/09/20 16:43:10 berenguier Exp $
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

#ifndef NL_DRIVER_OPENGL_STATES_H
#define NL_DRIVER_OPENGL_STATES_H

#include "nel/misc/types_nl.h"
#include <GL/gl.h>


namespace NL3D 
{


// ***************************************************************************
/**
 * Class for optimizing calls to openGL states, by caching old ones.
 *	All following call with OpenGL must be done with only one instance of this class:
		- glEnable() glDisable() with:
			- GL_BLEND
			- GL_CULL_FACE
			- GL_ALPHA_TEST
			- GL_LIGHTING
		- glDepthMask()
		- glAlphaFunc()
		- glBlendFunc()
		- glDepthFunc()
		- glMaterialf() and glMaterialfv() for:
			- GL_EMISSION
			- GL_AMBIENT
			- GL_DIFFUSE
			- GL_SPECULAR
			- GL_SHININESS
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CDriverGLStates
{
public:

	/// Constructor. no-op.
	CDriverGLStates();

	/// Reset all OpenGL states of interest to default, and update caching.
	void			forceDefaults();

	/// \name enable if !0
	// @{
	void			enableBlend(uint enable);
	void			enableCullFace(uint enable);
	/// enable and set good AlphaFunc.
	void			enableAlphaTest(uint enable);
	void			enableLighting(uint enable);
	void			enableZWrite(uint enable);
	// @}

	/// glBlendFunc.
	void			blendFunc(GLenum src, GLenum dst);
	/// glDepthFunc.
	void			depthFunc(GLenum zcomp);

	/// \name Material setting.
	/// Each f() get an uint32 for fast comparison, and OpenGL colors.
	// @{
	void			setEmissive(uint32 packedColor, const GLfloat color[4]);
	void			setAmbient(uint32 packedColor, const GLfloat color[4]);
	void			setDiffuse(uint32 packedColor, const GLfloat color[4]);
	void			setSpecular(uint32 packedColor, const GLfloat color[4]);
	void			setShininess(float shin);
	// @}


private:
	bool			_CurBlend;
	bool			_CurCullFace;
	bool			_CurAlphaTest;
	bool			_CurLighting;
	bool			_CurZWrite;

	GLenum			_CurBlendSrc;
	GLenum			_CurBlendDst;
	GLenum			_CurDepthFunc;

	uint32			_CurEmissive;
	uint32			_CurAmbient;
	uint32			_CurDiffuse;
	uint32			_CurSpecular;
	float			_CurShininess;

};


} // NL3D


#endif // NL_DRIVER_OPENGL_STATES_H

/* End of driver_opengl_states.h */
