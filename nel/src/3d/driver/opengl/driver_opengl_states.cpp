/** \file driver_opengl_states.cpp
 * <File description>
 *
 * $Id: driver_opengl_states.cpp,v 1.21 2004/04/01 19:09:39 vizerie Exp $
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

#include "stdopengl.h"

#include "driver_opengl.h"

// ***************************************************************************
// define it For Debug purpose only. Normal use is to hide this line
// #define		NL3D_GLSTATE_DISABLE_CACHE

namespace NL3D 
{

// ***************************************************************************
CDriverGLStates::CDriverGLStates()
{
	_TextureCubeMapSupported= false;
	_CurrARBVertexBuffer = 0;
}


// ***************************************************************************
void			CDriverGLStates::init(bool supportTextureCubeMap)
{
	_TextureCubeMapSupported= supportTextureCubeMap;

	// By default all arrays are disabled.
	_VertexArrayEnabled= false;
	_NormalArrayEnabled= false;
	_WeightArrayEnabled= false;
	_ColorArrayEnabled= false;	
	_SecondaryColorArrayEnabled= false;
	uint	i;
	for(i=0; i<IDRV_MAT_MAXTEXTURES; i++)
	{
		_TexCoordArrayEnabled[i]= false;
	}
	for(i=0; i<CVertexBuffer::NumValue; i++)
	{
		_VertexAttribArrayEnabled[i]= false;
	}
}


// ***************************************************************************
void			CDriverGLStates::forceDefaults(uint nbStages)
{
	// Enable / disable.
	_CurFog= false;
	_CurBlend= false;
	_CurCullFace= true;
	_CurAlphaTest= false;
	_CurLighting= false;
	_CurZWrite= true;
	// setup GLStates.
	glDisable(GL_FOG);
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_LIGHTING);
	glDepthMask(GL_TRUE);

	// Func.
	_CurBlendSrc= GL_SRC_ALPHA;
	_CurBlendDst= GL_ONE_MINUS_SRC_ALPHA;
	_CurDepthFunc= GL_LEQUAL;
	_CurAlphaTestThreshold= 0.5f;
	// setup GLStates.
	glBlendFunc(_CurBlendSrc, _CurBlendDst);
	glDepthFunc(_CurDepthFunc);
	glAlphaFunc(GL_GREATER, _CurAlphaTestThreshold);

	// Materials.
	uint32			packedOne= (CRGBA(255,255,255,255)).getPacked();
	uint32			packedZero= (CRGBA(0,0,0,255)).getPacked();
	_CurEmissive= packedZero;
	_CurAmbient= packedOne;
	_CurDiffuse= packedOne;
	_CurSpecular= packedZero;
	_CurShininess= 1;

	// Lighted vertex color
	_VertexColorLighted=false;
	glDisable(GL_COLOR_MATERIAL);

	// setup GLStates.
	static const GLfloat		one[4]= {1,1,1,1};
	static const GLfloat		zero[4]= {0,0,0,1};
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zero);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, one);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, one);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, zero);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, _CurShininess);
	

	// TexModes
	uint stage;
	for(stage=0;stage<nbStages; stage++)
	{
		// disable texturing.
		nglActiveTextureARB(GL_TEXTURE0_ARB+stage);
		glDisable(GL_TEXTURE_2D);
		if(_TextureCubeMapSupported)
			glDisable(GL_TEXTURE_CUBE_MAP_ARB);
		_TextureMode[stage]= TextureDisabled;

		// Tex gen init
		_TexGenMode[stage] = 0;
		glDisable( GL_TEXTURE_GEN_S );
		glDisable( GL_TEXTURE_GEN_T );
		glDisable( GL_TEXTURE_GEN_R );
		glDisable( GL_TEXTURE_GEN_Q );
	}

	// ActiveTexture current texture to 0.
	nglActiveTextureARB(GL_TEXTURE0_ARB);
	_CurrentActiveTextureARB= 0;
	nglClientActiveTextureARB(GL_TEXTURE0_ARB);
	_CurrentClientActiveTextureARB= 0;

	// Depth range
	_CurZRangeDelta = 0;
	glDepthRange (0, 1);
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
		}
		else
		{
			glDisable(GL_ALPHA_TEST);
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
void			CDriverGLStates::alphaFunc(float threshold)
{
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if(threshold != _CurAlphaTestThreshold)
#endif
	{
		// new state
		_CurAlphaTestThreshold= threshold;
		// setup function.
		glAlphaFunc(GL_GREATER, _CurAlphaTestThreshold);
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


// ***************************************************************************
static void	convColor(CRGBA col, GLfloat glcol[4])
{
	static	const float	OO255= 1.0f/255;
	glcol[0]= col.R*OO255;
	glcol[1]= col.G*OO255;
	glcol[2]= col.B*OO255;
	glcol[3]= col.A*OO255;
}

// ***************************************************************************
void			CDriverGLStates::setVertexColorLighted(bool enable)
{
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( enable != _VertexColorLighted)
#endif
	{
		_VertexColorLighted= enable;

		if (_VertexColorLighted)
		{
			glEnable (GL_COLOR_MATERIAL);
			glColorMaterial (GL_FRONT_AND_BACK, GL_DIFFUSE);
		}
		else
		{
			glDisable (GL_COLOR_MATERIAL);
			// Since we leave glColorMaterial mode, GL diffuse is now scracth. reset him to current value.
			CRGBA	diffCol;
			diffCol.R= (uint8)((_CurDiffuse >> 24) & 255);
			diffCol.G= (uint8)((_CurDiffuse >> 16) & 255);
			diffCol.B= (uint8)((_CurDiffuse >>  8) & 255);
			diffCol.A= (uint8)((_CurDiffuse      ) & 255);
			GLfloat	glColor[4];
			convColor(diffCol, glColor);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, glColor);
		}
	}
}

// ***************************************************************************
void		CDriverGLStates::setDepthRange (float zDelta)
{
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (zDelta != _CurZRangeDelta)
#endif
	{		
		_CurZRangeDelta = zDelta;

		// Setup the range
		glDepthRange (zDelta, 1+zDelta);
	}
}

// ***************************************************************************
void		CDriverGLStates::setTexGenMode (uint stage, GLint mode)
{
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (mode != _TexGenMode[stage])
#endif
	{		
		_TexGenMode[stage] = mode;

		if(mode==0)
		{
			glDisable( GL_TEXTURE_GEN_S );
			glDisable( GL_TEXTURE_GEN_T );
			glDisable( GL_TEXTURE_GEN_R );
			glDisable( GL_TEXTURE_GEN_Q );
		}
		else
		{
			glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, mode);
			glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, mode);
			glTexGeni( GL_R, GL_TEXTURE_GEN_MODE, mode);
			/* Object or Eye Space ? => enable W generation. VERY IMPORTANT because 
				was a bug with VegetableRender and ShadowRender:
					- Vegetable use the TexCoord1.w in his VertexProgram
					- Shadow Render don't use any TexCoord in VB (since projected)
					=> TexCoord1.w dirty!!
			*/
			if(mode==GL_OBJECT_LINEAR || mode==GL_EYE_LINEAR)
			{
				glTexGeni( GL_Q, GL_TEXTURE_GEN_MODE, mode);
			}
			else
			{
				glTexGeni( GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
			}
			// Enable All.
			glEnable( GL_TEXTURE_GEN_S );
			glEnable( GL_TEXTURE_GEN_T );
			glEnable( GL_TEXTURE_GEN_R );
			glEnable( GL_TEXTURE_GEN_Q );
		}
	}
}




