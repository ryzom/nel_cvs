/** \file tile_bank.cpp
 * Management of tile texture.
 *
 * $Id: tile_bank.cpp,v 1.17 2001/01/09 14:31:59 corvazier Exp $
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
const sint CTileLand::_Version=0;
// ***************************************************************************
void CTileLand::serial(IStream &f) throw(EStream)
{
	sint streamver = f.serialVersion(_Version);

	f.serial (_Name);
	f.serialCont (_TileSet);
}
// ***************************************************************************
void CTileLand::addTileSet (const std::string& name)
{
	_TileSet.insert (name);
}
// ***************************************************************************
void CTileLand::removeTileSet (const std::string& name)
{
	_TileSet.erase (name);
}
// ***************************************************************************
void CTileLand::setName (const std::string& name)
{ 
	_Name=name;
};


// ***************************************************************************
// ***************************************************************************
// CTileBank.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
const sint CTileBank::_Version=2;
// ***************************************************************************
void    CTileBank::serial(IStream &f) throw(EStream)
{
	f.serialCheck (std::string ("BANK"));

	sint streamver = f.serialVersion(_Version);
	
	// Version 1 not compatible
	if (f.isReading())
	{
		if (streamver<2)
			throw EOlderStream();
	}

	f.serialCont (_LandVector);
	f.serialCont (_TileSetVector);
	f.serialCont (_TileVector);
}
// ***************************************************************************
sint CTileBank::addLand (const std::string& name)
{
	sint last=_LandVector.size();
	_LandVector.push_back(CTileLand());
	_LandVector[last].setName (name);
	return last;
}
// ***************************************************************************
void CTileBank::removeLand (sint landIndex)
{
	// Check args
	nlassert (landIndex>=0);
	nlassert (landIndex<(sint)_LandVector.size());

	_LandVector.erase (_LandVector.begin ()+landIndex);
}
// ***************************************************************************
sint CTileBank::addTileSet (const std::string& name)
{
	sint last=_TileSetVector.size();
	_TileSetVector.push_back(CTileSet());
	_TileSetVector[last].setName (name);
	for (int i=0; i<CTileSet::count; i++)
	{
		_TileSetVector[last]._TileTransition[i]._Tile=createTile ();
	}
	return last;
}
// ***************************************************************************
void CTileBank::removeTileSet (sint setIndex)
{
	// Check args
	nlassert (setIndex>=0);
	nlassert (setIndex<(sint)_TileSetVector.size());

	for (int i=0; i<CTileSet::count; i++)
	{
		int index=_TileSetVector[setIndex]._TileTransition[i]._Tile;
		if (index!=-1)
			freeTile (index);
	}
	_TileSetVector.erase (_TileSetVector.begin ()+setIndex);
}
// ***************************************************************************
void CTileBank::clear ()
{
	_LandVector.clear ();
	_TileSetVector.clear ();
	_TileVector.clear ();
}
// ***************************************************************************
sint CTileBank::createTile ()
{
	// Look for a free tile
	for (int i=0; i<(sint)_TileVector.size(); i++)
	{
		if (_TileVector[i].isFree())
		{
			_TileVector[i].setFileName (CTile::diffuse, "");
			_TileVector[i].setFileName (CTile::additive, "");
			_TileVector[i].setFileName (CTile::bump, "");
			return i;
		}
	}

	// Nothing free, add a tile at the end
	_TileVector.push_back (CTile());
	_TileVector[_TileVector.size()-1].setFileName (CTile::diffuse, "");
	_TileVector[_TileVector.size()-1].setFileName (CTile::additive, "");
	_TileVector[_TileVector.size()-1].setFileName (CTile::bump, "");
	return _TileVector.size()-1;
}
// ***************************************************************************
void CTileBank::freeTile (int tileIndex)
{
	// Check args
	nlassert (tileIndex>=0);
	nlassert (tileIndex<(sint)_TileVector.size());

	// Free
	_TileVector[tileIndex].free();

	// Resize tile table
	int i;
	for (i=(sint)_TileVector.size()-1; i>=0; i--)
	{
		if (!_TileVector[i].isFree ())
			break;
	}
	if (i<(sint)_TileVector.size()-1)
		_TileVector.resize (i+1);
}
// ***************************************************************************
sint CTileBank::getNumBitmap (CTile::TBitmap bitmap) const
{
	std::set<std::string> setString;
	for (int i=0; i<(sint)_TileVector.size(); i++)
	{
		if (!_TileVector[i].isFree())
		{
			const std::string &str=_TileVector[i].getFileName (bitmap);
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
	_TileXRef.resize (_TileVector.size());
	for (int s=0; s<(sint)_TileSetVector.size(); s++)
	{
		int t;
		CTileSet *tileSet=getTileSet (s);
		for (t=0; t<tileSet->getNumTile128(); t++)
		{
			int index=tileSet->getTile128 (t);
			_TileXRef[index]._XRefTileSet=s;
			_TileXRef[index]._XRefTileNumber=t;
			_TileXRef[index]._XRefTileType=_128x128;
		}
		for (t=0; t<tileSet->getNumTile256(); t++)
		{
			int index=tileSet->getTile256 (t);
			_TileXRef[index]._XRefTileSet=s;
			_TileXRef[index]._XRefTileNumber=t;
			_TileXRef[index]._XRefTileType=_256x256;
		}
		for (t=0; t<CTileSet::count; t++)
		{
			int index=tileSet->getTransition (t)->getTile();
			_TileXRef[index]._XRefTileSet=s;
			_TileXRef[index]._XRefTileNumber=t;
			_TileXRef[index]._XRefTileType=transition;
		}
	}
}
// ***************************************************************************
void CTileBank::getTileXRef (int tile, int &tileSet, int &number, TTileType& type) const
{
	nlassert (tile>=0);
	nlassert (tile<(sint)_TileXRef.size());
	tileSet=_TileXRef[tile]._XRefTileSet;
	number=_TileXRef[tile]._XRefTileNumber;
	type=_TileXRef[tile]._XRefTileType;
}
// ***************************************************************************
void CTileBank::xchgTileset (sint firstTileSet, sint secondTileSet)
{
	// Some check
	nlassert ((firstTileSet>=0)&&(firstTileSet<(sint)_TileSetVector.size()));
	nlassert ((secondTileSet>=0)&&(secondTileSet<(sint)_TileSetVector.size()));

	// Xchange the sets
	CTileSet tmp=_TileSetVector[firstTileSet];
	_TileSetVector[firstTileSet]=_TileSetVector[secondTileSet];
	_TileSetVector[secondTileSet]=tmp;
}
// ***************************************************************************
void TroncFileName (char* sDest, const char* sSrc)
{
	char* ptr=strrchr (sSrc, '\\');
	if (ptr==NULL)
		ptr=strrchr (sSrc, '/');
	if (ptr)
	{
		ptr++;
		strcpy (sDest, ptr);
	}
	else
	{
		strcpy (sDest, sSrc);
	}
}
// ***************************************************************************
// Je parie que ce patch reste jusqu'à la fin du jeu. (Hulud)
void CTileBank::makeAllPathRelative ()
{
	// For all tiles
	for (sint nTile=0; nTile<(sint)_TileVector.size(); nTile++)
	{
		// Tronc filename
		char sTmpFileName[512];

		// Diffuse
		TroncFileName (sTmpFileName, _TileVector[nTile].getFileName (CTile::diffuse).c_str());
		_TileVector[nTile].setFileName (CTile::diffuse, sTmpFileName);

		// Additive
		TroncFileName (sTmpFileName, _TileVector[nTile].getFileName (CTile::additive).c_str());
		_TileVector[nTile].setFileName (CTile::additive, sTmpFileName);

		// Bump
		TroncFileName (sTmpFileName, _TileVector[nTile].getFileName (CTile::bump).c_str());
		_TileVector[nTile].setFileName (CTile::bump, sTmpFileName);
	}
}


// ***************************************************************************
// ***************************************************************************
// CTile.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
const sint CTile::_Version=1;
// ***************************************************************************
void CTile::serial(IStream &f) throw(EStream)
{
	sint streamver = f.serialVersion(_Version);

	switch (streamver)
	{
	case 1:
		f.serial (_Invert);				// Don't break, read the version 0
	case 0:
		f.serial (_Free);
		f.serial (_BitmapName[diffuse]);
		f.serial (_BitmapName[additive]);
		f.serial (_BitmapName[bump]);
		break;
	}
}
// ***************************************************************************
void CTile::clearTile (CTile::TBitmap type)
{
	_BitmapName[type]="";
}


// ***************************************************************************
// ***************************************************************************
// CTileSet.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
const sint CTileSet::_Version=0;
// ***************************************************************************
const char* CTileSet::_ErrorMessage[CTileSet::errorCount]=
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
const CTileSet::TFlagBorder CTileSet::_TransitionFlags[CTileSet::count][4]=
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
	_Name=name;
}
// ***************************************************************************
const std::string& CTileSet::getName () const
{
	return _Name;
}
// ***************************************************************************
void CTileSet::serial(IStream &f) throw(EStream)
{
	sint streamver = f.serialVersion(_Version);

	int i;
	f.serial (_Name);
	f.serialCont (_Tile128);
	f.serialCont (_Tile256);
	for (i=0; i<count; i++)
		f.serial (_TileTransition[i]);
	f.serialCont (_ChildName);
	f.serial (_Border128[CTile::diffuse]);
	f.serial (_Border128[CTile::additive]);
	f.serial (_Border128[CTile::bump]);
	f.serial (_Border256[CTile::diffuse]);
	f.serial (_Border256[CTile::additive]);
	f.serial (_Border256[CTile::bump]);
	for (i=0; i<count; i++)
	{
		f.serial (_BorderTransition[i][CTile::diffuse]);
		f.serial (_BorderTransition[i][CTile::additive]);
		f.serial (_BorderTransition[i][CTile::bump]);
	}
}
// ***************************************************************************
void CTileSet::addTile128 (int& indexInTileSet, CTileBank& bank)
{
	// Create a tile
	sint index=bank.createTile ();

	// Index of the new tile
	indexInTileSet=_Tile128.size();

	// Add to the end of the list
	_Tile128.push_back (index);
}
// ***************************************************************************
void CTileSet::setBorder (CTile::TBitmap type, const CTileBorder& border)
{
	// This is our new border desc
	_Border128[type]=border;
	_Border256[type]=border;
	_Border256[type].doubleSize ();
}
// ***************************************************************************
void CTileSet::setTile128 (int indexInTileSet, const std::string& name, CTile::TBitmap type, CTileBank& bank)
{
	// Edit a tile
	CTile *tile=bank.getTile (_Tile128[indexInTileSet]);
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
	if (_Border128[type].isSet())
	{

		// Other check
		if (!CTileBorder::compare (border, _Border128[type], CTileBorder::top, CTileBorder::top, pixel, composante))
			return topInterfaceProblem;
		if (!CTileBorder::compare (border, _Border128[type], CTileBorder::bottom, CTileBorder::bottom, pixel, composante))
			return bottomInterfaceProblem;
		if (!CTileBorder::compare (border, _Border128[type], CTileBorder::left, CTileBorder::left, pixel, composante))
			return leftInterfaceProblem;
		if (!CTileBorder::compare (border, _Border128[type], CTileBorder::right, CTileBorder::right, pixel, composante))
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
	indexInTileSet=_Tile256.size();

	// Add to the end of the list
	_Tile256.push_back (index);
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
	if (_Border256[type].isSet())
	{
		// Other check
		if (!CTileBorder::compare (border, _Border256[type], CTileBorder::top, CTileBorder::top, pixel, composante))
			return topInterfaceProblem;
		if (!CTileBorder::compare (border, _Border256[type], CTileBorder::bottom, CTileBorder::bottom, pixel, composante))
			return bottomInterfaceProblem;
		if (!CTileBorder::compare (border, _Border256[type], CTileBorder::left, CTileBorder::left, pixel, composante))
			return leftInterfaceProblem;
		if (!CTileBorder::compare (border, _Border256[type], CTileBorder::right, CTileBorder::right, pixel, composante))
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
	CTile *tile=bank.getTile (_Tile256[indexInTileSet]);
	tile->setFileName (type, name);
}
// ***************************************************************************
void CTileSet::setTileTransition (TTransition transition, const std::string& name, CTile::TBitmap type,	CTileBank& bank, 
											  const CTileBorder& border, bool bInvert)
{
	// Create a tile
	_BorderTransition[transition][type]=border;

	// Invert alpha if needed. All border always not inverted
	if (bInvert)
		_BorderTransition[transition][type].invertAlpha();

	// Set the tile file name
	CTile *tile=bank.getTile (_TileTransition[transition]._Tile);
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
	if (_Border128[type].isSet())
	{
		// Top
		indexError=getExistingTransitionTile ((TFlagBorder)_TransitionFlags[transition][top], dontcare, dontcare, dontcare, transition, type);
		if (indexError!=-1)
		{
			if (!CTileBorder::compare (border, _BorderTransition[indexError][type], CTileBorder::top, CTileBorder::top, pixel, composante, bInvert, false))
				return topInterfaceProblem;
		}
		indexError=getExistingTransitionTile (dontcare, (TFlagBorder)_TransitionFlags[transition][top], dontcare, dontcare, transition, type);
		if (indexError!=-1)
		{
			if (!CTileBorder::compare (border, _BorderTransition[indexError][type], CTileBorder::top, CTileBorder::bottom, pixel, composante, bInvert, false))
				return topInterfaceProblem;
		}
		indexError=-1;
		if (_TransitionFlags[transition][top]==_1111)
		{
			if (!CTileBorder::compare (border, _Border128[type], CTileBorder::top, CTileBorder::top, pixel, composante, bInvert, false))
				return topInterfaceProblem;
		}

		// Bottom
		indexError=getExistingTransitionTile (dontcare, (TFlagBorder)_TransitionFlags[transition][bottom], dontcare, dontcare, transition, type);
		if (indexError!=-1)
		{
			if (!CTileBorder::compare (border, _BorderTransition[indexError][type], CTileBorder::bottom, CTileBorder::bottom, pixel, composante, bInvert, false))
				return bottomInterfaceProblem;
		}
		indexError=getExistingTransitionTile ((TFlagBorder)_TransitionFlags[transition][bottom], dontcare, dontcare, dontcare, transition, type);
		if (indexError!=-1)
		{
			if (!CTileBorder::compare (border, _BorderTransition[indexError][type], CTileBorder::bottom, CTileBorder::top, pixel, composante, bInvert, false))
				return bottomInterfaceProblem;
		}
		indexError=-1;
		if (_TransitionFlags[transition][bottom]==_1111)
		{
			if (!CTileBorder::compare (border, _Border128[type], CTileBorder::bottom, CTileBorder::bottom, pixel, composante, bInvert, false))
				return bottomInterfaceProblem;
		}

		// Left
		indexError=getExistingTransitionTile (dontcare, dontcare, (TFlagBorder)_TransitionFlags[transition][left], dontcare, transition, type);
		if (indexError!=-1)
		{
			if (!CTileBorder::compare (border, _BorderTransition[indexError][type], CTileBorder::left, CTileBorder::left, pixel, composante, bInvert, false))
				return leftInterfaceProblem;
		}
		indexError=getExistingTransitionTile (dontcare, dontcare, dontcare, (TFlagBorder)_TransitionFlags[transition][left], transition, type);
		if (indexError!=-1)
		{
			if (!CTileBorder::compare (border, _BorderTransition[indexError][type], CTileBorder::left, CTileBorder::right, pixel, composante, bInvert, false))
				return leftInterfaceProblem;
		}
		indexError=-1;
		if (_TransitionFlags[transition][left]==_1111)
		{
			if (!CTileBorder::compare (border, _Border128[type], CTileBorder::left, CTileBorder::left, pixel, composante, bInvert, false))
				return leftInterfaceProblem;
		}

		// Right
		indexError=getExistingTransitionTile (dontcare, dontcare, dontcare, (TFlagBorder)_TransitionFlags[transition][right], transition, type);
		if (indexError!=-1)
		{
			if (!CTileBorder::compare (border, _BorderTransition[indexError][type], CTileBorder::right, CTileBorder::right, pixel, composante, bInvert, false))
				return rightInterfaceProblem;
		}
		indexError=getExistingTransitionTile (dontcare, dontcare, (TFlagBorder)_TransitionFlags[transition][right], dontcare, transition, type);
		if (indexError!=-1)
		{
			if (!CTileBorder::compare (border, _BorderTransition[indexError][type], CTileBorder::right, CTileBorder::left, pixel, composante, bInvert, false))
				return rightInterfaceProblem;
		}
		indexError=-1;
		if (_TransitionFlags[transition][right]==_1111)
		{
			if (!CTileBorder::compare (border, _Border128[type], CTileBorder::right, CTileBorder::right, pixel, composante, bInvert, false))
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
	nlassert (indexInTileSet<(sint)_Tile128.size());

	// Old index
	int index=_Tile128[indexInTileSet];

	// Erase
	_Tile128.erase (_Tile128.begin()+indexInTileSet);
	bank.freeTile (index);

	// Erase border if it is the last texture
	deleteBordersIfLast (bank, CTile::diffuse);
	deleteBordersIfLast (bank, CTile::additive);
	deleteBordersIfLast (bank, CTile::bump);
}
// ***************************************************************************
void CTileSet::removeTile256 (int indexInTileSet, CTileBank& bank)
{
	// Check args
	nlassert (indexInTileSet>=0);
	nlassert (indexInTileSet<(sint)_Tile256.size());

	// Old index
	int index=_Tile256[indexInTileSet];

	// Erase
	_Tile256.erase (_Tile256.begin()+indexInTileSet);
	bank.freeTile (index);

	// Erase border if it is the last texture
	deleteBordersIfLast (bank, CTile::diffuse);
	deleteBordersIfLast (bank, CTile::additive);
	deleteBordersIfLast (bank, CTile::bump);
}
// ***************************************************************************
CTileSet::TTransition CTileSet::getTransitionTile (TFlagBorder _top, TFlagBorder _bottom, TFlagBorder _left, TFlagBorder _right)
{
	for (int n=first; n<count; n++)
	{
		if (((_top==dontcare)||(_top==(TFlagBorder)_TransitionFlags[n][top]))&&
			((_bottom==dontcare)||(_bottom==(TFlagBorder)_TransitionFlags[n][bottom]))&&
			((_left==dontcare)||(_left==(TFlagBorder)_TransitionFlags[n][left]))&&
			((_right==dontcare)||(_right==(TFlagBorder)_TransitionFlags[n][right])))
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
			(_BorderTransition[n][type].isSet ())&&
			((_top==dontcare)||(_top==(TFlagBorder)_TransitionFlags[n][top]))&&
			((_bottom==dontcare)||(_bottom==(TFlagBorder)_TransitionFlags[n][bottom]))&&
			((_left==dontcare)||(_left==(TFlagBorder)_TransitionFlags[n][left]))&&
			((_right==dontcare)||(_right==(TFlagBorder)_TransitionFlags[n][right])))
		{
			return (TTransition)n;
		}	
	}
	return notfound;
}
// ***************************************************************************
void CTileSet::addChild (const std::string& name)
{
	_ChildName.insert (name);
}
// ***************************************************************************
void CTileSet::removeChild (const std::string& name)
{
	_ChildName.erase (name);
}
// ***************************************************************************
CTileSet::TTransition CTileSet::getComplementaryTransition (TTransition transition)
{
	nlassert ((transition>=first)&&(transition<=last));
	TTransition trans=getTransitionTile (getComplementaryBorder (_TransitionFlags[transition][top]),
		getComplementaryBorder (_TransitionFlags[transition][bottom]),
		getComplementaryBorder (_TransitionFlags[transition][left]),
		getComplementaryBorder (_TransitionFlags[transition][right]));
	
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
void CTileSet::clearTile128 (int indexInTileSet, CTile::TBitmap type, CTileBank& bank)
{
	int nTile=_Tile128[indexInTileSet];
	bank.getTile (nTile)->clearTile(type);
	
	// Erase border if it is the last texture
	deleteBordersIfLast (bank, type);
}
// ***************************************************************************
void CTileSet::clearTile256 (int indexInTileSet, CTile::TBitmap type, CTileBank& bank)
{
	int nTile=_Tile256[indexInTileSet];
	bank.getTile (nTile)->clearTile(type);
	
	// Erase border if it is the last texture
	deleteBordersIfLast (bank, type);
}
// ***************************************************************************
void CTileSet::clearTransition (TTransition transition, CTile::TBitmap type, CTileBank& bank)
{
	int nTile=_TileTransition[transition]._Tile;
	if (nTile!=-1)
		bank.getTile (nTile)->clearTile(type);
	_BorderTransition[transition][type].reset();
	
	// Erase border if it is the last texture
	deleteBordersIfLast (bank, type);
}
// ***************************************************************************
// Delete 128 and 256 borders if no more valid texture file name for each bitmap type.
void CTileSet::deleteBordersIfLast (const CTileBank& bank, CTile::TBitmap type)
{
	// delete is true
	bool bDelete=true;

	// iterator..
	std::vector<sint32>::iterator ite=_Tile128.begin();

	// Check all the 128x128 tiles
	while (ite!=_Tile128.end())
	{
		// If the file name is valid
		if (bank.getTile (*ite)->getFileName(type)!="")
		{
			// Don't delete, 
			bDelete=false;
			break;
		}
		ite++;
	}
	// If break, not empty, return
	if (ite!=_Tile128.end())
		return;

	// Check all the 256x256 tiles
	ite=_Tile256.begin();
	while (ite!=_Tile256.end())
	{
		// If the file name is valid
		if (bank.getTile (*ite)->getFileName(type)!="")
		{
			// Don't delete, 
			bDelete=false;
			break;
		}
		ite++;
	}
	// If break, not empty, return
	if (ite!=_Tile256.end())
		return;


	// Check all the transitions tiles
	sint trans;
	for (trans=0; trans<count; trans++)
	{
		// Get the tile associed with the transition
		int nTile=_TileTransition[trans]._Tile;

		// If it is not NULL..
		if (nTile!=-1)
		{
			// If the file name is valid
			if (bank.getTile (nTile)->getFileName(type)!="")
			{
				// Don't delete, 
				bDelete=false;
				break;
			}
		}
		ite++;
	}
	if (trans!=count)
		return;

	// Ok, erase borders because no tile use it anymore
	_Border128[type].reset();
	_Border256[type].reset();
}


// ***************************************************************************
// ***************************************************************************
// CTileBorder.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
const sint CTileBorder::_Version=0;
// ***************************************************************************
void CTileBorder::serial(IStream &f) throw(EStream)
{
	sint streamver = f.serialVersion(_Version);

	f.serial (_Set);
	f.serial (_Width);
	f.serial (_Height);
	f.serialCont (_Borders[top]);
	f.serialCont (_Borders[bottom]);
	f.serialCont (_Borders[left]);
	f.serialCont (_Borders[right]);
}
// ***************************************************************************
void CTileBorder::set (int width, int height, const std::vector<CBGRA>& array)
{
	// Check array size
	nlassert (width>0);
	nlassert (height>0);
	nlassert ((sint)array.size()==width*height);

	// Copy size
	_Width=width;
	_Height=height;

	// Last line
	int lastLine=(_Height-1)*width;
	int lastCol=(_Width-1);
	_Borders[top].resize (_Width);
	_Borders[bottom].resize (_Width);
	_Borders[left].resize (_Height);
	_Borders[right].resize (_Height);

	// Copy top/bottom border
	for (int w=0; w<_Width; w++)
	{
		_Borders[top][w]=array[w];
		_Borders[bottom][w]=array[w+lastLine];
	}

	// Copy left/right border
	for (int h=0; h<_Height; h++)
	{
		_Borders[left][h]=array[h*_Width];
		_Borders[right][h]=array[h*_Width+lastCol];
	}

	// Set
	_Set=true;
}
// ***************************************************************************
bool CTileBorder::compare (const CTileBorder& border1, const CTileBorder& border2, TBorder where1, TBorder where2, int& pixel, int& composante, bool bInvertFirst, bool bInvertSecond)
{
	// Check border is initialized
	nlassert (border1.isSet());
	nlassert (border2.isSet());

	if (border1._Borders[where1].size()!=border2._Borders[where2].size())
		return false;
	for (pixel=0; pixel<(int)border1._Borders[where1].size(); pixel++)
	{
		pixel=pixel;
		if (border1._Borders[where1][pixel].R!=border2._Borders[where2][pixel].R)
		{
			composante=0;
			return false;
		}
		else if (border1._Borders[where1][pixel].G!=border2._Borders[where2][pixel].G)
		{
			composante=1;
			return false;
		}
		else if (border1._Borders[where1][pixel].B!=border2._Borders[where2][pixel].B)
		{
			composante=2;
			return false;
		}
		else
		{
			int alpha1=border1._Borders[where1][pixel].A;
			int alpha2=border2._Borders[where2][pixel].A;
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
	return (_Width==border._Width) && (_Height==border._Height) && (_Borders==border._Borders);
}
// ***************************************************************************
void CTileBorder::operator= (const CTileBorder& border)
{
	_Set=border._Set;
	_Width=border._Width;
	_Height=border._Width;
	_Borders[top]=border._Borders[top];
	_Borders[bottom]=border._Borders[bottom];
	_Borders[left]=border._Borders[left];
	_Borders[right]=border._Borders[right];
}
	
// ***************************************************************************
void CTileBorder::doubleSize ()
{
	_Borders[top].resize (_Width*2);
	_Borders[bottom].resize (_Width*2);
	_Borders[left].resize (_Height*2);
	_Borders[right].resize (_Height*2);

	for (int w=0; w<_Width; w++)
	{
		_Borders[top][w+_Width]=_Borders[top][w];
		_Borders[bottom][w+_Width]=_Borders[bottom][w];
	}
	for (int h=0; h<_Height; h++)
	{
		_Borders[left][h+_Height]=_Borders[left][h];
		_Borders[right][h+_Height]=_Borders[right][h];
	}
}
// ***************************************************************************
void CTileBorder::invertAlpha()
{
	for (int b=0; b<borderCount; b++)
	{
		for (int s=0; s<(int)_Borders[b].size(); s++)
			_Borders[b][s].A=255-_Borders[b][s].A;
	}
}

// ***************************************************************************
// ***************************************************************************
// CTileSetTransition.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
const sint CTileSetTransition::_Version=1;
// ***************************************************************************
void CTileSetTransition::serial(class NLMISC::IStream &f) throw(EStream)
{
	sint streamver = f.serialVersion(_Version);

	switch (streamver)
	{
	case 0:
		{
			bool doomy;
			f.serial (_Tile);
			f.serial (doomy);		// skip the old argu
		}
		break;
	case 1:
		f.serial (_Tile);
		break;
	}
}


}	// NL3D
