/** \file font_manager.cpp
 * <File description>
 *
 * $Id: font_manager.cpp,v 1.35 2002/09/11 13:51:26 besson Exp $
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



/*------------------------------------------------------------------*\
							getFontMaterial()
\*------------------------------------------------------------------*/
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



/**
 * This function compute a basic_string
 *
 */
/*
template  <class T> static void NL3DcomputeString (CFontManager *fm, const std::basic_string<T> &s,
				CFontGenerator *fontGen,
				const NLMISC::CRGBA &color,
				uint32 fontSize,
				IDriver *driver,
				CComputedString &output,
				bool	keep800x600Ratio)
{

	float FontRatio = 1.0;
	uint32 width, height;

	output.Color = color;
	driver->getWindowSize (width, height);
	if (height > 0)
		FontRatio = FontRatio / height;
	
	// resize fontSize if window not of 800x600.
	if (keep800x600Ratio)
	{
		// keep the 800*600 ratio
		fontSize = (uint32)floor(fontSize*height/600.f);
		fontSize = max(fontSize, (uint32)2);
	}
	
	// Setting vertices format
	output.Vertices.setNumVertices (4 * s.size());
	
	// 1 character <-> 1 primitive block
	sint32 penx = 0, dx;
	sint32 penz = 0, dz;
	float x1, z1, x2, z2;
	float u1, v1, u2, v2;
	CMaterial		*pMatFont = fm->getFontMaterial();
	CTextureFont	*pTexFont = (CTextureFont*)(pMatFont->getTexture (0));
	float hlfW = 0.5f / pTexFont->getWidth();
	float hlfH = 0.5f / pTexFont->getHeight();
	float hlfPix = 0.5f;
	if (height > 0)
		hlfPix = hlfPix / height;
	CTextureFont::SLetterKey k;

	output.StringHeight = 0;
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
				u1 = pLI->U - hlfW;
				v1 = pLI->V - hlfH;
				u2 = pLI->U + ((float)pLI->CharWidth) / pTexFont->getWidth() + hlfW;
				v2 = pLI->V + ((float)pLI->CharHeight) / pTexFont->getHeight() + hlfH;

				x1 = (penx + dx) * FontRatio - hlfPix;
				z1 = (penz + dz) * FontRatio - hlfPix;
				x2 = (penx + dx + (sint32)pLI->CharWidth)  * FontRatio + hlfPix;
				z2 = (penz + dz + (sint32)pLI->CharHeight) * FontRatio + hlfPix;

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
				
				if (z2 > output.StringHeight) 
					output.StringHeight = z2;
			}
			penx += pLI->AdvX;
		}

		// Building Material
		output.Material = pMatFont;
	}
	output.Vertices.setNumVertices (j);

	output.StringWidth = penx * FontRatio;
}
  */

static void NL3DcomputeStringChar (CFontManager *fm, const std::basic_string<char> &s,
				CFontGenerator *fontGen,
				const NLMISC::CRGBA &color,
				uint32 fontSize,
				IDriver *driver,
				CComputedString &output,
				bool	keep800x600Ratio)
{

	float FontRatio = 1.0;
	uint32 width, height;

	output.Color = color;
	driver->getWindowSize (width, height);
	if (height > 0)
		FontRatio = FontRatio / height;
	
	// resize fontSize if window not of 800x600.
	if (keep800x600Ratio)
	{
		// keep the 800*600 ratio
		fontSize = (uint32)floor(fontSize*height/600.f);
		fontSize = max(fontSize, (uint32)2);
	}
	
	// Setting vertices format
	output.Vertices.setNumVertices (4 * s.size());
	
	// 1 character <-> 1 primitive block
	sint32 penx = 0, dx;
	sint32 penz = 0, dz;
	float x1, z1, x2, z2;
	float u1, v1, u2, v2;
	CMaterial		*pMatFont = fm->getFontMaterial();
	CTextureFont	*pTexFont = (CTextureFont*)(pMatFont->getTexture (0));
	float hlfW = 0.5f / pTexFont->getWidth();
	float hlfH = 0.5f / pTexFont->getHeight();
	float hlfPix = 0.5f;
	if (height > 0)
		hlfPix = hlfPix / height;
	CTextureFont::SLetterKey k;

	output.StringHeight = 0;
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
			sint32 width = pLI->CharWidth - 1;
			sint32 height = pLI->CharHeight - 1;
			if ((pLI->CharWidth > 0) && (pLI->CharHeight > 0))
			{
				// Creating vertices
				dx = pLI->Left;
				dz = -(height-(sint32)(pLI->Top));
				u1 = pLI->U - hlfW;
				v1 = pLI->V - hlfH;
				u2 = pLI->U + ((float)width) / pTexFont->getWidth() + hlfW;
				v2 = pLI->V + ((float)height) / pTexFont->getHeight() + hlfH;

				x1 = (penx + dx) * FontRatio - hlfPix;
				z1 = (penz + dz) * FontRatio - hlfPix;
				x2 = (penx + dx + width)  * FontRatio + hlfPix;
				z2 = (penz + dz + height) * FontRatio + hlfPix;

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
				
				if (fabsf(z1-z2) > output.StringHeight) 
					output.StringHeight = fabsf(z1-z2);
			}
			penx += pLI->AdvX;
		}

		// Building Material
		output.Material = pMatFont;
	}
	output.Vertices.setNumVertices (j);

	output.StringWidth = penx * FontRatio;
}

