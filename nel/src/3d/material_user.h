/** \file material_user.h
 * <File description>
 *
 * $Id: material_user.h,v 1.10 2004/04/26 17:15:43 corvazier Exp $
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

#ifndef NL_MATERIAL_USER_H
#define NL_MATERIAL_USER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/u_material.h"
#include "3d/material.h"
#include "3d/texture_user.h"


#define NL3D_MEM_MATERIAL						NL_ALLOC_CONTEXT( 3dMat )

namespace NL3D 
{


class UDriver;


// ***************************************************************************
/**
 * UMaterial implementation.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CMaterialUser : public UMaterial
{
protected:
	CMaterial		_Material;
	friend class	CDriverUser;

public:

	/// \name Object
	// @{
	CMaterialUser()
	{
		NL3D_MEM_MATERIAL
		_Material.initUnlit();
		// Enum assertion. User have access only to 6 first blend modes. Other are internals.
		nlassert((uint)UMaterial::blendCount==(uint)CMaterial::blendConstantColor);
		// Enum assertion.
		nlassert((uint)UMaterial::zfuncCount==(uint)CMaterial::zfuncCount);
	}
	virtual ~CMaterialUser()
	{
		NL3D_MEM_MATERIAL
	}
	// @}

	/// \name Texture.
	// @{
	virtual void 			setTexture(UTexture* ptex) 
	{
		setTexture (0, ptex);
	}

	virtual void 			setTexture(uint stage, UTexture* ptex) 
	{
		NL3D_MEM_MATERIAL
		CTextureUser	*text= dynamic_cast<CTextureUser*>(ptex);
		if (text != NULL)
		{
			_Material.setTexture (stage, text->getITexture());
		}
		else
		{
			_Material.setTexture (stage, NULL);
		}
		// NB: _Material smartpoint to this ITexture. But this is correct because so does CTextureUser.
	}

	virtual bool			texturePresent() 
	{
		return texturePresent (0);
	}

	virtual bool			texturePresent (uint stage) 
	{
		NL3D_MEM_MATERIAL
		return _Material.texturePresent (stage);
	}

	virtual void			selectTextureSet(uint id)
	{
		NL3D_MEM_MATERIAL
		_Material.selectTextureSet(id);
	}

	// @}


	/// \name Blending.
	// @{
	virtual void			setBlend(bool active) 
	{
		NL3D_MEM_MATERIAL
		_Material.setBlend(active);
	}
	virtual void			setBlendFunc(TBlend src, TBlend dst) 
	{
		NL3D_MEM_MATERIAL
		_Material.setBlendFunc((CMaterial::TBlend)(uint32)src, (CMaterial::TBlend)(uint32)dst);
	}
	virtual void			setSrcBlend(TBlend val) 
	{
		NL3D_MEM_MATERIAL
		_Material.setSrcBlend((CMaterial::TBlend)(uint32)val);
	}
	virtual void			setDstBlend(TBlend val) 
	{
		NL3D_MEM_MATERIAL
		_Material.setDstBlend((CMaterial::TBlend)(uint32)val);
	}

	virtual bool			getBlend() const 
	{
		NL3D_MEM_MATERIAL
		return _Material.getBlend();
	}
	virtual TBlend			getSrcBlend(void)  const 
	{
		NL3D_MEM_MATERIAL
		return (UMaterial::TBlend)(uint32)_Material.getSrcBlend();
	}
	virtual TBlend			getDstBlend(void)  const 
	{
		NL3D_MEM_MATERIAL
		return (UMaterial::TBlend)(uint32)_Material.getDstBlend();
	}
	// @}

	/// \name Texture environnement.
	// @{

	virtual void texEnvOpRGB(uint stage, TTexOperator ope)
	{
		NL3D_MEM_MATERIAL
		_Material.texEnvOpRGB(stage, (CMaterial::TTexOperator)(uint32)ope);
	}
	
	virtual void texEnvArg0RGB (uint stage, TTexSource src, TTexOperand oper)
	{
		NL3D_MEM_MATERIAL
		_Material.texEnvArg0RGB (stage, (CMaterial::TTexSource)(uint32)src, (CMaterial::TTexOperand)(uint32)oper);
	}
	
	virtual void texEnvArg1RGB (uint stage, TTexSource src, TTexOperand oper)
	{
		NL3D_MEM_MATERIAL
		_Material.texEnvArg1RGB (stage, (CMaterial::TTexSource)(uint32)src, (CMaterial::TTexOperand)(uint32)oper);
	}

	virtual void texEnvArg2RGB (uint stage, TTexSource src, TTexOperand oper)
	{
		NL3D_MEM_MATERIAL
			_Material.texEnvArg2RGB (stage, (CMaterial::TTexSource)(uint32)src, (CMaterial::TTexOperand)(uint32)oper);
	}
	
	virtual void texEnvOpAlpha(uint stage, TTexOperator ope)
	{
		NL3D_MEM_MATERIAL
		_Material.texEnvOpAlpha (stage, (CMaterial::TTexOperator)(uint32)ope);
	}
	
	virtual void texEnvArg0Alpha(uint stage, TTexSource src, TTexOperand oper)
	{
		NL3D_MEM_MATERIAL
		_Material.texEnvArg0Alpha (stage, (CMaterial::TTexSource)(uint32)src, (CMaterial::TTexOperand)(uint32)oper);
	}
	
	virtual void texEnvArg1Alpha(uint stage, TTexSource src, TTexOperand oper)
	{
		NL3D_MEM_MATERIAL
		_Material.texEnvArg1Alpha (stage, (CMaterial::TTexSource)(uint32)src, (CMaterial::TTexOperand)(uint32)oper);
	}

	virtual void texEnvArg2Alpha(uint stage, TTexSource src, TTexOperand oper)
	{
		NL3D_MEM_MATERIAL
			_Material.texEnvArg2Alpha (stage, (CMaterial::TTexSource)(uint32)src, (CMaterial::TTexOperand)(uint32)oper);
	}
	
	// @}

	/// \name ZBuffer.
	// @{
	virtual void			setZFunc(ZFunc val) 
	{
		NL3D_MEM_MATERIAL
		_Material.setZFunc((CMaterial::ZFunc)(uint32) val);
	}
	virtual void			setZWrite(bool active) 
	{
		NL3D_MEM_MATERIAL
		_Material.setZWrite(active);
	}
	virtual void			setZBias(float val) 
	{
		_Material.setZBias(val);
	}

	virtual ZFunc			getZFunc(void)  const  
	{
		NL3D_MEM_MATERIAL
		return (UMaterial::ZFunc)(uint32)_Material.getZFunc();
	}
	virtual bool			getZWrite(void)  const 
	{
		NL3D_MEM_MATERIAL
		return _Material.getZWrite();
	}
	virtual float			getZBias(void)  const  
	{
		NL3D_MEM_MATERIAL
		return _Material.getZBias();
	}
	// @}

	virtual void			setAlphaTest(bool active);
	virtual bool			getAlphaTest() const;
	virtual void			setAlphaTestThreshold(float threshold);
	virtual float			getAlphaTestThreshold() const;

	/// \name Color/Lighting..
	// @{
	/// The Color is used only if lighting is disabled. Also, color is replaced by per vertex color (if any).
	virtual void			setColor(CRGBA rgba) 
	{
		NL3D_MEM_MATERIAL
		_Material.setColor(rgba);
	}

	virtual CRGBA			getColor(void) const 
	{
		NL3D_MEM_MATERIAL
		return _Material.getColor();
	}
	// @}

	/// \name Culling
	// @{
		virtual void			setDoubleSided(bool doubleSided = true)
		{
			NL3D_MEM_MATERIAL
			_Material.setDoubleSided(doubleSided);
		}
		virtual bool			getDoubleSided() const
		{
			NL3D_MEM_MATERIAL
			return _Material.getDoubleSided();
		}
	// @}

	/// \name Misc
	// @{
	/** Init the material as unlit. normal shader, no lighting ....
	 * Default to: normal shader, no lighting, color to White(1,1,1,1), no texture, ZBias=0, ZFunc= lessequal, ZWrite==true, no blend.
	 * All other states are undefined (such as blend function, since blend is disabled).
	 */
	virtual void			initUnlit()
	{
		NL3D_MEM_MATERIAL
		_Material.initUnlit();
	}
	// @}

	virtual	bool			isSupportedByDriver(UDriver &drv);
	

public:

	// Internal Use only.
	CMaterial	&getMaterial() 
	{
		NL3D_MEM_MATERIAL
		return _Material;
	}

};


} // NL3D


#endif // NL_MATERIAL_USER_H

/* End of material_user.h */
