/** \file texture_file.h
 * TODO: File description
 *
 * $Id: texture_file.h,v 1.15.16.1 2005/03/31 19:33:28 berenguier Exp $
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

#ifndef NL_TEXTURE_FILE_H
#define NL_TEXTURE_FILE_H

#include "nel/misc/types_nl.h"
#include "3d/texture.h"


namespace NL3D
{


//****************************************************************************
/**
 * CTextureFile
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2000
 */
class CTextureFile : public ITexture
{
public:
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/
	

	/** 
	 * Default constructor
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	CTextureFile() { _AllowDegradation=true; _SupportSharing= true; _EnlargeCanvasNonPOW2Tex = false; _MipMapSkipAtLoad=0; }

	// copy ctor
	CTextureFile(const CTextureFile &other);

	// assignment operator
	CTextureFile &operator = (const CTextureFile &other);

	/** 
	 * constructor
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	CTextureFile(const std::string &s) 
	{ 
		/* ***********************************************
		 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
		 *	It can be loaded/called through CAsyncFileManager for instance
		 * ***********************************************/
		
		touch(); _FileName = s; 
		_AllowDegradation=true;
		_SupportSharing= true; 
		_EnlargeCanvasNonPOW2Tex = false;
		_MipMapSkipAtLoad=0;
	} 


	/** 
	 * Set the name of the file containing the texture
	 * \param name of the file
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	void setFileName(const std::string &s) 
	{ 
		touch(); 
		_FileName = s; 
	}


	/** 
	 * get the name of the file containing the texture
	 * \return name of the file
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	const std::string		&getFileName() const { return _FileName; } 


	/// texture file may allow the driver to degrade (default is true).
	virtual bool	allowDegradation() const { return _AllowDegradation; }
	/// Change the degradation mode. NB: this does not touch() the ITexture... This state is serialized.
	void			setAllowDegradation(bool allow);


	/** 
	 * sharing system.
	 * \author Lionel Berenguier
	 * \date 2000
	 */	
	virtual bool			supportSharing() const {return _SupportSharing;}

	virtual std::string		getShareName() const;
	// User can disable the sharing system. Default is to be enabled. Not serialized
	void					enableSharing(bool enable);


	/** 
	 * Generate the texture, looking in CPath if necessary.
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	void doGenerate(bool async = false);

	/// Save the texture file name.
	virtual void	serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	NLMISC_DECLARE_CLASS(CTextureFile);


	//// Used to fill a bitmap by reading a file, looking in CPath if necessary, and using user_color
	static void buildBitmapFromFile(NLMISC::CBitmap &dest, const std::string &fileName, bool asyncload, uint8 mipMapSkip=0, bool enlargeCanvasNonPOW2Tex = false);


	/// If the file is a DDS texture with mipmap, skip the first skipLod mipmaps (0 by default) at loading
	void			setMipMapSkipAtLoad(uint8 level);
	uint8			getMipMapSkipAtLoad() const {return _MipMapSkipAtLoad;}

	/** Flag that tell that textures that have dimension that are not power of 2 are snapped to the top-left corner of a power-of-2 sized texture
	 *	Default is false. Any texture that are non power of 2 are considered as a bug and thus a DummyTexture is displayed
	 *	NB: if true, CBitmap::getWidth() and CBitmap::getHeight() will return the new enlarged size
	 */
	void			setEnlargeCanvasNonPOW2Tex(bool enlarge) { _EnlargeCanvasNonPOW2Tex =  enlarge; }
	bool			getEnlargeCanvasNonPOW2Tex() const { return _EnlargeCanvasNonPOW2Tex; }

private:
	std::string _FileName;
	bool		_AllowDegradation;	// Default is true.
	bool		_SupportSharing;	// Default is true.
	bool        _EnlargeCanvasNonPOW2Tex;    // Non power of 2 textures are cropped. Default is false
	uint8		_MipMapSkipAtLoad;	// Default is 0.
private:
	void		dupInfo(const CTextureFile &other);
};






} // NL3D


#endif // NL_TEXTURE_FILE_H

/* End of texture_file.h */