/*------------------------------------------------------------------*\
							computeString()
\*------------------------------------------------------------------*/
void CFontManager::computeString (const std::string &s,
								  CFontGenerator *fontGen,
								  const NLMISC::CRGBA &color,
								  uint32 fontSize,
								  IDriver *driver,
								  CComputedString &output,
								  bool	keep800x600Ratio)
{
	NL3DcomputeStringChar (this, s, fontGen, color, fontSize, driver, output,keep800x600Ratio);
}


/*------------------------------------------------------------------*\
							computeString()
\*------------------------------------------------------------------*/
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

	float FontRatio = 1.0f / height;
	
	// resize fontSize if window not of 800x600.
	if (keep800x600Ratio)
	{
		// keep the 800*600 ratio
		fontSize = (uint32)floor(fontSize*height/600.f);
		fontSize = max(fontSize, (uint32)2);
	}
	
	// Setting vertices format
	output.Vertices.setNumVertices (4 * s.size());
	
	// 1 character <-> 1 primitive block
	sint32 penx = 0, dx;
	sint32 penz = 0, dz;
	float x1, z1, x2, z2;
	float u1, v1, u2, v2;
	float maxZ=-1.0f, minZ=1.0f;
	CMaterial		*pMatFont = getFontMaterial();
	CTextureFont	*pTexFont = (CTextureFont*)(pMatFont->getTexture (0));
	float TexRatioW = 1.0f / pTexFont->getWidth();
	float TexRatioH = 1.0f / pTexFont->getHeight();
	float hlfPixTexW = 0.5f * TexRatioW;
	float hlfPixTexH = 0.5f * TexRatioH;
	float hlfPixScrW = 0.5f / width;
	float hlfPixScrH = 0.5f / height;

	CTextureFont::SLetterKey k;

	output.StringHeight = 0;
	uint j = 0; // Vertex index
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

				x1 = (penx + dx) * FontRatio - hlfPixScrW;
				z1 = (penz + dz) * FontRatio - hlfPixScrH;
				x2 = (penx + dx + (sint32)pLI->CharWidth)  * FontRatio + hlfPixScrW;
				z2 = (penz + dz + (sint32)pLI->CharHeight) * FontRatio + hlfPixScrH;

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
				
				if (z1 < minZ) minZ = z1;
				if (z2 > maxZ) maxZ = z2;
			}
			penx += pLI->AdvX;
		}

		// Building Material
		output.Material = pMatFont;
	}
	output.Vertices.setNumVertices (j);

	output.StringWidth = penx * FontRatio;
	output.StringHeight = maxZ - minZ;
	output.StringLine = fabsf(minZ);
}

/*------------------------------------------------------------------*\
						computeStringInfo()
\*------------------------------------------------------------------*/
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

	float FontRatio = 1.0f / height;
	
	// resize fontSize if window not of 800x600.
	if (keep800x600Ratio)
	{
		// keep the 800*600 ratio
		fontSize = (uint32)floor(fontSize*height/600.f);
		fontSize = max(fontSize, (uint32)2);
	}
	
	sint32 penx = 0;
	sint32 nZ1, nZ2;
	sint32 nMaxZ = -(sint32)height*2, nMinZ = height*2;
	CMaterial		*pMatFont = getFontMaterial();
	CTextureFont	*pTexFont = (CTextureFont*)(pMatFont->getTexture (0));

	CTextureFont::SLetterKey k;
	CTextureFont::SLetterInfo *pLI;

	output.StringHeight = 0;
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
				// Creating vertices
				nZ1 = -((sint32)pLI->CharHeight-(sint32)(pLI->Top));
				nZ2 = pLI->Top;
				
				if (nZ1 < nMinZ) nMinZ = nZ1;
				if (nZ2 > nMaxZ) nMaxZ = nZ2;
			}
			penx += pLI->AdvX;
		}
	}
	output.StringWidth = penx * FontRatio;
	output.StringHeight = (nMaxZ - nMinZ + 1) * FontRatio;
	output.StringLine = fabsf((nMinZ - 0.5f) * FontRatio);
}


string CFontManager::getCacheInformation() const
{
	stringstream ss;

	ss << "MaxMemory: " << (uint) _MaxMemory << " MemSize: " << (uint) _MemSize << " NbChar: " << (uint) _NbChar;

	return ss.str();
}





} // NL3D
