/** \file font_manager.cpp
 * <File description>
 *
 * $Id: font_manager.cpp,v 1.36 2002/11/21 15:55:55 berenguier Exp $
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

#include "std3d.h"

#include <string>
#include <sstream>

#include "3d/font_manager.h"
#include "3d/font_generator.h"
#include "3d/texture_font.h"
#include "3d/computed_string.h"
#include "3d/primitive_block.h"
#include "3d/material.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/debug.h"

#include "nel/misc/file.h"

using namespace std;

namespace NL3D {



// ***************************************************************************
inline CMaterial* CFontManager::getFontMaterial()
{
	if (_TexFont == NULL)
	{
		_TexFont = new CTextureFont;
	}

	if (_MatFont == NULL)
	{
		_MatFont= new CMaterial;
		_MatFont->initUnlit();
		_MatFont->setSrcBlend(CMaterial::srcalpha);
		_MatFont->setDstBlend(CMaterial::invsrcalpha);
		_MatFont->setBlend(true);
		_MatFont->setTexture(0, _TexFont);
		_MatFont->texEnvOpRGB(0, CMaterial::Replace);
		_MatFont->texEnvArg0RGB(0, CMaterial::Diffuse, CMaterial::SrcColor);
	}
	return _MatFont;
}


// ***************************************************************************
void CFontManager::computeString (const std::string &s,
								  CFontGenerator *fontGen,
								  const NLMISC::CRGBA &color,
								  uint32 fontSize,
								  IDriver *driver,
								  CComputedString &output,
								  bool	keep800x600Ratio)
{
	// static to avoid reallocation
	static ucstring	ucs;
	ucs= s;
	computeString(ucs, fontGen, color, fontSize, driver, output, keep800x600Ratio);
}


// ***************************************************************************
void CFontManager::computeString (const ucstring &s,
								  CFontGenerator *fontGen,
								  const NLMISC::CRGBA &color,
								  uint32 fontSize,
								  IDriver *driver,
								  CComputedString &output,
								  bool	keep800x600Ratio)
{
	uint32 width, height;

	output.Color = color;
	driver->getWindowSize (width, height);
	if ((height == 0) || (width == 0))
		return;

	// resize fontSize if window not of 800x600.
	if (keep800x600Ratio)
	{
		// keep the 800*600 ratio
		fontSize = (uint32)floor(fontSize*height/600.f);
		fontSize = max(fontSize, (uint32)2);
	}
	
	// Setting vertices format
	output.Vertices.setNumVertices (4 * s.size());
	
	// 1 character <-> 1 quad
	sint32 penx = 0, dx;
	sint32 penz = 0, dz;
	float x1, z1, x2, z2;
	float u1, v1, u2, v2;
	CMaterial		*pMatFont = getFontMaterial();
	CTextureFont	*pTexFont = (CTextureFont*)(pMatFont->getTexture (0));
	float TexRatioW = 1.0f / pTexFont->getWidth();
	float TexRatioH = 1.0f / pTexFont->getHeight();
	/*float hlfPixTexW = 0.5f * TexRatioW;
	float hlfPixTexH = 0.5f * TexRatioH;
	float hlfPixScrW = 0.5f;
	float hlfPixScrH = 0.5f;*/
	// Yoyo: Do not need Half Pixel/Texel displacement!!
	float hlfPixTexW = 0;
	float hlfPixTexH = 0;
	float hlfPixScrW = 0;
	float hlfPixScrH = 0;


	CTextureFont::SLetterKey k;

	// string info.
	sint32 nMaxZ = -(sint32)height*2, nMinZ = height*2;
	output.StringHeight = 0;

	// For all chats
	uint j = 0;
	for (uint i = 0; i < s.size(); i++)
	{
		// Creating font
		k.Char = s[i];
		k.FontGenerator = fontGen;
		k.Size = fontSize;
		CTextureFont::SLetterInfo *pLI = pTexFont->getLetterInfo (k);
		if(pLI != NULL)
		{
			if ((pLI->CharWidth > 0) && (pLI->CharHeight > 0))
			{
				// Creating vertices
				dx = pLI->Left;
				dz = -((sint32)pLI->CharHeight-(sint32)(pLI->Top));
				u1 = pLI->U - hlfPixTexW;
				v1 = pLI->V - hlfPixTexH;
				u2 = pLI->U + ((float)pLI->CharWidth) * TexRatioW + hlfPixTexW;
				v2 = pLI->V + ((float)pLI->CharHeight) * TexRatioH + hlfPixTexH;

				x1 = (penx + dx) - hlfPixScrW;
				z1 = (penz + dz) - hlfPixScrH;
				x2 = (penx + dx + (sint32)pLI->CharWidth)  + hlfPixScrW;
				z2 = (penz + dz + (sint32)pLI->CharHeight) + hlfPixScrH;

				output.Vertices.setVertexCoord	(j, x1, 0, z1);
				output.Vertices.setTexCoord		(j, 0, u1, v2);
				++j;

				output.Vertices.setVertexCoord	(j, x2, 0, z1);
				output.Vertices.setTexCoord		(j, 0, u2, v2);
				++j;

				output.Vertices.setVertexCoord	(j, x2, 0, z2); 
				output.Vertices.setTexCoord		(j, 0, u2, v1);
				++j;

				output.Vertices.setVertexCoord	(j, x1, 0, z2); 
				output.Vertices.setTexCoord		(j, 0, u1, v1);
				++j;
				
				// String info
				sint32	nZ1 = (sint32)pLI->Top-(sint32)pLI->CharHeight;
				sint32	nZ2 = pLI->Top;
				
				if (nZ1 < nMinZ) nMinZ = nZ1;
				if (nZ2 > nMaxZ) nMaxZ = nZ2;
			}
			penx += pLI->AdvX;
		}

		// Building Material
		output.Material = pMatFont;
	}
	output.Vertices.setNumVertices (j);

	// compile string info
	output.StringWidth = penx;
	if(nMaxZ>nMinZ)
	{
		output.StringHeight = (nMaxZ - nMinZ);
		output.StringLine = -nMinZ;
	}
	else
	{
		output.StringHeight = 0;
		output.StringLine = 0;
	}
}


