/** \file zone_bank.h
 * Zone Bank
 *
 * $Id: zone_bank.h,v 1.1 2001/10/24 14:38:25 besson Exp $
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
#include <string>
#include <vector>

namespace NLLIGO
{


// ***************************************************************************


// ***************************************************************************

class CZoneBankElement
{
	struct SCategory
	{
		std::string		Type;		// Ex: "Material", "Size", ...
		std::string		Value;		// Ex: "Grass", "2x2", ...
	};

	// Category stuff
	std::vector<SCategory>	_Categories;
	// In this list the category type and value must be unique and 2 categories must 
	// appears : "Zone" (The zone name) and "Size" (*x* (ex:4x4 3x1 etc...))

public:

	void addCategory (const std::string &CatType, const std::string &CatValue);
	const std::string& getName ();
	const std::string& getSize ();

	friend class CZoneBank;
};

// ***************************************************************************

class CZoneBank
{

	std::vector<CZoneBankElement>	_Elements;

	std::vector<CZoneBankElement*>	_Selection;

public:

	void debugInit(); // \todo trap see with hulud how we initialize this structure

	void getCategoriesType (std::vector<std::string> &CategoriesType);
	void getCategoryValues (const std::string &CategoryType, std::vector<std::string> &CategoryValues);

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
