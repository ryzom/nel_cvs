/** \file zone_bank.cpp
 * Zone Bank
 *
 * $Id: zone_bank.cpp,v 1.1 2001/10/24 14:38:25 besson Exp $
 */

/* Copyright, 2000, 2001 Nevrax Ltd.
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

#include "zone_bank.h"
#include "nel/misc/debug.h"

using namespace std;

namespace NLLIGO
{

// ***************************************************************************
// CZoneBankElement
// ***************************************************************************

// ---------------------------------------------------------------------------
void CZoneBankElement::addCategory (const std::string &CatType, const std::string &CatValue)
{
	SCategory cat;

	for (uint32 i = 0; i < _Categories.size(); ++i)
		if (_Categories[i].Type == CatType)
		{
			_Categories[i].Value = CatValue;
			return;
		}

	cat.Type = CatType;
	cat.Value = CatValue;
	_Categories.push_back (cat);
}

// ---------------------------------------------------------------------------
const string& CZoneBankElement::getName ()
{
	for (uint32 i = 0; i < _Categories.size(); ++i)
		if (_Categories[i].Type == "Zone")
			return _Categories[i].Value;
	nlstop;
	return "";
}

// ---------------------------------------------------------------------------
const string& CZoneBankElement::getSize ()
{
	for (uint32 i = 0; i < _Categories.size(); ++i)
		if (_Categories[i].Type == "Size")
			return _Categories[i].Value;
	nlstop;
	return "";
}


// ***************************************************************************
// CZoneBank
// ***************************************************************************


void CZoneBank::debugInit() // \ todo trap remove this
{
	CZoneBankElement zbeTmp;
	zbeTmp.addCategory ("Zone", "Zone001");
	zbeTmp.addCategory ("Size", "1x1");
	zbeTmp.addCategory ("Material", "titFleur");
	_Elements.push_back (zbeTmp);
	zbeTmp._Categories.clear ();

	zbeTmp.addCategory ("Zone", "Zone002");
	zbeTmp.addCategory ("Size", "1x1");
	zbeTmp.addCategory ("Material", "titFleur");
	_Elements.push_back (zbeTmp);
	zbeTmp._Categories.clear ();

	zbeTmp.addCategory ("Zone", "Zone003");
	zbeTmp.addCategory ("Size", "2x2");
	zbeTmp.addCategory ("Material", "titFleur");
	_Elements.push_back (zbeTmp);
	zbeTmp._Categories.clear ();

	zbeTmp.addCategory ("Zone", "Zone004");
	zbeTmp.addCategory ("Size", "2x2");
	zbeTmp.addCategory ("Material", "grozFleur");
	_Elements.push_back (zbeTmp);
	zbeTmp._Categories.clear ();

	zbeTmp.addCategory ("Zone", "Zone005");
	zbeTmp.addCategory ("Size", "1x1");
	zbeTmp.addCategory ("Material", "grozFleur");
	_Elements.push_back (zbeTmp);
	zbeTmp._Categories.clear ();

	zbeTmp.addCategory ("Zone", "Zone006");
	zbeTmp.addCategory ("Size", "1x1");
	zbeTmp.addCategory ("Material", "grozFleur");
	_Elements.push_back (zbeTmp);
	zbeTmp._Categories.clear ();
}

// ---------------------------------------------------------------------------
void CZoneBank::getCategoriesType (std::vector<std::string> &CategoriesType)
{
	for (uint32 i = 0; i < _Elements.size(); ++i)
	{
		CZoneBankElement &rZBE = _Elements[i];
		for (uint32 j = 0; j < rZBE._Categories.size(); ++j)
		{
			CZoneBankElement::SCategory &rCat = rZBE._Categories[j];
			bool bFound = false;
			for (uint32 k = 0; k < CategoriesType.size(); ++k)
				if (rCat.Type == CategoriesType[k])
				{
					bFound = true;
					break;
				}
			if (!bFound)
				CategoriesType.push_back (rCat.Type);
		}
	}
}

// ---------------------------------------------------------------------------
void CZoneBank::getCategoryValues (const std::string &CategoryType, std::vector<std::string> &CategoryValues)
{
	for (uint32 i = 0; i < _Elements.size(); ++i)
	{
		CZoneBankElement &rZBE = _Elements[i];
		for (uint32 j = 0; j < rZBE._Categories.size(); ++j)
		{
			CZoneBankElement::SCategory &rCat = rZBE._Categories[j];

			if (rCat.Type == CategoryType)
			{
				bool bFound = false;
				for (uint32 k = 0; k < CategoryValues.size(); ++k )
					if (rCat.Value == CategoryValues[k])
					{
						bFound = true;
						break;
					}
				if (!bFound)
					CategoryValues.push_back (rCat.Value);
			}
		}
	}
}

// ---------------------------------------------------------------------------
void CZoneBank::resetSelection ()
{
	_Selection.clear ();
}

// ---------------------------------------------------------------------------
void CZoneBank::addOrSwitch (const std::string &CategoryType, const std::string &CategoryValue)
{
	for (uint32 i = 0; i < _Elements.size(); ++i)
	{
		CZoneBankElement &rZBE = _Elements[i];
		for (uint32 j = 0; j < rZBE._Categories.size(); ++j)
		{
			CZoneBankElement::SCategory &rCat = rZBE._Categories[j];

			if ((rCat.Type == CategoryType) && (rCat.Value == CategoryValue))
			{
				// Check if the element is not already present in the selection
				bool bFound = false;
				for (uint32 k = 0; k < _Selection.size(); ++k )
					if (&rZBE == _Selection[k])
					{
						bFound = true;
						break;
					}
				if (!bFound)
					_Selection.push_back (&rZBE);
			}
		}
	}
}

// ---------------------------------------------------------------------------
void CZoneBank::addAndSwitch (const std::string &CategoryType, const std::string &CategoryValue)
{
	uint32 i, j;
	// And the selection with some constraints
	// All elements of the selection must have a catType and catValue equal to those given in parameters
	for (i = 0; i < _Selection.size(); ++i)
	{
		CZoneBankElement *pZBE = _Selection[i];
		bool bFound = false;
		for (j = 0; j < pZBE->_Categories.size(); ++j)
		{
			CZoneBankElement::SCategory &rCat = pZBE->_Categories[j];
			if ((rCat.Type == CategoryType) && (rCat.Value == CategoryValue))
				bFound = true;
		}
		if (!bFound)
		{
			_Selection[i] = NULL; // Mark this item to be removed
		}
	}
	// Remove all unused items	
	for (i = 0, j = 0; i < _Selection.size(); ++i)
	{
		if (_Selection[i] != NULL)
		{
			_Selection[j] = _Selection[i];
			++j;
		}
	}
	_Selection.resize (j);
}

// ---------------------------------------------------------------------------
void CZoneBank::getSelection (std::vector<CZoneBankElement*> &SelectedElements)
{
	SelectedElements = _Selection;
}

// ***************************************************************************

} // namespace NLLIGO