// ***************************************************************************
void CFontManager::computeStringInfo (	const ucstring &s,
										CFontGenerator *fontGen,
										const NLMISC::CRGBA &color,
										uint32 fontSize,
										IDriver *driver,
										CComputedString &output,
										bool keep800x600Ratio	)
{
	uint32 width, height;

	output.Color = color;
	driver->getWindowSize (width, height);
	if ((height == 0) || (width == 0))
		return;

	// resize fontSize if window not of 800x600.
	if (keep800x600Ratio)
	{
		// keep the 800*600 ratio
		fontSize = (uint32)floor(fontSize*height/600.f);
		fontSize = max(fontSize, (uint32)2);
	}
	
	sint32 penx = 0;
	sint32 nMaxZ = -(sint32)height*2, nMinZ = height*2;
	CMaterial		*pMatFont = getFontMaterial();
	CTextureFont	*pTexFont = (CTextureFont*)(pMatFont->getTexture (0));

	CTextureFont::SLetterKey k;
	CTextureFont::SLetterInfo *pLI;

	for (uint i = 0; i < s.size(); i++)
	{
		// Creating font
		k.Char = s[i];
		k.FontGenerator = fontGen;
		k.Size = fontSize;
		pLI = pTexFont->getLetterInfo (k);
		if(pLI != NULL)
		{
			if ((pLI->CharWidth > 0) && (pLI->CharHeight > 0))
			{
				// String info
				sint32	nZ1 = (sint32)pLI->Top-(sint32)pLI->CharHeight;
				sint32	nZ2 = pLI->Top;
				
				if (nZ1 < nMinZ) nMinZ = nZ1;
				if (nZ2 > nMaxZ) nMaxZ = nZ2;
			}
			penx += pLI->AdvX;
		}
	}

	// compile string info
	output.StringWidth = penx;
	if(nMaxZ>nMinZ)
	{
		output.StringHeight = (nMaxZ - nMinZ);
		output.StringLine = -nMinZ;
	}
	else
	{
		output.StringHeight = 0;
		output.StringLine = 0;
	}

}


// ***************************************************************************
string CFontManager::getCacheInformation() const
{
	stringstream ss;

	ss << "MaxMemory: " << (uint) _MaxMemory << " MemSize: " << (uint) _MemSize << " NbChar: " << (uint) _NbChar;

	return ss.str();
}





} // NL3D
