/** \file ligo_config.cpp
 * Ligo config file 
 *
 * $Id: ligo_config.cpp,v 1.9 2003/11/17 14:26:38 distrib Exp $
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

#include <nel/ligo/ligo_config.h>

#include <nel/ligo/primitive.h>
#include <nel/misc/config_file.h>
#include <nel/misc/i_xml.h>
#include <nel/misc/path.h>
#include <nel/misc/file.h>

using namespace std;
using namespace NLMISC;

namespace NLLIGO
{

// ***************************************************************************

bool CLigoConfig::read (const char *fileName)
{
	// The CF
	CConfigFile cf;

	// Load and parse the file
	cf.load (fileName);

	// Read the parameters
	CConfigFile::CVar &cell_size = cf.getVar ("cell_size");
	CellSize = cell_size.asFloat ();
	CConfigFile::CVar &snap = cf.getVar ("snap");
	Snap = snap.asFloat ();
	CConfigFile::CVar &snapShot = cf.getVar ("zone_snapeshot_res");
	ZoneSnapShotRes = (uint)snapShot.asInt ();
	CConfigFile::CVar &primitiveClassFilename = cf.getVar ("primitive_class_filename");
	PrimitiveClassFilename= primitiveClassFilename.asString ();
	
	// Clear the previous classes
	_Contexts.clear();
	_PrimitiveClasses.clear();
	_PrimitiveConfigurations.clear();

	// Read the primitive class name
	if (!PrimitiveClassFilename.empty())
	{
		return readPrimitiveClass (PrimitiveClassFilename.c_str());
	}
	return true;
}

// ***************************************************************************

bool CLigoConfig::readPrimitiveClass (const char *_fileName)
{
	// File exist ?
	string filename = _fileName;
	filename = CPath::lookup (_fileName, false, false, false);
	if (filename.empty())
		filename = _fileName;

	// The context strings
	set<string> contextStrings;

	// Read the document
	CIFile file;
	if (file.open (filename))
	{
		try
		{
			// XML stream
			CIXml xml;
			xml.init (file);

			// Get the root node
			xmlNodePtr root = xml.getRootNode ();
			nlassert (root);
			
			// Check the header
			if (strcmp ((const char*)root->name, "NEL_LIGO_PRIMITIVE_CLASS") == 0)
			{
				// Get the first primitive description
				xmlNodePtr primitive = CIXml::getFirstChildNode (root, "PRIMITIVE");
				if (primitive)
				{
					do
					{
						// Get the primitive name
						std::string name;
						if (getPropertyString (name, filename.c_str(), primitive, "CLASS_NAME"))
						{
							// Add the primitive
							pair<std::map<std::string, CPrimitiveClass>::iterator, bool> insertResult = 
								_PrimitiveClasses.insert (std::map<std::string, CPrimitiveClass>::value_type (name, CPrimitiveClass ()));
							if (insertResult.second)
							{
								if (!insertResult.first->second.read (primitive, filename.c_str(), name.c_str (), contextStrings, _ContextFilesLookup, *this))
									return false;
							}
							else
							{
								syntaxError (filename.c_str(), root, "Class (%s) aready defined", name.c_str ());
							}
						}
						else
							return false;
					}
					while ((primitive = CIXml::getNextChildNode (primitive, "PRIMITIVE")));
				}
					
				// Add the context strings
				{
					set<string>::iterator ite = contextStrings.begin ();
					while (ite != contextStrings.end ())
					{
						if (*ite != "default")
							_Contexts.push_back (*ite);
						ite++;
					}
					_Contexts.push_back ("default");
				}

				// Get the first primitive configuration
				_PrimitiveConfigurations.reserve (CIXml::countChildren (root, "CONFIGURATION"));
				xmlNodePtr configuration = CIXml::getFirstChildNode (root, "CONFIGURATION");
				if (configuration)
				{
					do
					{
						// Get the configuration name
						std::string name;
						if (getPropertyString (name, filename.c_str(), configuration, "NAME"))
						{
							// Add the configuration
							_PrimitiveConfigurations.resize (_PrimitiveConfigurations.size()+1);
							if (!_PrimitiveConfigurations.back().read (configuration, filename.c_str(), name.c_str (), *this))
								return false;
						}
						else
							return false;
					}
					while ((configuration = CIXml::getNextChildNode (configuration, "CONFIGURATION")));
				}

				// Ok
				return true;
			}
			else
			{
				syntaxError (filename.c_str(), root, "Wrong root node, should be NEL_LIGO_PRIMITIVE_CLASS");
			}
		}
		catch (Exception &e)
		{
			errorMessage ("File read error (%s):%s", filename.c_str(), e.what ());
		}
	}
	else
	{
		errorMessage ("Can't open the file %s for reading.", filename.c_str());
	}
	return false;
}

// ***************************************************************************

NLMISC::CRGBA CLigoConfig::getPrimitiveColor (const NLLIGO::IPrimitive &primitive)
{
	// Get the class
	string className;
	if (primitive.getPropertyByName ("class", className))
	{
		// Get the class
		std::map<std::string, CPrimitiveClass>::iterator ite = _PrimitiveClasses.find (className);
		if (ite != _PrimitiveClasses.end ())
		{
			return ite->second.Color;
		}
	}
	return DEFAULT_PRIMITIVE_COLOR;
}

// ***************************************************************************

bool CLigoConfig::isPrimitiveLinked (const NLLIGO::IPrimitive &primitive)
{
	// Get the class
	string className;
	if (primitive.getPropertyByName ("class", className))
	{
		// Get the class
		std::map<std::string, CPrimitiveClass>::iterator ite = _PrimitiveClasses.find (className);
		if (ite != _PrimitiveClasses.end ())
		{
			return ite->second.LinkBrothers;
		}
	}
	return false;
}

// ***************************************************************************

bool CLigoConfig::isPrimitiveDeletable (const NLLIGO::IPrimitive &primitive)
{
	// Get the class
	string className;
	if (primitive.getPropertyByName ("class", className))
	{
		// Get the class
		std::map<std::string, CPrimitiveClass>::iterator ite = _PrimitiveClasses.find (className);
		if (ite != _PrimitiveClasses.end ())
		{
			return ite->second.Deletable;
		}
	}
	return false;
}

// ***************************************************************************

bool CLigoConfig::canBeChild (const NLLIGO::IPrimitive &child, const NLLIGO::IPrimitive &parent)
{
	// Get the child class
	string childClassName;
	if (child.getPropertyByName ("class", childClassName))
	{
		// Get the parent class
		const CPrimitiveClass *parentClass = getPrimitiveClass (parent);
		if (parentClass)
		{
			// Search for the child class
			uint i;
			for (i=0; i<parentClass->DynamicChildren.size (); i++)
			{
				// The same ?
				if (parentClass->DynamicChildren[i].ClassName == childClassName)
					break;
			}

			return (i<parentClass->DynamicChildren.size ());
		}
		else
			return true;
	}
	else
	{
		// Only if it is a root node or parent class doesn't exist
		string parentClassName;
		return ( (parent.getParent () == NULL) || (!parent.getPropertyByName ("class", parentClassName) ) );
	}	
}

// ***************************************************************************

bool CLigoConfig::canBeRoot (const NLLIGO::IPrimitive &child)
{
	// Get the child class
	string childClassName;
	if (child.getPropertyByName ("class", childClassName))
	{
		// Get the parent class
		const CPrimitiveClass *parentClass = getPrimitiveClass ("root");
		if (parentClass)
		{
			// Search for the child class
			uint i;
			for (i=0; i<parentClass->DynamicChildren.size (); i++)
			{
				// The same ?
				if (parentClass->DynamicChildren[i].ClassName == childClassName)
					break;
			}

			return (i<parentClass->DynamicChildren.size ());
		}
		else
			return true;
	}
	else
	{
		// Root class doesn't exist
		return ( !getPrimitiveClass ("root") );
	}	
}

// ***************************************************************************

bool CLigoConfig::getPropertyString (std::string &result, const char *filename, xmlNodePtr xmlNode, const char *propName)
{
	// Call the CIXml version
	if (!CIXml::getPropertyString (result, xmlNode, propName))
	{
		// Output a formated error
		syntaxError (filename, xmlNode, "Missing XML node property (%s)", propName);
		return false;
	}
	return true;
}

// ***************************************************************************

void CLigoConfig::syntaxError (const char *filename, xmlNodePtr xmlNode, const char *format, ...)
{
	va_list args;
	va_start( args, format );
	char buffer[1024];
	vsnprintf( buffer, 1024, format, args );
	va_end( args );

	errorMessage ("(%s), node (%s), line (%d) :\n%s", filename, xmlNode->name, (int)xmlNode->content, buffer);
}

// ***************************************************************************

void CLigoConfig::errorMessage (const char *format, ... )
{
	// Make a buffer string
	va_list args;
	va_start( args, format );
	char buffer[1024];
	vsnprintf( buffer, 1024, format, args );
	va_end( args );

	nlwarning (buffer);
}

// ***************************************************************************

const std::vector<std::string> &CLigoConfig::getContextString () const
{
	return _Contexts;
}

// ***************************************************************************

const CPrimitiveClass *CLigoConfig::getPrimitiveClass (const IPrimitive &primitive) const
{
	const CPrimitiveClass *primClass = NULL;

	// Get property class
	string className;
	if (primitive.getPropertyByName ("class", className))
	{
		std::map<std::string, CPrimitiveClass>::const_iterator ite = _PrimitiveClasses.find (className);
		if (ite != _PrimitiveClasses.end ())
		{
			primClass = &(ite->second);
		}
	}

	// Not found ?
	if (!primClass)
	{
		// Root ?
		if (!primitive.getParent ())
		{
			std::map<std::string, CPrimitiveClass>::const_iterator ite = _PrimitiveClasses.find ("root");
			if (ite != _PrimitiveClasses.end ())
			{
				primClass = &(ite->second);
			}
		}
	}
	return primClass;
}

// ***************************************************************************

const CPrimitiveClass *CLigoConfig::getPrimitiveClass (const char *className) const
{
	std::map<std::string, CPrimitiveClass>::const_iterator ite = _PrimitiveClasses.find (className);
	if (ite != _PrimitiveClasses.end ())
	{
		return &(ite->second);
	}
	return NULL;
}

// ***************************************************************************

}
