/** \file texture_user.h
 * <File description>
 *
 * $Id: texture_user.h,v 1.1 2001/06/15 16:24:45 corvazier Exp $
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

#ifndef NL_TEXTURE_USER_H
#define NL_TEXTURE_USER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/u_texture.h"
#include "3d/texture.h"
#include "3d/texture_file.h"


namespace NL3D 
{


using	NLMISC::CSmartPtr;


// ***************************************************************************
/**
 * UTexture implementation. This Texture must keep a SmartPtr on the real ITexture. Because CMaterial attach texture 
 * with SmartPtr too!!
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CTextureUser : virtual public UTexture
{
protected:
	CSmartPtr<ITexture>		_Texture;

public:

	/// \name Object
	// @{
	/// This ctpor takes a freshly created texture!! UTexture will owns this texture (via smartptr).
	CTextureUser(ITexture	*text)
	{
		nlassert(text);
		_Texture=text;

		// UTexture and ITexture MUST be the same!
		nlassert((uint)UTexture::WrapModeCount==(uint)ITexture::WrapModeCount);
		nlassert((uint)UTexture::UploadFormatCount==(uint)ITexture::UploadFormatCount);
		nlassert((uint)UTexture::MagFilterCount==(uint)ITexture::MagFilterCount);
		nlassert((uint)UTexture::MinFilterCount==(uint)ITexture::MinFilterCount);
	}
	virtual ~CTextureUser()
	{
		// texture auto deleted.
	}
	// @}

	virtual	void			setWrapS(TWrapMode mode) 
	{
		_Texture->setWrapS((ITexture::TWrapMode)(uint32)mode);
	}
	virtual	void			setWrapT(TWrapMode mode) 
	{
		_Texture->setWrapT((ITexture::TWrapMode)(uint32)mode);
	}
	virtual	TWrapMode		getWrapS() const 
	{
		return (UTexture::TWrapMode)(uint32)_Texture->getWrapS();
	}
	virtual	TWrapMode		getWrapT() const 
	{
		return (UTexture::TWrapMode)(uint32)_Texture->getWrapT();
	}
	virtual	void			setUploadFormat(TUploadFormat pf) 
	{
		_Texture->setUploadFormat((ITexture::TUploadFormat)(uint32)pf);
	}
	virtual	TUploadFormat	getUploadFormat() const 
	{
		return (UTexture::TUploadFormat)(uint32)_Texture->getUploadFormat();
	}
	virtual	void			setFilterMode(TMagFilter magf, TMinFilter minf) 
	{
		_Texture->setFilterMode((ITexture::TMagFilter)(uint32)magf, (ITexture::TMinFilter)(uint32)minf);
	}
	virtual	TMagFilter		getMagFilter() const 
	{
		return (UTexture::TMagFilter)(uint32)_Texture->getMagFilter();
	}
	virtual	TMinFilter		getMinFilter() const 
	{
		return (UTexture::TMinFilter)(uint32)_Texture->getMinFilter();
	}
	virtual	bool			mipMapOff() const 
	{
		return _Texture->mipMapOff();
	}
	virtual	bool			mipMapOn() const 
	{
		return _Texture->mipMapOn();
	}


	/// Accessor for UMaterial imp.
	ITexture*				getITexture() {return _Texture;}

};


// ***************************************************************************
/**
 * UTextureFile implementation.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class	CTextureFileUser : virtual public UTextureFile, public CTextureUser
{
public:


	/// \name Object
	// @{
	CTextureFileUser() : CTextureUser(new CTextureFile)
	{
	}
	CTextureFileUser(std::string s) : CTextureUser(new CTextureFile(s))
	{
	}
	virtual ~CTextureFileUser()
	{
	}
	// @}


	virtual	void setFileName(std::string s)
	{
		((CTextureFile*)(ITexture*)_Texture)->setFileName(s);
	}

	virtual	std::string getFileName() const
	{
		return ((CTextureFile*)(ITexture*)_Texture)->getFileName();
	}

};


// ***************************************************************************
/**
 * UTextureRaw implementation.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class	CTextureRawUser : virtual public UTextureRaw, public CTextureUser
{
public:

	/// TODO_TEXTURERAW
	CTextureRawUser() : CTextureUser(NULL)
	{
	}
};


} // NL3D


#endif // NL_TEXTURE_USER_H

/* End of texture_user.h */
