/** \file driver_direct3d_shader.cpp
 * Direct 3d driver implementation
 *
 * $Id: driver_direct3d_shader.cpp,v 1.7.4.1 2004/09/14 15:33:43 vizerie Exp $
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

#include "driver_direct3d.h"
#include "resource.h"

using namespace std;
using namespace NLMISC;

namespace NL3D 
{

// ***************************************************************************
// The state manager with cache
// ***************************************************************************

HRESULT CDriverD3D::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
	H_AUTO_D3D(CDriverD3D_QueryInterface)
	return D3D_OK;
}

// ***************************************************************************

ULONG CDriverD3D::AddRef(VOID)
{
	H_AUTO_D3D(CDriverD3D_AddRef)
	return 0;
}

// ***************************************************************************

ULONG CDriverD3D::Release(VOID)
{
	H_AUTO_D3D(CDriverD3D_Release)
	return 0;
}

// ***************************************************************************

HRESULT CDriverD3D::LightEnable(DWORD Index, BOOL Enable)
{
	H_AUTO_D3D(CDriverD3D_LightEnable)
	enableLight ((uint8)Index, Enable!=FALSE);
	return D3D_OK;
}

// ***************************************************************************

HRESULT CDriverD3D::SetFVF(DWORD FVF)
{
	H_AUTO_D3D(CDriverD3D_SetFVF)
	// Not implemented
	return D3D_OK;
}

// ***************************************************************************

HRESULT CDriverD3D::SetLight(DWORD Index, CONST D3DLIGHT9* pLight)
{
	H_AUTO_D3D(CDriverD3D_SetLight)
	_LightCache[Index].Light = *pLight;
	touchRenderVariable (&_LightCache[Index]);
	return D3D_OK;
}

// ***************************************************************************

HRESULT CDriverD3D::SetMaterial(CONST D3DMATERIAL9* pMaterial)
{
	H_AUTO_D3D(CDriverD3D_SetMaterial)
	setMaterialState( *pMaterial );
	return D3D_OK;
}

// ***************************************************************************

HRESULT CDriverD3D::SetNPatchMode(FLOAT nSegments)
{
	H_AUTO_D3D(CDriverD3D_SetNPatchMode)
	// Not implemented
	return D3D_OK;
}

// ***************************************************************************

HRESULT CDriverD3D::SetPixelShader(LPDIRECT3DPIXELSHADER9 pShader)
{
	H_AUTO_D3D(CDriverD3D_SetPixelShader)
	setPixelShader (pShader);
	return D3D_OK;
}

// ***************************************************************************

HRESULT CDriverD3D::SetPixelShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT RegisterCount)
{
	H_AUTO_D3D(CDriverD3D_SetPixelShaderConstantB)
	uint i;
	for (i=0; i<RegisterCount; i++)
		setPixelShaderConstant (i+StartRegister, (int*)(pConstantData+i*4));
	return D3D_OK;
}

// ***************************************************************************

HRESULT CDriverD3D::SetPixelShaderConstantF(UINT StartRegister, CONST FLOAT* pConstantData, UINT RegisterCount)
{
	H_AUTO_D3D(CDriverD3D_SetPixelShaderConstantF)
	uint i;
	for (i=0; i<RegisterCount; i++)
		setPixelShaderConstant (i+StartRegister, pConstantData+i*4);
	return D3D_OK;
}

// ***************************************************************************

HRESULT CDriverD3D::SetPixelShaderConstantI(UINT StartRegister, CONST INT* pConstantData, UINT RegisterCount)
{
	H_AUTO_D3D(CDriverD3D_SetPixelShaderConstantI)
	uint i;
	for (i=0; i<RegisterCount; i++)
		setPixelShaderConstant (i+StartRegister, pConstantData+i*4);
	return D3D_OK;
}

// ***************************************************************************

HRESULT CDriverD3D::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
	H_AUTO_D3D(CDriverD3D_SetRenderState)
	setRenderState (State, Value);
	return D3D_OK;
}

// ***************************************************************************

HRESULT CDriverD3D::SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value)
{
	H_AUTO_D3D(CDriverD3D_SetSamplerState)
	setSamplerState (Sampler, Type, Value);
	return D3D_OK;
}

// ***************************************************************************

HRESULT CDriverD3D::SetTexture (DWORD Stage, LPDIRECT3DBASETEXTURE9 pTexture)
{
	H_AUTO_D3D(CDriverD3D_SetTexture )
	// Look for the current texture
	uint i;
	const uint count = _CurrentShaderTextures.size();
	for (i=0; i<count; i++)
	{
		const CTextureRef &ref = _CurrentShaderTextures[i];
		if (ref.D3DTexture == pTexture)
		{
			// Set the additionnal stage set by NeL texture (D3DSAMP_ADDRESSU, D3DSAMP_ADDRESSV, D3DSAMP_MAGFILTER, D3DSAMP_MINFILTER and D3DSAMP_MIPFILTER)
			setTexture (Stage, ref.NeLTexture);
			break;
		}
	}
	if (i == count)
		setTexture (Stage, pTexture);

	return D3D_OK;
}

// ***************************************************************************

HRESULT CDriverD3D::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
	H_AUTO_D3D(CDriverD3D_SetTextureStageState)
	if (Type == D3DTSS_TEXCOORDINDEX)
		setTextureIndexUV (Stage, Value);
	else
		setTextureState (Stage, Type, Value);
	return D3D_OK;
}

// ***************************************************************************

HRESULT CDriverD3D::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix)
{
	H_AUTO_D3D(CDriverD3D_SetTransform)
	setMatrix (State, *pMatrix);
	return D3D_OK;
}

// ***************************************************************************

HRESULT CDriverD3D::SetVertexShader(LPDIRECT3DVERTEXSHADER9 pShader)
{
	H_AUTO_D3D(CDriverD3D_SetVertexShader)
	setVertexProgram (pShader, NULL);
	return D3D_OK;
}

// ***************************************************************************

HRESULT CDriverD3D::SetVertexShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT RegisterCount)
{
	H_AUTO_D3D(CDriverD3D_SetVertexShaderConstantB)
	uint i;
	for (i=0; i<RegisterCount; i++)
		setVertexProgramConstant (i+StartRegister, (int*)(pConstantData+i*4));
	return D3D_OK;
}

// ***************************************************************************

HRESULT CDriverD3D::SetVertexShaderConstantF(UINT StartRegister, CONST FLOAT* pConstantData, UINT RegisterCount)
{
	H_AUTO_D3D(CDriverD3D_SetVertexShaderConstantF)
	uint i;
	for (i=0; i<RegisterCount; i++)
		setVertexProgramConstant (i+StartRegister, pConstantData+i*4);
	return D3D_OK;
}

// ***************************************************************************

HRESULT CDriverD3D::SetVertexShaderConstantI(UINT StartRegister, CONST INT* pConstantData, UINT RegisterCount)
{
	H_AUTO_D3D(CDriverD3D_SetVertexShaderConstantI)
	uint i;
	for (i=0; i<RegisterCount; i++)
		setVertexProgramConstant (i+StartRegister, pConstantData+i*4);
	return D3D_OK;
}

// ***************************************************************************

CShaderDrvInfosD3D::CShaderDrvInfosD3D(IDriver *drv, ItShaderDrvInfoPtrList it) : IShaderDrvInfos(drv, it)
{
	H_AUTO_D3D(CShaderDrvInfosD3D_CShaderDrvInfosD3D)
	Validated = false;
}

// ***************************************************************************

CShaderDrvInfosD3D::~CShaderDrvInfosD3D()
{
	H_AUTO_D3D(CShaderDrvInfosD3D_CShaderDrvInfosD3DDtor)
	Effect->Release();
}

// ***************************************************************************

bool CDriverD3D::validateShader(CShader *shader)
{
	H_AUTO_D3D(CDriverD3D_validateShader)
	CShaderDrvInfosD3D *shaderInfo = static_cast<CShaderDrvInfosD3D*>((IShaderDrvInfos*)shader->_DrvInfo);

	if (!shaderInfo->Validated)
	{
		// Choose the good method
		D3DXHANDLE hTechnique = NULL;
		D3DXHANDLE hCurrentTechnique = NULL;
		while (hTechnique == NULL)
		{
			if (shaderInfo->Effect->FindNextValidTechnique(hCurrentTechnique, &hCurrentTechnique) != D3D_OK)
				return false;

			// Info

#ifdef NL_FORCE_TEXTURE_STAGE_COUNT
			D3DXTECHNIQUE_DESC desc;
			nlverify (shaderInfo->Effect->GetTechniqueDesc(hCurrentTechnique, &desc) == D3D_OK);

			// Check this is compatible
			const uint len = strlen(desc.Name);
			if (len)
			{
				char shaderStageCount = desc.Name[len-1];
				if ((shaderStageCount>='0') && (shaderStageCount<='9'))
				{
					uint stageCount = NL_FORCE_TEXTURE_STAGE_COUNT;

					if ((uint)(shaderStageCount-'0')<=stageCount)
						// The good technique
						hTechnique = hCurrentTechnique;
				}
			}
#else // NL_FORCE_TEXTURE_STAGE_COUNT
			hTechnique = hCurrentTechnique;
#endif // NL_FORCE_TEXTURE_STAGE_COUNT

#ifdef NL_DEBUG_D3D
			{
				D3DXTECHNIQUE_DESC desc;
				nlverify (shaderInfo->Effect->GetTechniqueDesc(hCurrentTechnique, &desc) == D3D_OK);
				if (hTechnique)
					nlinfo ("Shader \"%s\" : use technique \"%s\" with %d passes.", shader->getName(), desc.Name, desc.Passes);
			}
#endif // NL_DEBUG_D3D
		}

		// Set the technique
		shaderInfo->Effect->SetTechnique(hTechnique);

		// Set the state manager
		shaderInfo->Effect->SetStateManager (this);

		shaderInfo->Validated = true;
	}
	return true;
}

// ***************************************************************************

bool CDriverD3D::activeShader(CShader *shd)
{
	H_AUTO_D3D(CDriverD3D_activeShader)
	if (_DisableHardwarePixelShader)
		return false;

	// Clear current textures
	_CurrentShaderTextures.clear();

	// Shader has been changed ?
	if (shd && shd->_ShaderChanged)
	{
		// Remove old shader
		shd->_DrvInfo.kill();

		// Already setuped ?
		CShaderDrvInfosD3D *shaderInfo = static_cast<CShaderDrvInfosD3D*>((IShaderDrvInfos*)shd->_DrvInfo);
		if ( !shd->_DrvInfo )
		{
			// insert into driver list. (so it is deleted when driver is deleted).
			ItShaderDrvInfoPtrList	it= _ShaderDrvInfos.insert(_ShaderDrvInfos.end());
			// create and set iterator, for future deletion.
			shaderInfo = new CShaderDrvInfosD3D(this, it);
			*it= shd->_DrvInfo = shaderInfo;
		}

		// Assemble the shader
		LPD3DXBUFFER pErrorMsgs;
		if (D3DXCreateEffect(_DeviceInterface,	shd->getText(), strlen(shd->getText())+1, NULL, NULL, 0, NULL, &(shaderInfo->Effect), &pErrorMsgs) 
			== D3D_OK)
		{
			// Get the texture handle
			uint i;
			for (i=0; i<CShaderDrvInfosD3D::MaxShaderTexture; i++)
			{
				string name = "texture" + toString (i);
				shaderInfo->TextureHandle[i] = shaderInfo->Effect->GetParameterByName(NULL, name.c_str());
				name = "color" + toString (i);
				shaderInfo->ColorHandle[i] = shaderInfo->Effect->GetParameterByName(NULL, name.c_str());
				name = "factor" + toString (i);
				shaderInfo->FactorHandle[i] = shaderInfo->Effect->GetParameterByName(NULL, name.c_str());
				name = "scalarFloat" + toString (i);
				shaderInfo->ScalarFloatHandle[i] = shaderInfo->Effect->GetParameterByName(NULL, name.c_str());				
			}
		}
		else
		{
			nlwarning ("Can't create shader:");
			nlwarning ((const char*)pErrorMsgs->GetBufferPointer());
			shd->_ShaderChanged = false;
			_CurrentShader = NULL;
			return false;
		}

		// Done
		shd->_ShaderChanged = false;
	}

	// Set the shader
	_CurrentShader = shd;

	return true;
}


// ***************************************************************************
// tmp for debug
static void setFX(CShader &s, const char *name, INT rsc, CDriverD3D *drv)
{
	H_AUTO_D3D(setFX)
	HRSRC hrsrc = FindResource(HInstDLL, MAKEINTRESOURCE(rsc), "FX");
	HGLOBAL hglob = LoadResource(HInstDLL, hrsrc);
	const char *datas = (const char *) LockResource(hglob);
	s.setName(name);
	s.setText(datas);
	nlverify (drv->activeShader (&s));
}


#define setFx(a,b) { setFX(a, #b, b, this); }

void CDriverD3D::initInternalShaders()
{
	H_AUTO_D3D(CDriverD3D_initInternalShaders)
	setFx(_ShaderLightmap0,lightmap0);
	setFx(_ShaderLightmap1,lightmap1);
	setFx(_ShaderLightmap2,lightmap2);
	setFx(_ShaderLightmap3,lightmap3);
	setFx(_ShaderLightmap4,lightmap4);
	setFx(_ShaderLightmap0Blend,lightmap0blend);
	setFx(_ShaderLightmap1Blend,lightmap1blend);
	setFx(_ShaderLightmap2Blend,lightmap2blend);
	setFx(_ShaderLightmap3Blend,lightmap3blend);
	setFx(_ShaderLightmap4Blend,lightmap4blend);
	setFx(_ShaderLightmap0X2,lightmap0_x2);
	setFx(_ShaderLightmap1X2,lightmap1_x2);
	setFx(_ShaderLightmap2X2,lightmap2_x2);
	setFx(_ShaderLightmap3X2,lightmap3_x2);
	setFx(_ShaderLightmap4X2,lightmap4_x2);
	setFx(_ShaderLightmap0BlendX2,lightmap0blend_x2);
	setFx(_ShaderLightmap1BlendX2,lightmap1blend_x2);
	setFx(_ShaderLightmap2BlendX2,lightmap2blend_x2);
	setFx(_ShaderLightmap3BlendX2,lightmap3blend_x2);
	setFx(_ShaderLightmap4BlendX2,lightmap4blend_x2);
	setFx(_ShaderCloud,cloud);
	setFx(_ShaderWaterNoDiffuse,water_no_diffuse);	
	setFx(_ShaderWaterDiffuse,water_diffuse);	
}


// ***************************************************************************

void CDriverD3D::releaseInternalShaders()
{
	H_AUTO_D3D(CDriverD3D_releaseInternalShaders)
	_ShaderLightmap0._DrvInfo.kill();
	_ShaderLightmap1._DrvInfo.kill();
	_ShaderLightmap2._DrvInfo.kill();
	_ShaderLightmap3._DrvInfo.kill();
	_ShaderLightmap4._DrvInfo.kill();
	_ShaderLightmap0Blend._DrvInfo.kill();
	_ShaderLightmap1Blend._DrvInfo.kill();
	_ShaderLightmap2Blend._DrvInfo.kill();
	_ShaderLightmap3Blend._DrvInfo.kill();
	_ShaderLightmap4Blend._DrvInfo.kill();
	_ShaderLightmap0X2._DrvInfo.kill();
	_ShaderLightmap1X2._DrvInfo.kill();
	_ShaderLightmap2X2._DrvInfo.kill();
	_ShaderLightmap3X2._DrvInfo.kill();
	_ShaderLightmap4X2._DrvInfo.kill();
	_ShaderLightmap0BlendX2._DrvInfo.kill();
	_ShaderLightmap1BlendX2._DrvInfo.kill();
	_ShaderLightmap2BlendX2._DrvInfo.kill();
	_ShaderLightmap3BlendX2._DrvInfo.kill();
	_ShaderLightmap4BlendX2._DrvInfo.kill();
	_ShaderCloud._DrvInfo.kill();
	_ShaderWaterNoDiffuse._DrvInfo.kill();	
	_ShaderWaterDiffuse._DrvInfo.kill();	
}

// ***************************************************************************

bool CDriverD3D::setShaderTexture (uint textureHandle, ITexture *texture)
{
	H_AUTO_D3D(CDriverD3D_setShaderTexture )
	// Setup the texture
	if (!setupTexture(*texture))
		return false;

	// Set the main texture
	nlassert (_CurrentShader);
	CShaderDrvInfosD3D *shaderInfo = static_cast<CShaderDrvInfosD3D*>((IShaderDrvInfos*)_CurrentShader->_DrvInfo);
	nlassert (shaderInfo);
	ID3DXEffect	*effect = shaderInfo->Effect;
	CTextureDrvInfosD3D *d3dtext = getTextureD3D (*texture);
	if (texture)
		effect->SetTexture (shaderInfo->TextureHandle[textureHandle], d3dtext->Texture);

	// Add a ref on this texture
	_CurrentShaderTextures.push_back (CTextureRef());
	CTextureRef &ref = _CurrentShaderTextures.back();
	ref.NeLTexture = texture;
	ref.D3DTexture = d3dtext->Texture;
	return true;
}

// ***************************************************************************

void CDriverD3D::disableHardwareTextureShader()
{
	// cannot disable pixel shader under DX, because it crashes with lightmap
	// => no-op
	/*
	_DisableHardwarePixelShader = true;
	_PixelShader = false;
	*/
}

