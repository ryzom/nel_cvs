/** \file driver_opengl_material.cpp
 * OpenGL driver implementation : setupMaterial
 *
 * $Id: driver_opengl_material.cpp,v 1.27 2001/05/31 10:05:09 berenguier Exp $
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
	// Here, for Lightmap materials, setup the lightmaps.
	if(mat.getShader()==CMaterial::LightMap)
	{
		for(stage=0 ; stage<(sint)mat._LightMaps.size() ; stage++)
		{
			ITexture	*text= mat._LightMaps[stage].Texture;
			if (text != NULL && !setupTexture(*text))
				return(false);
		}
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
		// Warning! this implies modification in setupPass()/endMultiPass() for multiPass materials.

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


// ***************************************************************************
sint			CDriverGL::beginMultiPass(const CMaterial &mat)
{
	// Depending on material type and hardware, return number of pass required to draw this material.
	switch(mat.getShader())
	{
	case CMaterial::LightMap: 
		return  beginLightMapMultiPass(mat);

	// All others materials require just 1 pass.
	default: return 1;
	}
}
// ***************************************************************************
void			CDriverGL::setupPass(const CMaterial &mat, uint pass)
{
	switch(mat.getShader())
	{
	case CMaterial::LightMap: 
		setupLightMapPass(mat, pass);

	// All others materials do not require multi pass.
	default: return;
	}
}


// ***************************************************************************
void			CDriverGL::endMultiPass(const CMaterial &mat)
{
	switch(mat.getShader())
	{
	case CMaterial::LightMap: 
		endLightMapMultiPass(mat);

	// All others materials do not require multi pass.
	default: return;
	}

}


// ***************************************************************************
void			CDriverGL::computeLightMapInfos(const CMaterial &mat, uint &nLMaps, uint &nLMapPerPass, uint &nPass) const
{
	nLMaps= mat._LightMaps.size();
	nLMapPerPass= getNbTextureStages()-1;
	if(!_Extensions.NVTextureEnvCombine4)
		nLMapPerPass= 1;
	nPass= (nLMaps+nLMapPerPass-1)/(nLMapPerPass);
}


// ***************************************************************************
sint			CDriverGL::beginLightMapMultiPass(const CMaterial &mat)
{
	// One texture stage hardware not supported.
	if(getNbTextureStages()<2)
		return 1;
	uint	nLMaps, nLMapPerPass, nPass;
	computeLightMapInfos(mat, nLMaps, nLMapPerPass, nPass);

	// Too be sure, disable vertex coloring / lightmap.
	glDisable(GL_LIGHTING);
	// reset VertexColor array if necessary.
	if (_LastVB->getVertexFormat() & IDRV_VF_COLOR)
		glDisableClientState(GL_COLOR_ARRAY);


	// Manage too if no lightmaps.
	nPass= std::max(nPass, (uint)1);
	return	nPass;
}
// ***************************************************************************
void			CDriverGL::setupLightMapPass(const CMaterial &mat, uint pass)
{
	// One texture stage hardware not supported.
	if(getNbTextureStages()<2)
		return;
	// compute Lightmaps infos.
	//=========================
	uint	nLMaps, nLMapPerPass, nPass;
	computeLightMapInfos(mat, nLMaps, nLMapPerPass, nPass);

	// No lightmap??, just setup "replace texture" for stage 0.
	if(nPass==0)
	{
		ITexture	*text= mat.getTexture(0);
		activateTexture(0,text);

		CMaterial::CTexEnv	env;
		env.Env.OpRGB= CMaterial::Replace;
		env.Env.SrcArg0RGB= CMaterial::Texture;
		env.Env.OpArg0RGB= CMaterial::SrcColor;
		if(_CurrentTexEnv[0].EnvPacked!= env.EnvPacked)
			activateTexEnvMode(0, env);

		return;
	}

	nlassert(pass<nPass);


	// setup Texture Pass.
	//=========================
	uint	lmapId;
	uint	nstages;
	lmapId= pass*nLMapPerPass;
	// N lightmaps for this pass, plus the texture.
	nstages= std::min(nLMapPerPass, nLMaps-lmapId) + 1;
	// setup all stages.
	for(uint stage= 0; stage<(uint)getNbTextureStages(); stage++, lmapId++)
	{
		if(stage<nstages-1)
		{
			// setup lightMap.
			ITexture	*text= mat._LightMaps[lmapId].Texture;
			activateTexture(stage,text);

			// If texture not NULL, Change texture env fonction.
			//==================================================
			if(text)
			{
				CMaterial::CTexEnv	env;
				uint8	lmapFactor= mat._LightMaps[lmapId].Factor;

				// NB, !_Extensions.NVTextureEnvCombine4, nstages==2, so here always stage==0.
				if(stage==0)
				{
					// do not use consant color to blend lightmap, but incoming diffuse color, for stage0 only.
					// (NB: lighting and vertexcolorArray are disabled here)
					glColor4ub(lmapFactor,lmapFactor,lmapFactor, 255);

					// Leave stage as default env (Modulate with previous)
					if(_CurrentTexEnv[stage].EnvPacked!= env.EnvPacked)
						activateTexEnvMode(stage, env);
					// NB: no need to disable modulate2x here, because always setuped at the last stage
					// (but for the last pass).
				}
				else
				{
					// Here, we are sure that texEnvCombine4 is OK.
					nlassert(_Extensions.NVTextureEnvCombine4);

					// setup constant color with Lightmap factor.
					env.ConstantColor.set(lmapFactor, lmapFactor, lmapFactor, 255);
					if(_CurrentTexEnv[stage].ConstantColor!= env.ConstantColor)
						activateTexEnvColor(stage, env);

					// setup TexEnvCombine4 (ignore alpha part).
					// What we want to setup is  Texture*Constant + Previous*1.
					glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE4_NV);

					// Operator.
					glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD );
					glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_ADD );
					// Arg0.
					glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_TEXTURE );
					glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
					// Arg1.
					glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_CONSTANT_EXT );
					glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR);
					// Arg2.
					glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, GL_PREVIOUS_EXT );
					glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, GL_SRC_COLOR);
					// Arg3.
					glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_RGB_NV, GL_ZERO);
					glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_RGB_NV, GL_ONE_MINUS_SRC_COLOR);


					// NB: no need to disable modulate2x here, because always setuped at the last stage
					// (but for the last pass).
				}


				// setup UV, with UV1.
				setupUVPtr(stage, *_LastVB, 1);
			}
		}
		else if(stage<nstages)
		{
			// optim: do this only for first pass, and last pass (last pass only if stage!=nLMapPerPass)
			if(pass==0 || (pass==nPass-1 && stage!=nLMapPerPass))
			{
				// setup texture in final stage.
				ITexture	*text= mat.getTexture(0);
				activateTexture(stage,text);

				// activate the texture at last stage, with modulate2x.
				// force setup default env (modulate). (to disable possible COMBINE4_NV setup).
				CMaterial::CTexEnv	env;
				activateTexEnvMode(stage, env);
				// special modulate2x.
				glActiveTextureARB(GL_TEXTURE0_ARB+stage);
				glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, 2);


				// setup UV, with UV0.
				setupUVPtr(stage, *_LastVB, 0);
			}
		}
		else
		{
			// else all other stages are disabled.
			activateTexture(stage,NULL);
		}
	}


	// setup blend / lighting.
	//=========================
	if(pass==0)
	{
		// no transparency for first pass.
		glDisable(GL_BLEND);
	}
	else if(pass==1)
	{
		// setup an Additive transparency (only for pass 1, will be kept for successives pass).
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
	}

}
// ***************************************************************************
void			CDriverGL::endLightMapMultiPass(const CMaterial &mat)
{
	// special for all stage, clean up envstages + blend mode.
	for(uint stage= 0; stage<(uint)getNbTextureStages(); stage++)
	{
		// if multiFactor is OK, force reset a normal modulate env.
		if(_Extensions.NVTextureEnvCombine4)
		{
			CMaterial::CTexEnv	env;
			activateTexEnvMode(stage, env);
		}
		// special: disable  modulate2x.
		glActiveTextureARB(GL_TEXTURE0_ARB+stage);
		glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, 1);
	}

	// special for all stage, normal UV behavior.
	for(sint i=0; i<getNbTextureStages(); i++)
	{
		// normal behavior: each texture has its own UV.
		setupUVPtr(i, *_LastVB, i);
	}

	// pop VertexColor array if necessary.
	if (_LastVB->getVertexFormat() & IDRV_VF_COLOR)
		glEnableClientState(GL_COLOR_ARRAY);


	// NB: for now, nothing to do with blending/lighting, since always setuped in activeMaterial().
}


} // NL3D
