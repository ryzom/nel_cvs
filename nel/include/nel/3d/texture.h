/** \file texture.h
 * Interface ITexture
 *
 * $Id: texture.h,v 1.2 2000/11/09 16:16:42 coutelas Exp $
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
#include "nel/3d/bitmap.h"
#include <string>


namespace NL3D {


/*===================================================================*/

/**
 * Interface for textures
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2000
 */
class ITexture : public CBitmap
{
	bool _Releasable;

public:

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
	void release() { resize(0); }

};


/*===================================================================*/

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
	CTextureFile(std::string s) { _FileName = s; } 


	/** 
	 * Set the name of the file containing the texture
	 * \param name of the file
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	void setFileName(std::string s) { _FileName = s; }


	/** 
	 * get the name of the file containing the texture
	 * \return name of the file
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	std::string getFileName() const { return _FileName; } 


	/** 
	 * Generate the texture
	 * \return name of the file
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	void generate();

};


/*===================================================================*/


/**
 * CTextureFont
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2000
 */
class CTextureFont : public ITexture, public NLMISC::CRefCount
{
	uint32 _TrueWidth, _TrueHeight;

public:

	/** 
	 * Default constructor
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	CTextureFont() { }

	
	/** 
	 * Generate the texture
	 * \return name of the file
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	void generate();

};


/*===================================================================*/


} // NL3D


#endif // NL_TEXTURE_H

/* End of texture.h */