// ***************************************************************************
//
/*
class CStateRecordLightEnable : public CStateRecord 
{ 
public:
	virtual void apply(class CDriverD3D &drv) 
	{ 
		drv.enableLight ((uint8)Index, Enable!=FALSE); 
	}
	DWORD Index;
	BOOL  Enable;
}
//
class CStateRecordLight : public CStateRecord
{
public:
	virtual void apply(class CDriverD3D &drv)
	{
		drv._LightCache[Index].Light = *pLight;
		touchRenderVariable (&_LightCache[Index]);
	}
	DWORD	  Index;
	D3DLIGHT9 Light;
};
//
class CStateRecordMaterial : public CStateRecord
{
public:
	virtual void apply(class CDriverD3D &drv) 
	{ 
		drv.setMaterialState(Material);
	}
	D3DMATERIAL9 Material;
};
//
class CStateRecordPixelShader : public CStateRecord
{
public:
	virtual void apply(class CDriverD3D &drv)
	{
		drv.setPixelShader(PixelShader);
	}
	LPDIRECT3DPIXELSHADER9 PixelShader;
};
//
class CStateRecordSetPixelShaderConstantB : public CStateRecord
{
public:
	virtual void apply(class CDriverD3D &drv)
	{
		const BOOL *curr = &Values[0];
		const BOOL *last = &Values[0] + Values.size();
		uint i = StartRegister;
		while (curr != last)		
		{
			drv.setPixelShaderConstant (i, curr);
			curr += 4;
			++ i;
		}
	}
	std::vector<BOOL> Values;
	uint			  StartRegister;
};
//
class CStateRecordPixelShaderConstantF : public CStateRecord
{
public:
	virtual void apply(class CDriverD3D &drv)
	{
		const FLOAT *curr = &Values[0];
		const FLOAT *last = &Values[0] + Values.size();
		uint i = StartRegister;
		while (curr != last)		
		{
			drv.setPixelShaderConstant (i, curr);
			curr += 4;
			++ i;
		}
	}
	std::vector<FLOAT> Values;
	uint			  StartRegister;
};
//
class CStateRecordPixelShaderConstantI : public CStateRecord
{
public:
	virtual void apply(class CDriverD3D &drv)
	{
		const INT *curr = &Values[0];
		const INT *last = &Values[0] + Values.size();
		uint i = StartRegister;
		while (curr != last)		
		{
			drv.setPixelShaderConstant (i, curr);
			curr += 4;
			++ i;
		}
	}
	std::vector<INT>  Values;
	uint			  StartRegister;
};
//
class CStateRecordRenderState : public CStateRecord
{
public:
	virtual void apply(class CDriverD3D &drv)
	{
		drv.setRenderState(State, Value);
	}
	D3DRENDERSTATETYPE State;
	DWORD Value;
};
//
class CStateRecordSamplerState : public CStateRecord
{
public:
	virtual void apply(class CDriverD3D &drv)
	{
		drv.setSamplerState(Sampler, Type, Value);
	}
	DWORD Sampler;
	D3DSAMPLERSTATETYPE Type;
	DWORD Value;
};
//
class CStateRecordTexture : public CStateRecord
{
public:
	virtual void apply(class CDriverD3D &drv)
	{
#error checker ça
	}
	DWORD Stage;
	LPDIRECT3DBASETEXTURE9 Texture;
};
//
class CStateRecordTextureStageState : public CStateRecord
{
public:
	virtual void apply(class CDriverD3D &drv)
	{
		if (Type == D3DTSS_TEXCOORDINDEX)
			drv.setTextureIndexUV (Stage, Value);
		else
			drv.setTextureState (Stage, Type, Value);
	}
	DWORD Stage;
	D3DTEXTURESTAGESTATETYPE Type;
	DWORD Value;
};
//
class CStateRecordTransform : public CStateRecord
{
public:
	virtual void apply(class CDriverD3D &drv)
	{
		drv.setMatrix(State, Matrix);
	}
	D3DTRANSFORMSTATETYPE State;
	D3DMATRIX Matrix;
};
//
class CStateRecordVertexShader : public CStateRecord
{
public:
	virtual void apply(class CDriverD3D &drv)
	{
		drv.setVertexProgram(Shader);
	}
	LPDIRECT3DVERTEXSHADER9 Shader;
};
class CStateRecordVertexShaderConstantB : public CStateRecord
{
public:
	virtual void apply(class CDriverD3D &drv)
	{
		const BOOL *curr = &Values[0];
		const BOOL *last = &Values[0] + Values.size();
		uint i = StartRegister;
		while (curr != last)		
		{
			drv.setVertexProgramconstant(i, curr);
			curr += 4;
			++ i;
		}
	}
	std::vector<BOOL> Values;
	uint			  StartRegister;
};
class CStateRecordVertexShaderConstantF : public CStateRecord
{
public:
	virtual void apply(class CDriverD3D &drv)
	{
		const FLOAT *curr = &Values[0];
		const FLOAT *last = &Values[0] + Values.size();
		uint i = StartRegister;
		while (curr != last)		
		{
			drv.setVertexProgramconstant(i, curr);
			curr += 4;
			++ i;
		}
	}
	std::vector<FLOAT> Values;
	uint			   StartRegister;
};
class CStateRecordVertexShaderConstantI : public CStateRecord
{
public:
	virtual void apply(class CDriverD3D &drv)
	{
		const INT *curr = &Values[0];
		const INT *last = &Values[0] + Values.size();
		uint i = StartRegister;
		while (curr != last)		
		{
			drv.setVertexProgramconstant(i, curr);
			curr += 4;
			++ i;
		}
	}
	std::vector<INT> Values;
	uint			   StartRegister;
};
*/

} // NL3D
