/** \file driver_opengl_states.h
 * <File description>
 *
 * $Id: driver_opengl_states.h,v 1.5 2001/11/07 10:51:18 vizerie Exp $
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
#include "3d/vertex_buffer.h"
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
			- GL_TEXTURE_2D or GL_TEXTURE_CUBE_MAP_ARB.
		- glActiveTextureARB()
		- glClientActiveTextureARB()
		- glEnableClientState() glDisableClientState() with:
			- GL_VERTEX_ARRAY
			- GL_NORMAL_ARRAY
			- GL_VERTEX_WEIGHTING_EXT
			- GL_COLOR_ARRAY
			- GL_SECONDARY_COLOR_ARRAY_EXT
			- GL_TEXTURE_COORD_ARRAY
			- GL_VERTEX_ATTRIB_ARRAY0_NV + i.
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
	// init. Do it just after setDisplay()
	void			init(bool supportTextureCubeMap);

	/// Reset all OpenGL states of interest to default, and update caching.
	void			forceDefaults(uint nbTextureStages);

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

	/// nv texture shaders. Should be used only if this caps is present!
	void            enableNVTextureShader(bool enabled);
	// check nv texture shader consistency
	void			verifyNVTextureShaderConfig();

	/// \name Texture Mode setting.
	// @{
	enum			TTextureMode {TextureDisabled, Texture2D, TextureCubeMap, TextureModeCount};
	/// same as glActiveTextureARB(). usefull for setTextureMode.
	void			activeTextureARB(uint stage);
	/** change if needed the texture mode of the current active Texture ARB.
	 *	NB: if CubeMap extension not supported, TextureCubeMap <=> TextureDisabled.
	 */
	void			setTextureMode(TTextureMode texMode);
	// reset texture mode to the default (disabled) for the current stage. It forces the state (useful after texture shaders)
	void			resetTextureMode();	
	// @}

	/// \name Vertex Array enabling.
	/// equivalent to glEnableClientState() / glDisableClientState(). NB: Not modified by forceDefaults()
	// @{
	void			enableVertexArray(bool enable);
	void			enableNormalArray(bool enable);
	void			enableWeightArray(bool enable);
	void			enableColorArray(bool enable);
	void			enableSecondaryColorArray(bool enable);
	/// same as glClientActiveTextureARB(). usefull for enableTexCoordArray.
	void			clientActiveTextureARB(uint stage);
	/// NB: caller must call correct clientActiveTextureARB() before.
	void			enableTexCoordArray(bool enable);
	/// For vertexProgram. do not check if supported or not.
	void			enableVertexAttribArray(uint glIndex, bool enable);
	// @}

private:
	bool			_CurBlend;
	bool			_CurCullFace;
	bool			_CurAlphaTest;
	bool			_CurLighting;
	bool			_CurZWrite;
	bool			_NVTextureShader;

	GLenum			_CurBlendSrc;
	GLenum			_CurBlendDst;
	GLenum			_CurDepthFunc;

	uint32			_CurEmissive;
	uint32			_CurAmbient;
	uint32			_CurDiffuse;
	uint32			_CurSpecular;
	float			_CurShininess;

	bool			_TextureCubeMapSupported;
	uint			_CurrentActiveTextureARB;
	TTextureMode	_TextureMode[IDRV_MAT_MAXTEXTURES];

	bool			_VertexArrayEnabled;
	bool			_NormalArrayEnabled;
	bool			_WeightArrayEnabled;
	bool			_ColorArrayEnabled;
	bool			_SecondaryColorArrayEnabled;
	uint			_CurrentClientActiveTextureARB;
	bool			_TexCoordArrayEnabled[IDRV_MAT_MAXTEXTURES];
	bool			_VertexAttribArrayEnabled[CVertexBuffer::NumValue];
};


} // NL3D


#endif // NL_DRIVER_OPENGL_STATES_H

/* End of driver_opengl_states.h */
