/** \file font_manager.h
 * Font manager
 *
 * $Id: font_manager.h,v 1.4 2000/11/21 14:55:25 berenguier Exp $
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
#include "nel/misc/smart_ptr.h"
#include "nel/3d/texture.h"

#include <map>
#include <list>
#include <functional>




namespace NL3D {

class CTextureFont;
class CFontGenerator;
struct CDisplayDescriptor;
struct CComputedString;
struct CFontDescriptor;


typedef std::pair< std::list<NLMISC::CSmartPtr<CTextureFont> >::iterator,  uint32> pairRefPtrIntProut;
struct pairRefPtrInt : public pairRefPtrIntProut
{
	pairRefPtrInt (std::list<NLMISC::CSmartPtr<CTextureFont> >::iterator ite,  uint32 intt)
	{
		first=ite;
		second=intt;
	}
};
typedef std::map< CFontDescriptor , pairRefPtrInt> mapFontDec;


/**
 * Font manager
 * The font manager manages CTextureFont pointers through a list
 * of CSmartPtr. When the user asks for the texture font representing
 * a character(font/size), it generates and stores this pointer in the list. 
 * If this character has already been generated, and lies in the list, 
 * it increments its reference count.
 * If the memory used by generated textures exceeds the max memory, 
 * then the useless character/pointer is erased from the list. 
 * Max memory is set to 0 by default, so this value should be set to non-zero
 * before generating textures to prevent immediate memory deletion.
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2000
 */
class CFontManager 
{
	uint32 _MemSize;
	uint32 _MaxMemory;
	mapFontDec _Letters;
	std::list<NLMISC::CSmartPtr<CTextureFont> > _TextureFontList;

public:

	/** 
	 * Default constructor
	 */	
	CFontManager()
	{
		_MemSize = 0;
		_MaxMemory = 0;
	}


	/** 
	 * define maximum memory allowed
	 * \param maximum memory
	 */	
	void setMaxMemory(uint32 mem) { _MaxMemory = mem; }


	/** 
	 * gives maximum memory allowed
	 * \return maximum memory
	 */	
	uint32 getMaxMemory() const { return _MaxMemory; }


	/** 
	 * manages fonts in memory using CSmartPtr
	 * \param character descriptor
	 * \return CSmartPtr to a font texture
	 */	
	NLMISC::CSmartPtr<CTextureFont> getFontTexture(CFontDescriptor desc);


	/** 
	 * Compute primitive blocks and materials of each character of
	 * the string.
	 * \param s string to compute
	 * \param fontGen font generator
	 * \param color primitive blocks color
	 * \param fontSize font size
	 * \param desc display descriptor (screen size, font ratio)
	 * \param output computed string
	 */	
	void computeString (//const ucstring s, 
						const std::string& s, //
						CFontGenerator * fontGen, 
						NLMISC::CRGBA& color,
						uint32 fontSize, 
						const CDisplayDescriptor& desc, 
						CComputedString& output);

};



} // NL3D


#endif // NL_FONT_MANAGER_H

/* End of font_manager.h */



