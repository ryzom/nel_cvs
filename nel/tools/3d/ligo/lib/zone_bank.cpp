/** \file zone_bank.cpp
 * Zone Bank
 *
 * $Id: zone_bank.cpp,v 1.6 2001/11/16 11:04:00 besson Exp $
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
#include "nel/misc/file.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/o_xml.h"

using namespace std;
using namespace NLMISC;

namespace NLLIGO
{

// ***************************************************************************
// CZoneBankElement
// ***************************************************************************

string CZoneBankElement::_NoCatTypeFound = STRING_NO_CAT_TYPE;

// ---------------------------------------------------------------------------
CZoneBankElement::CZoneBankElement()
{
	_SizeX = _SizeY = 0;
}

// ---------------------------------------------------------------------------
void CZoneBankElement::addCategory (const std::string &CatType, const std::string &CatValue)
{
	_CategoriesMap.insert(pair<string,string>(CatType, CatValue));
}

// ---------------------------------------------------------------------------
const string& CZoneBankElement::getName ()
{
	return getCategory ("Zone");
}

// ---------------------------------------------------------------------------
const string& CZoneBankElement::getSize ()
{
	return getCategory ("Size");
}

// ---------------------------------------------------------------------------
const string& CZoneBankElement::getCategory (const string &CatType)
{
	map<string,string>::iterator it = _CategoriesMap.find (CatType);
	if (it == _CategoriesMap.end())
		return _NoCatTypeFound;
	else
		return it->second;
}

// ---------------------------------------------------------------------------
void CZoneBankElement::convertSize()
{
	const string &sizeString =  getSize();
	string sTmp;
	uint32 i;

	for (i = 0; i < sizeString.size(); ++i)
	{
		if (sizeString[i] == 'x')
			break;
		else
			sTmp += sizeString[i];
	}
	_SizeX = atoi (sTmp.c_str());

	++i; sTmp = "";
	for (; i < sizeString.size(); ++i)
	{
		sTmp += sizeString[i];
	}
	_SizeY = atoi (sTmp.c_str());
}

// ---------------------------------------------------------------------------
void CZoneBankElement::serial (IStream &f)
{
	f.xmlPush ("LIGOZONE");
	
	sint version = 1;
	f.serialVersion (version);
	string check = "LIGOZONE";
	f.serialCheck (check);

	f.xmlPush ("CATEGORIES");
		f.serialCont (_CategoriesMap);
	f.xmlPop ();
	
	f.xmlPush ("MASK");
		f.serialCont (_Mask);
	f.xmlPop ();

	f.xmlPop ();

	convertSize();
}


// ---------------------------------------------------------------------------
void CZoneBankElement::setMask (const std::vector<bool> &mask, uint8 sizeX, uint8 sizeY)
{
	_SizeX = sizeX;
	_SizeY = sizeY;
	_Mask = mask;
}

// ***************************************************************************
// CZoneBank
// ***************************************************************************

// ---------------------------------------------------------------------------
void CZoneBank::debugSaveInit (CZoneBankElement &zbeTmp, const string &fileName)
{
	try
	{
		COFile fileOut;
		fileOut.open (fileName);
		COXml output;
		output.init (&fileOut);
		zbeTmp.serial (output);
	}
	catch (Exception& /*e*/)
	{
	}

}

