/** \file texture_dlm.h
 * <File description>
 *
 * $Id: texture_dlm.h,v 1.1 2002/04/12 15:59:57 berenguier Exp $
 */

/* Copyright, 2000-2002 Nevrax Ltd.
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

#ifndef NL_TEXTURE_DLM_H
#define NL_TEXTURE_DLM_H

#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"
#include "3d/texture.h"


// Size of a Block in the texture. Must be 10.
#define NL_DLM_BLOCK_SIZE	10

// Number of lightmap type. 4*4
#define NL_DLM_LIGHTMAP_TYPE_SIZE	16


namespace NL3D 
{


using NLMISC::CRGBA;

// ***************************************************************************
/**
 * This texture is used by landscape to perform Dynamic LightMap (DLM).
 *	Actually a CTextureDLM handle many block of lightmap in one single big texture.
 *	Block can be of size of 2,3,5 or 9 * 2,3,5 or 9  (eg 2x9, or 5x5 texture).
 *
 *	Implementation note:
 *	To make this possible easily, blocks of 10x10 are created and placed in the texture. 
 *	Hence a 9x9 texture lies in a single block, 3 textures of 3x9 lies in a block etc... 
 *	In worst case, lost space is 19%  (1 - 9*9 / 10*10).
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CTextureDLM : public ITexture
{
public:

	/// Constructor
	CTextureDLM(uint width, uint height);

	/// Since texture is always in memory...
	void			doGenerate()
	{
		// Do nothing. texture still in memory... :o)
	}

	/// TextureDLM are system. Do not need to serialize them...
	// default ctor is required for compilation with NLMISC_DECLARE_CLASS, but never called...
	CTextureDLM() {nlstop;}
	virtual void	serial(NLMISC::IStream &f) throw(NLMISC::EStream) {nlstop;}
	NLMISC_DECLARE_CLASS(CTextureDLM);



	/// \name Lightmap mgt.
	// @{

	/// return true if can create a texture of this size.
	bool			canCreateLightMap(uint w, uint h);

	/** create a space for a lightmap. NB: texture space is not filled with black.
	 *	return false if cannot, else return true, and return in x/y the position in the texture.
	 */
	bool			createLightMap(uint w, uint h, uint &x, uint &y);

	/** refill the texture with raw data. NB: no check is made on x,y,w,h lightmap validity.
	 *	CRGBA are transformed to texture format (16 bits or better)
	 *	The texture is invalidate (on this part only...)
	 *	\param map is the raw array of RGBA colors to fills. must be of w*h size
	 */
	void			fillRect(uint x, uint y, uint w, uint h, CRGBA  *textMap);

	/// Set a lightmap as free for use. It is an error to free a not allocated lightmap. (nlassert!!)
	void			releaseLightMap(uint x, uint y);

	// @}



// *****************************
private:

	/// A block descriptor.
	struct	CBlock
	{
		// Size of a lightmap in the block. eg: 9x9. Not relevant if FreeSpace==0 (because block completely free).
		uint8		Width, Height;
		// BitField of Space free (1 if not free). since 2x2 is the minimum size, there is 
		// at max 10 lightmaps in a blocks. Hence a uint16.
		uint16		FreeSpace;

		// Position of block in texture, in pixels.
		uint16		PosX, PosY;

		/// Free List.
		CBlock		*FreePrec, *FreeNext;

		CBlock()
		{
			FreeSpace= 0;
			// No List
			FreePrec= FreeNext= NULL;
		}
	};

private:

	/// Number of block per line
	uint				_WBlock;

	/** The list of blocks. There is TextureWidth/10 * TextureHeight/10 blocks, ranged from left to right 
	 *	then top to bottom.
	 */
	std::vector<CBlock>	_Blocks;


	/// The list of available Blocks, ie Blocks with FreeSpace==0
	std::vector<uint>	_EmptyBlocks;


	/// For each type of lightmaps (2x2, 2x3 etc...), list of blocks which are not full
	CBlock				*_FreeBlocks[NL_DLM_LIGHTMAP_TYPE_SIZE];

	/// get the lightmap type id according to lightmap size.
	uint				getTypeForSize(uint width, uint height);

	/// FreeBlock list mgt.
	void				linkFreeBlock(uint lMapType, CBlock *block);
	void				unlinkFreeBlock(uint lMapType, CBlock *block);

};


} // NL3D


#endif // NL_TEXTURE_DLM_H

/* End of texture_dlm.h */
