/* tile_bank.cpp
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
 * $Id: tile_bank.cpp,v 1.2 2000/10/09 14:53:44 lecroart Exp $
 *
 * <Replace this by a description of the file>
 */

#include "nel/3d/tile_bank.h"
#include "nel/misc/stream.h"

using namespace NLMISC;


namespace	NL3D
{






// ***************************************************************************
// ***************************************************************************
// TileBankLand.
// ***************************************************************************
// ***************************************************************************


// Intersect two set and return in a set
void CTileBankLand::intersect (const std::set<sint32>& setSrc1, const std::set<sint32>& setSrc2, std::set<sint32>& setDst)
{
	// Tabl tmp
	std::vector<sint32> vectTmp;
	
	// Assez de place
	vectTmp.resize (setSrc1.size()+setSrc1.size());

	// Intersection
	std::set_intersection (setSrc1.begin(), setSrc1.end(), setSrc2.begin(), setSrc2.end(), vectTmp.begin());
	
	// Erase 
	setDst.erase (setDst.begin(), setDst.end());

	// Copy
	sint32 *pTmp=&vectTmp[0];
	sint nSize=vectTmp.size();
	for (sint n=0; n<nSize; n++)
		setDst.insert (*(pTmp++));
}
/// Search all tiles matching the 4 transitions in clockzize order. Transition equal to -1 must by ignored.
void CTileBankLand::getMatchingTile (std::vector<sint32> vectTiles, sint32 transition1, sint32 transition2, sint32 transition3, sint32 transition4)
{
	// Transition pointer
	sint32 transition[4]={ transition1, transition2, transition3, transition4 };
	CTileBankTransition *trans1=NULL;
	CTileBankTransition *trans2=NULL;
	CTileBankTransition *trans3=NULL;
	CTileBankTransition *trans4=NULL;
	if (transition1!=-1)
		trans1=getTransition (transition1);
	if (transition2!=-1)
		trans2=getTransition (transition1);
	if (transition3!=-1)
		trans3=getTransition (transition1);
	if (transition4!=-1)
		trans4=getTransition (transition1);

	// Tmp set
	std::set<sint32> setTmp;

	// Intersection...
	if (trans1)
		setTmp=trans1->getTileSet ();
	if (trans2)
		intersect (setTmp, trans1->getTileSet(), setTmp);
	if (trans3)
		intersect (setTmp, trans1->getTileSet(), setTmp);
	if (trans4)
		intersect (setTmp, trans1->getTileSet(), setTmp);

	// Check the result
	std::set<sint32>::iterator ite=setTmp.begin();
	while (ite!=setTmp.end())
	{
		// Get the tile
		CTileBankTile *tile=getTile (*ite);
		nlassert (tile);

		// Check if it's a good tile
		for (int start=0; start<4; start++)
		{
			for (int in=0; in<4; in++)
			{
				int trans=tile->getTransition ((CTileBankTile::TBorder)((start+in)%4));
				if ((trans!=transition[in])&&(transition[in]!=-1))
					break;
			}
			if (in==4)
				break;
		}
		if (start==4)
		{
			std::set<sint32>::iterator iteNext=ite;
			iteNext++;
			setTmp.erase (ite);
			ite=iteNext;
		}
		else
			ite++;
	}

	// Copy the result
	ite=setTmp.begin();
	vectTiles.resize (setTmp.size());
	sint32 *tmp=&vectTiles[0];
	while (ite!=setTmp.end())
		*(tmp++)=*(ite++);
}
// ***************************************************************************
void CTileBankLand::resizeTile (sint tileCount)
{
	TileVector.resize (tileCount);
	for (sint n=0; n<tileCount; n++)
		TileVector[n].Index=n;
}
// ***************************************************************************
sint CTileBankLand::addType (const std::string& name)
{
	sint last=TypeVector.size();
	TypeVector.push_back(name);
	return last;
}
// ***************************************************************************
sint CTileBankLand::addTransition ()
{
	sint last=TransitionVector.size();
	TransitionVector.push_back();
	return last;
}
// ***************************************************************************
const sint CTileBankLand::Version=0;
// ***************************************************************************
void CTileBankLand::serial(IStream &f)
{
	sint streamver = f.serialVersion(Version);

	f.serial (Name);
	f.serialCont (TileVector);
	f.serialCont (TypeVector);
	f.serialCont (TransitionVector);
}


// ***************************************************************************
// ***************************************************************************
// CTileBank.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
sint CTileBank::addLand (const std::string& name)
{
	sint last=LandVector.size();
	LandVector.push_back();
	LandVector[last].Name=name;
	return last;
}
void CTileBank::removeLand (sint landIndex)
{
	LandVector.erase (LandVector.begin ()+landIndex);
}
// ***************************************************************************
void CTileBank::clear ()
{
	LandVector.clear ();
}
// ***************************************************************************
const sint CTileBank::Version=0;
// ***************************************************************************
void    CTileBank::serial(IStream &f)
{
	sint streamver = f.serialVersion(Version);

	f.serialCont (LandVector);
}


// ***************************************************************************
// ***************************************************************************
// CTileBankTile.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
const sint CTileBankTile::Version=0;
// ***************************************************************************
void CTileBankTile::serial(IStream &f)
{
	sint streamver = f.serialVersion(Version);

	f.serial (Mask);
	f.serial (Index);

	f.serial (BitmapName[normal]);
	f.serial (BitmapName[add]);
	f.serial (BitmapName[bump]);
	f.serial (Transitions[north]);
	f.serial (Transitions[east]);
	f.serial (Transitions[south]);
	f.serial (Transitions[west]);
}


// ***************************************************************************
// ***************************************************************************
// CTileBankTransition.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void CTileBankTransition::AddTile (sint32 tileIndex)
{
	TileSet.insert (tileIndex);
}
// ***************************************************************************
const sint CTileBankTransition::Version=0;
// ***************************************************************************
void CTileBankTransition::serial(IStream &f)
{
	sint streamver = f.serialVersion(Version);

	f.serialCont (TileSet);
}



}