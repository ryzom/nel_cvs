/** \file font_manager.h
 * Font manager
 *
 * $Id: font_manager.h,v 1.2 2000/11/10 15:20:17 coutelas Exp $
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

#ifndef NL_FONT_MANAGER_H
#define NL_FONT_MANAGER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/texture.h"
#include <map>
#include <list>
#include <functional>

namespace NLMISC {

template <class T> class CRefPtr;

}



namespace NL3D {

class CTextureFont;
struct CFontDescriptor;



typedef std::pair< std::list<NLMISC::CRefPtr<CTextureFont> >::iterator,  uint32> pairRefPtrInt;
typedef std::map< CFontDescriptor , pairRefPtrInt> mapFontDec;



/**
 * Font manager
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2000
 */
class CFontManager 
{
	uint32 _MemSize;
	uint32 _MaxMemory;
	mapFontDec _Letters;
	std::list<NLMISC::CRefPtr<CTextureFont> > _TextureFontList;

public:

	/** 
	 * Default constructor
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	CFontManager()
	{
		_MemSize = 0;
		_MaxMemory = 0;
	}


	/** 
	 * define maximum memory allowed
	 * \param maximum memory
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	void setMaxMemory(uint32 mem) { _MaxMemory = mem; }


	/** 
	 * gives maximum memory allowed
	 * \return maximum memory
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	uint32 getMaxMemory() const { return _MaxMemory; }


	/** 
	 * manages fonts in memory using CRefPtr
	 * \param character descriptor
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	void getFontTexture(CFontDescriptor desc);


};


} // NL3D


#endif // NL_FONT_MANAGER_H

/* End of font_manager.h */



