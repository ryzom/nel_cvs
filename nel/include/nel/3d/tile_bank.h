/* tile_bank.h
 *
 * Copyright, 2000 Nevrax Ltd.
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

/*
 * $Id: tile_bank.h,v 1.1 2000/10/06 10:04:54 corvazier Exp $
 *
 * Management of tile texture.
 */

#ifndef NL_TILE_BANK_H
#define NL_TILE_BANK_H

#include "nel/misc/stream.h"
#include <vector>
#include <set>
#include <string>

namespace	NLMISC
{
	class	IStream;
}


namespace	NL3D
{


/**
 * Tile transition
 * \author Cyril Corvazier
 * \author Nevrax France
 * \date 2000
 */
class CTileBankTransition
{
private:
	// Class CTileBank;
	friend class CTileBankLand;
public:
	// Return the set of tiles of this transition
	const std::set<sint32> getTileSet () const { return TileSet; };

	// Access
	void AddTile (sint32 tileIndex);

	void    serial(class NLMISC::IStream &f);
private:
	std::set<sint32>	TileSet;

	static const sint Version;
};

/**
 * Tiles
 * \author Cyril Corvazier
 * \author Nevrax France
 * \date 2000
 */
class CTileBankTile
{
private:
	// Class CTileBank;
	friend class CTileBankLand;
public:
	enum TBorder { north=0, east, south, west, borderCount };
	enum TBitmap { normal=0, add, bump, bitmapCount };
public:
	CTileBankTile ()
	{
		Mask=0;
		Index=-1;
	}

	// Return the type mask
	uint32 getTypeMask () const 
	{ 
		return Mask; 
	};

	// Get the index of the tile in the land bank
	sint32 getTileIndex () const 
	{ 
		return Index; 
	};

	// Get filename, return NULL if no bitmap defined
	std::string getFileName (TBitmap bitmapType) const 
	{ 
		return BitmapName[bitmapType]; 
	}

	// Get the transition number
	sint32 getTransition (TBorder where) const 
	{ 
		return Transitions[where]; 
	};

	// Access
	uint32 AddTypeFlags (uint32 flags) 
	{ 
		return Mask|=flags; 
	}
	uint32 RemoveTypeFlags (uint32 flags) 
	{ 
		return Mask&=~flags; 
	}

	void    serial(class NLMISC::IStream &f);
private:
	uint32	Mask;
	sint32	Index;
	std::string	BitmapName[bitmapCount];
	sint32 Transitions[borderCount];

	static const sint Version;
};

/**
 * Set of tiles for a land
 * \author Cyril Corvazier
 * \author Nevrax France
 * \date 2000
 */
class CTileBankLand
{
private:
	// Class CTileBank;
	friend class CTileBank;
public:
	enum TTextureResolution { texRes128=0, texRes64, texRes32 };

	// *** Name ***
	std::string getName () const 
	{ 
		return Name; 
	};

	// *** Tiles ***

	// Get number of tile in this land
	// This is the number of the last tile + 1
	sint getTileCount () const 
	{ 
		return TileVector.size(); 
	};

	// Get a tile
	// Can return NULL if the tile is empty
	const CTileBankTile* getTile (sint tileIndex) const 
	{ 
		return &TileVector[tileIndex]; 
	};
	CTileBankTile* getTile (sint tileIndex) 
	{ 
		return &TileVector[tileIndex]; 
	};

	// *** Types ***
	
	// Get number of tile type
	sint getTileTypeCount () const 
	{ 
		return TypeVector.size(); 
	};

	// Get a tile type
	std::string getTileType (sint typeIndex) const 
	{ 
		return TypeVector[typeIndex]; 
	};

	// *** Compressed tile file ***

	// Return the name for the "nTextureIndex" texture file in the resolution "texRes"
	std::string GetTextureFileName (sint textureIndex, TTextureResolution texRes) const;

	// *** Transition ***

	// Get transition count
	sint getTransitionCount () const 
	{ 
		return TransitionVector.size(); 
	};

	// Return a transition
	const CTileBankTransition* getTransition (sint transitionIndex) const 
	{ 
		return &TransitionVector[transitionIndex]; 
	};
	CTileBankTransition* getTransition (sint transitionIndex) 
	{ 
		return &TransitionVector[transitionIndex]; 
	};

	// *** Advanced ***

	// Search all tiles matching the 4 transitions in clockzize order. Transition equal to -1 must by ignored.
	void getMatchingTile (std::vector<sint32> setTiles, sint32 transition1, sint32 transition2, sint32 transition3, sint32 transition4);

	// Access
	void resizeTile (sint tileCount);
	sint addType (const std::string& name);
	sint addTransition ();

	void    serial(class NLMISC::IStream &f);
private:

	// internal use
	static void intersect (const std::set<sint32>& setSrc1, const std::set<sint32>& setSrc2, std::set<sint32>& setDst);

	std::string	Name;
	std::vector<CTileBankTile>	TileVector;
	std::vector<std::string>	TypeVector;
	std::vector<CTileBankTransition>	TransitionVector;

	static const sint Version;
};

/**
 * This class manage tile texture. It can load banktile description file 
 * (*.bank), and then gives access to land infos.
 * \author Cyril Corvazier
 * \author Nevrax France
 * \date 2000
 */
class CTileBank
{
public:
	/// \name Land access
	/**
	 * Get count of land in this bank
	 * \return the number of land in this bank.
	 */
	sint getLandCount () const 
	{ 
		return LandVector.size(); 
	};
	/**
	 * Get a pointer on a land class
	 * \param index Index of the land.
	 * \return A pointer of the land.
	 */
	const CTileBankLand* getLand (int landIndex) const 
	{ 
		return &LandVector[landIndex]; 
	};
	CTileBankLand* getLand (int landIndex) 
	{ 
		return &LandVector[landIndex]; 
	};
	sint addLand (const std::string& name);
	void removeLand (sint landIndex);
	void clear ();

	void    serial(class NLMISC::IStream &f);
private:
	std::vector<CTileBankLand>	LandVector;
	static const sint Version;
};



}

#endif // NL_TILE_BANK_H

/* End of tile_bank.h */
