/** \file texture_user.h
 * <File description>
 *
 * $Id: texture_user.h,v 1.7 2004/05/26 17:55:01 vizerie Exp $
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

#define NL3D_MEM_TEXTURE						NL_ALLOC_CONTEXT( 3dTextu )

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
		NL3D_MEM_TEXTURE
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
		NL3D_MEM_TEXTURE
		// texture auto deleted.
	}
	// @}

	virtual	void			setWrapS(TWrapMode mode) 
	{
		NL3D_MEM_TEXTURE
		_Texture->setWrapS((ITexture::TWrapMode)(uint32)mode);
	}
	virtual	void			setWrapT(TWrapMode mode) 
	{
		NL3D_MEM_TEXTURE
		_Texture->setWrapT((ITexture::TWrapMode)(uint32)mode);
	}
	virtual	TWrapMode		getWrapS() const 
	{
		NL3D_MEM_TEXTURE
		return (UTexture::TWrapMode)(uint32)_Texture->getWrapS();
	}
	virtual	TWrapMode		getWrapT() const 
	{
		NL3D_MEM_TEXTURE
		return (UTexture::TWrapMode)(uint32)_Texture->getWrapT();
	}
	virtual	void			setUploadFormat(TUploadFormat pf) 
	{
		NL3D_MEM_TEXTURE
		_Texture->setUploadFormat((ITexture::TUploadFormat)(uint32)pf);
	}
	virtual	TUploadFormat	getUploadFormat() const 
	{
		NL3D_MEM_TEXTURE
		return (UTexture::TUploadFormat)(uint32)_Texture->getUploadFormat();
	}
	virtual	void			setFilterMode(TMagFilter magf, TMinFilter minf) 
	{
		NL3D_MEM_TEXTURE
		_Texture->setFilterMode((ITexture::TMagFilter)(uint32)magf, (ITexture::TMinFilter)(uint32)minf);
	}
	virtual	TMagFilter		getMagFilter() const 
	{
		NL3D_MEM_TEXTURE
		return (UTexture::TMagFilter)(uint32)_Texture->getMagFilter();
	}
	virtual	TMinFilter		getMinFilter() const 
	{
		NL3D_MEM_TEXTURE
		return (UTexture::TMinFilter)(uint32)_Texture->getMinFilter();
	}
	virtual	bool			mipMapOff() const 
	{
		NL3D_MEM_TEXTURE
		return _Texture->mipMapOff();
	}
	virtual	bool			mipMapOn() const 
	{
		NL3D_MEM_TEXTURE
		return _Texture->mipMapOn();
	}
	virtual	NLMISC::CRGBA	getPixelColor(sint32 x, sint32 y) const
	{
		NL3D_MEM_TEXTURE
		return _Texture->getPixelColor(x,y);
	}
	virtual	void setReleasable(bool bReleasable)
	{
		NL3D_MEM_TEXTURE
		_Texture->setReleasable(bReleasable);
	}

	/// Accessor for UMaterial imp.
	ITexture*				getITexture() 
	{
		NL3D_MEM_TEXTURE
		return _Texture;
	}

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
		((CTextureFile*)(ITexture*)_Texture)->setAllowDegradation(false);
	}
	CTextureFileUser(std::string s) : CTextureUser(new CTextureFile(s))
	{
		((CTextureFile*)(ITexture*)_Texture)->setAllowDegradation(false);
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

	virtual bool	allowDegradation() const
	{
		return _Texture->allowDegradation();
	}
	virtual void	setAllowDegradation(bool allow)
	{
		((CTextureFile*)(ITexture*)_Texture)->setAllowDegradation(allow);
	}
	virtual void	setDontStretchNonPOW2Tex(bool dontStretch) 
	{ 
		((CTextureFile*)(ITexture*)_Texture)->setDontStretchNonPOW2Tex(dontStretch); 
	}
	virtual bool	getDontStretchNonPOW2Tex() const 
	{ 
		return ((CTextureFile*)(ITexture*)_Texture)->getDontStretchNonPOW2Tex(); 
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

	/// \todo yoyo: TODO_TEXTURERAW
	CTextureRawUser() : CTextureUser(NULL)
	{
	}
};


} // NL3D


#endif // NL_TEXTURE_USER_H

/* End of texture_user.h */
