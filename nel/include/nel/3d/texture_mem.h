/** \file texture_mem.h
 * <File description>
 *
 * $Id: texture_mem.h,v 1.1 2000/12/15 18:19:42 berenguier Exp $
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
#include "nel/3d/texture.h"


namespace NL3D 
{


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


} // NL3D


#endif // NL_TEXTURE_MEM_H

/* End of texture_mem.h */
