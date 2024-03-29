/** \file texture_font.h
 * TODO: File description
 *
 * $Id: texture_font.h,v 1.10 2005/02/22 10:19:12 besson Exp $
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

#ifndef NL_TEXTURE_FONT_H
#define NL_TEXTURE_FONT_H

#include "nel/misc/types_nl.h"
#include "texture.h"

namespace NL3D
{

class CFontGenerator;

#define TEXTUREFONT_NBCATEGORY	5 // Config 1
//#define TEXTUREFONT_NBCATEGORY	4

//****************************************************************************
/**
 * CTextureFont
 */
class CTextureFont : public ITexture
{

public:

	struct SLetterInfo
	{	
		// To generate the letter
		ucchar Char;
		CFontGenerator *FontGenerator;
		sint Size;
		

		// The less recently used infos
		SLetterInfo *Next, *Prev;

		uint Cat; // 8x8, 16x16, 24x24, 32x32

		//////////////////////////////////////////////////////////////////////

		float U ,V;
		uint32 CharWidth;
		uint32 CharHeight;
		uint32 GlyphIndex;	// number of the character in the this font
		sint32 Top;			// Distance between origin and top of the texture
		sint32 Left;		// Distance between origin and left of the texture
		sint32 AdvX;		// Advance to the next caracter
	};

	struct SLetterKey
	{
		ucchar Char;
		CFontGenerator *FontGenerator;
		sint Size;

		uint32 getVal();
		//bool operator < (const SLetterKey&k) const;
		//bool operator == (const SLetterKey&k) const;
	};

public:

	/** 
	 * Default constructor
	 */	
	CTextureFont();
	virtual ~CTextureFont();

	// Generate the texture
	void doGenerate (bool async = false);

	// This function manage the cache if the letter wanted does not exist
	SLetterInfo* getLetterInfo (SLetterKey& k);

	void dumpTextureFont (const char *filename);

private:

	// To find a letter in the texture
	std::map<uint32, SLetterInfo*> Accel;

	std::vector<SLetterInfo> Letters[TEXTUREFONT_NBCATEGORY];
	SLetterInfo *Front[TEXTUREFONT_NBCATEGORY], *Back[TEXTUREFONT_NBCATEGORY];

	void rebuildLetter (sint cat, sint x, sint y);

	/// Todo: serialize a font texture.
public:
	virtual void	serial(NLMISC::IStream &f) throw(NLMISC::EStream) {nlstop;}
	NLMISC_DECLARE_CLASS(CTextureFont);

};


} // NL3D


#endif // NL_TEXTURE_FONT_H

/* End of texture_font.h */
