/** \file tile_bank.h
 * Management of tile texture.
 *
 * $Id: tile_bank.h,v 1.13 2001/01/08 17:58:29 corvazier Exp $
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

#ifndef NL_Tile_BANK_H
#define NL_Tile_BANK_H

#include "nel/misc/debug.h"
#include "nel/misc/stream.h"
#include "nel/misc/rgba.h"
#include <vector>
#include <set>
#include <string>

namespace	NLMISC
{
	class	IStream;
}


namespace	NL3D
{

class CTileBank;

/**
 * Tiles
 * \author Cyril Corvazier
 * \author Nevrax France
 * \date 2000
 */
class CTile
{
public:
	friend class CTileSet;
	friend class CTileBank;
	enum TBitmap { diffuse=0, additive, bump, bitmapCount };
public:
	CTile ()
	{
		_Free=true;
		_Invert=false;
	}
	const std::string& getFileName (TBitmap bitmapType) const 
	{ 
		return _BitmapName[bitmapType]; 
	}
	bool isFree () const
	{
		return _Free;
	}
	bool isInvert () const
	{
		return _Invert;
	}
	void setInvert (bool invert)
	{
		_Invert=invert;
	}
	void    serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
	void setFileName (TBitmap bitmapType, const std::string& name)
	{ 
		_Free=false;
		_BitmapName[bitmapType]=name;
	}

private:
	void	clearTile (CTile::TBitmap type);
	void	free ()
	{
		nlassert (!_Free);
		_Free=true;
	}

	bool						_Free;
	bool						_Invert;
	std::string					_BitmapName[bitmapCount];
	static const sint			_Version;
};

/**
 * Set of tiles for a land
 * \author Cyril Corvazier
 * \author Nevrax France
 * \date 2000
 */
class CTileLand
{
private:
	// Class CTileLand;
public:
	const std::string& getName () const 
	{ 
		return _Name; 
	};
	void setName (const std::string& name);
	void addTileSet (const std::string& name);
	void removeTileSet (const std::string& name);
	bool isTileSet (const std::string& name)
	{
		return _TileSet.find (name)!=_TileSet.end();
	}

	void    serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
private:

	// internal use
	static void intersect (const std::set<sint32>& setSrc1, const std::set<sint32>& setSrc2, std::set<sint32>& setDst);

	std::string	_Name;
	std::set<std::string>	_TileSet;
	static const sint _Version;
};

/**
 * This class manage a transition tile.
 * \author Cyril Corvazier
 * \author Nevrax France
 * \date 2000
 */
class CTileSetTransition
{
	friend class CTileSet;
	friend class CTileBank;
public:
	CTileSetTransition ()
	{
		_Tile=-1;
	}
	sint32 getTile () const
	{
		return _Tile;
	}
	void    serial(class NLMISC::IStream &f) throw(NLMISC::EStream);

private:
	sint32	_Tile;
	static const sint _Version;
};

/**
 * This class is a tile set. It handles all the tile of the same material. 
 * \author Cyril Corvazier
 * \author Nevrax France
 * \date 2000
 */
class CTileBorder
{
public:
	enum TBorder { top=0, bottom, left, right, borderCount };
	CTileBorder ()
	{
		reset();
	}
	void set (int width, int height, const std::vector<NLMISC::CBGRA>& array);
	void doubleSize ();
	bool operator== (const CTileBorder& border) const;
	void operator= (const CTileBorder& border);
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	bool isSet() const
	{
		return _Set;
	}
	void reset()
	{
		_Set=false;
		_Borders[CTile::diffuse].clear();
		_Borders[CTile::additive].clear();
		_Borders[CTile::bump].clear();
	}
	sint32 getWidth() const
	{
		return _Width;
	}
	sint32 getHeight() const
	{
		return _Height;
	}
	void	invertAlpha();

	static bool compare (const CTileBorder& border1, const CTileBorder& border2, TBorder where1, TBorder where2, int& pixel, int& composante, bool bInvertFirst=false, bool bInvertSecond=false);

private:
	bool _Set;
	sint32 _Width;
	sint32 _Height;
	std::vector<NLMISC::CBGRA> _Borders[borderCount];
	static const sint _Version;
};

/**
 * This class is a tile set. It handles all the tile of the same material. 
 * \author Cyril Corvazier
 * \author Nevrax France
 * \date 2000
 */
class CTileSet
{
	friend class CTileBank;
public:
	enum TError { ok=0, topInterfaceProblem, bottomInterfaceProblem, leftInterfaceProblem,
		rightInterfaceProblem, addFirstA128128, topBottomNotTheSame, rightLeftNotTheSame, 
		sizeInvalide, errorCount };
	enum TTransition { first=0, last=47, count=48, notfound=-1 };
	enum TBorder { top=0, bottom, left, right, borderCount };
	enum TFlagBorder { _1111=0,	_0111, _1110, _0001, _1000, _0000, dontcare=-1 };

	// add
	void addTile128 (int& indexInTileSet, CTileBank& bank);
	void addTile256 (int& indexInTileSet, CTileBank& bank);

	// remove
	void removeTile128 (int indexInTileSet, CTileBank& bank);
	void removeTile256 (int indexInTileSet, CTileBank& bank);

	// clear
	void clearTile128 (int indexInTileSet, CTile::TBitmap type, CTileBank& bank);
	void clearTile256 (int indexInTileSet, CTile::TBitmap type, CTileBank& bank);
	void clearTransition (TTransition transition, CTile::TBitmap type, CTileBank& bank);

