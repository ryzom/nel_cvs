/** \file driver_direct3d_material.cpp
 * Direct 3d driver implementation
 *
 * $Id: driver_direct3d_material.cpp,v 1.6 2004/05/18 16:34:27 berenguier Exp $
 *
 * \todo manage better the init/release system (if a throw occurs in the init, we must release correctly the driver)
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

#include "stddirect3d.h"

#include "3d/vertex_buffer.h"
#include "3d/light.h"
#include "3d/index_buffer.h"
#include "nel/misc/rect.h"
#include "nel/misc/di_event_emitter.h"
#include "nel/misc/mouse_device.h"
#include "nel/3d/viewport.h"
#include "nel/3d/scissor.h"
#include "nel/3d/u_driver.h"

#include "driver_direct3d.h"

using namespace std;
using namespace NLMISC;

namespace NL3D 
{

// ***************************************************************************

const D3DBLEND RemapBlendTypeNeL2D3D[CMaterial::blendCount]=
{
	D3DBLEND_ONE,			// one
	D3DBLEND_ZERO,			// zero
	D3DBLEND_SRCALPHA,		// srcalpha
	D3DBLEND_INVSRCALPHA,	// invsrcalpha
	D3DBLEND_SRCCOLOR,		// srccolor
	D3DBLEND_INVSRCCOLOR,	// invsrccolor
	D3DBLEND_ONE,			// blendConstantColor
	D3DBLEND_ONE,			// blendConstantInvColor
	D3DBLEND_ONE,			// blendConstantAlpha
	D3DBLEND_ONE,			// blendConstantInvAlpha
};

// ***************************************************************************

const D3DCMPFUNC RemapZFuncTypeNeL2D3D[CMaterial::zfuncCount]=
{
	D3DCMP_ALWAYS,		// always
	D3DCMP_NEVER,		// never
	D3DCMP_EQUAL,		// equal
	D3DCMP_NOTEQUAL,	// notequal
	D3DCMP_LESS,		// less
	D3DCMP_LESSEQUAL,	// lessequal
	D3DCMP_GREATER,		// greater
	D3DCMP_GREATEREQUAL,// greaterequal
};

// ***************************************************************************

// For stage 0 only
const D3DTEXTUREOP RemapTexOpType0NeL2D3D[CMaterial::TexOperatorCount]=
{
	D3DTOP_SELECTARG1,			// Replace
	D3DTOP_MODULATE,			// Modulate
	D3DTOP_ADD,					// Add
	D3DTOP_ADDSIGNED,			// AddSigned
	D3DTOP_BLENDTEXTUREALPHA,	// InterpolateTexture
	D3DTOP_BLENDDIFFUSEALPHA,	// InterpolatePrevious
	D3DTOP_BLENDDIFFUSEALPHA,	// InterpolateDiffuse
	D3DTOP_LERP,				// InterpolateConstant
	D3DTOP_BUMPENVMAP,			// EMBM
};

// ***************************************************************************

const D3DTEXTUREOP RemapTexOpTypeNeL2D3D[CMaterial::TexOperatorCount]=
{
	D3DTOP_SELECTARG1,			// Replace
	D3DTOP_MODULATE,			// Modulate
	D3DTOP_ADD,					// Add
	D3DTOP_ADDSIGNED,			// AddSigned
	D3DTOP_BLENDTEXTUREALPHA,	// InterpolateTexture
	D3DTOP_BLENDCURRENTALPHA,	// InterpolatePrevious
	D3DTOP_BLENDDIFFUSEALPHA,	// InterpolateDiffuse
	D3DTOP_LERP,				// InterpolateConstant
	D3DTOP_BUMPENVMAP,			// EMBM
	D3DTOP_MULTIPLYADD
};

// ***************************************************************************

// For stage 0 only
const DWORD RemapTexArg0NeL2D3D[CMaterial::TexSourceCount]=
{
	D3DTA_TEXTURE,	// Texture
	D3DTA_DIFFUSE,	// Previous
	D3DTA_DIFFUSE,	// Diffuse
	D3DTA_TFACTOR, // todo hulud constant color D3DTA_CONSTANT,	// Constant
};

// ***************************************************************************

const DWORD RemapTexArgNeL2D3D[CMaterial::TexSourceCount]=
{
	D3DTA_TEXTURE,	// Texture
	D3DTA_CURRENT,	// Previous
	D3DTA_DIFFUSE,	// Diffuse
	D3DTA_TFACTOR, // todo hulud constant color D3DTA_CONSTANT,	// Constant
};

// ***************************************************************************

const DWORD RemapTexOpArgTypeNeL2D3D[CMaterial::TexOperandCount]=
{
	0,										// SrcColor
	D3DTA_COMPLEMENT,						// InvSrcColor
	D3DTA_ALPHAREPLICATE,					// SrcAlpha
	D3DTA_ALPHAREPLICATE|D3DTA_COMPLEMENT,	// InvSrcAlpha
};

// ***************************************************************************

const DWORD RemapTexArg0TypeNeL2D3D[CMaterial::TexOperatorCount]=
{
	D3DTA_TFACTOR, // todo hulud constant color D3DTA_CONSTANT,							// Replace not used
	D3DTA_TFACTOR, // todo hulud constant color D3DTA_CONSTANT,							// Modulate not used
	D3DTA_TFACTOR, // todo hulud constant color D3DTA_CONSTANT,							// Add not used
	D3DTA_TFACTOR, // todo hulud constant color D3DTA_CONSTANT,							// AddSigned not used
	D3DTA_TFACTOR, // todo hulud constant color D3DTA_CONSTANT,							// InterpolateTexture not used
	D3DTA_TFACTOR, // todo hulud constant color D3DTA_CONSTANT,							// InterpolatePrevious not used
	D3DTA_TFACTOR, // todo hulud constant color D3DTA_CONSTANT,							// InterpolateDiffuse not used
	D3DTA_TFACTOR|D3DTA_ALPHAREPLICATE, // todo hulud constant color D3DTA_CONSTANT|D3DTA_ALPHAREPLICATE,	// InterpolateConstant
	D3DTA_TFACTOR, // todo hulud constant color D3DTA_CONSTANT,							// EMBM not used
	D3DTOP_MULTIPLYADD
};

// ***************************************************************************

const DWORD RemapTexGenTypeNeL2D3D[CMaterial::numTexCoordGenMode]=
{
	D3DTSS_TCI_SPHEREMAP,					// TexCoordGenReflect
	D3DTSS_TCI_CAMERASPACEPOSITION,			// TexCoordGenObjectSpace, not supported
	D3DTSS_TCI_CAMERASPACEPOSITION,			// TexCoordGenEyeSpace
};

// ***************************************************************************

const DWORD RemapTexGenCubeTypeNeL2D3D[CMaterial::numTexCoordGenMode]=
{
	D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR,	// TexCoordGenReflect
	D3DTSS_TCI_CAMERASPACEPOSITION,			// TexCoordGenObjectSpace, not supported
	D3DTSS_TCI_CAMERASPACEPOSITION,			// TexCoordGenEyeSpace
};

// ***************************************************************************

void CMaterialDrvInfosD3D::buildTexEnv (uint stage, const CMaterial::CTexEnv &env, bool textured)
{
	if (textured)
	{
		// The source operator pointer
		const DWORD *srcOp = (stage==0)?RemapTexArg0NeL2D3D:RemapTexArgNeL2D3D;

		ColorOp[stage] = ((stage==0)?RemapTexOpType0NeL2D3D:RemapTexOpTypeNeL2D3D)[env.Env.OpRGB];
		// Only used for InterpolateConstant
		ColorArg0[stage] = RemapTexArg0TypeNeL2D3D[env.Env.OpRGB];
		if (env.Env.OpRGB == CMaterial::Mad)
		{
			ColorArg0[stage] |= RemapTexOpArgTypeNeL2D3D[env.Env.OpArg2RGB];
		}
		ColorArg1[stage] = srcOp[env.Env.SrcArg0RGB];
		ColorArg1[stage] |= RemapTexOpArgTypeNeL2D3D[env.Env.OpArg0RGB];
		ColorArg2[stage] = srcOp[env.Env.SrcArg1RGB];
		ColorArg2[stage] |= RemapTexOpArgTypeNeL2D3D[env.Env.OpArg1RGB];
		AlphaOp[stage] = ((stage==0)?RemapTexOpType0NeL2D3D:RemapTexOpTypeNeL2D3D)[env.Env.OpAlpha];
		// Only used for InterpolateConstant
		AlphaArg0[stage] = RemapTexArg0TypeNeL2D3D[env.Env.OpAlpha];
		if (env.Env.OpAlpha == CMaterial::Mad)
		{
			AlphaArg0[stage] |= RemapTexOpArgTypeNeL2D3D[env.Env.OpArg2Alpha];	
		}
		AlphaArg1[stage] = srcOp[env.Env.SrcArg0Alpha];
		AlphaArg1[stage] |= RemapTexOpArgTypeNeL2D3D[env.Env.OpArg0Alpha];
		AlphaArg2[stage] = srcOp[env.Env.SrcArg1Alpha];
		AlphaArg2[stage] |= RemapTexOpArgTypeNeL2D3D[env.Env.OpArg1Alpha];
		ConstantColor[stage] = NL_D3DCOLOR_RGBA(env.ConstantColor);				
	}
	else
	{
		// The stage is disabled, active only the lighting
		ColorOp[stage] = D3DTOP_SELECTARG1;
		DWORD opSrc = D3DTA_DIFFUSE;
		ColorArg0[stage] = opSrc;
		ColorArg1[stage] = opSrc;
		ColorArg2[stage] = opSrc;
		AlphaOp[stage] = D3DTOP_SELECTARG1;
		AlphaArg0[stage] = opSrc;
		AlphaArg1[stage] = opSrc;
		AlphaArg2[stage] = opSrc;
		ConstantColor[stage] = NL_D3DCOLOR_RGBA(CRGBA::White);
	}
}

// ***************************************************************************

bool CDriverD3D::setupMaterial (CMaterial& mat)
{
	CMaterialDrvInfosD3D*	pShader;

	// Stats
	_NbSetupMaterialCall++;

	// Max texture
	const uint maxTexture = inlGetNumTextStages();

	// Update material
	uint32 touched=mat.getTouched();

	// No shader ?
	if (!mat._MatDrvInfo)
	{
		// Insert into driver list. (so it is deleted when driver is deleted).
		ItMatDrvInfoPtrList		it= _MatDrvInfos.insert(_MatDrvInfos.end());

		*it = mat._MatDrvInfo = new CMaterialDrvInfosD3D(this, it);

		// Must create all OpenGL shader states.
		touched = IDRV_TOUCHED_ALL;
	}
	pShader = static_cast<CMaterialDrvInfosD3D*>((IMaterialDrvInfos*)(mat._MatDrvInfo));

	// Now we can get the supported shader from the cache.
	CMaterial::TShader matShader = mat.getShader();

	// Something to setup ?
	if (touched)
	{
		/* Exception: if only Textures are modified in the material, no need to "Bind OpenGL States", or even to test
			for change, because textures are activated alone, see below.
			No problem with delete/new problem (see below), because in this case, IDRV_TOUCHED_ALL is set (see above).
		*/
		// If any flag is set (but a flag of texture)
		if( touched & (~_MaterialAllTextureTouchedFlag) )
		{
			// Convert Material to driver shader.
			if (touched & IDRV_TOUCHED_BLENDFUNC)
			{
				pShader->SrcBlend = RemapBlendTypeNeL2D3D[mat.getSrcBlend()];
				pShader->DstBlend = RemapBlendTypeNeL2D3D[mat.getDstBlend()];
			}
			if (touched & IDRV_TOUCHED_ZFUNC)
			{
				pShader->ZComp = RemapZFuncTypeNeL2D3D[mat.getZFunc()];
			}
			if (touched & IDRV_TOUCHED_LIGHTING)
			{
				// Lighted material ?
				if (mat.isLighted())
				{
					// Setup the color
					NL_D3DCOLORVALUE_RGBA(pShader->Material.Diffuse, mat.getDiffuse());
					NL_D3DCOLORVALUE_RGBA(pShader->Material.Ambient, mat.getAmbient());
					NL_D3DCOLORVALUE_RGBA(pShader->Material.Emissive, mat.getEmissive());

					// Specular
					CRGBA spec = mat.getSpecular();
					if (spec != CRGBA::Black)
					{
						NL_D3DCOLORVALUE_RGBA(pShader->Material.Specular, spec);
						pShader->SpecularEnabled = TRUE;
						pShader->Material.Power = mat.getShininess();
					}
					else
						setRenderState (D3DRS_SPECULARENABLE, FALSE);
				}
				else
				{
					// No specular
					pShader->SpecularEnabled = FALSE;
				}
			}
			if (touched & IDRV_TOUCHED_COLOR)
			{
				// Setup the color
				pShader->UnlightedColor = NL_D3DCOLOR_RGBA(mat.getColor());
			}
			if (touched & IDRV_TOUCHED_ALPHA_TEST_THRE)
			{
				float alphaRef = (float)floor(mat.getAlphaTestThreshold() * 255.f + 0.5f);
				clamp (alphaRef, 0.f, 255.f);
				pShader->AlphaRef = (DWORD)alphaRef;
			}
			if (touched & IDRV_TOUCHED_SHADER)
			{
				// todo hulud d3d material shaders
				// Get shader. Fallback to other shader if not supported.
				// pShader->SupportedShader= getSupportedShader(mat.getShader());
			}
			if (touched & IDRV_TOUCHED_TEXENV)
			{
				// Build the tex env cache.
				// Do not do it for Lightmap and per pixel lighting , because done in multipass in a very special fashion.
				// This avoid the useless multiple change of texture states per lightmapped object.
				// Don't do it also for Specular because the EnvFunction and the TexGen may be special.
				if(matShader == CMaterial::Normal)
				{
					uint stage;
					for(stage=0 ; stage<(uint)maxTexture; stage++)
					{
						// Build the tex env
						pShader->buildTexEnv (stage, mat._TexEnvs[stage], mat.getTexture(stage) != NULL);
					}
				}
			}
			if (touched & (IDRV_TOUCHED_TEXGEN|IDRV_TOUCHED_ALLTEX))
			{
				uint stage;
				for(stage=0 ; stage<(uint)maxTexture; stage++)
				{
					pShader->ActivateSpecularWorldTexMT[stage] = false;
					pShader->ActivateInvViewModelTexMT[stage] = false;

					// Build the tex env
					ITexture	*text= mat.getTexture(stage);
					if (text && text->isTextureCube())
					{
						if (mat.getTexCoordGen(stage))
						{
							// Texture cube
							pShader->TexGen[stage] = RemapTexGenCubeTypeNeL2D3D[mat.getTexCoordGenMode (stage)];

							// Texture matrix
							pShader->ActivateSpecularWorldTexMT[stage] = mat.getTexCoordGenMode (stage) == CMaterial::TexCoordGenReflect;
						}
						else
						{
							pShader->TexGen[stage] = mat.getTexCoordGen(stage);
						}
					}
					else
					{
						if (mat.getTexCoordGen(stage))
						{
							pShader->TexGen[stage] = RemapTexGenTypeNeL2D3D[mat.getTexCoordGenMode (stage)];

							// Texture matrix
							pShader->ActivateInvViewModelTexMT[stage] = mat.getTexCoordGenMode (stage) == CMaterial::TexCoordGenObjectSpace;
						}
						else
						{
							pShader->TexGen[stage] = mat.getTexCoordGen(stage);
						}
					}
				}				
			}

			// Does this material needs a pixel shader ?
			if (touched & (IDRV_TOUCHED_TEXENV|IDRV_TOUCHED_BLEND|IDRV_TOUCHED_ALPHA_TEST))
			{
				bool _needPixelShader = false;
				if (matShader == CMaterial::Normal)
				{
					// Need alpha for this shader ?
					const bool _needsAlpha = needsAlpha (mat);

					// Need of constants ?
					uint numConstants;
					uint firstConstant;
					needsConstants (numConstants, firstConstant, mat, _needsAlpha);
					pShader->ConstantIndex = (uint8)((firstConstant==0xffffffff)?0:firstConstant);

					// Need a constant color for the diffuse component ?
					pShader->NeedsConstantForDiffuse = needsConstantForDiffuse (mat, _needsAlpha);

					// Need pixel shader ?
#ifndef NL_FORCE_PIXEL_SHADER_USE_FOR_NORMAL_SHADERS
					_needPixelShader = (numConstants > 1) || ((numConstants==1) && pShader->NeedsConstantForDiffuse);
#else // NL_FORCE_PIXEL_SHADER_USE_FOR_NORMAL_SHADERS
					_needPixelShader = true;
#endif // NL_FORCE_PIXEL_SHADER_USE_FOR_NORMAL_SHADERS
					if (_needPixelShader)
					{
#ifdef NL_DEBUG_D3D
						// Check, should not occured
						nlassertex (_PixelShader, ("STOP : no pixel shader available. Can't render this material."));
#endif // NL_DEBUG_D3D

						// The shader description
						CNormalShaderDesc normalShaderDesc;

						// Setup descriptor
						uint stage;
						for(stage=0 ; stage<(uint)maxTexture; stage++)
						{
							// Stage used ?
							normalShaderDesc.StageUsed[stage] = mat.getTexture (stage) != NULL;
							normalShaderDesc.TexEnvMode[stage] = mat.getTexEnvMode(stage);
						}

						// Build the pixel shader
						pShader->PixelShader = buildPixelShader (normalShaderDesc, false);
						if (!mat.isLighted())
							pShader->PixelShaderUnlightedNoVertexColor = buildPixelShader (normalShaderDesc, true);
						else
							pShader->PixelShaderUnlightedNoVertexColor = NULL;
					}
				}
				else
				{
					// Other shaders
					pShader->NeedsConstantForDiffuse = false;
					pShader->ConstantIndex = 0;
				}

				if (!_needPixelShader)
				{
					// Remove the old one
					pShader->PixelShader = NULL;
					pShader->PixelShaderUnlightedNoVertexColor = NULL;
				}
			}

			// Since modified, must rebind all D3D states. And do this also for the delete/new problem.
			/* If an old material is deleted, _CurrentMaterial is invalid. But this is grave only if a new 
				material is created, with the same pointer (bad luck). Since an newly allocated material always 
				pass here before use, we are sure to avoid any problems.
			*/
			_CurrentMaterial= NULL;
		}

		// Optimize: reset all flags at the end.
		mat.clearTouched(0xFFFFFFFF);
	}


	// 2. Setup / Bind Textures.
	//==========================
	// Must setup textures each frame. (need to test if touched).
	// Must separate texture setup and texture activation in 2 "for"...
	// because setupTexture() may disable all stage.

	if (matShader == CMaterial::Normal)
	{	
		uint stage;
		for(stage=0 ; stage<(uint)maxTexture; stage++)
		{
			ITexture	*text= mat.getTexture(stage);
			if (text != NULL && !setupTexture(*text))
				return(false);
		}
	}
		
	// Activate the textures.
	// Do not do it for Lightmap and per pixel lighting , because done in multipass in a very special fashion.
	// This avoid the useless multiple change of texture states per lightmapped object.
	// Don't do it also for Specular because the EnvFunction and the TexGen may be special.
	if(matShader == CMaterial::Normal)
	{
		uint stage;
		for(stage=0 ; stage<(uint)maxTexture; stage++)
		{
			ITexture	*text= mat.getTexture(stage);
			if (text)
			{
				// activate the texture, or disable texturing if NULL.
				setTexture(stage, text);
			}
			else
			{
				setTexture (stage, (LPDIRECT3DBASETEXTURE9)NULL);
				if (stage != 0)
				{
					setTextureState (stage, D3DTSS_COLOROP, D3DTOP_DISABLE);
					setTextureState (stage, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
				}
			}

			// Set the texture states
			if (text || (stage == 0))
			{
				// Doesn't use a pixel shader ? Set the textures stages
				if (pShader->PixelShader == NULL)
				{
					setTextureState (stage, D3DTSS_COLOROP, pShader->ColorOp[stage]);
					setTextureState (stage, D3DTSS_COLORARG0, pShader->ColorArg0[stage]);
					setTextureState (stage, D3DTSS_COLORARG1, pShader->ColorArg1[stage]);
					setTextureState (stage, D3DTSS_COLORARG2, pShader->ColorArg2[stage]);
					setTextureState (stage, D3DTSS_ALPHAOP, pShader->AlphaOp[stage]);
					setTextureState (stage, D3DTSS_ALPHAARG0, pShader->AlphaArg0[stage]);
					setTextureState (stage, D3DTSS_ALPHAARG1, pShader->AlphaArg1[stage]);
					setTextureState (stage, D3DTSS_ALPHAARG2, pShader->AlphaArg2[stage]);

					// If there is one constant and the tfactor is not needed for diffuse, use the tfactor as constant
					if (!pShader->NeedsConstantForDiffuse && pShader->ConstantIndex == stage)
						setRenderState (D3DRS_TEXTUREFACTOR, pShader->ConstantColor[stage]);
				}
				else
				{
					float colors[4];
					D3DCOLOR_FLOATS (colors, pShader->ConstantColor[stage]);
					setPixelShaderConstant (stage, colors);
				}
			}

			// Set texture generator state
			setTextureIndexMode (stage, pShader->TexGen[stage]!=D3DTSS_TCI_PASSTHRU, pShader->TexGen[stage]);

			// Need user matrix ?
			bool needUserMtx = mat.isUserTexMatEnabled(stage);
			D3DXMATRIX userMtx;
			if (needUserMtx)
				NL_D3D_MATRIX (userMtx, mat.getUserTexMat(stage));

			// Need cubic texture coord generator ?
			if (pShader->ActivateSpecularWorldTexMT[stage])
			{
				// Set the driver matrix
				setTextureState (stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3);
				if (!needUserMtx)
					setMatrix ((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0+stage), _D3DSpecularWorldTex);
				else
				{
					D3DXMatrixMultiply (&userMtx, &_D3DSpecularWorldTex, &userMtx);
					setMatrix ((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0+stage), userMtx);
				}
			}
			else if (pShader->ActivateInvViewModelTexMT[stage])
			{
				// Set the driver matrix
				setTextureState (stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3);
				if (!needUserMtx)
					setMatrix ((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0+stage), _D3DInvModelView);
				else
				{
					D3DXMatrixMultiply (&userMtx, &_D3DInvModelView, &userMtx);
					setMatrix ((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0+stage), userMtx);
				}
			}
			else
			{
				// Set the driver matrix
				if (needUserMtx)
				{
					setTextureState (stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3);
					setMatrix ((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0+stage), userMtx);
				}
				else
					setTextureState (stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
			}
		}
	}

	// Material has changed ?
	if (_CurrentMaterial != &mat)
	{
		// Restaure fog state to its current value
		setRenderState (D3DRS_FOGENABLE, _FogEnabled?TRUE:FALSE);

		// Flags
		const uint32 flags = mat.getFlags();

		// Two sided
		_DoubleSided = (flags&IDRV_MAT_DOUBLE_SIDED)!=0;

		// Handle backside
		setRenderState (D3DRS_CULLMODE, _DoubleSided?D3DCULL_NONE:_InvertCullMode?D3DCULL_CCW:D3DCULL_CW);

		if (matShader == CMaterial::Normal || matShader == CMaterial::Specular || matShader == CMaterial::Cloud)
		{
			// Active states
			bool blend = (flags&IDRV_MAT_BLEND) != 0;
			if (blend)
			{
				setRenderState (D3DRS_ALPHABLENDENABLE, TRUE);
				setRenderState (D3DRS_SRCBLEND, pShader->SrcBlend);
				setRenderState (D3DRS_DESTBLEND, pShader->DstBlend);
			}
			else
				setRenderState (D3DRS_ALPHABLENDENABLE, FALSE);

			// Alpha test
			if (flags&IDRV_MAT_ALPHA_TEST)
			{
				setRenderState (D3DRS_ALPHATESTENABLE, TRUE);
				setRenderState (D3DRS_ALPHAREF, pShader->AlphaRef);
				setRenderState (D3DRS_ALPHAFUNC, D3DCMP_GREATER);
			}
			else
				setRenderState (D3DRS_ALPHATESTENABLE, FALSE);

			// Z buffer
			setRenderState (D3DRS_ZWRITEENABLE, (flags&IDRV_MAT_ZWRITE)?TRUE:FALSE);
			setRenderState (D3DRS_ZFUNC, pShader->ZComp);
			float flt = mat.getZBias () * _OODeltaZ;
			setRenderState (D3DRS_DEPTHBIAS, FTODW(flt));

			// Active lighting
			if (mat.isLighted())
			{
				setRenderState (D3DRS_LIGHTING, TRUE);
				_DeviceInterface->SetMaterial( &(pShader->Material) );
			}
			else
			{
				setRenderState (D3DRS_LIGHTING, FALSE);

				// Set pixel shader unlighted color ?
				if (pShader->PixelShader)
				{
					float colors[4];
					D3DCOLOR_FLOATS(colors,pShader->UnlightedColor);
					setPixelShaderConstant (5, colors);
				}
				else
				{
					if (pShader->NeedsConstantForDiffuse)
						setRenderState (D3DRS_TEXTUREFACTOR, pShader->UnlightedColor);
				}
			}

			setRenderState (D3DRS_SPECULARENABLE, pShader->SpecularEnabled);

			// Active vertex color if not lighted or vertex color forced
			if (mat.isLightedVertexColor ())
			{
				setRenderState (D3DRS_COLORVERTEX, TRUE);
				setRenderState (D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
			}
			else
			{
				setRenderState (D3DRS_COLORVERTEX, FALSE);
				setRenderState (D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
			}

			// Disable fog if dest blend is ONE
			if (blend && (pShader->DstBlend == D3DBLEND_ONE))
			{
				setRenderState (D3DRS_FOGENABLE, FALSE);
			}
		}

		// Set the good shader
		switch (matShader)
		{
		case CMaterial::Normal:
			{
				// No shader
				activeShader (NULL);

				/* If unlighted trick is needed, set the shader later */
				if (!pShader->NeedsConstantForDiffuse)
					setPixelShader (pShader->PixelShader);
			}
			break;
		case CMaterial::LightMap:
			{
				static const uint32 RGBMaskPacked = CRGBA(255,255,255,0).getPacked();			

				// Count the lightmaps
				uint lightmap;
				uint lightmapCount = 0;
				uint lightmapMask = 0;
				for(lightmap=0 ; lightmap<(sint)mat._LightMaps.size() ; lightmap++)
				{
					if (mat._LightMaps[lightmap].Factor.getPacked() & RGBMaskPacked)
					{
						lightmapCount++;
						lightmapMask |= 1<<lightmap;
					}
				}

				// activate the appropriate shader
				if (mat.getBlend())
				{
					if(mat._LightMapsMulx2)
					{
						switch (lightmapCount)
						{
						case 0:	activeShader (&_ShaderLightmap0BlendX2);	break;
						case 1:	activeShader (&_ShaderLightmap1BlendX2);	break;
						case 2:	activeShader (&_ShaderLightmap2BlendX2);	break;
						case 3:	activeShader (&_ShaderLightmap3BlendX2);	break;
						default:	activeShader (&_ShaderLightmap4BlendX2);	break;
						}
					}
					else
					{
						switch (lightmapCount)
						{
						case 0:	activeShader (&_ShaderLightmap0Blend);	break;
						case 1:	activeShader (&_ShaderLightmap1Blend);	break;
						case 2:	activeShader (&_ShaderLightmap2Blend);	break;
						case 3:	activeShader (&_ShaderLightmap3Blend);	break;
						default:	activeShader (&_ShaderLightmap4Blend);	break;
						}
					}
				}
				else
				{
					if(mat._LightMapsMulx2)
					{
						switch (lightmapCount)
						{
						case 0:	activeShader (&_ShaderLightmap0X2);	break;
						case 1:	activeShader (&_ShaderLightmap1X2);	break;
						case 2:	activeShader (&_ShaderLightmap2X2);	break;
						case 3:	activeShader (&_ShaderLightmap3X2);	break;
						default:	activeShader (&_ShaderLightmap4X2);	break;
						}
					}
					else
					{
						switch (lightmapCount)
						{
						case 0:	activeShader (&_ShaderLightmap0);	break;
						case 1:	activeShader (&_ShaderLightmap1);	break;
						case 2:	activeShader (&_ShaderLightmap2);	break;
						case 3:	activeShader (&_ShaderLightmap3);	break;
						default:	activeShader (&_ShaderLightmap4);	break;
						}
					}
				}

				// Setup the texture
				ITexture *texture = mat.getTexture(0);
				if (!setShaderTexture (0, texture))
					return false;

				// Get the effect
				nlassert (_CurrentShader);
				CShaderDrvInfosD3D *shaderInfo = static_cast<CShaderDrvInfosD3D*>((IShaderDrvInfos*)_CurrentShader->_DrvInfo);
				ID3DXEffect			*effect = shaderInfo->Effect;


				// Set the ambiant for 8Bit Light Compression
				{
					// Sum ambiant of all active lightmaps.
					uint32	r=0;
					uint32	g=0;
					uint32	b=0;
					for(lightmap=0 ; lightmap<(sint)mat._LightMaps.size() ; lightmap++)
					{
						if (lightmapMask & (1<<lightmap))
						{
							uint	wla= lightmap;
							// must mul them by their respective mapFactor too
							CRGBA ambFactor = mat._LightMaps[wla].Factor;
							CRGBA lmcAmb= mat._LightMaps[wla].LMCAmbient;
							r+= ((uint32)ambFactor.R  * ((uint32)lmcAmb.R+(lmcAmb.R>>7))) >>8;
							g+= ((uint32)ambFactor.G  * ((uint32)lmcAmb.G+(lmcAmb.G>>7))) >>8;
							b+= ((uint32)ambFactor.B  * ((uint32)lmcAmb.B+(lmcAmb.B>>7))) >>8;
						}
					}
					r= std::min(r, (uint32)255);
					g= std::min(g, (uint32)255);
					b= std::min(b, (uint32)255);
					CRGBA	lmcAmbient((uint8)r,(uint8)g,(uint8)b,255);

					// Set into FX shader
					effect->SetInt(shaderInfo->ColorHandle[0], NL_D3DCOLOR_RGBA(lmcAmbient));
					if (shaderInfo->FactorHandle[0])
					{
						float colors[4];
						NL_FLOATS(colors,lmcAmbient);
						effect->SetFloatArray (shaderInfo->FactorHandle[0], colors, 4);
					}
				}
				

				// Set the lightmaps
				lightmapCount = 0;
				for(lightmap=0 ; lightmap<(sint)mat._LightMaps.size() ; lightmap++)
				{
					if (lightmapMask & (1<<lightmap))
					{
						// Set the lightmap texture
						ITexture	*text= mat._LightMaps[lightmap].Texture;
						if (text != NULL && !setShaderTexture (lightmapCount+1, text))
							return false;

						// Get the lightmap color factor, and mul by lmcDiffuse compression
						CRGBA lmapFactor = mat._LightMaps[lightmap].Factor;
						CRGBA lmcDiff= mat._LightMaps[lightmap].LMCDiffuse;
						lmapFactor.R = (uint8)(((uint32)lmapFactor.R  * ((uint32)lmcDiff.R+(lmcDiff.R>>7))) >>8);
						lmapFactor.G = (uint8)(((uint32)lmapFactor.G  * ((uint32)lmcDiff.G+(lmcDiff.G>>7))) >>8);
						lmapFactor.B = (uint8)(((uint32)lmapFactor.B  * ((uint32)lmcDiff.B+(lmcDiff.B>>7))) >>8);
						lmapFactor.A = 255;
						
						// Set the lightmap color factor into shader
						effect->SetInt(shaderInfo->ColorHandle[lightmapCount+1], NL_D3DCOLOR_RGBA(lmapFactor));
						if (shaderInfo->FactorHandle[lightmapCount+1])
						{
							float colors[4];
							NL_FLOATS(colors,lmapFactor);
							effect->SetFloatArray (shaderInfo->FactorHandle[lightmapCount+1], colors, 4);
						}
						lightmapCount++;
					}
				}
			}
			break;
		case CMaterial::Specular:
			{
				// No shader
				activeShader (NULL);
				setPixelShader (NULL);

				// Setup the texture
				ITexture *texture = mat.getTexture(0);
				if (!texture || !setupTexture (*texture))
					return false;
				setTexture (0, texture);

				// Setup the texture
				texture = mat.getTexture(1);
				if (!texture || !setupTexture (*texture))
					return false;
				setTexture (1, texture);

				// Set the driver matrix
				if (texture->isTextureCube())
				{
					setTextureIndexMode (1, true, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);
					setTextureState (1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3);
					setMatrix ((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0+1), _D3DSpecularWorldTex);
				}

				setRenderState (D3DRS_LIGHTING, mat.isLighted()?TRUE:FALSE);
				setRenderState (D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
				setTextureState (0, D3DTSS_COLOROP, D3DTOP_MODULATE);
				setTextureState (0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
				setTextureState (0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
				setTextureState (0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
				setTextureState (0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
				setTextureState (0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
				setTextureState (1, D3DTSS_COLOROP, D3DTOP_MULTIPLYADD);
				setTextureState (1, D3DTSS_COLORARG0, D3DTA_CURRENT);
				setTextureState (1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
				setTextureState (1, D3DTSS_COLORARG2, D3DTA_CURRENT|D3DTA_ALPHAREPLICATE);
			}
			break;
		case CMaterial::Cloud:
			{
				activeShader (&_ShaderCloud);

				// Get the shader
				nlassert (_CurrentShader);
				CShaderDrvInfosD3D *shaderInfo = static_cast<CShaderDrvInfosD3D*>((IShaderDrvInfos*)_CurrentShader->_DrvInfo);

				// Set the constant
				ID3DXEffect			*effect = shaderInfo->Effect;
				float colors[4];
				CRGBA color = mat.getColor();
				color.R = 255;
				color.G = 255;
				color.B = 255;
				NL_FLOATS(colors,color);
				effect->SetFloatArray (shaderInfo->FactorHandle[0], colors, 4);

				// Set the texture
				if (!setShaderTexture (0, mat.getTexture(0)) || !setShaderTexture (1, mat.getTexture(1)))
					return false;
			}
			break;
		}

		// New material setuped
		_CurrentMaterial = &mat;

		// Update variables
		_CurrentMaterialInfo = pShader;
	}

	return true;
}

// ***************************************************************************

bool CDriverD3D::needsConstants (uint &numConstant, uint &firstConstant, CMaterial &mat, bool needAlpha)
{
	/* Use a pixel shader if more than a stage constant color is used */
	uint i;
	numConstant = 0;
	firstConstant = 0xffffffff;
	for (i=0; i<IDRV_MAT_MAXTEXTURES; i++)
	{
		if (mat.getTexture(i))
		{
			CMaterial::CTexEnv texEnv;
			texEnv.EnvPacked = mat.getTexEnvMode(i);

			// Does this stage use a constant color ?
			if ((texEnv.Env.SrcArg0RGB == CMaterial::Constant) ||
				((texEnv.Env.SrcArg1RGB == CMaterial::Constant) && (texEnv.Env.OpRGB != CMaterial::Replace)) ||
				((texEnv.Env.SrcArg0Alpha == CMaterial::Constant) && needAlpha) ||
				((texEnv.Env.SrcArg1Alpha == CMaterial::Constant) && (texEnv.Env.OpRGB != CMaterial::Replace) && needAlpha) ||
				(texEnv.Env.OpRGB == CMaterial::InterpolateConstant) )
			{
				if (firstConstant == 0xffffffff)
					firstConstant = i;
				numConstant++;
			}
		}
	}
	return numConstant > 1;
}

// ***************************************************************************

bool CDriverD3D::needsConstantForDiffuse (CMaterial &mat, bool needAlpha)
{
	// If lighted, don't need the tfactor
	if (mat.isLighted())
		return false;
	return true;

	/* Use a pixel shader if more than a stage constant color is used */
	uint i;
	uint constantColor = 0;
	for (i=0; i<IDRV_MAT_MAXTEXTURES; i++)
	{
		if (mat.getTexture(i))
		{
			CMaterial::CTexEnv texEnv;
			texEnv.EnvPacked = mat.getTexEnvMode(i);

			// Does this stage use a constant color ?
			if ((texEnv.Env.SrcArg0RGB == CMaterial::Diffuse) ||
				((texEnv.Env.SrcArg1RGB == CMaterial::Diffuse) && (texEnv.Env.OpRGB != CMaterial::Replace)) ||
				((texEnv.Env.SrcArg0Alpha == CMaterial::Diffuse) && needAlpha) ||
				((texEnv.Env.SrcArg1Alpha == CMaterial::Diffuse) && (texEnv.Env.OpRGB != CMaterial::Replace) && needAlpha) ||
				(texEnv.Env.OpRGB == CMaterial::InterpolateDiffuse) )
				return true;
		}
		else if (i == 0)
		{
			return true;
		}
	}
	return false;
}

// ***************************************************************************

bool CDriverD3D::needsAlpha (CMaterial &mat)
{
	// Alpha blend or alpha test
	if (mat.getBlend() || mat.getAlphaTest())
		return true;

	// Normal shader ?
	if (mat.getShader() == CMaterial::Normal)
	{
		// Alpha used in rgb stages ?
		uint i;
		for (i=0; i<IDRV_MAT_MAXTEXTURES; i++)
		{
			// Stage used ?
			if (mat.getTexture(i))
			{
				// Does the RGB pipeline needs previous alpha result ?
				CMaterial::CTexEnv texEnv;
				texEnv.EnvPacked = mat.getTexEnvMode(i);

				if (texEnv.Env.SrcArg0RGB == CMaterial::Previous)
				{
					if ((texEnv.Env.OpArg0RGB == CMaterial::SrcAlpha) || 
						(texEnv.Env.OpArg0RGB == CMaterial::InvSrcAlpha))
						return true;
				}
				if ((texEnv.Env.SrcArg1RGB == CMaterial::Previous) && (texEnv.Env.OpRGB != CMaterial::Replace))
				{
					if ((texEnv.Env.OpArg1RGB == CMaterial::SrcAlpha) || 
						(texEnv.Env.OpArg1RGB == CMaterial::InvSrcAlpha))
						return true;
				}
				if ((texEnv.Env.OpRGB == CMaterial::InterpolatePrevious) || (texEnv.Env.OpRGB == CMaterial::InterpolateConstant))
				{
					return true;
				}
			}
		}
	}

	// Doesn't need alpha
	return false;
}

// ***************************************************************************

const char *RemapPSInstructions[CMaterial::TexOperatorCount]=
{
	"mov",	// Replace
	"mul",	// Modulate
	"add",	// Add
	"add",	// AddSigned
	"lrp",	// InterpolateTexture
	"lrp",	// InterpolatePrevious
	"lrp",	// InterpolateDiffuse
	"lrp",	// InterpolateConstant
	"bem",	// EMBM
};

// ***************************************************************************

const char *RemapPSThirdArguments[CMaterial::TexOperatorCount][2]=
{
	{"", ""},		// Replace
	{"", ""},		// Modulate
	{"", ""},		// Add
	{"", ""},		// AddSigned
	{"t", "t"},		// InterpolateTexture
	{"r0", "r0"},	// InterpolatePrevious
	{"v0", "c5"},	// InterpolateDiffuse
	{"c", "c"},		// InterpolateConstant
	{"", ""},		// EMBM
};

// ***************************************************************************

// Only for stage 0
const char *RemapPSThirdArguments0[CMaterial::TexOperatorCount][2]=
{
	{"", ""},		// Replace
	{"", ""},		// Modulate
	{"", ""},		// Add
	{"", ""},		// AddSigned
	{"t", "t"},		// InterpolateTexture
	{"v0", "c5"},	// InterpolatePrevious
	{"v0", "c5"},	// InterpolateDiffuse
	{"c", "c"},		// InterpolateConstant
	{"", ""},		// EMBM
};

// ***************************************************************************

const char *RemapPSSecondRegisterModificator[CMaterial::TexOperatorCount]=
{
	"",		// Replace
	"",		// Modulate
	"",		// Add
	"_bias",	// AddSigned
	"",		// InterpolateTexture
	"",		// InterpolatePrevious
	"",		// InterpolateDiffuse
	"",		// InterpolateConstant
	"",		// EMBM
};

// ***************************************************************************

const char *RemapPSArguments[CMaterial::TexOperatorCount][2]=
{
	{"t", "t"},	// Texture
	{"r0", "r0"},	// Previous
	{"v0", "c5"},	// Diffuse
	{"c", "c"},	// Constant
};

// ***************************************************************************

// Only for stage 0
const char *RemapPSArguments0[CMaterial::TexOperatorCount][2]=
{
	{"t", "t"},	// Texture
	{"v0", "c5"},	// Previous
	{"v0", "c5"},	// Diffuse
	{"c", "c"},	// Constant
};

// ***************************************************************************

void buildColorOperation (string &dest, const char *prefix, const char *destSizzle, uint stage, CMaterial::TTexOperator &op, CMaterial::TTexSource src0, CMaterial::TTexSource src1, CMaterial::TTexOperand &op0, CMaterial::TTexOperand &op1, bool unlightedNoVertexColor)
{
	// Refix
	dest += prefix;

	// RGB operation
	dest += RemapPSInstructions[op];

	// Destination argument
	dest += " r0";
	dest += destSizzle;

	// Need a third argument ?
	const char *remapPSThirdArguments = ((stage==0)?RemapPSThirdArguments0:RemapPSThirdArguments)[op][(uint)unlightedNoVertexColor];
	if (remapPSThirdArguments[0] != 0)
	{
		dest += ", ";
		dest += remapPSThirdArguments;

		// Need stage postfix ?
		if ((op == CMaterial::InterpolateTexture) || (op == CMaterial::InterpolateConstant))
			dest += toString (stage);

		// Add alpha swizzle
		dest += ".w";
	}

	// First argument
	dest += ", ";

	// Inverted ?
	if ((op0 == CMaterial::InvSrcColor) || (op0 == CMaterial::InvSrcAlpha))
		dest += "1-";

	// The first operator argument
	dest += ((stage==0)?RemapPSArguments0:RemapPSArguments)[src0][(uint)unlightedNoVertexColor];

	// Need stage postfix ?
	if ((src0 == CMaterial::Texture) || (src0 == CMaterial::Constant))
		dest += toString (stage);

	// Need alpha ?
	if ((op0 == CMaterial::SrcAlpha) || (op0 == CMaterial::InvSrcAlpha))
		dest += ".w";

	if (op != CMaterial::Replace)
	{
		dest += ", ";

		// Inverted ?
		if ((op1 == CMaterial::InvSrcColor) || (op1 == CMaterial::InvSrcAlpha))
			dest += "1-";

		dest += ((stage==0)?RemapPSArguments0:RemapPSArguments)[src1][(uint)unlightedNoVertexColor];

		// Second register modifier
		dest += RemapPSSecondRegisterModificator[op];

		// Need stage postfix ?
		if ((src1 == CMaterial::Texture) || (src1 == CMaterial::Constant))
			dest += toString (stage);

		// Need alpha ?
		if ((op1 == CMaterial::SrcAlpha) || (op1 == CMaterial::InvSrcAlpha))
			dest += ".w";
	}

	// End
	dest += ";\n";
}

// ***************************************************************************

IDirect3DPixelShader9	*CDriverD3D::buildPixelShader (const CNormalShaderDesc &normalShaderDesc, bool unlightedNoVertexColor)
{
	static string shaderText;
	shaderText = "ps_1_1;\n";
	
	// Look for a shader already created
	std::list<CNormalShaderDesc> &normalPixelShaders = _NormalPixelShaders[(uint)unlightedNoVertexColor];
	std::list<CNormalShaderDesc>::iterator ite = normalPixelShaders.begin();	
	while (ite != normalPixelShaders.end())
	{
		if (normalShaderDesc == *ite)
		{
			// Good one
			return (*ite).PixelShader;
		}
		ite++;
	}

	// For each state, texture lookup
	uint i;
	for (i=0; i<IDRV_MAT_MAXTEXTURES; i++)
	{
		// Stage used ?
		if (normalShaderDesc.StageUsed[i])
			shaderText += "tex t"+toString (i)+";\n";
	}
	
	// For each state, color operations
	for (i=0; i<IDRV_MAT_MAXTEXTURES; i++)
	{
		// Stage used ?
		if (normalShaderDesc.StageUsed[i])
		{
			// Texenv
			CMaterial::CTexEnv texEnv;
			texEnv.EnvPacked = normalShaderDesc.TexEnvMode[i];

			// RGB
			CMaterial::TTexOperator texOp = (CMaterial::TTexOperator)texEnv.Env.OpRGB;
			CMaterial::TTexSource src0 = (CMaterial::TTexSource)texEnv.Env.SrcArg0RGB;
			CMaterial::TTexSource src1 = (CMaterial::TTexSource)texEnv.Env.SrcArg1RGB;
			CMaterial::TTexOperand op0 = (CMaterial::TTexOperand)texEnv.Env.OpArg0RGB;
			CMaterial::TTexOperand op1 = (CMaterial::TTexOperand)texEnv.Env.OpArg1RGB;
			buildColorOperation (shaderText, "", ".xyz", i, texOp, src0, src1, op0, op1, unlightedNoVertexColor);

			// Alpha
			texOp = (CMaterial::TTexOperator)texEnv.Env.OpAlpha;
			src0 = (CMaterial::TTexSource)texEnv.Env.SrcArg0Alpha;
			src1 = (CMaterial::TTexSource)texEnv.Env.SrcArg1Alpha;
			op0 = (CMaterial::TTexOperand)texEnv.Env.OpArg0Alpha;
			op1 = (CMaterial::TTexOperand)texEnv.Env.OpArg1Alpha;
			buildColorOperation (shaderText, "+", ".w", i, texOp, src0, src1, op0, op1, unlightedNoVertexColor);
		}
		// No texture shader ?
		else if (i == 0)
		{
			// RGB
			CMaterial::TTexOperator texOp = CMaterial::Replace;
			CMaterial::TTexSource src0 = CMaterial::Diffuse;
			CMaterial::TTexOperand op0 = CMaterial::SrcColor;
			buildColorOperation (shaderText, "", ".xyz", i, texOp, src0, src0, op0, op0, unlightedNoVertexColor);

			// Alpha
			texOp = CMaterial::Replace;
			src0 = CMaterial::Diffuse;
			op0 = CMaterial::SrcAlpha;
			buildColorOperation (shaderText, "+", ".w", i, texOp, src0, src0, op0, op0, unlightedNoVertexColor);
		}
	}

	// Add the specular
	shaderText += "add r0.rgb, r0, v1;\n";

	// Dump the pixel shader
#ifdef NL_DEBUG_D3D
	nlinfo("Assemble Pixel Shader : ");
	int lineBegin = 0;
	int lineEnd;
	while ((lineEnd = shaderText.find('\n', lineBegin)) != string::npos)
	{
		nlinfo(shaderText.substr (lineBegin, lineEnd-lineBegin).c_str());
		lineBegin = lineEnd+1;
	}
	nlinfo(shaderText.substr (lineBegin, lineEnd-lineBegin).c_str());
#endif // NL_DEBUG_D3D

	// Add the shader
	normalPixelShaders.push_back (normalShaderDesc);

	// Assemble and create the shader
	LPD3DXBUFFER pShader;
	LPD3DXBUFFER pErrorMsgs;
	if (D3DXAssembleShader (shaderText.c_str(), shaderText.size(), NULL, NULL, 0, &pShader, &pErrorMsgs) == D3D_OK)
	{
		IDirect3DPixelShader9 *shader;
		if (_DeviceInterface->CreatePixelShader((DWORD*)pShader->GetBufferPointer(), &shader) == D3D_OK)
			normalPixelShaders.back().PixelShader = shader;
		else
			normalPixelShaders.back().PixelShader = NULL;
	}
	else
	{
		nlwarning ("Can't assemble pixel shader:");
		nlwarning ((const char*)pErrorMsgs->GetBufferPointer());
		normalPixelShaders.back().PixelShader = NULL;
	}

	return normalPixelShaders.back().PixelShader;
}

// ***************************************************************************

void CDriverD3D::startSpecularBatch()
{
	/* Not used in direct3d, use normal caching */
}

// ***************************************************************************

void CDriverD3D::endSpecularBatch()
{
	/* Not used in direct3d, use normal caching */
}

// ***************************************************************************

bool CDriverD3D::supportBlendConstantColor() const
{
	/* Not supported in D3D */
	return false;
};

// ***************************************************************************

void CDriverD3D::setBlendConstantColor(NLMISC::CRGBA col)
{
	/* Not supported in D3D */
};

// ***************************************************************************

NLMISC::CRGBA CDriverD3D::getBlendConstantColor() const
{
	/* Not supported in D3D */
	return CRGBA::White;
};

// ***************************************************************************

void CDriverD3D::enablePolygonSmoothing(bool smooth)
{
	/* Not supported in D3D */
}

// ***************************************************************************

bool CDriverD3D::isPolygonSmoothingEnabled() const
{
	/* Not supported in D3D */
	return false;
}

// ***************************************************************************

sint CDriverD3D::beginMaterialMultiPass()
{ 
	beginMultiPass ();
	return _CurrentShaderPassCount;
}

// ***************************************************************************

void CDriverD3D::setupMaterialPass(uint pass)
{ 
	activePass (pass);
}

// ***************************************************************************

void CDriverD3D::endMaterialMultiPass()
{
	endMultiPass ();
}

// ***************************************************************************

bool CDriverD3D::supportCloudRenderSinglePass () const
{
	return _PixelShader;
}

// ***************************************************************************

} // NL3D
