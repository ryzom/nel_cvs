/** \file texture.h
 * Interface ITexture
 *
 * $Id: texture.h,v 1.17 2000/12/22 13:17:09 corvazier Exp $
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
#include "nel/misc/rect.h"
#include "nel/3d/bitmap.h"
#include <string>
#include <list>


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
 *
 * *** IMPORTANT ********************
 * *** IF YOU MODIFY THE STRUCTURE OF THIS CLASS, PLEASE INCREMENT IDriver::InterfaceVersion TO INVALIDATE OLD DRIVER DLL
 * **********************************
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
	bool				_Touched;

	/**
	 *  List of invalided rectangle. If the list is empty, generate() will rebuild all the texture.
     *
	 * \see isAllInvalidated(), generate(), touch(), touchRect(), touched()
	 */
	std::list<NLMISC::CRect>	_ListInvalidRect;

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


	/**
	 *  This method return the touched flag. If it is true, the driver will call generate to rebuild the texture.
     *
	 * \see isAllInvalidated(), generate(), touch(), touchRect(), _ListInvalidRect
	 */
	bool	touched (void)
	{
		return _Touched;
	}

	/**
	 *  This method invalidates all the texture surface. When the driver calls generate, the
	 *  texture will rebuild all the texture and the driver will update it.
     *
	 * \see isAllInvalidated(), generate(), touchRect(), touched(), _ListInvalidRect
	 */
	void	touch (NLMISC::CRect& rect) 
	{ 
		_ListInvalidRect.clear (); 
		_Touched=true; 
	}

	/**
	 *  This method invalidates a rectangle of the texture surface. When the driver calls generate, the
	 *  texture could rebuild only this part of texture and the driver will update only those rectangles.
     *
	 * \see isAllInvalidated(), generate(), touch(), touched(), _ListInvalidRect
	 */
	void	touchRect(NLMISC::CRect& rect) 
	{ 
		// Don't invalidate the rectangle if the full texture is already invalidate
		if (!isAllInvalidated ())
		{
			// Add the region to invalidate list
			_ListInvalidRect.push_back (rect); 
			// Touch flag
			_Touched=true; 
		}
	}

	/*
	 * Clear the touched flag and the invalid rectangle list
	 *
	 * \see isAllInvalidated(), generate(), touch(), touched(), touchRect(), _ListInvalidRect
	 */
	void	clearTouched(void) 
	{ 
		_Touched=0; 
		_ListInvalidRect.clear();
	}
	
	/** 
	 * Return whether texture can be released. If it returns true, the driver will release the texture
	 * after generate it and upload it into the videomemory by calling release(). If it returns false,
	 * the driver won't release the texture.
	 *
	 * \return true if texture can be released, false else
	 * \see setReleasable(), generate()
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	bool getReleasable() const { return _Releasable; }


	/** 
	 * Set if texture can be released
	 * If it is true, the driver will release the texture after generating it and upload it into the 
	 * videomemory by calling release(). If it is false, the driver won't release the texture.
     *
	 * \see getReleasable(), generate()
	 * \param true if texture can be released, false else
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	void setReleasable(bool r) { _Releasable = r; }

	/** 
	 * Generate the texture pixels.
	 * 
	 * This method is called by the driver when it needs to generate pixels of the texture. If the 
	 * texture is used for the first time or if it is touched, the driver will call this method.
	 * For exemple, a texture file will load the bitmap in this method.
	 *
	 * If the invalidate rect list is empty, generate() must rebuild all the texture.
	 * If the invalidate rect list is not empty, generate() rebuilds only the invalidate rectangles 
	 * in the list.
	 *
	 * Don't clear the touch flag or the invalid rectangle list until updating the texture in generate(). 
	 * It's the generate()'s caller jobs.
	 *
	 * After generation, if the texture is releasable, the driver will release the texture by calling
	 * release(). 
	 *
	 * \see isAllInvalidated(), touch(), touched(), touchRect(), clearTouched(), _ListInvalidRect
	 * \see getReleasable(), setReleasable()
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
	bool	loadedIntoDriver() const 
	{
		return TextureDrvShare!=NULL;
	}

protected:
	/** 
	 * Return true if ALL the texture is invalidate, else return false.
	 * \author Cyril Corvazier
	 * \date 2000
	 */	
	bool					isAllInvalidated () const
	{
		return  _Touched&&(_ListInvalidRect.begin()==_ListInvalidRect.end());
	}
};



} // NL3D


#endif // NL_TEXTURE_H

/* End of texture.h */
