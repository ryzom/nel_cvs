/** \file _form_dfn.cpp
 * Georges form definition class
 *
 * $Id: form_dfn.cpp,v 1.2 2002/05/22 12:09:44 cado Exp $
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


#include "stdgeorges.h"

#include "nel/misc/i_xml.h"

#include "form_dfn.h"
#include "form_loader.h"

using namespace NLMISC;
using namespace std;

#ifndef NL_OS_WINDOWS
#define stricmp strcasecmp
#endif


namespace NLGEORGES
{

// ***************************************************************************

void CFormDfn::write (xmlDocPtr doc) const
{
	// Create the first node
	xmlNodePtr node = xmlNewDocNode (doc, NULL, (const xmlChar*)"DFN", NULL);
	xmlDocSetRootElement (doc, node);

	// Write elements
	uint parent;
	for (parent=0; parent<Parents.size(); parent++)
	{
		// Parent node
		xmlNodePtr parentNode = xmlNewChild ( node, NULL, (const xmlChar*)"PARENT", NULL);

		// Save parent
		xmlSetProp (parentNode, (const xmlChar*)"Name", (const xmlChar*)Parents[parent].ParentFilename.c_str());
	}

	// Write elements
	uint elm;
	for (elm=0; elm<Entries.size(); elm++)
	{
		// Add a node
		xmlNodePtr elmPtr = xmlNewChild ( node, NULL, (const xmlChar*)"ELEMENT", NULL);
		xmlSetProp (elmPtr, (const xmlChar*)"Name", (const xmlChar*)Entries[elm].Name.c_str());
		
		// What kind of element
		switch (Entries[elm].TypeElement)
		{
		case CEntry::EntryType:
			xmlSetProp (elmPtr, (const xmlChar*)"Type", (const xmlChar*)"Type");
			xmlSetProp (elmPtr, (const xmlChar*)"Filename", (const xmlChar*)Entries[elm].Filename.c_str());
			break;
		case CEntry::EntryDfn:
			xmlSetProp (elmPtr, (const xmlChar*)"Type", (const xmlChar*)"Dfn");
			xmlSetProp (elmPtr, (const xmlChar*)"Filename", (const xmlChar*)Entries[elm].Filename.c_str());
			break;
		case CEntry::EntryDfnPointer:
			xmlSetProp (elmPtr, (const xmlChar*)"Type", (const xmlChar*)"DfnPointer");
			break;
		}

		// Is an array ?
		if (Entries[elm].Array)
			xmlSetProp (elmPtr, (const xmlChar*)"Array", (const xmlChar*)"true");

		// Default value for type
		if ((Entries[elm].TypeElement == CEntry::EntryType) && (!Entries[elm].Default.empty ()))
			xmlSetProp (elmPtr, (const xmlChar*)"Default", (const xmlChar*)Entries[elm].Default.c_str ());
	}

	// Header
	Header.write (node);
}

// ***************************************************************************

void CFormDfn::read (xmlNodePtr root, CFormLoader &loader)
{
	// Check node name
	if ( ((const char*)root->name == NULL) || (strcmp ((const char*)root->name, "DFN") != 0) )
	{
		// Make an error message
		char tmp[512];
		smprintf (tmp, 512, "Georges DFN XML Syntax error in block line %d, node %s should be DFN", 
			(int)root->content, root->name);
		throw EXmlParsingError (tmp);
	}

	// Count the parent
	uint parentCount = CIXml::countChildren (root, "PARENT");
	Parents.resize (parentCount);

	// For each element entry
	uint parentNumber = 0;
	xmlNodePtr parent = CIXml::getFirstChildNode (root, "PARENT");
	while (parentNumber<parentCount)
	{
		// Get the Parent
		const char *parentFilename = (const char*)xmlGetProp (parent, (xmlChar*)"Name");
		if (parentFilename)
		{
			Parents[parentNumber].ParentFilename = parentFilename;

			// Delete the value
			xmlFree ((void*)parentFilename);

			// Load the parent
			Parents[parentNumber].Parent = loader.loadFormDfn (Parents[parentNumber].ParentFilename.c_str ());
		}
		else
		{
			// Make an error message
			char tmp[512];
			smprintf (tmp, 512, "Georges DFN XML Syntax error in block %s line %d, aguments Name not found", 
				parent->name, (int)parent->content);
			throw EXmlParsingError (tmp);
		}

		// Next parent
		parent = CIXml::getNextChildNode (parent, "PARENT");
		parentNumber++;
	}

	// Count the element children
	uint childCount = CIXml::countChildren (root, "ELEMENT");

	// Resize the element table
	Entries.resize (childCount);

	// For each element entry
	uint childNumber = 0;
	xmlNodePtr child = CIXml::getFirstChildNode (root, "ELEMENT");
	while (childNumber<childCount)
	{
		// Checks
		nlassert (child);

		// Get the name
		const char *value = (const char*)xmlGetProp (child, (xmlChar*)"Name");
		if (value)
		{
			// Store the value
			Entries[childNumber].Name = value;

			// Delete the value
			xmlFree ((void*)value);

			// Reset
			Entries[childNumber].Dfn = NULL;
			Entries[childNumber].Type = NULL;
			Entries[childNumber].Default.clear ();

			const char *filename = (const char*)xmlGetProp (child, (xmlChar*)"Filename");

			if ( filename )
			{
				Entries[childNumber].Filename = filename;

				// Delete the value
				xmlFree ((void*)filename);
			}
			else
			{
				Entries[childNumber].Filename.clear ();
			}
			
			// Read the type
			const char *typeName = (const char*)xmlGetProp (child, (xmlChar*)"Type");
			if (typeName)
			{
				bool type = false;
				bool dfn = false;
				if (stricmp (typeName, "Type") == 0)
				{
					Entries[childNumber].TypeElement = CEntry::EntryType;
					type = true;

					// Load the filename
					if (!Entries[childNumber].Filename.empty ())
					{
						Entries[childNumber].Type = loader.loadType (Entries[childNumber].Filename.c_str ());
						if (Entries[childNumber].Type == NULL)
						{
							// Make an error message
							char tmp[512];
							smprintf (tmp, 512, "Georges TYPE XML In block %s line %d, file not found %s.", 
								child->name, (int)child->content, Entries[childNumber].Filename.c_str ());

							throw EXmlParsingError (tmp);
						}

						// Read the default value
						const char *defaultName = (const char*)xmlGetProp (child, (xmlChar*)"Default");
						if (defaultName)
						{
							Entries[childNumber].Default = defaultName;

							// Delete the value
							xmlFree ((void*)defaultName);
						}
					}
					else
					{
						// Make an error message
						char tmp[512];
						smprintf (tmp, 512, "Georges TYPE XML In block %s line %d, no filename found for the .typ file.", 
							child->name, (int)child->content, Entries[childNumber].Filename.c_str ());

						throw EXmlParsingError (tmp);
					}
				}
				else if (stricmp (typeName, "Dfn") == 0)
				{
					Entries[childNumber].TypeElement = CEntry::EntryDfn;
					dfn = true;

					// Load the filename
					if (!Entries[childNumber].Filename.empty ())
					{
						// Load the filename
						Entries[childNumber].Dfn = loader.loadFormDfn (Entries[childNumber].Filename.c_str ());
						if (Entries[childNumber].Dfn == NULL)
						{
							// Make an error message
							char tmp[512];
							smprintf (tmp, 512, "Georges DFN XML In block %s line %d, file not found %s.", 
								child->name, (int)child->content, Entries[childNumber].Filename.c_str ());

							throw EXmlParsingError (tmp);
						}
					}
					else
					{
						// Make an error message
						char tmp[512];
						smprintf (tmp, 512, "Georges TYPE XML In block %s line %d, no filename found for the .typ file.", 
							child->name, (int)child->content, Entries[childNumber].Filename.c_str ());

						throw EXmlParsingError (tmp);
					}
				}
				else if (stricmp (typeName, "DfnPointer") == 0)
				{
					Entries[childNumber].TypeElement = CEntry::EntryDfnPointer;
				}
				else
				{
					// Make an error message
					char tmp[512];
					smprintf (tmp, 512, "Georges DFN XML Syntax error in block %s line %d, element has not a valid type name attribut \"Type = %s\".", 
						child->name, (int)child->content, typeName);
					throw EXmlParsingError (tmp);
				}

				// Delete the value
				xmlFree ((void*)typeName);
			}
			else
			{
				// Make an error message
				char tmp[512];
				smprintf (tmp, 512, "Georges DFN XML Syntax error in block %s line %d, element has no type name attribut \"Type = [Type][Dfn][DfnPointer]\".", 
					child->name, (int)child->content);
				throw EXmlParsingError (tmp);
			}

			// Get the array attrib
			Entries[childNumber].Array = false;
			const char* arrayFlag = (const char*)xmlGetProp (child, (xmlChar*)"Array");
			if (arrayFlag)
			{
				Entries[childNumber].Array =  (stricmp (arrayFlag, "true") == 0);
					
				// Delete the value
				xmlFree ((void*)arrayFlag);
			}
		}
		else
		{
			// Make an error message
			char tmp[512];
			smprintf (tmp, 512, "Georges DFN XML Syntax error in block %s line %d, aguments Name not found", 
				root->name, (int)root->content);
			throw EXmlParsingError (tmp);
		}

		// Next child
		child = CIXml::getNextChildNode (child, "ELEMENT");
		childNumber++;
	}

	// Read the header
	Header.read (root);
}

// ***************************************************************************

uint CFormDfn::countParentDfn () const
{
	uint count = 0;
	uint i;
	for (i=0; i<Parents.size (); i++)
	{
		count += Parents[i].Parent->countParentDfn ();
	}
	return count+1;
}

// ***************************************************************************

void CFormDfn::getParentDfn (std::vector<CFormDfn*> &array)
{
	uint count = 0;
	uint i;
	for (i=0; i<Parents.size (); i++)
	{
		Parents[i].Parent->getParentDfn (array);
	}
	array.push_back (this);
}

// ***************************************************************************

void CFormDfn::getParentDfn (std::vector<const CFormDfn*> &array) const
{
	uint count = 0;
	uint i;
	for (i=0; i<Parents.size (); i++)
	{
		Parents[i].Parent->getParentDfn (array);
	}
	array.push_back (this);
}

// ***************************************************************************

uint CFormDfn::getNumParent () const
{
	return Parents.size ();
}

// ***************************************************************************

CFormDfn *CFormDfn::getParent (uint parent) const
{
	return Parents[parent].Parent;
}

// ***************************************************************************

const string& CFormDfn::getParentFilename (uint parent) const
{
	return Parents[parent].ParentFilename;
}

// ***************************************************************************

uint CFormDfn::getNumEntry () const
{
	return Entries.size();
}

// ***************************************************************************

void CFormDfn::setNumEntry (uint size)
{
	Entries.resize (size);
}

// ***************************************************************************

const CFormDfn::CEntry &CFormDfn::getEntry (uint entry) const
{
	return Entries[entry];
}

// ***************************************************************************

CFormDfn::CEntry &CFormDfn::getEntry (uint entry)
{
	return Entries[entry];
}

// ***************************************************************************

void CFormDfn::setNumParent (uint size)
{
	Parents.resize (size);
}

// ***************************************************************************

void CFormDfn::setParent (uint parent, CFormLoader &loader, const char *filename)
{
	if (strcmp (filename, "")==0)
		Parents[parent].Parent = NULL;
	else
		Parents[parent].Parent = loader.loadFormDfn (filename);
	Parents[parent].ParentFilename = filename;
}

// ***************************************************************************

void CFormDfn::CEntry::setType (CFormLoader &loader, const char *filename)
{
	TypeElement = EntryType;
	Dfn = NULL;
	Filename = filename;
	Type = loader.loadType (filename);
}

// ***************************************************************************

void CFormDfn::CEntry::setDfn (CFormLoader &loader, const char *filename)
{
	TypeElement = EntryDfn;
	Filename = filename;
	Type = NULL;
	Dfn = loader.loadFormDfn (filename);
}

// ***************************************************************************

void CFormDfn::CEntry::setDfnPointer ()
{
	TypeElement = EntryDfnPointer;
	Filename = "";
	Type = NULL;
	Dfn = NULL;
}

// ***************************************************************************

const std::string &CFormDfn::CEntry::getName () const
{
	return Name;
}

// ***************************************************************************

void CFormDfn::CEntry::setName (const char *name)
{
	Name = name;
}

// ***************************************************************************

const std::string &CFormDfn::CEntry::getDefault () const
{
	return Default;
}

// ***************************************************************************

void CFormDfn::CEntry::setDefault (const char *def)
{
	Default = def;
}

// ***************************************************************************

void CFormDfn::CEntry::setArrayFlag (bool flag)
{
	Array = flag;
}

// ***************************************************************************

bool CFormDfn::CEntry::getArrayFlag () const
{
	return Array;
}

// ***************************************************************************

CFormDfn::CEntry::TType CFormDfn::CEntry::getType () const
{
	return TypeElement;
}

// ***************************************************************************

const std::string &CFormDfn::CEntry::getFilename() const
{
	return Filename;
}

// ***************************************************************************

void CFormDfn::CEntry::setFilename (const char *def)
{
	Filename = def;
}

// ***************************************************************************

CType *CFormDfn::CEntry::getTypePtr ()
{
	return Type;
}

// ***************************************************************************

CFormDfn *CFormDfn::CEntry::getDfnPtr ()
{
	return Dfn;
}

// ***************************************************************************

const CType *CFormDfn::CEntry::getTypePtr () const
{
	return Type;
}

// ***************************************************************************

const CFormDfn *CFormDfn::CEntry::getDfnPtr () const
{
	return Dfn;
}

// ***************************************************************************

} // NLGEORGES