// ---------------------------------------------------------------------------
void CZoneBank::debugInit() // \ todo trap remove this
{
	CZoneBankElement zbeTmp;
	zbeTmp.addCategory ("Zone", "Zone001");
	zbeTmp.addCategory ("Size", "1x1");
	zbeTmp.addCategory ("Material", "titFleur");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, "ZoneLigos\\Zone001.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	

	zbeTmp.addCategory ("Zone", "Zone002");
	zbeTmp.addCategory ("Size", "1x1");
	zbeTmp.addCategory ("Material", "titFleur");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, "ZoneLigos\\Zone002.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();

	zbeTmp.addCategory ("Zone", "Zone003");
	zbeTmp.addCategory ("Size", "2x2");
	zbeTmp.addCategory ("Material", "titFleur");
	zbeTmp._Mask.push_back (true);
	zbeTmp._Mask.push_back (true);
	zbeTmp._Mask.push_back (false);
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, "ZoneLigos\\Zone003.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();

	zbeTmp.addCategory ("Zone", "Zone004");
	zbeTmp.addCategory ("Size", "2x2");
	zbeTmp.addCategory ("Material", "grozFleur");
	zbeTmp._Mask.push_back (false);
	zbeTmp._Mask.push_back (true);
	zbeTmp._Mask.push_back (true);
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, "ZoneLigos\\Zone004.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();

	zbeTmp.addCategory ("Zone", "Zone005");
	zbeTmp.addCategory ("Size", "1x1");
	zbeTmp.addCategory ("Material", "grozFleur");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, "ZoneLigos\\Zone005.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();

	zbeTmp.addCategory ("Zone", "Zone006");
	zbeTmp.addCategory ("Size", "4x2");
	zbeTmp.addCategory ("Material", "grozFleur");
	zbeTmp._Mask.push_back (true);
	zbeTmp._Mask.push_back (true);
	zbeTmp._Mask.push_back (false);
	zbeTmp._Mask.push_back (false);
	zbeTmp._Mask.push_back (false);
	zbeTmp._Mask.push_back (true);
	zbeTmp._Mask.push_back (true);
	zbeTmp._Mask.push_back (false);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, "ZoneLigos\\Zone006.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("Zone", "Zone007");
	zbeTmp.addCategory ("Size", "1x1");
	zbeTmp.addCategory ("Material", "grozFleur");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, "ZoneLigos\\Zone007.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("Zone", "Zone008");
	zbeTmp.addCategory ("Size", "2x2");
	zbeTmp.addCategory ("Material", "prairie");
	zbeTmp._Mask.push_back (true);
	zbeTmp._Mask.push_back (true);
	zbeTmp._Mask.push_back (true);
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, "ZoneLigos\\Zone008.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("Zone", "Zone009");
	zbeTmp.addCategory ("Size", "2x2");
	zbeTmp.addCategory ("Material", "prairie");
	zbeTmp._Mask.push_back (true);
	zbeTmp._Mask.push_back (true);
	zbeTmp._Mask.push_back (true);
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, "ZoneLigos\\Zone009.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("Zone", "Zone010");
	zbeTmp.addCategory ("Size", "1x1");
	zbeTmp.addCategory ("Material", "prairie");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, "ZoneLigos\\Zone010.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("Zone", "WT0");
	zbeTmp.addCategory ("Size", "1x1");
	zbeTmp.addCategory ("TransName", "grozFleur-prairie");
	zbeTmp.addCategory ("TransType", "Flat");
	zbeTmp.addCategory ("TransNum", "0");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, "ZoneLigos\\WT0.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("Zone", "WT1");
	zbeTmp.addCategory ("Size", "1x1");
	zbeTmp.addCategory ("TransName", "grozFleur-prairie");
	zbeTmp.addCategory ("TransType", "Flat");
	zbeTmp.addCategory ("TransNum", "1");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, "ZoneLigos\\WT1.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("Zone", "WT2");
	zbeTmp.addCategory ("Size", "1x1");
	zbeTmp.addCategory ("TransName", "grozFleur-prairie");
	zbeTmp.addCategory ("TransType", "Flat");
	zbeTmp.addCategory ("TransNum", "2");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, "ZoneLigos\\WT2.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("Zone", "WT3");
	zbeTmp.addCategory ("Size", "1x1");
	zbeTmp.addCategory ("TransName", "grozFleur-prairie");
	zbeTmp.addCategory ("TransType", "CornerA");
	zbeTmp.addCategory ("TransNum", "3");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, "ZoneLigos\\WT3.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("Zone", "WT4");
	zbeTmp.addCategory ("Size", "1x1");
	zbeTmp.addCategory ("TransName", "grozFleur-prairie");
	zbeTmp.addCategory ("TransType", "CornerA");
	zbeTmp.addCategory ("TransNum", "4");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, "ZoneLigos\\WT4.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("Zone", "WT5");
	zbeTmp.addCategory ("Size", "1x1");
	zbeTmp.addCategory ("TransName", "grozFleur-prairie");
	zbeTmp.addCategory ("TransType", "CornerA");
	zbeTmp.addCategory ("TransNum", "5");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, "ZoneLigos\\WT5.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("Zone", "WT6");
	zbeTmp.addCategory ("Size", "1x1");
	zbeTmp.addCategory ("TransName", "grozFleur-prairie");
	zbeTmp.addCategory ("TransType", "CornerB");
	zbeTmp.addCategory ("TransNum", "6");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, "ZoneLigos\\WT6.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("Zone", "WT7");
	zbeTmp.addCategory ("Size", "1x1");
	zbeTmp.addCategory ("TransName", "grozFleur-prairie");
	zbeTmp.addCategory ("TransType", "CornerB");
	zbeTmp.addCategory ("TransNum", "7");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, "ZoneLigos\\WT7.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("Zone", "WT8");
	zbeTmp.addCategory ("Size", "1x1");
	zbeTmp.addCategory ("TransName", "grozFleur-prairie");
	zbeTmp.addCategory ("TransType", "CornerB");
	zbeTmp.addCategory ("TransNum", "8");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, "ZoneLigos\\WT8.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("Zone", "ZT0");
	zbeTmp.addCategory ("Size", "1x1");
	zbeTmp.addCategory ("TransName", "titFleur-grozFleur");
	zbeTmp.addCategory ("TransType", "Flat");
	zbeTmp.addCategory ("TransNum", "0");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, "ZoneLigos\\ZT0.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("Zone", "ZT1");
	zbeTmp.addCategory ("Size", "1x1");
	zbeTmp.addCategory ("TransName", "titFleur-grozFleur");
	zbeTmp.addCategory ("TransType", "Flat");
	zbeTmp.addCategory ("TransNum", "1");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, "ZoneLigos\\ZT1.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("Zone", "ZT2");
	zbeTmp.addCategory ("Size", "1x1");
	zbeTmp.addCategory ("TransName", "titFleur-grozFleur");
	zbeTmp.addCategory ("TransType", "Flat");
	zbeTmp.addCategory ("TransNum", "2");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, "ZoneLigos\\ZT2.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("Zone", "ZT3");
	zbeTmp.addCategory ("Size", "1x1");
	zbeTmp.addCategory ("TransName", "titFleur-grozFleur");
	zbeTmp.addCategory ("TransType", "CornerA");
	zbeTmp.addCategory ("TransNum", "3");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, "ZoneLigos\\ZT3.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("Zone", "ZT4");
	zbeTmp.addCategory ("Size", "1x1");
	zbeTmp.addCategory ("TransName", "titFleur-grozFleur");
	zbeTmp.addCategory ("TransType", "CornerA");
	zbeTmp.addCategory ("TransNum", "4");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, "ZoneLigos\\ZT4.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("Zone", "ZT5");
	zbeTmp.addCategory ("Size", "1x1");
	zbeTmp.addCategory ("TransName", "titFleur-grozFleur");
	zbeTmp.addCategory ("TransType", "CornerA");
	zbeTmp.addCategory ("TransNum", "5");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, "ZoneLigos\\ZT5.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("Zone", "ZT6");
	zbeTmp.addCategory ("Size", "1x1");
	zbeTmp.addCategory ("TransName", "titFleur-grozFleur");
	zbeTmp.addCategory ("TransType", "CornerB");
	zbeTmp.addCategory ("TransNum", "6");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, "ZoneLigos\\ZT6.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("Zone", "ZT7");
	zbeTmp.addCategory ("Size", "1x1");
	zbeTmp.addCategory ("TransName", "titFleur-grozFleur");
	zbeTmp.addCategory ("TransType", "CornerB");
	zbeTmp.addCategory ("TransNum", "7");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, "ZoneLigos\\ZT7.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("Zone", "ZT8");
	zbeTmp.addCategory ("Size", "1x1");
	zbeTmp.addCategory ("TransName", "titFleur-grozFleur");
	zbeTmp.addCategory ("TransType", "CornerB");
	zbeTmp.addCategory ("TransNum", "8");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, "ZoneLigos\\ZT8.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

}

