/** \file font_manager.cpp
 * <File description>
 *
 * $Id: font_manager.cpp,v 1.16 2001/01/09 14:55:20 berenguier Exp $
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
#include "nel/3d/texture_font.h"
#include "nel/3d/computed_string.h"
#include "nel/3d/primitive_block.h"
#include "nel/3d/material.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/debug.h"

#include "nel/misc/file.h"


namespace NL3D {



/*------------------------------------------------------------------*\
							getFontMaterial()
\*------------------------------------------------------------------*/
NLMISC::CSmartPtr<CMaterial> CFontManager::getFontMaterial(CFontDescriptor desc)
{
	NLMISC::CSmartPtr<CMaterial>	pMatFont;
	NLMISC::CSmartPtr<CTextureFont> pTexFont;

	mapFontDec::iterator ifont = _Letters.find(desc);
	
	if (ifont != _Letters.end())
	{
		// if the letter is already in the list

		// moving the CMaterial to the begining of the list
		pMatFont = *(ifont->second.first);
		_MaterialFontList.erase(ifont->second.first);

		// put at the begining of the priority list
		_MaterialFontList.push_front(pMatFont);

		// and updating iterator in the map
		ifont->second.first = _MaterialFontList.begin();
	}
	else
	{
		// the letter isn't yet in the list

		// creating new CMaterial and adding it at the begining of the list
		pTexFont = new CTextureFont(desc);

		// generating texture
		pTexFont->generate();
		
		// Eval mem Size
		int nMemSize = pTexFont->getWidth()*pTexFont->getHeight()*4; 
			
		// adding TexTure to list
		pMatFont= new CMaterial;
		pMatFont->initUnlit();
		pMatFont->setSrcBlend(CMaterial::srcalpha);
		pMatFont->setDstBlend(CMaterial::invsrcalpha);
		pMatFont->setBlend(true);
		pMatFont->setTexture(0, pTexFont);

		_MaterialFontList.push_front(pMatFont);

		// Add to global mem size
		_MemSize += nMemSize;

		// updating iterator in the map
		_Letters.insert ( mapFontDec::value_type (desc, pairRefPtrInt(_MaterialFontList.begin(), nMemSize)));

	}


	// while memory used is too high, we pop the back of the list
	while(_MemSize>_MaxMemory && _MaterialFontList.size()!=0)
	{
		NLMISC::CSmartPtr<CMaterial> pMatFontBack = _MaterialFontList.back();
		pTexFont= (CTextureFont*)(pMatFontBack->getTexture(0));
		CFontDescriptor descBack = 	pTexFont->getDescriptor();
		
		// Find the desc to kill in the map
		mapFontDec::iterator ite=_Letters.find (descBack);
		
		// must exist!!
		nlassert (ite!=_Letters.end());

		// Free mem
		_MemSize -= ite->second.second;

		// Erase it
		_Letters.erase (ite);

		// Unstack
		_MaterialFontList.pop_back();
	}

	return pMatFont;
}



/**
 * This function compute a basic_string
 *
 */
template  <class T> static void NL3DcomputeString (CFontManager *fm, const std::basic_string<T, std::char_traits<T>, std::allocator<T> > &s,
				CFontGenerator *fontGen,
				const NLMISC::CRGBA &color,
				uint32 fontSize,
				IDriver *driver,
				CComputedString &output)
{
	uint32 width, height;
	driver->getWindowSize (width, height);
	float FontRatio = 1.0;

	// keep the 800*600 ratio
	fontSize = fontSize*width/800;
	
	// Setting vertices format
	output.Vertices.setVertexFormat(IDRV_VF_XYZ | IDRV_VF_COLOR | IDRV_VF_UV[0]);
	output.Vertices.setNumVertices(4 * s.size());
	
	output.Primitives.resize(s.size());
	output.Materials.resize(s.size());
	
	// 1 character <-> 1 primitive block
	sint32 penx = 0;
	sint32 penz = 0;
	float x, z;
	output.StringHeight = 0;
	for(uint i=0; i<s.size(); i++)
	{
		// Creating font
		CSmartPtr<CMaterial> pMatFont = fm->getFontMaterial(CFontDescriptor(fontGen,s[i],fontSize));
		CTextureFont		*pTexFont = (CTextureFont*)(pMatFont->getTexture(0));
		
		// Creating vertices
		sint32 dx = pTexFont->Left;
		sint32 dz = -((sint32)pTexFont->getCharHeight()-(sint32)(pTexFont->Top));
		float	um= (float)pTexFont->getCharWidth()/pTexFont->getWidth();
		float	vm= (float)pTexFont->getCharHeight()/pTexFont->getHeight();

		if (pTexFont->getWidth() > 0 && pTexFont->getHeight() > 0)
		{
			x = (penx + dx) * FontRatio;
			z = (penz + dz) * FontRatio;
			x/= height;
			z/= height;
			output.Vertices.setVertexCoord(4*i, x, 0, z);
			output.Vertices.setTexCoord(4*i,0,0,vm);
			output.Vertices.setColor(4*i, color);

			x = (penx + dx + (sint32)pTexFont->getCharWidth()) * FontRatio;
			z = (penz + dz) * FontRatio;
			x/= height;
			z/= height;
			output.Vertices.setVertexCoord(4*i+1, x, 0, z);
			output.Vertices.setTexCoord(4*i+1,0,um,vm);
			output.Vertices.setColor(4*i+1, color);

			x = (penx + dx + (sint32)pTexFont->getCharWidth()) * FontRatio;
			z = (penz + dz + (sint32)pTexFont->getCharHeight()) * FontRatio;
			x/= height;
			z/= height;
			output.Vertices.setVertexCoord(4*i+2, x, 0, z); 
			output.Vertices.setTexCoord(4*i+2,0,um,0);
			output.Vertices.setColor(4*i+2, color);

			x = (penx + dx) * FontRatio;
			z = (penz + dz + (sint32)pTexFont->getCharHeight()) * FontRatio;
			x/= height;
			z/= height;
			output.Vertices.setVertexCoord(4*i+3, x, 0, z); 
			output.Vertices.setTexCoord(4*i+3,0,0,0);
			output.Vertices.setColor(4*i+3, color);
			
			if(z>output.StringHeight) output.StringHeight = z;
		}
		penx += pTexFont->AdvX;
		
		// Building primitive block
		output.Primitives[i].setNumQuad(1);
		output.Primitives[i].setQuad(0, 4*i, 4*i+1, 4*i+2, 4*i+3);

		// Building Material
		output.Materials[i]= pMatFont;
		
	}

	output.StringWidth = penx * FontRatio;
	output.StringWidth /= height;
}



/*------------------------------------------------------------------*\
							computeString()
\*------------------------------------------------------------------*/
void CFontManager::computeString (const std::string &s,
								  CFontGenerator *fontGen,
								  const NLMISC::CRGBA &color,
								  uint32 fontSize,
								  IDriver *driver,
								  CComputedString &output)
{
	NL3DcomputeString (this, s, fontGen, color, fontSize, driver, output);
}


/*------------------------------------------------------------------*\
							computeString()
\*------------------------------------------------------------------*/
void CFontManager::computeString (const ucstring &s,
								  CFontGenerator *fontGen,
								  const NLMISC::CRGBA &color,
								  uint32 fontSize,
								  IDriver *driver,
								  CComputedString &output)
{
	NL3DcomputeString (this, s, fontGen, color, fontSize, driver, output);
}








} // NL3D
