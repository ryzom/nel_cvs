/** \file instance_material_user.h
 * <File description>
 *
 * $Id: instance_material_user.h,v 1.9 2002/10/10 13:03:28 berenguier Exp $
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
		nlassert(mat && mbi && asyncTextBlock);
		_MBI= mbi;
		_Material= mat;
		_AsyncTextureBlock= asyncTextBlock;
	}
	/// dtor
	virtual	~CInstanceMaterialUser()
	{
	}



	/// \name Modes.
	// @{
	virtual	bool				isLighted() const 
	{
		return _Material->isLighted();
	}
	virtual	bool				isUserColor() const 
	{
		return _Material->getShader()==CMaterial::UserColor;
	}
	// @}


	/// \name Lighted material mgt. Has effect only if isLighted().
	// @{

	virtual	void				setEmissive( CRGBA emissive=CRGBA(0,0,0) )
	{
		_Material->setEmissive(emissive);
	}
	virtual	void				setAmbient( CRGBA ambient=CRGBA(0,0,0) )
	{
		_Material->setAmbient( ambient);
	}
	virtual	void				setDiffuse( CRGBA diffuse=CRGBA(0,0,0) )
	{
		_Material->setDiffuse( diffuse);
	}
	virtual	void				setOpacity( uint8	opa )
	{
		_Material->setOpacity( opa );
	}
	virtual	void				setSpecular( CRGBA specular=CRGBA(0,0,0) )
	{
		_Material->setSpecular( specular);
	}
	virtual	void				setShininess( float shininess )
	{
		_Material->setShininess( shininess );
	}

	virtual	CRGBA				getEmissive() const 
	{
		return _Material->getEmissive();
	}
	virtual	CRGBA				getAmbient() const 
	{
		return _Material->getAmbient();
	}
	virtual	CRGBA				getDiffuse() const 
	{
		return _Material->getDiffuse();
	}
	virtual	uint8				getOpacity() const 
	{
		return _Material->getOpacity();
	}
	virtual	CRGBA				getSpecular() const 
	{
		return _Material->getSpecular();
	}
	virtual	float				getShininess() const 
	{
		return _Material->getShininess();
	}

	// @}


	/// \name UnLighted material mgt. Has effect only if !isLighted().
	// @{
	virtual	void				setColor(CRGBA rgba) 
	{
		_Material->setColor(rgba) ;
	}
	virtual	CRGBA				getColor(void) const 
	{
		return _Material->getColor();
	}
	// @}


	/// \name Texture UserColor. No effect if !isUserColor(). (getUserColor() return CRGBA(0,0,0,0))
	// @{
	virtual	void				setUserColor(CRGBA userColor)
	{
		if(isUserColor())
			_Material->setUserColor(userColor);
	}
	virtual	CRGBA				getUserColor() const 
	{
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
		if (stage >= IDRV_MAT_MAXTEXTURES)
		{
			nlwarning("UInstanceMaterialUser::setConstantColor : invalid stage");
			return;
		}
		_Material->texConstantColor(stage, color);
	}
	virtual NLMISC::CRGBA		getConstantColor(uint stage) const
	{
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


	virtual sint				getLastTextureStage() const
	{
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


private:
	CMeshBaseInstance	*_MBI;
	CMaterial			*_Material;
	CAsyncTextureBlock	*_AsyncTextureBlock;

public:

	// Internal Use only.
	CMaterial	*getMaterial() const {return _Material;}

};


} // NL3D


#endif // NL_INSTANCE_MATERIAL_USER_H

/* End of instance_material_user.h */
