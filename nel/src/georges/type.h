/** \file type.h
 * Georges type class
 *
 * $Id: type.h,v 1.1 2002/05/17 06:28:05 corvazier Exp $
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

#ifndef _NLGEORGES__TYPE_H
#define _NLGEORGES__TYPE_H

#include	"nel/misc/types_nl.h"
#include	"nel/misc/smart_ptr.h"
#include	"header.h"

namespace NLGEORGES
{

class UFormElm;

/**
  * This class is a basic type used by atomic form element.
  */
class CType : public NLMISC::CRefCount
{
	friend class CFormLoader;
public:

	CType ();
	~CType ();
	
	// ** Type
	enum TType
	{
		UnsignedInt=0,
		SignedInt,
		Double,
		String,
		Color,
		TypeCount
	};

	// ** UI
	enum TUI
	{
		Edit,				// Default, for all types
		EditSpin,			// For number types
		NonEditableCombo,	// For all types
		FileBrowser,		// Browse file
		BigEdit,			// Edit a huge text
		ColorEdit,			// Edit a color
		UITypeCount
	};

	// Get the default value
	const std::string& getDefault () const
	{
		return Default;
	}

	// ** IO functions
	void				write (xmlDocPtr doc) const;

	// Header
	CFileHeader			Header;
	
	// Type of the type
	TType				Type;

	// Type fo user interface
	TUI					UIType;

	// Default value
	std::string			Default;

	// Min value
	std::string			Min;

	// Max value
	std::string			Max;

	// Evaluate a node
	bool				getValue (std::string &result, const class CForm *form, const class CFormElmAtom *node, const class CFormDfn &parentDfn, 
									uint parentIndex, bool evaluate, uint32 *where) const;

	// Definitions
	class CDefinition
	{
	public:
		// Label of the definition
		std::string		Label;

		// Value of the definition
		std::string		Value;
	};

	// Array of definition
	std::vector<CDefinition>	Definitions;

	// Get the type names
	static const char *getTypeName (TType type);
	static const char *getUIName (TUI type);

private:
	// Type names
	static const char	*TypeNames[];
	static const char	*UITypeNames[];

	// CFormLoader call it
	void				read (xmlNodePtr root);

};

} // NLGEORGES

#endif // _NLGEORGES__TYPE_H