// ***************************************************************************
void			CDriverGLStates::resetTextureMode()
{
	glDisable(GL_TEXTURE_2D);
	if (_TextureCubeMapSupported)
	{
		glDisable(GL_TEXTURE_CUBE_MAP_ARB);
	}
	_TextureMode[_CurrentActiveTextureARB]= TextureDisabled;
}


// ***************************************************************************
void			CDriverGLStates::setTextureMode(TTextureMode texMode)
{	
	TTextureMode	oldTexMode = _TextureMode[_CurrentActiveTextureARB];
	if(oldTexMode != texMode)
	{
		// Disable first old mode.
		if(oldTexMode == Texture2D)
			glDisable(GL_TEXTURE_2D);
		else if(oldTexMode == TextureCubeMap)
		{
			if(_TextureCubeMapSupported)
				glDisable(GL_TEXTURE_CUBE_MAP_ARB);
			else
				glDisable(GL_TEXTURE_2D);
		}

		// Enable new mode.
		if(texMode == Texture2D)
			glEnable(GL_TEXTURE_2D);
		else if(texMode == TextureCubeMap)
		{
			if(_TextureCubeMapSupported)
				glEnable(GL_TEXTURE_CUBE_MAP_ARB);
			else
				glDisable(GL_TEXTURE_2D);
		}

		// new mode.
		_TextureMode[_CurrentActiveTextureARB]= texMode;
	}
}


