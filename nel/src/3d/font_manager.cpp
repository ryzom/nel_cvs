/** \file font_manager.cpp
 * <File description>
 *
 * $Id: font_manager.cpp,v 1.2 2000/11/10 15:19:41 coutelas Exp $
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

#include "nel/3d/font_manager.h"
#include "nel/3d/font_generator.h"
#include "nel/3d/texture.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/debug.h"


namespace NL3D {




/*------------------------------------------------------------------*\
							getFontTexture()
\*------------------------------------------------------------------*/
void CFontManager::getFontTexture(CFontDescriptor desc)
{
	
	mapFontDec::iterator ifont = _Letters.find(desc);
	
	if (ifont != _Letters.end())
	{
		// if the letter is already in the list

		// moving the CTextureFont to the begining of the list
		NLMISC::CRefPtr<CTextureFont> backupRef = *(ifont->second.first);
		_TextureFontList.erase(ifont->second.first);

		// put at the begining of the priority list
		_TextureFontList.push_front(backupRef);

		// and updating iterator in the map
		ifont->second.first = _TextureFontList.begin();
	}
	else
	{
		// the letter isn't yet in the list

		// creating new CTextureFont and adding it at the begining of the list
		NLMISC::CRefPtr<CTextureFont> pTexFont;
		pTexFont = new CTextureFont(desc);
		
		// Eval mem Size
		int nMemSize = pTexFont->getWidth()*pTexFont->getHeight()*4; // accurate/sufficiant ??
			
		// adding TexTure to list
		_TextureFontList.push_front(pTexFont);

		// Add to global mem size
		_MemSize += nMemSize;

		// updating iterator in the map
		_Letters.insert ( mapFontDec::value_type (desc, pairRefPtrInt(_TextureFontList.begin(), nMemSize)));
	}


	// while memory used is too high, we pop the back of the list
	while(_MemSize>_MaxMemory && _TextureFontList.size()!=0)
	{
		NLMISC::CRefPtr<CTextureFont> pTexFontBack = _TextureFontList.back();
		CFontDescriptor descBack = pTexFontBack->getDescriptor();
		
		// Find the desc to kill in the map
		mapFontDec::iterator ite=_Letters.find (descBack);
		
		// must exist!!
		nlassert (ite!=_Letters.end());

		// Free mem
		_MemSize -= ite->second.second;

		// Erase it
		_Letters.erase (ite);

		// Unstack
		_TextureFontList.pop_back();
	}
	
}


} // NL3D
