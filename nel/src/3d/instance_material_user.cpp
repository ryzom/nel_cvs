/** \file instance_material_user.cpp
 * <File description>
 *
 * $Id: instance_material_user.cpp,v 1.9 2004/04/09 14:31:36 vizerie Exp $
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

#include "std3d.h"

#include "3d/instance_material_user.h"
#include "3d/async_texture_block.h"
#include "3d/mesh_base_instance.h"
#include "3d/driver.h"
#include "3d/texture_mem.h"


namespace NL3D
{


// ***************************************************************************
bool				CInstanceMaterialUser::isTextureFile(uint stage) const
{
	NL3D_MEM_MATERIAL_INSTANCE
	if (stage >= IDRV_MAT_MAXTEXTURES)
	{
		nlwarning("UInstanceMaterialUser::isTextureFile : invalid stage");
		return false;
	}
	return dynamic_cast<CTextureFile *>(_Material->getTexture(stage)) != NULL;
}

// ***************************************************************************
std::string			CInstanceMaterialUser::getTextureFileName(uint stage) const
{		
	NL3D_MEM_MATERIAL_INSTANCE
	if (stage >= IDRV_MAT_MAXTEXTURES)
	{
		nlwarning("UInstanceMaterialUser::getTextureFileName : invalid stage");
		return "";
	}

	// If Async mode
	if(_MBI->getAsyncTextureMode())
	{
		nlassert(_AsyncTextureBlock->isTextureFile(stage));
		// return name of the async one.
		return _AsyncTextureBlock->TextureNames[stage];
	}
	else
	{
		// return the name in the material
		return NLMISC::safe_cast<CTextureFile *>(_Material->getTexture(stage))->getFileName();
	}
}

// ***************************************************************************
void				CInstanceMaterialUser::setTextureFileName(const std::string &fileName, uint stage)
{
	NL3D_MEM_MATERIAL_INSTANCE
	if (stage >= IDRV_MAT_MAXTEXTURES)
	{
		nlwarning("UInstanceMaterialUser::setTextureFileName : invalid stage");
		return;
	}

	// If Async mode
	if(_MBI->getAsyncTextureMode())
	{
		if(!_AsyncTextureBlock->isTextureFile(stage))
		{
			nlwarning("UInstanceMaterialUser::setTextureFileName : the texture is not a texture file");
			return;
		}
		// replace the fileName
		_AsyncTextureBlock->TextureNames[stage]= fileName;
		// Flag the instance.
		_MBI->setAsyncTextureDirty(true);
	}
	else
	{
		CTextureFile *otherTex = dynamic_cast<CTextureFile *>(_Material->getTexture(stage));
		if (!otherTex)
		{
			nlwarning("UInstanceMaterialUser::setTextureFileName : the texture is not a texture file");
			return;
		}
		CTextureFile *tf = new CTextureFile(*otherTex);
		tf->setFileName(fileName);
		NLMISC::CSmartPtr<ITexture> old = _Material->getTexture(stage);
		_Material->setTexture(stage, tf);
	}
}

// ***************************************************************************
void CInstanceMaterialUser::emptyTexture(uint stage /*=0*/)
{
	NL3D_MEM_MATERIAL_INSTANCE
	if (stage >= IDRV_MAT_MAXTEXTURES)
	{
		nlwarning("UInstanceMaterialUser::emptyTexture : invalid stage");
		return;
	}
	_Material->setTexture(stage, NULL);
}

// ***************************************************************************
bool CInstanceMaterialUser::isSupportedByDriver(UDriver &drv)
{	
	IDriver *idrv = NLMISC::safe_cast<CDriverUser *>(&drv)->getDriver();
	return _Material->isSupportedByDriver(*idrv);	
}

// ***************************************************************************
void CInstanceMaterialUser::setTextureMem(uint stage, uint8 *data, uint32 length, bool _delete, bool isFile /*=true*/, uint width /*=0*/, uint height /*=0*/, CBitmap::TType texType /*=CBitmap::RGBA*/)
{
	if (stage >= IDRV_MAT_MAXTEXTURES)
	{
		nlwarning("UInstanceMaterialUser::emptyTexture : invalid stage");
		return;
	}	
	_Material->setTexture((uint8) stage, new CTextureMem(data, length, _delete, isFile, width, height, texType));
}





} // NL3D
