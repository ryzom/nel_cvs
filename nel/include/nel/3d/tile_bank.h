/** \file tile_bank.h
 * Management of tile texture.
 *
 * $Id: tile_bank.h,v 1.7 2000/12/18 09:46:21 corvazier Exp $
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

#ifndef NL_TILE_BANK_H
#define NL_TILE_BANK_H

#include "nel/misc/debug.h"
//#include "nel/misc/stream.h"
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
	enum TBitmap { diffuse=0, additive, bump, bitmapCount };
public:
	CTile ()
	{
		_free=true;
		_invert=false;
	}
	const std::string& getFileName (TBitmap bitmapType) const 
	{ 
		return _bitmapName[bitmapType]; 
	}
	void setFileName (TBitmap bitmapType, const std::string& name)
	{ 
		_free=false;
		_bitmapName[bitmapType]=name;
	}
	bool isFree () const
	{
		return _free;
	}
	void Free ()
	{
		nlassert (!_free);
		_free=true;
	}
	bool isInvert () const
	{
		return _invert;
	}
	void setInvert (bool invert)
	{
		_invert=invert;
	}
	void    serial(class NLMISC::IStream &f);
	void	clearTile (CTile::TBitmap type);

private:
	bool _free;
	bool _invert;
	std::string	_bitmapName[bitmapCount];
	static const sint _version;
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
		return _name; 
	};
	void setName (const std::string& name);
	void addTileSet (const std::string& name);
	void removeTileSet (const std::string& name);
	bool isTileSet (const std::string& name)
	{
		return _tileSet.find (name)!=_tileSet.end();
	}

	void    serial(class NLMISC::IStream &f);
private:

	// internal use
	static void intersect (const std::set<sint32>& setSrc1, const std::set<sint32>& setSrc2, std::set<sint32>& setDst);

	std::string	_name;
	std::set<std::string>	_tileSet;
	static const sint _version;
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
		_tile=-1;
	}
	sint32 getTile () const
	{
		return _tile;
	}
	void    serial(class NLMISC::IStream &f);

private:
	sint32	_tile;
	static const sint _version;
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
	void serial(NLMISC::IStream &f);
	bool isSet() const
	{
		return _set;
	}
	void reset()
	{
		_set=false;
	}
	sint32 getWidth() const
	{
		return _width;
	}
	sint32 getHeight() const
	{
		return _height;
	}
	void	invertAlpha();

	static bool compare (const CTileBorder& border1, const CTileBorder& border2, TBorder where1, TBorder where2, int& pixel, int& composante, bool bInvertFirst=false, bool bInvertSecond=false);

private:
	bool _set;
	sint32 _width;
	sint32 _height;
	std::vector<NLMISC::CBGRA> _borders[borderCount];
	static const sint _version;
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
	void ClearTransition (TTransition transition, CTile::TBitmap type, CTileBank& bank);

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
		return (sint)_tile128.size();
	}
	sint getNumTile256 () const
	{
		return _tile256.size();
	}
	sint32 getTile128 (sint index) const
	{
		return _tile128[index];
	}
	sint32 getTile256 (sint index) const
	{
		return _tile256[index];
	}
	CTileSetTransition* getTransition (sint index)
	{
		return _tileTransition+index;
	}
	const CTileSetTransition* getTransition (sint index) const
	{
		return _tileTransition+index;
	}
	static const char* getErrorMessage (TError error)
	{
		return _errorMessage[error];
	}
	static TTransition getTransitionTile (TFlagBorder top, TFlagBorder bottom, TFlagBorder left, TFlagBorder right);
	TTransition getExistingTransitionTile (TFlagBorder _top, TFlagBorder _bottom, TFlagBorder _left, 
		TFlagBorder _right, int reject, CTile::TBitmap type);
	static TTransition getComplementaryTransition (TTransition transition);
	static TFlagBorder getInvertBorder (TFlagBorder border);
	static TFlagBorder getOrientedBorder (TBorder where, TFlagBorder border);
	static TFlagBorder getEdgeType (TTransition _what, TBorder _where)
	{
		return _transitionFlags[_what][_where];
	}

	// other
	void addChild (const std::string& name);
	void removeChild (const std::string& name);
	bool isChild (const std::string& name)
	{
		return _childName.find(name)!=_childName.end();
	}
	void serial(NLMISC::IStream &f);
private:
	static TFlagBorder getComplementaryBorder (TFlagBorder border);


private:
	std::string	_name;
	std::vector<sint32>	_tile128;
	std::vector<sint32>	_tile256;
	CTileSetTransition _tileTransition[count];
	std::set<std::string> _childName;
	CTileBorder _border128[CTile::bitmapCount];
	CTileBorder _border256[CTile::bitmapCount];
	CTileBorder _borderTransition[count][CTile::bitmapCount];
	static const sint _version;
	static const char* _errorMessage[CTileSet::errorCount];
	static const TFlagBorder _transitionFlags[count][4];
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
		return _landVector.size(); 
	};
	const CTileLand* getLand (int landIndex) const
	{ 
		return &_landVector[landIndex]; 
	};
	CTileLand* getLand (int landIndex) 
	{ 
		return &_landVector[landIndex]; 
	};
	sint getTileSetCount () const 
	{ 
		return _tileSetVector.size(); 
	};
	const CTileSet* getTileSet (int tileIndex) const
	{ 
		return &_tileSetVector[tileIndex]; 
	};
	CTileSet* getTileSet (int tileIndex)
	{ 
		return &_tileSetVector[tileIndex]; 
	};
	sint getTileCount () const 
	{ 
		return _tileVector.size(); 
	};
	const CTile* getTile (int tileIndex) const
	{ 
		return &_tileVector[tileIndex]; 
	};
	CTile* getTile (int tileIndex)
	{ 
		return &_tileVector[tileIndex]; 
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

	void    serial(class NLMISC::IStream &f);
private:
	sint	createTile ();
	void	freeTile (int tileIndex);
private:
	struct CTileXRef
	{
		CTileXRef ()
		{
			_xRefTileType=undefined;
		}
		CTileXRef (int tileSet, int number, TTileType type)
		{
			_xRefTileSet=tileSet;
			_xRefTileNumber=number;
			_xRefTileType=type;
		}
		int	_xRefTileSet;
		int	_xRefTileNumber;
		TTileType	_xRefTileType;
	};
	std::vector<CTileLand>	_landVector;
	std::vector<CTileSet>	_tileSetVector;
	std::vector<CTile>	_tileVector;
	std::vector<CTileXRef>	_tileXRef;
	static const sint	_version;
};



}

#endif // NL_TILE_BANK_H

/* End of tile_bank.h */
