/** \file tile_far_bank.h
 * <File description>
 *
 * $Id: tile_far_bank.h,v 1.1 2000/12/19 14:25:03 corvazier Exp $
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

#ifndef NL_TILE_FAR_BANK_H
#define NL_TILE_FAR_BANK_H

#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"

#include <vector>

namespace NLMISC 
{
	class IStream;
	struct EStream;
}

namespace NL3D 
{


/**
 * A bank for the far textures
 * \author Cyril Corvazier
 * \author Nevrax France
 * \date 2000
 */
class CTileFarBank
{
public:

	/// Constructor
	CTileFarBank();

	// Class for a tile far descriptor
	class CTileFar
	{
		// Friend of CTileFarBank
		friend class CTileFarBank;
	public:
		/// Default constructor
		CTileFar ()
		{
			_Width=0;
			_Height=0;
		}

		/// Return the width of the far tile texture. Should be 0, 4 or 8
		uint16				getWidth () const
		{
			return _Width;
		}

		/// Return the width of the far tile texture. Should be 0, 4 or 8
		uint16				getHeight () const
		{
			return _Height;
		}

		/// Return the pointer on the pixels data. Call this method only if isFill () returns true. 
		const NLMISC::CRGBA*		getPixels () const
		{
			return &_Pixels[0];
		}

		/// Return true if pixel value are presents, else return false.
		bool				isFill () const
		{
			return _Width&&_Height;
		}

		/// Serial this tile
		void				serial(class NLMISC::IStream &f) throw(NLMISC::EStream);

	private:
		/// Width of the far tile. Should be 0, 4 or 8
		uint16				_Width;
		
		/// Height of the far tile. Should be 0, 4 or 8
		uint16				_Height;

		/// RGBA Pixels vector
		std::vector<NLMISC::CRGBA>	_Pixels;

		/// The version of this class
		static const sint	_Version;
	};

	/// Get number of tile in this bank
	sint					getNumTile () const
	{
		return (sint)_TileVector.size();
	}

	/// Get a read only far tile pointer.
	const CTileFar*			getTile (sint tile) const
	{
		return &_TileVector[tile];
	}

	/// Serial this bank
	void    serial(class NLMISC::IStream &f) throw(NLMISC::EStream);

	/// The far tile vector
	std::vector<CTileFar>	_TileVector;

	/// The version of this class
	static const sint	_Version;
};


} // NL3D


#endif // NL_TILE_FAR_BANK_H

/* End of tile_far_bank.h */
