/** \file driver_opengl_material.cpp
 * OpenGL driver implementation : setupMaterial
 *
 * $Id: driver_opengl_material.cpp,v 1.25 2001/05/07 14:41:57 berenguier Exp $
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


#include "driver_opengl.h"

namespace NL3D {

static void convBlend(CMaterial::TBlend blend, GLenum& glenum)
{
	switch(blend)
	{
		case CMaterial::one:		glenum=GL_ONE; break;
		case CMaterial::zero:		glenum=GL_ZERO; break;
		case CMaterial::srcalpha:	glenum=GL_SRC_ALPHA; break;
		case CMaterial::invsrcalpha:glenum=GL_ONE_MINUS_SRC_ALPHA; break;
		case CMaterial::srccolor:	glenum=GL_SRC_COLOR; break;
		case CMaterial::invsrccolor:glenum=GL_ONE_MINUS_SRC_COLOR; break;
		default: nlstop;
	}
}

static void convZFunction(CMaterial::ZFunc zfunc, GLenum& glenum)
{
	switch(zfunc)
	{
		case CMaterial::lessequal:	glenum=GL_LEQUAL; break;
		case CMaterial::less:		glenum=GL_LESS; break;
		case CMaterial::always:		glenum=GL_ALWAYS; break;
		case CMaterial::never:		glenum=GL_NEVER; break;
		case CMaterial::equal:		glenum=GL_EQUAL; break;
		case CMaterial::notequal:	glenum=GL_NOTEQUAL; break;
		case CMaterial::greater:	glenum=GL_GREATER; break;
		case CMaterial::greaterequal:	glenum=GL_GEQUAL; break;
		default: nlstop;
	}
}

static void	convColor(CRGBA col, GLfloat glcol[4])
{
	static	const float	OO255= 1.0f/255;
	glcol[0]= col.R*OO255;
	glcol[1]= col.G*OO255;
	glcol[2]= col.B*OO255;
	glcol[3]= col.A*OO255;
}


// --------------------------------------------------

bool CDriverGL::setupMaterial(CMaterial& mat)
{
	CShaderGL*	pShader;
	GLenum		glenum;
	uint32		touched=mat.getTouched();
	sint		stage;


	// 0. Setup / Bind Textures.
	//==========================
	// Must setup textures each frame. (need to test if touched).
	// Must separate texture setup and texture activation in 2 "for"...
	// because setupTexture() may modify the stage 0.
	for(stage=0 ; stage<getNbTextureStages() ; stage++)
	{
		ITexture	*text= mat.getTexture(stage);
		if (text != NULL && !setupTexture(*text))
			return(false);
	}
	// Activate the textures.
	for(stage=0 ; stage<getNbTextureStages() ; stage++)
	{
		ITexture	*text= mat.getTexture(stage);

		// activate the texture, or disable texturing if NULL.
		activateTexture(stage,text);

		// If texture not NULL, Change texture env fonction.
		//==================================================
		if(text)
		{
			CMaterial::CTexEnv	&env= mat._TexEnvs[stage];

			if(_CurrentTexEnv[stage].EnvPacked!= env.EnvPacked)
			{
				activateTexEnvMode(stage, env);
			}
			if(_CurrentTexEnv[stage].ConstantColor!= env.ConstantColor)
			{
				activateTexEnvColor(stage, env);
			}
		}

	}

	
	// 1. Retrieve/Create driver shader.
	//==================================
	if (!mat.pShader)
	{
		// insert into driver list. (so it is deleted when driver is deleted).
		ItShaderPtrList		it= _Shaders.insert(_Shaders.end());
		// create and set iterator, for future deletion.
		*it= mat.pShader= new CShaderGL(this, it);

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
		if (touched & IDRV_TOUCHED_ZFUNC)
		{
			convZFunction( mat.getZFunc(),glenum);
			pShader->ZComp= glenum;
		}
		if (touched & IDRV_TOUCHED_LIGHTING)
		{
			if(! (mat.getFlags()&IDRV_MAT_DEFMAT) )
			{
				convColor(mat.getEmissive(), pShader->Emissive);
				convColor(mat.getAmbient(), pShader->Ambient);
				convColor(mat.getDiffuse(), pShader->Diffuse);
				convColor(mat.getSpecular(), pShader->Specular);
			}
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
		//=================
		if(mat.getFlags()&IDRV_MAT_BLEND)
		{
			glEnable(GL_BLEND);
			glBlendFunc(pShader->SrcBlend, pShader->DstBlend);
		}
		else
		{
			glDisable(GL_BLEND);
		}

		// Double Sided Part.
		//=================
		if(mat.getFlags()&IDRV_MAT_DOUBLE_SIDED)
		{
			glDisable(GL_CULL_FACE);
		}
		else
		{
			glEnable(GL_CULL_FACE);
		}

		// Bind ZBuffer Part.
		//===================
		if(mat.getFlags()&IDRV_MAT_ZWRITE)
			glDepthMask(GL_TRUE);
		else
			glDepthMask(GL_FALSE);
		glDepthFunc(pShader->ZComp);


		// Color-Lighting Part.
		//=====================
		// Color unlit part.
		CRGBA	col= mat.getColor();
		glColor4ub(col.R, col.G, col.B, col.A);

		// Light Part.
		if(mat.getFlags()&IDRV_MAT_LIGHTING)
		{
			glEnable(GL_LIGHTING);
			// Temp. Yoyo, defo light.
			glEnable(GL_LIGHT0);
			if(mat.getFlags()&IDRV_MAT_DEFMAT)
			{
				GLfloat		one[4]= {1,1,1,1};
				GLfloat		zero[4]= {0,0,0,1};
				glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zero);
				glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, one);
				glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, zero);
			}
			else
			{
				glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, pShader->Emissive);
				glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, pShader->Ambient);
				glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, pShader->Diffuse);
				glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, pShader->Specular);
				glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat.getShininess());
			}
		}
		else
			glDisable(GL_LIGHTING);



		_CurrentMaterial=&mat;
	}

	return true;
}


} // NL3D
