/** \file texture.h
 * Interface ITexture
 *
 * $Id: texture.h,v 1.15 2000/12/15 18:19:42 berenguier Exp $
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

#ifndef NL_TEXTURE_H
#define NL_TEXTURE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/stream.h"
#include "nel/3d/bitmap.h"
#include <string>


namespace NL3D 
{


//****************************************************************************
// Class for interaction of textures with Driver.
// ITextureDrvInfos represent the real data of the texture, stored into the driver (eg: just a GLint for opengl).
class ITextureDrvInfos : public NLMISC::CRefCount
{
private:
public:
			ITextureDrvInfos() {};
			ITextureDrvInfos(class IDriver& driver);
			virtual ~ITextureDrvInfos(void){ };
};

// Many ITexture may point to the same ITextureDrvInfos, through CTextureDrvShare.
class CTextureDrvShare : public NLMISC::CRefCount
{
public:
	NLMISC::CSmartPtr<ITextureDrvInfos>		DrvTexture;
};


//****************************************************************************
/**
 * Interface for textures
 * Sharing System note: The deriver may implement sharing system by implement supportSharing() and getShareName().
 * Such a texture may return a Unique Name for sharing. If the driver already has this texture, it will reuse it.
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2000
 */
class ITexture : public CBitmap, public NLMISC::CRefCount, public NLMISC::IStreamable
{
private:
	bool	_Releasable;

protected:
	// Derived texture should set it to true when they are updated.
	bool	_Touched;

public:
	// Private. For Driver Only.
	NLMISC::CRefPtr<CTextureDrvShare>	TextureDrvShare;

public:

	// Object.
	/// By default, a texture is releasable.
	ITexture() {_Touched= false; _Releasable= true;}
	/// see operator=.
	ITexture(const ITexture &tex) {_Touched= false; _Releasable= true; operator=(tex);}
	/// Need a virtual dtor.
	virtual ~ITexture();
	/// The operator= do not copy drv info, and set touched=true. _Releasable is copied.
	ITexture &operator=(const ITexture &tex);


	bool	touched(void) { return(_Touched); }
	void	clearTouched(void) { _Touched=0; }

	
	/** 
	 * Return whether texture can be released
	 * \return true if texture can be released, false else
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	bool getReleasable() const { return _Releasable; }


	/** 
	 * Set if texture can be released
	 * \param true if texture can be released, false else
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	void setReleasable(bool r) { _Releasable = r; }

	/** 
	 * Generate the texture
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	virtual void generate() = 0;


	/** 
	 * Release the texure (free memory)
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	void release() { reset(); }

	/** 
	 * Does this texture support sharing system.
	 * \author Lionel Berenguier
	 * \date 2000
	 */	
	virtual bool			supportSharing() const {return false;}

	/** 
	 * Return the Unique ident/name of the texture, used for Driver sharing caps.
	 * Deriver should add a prefix for their texture type. eg "file::pipoland", "noise::4-4-2" etc....
	 * \author Lionel Berenguier
	 * \date 2000
	 */	
	virtual std::string		getShareName() const {return std::string();}

	/** 
	 * Tells if the texture has been setuped by the driver.
	 * \author Lionel Berenguier
	 * \date 2000
	 */	
	bool	loadedIntoDriver() const {return TextureDrvShare!=NULL;}

};



} // NL3D


#endif // NL_TEXTURE_H

/* End of texture.h */
