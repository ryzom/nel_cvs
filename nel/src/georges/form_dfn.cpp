/** \file _form_dfn.cpp
 * Georges form definition class
 *
 * $Id: form_dfn.cpp,v 1.6 2002/05/28 14:06:57 corvazier Exp $
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
#include "form_elm.h"

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
		case UFormDfn::EntryType:
			xmlSetProp (elmPtr, (const xmlChar*)"Type", (const xmlChar*)"Type");
			xmlSetProp (elmPtr, (const xmlChar*)"Filename", (const xmlChar*)Entries[elm].Filename.c_str());
			break;
		case UFormDfn::EntryDfn:
			xmlSetProp (elmPtr, (const xmlChar*)"Type", (const xmlChar*)"Dfn");
			xmlSetProp (elmPtr, (const xmlChar*)"Filename", (const xmlChar*)Entries[elm].Filename.c_str());
			break;
		case UFormDfn::EntryVirtualDfn:
			xmlSetProp (elmPtr, (const xmlChar*)"Type", (const xmlChar*)"DfnPointer");
			break;
		}

		// Is an array ?
		if (Entries[elm].Array)
			xmlSetProp (elmPtr, (const xmlChar*)"Array", (const xmlChar*)"true");

		// Default value for type
		if ((Entries[elm].TypeElement == UFormDfn::EntryType) && (!Entries[elm].Default.empty ()))
			xmlSetProp (elmPtr, (const xmlChar*)"Default", (const xmlChar*)Entries[elm].Default.c_str ());
	}

	// Header
	Header.write (node);
}

// ***************************************************************************

void CFormDfn::read (xmlNodePtr root, CFormLoader &loader, bool forceLoad)
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
			Parents[parentNumber].Parent = loader.loadFormDfn (Parents[parentNumber].ParentFilename.c_str (), forceLoad);
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
					Entries[childNumber].TypeElement = UFormDfn::EntryType;
					type = true;

					// Load the filename
					if (!Entries[childNumber].Filename.empty ())
					{
						Entries[childNumber].Type = loader.loadType (Entries[childNumber].Filename.c_str ());
						if ((Entries[childNumber].Type == NULL) && !forceLoad)
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
					Entries[childNumber].TypeElement = UFormDfn::EntryDfn;
					dfn = true;

					// Load the filename
					if (!Entries[childNumber].Filename.empty ())
					{
						// Load the filename
						Entries[childNumber].Dfn = loader.loadFormDfn (Entries[childNumber].Filename.c_str (), forceLoad);
						if ((Entries[childNumber].Dfn == NULL) && !forceLoad)
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
					Entries[childNumber].TypeElement = UFormDfn::EntryVirtualDfn;
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
		Parents[parent].Parent = loader.loadFormDfn (filename, false);
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
	Dfn = loader.loadFormDfn (filename, false);
}

// ***************************************************************************

void CFormDfn::CEntry::setDfnPointer ()
{
	TypeElement = EntryVirtualDfn;
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

UFormDfn::TEntryType CFormDfn::CEntry::getType () const
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

bool CFormDfn::getNodeByName (const UFormElm **result, const char *name, UFormElm::TWhereIsNode *where) const
{
	const CFormDfn *parentDfn = NULL;
	uint lastElement = 0xffffffff;
	const CFormDfn *nodeDfn = this;
	const CType *nodeType = NULL;
	CFormElm *node = NULL;
	UFormDfn::TEntryType type = UFormDfn::EntryDfn;
	bool array = false;
	bool created;

	if (CFormElm::getIternalNodeByName (NULL, name, &parentDfn, lastElement, &nodeDfn, &nodeType, &node, type, array, CFormElm::Return, created))
	{
		if (type == UFormDfn::EntryDfn)
		{
			nlassert (nodeType);
			*result = NULL;
			if (where)
				*where = NodeType;
			return true;
		}
		else 
		{
			nlassert (type == UFormDfn::EntryDfn);
			nlassert (nodeDfn);
			*result = nodeDfn;
			if (where)
				*where = NodeDfn;
			return true;
		}
	}

	return false;
}

// ***************************************************************************

bool CFormDfn::getNodeByName (UFormElm **result, const char *name, UFormElm::TWhereIsNode *where)
{
	const UFormElm *resultConst = NULL;
	if (((const UFormElm*)this)->getNodeByName (&resultConst, name, where))
	{
		*result = const_cast<UFormElm*> (resultConst);
		return true;
	}
	return false;
}

// ***************************************************************************

bool CFormDfn::getValueByName (std::string &result, const char *name, bool evaluate, TWhereIsValue *where) const
{
	const CFormDfn *parentDfn = NULL;
	uint lastElement = 0xffffffff;
	const CFormDfn *nodeDfn = this;
	const CType *nodeType = NULL;
	CFormElm *node = NULL;
	UFormDfn::TEntryType type = UFormDfn::EntryDfn;
	bool array = false;
	bool created;

	if (CFormElm::getIternalNodeByName (NULL, name, &parentDfn, lastElement, &nodeDfn, &nodeType, &node, type, array, CFormElm::Return, created))
	{
		if (type == UFormDfn::EntryType)
		{
			nlassert (nodeType);
			if (nodeType->getValue (result, NULL, NULL, *parentDfn, lastElement, evaluate, (uint32*)where) )
				return true;
		}
		else
		{
			nlwarning ("Georges (CFormDfn::getValueByName) : %s node is note an atom, it is a structure.", name);
		}
	}
	return false;
}

// ***************************************************************************

bool CFormDfn::getValueByName (sint8 &result, const char *name, bool evaluate, TWhereIsValue *where) const
{
	string tmp;
	if (getValueByName (tmp, name, evaluate, where))
	{
		CFormElm::convertValue (result, tmp.c_str ());
	}
	return false;
}

// ***************************************************************************

bool CFormDfn::getValueByName (uint8 &result, const char *name, bool evaluate, TWhereIsValue *where) const
{
	string tmp;
	if (getValueByName (tmp, name, evaluate, where))
	{
		CFormElm::convertValue (result, tmp.c_str ());
	}
	return false;
}

// ***************************************************************************

bool CFormDfn::getValueByName (sint16 &result, const char *name, bool evaluate, TWhereIsValue *where) const
{
	string tmp;
	if (getValueByName (tmp, name, evaluate, where))
	{
		CFormElm::convertValue (result, tmp.c_str ());
	}
	return false;
}

// ***************************************************************************

bool CFormDfn::getValueByName (uint16 &result, const char *name, bool evaluate, TWhereIsValue *where) const
{
	string tmp;
	if (getValueByName (tmp, name, evaluate, where))
	{
		CFormElm::convertValue (result, tmp.c_str ());
	}
	return false;
}

// ***************************************************************************

bool CFormDfn::getValueByName (sint32 &result, const char *name, bool evaluate, TWhereIsValue *where) const
{
	string tmp;
	if (getValueByName (tmp, name, evaluate, where))
	{
		CFormElm::convertValue (result, tmp.c_str ());
	}
	return false;
}

// ***************************************************************************

bool CFormDfn::getValueByName (uint32 &result, const char *name, bool evaluate, TWhereIsValue *where) const
{
	string tmp;
	if (getValueByName (tmp, name, evaluate, where))
	{
		CFormElm::convertValue (result, tmp.c_str ());
	}
	return false;
}

// ***************************************************************************

bool CFormDfn::getValueByName (float &result, const char *name, bool evaluate, TWhereIsValue *where) const
{
	string tmp;
	if (getValueByName (tmp, name, evaluate, where))
	{
		CFormElm::convertValue (result, tmp.c_str ());
	}
	return false;
}

// ***************************************************************************

bool CFormDfn::getValueByName (double &result, const char *name, bool evaluate, TWhereIsValue *where) const
{
	string tmp;
	if (getValueByName (tmp, name, evaluate, where))
	{
		CFormElm::convertValue (result, tmp.c_str ());
	}
	return false;
}

// ***************************************************************************

bool CFormDfn::getValueByName (bool &result, const char *name, bool evaluate, TWhereIsValue *where) const
{
	string tmp;
	if (getValueByName (tmp, name, evaluate, where))
	{
		CFormElm::convertValue (result, tmp.c_str ());
	}
	return false;
}

// ***************************************************************************

bool CFormDfn::isArray () const
{
	nlwarning ("Georges (CFormDfn::isArray) : this node is not an array"); 
	return false;
}

// ***************************************************************************

bool CFormDfn::getArraySize (uint &size) const
{
	nlwarning ("Georges (CFormDfn::getArraySize) : this node is not an array"); 
	return false;
}

// ***************************************************************************

bool CFormDfn::getArrayNode (const UFormElm **result, uint arrayIndex) const
{
	nlwarning ("Georges (CFormDfn::getArrayNode) : this node is not an array"); 
	return false;
}

// ***************************************************************************

bool CFormDfn::getArrayNode (UFormElm **result, uint arrayIndex)
{
	nlwarning ("Georges (CFormDfn::getArrayNode) : this node is not an array"); 
	return false;
}

// ***************************************************************************

bool CFormDfn::getArrayValue (std::string &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const
{
	nlwarning ("Georges (CFormDfn::getArrayValue) : this node is not an array"); 
	return false;
}

// ***************************************************************************

bool CFormDfn::getArrayValue (sint8 &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const
{
	nlwarning ("Georges (CFormDfn::getArrayValue) : this node is not an array"); 
	return false;
}

// ***************************************************************************

bool CFormDfn::getArrayValue (uint8 &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const
{
	nlwarning ("Georges (CFormDfn::getArrayValue) : this node is not an array"); 
	return false;
}

// ***************************************************************************

bool CFormDfn::getArrayValue (sint16 &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const
{
	nlwarning ("Georges (CFormDfn::getArrayValue) : this node is not an array"); 
	return false;
}

// ***************************************************************************

bool CFormDfn::getArrayValue (uint16 &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const
{
	nlwarning ("Georges (CFormDfn::getArrayValue) : this node is not an array"); 
	return false;
}

// ***************************************************************************

bool CFormDfn::getArrayValue (sint32 &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const
{
	nlwarning ("Georges (CFormDfn::getArrayValue) : this node is not an array"); 
	return false;
}

// ***************************************************************************

bool CFormDfn::getArrayValue (uint32 &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const
{
	nlwarning ("Georges (CFormDfn::getArrayValue) : this node is not an array"); 
	return false;
}

// ***************************************************************************

bool CFormDfn::getArrayValue (float &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const
{
	nlwarning ("Georges (CFormDfn::getArrayValue) : this node is not an array"); 
	return false;
}

// ***************************************************************************

bool CFormDfn::getArrayValue (double &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const
{
	nlwarning ("Georges (CFormDfn::getArrayValue) : this node is not an array"); 
	return false;
}

// ***************************************************************************

bool CFormDfn::getArrayValue (bool &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const
{
	nlwarning ("Georges (CFormDfn::getArrayValue) : this node is not an array"); 
	return false;
}

// ***************************************************************************

bool CFormDfn::isStruct () const
{
	return true;
}

// ***************************************************************************

bool CFormDfn::isVirtualStruct () const
{
	return false;
}

// ***************************************************************************

bool CFormDfn::getStructSize (uint &size) const
{
	std::vector<const CFormDfn*> array;
	array.reserve (countParentDfn ());
	getParentDfn (array);
	uint entryCount = 0;
	for (uint dfn=0; dfn<array.size (); dfn++)
	{
		entryCount += array[dfn]->Entries.size ();
	}
	size = entryCount;
	return true;
}

// ***************************************************************************

bool CFormDfn::getStructNodeName (uint element, std::string &result) const
{
	std::vector<const CFormDfn*> array;
	array.reserve (countParentDfn ());
	getParentDfn (array);
	for (uint dfn=0; dfn<array.size (); dfn++)
	{
		if (element < array[dfn]->Entries.size ())
		{
			result = array[dfn]->Entries[element].Name;
			return true;
		}
		element -= array[dfn]->Entries.size ();
	}

	// Error
	nlwarning ("Georges (CFormDfn::getStructNodeName) : element id doesn't exist"); 
	return false;
}

// ***************************************************************************

bool CFormDfn::getStructNode (uint element, const UFormElm **result) const
{
	std::vector<const CFormDfn*> array;
	array.reserve (countParentDfn ());
	getParentDfn (array);
	for (uint dfn=0; dfn<array.size (); dfn++)
	{
		if (element < array[dfn]->Entries.size ())
		{
			if (array[dfn]->Entries[element].TypeElement == UFormDfn::EntryType)
				*result = NULL;
			else 
				*result = array[dfn]->Entries[element].Dfn;
			return true;
		}
		element -= array[dfn]->Entries.size ();
	}

	// Error
	nlwarning ("Georges (CFormDfn::getStructNodeName) : element id doesn't exist"); 
	return false;
}

// ***************************************************************************

bool CFormDfn::getStructNode (uint element, UFormElm **result)
{
	std::vector<const CFormDfn*> array;
	array.reserve (countParentDfn ());
	getParentDfn (array);
	for (uint dfn=0; dfn<array.size (); dfn++)
	{
		if (element < array[dfn]->Entries.size ())
		{
			if (array[dfn]->Entries[element].TypeElement == UFormDfn::EntryType)
				*result = NULL;
			else 
				*result = array[dfn]->Entries[element].Dfn;
			return true;
		}
		element -= array[dfn]->Entries.size ();
	}

	// Error
	nlwarning ("Georges (CFormDfn::getStructNodeName) : element id doesn't exist"); 
	return false;
}

// ***************************************************************************

bool CFormDfn::isAtom () const
{
	return false;
}

// ***************************************************************************

bool CFormDfn::getValue (std::string &result, bool evaluate) const
{
	nlwarning ("Georges (CFormDfn::getValue) : the node is not an atom."); 
	return false;
}

// ***************************************************************************

bool CFormDfn::getValue (sint8 &result, bool evaluate) const
{
	nlwarning ("Georges (CFormDfn::getValue) : the node is not an atom."); 
	return false;
}

// ***************************************************************************

bool CFormDfn::getValue (uint8 &result, bool evaluate) const
{
	nlwarning ("Georges (CFormDfn::getValue) : the node is not an atom."); 
	return false;
}

// ***************************************************************************

bool CFormDfn::getValue (sint16 &result, bool evaluate) const
{
	nlwarning ("Georges (CFormDfn::getValue) : the node is not an atom."); 
	return false;
}

// ***************************************************************************

bool CFormDfn::getValue (uint16 &result, bool evaluate) const
{
	nlwarning ("Georges (CFormDfn::getValue) : the node is not an atom."); 
	return false;
}

// ***************************************************************************

bool CFormDfn::getValue (sint32 &result, bool evaluate) const
{
	nlwarning ("Georges (CFormDfn::getValue) : the node is not an atom."); 
	return false;
}

// ***************************************************************************

bool CFormDfn::getValue (uint32 &result, bool evaluate) const
{
	nlwarning ("Georges (CFormDfn::getValue) : the node is not an atom."); 
	return false;
}

// ***************************************************************************

bool CFormDfn::getValue (float &result, bool evaluate) const
{
	nlwarning ("Georges (CFormDfn::getValue) : the node is not an atom."); 
	return false;
}

// ***************************************************************************

bool CFormDfn::getValue (double &result, bool evaluate) const
{
	nlwarning ("Georges (CFormDfn::getValue) : the node is not an atom."); 
	return false;
}

// ***************************************************************************

bool CFormDfn::getValue (bool &result, bool evaluate) const
{
	nlwarning ("Georges (CFormDfn::getValue) : the node is not an atom."); 
	return false;
}

// ***************************************************************************

CFormDfn *CFormDfn::getSubDfn (uint index, uint &dfnIndex)
{
	// Get the sub DFN
	vector<CFormDfn*> parentDfn;
	parentDfn.reserve (countParentDfn ());
	getParentDfn (parentDfn);

	// For each parent
	uint dfn;
	dfnIndex = index;
	uint parentSize = parentDfn.size();
	for (dfn=0; dfn<parentSize; dfn++)
	{
		// Good element ?
		uint size = parentDfn[dfn]->Entries.size ();
		if (dfnIndex<size)
			return parentDfn[dfn];
		dfnIndex -= size;
	}

	// Should be found..
	nlstop
	return NULL;
}

// ***************************************************************************

const CFormDfn *CFormDfn::getSubDfn (uint index, uint &dfnIndex) const
{
	// Get the sub DFN
	vector<const CFormDfn*> parentDfn;
	parentDfn.reserve (countParentDfn ());
	getParentDfn (parentDfn);

	// For each parent
	uint dfn;
	dfnIndex = index;
	uint parentSize = parentDfn.size();
	for (dfn=0; dfn<parentSize; dfn++)
	{
		// Good element ?
		uint size = parentDfn[dfn]->Entries.size ();
		if (dfnIndex<size)
			return parentDfn[dfn];
		dfnIndex -= size;
	}

	// Should be found..
	nlstop
	return NULL;
}

// ***************************************************************************

bool CFormDfn::getEntryType (uint entry, TEntryType &type, bool &array) const
{
	if (entry < Entries.size ())
	{
		type = Entries[entry].TypeElement;
		array = Entries[entry].Array;
		return true;
	}
	nlwarning ("Georges (CFormDfn::getEntryType) : wrong entry ID."); 
	return false;
}

// ***************************************************************************

bool CFormDfn::getEntryFilename (uint entry, std::string& filename) const
{
	if (entry < Entries.size ())
	{
		filename = Entries[entry].Filename;
		return true;
	}
	nlwarning ("Georges (CFormDfn::getEntryFilename) : wrong entry ID."); 
	return false;
}

// ***************************************************************************

bool CFormDfn::getEntryName (uint entry, std::string &name) const
{
	if (entry < Entries.size ())
	{
		if (Entries[entry].TypeElement != EntryVirtualDfn)
		{
			name = Entries[entry].Name;
			return true;
		}
		nlwarning ("Georges (CFormDfn::getEntryName) : the entry is a virtual DFN."); 
		return false;
	}
	nlwarning ("Georges (CFormDfn::getEntryName) : wrong entry ID."); 
	return false;
}

// ***************************************************************************

bool CFormDfn::getEntryDfn (uint entry, UFormDfn **dfn)
{
	if (entry < Entries.size ())
	{
		if (Entries[entry].TypeElement == EntryDfn)
		{
			*dfn = Entries[entry].Dfn;
			return true;
		}
		else
			nlwarning ("Georges (CFormDfn::getEntryDfn) : this entry is not a DFN."); 
	}
	nlwarning ("Georges (CFormDfn::getEntryDfn) : wrong entry ID."); 
	return false;
}

// ***************************************************************************

bool CFormDfn::getEntryType (uint entry, UType **type)
{
	if (entry < Entries.size ())
	{
		if (Entries[entry].TypeElement == EntryType)
		{
			*type = Entries[entry].Type;
			return true;
		}
		else
			nlwarning ("Georges (CFormDfn::getEntryType) : this entry is not a type."); 
	}
	nlwarning ("Georges (CFormDfn::getEntryType) : wrong entry ID."); 
	return false;
}

// ***************************************************************************

uint CFormDfn::getNumParents () const
{
	return Parents.size ();
}

// ***************************************************************************

bool CFormDfn::getParent (uint parent, UFormDfn **parentRet)
{
	if (parent < Parents.size ())
	{
		*parentRet = Parents[parent].Parent;
		return true;
	}
	nlwarning ("Georges (CFormDfn::getParent) : wrong parent ID."); 
	return false;
}


// ***************************************************************************

bool CFormDfn::getParentFilename (uint parent, std::string &filename) const
{
	if (parent < Parents.size ())
	{
		filename = Parents[parent].ParentFilename;
		return true;
	}
	nlwarning ("Georges (CFormDfn::getParentFilename) : wrong parent ID."); 
	return false;
}

// ***************************************************************************

const std::string& CFormDfn::getComment (std::string &comment) const
{
	return Header.Comments;
}

// ***************************************************************************

} // NLGEORGES
