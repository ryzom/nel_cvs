/** \file tile_bank.cpp
 * Management of tile texture.
 *
 * $Id: tile_bank.cpp,v 1.11 2000/12/13 15:01:46 corvazier Exp $
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

#include "nel/3d/tile_bank.h"
#include "nel/misc/stream.h"
#include "nel/misc/common.h"

using namespace NLMISC;

namespace	NL3D
{


// ***************************************************************************
// ***************************************************************************
// TileBankLand.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
const sint CTileLand::_version=0;
// ***************************************************************************
void CTileLand::serial(IStream &f)
{
	sint streamver = f.serialVersion(_version);

	f.serial (_name);
	f.serialCont (_tileSet);
}
// ***************************************************************************
void CTileLand::addTileSet (const std::string& name)
{
	_tileSet.insert (name);
}
// ***************************************************************************
void CTileLand::removeTileSet (const std::string& name)
{
	_tileSet.erase (name);
}
// ***************************************************************************
void CTileLand::setName (const std::string& name)
{ 
	_name=name;
};


// ***************************************************************************
// ***************************************************************************
// CTileBank.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
const sint CTileBank::_version=2;
// ***************************************************************************
void    CTileBank::serial(IStream &f)
{
	f.serialCheck (std::string ("BANK"));

	sint streamver = f.serialVersion(_version);
	
	// Version 1 not compatible
	if (f.isReading())
	{
		if (streamver<2)
			throw EOlderStream();
	}

	f.serialCont (_landVector);
	f.serialCont (_tileSetVector);
	f.serialCont (_tileVector);
}
// ***************************************************************************
sint CTileBank::addLand (const std::string& name)
{
	sint last=_landVector.size();
	_landVector.push_back(CTileLand());
	_landVector[last].setName (name);
	return last;
}
// ***************************************************************************
void CTileBank::removeLand (sint landIndex)
{
	// Check args
	nlassert (landIndex>=0);
	nlassert (landIndex<(sint)_landVector.size());

	_landVector.erase (_landVector.begin ()+landIndex);
}
// ***************************************************************************
sint CTileBank::addTileSet (const std::string& name)
{
	sint last=_tileSetVector.size();
	_tileSetVector.push_back(CTileSet());
	_tileSetVector[last].setName (name);
	for (int i=0; i<CTileSet::count; i++)
	{
		_tileSetVector[last]._tileTransition[i]._tile=createTile ();
	}
	return last;
}
// ***************************************************************************
void CTileBank::removeTileSet (sint setIndex)
{
	// Check args
	nlassert (setIndex>=0);
	nlassert (setIndex<(sint)_tileSetVector.size());

	for (int i=0; i<CTileSet::count; i++)
	{
		int index=_tileSetVector[setIndex]._tileTransition[i]._tile;
		if (index!=-1)
			freeTile (index);
	}
	_tileSetVector.erase (_tileSetVector.begin ()+setIndex);
}
// ***************************************************************************
void CTileBank::clear ()
{
	_landVector.clear ();
	_tileSetVector.clear ();
	_tileVector.clear ();
}
// ***************************************************************************
sint CTileBank::createTile ()
{
	// Look for a free tile
	for (int i=0; i<(sint)_tileVector.size(); i++)
	{
		if (_tileVector[i].isFree())
		{
			_tileVector[i].setFileName (CTile::diffuse, "");
			_tileVector[i].setFileName (CTile::additive, "");
			_tileVector[i].setFileName (CTile::bump, "");
			return i;
		}
	}

	// Nothing free, add a tile at the end
	_tileVector.push_back (CTile());
	_tileVector[_tileVector.size()-1].setFileName (CTile::diffuse, "");
	_tileVector[_tileVector.size()-1].setFileName (CTile::additive, "");
	_tileVector[_tileVector.size()-1].setFileName (CTile::bump, "");
	return _tileVector.size()-1;
}
// ***************************************************************************
void CTileBank::freeTile (int tileIndex)
{
	// Check args
	nlassert (tileIndex>=0);
	nlassert (tileIndex<(sint)_tileVector.size());

	// Free
	_tileVector[tileIndex].Free();

	// Resize tile table
	int i;
	for (i=(sint)_tileVector.size()-1; i>=0; i--)
	{
		if (!_tileVector[i].isFree ())
			break;
	}
	if (i<(sint)_tileVector.size()-1)
		_tileVector.resize (i+1);
}
// ***************************************************************************
sint CTileBank::getNumBitmap (CTile::TBitmap bitmap) const
{
	std::set<std::string> setString;
	for (int i=0; i<(sint)_tileVector.size(); i++)
	{
		if (!_tileVector[i].isFree())
		{
			const std::string &str=_tileVector[i].getFileName (bitmap);
			if (str!="")
			{
				std::vector<char> vect (str.length()+1);
				memcpy (&*vect.begin(), str.c_str(), str.length()+1);
				NLMISC::strlwr (&*vect.begin());
				setString.insert (std::string (&*vect.begin()));
			}
		}
	}
	return setString.size();
}
// ***************************************************************************
void CTileBank::computeXRef ()
{
	_tileXRef.resize (_tileVector.size());
	for (int s=0; s<(sint)_tileSetVector.size(); s++)
	{
		int t;
		CTileSet *tileSet=getTileSet (s);
		for (t=0; t<tileSet->getNumTile128(); t++)
		{
			int index=tileSet->getTile128 (t);
			_tileXRef[index]._xRefTileSet=s;
			_tileXRef[index]._xRefTileNumber=t;
			_tileXRef[index]._xRefTileType=_128x128;
		}
		for (t=0; t<tileSet->getNumTile256(); t++)
		{
			int index=tileSet->getTile256 (t);
			_tileXRef[index]._xRefTileSet=s;
			_tileXRef[index]._xRefTileNumber=t;
			_tileXRef[index]._xRefTileType=_256x256;
		}
		for (t=0; t<CTileSet::count; t++)
		{
			int index=tileSet->getTransition (t)->getTile();
			_tileXRef[index]._xRefTileSet=s;
			_tileXRef[index]._xRefTileNumber=t;
			_tileXRef[index]._xRefTileType=transition;
		}
	}
}
// ***************************************************************************
void CTileBank::getTileXRef (int tile, int &tileSet, int &number, TTileType& type) const
{
	nlassert (tile>=0);
	nlassert (tile<(sint)_tileXRef.size());
	tileSet=_tileXRef[tile]._xRefTileSet;
	number=_tileXRef[tile]._xRefTileNumber;
	type=_tileXRef[tile]._xRefTileType;
}


// ***************************************************************************
// ***************************************************************************
// CTile.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
const sint CTile::_version=1;
// ***************************************************************************
void CTile::serial(IStream &f)
{
	sint streamver = f.serialVersion(_version);

	switch (streamver)
	{
	case 1:
		f.serial (_invert);
		// Don't break, read the version 1
	case 0:
		f.serial (_free);
		f.serial (_bitmapName[diffuse]);
		f.serial (_bitmapName[additive]);
		f.serial (_bitmapName[bump]);
		break;
	}
}
// ***************************************************************************
void CTile::clearTile (CTile::TBitmap type)
{
	_bitmapName[type]="";
}


// ***************************************************************************
// ***************************************************************************
// CTileSet.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
const sint CTileSet::_version=0;
// ***************************************************************************
const char* CTileSet::_errorMessage[CTileSet::errorCount]=
{
	"No error.",								// ok
	"Top interface is incompatible.",			// topInterfaceProblem
	"Bottom interface is incompatible.",		// bottomInterfaceProblem
	"Left interface is incompatible.",			// leftInterfaceProblem
	"Right interface is incompatible.",			// rightInterfaceProblem
	"Add first a 128x128 tile.",				// addFirstA128128
	"Top and bottom interface not the same.",	// topBottomNotTheSame, 
	"Right and left interface not the same.",	// rightLeftNotTheSame
	"Invalide bitmap size.",					// sizeInvalide
};
// ***************************************************************************
const CTileSet::TFlagBorder CTileSet::_transitionFlags[CTileSet::count][4]=
{
	{_0000,_1111,_0111,_0111},	// tile 0
	{_0111,_1111,_0111,_1111},	// tile 1
	{_0000,_0111,_0000,_0111},	// tile 2
	{_1110,_1110,_1111,_0000},	// tile 3
	{_1110,_1111,_1111,_0111},	// tile 4
	{_0000,_1110,_0111,_0000},	// tile 5

	{_0000,_1111,_0001,_0001},	// tile 6
	{_0000,_1000,_0001,_0000},	// tile 7
	{_1111,_1000,_1111,_1000},	// tile 8
	{_1000,_1000,_1111,_0000},	// tile 9
	{_1000,_0000,_1000,_0000},	// tile 10
	{_1111,_0001,_1000,_1111},	// tile 11

	{_0000,_1111,_0111,_0001},	// tile 12
	{_0000,_1111,_0001,_0111},	// tile 13
	{_0111,_1111,_0001,_1111},	// tile 14
	{_1110,_1000,_1111,_0000},	// tile 15
	{_1000,_1110,_1111,_0000},	// tile 16
	{_1111,_0001,_1110,_1111},	// tile 17

	{_1000,_0000,_1110,_0000},	// tile 18
	{_0000,_0111,_0000,_0001},	// tile 19
	{_1111,_1000,_1111,_1110},	// tile 21
	{_0111,_0000,_0000,_1000},	// tile 21
	{_0000,_1000,_0111,_0000},	// tile 22
	{_1111,_0111,_1000,_1111},	// tile 23

	{_1111,_0000,_1110,_1110},	// tile 24
	{_1111,_1110,_1111,_1110},	// tile 25
	{_1110,_0000,_1110,_0000},	// tile 26
	{_0111,_0111,_0000,_1111},	// tile 27
	{_1111,_0111,_1110,_1111},	// tile 28
	{_0111,_0000,_0000,_1110},	// tile 29

	{_1111,_0000,_1000,_1000},	// tile 30
	{_0001,_0000,_0000,_1000},	// tile 31
	{_0001,_1111,_0001,_1111},	// tile 32
	{_0001,_0001,_0000,_1111},	// tile 33
	{_0000,_0001,_0000,_0001},	// tile 34
	{_1000,_1111,_1111,_0001},	// tile 35

	{_1111,_0000,_1000,_1110},	// tile 36
	{_1111,_0000,_1110,_1000},	// tile 37
	{_1000,_1111,_1111,_0111},	// tile 38
	{_0001,_0111,_0000,_1111},	// tile 39
	{_0111,_0001,_0000,_1111},	// tile 40
	{_1111,_1110,_1111,_1000},	// tile 41

	{_0000,_0001,_0000,_0111},	// tile 42
	{_1110,_0000,_1000,_0000},	// tile 43
	{_0001,_1111,_0111,_1111},	// tile 44
	{_0000,_1110,_0001,_0000},	// tile 45
	{_0001,_0000,_0000,_1110},	// tile 46
	{_1110,_1111,_1111,_0001}	// tile 47
};
// ***************************************************************************
void CTileSet::setName (const std::string& name)
{
	_name=name;
}
// ***************************************************************************
const std::string& CTileSet::getName () const
{
	return _name;
}
// ***************************************************************************
void CTileSet::serial(IStream &f)
{
	sint streamver = f.serialVersion(_version);

	int i;
	f.serial (_name);
	f.serialCont (_tile128);
	f.serialCont (_tile256);
	for (i=0; i<count; i++)
		f.serial (_tileTransition[i]);
	f.serialCont (_childName);
	f.serial (_border128[CTile::diffuse]);
	f.serial (_border128[CTile::additive]);
	f.serial (_border128[CTile::bump]);
	f.serial (_border256[CTile::diffuse]);
	f.serial (_border256[CTile::additive]);
	f.serial (_border256[CTile::bump]);
	for (i=0; i<count; i++)
	{
		f.serial (_borderTransition[i][CTile::diffuse]);
		f.serial (_borderTransition[i][CTile::additive]);
		f.serial (_borderTransition[i][CTile::bump]);
	}
}
// ***************************************************************************
void CTileSet::addTile128 (int& indexInTileSet, CTileBank& bank)
{
	// Create a tile
	sint index=bank.createTile ();

	// Index of the new tile
	indexInTileSet=_tile128.size();

	// Add to the end of the list
	_tile128.push_back (index);
}
// ***************************************************************************
void CTileSet::setBorder (CTile::TBitmap type, const CTileBorder& border)
{
	// This is our new border desc
	_border128[type]=border;
	_border256[type]=border;
	_border256[type].doubleSize ();
}
// ***************************************************************************
void CTileSet::setTile128 (int indexInTileSet, const std::string& name, CTile::TBitmap type, CTileBank& bank)
{
	// Edit a tile
	CTile *tile=bank.getTile (_tile128[indexInTileSet]);
	tile->setFileName (type, name);
}
// ***************************************************************************
CTileSet::TError CTileSet::checkTile128 (CTile::TBitmap type, const CTileBorder& border, int& pixel, int& composante)
{
	// Self check
	if ((border.getWidth()!=128)||(border.getHeight()!=128))
		return sizeInvalide;
	if (!CTileBorder::compare (border, border, CTileBorder::top, CTileBorder::bottom, pixel, composante))
		return topBottomNotTheSame;
	if (!CTileBorder::compare (border, border, CTileBorder::left, CTileBorder::right, pixel, composante))
		return rightLeftNotTheSame;

	// Check
	if (_border128[type].isSet())
	{

		// Other check
		if (!CTileBorder::compare (border, _border128[type], CTileBorder::top, CTileBorder::top, pixel, composante))
			return topInterfaceProblem;
		if (!CTileBorder::compare (border, _border128[type], CTileBorder::bottom, CTileBorder::bottom, pixel, composante))
			return bottomInterfaceProblem;
		if (!CTileBorder::compare (border, _border128[type], CTileBorder::left, CTileBorder::left, pixel, composante))
			return leftInterfaceProblem;
		if (!CTileBorder::compare (border, _border128[type], CTileBorder::right, CTileBorder::right, pixel, composante))
			return rightInterfaceProblem;
	}
	else
	{		
		return addFirstA128128;
	}

	return ok;
}
// ***************************************************************************
void CTileSet::addTile256 (int& indexInTileSet, CTileBank& bank)
{
	// Create a tile
	sint index=bank.createTile ();

	// Index of the new tile
	indexInTileSet=_tile256.size();

	// Add to the end of the list
	_tile256.push_back (index);
}
// ***************************************************************************
CTileSet::TError CTileSet::checkTile256 (CTile::TBitmap type, const CTileBorder& border, int& pixel, int& composante)
{
	// Self check
	if ((border.getWidth()!=256)||(border.getHeight()!=256))
		return sizeInvalide;
	if (!CTileBorder::compare (border, border, CTileBorder::top, CTileBorder::bottom, pixel, composante))
		return topBottomNotTheSame;
	if (!CTileBorder::compare (border, border, CTileBorder::left, CTileBorder::right, pixel, composante))
		return rightLeftNotTheSame;

	// Check
	if (_border256[type].isSet())
	{
		// Other check
		if (!CTileBorder::compare (border, _border256[type], CTileBorder::top, CTileBorder::top, pixel, composante))
			return topInterfaceProblem;
		if (!CTileBorder::compare (border, _border256[type], CTileBorder::bottom, CTileBorder::bottom, pixel, composante))
			return bottomInterfaceProblem;
		if (!CTileBorder::compare (border, _border256[type], CTileBorder::left, CTileBorder::left, pixel, composante))
			return leftInterfaceProblem;
		if (!CTileBorder::compare (border, _border256[type], CTileBorder::right, CTileBorder::right, pixel, composante))
			return rightInterfaceProblem;
	}
	else
	{
		return addFirstA128128;
	}

	return ok;
}
// ***************************************************************************
void CTileSet::setTile256 (int indexInTileSet, const std::string& name, CTile::TBitmap type, CTileBank& bank)
{
	// Edit a tile
	CTile *tile=bank.getTile (_tile256[indexInTileSet]);
	tile->setFileName (type, name);
}
// ***************************************************************************
void CTileSet::setTileTransition (TTransition transition, const std::string& name, CTile::TBitmap type,	CTileBank& bank, 
											  const CTileBorder& border, bool bInvert)
{
	// Create a tile
	_borderTransition[transition][type]=border;

	// Invert alpha if needed. All border always not inverted
	if (bInvert)
		_borderTransition[transition][type].invertAlpha();

	// Set the tile file name
	CTile *tile=bank.getTile (_tileTransition[transition]._tile);
	tile->setFileName (type, name);
}
// ***************************************************************************
CTileSet::TError CTileSet::checkTileTransition (TTransition transition, CTile::TBitmap type, const CTileBorder& border, int& indexError,
		int& pixel, int& composante, bool bInvert)
{
	nlassert (transition>=0);
	nlassert (transition<count);

	// Check
	indexError=-1;
	if (_border128[type].isSet())
	{
		// Top
		indexError=getExistingTransitionTile ((TFlagBorder)_transitionFlags[transition][top], dontcare, dontcare, dontcare, transition, type);
		if (indexError!=-1)
		{
			if (!CTileBorder::compare (border, _borderTransition[indexError][type], CTileBorder::top, CTileBorder::top, pixel, composante, bInvert, false))
				return topInterfaceProblem;
		}
		indexError=getExistingTransitionTile (dontcare, (TFlagBorder)_transitionFlags[transition][top], dontcare, dontcare, transition, type);
		if (indexError!=-1)
		{
			if (!CTileBorder::compare (border, _borderTransition[indexError][type], CTileBorder::top, CTileBorder::bottom, pixel, composante, bInvert, false))
				return topInterfaceProblem;
		}
		indexError=-1;
		if (_transitionFlags[transition][top]==_1111)
		{
			if (!CTileBorder::compare (border, _border128[type], CTileBorder::top, CTileBorder::top, pixel, composante, bInvert, false))
				return topInterfaceProblem;
		}

		// Bottom
		indexError=getExistingTransitionTile (dontcare, (TFlagBorder)_transitionFlags[transition][bottom], dontcare, dontcare, transition, type);
		if (indexError!=-1)
		{
			if (!CTileBorder::compare (border, _borderTransition[indexError][type], CTileBorder::bottom, CTileBorder::bottom, pixel, composante, bInvert, false))
				return bottomInterfaceProblem;
		}
		indexError=getExistingTransitionTile ((TFlagBorder)_transitionFlags[transition][bottom], dontcare, dontcare, dontcare, transition, type);
		if (indexError!=-1)
		{
			if (!CTileBorder::compare (border, _borderTransition[indexError][type], CTileBorder::bottom, CTileBorder::top, pixel, composante, bInvert, false))
				return bottomInterfaceProblem;
		}
		indexError=-1;
		if (_transitionFlags[transition][bottom]==_1111)
		{
			if (!CTileBorder::compare (border, _border128[type], CTileBorder::bottom, CTileBorder::bottom, pixel, composante, bInvert, false))
				return bottomInterfaceProblem;
		}

		// Left
		indexError=getExistingTransitionTile (dontcare, dontcare, (TFlagBorder)_transitionFlags[transition][left], dontcare, transition, type);
		if (indexError!=-1)
		{
			if (!CTileBorder::compare (border, _borderTransition[indexError][type], CTileBorder::left, CTileBorder::left, pixel, composante, bInvert, false))
				return leftInterfaceProblem;
		}
		indexError=getExistingTransitionTile (dontcare, dontcare, dontcare, (TFlagBorder)_transitionFlags[transition][left], transition, type);
		if (indexError!=-1)
		{
			if (!CTileBorder::compare (border, _borderTransition[indexError][type], CTileBorder::left, CTileBorder::right, pixel, composante, bInvert, false))
				return leftInterfaceProblem;
		}
		indexError=-1;
		if (_transitionFlags[transition][left]==_1111)
		{
			if (!CTileBorder::compare (border, _border128[type], CTileBorder::left, CTileBorder::left, pixel, composante, bInvert, false))
				return leftInterfaceProblem;
		}

		// Right
		indexError=getExistingTransitionTile (dontcare, dontcare, dontcare, (TFlagBorder)_transitionFlags[transition][right], transition, type);
		if (indexError!=-1)
		{
			if (!CTileBorder::compare (border, _borderTransition[indexError][type], CTileBorder::right, CTileBorder::right, pixel, composante, bInvert, false))
				return rightInterfaceProblem;
		}
		indexError=getExistingTransitionTile (dontcare, dontcare, (TFlagBorder)_transitionFlags[transition][right], dontcare, transition, type);
		if (indexError!=-1)
		{
			if (!CTileBorder::compare (border, _borderTransition[indexError][type], CTileBorder::right, CTileBorder::left, pixel, composante, bInvert, false))
				return rightInterfaceProblem;
		}
		indexError=-1;
		if (_transitionFlags[transition][right]==_1111)
		{
			if (!CTileBorder::compare (border, _border128[type], CTileBorder::right, CTileBorder::right, pixel, composante, bInvert, false))
				return rightInterfaceProblem;
		}
		return ok;
	}
	else
	{
		return addFirstA128128;
	}
}
// ***************************************************************************
void CTileSet::removeTile128 (int indexInTileSet, CTileBank& bank)
{
	// Check args
	nlassert (indexInTileSet>=0);
	nlassert (indexInTileSet<(sint)_tile128.size());

	// Old index
	int index=_tile128[indexInTileSet];

	// Erase
	_tile128.erase (_tile128.begin()+indexInTileSet);
	bank.freeTile (index);
}
// ***************************************************************************
void CTileSet::removeTile256 (int indexInTileSet, CTileBank& bank)
{
	// Check args
	nlassert (indexInTileSet>=0);
	nlassert (indexInTileSet<(sint)_tile256.size());

	// Old index
	int index=_tile256[indexInTileSet];

	// Erase
	_tile256.erase (_tile256.begin()+indexInTileSet);
	bank.freeTile (index);
}
// ***************************************************************************
CTileSet::TTransition CTileSet::getTransitionTile (TFlagBorder _top, TFlagBorder _bottom, TFlagBorder _left, TFlagBorder _right)
{
	for (int n=first; n<count; n++)
	{
		if (((_top==dontcare)||(_top==(TFlagBorder)_transitionFlags[n][top]))&&
			((_bottom==dontcare)||(_bottom==(TFlagBorder)_transitionFlags[n][bottom]))&&
			((_left==dontcare)||(_left==(TFlagBorder)_transitionFlags[n][left]))&&
			((_right==dontcare)||(_right==(TFlagBorder)_transitionFlags[n][right])))
		{
			return (TTransition)n;
		}	
	}
	return notfound;
}
// ***************************************************************************
CTileSet::TTransition CTileSet::getExistingTransitionTile (TFlagBorder _top, TFlagBorder _bottom, TFlagBorder _left, TFlagBorder _right, int reject, CTile::TBitmap type)
{
	for (int n=first; n<count; n++)
	{
		if ((n!=reject)&&
			(_borderTransition[n][type].isSet ())&&
			((_top==dontcare)||(_top==(TFlagBorder)_transitionFlags[n][top]))&&
			((_bottom==dontcare)||(_bottom==(TFlagBorder)_transitionFlags[n][bottom]))&&
			((_left==dontcare)||(_left==(TFlagBorder)_transitionFlags[n][left]))&&
			((_right==dontcare)||(_right==(TFlagBorder)_transitionFlags[n][right])))
		{
			return (TTransition)n;
		}	
	}
	return notfound;
}
// ***************************************************************************
void CTileSet::addChild (const std::string& name)
{
	_childName.insert (name);
}
// ***************************************************************************
void CTileSet::removeChild (const std::string& name)
{
	_childName.erase (name);
}
// ***************************************************************************
CTileSet::TTransition CTileSet::getComplementaryTransition (TTransition transition)
{
	nlassert ((transition>=first)&&(transition<=last));
	TTransition trans=getTransitionTile (getComplementaryBorder (_transitionFlags[transition][top]),
		getComplementaryBorder (_transitionFlags[transition][bottom]),
		getComplementaryBorder (_transitionFlags[transition][left]),
		getComplementaryBorder (_transitionFlags[transition][right]));
	
	nlassert (trans!=notfound);
	
	return trans;
}
// ***************************************************************************
CTileSet::TFlagBorder CTileSet::getComplementaryBorder (TFlagBorder border)
{
	switch (border)
	{
	case _0000:
		return _1111;
	case _0001:
		return _1110;
	case _0111:
		return _1000;
	case _1000:
		return _0111;
	case _1110:
		return _0001;
	case _1111:
		return _0000;
	default:
		nlassert (0);	// no
	}
	return _0000;
}
// ***************************************************************************
CTileSet::TFlagBorder CTileSet::getInvertBorder (TFlagBorder border)
{
	switch (border)
	{
	case _0000:
		return _0000;
	case _0001:
		return _1000;
	case _0111:
		return _1110;
	case _1000:
		return _0001;
	case _1110:
		return _0111;
	case _1111:
		return _1111;
	default:
		nlassert (0);	// no
	}
	return _0000;
}
// ***************************************************************************
CTileSet::TFlagBorder CTileSet::getOrientedBorder (TBorder where, TFlagBorder border)
{
	switch (where)
	{
	case left:
	case bottom:
		return border;
	case top:
	case right:
		return getInvertBorder (border);
	default:
		nlassert (0);	// no
	}
	return _0000;
}
// ***************************************************************************
void CTileSet::ClearTransition (TTransition transition, CTile::TBitmap type, CTileBank& bank)
{
	int nTile=_tileTransition[transition]._tile;
	if (nTile!=-1)
		bank.getTile (nTile)->clearTile(type);
	_borderTransition[transition][type].reset();
}


// ***************************************************************************
// ***************************************************************************
// CTileBorder.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
const sint CTileBorder::_version=0;
// ***************************************************************************
void CTileBorder::serial(IStream &f)
{
	sint streamver = f.serialVersion(_version);

	f.serial (_set);
	f.serial (_width);
	f.serial (_height);
	f.serialCont (_borders[top]);
	f.serialCont (_borders[bottom]);
	f.serialCont (_borders[left]);
	f.serialCont (_borders[right]);
}
// ***************************************************************************
void CTileBorder::set (int width, int height, const std::vector<CBGRA>& array)
{
	// Check array size
	nlassert (width>0);
	nlassert (height>0);
	nlassert ((sint)array.size()==width*height);

	// Copy size
	_width=width;
	_height=height;

	// Last line
	int lastLine=(_height-1)*width;
	int lastCol=(_width-1);
	_borders[top].resize (_width);
	_borders[bottom].resize (_width);
	_borders[left].resize (_height);
	_borders[right].resize (_height);

	// Copy top/bottom border
	for (int w=0; w<_width; w++)
	{
		_borders[top][w]=array[w];
		_borders[bottom][w]=array[w+lastLine];
	}

	// Copy left/right border
	for (int h=0; h<_height; h++)
	{
		_borders[left][h]=array[h*_width];
		_borders[right][h]=array[h*_width+lastCol];
	}

	// Set
	_set=true;
}
// ***************************************************************************
bool CTileBorder::compare (const CTileBorder& border1, const CTileBorder& border2, TBorder where1, TBorder where2, int& pixel, int& composante, bool bInvertFirst, bool bInvertSecond)
{
	// Check border is initialized
	nlassert (border1.isSet());
	nlassert (border2.isSet());

	if (border1._borders[where1].size()!=border2._borders[where2].size())
		return false;
	for (pixel=0; pixel<(int)border1._borders[where1].size(); pixel++)
	{
		pixel=pixel;
		if (border1._borders[where1][pixel].R!=border2._borders[where2][pixel].R)
		{
			composante=0;
			return false;
		}
		else if (border1._borders[where1][pixel].G!=border2._borders[where2][pixel].G)
		{
			composante=1;
			return false;
		}
		else if (border1._borders[where1][pixel].B!=border2._borders[where2][pixel].B)
		{
			composante=2;
			return false;
		}
		else
		{
			int alpha1=border1._borders[where1][pixel].A;
			int alpha2=border2._borders[where2][pixel].A;
			if ((bInvertFirst?(255-alpha1):alpha1)!=(bInvertSecond?(255-alpha2):alpha2))
			{
				composante=3;
				return false;
			}
		}
	}

	return true;
}
// ***************************************************************************
bool CTileBorder::operator== (const CTileBorder& border) const
{
	return (_width==border._width) && (_height==border._height) && (_borders==border._borders);
}
// ***************************************************************************
void CTileBorder::operator= (const CTileBorder& border)
{
	_set=border._set;
	_width=border._width;
	_height=border._width;
	_borders[top]=border._borders[top];
	_borders[bottom]=border._borders[bottom];
	_borders[left]=border._borders[left];
	_borders[right]=border._borders[right];
}
	
// ***************************************************************************
void CTileBorder::doubleSize ()
{
	_borders[top].resize (_width*2);
	_borders[bottom].resize (_width*2);
	_borders[left].resize (_height*2);
	_borders[right].resize (_height*2);

	for (int w=0; w<_width; w++)
	{
		_borders[top][w+_width]=_borders[top][w];
		_borders[bottom][w+_width]=_borders[bottom][w];
	}
	for (int h=0; h<_height; h++)
	{
		_borders[left][h+_height]=_borders[left][h];
		_borders[right][h+_height]=_borders[right][h];
	}
}
// ***************************************************************************
void CTileBorder::invertAlpha()
{
	for (int b=0; b<borderCount; b++)
	{
		for (int s=0; s<(int)_borders[b].size(); s++)
			_borders[b][s].A=255-_borders[b][s].A;
	}
}

// ***************************************************************************
// ***************************************************************************
// CTileSetTransition.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
const sint CTileSetTransition::_version=1;
// ***************************************************************************
void CTileSetTransition::serial(class NLMISC::IStream &f)
{
	sint streamver = f.serialVersion(_version);

	switch (streamver)
	{
	case 0:
		{
			bool doomy;
			f.serial (_tile);
			f.serial (doomy);		// skip the old argu
		}
		break;
	case 1:
		f.serial (_tile);
		break;
	}
}


}	// NL3D