	// set
	void setName (const std::string& name);
	void setTile128 (int indexInTileSet, const std::string& name, CTile::TBitmap type, CTileBank& bank);
	void setTile256 (int indexInTileSet, const std::string& name, CTile::TBitmap type, CTileBank& bank);
	void setTileTransition (TTransition transition, const std::string& name, CTile::TBitmap type, CTileBank& bank, const CTileBorder& border, bool bInvert);
	void setBorder (CTile::TBitmap type, const CTileBorder& border);

	// check
	TError checkTile128 (CTile::TBitmap type, const CTileBorder& border, int& pixel, int& composante);
	TError checkTile256 (CTile::TBitmap type, const CTileBorder& border, int& pixel, int& composante);
	TError checkTileTransition (TTransition transition, CTile::TBitmap type, const CTileBorder& border, int& indexError,
		int& pixel, int& composante, bool bInvert);

	// get
	const std::string& getName () const;
	sint getNumTile128 () const
	{
		return (sint)_Tile128.size();
	}
	sint getNumTile256 () const
	{
		return _Tile256.size();
	}
	sint32 getTile128 (sint index) const
	{
		return _Tile128[index];
	}
	sint32 getTile256 (sint index) const
	{
		return _Tile256[index];
	}
	CTileSetTransition* getTransition (sint index)
	{
		return _TileTransition+index;
	}
	const CTileSetTransition* getTransition (sint index) const
	{
		return _TileTransition+index;
	}
	static const char* getErrorMessage (TError error)
	{
		return _ErrorMessage[error];
	}
	static TTransition getTransitionTile (TFlagBorder top, TFlagBorder bottom, TFlagBorder left, TFlagBorder right);
	TTransition getExistingTransitionTile (TFlagBorder _top, TFlagBorder _bottom, TFlagBorder _left, 
		TFlagBorder _right, int reject, CTile::TBitmap type);
	static TTransition getComplementaryTransition (TTransition transition);
	static TFlagBorder getInvertBorder (TFlagBorder border);
	static TFlagBorder getOrientedBorder (TBorder where, TFlagBorder border);
	static TFlagBorder getEdgeType (TTransition _what, TBorder _where)
	{
		return _TransitionFlags[_what][_where];
	}

	// other
	void addChild (const std::string& name);
	void removeChild (const std::string& name);
	bool isChild (const std::string& name)
	{
		return _ChildName.find(name)!=_ChildName.end();
	}
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
private:
	static TFlagBorder getComplementaryBorder (TFlagBorder border);

	// Delete 128 and 256 borders if no more valid texture file name for each bitmap type.
	void			deleteBordersIfLast (const CTileBank& bank, CTile::TBitmap type);

private:
	std::string	_Name;
	std::vector<sint32>	_Tile128;
	std::vector<sint32>	_Tile256;
	CTileSetTransition _TileTransition[count];
	std::set<std::string> _ChildName;
	CTileBorder _Border128[CTile::bitmapCount];
	CTileBorder _Border256[CTile::bitmapCount];
	CTileBorder _BorderTransition[count][CTile::bitmapCount];
	static const sint _Version;
	static const char* _ErrorMessage[CTileSet::errorCount];
	static const TFlagBorder _TransitionFlags[count][4];
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
	friend class CTileSet;
public:
	enum TTileType { _128x128=0, _256x256, transition, undefined };
	// Get
	sint getLandCount () const 
	{ 
		return _LandVector.size(); 
	};
	const CTileLand* getLand (int landIndex) const
	{ 
		return &_LandVector[landIndex]; 
	};
	CTileLand* getLand (int landIndex) 
	{ 
		return &_LandVector[landIndex]; 
	};
	sint getTileSetCount () const 
	{ 
		return _TileSetVector.size(); 
	};
	const CTileSet* getTileSet (int tileIndex) const
	{ 
		return &_TileSetVector[tileIndex]; 
	};
	CTileSet* getTileSet (int tileIndex)
	{ 
		return &_TileSetVector[tileIndex]; 
	};
	sint getTileCount () const 
	{ 
		return _TileVector.size(); 
	};
	const CTile* getTile (int tileIndex) const
	{ 
		return &_TileVector[tileIndex]; 
	};
	CTile* getTile (int tileIndex)
	{ 
		return &_TileVector[tileIndex]; 
	};
	sint addLand (const std::string& name);
	void removeLand (sint landIndex);
	sint addTileSet (const std::string& name);
	void removeTileSet (sint landIndex);
	void xchgTileset (sint first, sint second);
	void clear ();
	sint getNumBitmap (CTile::TBitmap bitmap) const;
	void computeXRef ();
	void getTileXRef (int tile, int &tileSet, int &number, TTileType& type) const;
	void makeAllPathRelative ();

	void    serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
private:
	sint	createTile ();
	void	freeTile (int tileIndex);
private:
	struct CTileXRef
	{
		CTileXRef ()
		{
			_XRefTileType=undefined;
		}
		CTileXRef (int tileSet, int number, TTileType type)
		{
			_XRefTileSet=tileSet;
			_XRefTileNumber=number;
			_XRefTileType=type;
		}
		int	_XRefTileSet;
		int	_XRefTileNumber;
		TTileType	_XRefTileType;
	};
	std::vector<CTileLand>	_LandVector;
	std::vector<CTileSet>	_TileSetVector;
	std::vector<CTile>	_TileVector;
	std::vector<CTileXRef>	_TileXRef;
	static const sint	_Version;
};



}

#endif // NL_Tile_BANK_H

/* End of tile_bank.h */
