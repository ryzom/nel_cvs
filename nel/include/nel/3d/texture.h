/** \file texture.h
 * Interface ITexture
 *
 * $Id: texture.h,v 1.14 2000/12/13 10:25:22 berenguier Exp $
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
#include "nel/3d/font_generator.h"
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


//****************************************************************************


/**
 * CTextureFile
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2000
 */
class CTextureFile : public ITexture
{
	std::string _FileName;

public:

	/** 
	 * Default constructor
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	CTextureFile() { }


	/** 
	 * constructor
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	CTextureFile(std::string s) { _Touched=true; _FileName = s; } 


	/** 
	 * Set the name of the file containing the texture
	 * \param name of the file
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	void setFileName(std::string s) { _Touched=true; _FileName = s; }


	/** 
	 * get the name of the file containing the texture
	 * \return name of the file
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	std::string getFileName() const { return _FileName; } 


	/** 
	 * sharing system.
	 * \author Lionel Berenguier
	 * \date 2000
	 */	
	virtual bool			supportSharing() const {return true;}
	virtual std::string		getShareName() const {return getFileName();}


	/** 
	 * Generate the texture
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	void generate();

	/// Save the texture file name.
	virtual void	serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serial(_FileName);
		if(f.isReading())
			_Touched= true;
	}
	NLMISC_DECLARE_CLASS(CTextureFile);

};


//****************************************************************************


/**
 * CTextureMem
 * \author Cyril Corvazier
 * \author Nevrax France
 * \date 2000
 */
class CTextureMem : public ITexture
{
	uint8		*_Data;
	uint32		_Length;
	bool		_Delete;

public:

	/** 
	 * Default constructor
	 * \date 2000
	 */	
	CTextureMem() 
	{ 
		_Data=NULL;
		_Delete=false;
	}


	/** 
	 * Destructor
	 * \date 2000
	 */	
	virtual ~CTextureMem() 
	{ 
		if (_Data&&_Delete)
			delete [] _Data;
	}


	/** 
	 * constructor
	 * \param data Pointer of the file.
	 * \param _delete Is true if the class must delete the pointer.
	 * \date 2000
	 */	
	CTextureMem(uint8 *data, uint32 lenght, bool _delete) 
	{ 
		_Data=NULL;
		_Delete=false;
		setPointer(data, lenght, _delete); 
	}


	/** 
	 * Set the pointer of the mem file containing the texture
	 * \param data Pointer of the file.
	 * \param _delete Is true if the class must delete the pointer.
	 * \date 2000
	 */	
	void setPointer(uint8 *data, uint32 length, bool _delete) 
	{ 
		if (_Data&&_Delete)
			delete [] _Data;
		_Touched=true;
		_Data=data;
		_Length=length;
		_Delete=_delete;
	}


	/** 
	 * Get the Pointer of the memory file containing the texture
	 * \date 2000
	 */	
	uint8* getPointer() const { return _Data; } 


	/** 
	 * Get length of the memory file containing the texture
	 * \date 2000
	 */	
	uint32 getLength() const { return _Length; } 


	/** 
	 * Return true if the class handle the delete of the pointer.
	 * \date 2000
	 */	
	bool isDeletable() const { return _Delete; } 


	/** 
	 * Generate the texture
	 * \date 2000
	 */	
	void generate();


	/// Todo: serialize a mem texture.
	virtual void	serial(NLMISC::IStream &f) throw(NLMISC::EStream) {nlstop;}
	NLMISC_DECLARE_CLASS(CTextureMem);

};



//****************************************************************************


/**
 * CTextureFont
 *
 * Rq : _Width and _Height are duplicated from CBitmap because the texture
 *      can be released by the driver, which sets these values to zero.
 *		But these values are necessary to compute strings.
 *
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2000
 */
class CTextureFont : public ITexture
{
	uint32 _CharWidth;
	uint32 _CharHeight;
	uint32 _Width;
	uint32 _Height;

	uint32 _Size;
	CFontGenerator *_FontGen;
public:

	/** Default constructor
	 * 
	 */	
	CTextureFont() 
	{ 
		// Default char. This ctor is usefull for polymorphic serialisation only.
		Char = ' ';
		_Size = 10;
		_CharWidth = 0;
		_CharHeight = 0;
		_FontGen = NULL;
		_Width = 0;
		_Height = 0;
	}

	
	/** Default constructor
	 * 
	 */	
	CTextureFont(const CFontDescriptor& desc) 
	{ 
		Char = desc.C;
		_Size = desc.Size;
		_CharWidth = 0;
		_CharHeight = 0;
		_FontGen = desc.FontGen;
		_Width = 0;
		_Height = 0;
	}

	/** constructor
	 * 
	 */	
	CTextureFont(CFontGenerator *fg, ucchar c, uint32 size) 
	{ 
		Char = c;
		_Size = size;
		_CharWidth = 0;
		_CharHeight = 0;
		_FontGen = fg;
		_Width = 0;
		_Height = 0;
	}

	uint32	getCharWidth() const {return _CharWidth;}
	uint32	getCharHeight() const {return _CharHeight;}

	uint32	getWidth() const {return _Width;}
	uint32	getHeight() const {return _Height;}

	/** return the descriptor of this letter
	 * /return CFontDescriptor letter descriptor
	 */
	CFontDescriptor getDescriptor() const
	{
		return CFontDescriptor(_FontGen, Char, _Size);
	}

	// Generate the texture
	void generate();

	/// the unicode character
	ucchar Char;
	/// number of the character in the this font
	uint32 GlyphIndex;
	/// Distance between origin and top of the texture
	sint32 Top;
	/// Distance between origin and left of the texture
	sint32 Left;
	/// Advance to the next caracter
	sint32 AdvX;


	/// Todo: serialize a font texture.
	virtual void	serial(NLMISC::IStream &f) throw(NLMISC::EStream) {nlstop;}
	NLMISC_DECLARE_CLASS(CTextureFont);

};


//****************************************************************************


} // NL3D


#endif // NL_TEXTURE_H

/* End of texture.h */
