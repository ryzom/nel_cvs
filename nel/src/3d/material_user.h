/** \file material_user.h
 * <File description>
 *
 * $Id: material_user.h,v 1.5 2002/07/18 17:41:13 vizerie Exp $
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


namespace NL3D 
{


class	CDriverUser;


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
		_Material.initUnlit();
		// Enum assertion. User have access only to 6 first blend modes. Other are internals.
		nlassert((uint)UMaterial::blendCount==(uint)CMaterial::blendConstantColor);
		// Enum assertion.
		nlassert((uint)UMaterial::zfuncCount==(uint)CMaterial::zfuncCount);
	}
	virtual ~CMaterialUser()
	{
	}
	// @}

	/// \name Texture.
	// @{
	virtual void 			setTexture(UTexture* ptex) 
	{
		CTextureUser	*text= dynamic_cast<CTextureUser*>(ptex);
		if (text != NULL)
		{
			_Material.setTexture (0, text->getITexture());
		}
		else
		{
			_Material.setTexture (0, NULL);
		}
		// NB: _Material smartpoint to this ITexture. But this is correct because so does CTextureUser.
	}

	virtual bool			texturePresent() 
	{
		return _Material.texturePresent(0);
	}

	virtual void			selectTextureSet(uint id)
	{
		_Material.selectTextureSet(id);
	}

	// @}


	/// \name Blending.
	// @{
	virtual void			setBlend(bool active) 
	{
		_Material.setBlend(active);
	}
	virtual void			setBlendFunc(TBlend src, TBlend dst) 
	{
		_Material.setBlendFunc((CMaterial::TBlend)(uint32)src, (CMaterial::TBlend)(uint32)dst);
	}
	virtual void			setSrcBlend(TBlend val) 
	{
		_Material.setSrcBlend((CMaterial::TBlend)(uint32)val);
	}
	virtual void			setDstBlend(TBlend val) 
	{
		_Material.setDstBlend((CMaterial::TBlend)(uint32)val);
	}

	virtual bool			getBlend() const 
	{
		return _Material.getBlend();
	}
	virtual TBlend			getSrcBlend(void)  const 
	{
		return (UMaterial::TBlend)(uint32)_Material.getSrcBlend();
	}
	virtual TBlend			getDstBlend(void)  const 
	{
		return (UMaterial::TBlend)(uint32)_Material.getDstBlend();
	}
	// @}


	/// \name ZBuffer.
	// @{
	virtual void			setZFunc(ZFunc val) 
	{
		_Material.setZFunc((CMaterial::ZFunc)(uint32) val);
	}
	virtual void			setZWrite(bool active) 
	{
		_Material.setZWrite(active);
	}
	virtual void			setZBias(float val) 
	{
		_Material.setZBias(val);
	}

	virtual ZFunc			getZFunc(void)  const  
	{
		return (UMaterial::ZFunc)(uint32)_Material.getZFunc();
	}
	virtual bool			getZWrite(void)  const 
	{
		return _Material.getZWrite();
	}
	virtual float			getZBias(void)  const  
	{
		return _Material.getZBias();
	}
	// @}


	/// \name Color/Lighting..
	// @{
	/// The Color is used only if lighting is disabled. Also, color is replaced by per vertex color (if any).
	virtual void			setColor(CRGBA rgba) 
	{
		_Material.setColor(rgba);
	}

	virtual CRGBA			getColor(void) const 
	{
		return _Material.getColor();
	}
	// @}

	/// \name Culling
	// @{
		virtual void			setDoubleSided(bool doubleSided = true)
		{
			_Material.setDoubleSided(doubleSided);
		}
		virtual bool			getDoubleSided() const
		{
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
		_Material.initUnlit();
	}
	// @}

};


} // NL3D


#endif // NL_MATERIAL_USER_H

/* End of material_user.h */
