/** \file texture_mem.h
 * <File description>
 *
 * $Id: texture_mem.h,v 1.5 2002/02/21 17:38:16 vizerie Exp $
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

#ifndef NL_TEXTURE_MEM_H
#define NL_TEXTURE_MEM_H

#include "nel/misc/types_nl.h"
#include "3d/texture.h"
#include <string>


namespace NL3D 
{


//****************************************************************************
/**
 * CTextureMem
 */
class CTextureMem : public ITexture
{
	uint8		*_Data;
	uint32		_Length;
	bool		_Delete;
	bool        _IsFile;
	std::string _ShareName;

	/// keep size for textures that aren't from a file
	uint		_TexWidth, _TexHeight;
public:

	/** 
	 * Default constructor
	 */	
	CTextureMem() 
	{ 
		_Data=NULL;
		_Delete=false;
	}


	/** 
	 * Destructor
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
	 * \param isFile is true if the data must be interpreted as a texture file. Otherwise, it is interpreted
	 *        as the raw datas of the texture, so the format and size of the texture must also have been set to match
	 *        the raw datas 
	 * \param width used only if isFile is set to false
	 * \param height used only if isFile is set to false
	 */	
	CTextureMem(uint8 *data, uint32 length, bool _delete, bool isFile = true, uint width = 0, uint height = 0) 
	{ 
		_Data=NULL;
		_Delete=false;
		_IsFile = isFile;
		setPointer(data, length, _delete, isFile, width, height);
		_TexWidth = width;
		_TexHeight = height;
	}


	/** 
	 * Set the pointer of the mem file containing the texture
	 * \param data Pointer of the file.
	 * \param _delete Is true if the class must delete the pointer.
	 */	
	void setPointer(uint8 *data, uint32 length, bool _delete, bool isFile = true, uint width = 0, uint height = 0) 
	{ 
		if (_Data&&_Delete)
			delete [] _Data;
		_Touched=true;
		_Data=data;
		_Length=length;
		_Delete=_delete;
		_IsFile = isFile;
		_TexWidth = width;
		_TexHeight = height;
	}



	/** 
	 * Get the Pointer of the memory file containing the texture
	 */	
	uint8* getPointer() const { return _Data; } 


	/** 
	 * Get length of the memory file containing the texture
	 */	
	uint32 getLength() const { return _Length; } 


	/** 
	 * Return true if the class handle the delete of the pointer.
	 */	
	bool isDeletable() const { return _Delete; } 


	/** 
	 * Generate the texture
	 */	
	void doGenerate();

	/// inherited from ITexture
	virtual bool			supportSharing() const 
	{
		return !_ShareName.empty();
	}

	/// inherited from ITexture
	virtual std::string		getShareName() const 
	{
		nlassert(!_ShareName.empty());
		return _ShareName;
	}

	/// Make this texture sharable by assigning it a share name. An empty share name means it is not sharable, which is the default
	void setShareName(const std::string &shareName) 
	{ 
		_ShareName = shareName;
	}

	/// Todo: serialize a mem texture.
	virtual void	serial(NLMISC::IStream &f) throw(NLMISC::EStream) {nlstop;}
	NLMISC_DECLARE_CLASS(CTextureMem);

	/** This create a white square texture of 1x1
	 *  It can be used to force the activation of a texture stage with no texture.
	 */
	static ITexture *Create1x1WhiteTex();	

};


} // NL3D


#endif // NL_TEXTURE_MEM_H

/* End of texture_mem.h */