// ***************************************************************************
void			CDriverGLStates::activeTextureARB(uint stage)
{
	if( _CurrentActiveTextureARB != stage )
	{
		nglActiveTextureARB(GL_TEXTURE0_ARB+stage);
		_CurrentActiveTextureARB= stage;
	}
}

// ***************************************************************************
void			CDriverGLStates::forceActiveTextureARB(uint stage)
{	
	nglActiveTextureARB(GL_TEXTURE0_ARB+stage);
	_CurrentActiveTextureARB= stage;	
}


// ***************************************************************************
void			CDriverGLStates::enableVertexArray(bool enable)
{
	if(_VertexArrayEnabled != enable)
	{
		if(enable)
			glEnableClientState(GL_VERTEX_ARRAY);
		else
			glDisableClientState(GL_VERTEX_ARRAY);
		_VertexArrayEnabled= enable;
	}
}
// ***************************************************************************
void			CDriverGLStates::enableNormalArray(bool enable)
{
	if(_NormalArrayEnabled != enable)
	{
		if(enable)
			glEnableClientState(GL_NORMAL_ARRAY);
		else
			glDisableClientState(GL_NORMAL_ARRAY);
		_NormalArrayEnabled= enable;
	}
}
// ***************************************************************************
void			CDriverGLStates::enableWeightArray(bool enable)
{
	if(_WeightArrayEnabled != enable)
	{
		if(enable)
			glEnableClientState(GL_VERTEX_WEIGHTING_EXT);
		else
			glDisableClientState(GL_VERTEX_WEIGHTING_EXT);
		_WeightArrayEnabled= enable;
	}
}
// ***************************************************************************
void			CDriverGLStates::enableColorArray(bool enable)
{
	if(_ColorArrayEnabled != enable)
	{
		if(enable)
			glEnableClientState(GL_COLOR_ARRAY);
		else
			glDisableClientState(GL_COLOR_ARRAY);
		_ColorArrayEnabled= enable;
	}
}


// ***************************************************************************
void			CDriverGLStates::enableSecondaryColorArray(bool enable)
{
	if(_SecondaryColorArrayEnabled != enable)
	{
		if(enable)
			glEnableClientState(GL_SECONDARY_COLOR_ARRAY_EXT);
		else
			glDisableClientState(GL_SECONDARY_COLOR_ARRAY_EXT);
		_SecondaryColorArrayEnabled= enable;
		// If disable
		if(!enable)
		{
			// GeForceFx Bug: Must reset Secondary color to 0 (if comes from a VP), else bugs
			nglSecondaryColor3ubEXT(0,0,0);
		}
	}
}

// ***************************************************************************
void			CDriverGLStates::clientActiveTextureARB(uint stage)
{
	if( _CurrentClientActiveTextureARB != stage )
	{
		nglClientActiveTextureARB(GL_TEXTURE0_ARB+stage);
		_CurrentClientActiveTextureARB= stage;
	}
}

