/** \file zone_bank.h
 * Zone Bank
 *
 * $Id: zone_bank.h,v 1.3 2001/11/07 13:28:38 besson Exp $
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

#ifndef NL_ZONE_BANK_H
#define NL_ZONE_BANK_H

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include <string>
#include <vector>

namespace NLLIGO
{


// ***************************************************************************

/// No category of the type given found
#define STRING_NO_CAT_TYPE	"< NOCATTYPE >"

// ***************************************************************************

class CZoneBankElement
{

	// Category stuff
	// The key is the Type of the category	(Ex: "Material", "Size", ...)
	// The second element is the value		(Ex: "Grass", "2x2", ...)
	std::map<std::string, std::string>	_CategoriesMap;

	// In this list the category type and value must be unique and 2 categories MUST
	// appears : "Zone" (The zone name) and "Size" (*x* (ex:4x4 3x1 etc...))
	// Some categories used in WorldEditor : "Material", "Transition"
	uint32					_SizeX, _SizeY;
	std::vector<bool>		_Mask;

	static std::string		_NoCatTypeFound; // = STRING_NO_CAT_TYPE

public:

	CZoneBankElement ();

	void addCategory (const std::string &CatType, const std::string &CatValue);
	const std::string &getName ();
	const std::string &getSize ();
	uint32 getSizeX () { return _SizeX; }
	uint32 getSizeY () { return _SizeY; }
	const std::vector<bool> &getMask () { return _Mask; }

	/// Return the CatValue or STRING_NO_CAT_TYPE if no category of that type found
	const std::string &getCategory(const std::string &CatType);

	/// Convert size in the categories to _SizeX, _SizeY
	void convertSize ();
	void serial (NLMISC::IStream &f);

	friend class CZoneBank;
};

// ***************************************************************************

class CZoneBank
{

	std::map<std::string,CZoneBankElement>	_ElementsMap;

	std::vector<CZoneBankElement*>	_Selection;

public:

	// Debug stuff beg
	// ---------------
	void debugInit(); // \todo trap see with hulud how we initialize this structure
	void debugSaveInit(CZoneBankElement &zbeTmp, const std::string &fileName);
	// ---------------
	// Debug stuff end

	/// Initialize the zone bank with all files present in the path given (note pathName must not end with '\\')
	void initFromPath (const std::string &pathName);
	/// Load an element in the current directory
	void addElement (const std::string &elementName);

	void getCategoriesType (std::vector<std::string> &CategoriesType);
	void getCategoryValues (const std::string &CategoryType, std::vector<std::string> &CategoryValues);
	CZoneBankElement *getElementByZoneName (const std::string &ZoneName);

	// Selection
	void resetSelection ();
	void addOrSwitch (const std::string &CategoryType, const std::string &CategoryValue);
	void addAndSwitch (const std::string &CategoryType, const std::string &CategoryValue);
	void getSelection (std::vector<CZoneBankElement*> &SelectedElements);

};

// ***************************************************************************

} // namespace NLLIGO

// ***************************************************************************

#endif // NL_ZONE_BANK_H

/* End of zone_bank.h */
