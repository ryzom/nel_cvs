/** \file instance_material_user.h
 * <File description>
 *
 * $Id: instance_material_user.h,v 1.13 2003/11/06 09:29:09 besson Exp $
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

#ifndef NL_INSTANCE_MATERIAL_USER_H
#define NL_INSTANCE_MATERIAL_USER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/u_instance_material.h"
#include "3d/material.h"
#include "3d/texture_file.h"
#include "3d/texture_multi_file.h"

#define NL3D_MEM_MATERIAL_INSTANCE						NL_ALLOC_CONTEXT( 3dMatI )

namespace NL3D
{


class CMeshBaseInstance;
class CInstanceUser;
class CAsyncTextureBlock;


// ***************************************************************************
/**
 * UInstanceMaterial implementation.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CInstanceMaterialUser : public UInstanceMaterial
{
public:
	/// Constructor
	CInstanceMaterialUser(CMeshBaseInstance	*mbi, CMaterial *mat, CAsyncTextureBlock *asyncTextBlock)
	{
		NL3D_MEM_MATERIAL_INSTANCE
		nlassert(mat && mbi && asyncTextBlock);
		_MBI= mbi;
		_Material= mat;
		_AsyncTextureBlock= asyncTextBlock;
	}
	/// dtor
	virtual	~CInstanceMaterialUser()
	{
		NL3D_MEM_MATERIAL_INSTANCE
	}



	/// \name Modes.
	// @{
	virtual	bool				isLighted() const 
	{
		NL3D_MEM_MATERIAL_INSTANCE
		return _Material->isLighted();
	}
	virtual	bool				isUserColor() const 
	{
		NL3D_MEM_MATERIAL_INSTANCE
		return _Material->getShader()==CMaterial::UserColor;
	}
	// @}


	/// \name Lighted material mgt. Has effect only if isLighted().
	// @{

	virtual	void				setEmissive( CRGBA emissive=CRGBA(0,0,0) )
	{
		NL3D_MEM_MATERIAL_INSTANCE
		_Material->setEmissive(emissive);
	}
	virtual	void				setAmbient( CRGBA ambient=CRGBA(0,0,0) )
	{
		NL3D_MEM_MATERIAL_INSTANCE
		_Material->setAmbient( ambient);
	}
	virtual	void				setDiffuse( CRGBA diffuse=CRGBA(0,0,0) )
	{
		NL3D_MEM_MATERIAL_INSTANCE
		_Material->setDiffuse( diffuse);
	}
	virtual	void				setOpacity( uint8	opa )
	{
		NL3D_MEM_MATERIAL_INSTANCE
		_Material->setOpacity( opa );
	}
	virtual	void				setSpecular( CRGBA specular=CRGBA(0,0,0) )
	{
		NL3D_MEM_MATERIAL_INSTANCE
		_Material->setSpecular( specular);
	}
	virtual	void				setShininess( float shininess )
	{
		NL3D_MEM_MATERIAL_INSTANCE
		_Material->setShininess( shininess );
	}

	virtual	CRGBA				getEmissive() const 
	{
		NL3D_MEM_MATERIAL_INSTANCE
		return _Material->getEmissive();
	}
	virtual	CRGBA				getAmbient() const 
	{
		NL3D_MEM_MATERIAL_INSTANCE
		return _Material->getAmbient();
	}
	virtual	CRGBA				getDiffuse() const 
	{
		NL3D_MEM_MATERIAL_INSTANCE
		return _Material->getDiffuse();
	}
	virtual	uint8				getOpacity() const 
	{
		NL3D_MEM_MATERIAL_INSTANCE
		return _Material->getOpacity();
	}
	virtual	CRGBA				getSpecular() const 
	{
		NL3D_MEM_MATERIAL_INSTANCE
		return _Material->getSpecular();
	}
	virtual	float				getShininess() const 
	{
		NL3D_MEM_MATERIAL_INSTANCE
		return _Material->getShininess();
	}

	// @}


	/// \name UnLighted material mgt. Has effect only if !isLighted().
	// @{
	virtual	void				setColor(CRGBA rgba) 
	{
		NL3D_MEM_MATERIAL_INSTANCE
		_Material->setColor(rgba) ;
	}
	virtual	CRGBA				getColor(void) const 
	{
		NL3D_MEM_MATERIAL_INSTANCE
		return _Material->getColor();
	}
	// @}


	/// \name Texture UserColor. No effect if !isUserColor(). (getUserColor() return CRGBA(0,0,0,0))
	// @{
	virtual	void				setUserColor(CRGBA userColor)
	{
		NL3D_MEM_MATERIAL_INSTANCE
		if(isUserColor())
			_Material->setUserColor(userColor);
	}
	virtual	CRGBA				getUserColor() const 
	{
		NL3D_MEM_MATERIAL_INSTANCE
		if(isUserColor())
			return _Material->getUserColor();
		else
			return CRGBA(0,0,0,0);
	}
	// @}

	/// \name constantColor
	// @{
	virtual void				setConstantColor(uint stage, NLMISC::CRGBA color)
	{
		NL3D_MEM_MATERIAL_INSTANCE
		if (stage >= IDRV_MAT_MAXTEXTURES)
		{
			nlwarning("UInstanceMaterialUser::setConstantColor : invalid stage");
			return;
		}
		_Material->texConstantColor(stage, color);
	}
	virtual NLMISC::CRGBA		getConstantColor(uint stage) const
	{
		NL3D_MEM_MATERIAL_INSTANCE
		if (stage >= IDRV_MAT_MAXTEXTURES)
		{
			nlwarning("UInstanceMaterialUser::getConstantColor : invalid stage");
			return NLMISC::CRGBA::Black;
		}
		return _Material->getTexConstantColor(stage);

	}
	// @}

	/// \name Texture files specific
	// @{	
	virtual bool				isTextureFile(uint stage = 0) const;
	virtual std::string getTextureFileName(uint stage = 0) const;
	virtual void				setTextureFileName(const std::string &fileName, uint stage = 0);
	// @}

	virtual void				emptyTexture(uint stage = 0);


	virtual sint				getLastTextureStage() const
	{
		NL3D_MEM_MATERIAL_INSTANCE
		sint lastStage = -1;
		for(uint k = 0; k < IDRV_MAT_MAXTEXTURES; ++k)
		{
			if (_Material->getTexture(k) != NULL)
			{
				lastStage = k;
			}
		}
		return lastStage;
	}

	/// \name Blending.
	// @{
	virtual void			setBlend(bool active) 
	{
		NL3D_MEM_MATERIAL_INSTANCE
		_Material->setBlend(active);
	}
	virtual void			setBlendFunc(TBlend src, TBlend dst) 
	{
		NL3D_MEM_MATERIAL_INSTANCE
		_Material->setBlendFunc((CMaterial::TBlend)(uint32)src, (CMaterial::TBlend)(uint32)dst);
	}
	virtual void			setSrcBlend(TBlend val) 
	{
		NL3D_MEM_MATERIAL_INSTANCE
		_Material->setSrcBlend((CMaterial::TBlend)(uint32)val);
	}
	virtual void			setDstBlend(TBlend val) 
	{
		NL3D_MEM_MATERIAL_INSTANCE
		_Material->setDstBlend((CMaterial::TBlend)(uint32)val);
	}
	virtual void			setAlphaTestThreshold(float at)
	{
		NL3D_MEM_MATERIAL_INSTANCE
		_Material->setAlphaTestThreshold(at);
	}

	virtual bool			getBlend() const 
	{
		NL3D_MEM_MATERIAL_INSTANCE
		return _Material->getBlend();
	}
	virtual TBlend			getSrcBlend(void)  const 
	{
		NL3D_MEM_MATERIAL_INSTANCE
		return (UInstanceMaterial::TBlend)(uint32)_Material->getSrcBlend();
	}
	virtual TBlend			getDstBlend(void)  const 
	{
		NL3D_MEM_MATERIAL_INSTANCE
		return (UInstanceMaterial::TBlend)(uint32)_Material->getDstBlend();
	}
	// @}



	/// \name Texture matrix
	// @{
	virtual void                    enableUserTexMat(uint stage, bool enabled = true)
	{
		NL3D_MEM_MATERIAL_INSTANCE
		if (stage >= IDRV_MAT_MAXTEXTURES)
		{
			nlwarning("UInstanceMaterial::enableUserTexMat : stage %d is invalid", stage);
			return;
		}
		_Material->enableUserTexMat(stage, enabled);
	}
	virtual bool                    isUserTexMatEnabled(uint stage) const
	{
		NL3D_MEM_MATERIAL_INSTANCE
		if (stage >= IDRV_MAT_MAXTEXTURES)
		{
			nlwarning("UInstanceMaterial::enableUserTexMat : stage %d is invalid", stage);
			return false;
		}
		return _Material->isUserTexMatEnabled(stage);
	}
	virtual void					setUserTexMat(uint stage, const NLMISC::CMatrix &m)
	{
		NL3D_MEM_MATERIAL_INSTANCE
		if (stage >= IDRV_MAT_MAXTEXTURES)
		{
			nlwarning("UInstanceMaterial::enableUserTexMat : stage %d is invalid", stage);
			return;
		}
		if (!_Material->isUserTexMatEnabled(stage))
		{
			nlwarning("UInstanceMaterial::setUserTexMat : texture stage %d has no user matrix.", stage);
		}
		_Material->setUserTexMat(stage, m);
	}
	virtual const NLMISC::CMatrix  &getUserTexMat(uint stage) const
	{
		NL3D_MEM_MATERIAL_INSTANCE
		if (stage >= IDRV_MAT_MAXTEXTURES)
		{
			nlwarning("UInstanceMaterial::enableUserTexMat : stage %d is invalid", stage);
			return CMatrix::Identity;
		}
		if (!_Material->isUserTexMatEnabled(stage))
		{
			nlwarning("UInstanceMaterial::setUserTexMat : texture stage %d has no user matrix.", stage);
			return CMatrix::Identity;
		}
		return _Material->getUserTexMat(stage);
	}
	// @}

private:
	CMeshBaseInstance	*_MBI;
	CMaterial			*_Material;
	CAsyncTextureBlock	*_AsyncTextureBlock;

public:

	// Internal Use only.
	CMaterial	*getMaterial() const 
	{
		NL3D_MEM_MATERIAL_INSTANCE
		return _Material;
	}

};


} // NL3D


#endif // NL_INSTANCE_MATERIAL_USER_H

/* End of instance_material_user.h */