// ***************************************************************************
void			CDriverGLStates::enableTexCoordArray(bool enable)
{
	if(_TexCoordArrayEnabled[_CurrentClientActiveTextureARB] != enable)
	{
		if(enable)
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		else
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		_TexCoordArrayEnabled[_CurrentClientActiveTextureARB]= enable;
	}
}

// ***************************************************************************
void			CDriverGLStates::enableVertexAttribArray(uint glIndex, bool enable)
{
	if(_VertexAttribArrayEnabled[glIndex] != enable)
	{
		if(enable)
			glEnableClientState(glIndex+GL_VERTEX_ATTRIB_ARRAY0_NV);
		else
			glDisableClientState(glIndex+GL_VERTEX_ATTRIB_ARRAY0_NV);
		_VertexAttribArrayEnabled[glIndex]= enable;
	}
}

// ***************************************************************************
void CDriverGLStates::enableVertexAttribArrayARB(uint glIndex,bool enable)
{
	if(_VertexAttribArrayEnabled[glIndex] != enable)
	{
		if(enable)
			nglEnableVertexAttribArrayARB(glIndex);
		else
			nglDisableVertexAttribArrayARB(glIndex);			
		_VertexAttribArrayEnabled[glIndex]= enable;
	}
}



// ***************************************************************************
void CDriverGLStates::enableVertexAttribArrayForEXTVertexShader(uint glIndex, bool enable, uint *variants)
{
	if(_VertexAttribArrayEnabled[glIndex] != enable)
	{		
		switch(glIndex)
		{
			case 0: // position 
				enableVertexArray(enable);
			break;
			case 1: // skin weight
				if(enable)
					nglEnableVariantClientStateEXT(variants[CDriverGL::EVSSkinWeightVariant]);
				else
					nglDisableVariantClientStateEXT(variants[CDriverGL::EVSSkinWeightVariant]);	
			break;
			case 2: // normal
				enableNormalArray(enable);
			break;
			case 3: // color
				enableColorArray(enable);
			break;
			case 4: // secondary color
				if(enable)
					nglEnableVariantClientStateEXT(variants[CDriverGL::EVSSecondaryColorVariant]);
				else
					nglDisableVariantClientStateEXT(variants[CDriverGL::EVSSecondaryColorVariant]);	
			break;
			case 5: // fog coordinate
				if(enable)
					nglEnableVariantClientStateEXT(variants[CDriverGL::EVSFogCoordsVariant]);
				else
					nglDisableVariantClientStateEXT(variants[CDriverGL::EVSFogCoordsVariant]);
			break;
			case 6: // palette skin
				if(enable)
					nglEnableVariantClientStateEXT(variants[CDriverGL::EVSPaletteSkinVariant]);
				else
					nglDisableVariantClientStateEXT(variants[CDriverGL::EVSPaletteSkinVariant]);
			break;
			case 7: // empty
				nlstop
			break;
			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
				clientActiveTextureARB(glIndex - 8);
				enableTexCoordArray(enable);
			break;
			default:
				nlstop; // invalid value
			break;
		}		
		_VertexAttribArrayEnabled[glIndex]= enable;
	}	
}



// ***************************************************************************
void			CDriverGLStates::enableFog(uint enable)
{
	// If different from current setup, update.
	bool	enabled= (enable!=0);
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( enabled != _CurFog )
#endif
	{
		// new state.
		_CurFog= enabled;
		// Setup GLState.
		if(_CurFog)
			glEnable(GL_FOG);
		else
			glDisable(GL_FOG);
	}
}

// ***************************************************************************
void CDriverGLStates::forceBindARBVertexBuffer(uint objectID)
{
	nglBindBufferARB(GL_ARRAY_BUFFER_ARB, objectID);
	_CurrARBVertexBuffer = objectID;
}


// ***************************************************************************
void CDriverGLStates::bindARBVertexBuffer(uint objectID)
{
	if (objectID != _CurrARBVertexBuffer)
	{
		forceBindARBVertexBuffer(objectID);
	} 
}


} // NL3D
