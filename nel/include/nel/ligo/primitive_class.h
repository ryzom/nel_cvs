/** \file primitive_class.h
 * Ligo primitive class description. Give access at common properties for a primitive class. Properties are given in an XML file
 *
 * $Id: primitive_class.h,v 1.1 2003/08/01 13:11:23 corvazier Exp $
 */

/* Copyright, 2000-2002 Nevrax Ltd.
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

#ifndef NL_PRIMITIVE_CLASS_H
#define NL_PRIMITIVE_CLASS_H

#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"
#include <map>
#include <set>

// Include from libxml2
#include <libxml/parser.h>

namespace NLLIGO
{

class IProperty;
class CLigoConfig;

/**
 * Class of primitive
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2002
 */
class CPrimitiveClass
{
public:

	// Type of the primitive
	enum TType
	{
		Node,
		Point,
		Path,
		Bitmap,
		Zone
	}						Type;

	/// Constructor
	CPrimitiveClass ();

	/// Class name
	std::string				Name;

	/// Filename extension (for type File)
	std::string				FileExtension;

	/// File type (for type File)
	std::string				FileType;

	/// Color
	NLMISC::CRGBA			Color;

	/// Auto init ?
	bool					AutoInit;

	/// Deletable ?
	bool					Deletable;

	/// Collision ?
	bool					Collision;

	/// Link children ?
	bool					LinkBrothers;

	/// Show arrow ?
	bool					ShowArrow;

	/// Autonaming
	std::string				Autoname;

	/// Init parameters
	class CInitParameters
	{
		// A default value
		class CDefaultValue
		{
		public:
			std::string		Name;
			bool			GenID;
			bool			operator== (const CDefaultValue &other) const
			{
				return (Name == other.Name) && (GenID == other.GenID);
			}
			bool			operator< (const CDefaultValue &other) const
			{
				if (Name < other.Name) 
					return true;
				else if (Name == other.Name)
				{
					return (GenID < other.GenID);
				}
				else
					return false;
			}
		};

	public:
		/// Parameter name
		std::string					Name;

		/// Default value
		std::vector<CDefaultValue>	DefaultValue;
	};

	// Parameter description
	class CParameter : public CInitParameters
	{
	public:
		CParameter () {}
		CParameter (const NLLIGO::IProperty &property, const char *propertyName);
		bool operator== (const CParameter &other) const;
		bool operator< (const CParameter &other) const;

		// Type
		enum TType
		{
			Boolean,
			ConstString,
			String,
			StringArray,
		}			Type;

		/// Is parameter visible ?
		bool		Visible;

		// Is a filename
		bool		Filename;

		// Make a look up ?
		bool		Lookup;

		// File extension
		std::string	FileExtension;
		
		// Autonaming
		std::string	Autoname;

		// Folder
		std::string	Folder;

		// Combobox value
		class CConstStringValue
		{
		public:
			bool operator== (const CConstStringValue &other) const;
			bool operator< (const CConstStringValue &other) const;
			std::vector<std::string>		Values;
		};

		// Map of combobox value per context
		std::map<std::string, CConstStringValue>	ComboValues;
	};

	/// Parameters
	std::vector<CParameter>	Parameters;

	// Child
	class CChild
	{
	public:
		/// Static child name
		std::string	Name;

		/// Child class name
		std::string	ClassName;

		/// Init parameters
		std::vector<CInitParameters>	Parameters;
	};

	// Static Children
	std::vector<CChild>			StaticChildren;

	// Dynamic Children
	std::vector<CChild>			DynamicChildren;

	// Generated Children
	std::vector<CChild>			GeneratedChildren;

	// Read
	bool	read (xmlNodePtr primitiveNode, const char *filename, const char *className, std::set<std::string> &contextStrings,
		std::map<std::string, std::string> &contextFilesLookup, NLLIGO::CLigoConfig &config);
};

} // NLLIGO

#endif // NL_PRIMITIVE_CLASS_H

/* End of primitive_class.h */