// ---------------------------------------------------------------------------
void CZoneBank::addElement (const std::string &elementName)
{
	try
	{
		CZoneBankElement zbeTmp;
		CIFile fileIn;
		fileIn.open (elementName);
		CIXml input;
		input.init (fileIn);
		zbeTmp.serial (input);
		_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(),zbeTmp));
	}
	catch (Exception& /*e*/)
	{
	}
}

// ---------------------------------------------------------------------------
void CZoneBank::getCategoriesType (std::vector<std::string> &CategoriesType)
{
	map<string,CZoneBankElement>::iterator itElt = _ElementsMap.begin();

	while (itElt != _ElementsMap.end())
	{
		CZoneBankElement &rZBE = itElt->second;

		map<string,string>::iterator it = rZBE._CategoriesMap.begin();

		while (it != rZBE._CategoriesMap.end())
		{
			bool bFound = false;
			for (uint32 k = 0; k < CategoriesType.size(); ++k)
				if (it->first == CategoriesType[k])
				{
					bFound = true;
					break;
				}
			if (!bFound)
				CategoriesType.push_back (it->first);

			++it;
		}
		++itElt;
	}
}

// ---------------------------------------------------------------------------
void CZoneBank::getCategoryValues (const std::string &CategoryType, std::vector<std::string> &CategoryValues)
{
	map<string,CZoneBankElement>::iterator itElt = _ElementsMap.begin();

	while (itElt != _ElementsMap.end())
	{
		CZoneBankElement &rZBE = itElt->second;

		map<string,string>::iterator it = rZBE._CategoriesMap.find (CategoryType);

		if (it != rZBE._CategoriesMap.end())
		{
			bool bFound = false;
			for (uint32 k = 0; k < CategoryValues.size(); ++k )
				if (it->second == CategoryValues[k])
				{
					bFound = true;
					break;
				}
			if (!bFound)
				CategoryValues.push_back (it->second);
		}
		++itElt;
	}
}

// ---------------------------------------------------------------------------
CZoneBankElement *CZoneBank::getElementByZoneName (const std::string &ZoneName)
{
	map<string,CZoneBankElement>::iterator it = _ElementsMap.find (ZoneName);
	if (it != _ElementsMap.end())
	{
		return &(it->second);
	}
	return NULL;
}

// ---------------------------------------------------------------------------
void CZoneBank::resetSelection ()
{
	_Selection.clear ();
}

// ---------------------------------------------------------------------------
void CZoneBank::addOrSwitch (const std::string &CategoryType, const std::string &CategoryValue)
{
	map<string,CZoneBankElement>::iterator itElt = _ElementsMap.begin();

	while (itElt != _ElementsMap.end())
	{
		CZoneBankElement &rZBE = itElt->second;

		map<string,string>::iterator it = rZBE._CategoriesMap.find (CategoryType);

		if (it != rZBE._CategoriesMap.end())
		{
			if (it->second == CategoryValue)
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
		++itElt;
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

		map<string,string>::iterator it = pZBE->_CategoriesMap.find (CategoryType);
		if (it != pZBE->_CategoriesMap.end())
		{
			if (it->second == CategoryValue)
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