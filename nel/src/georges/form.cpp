/** \file form.cpp
 * Georges form class
 *
 * $Id: form.cpp,v 1.9 2002/06/11 17:38:58 corvazier Exp $
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
#include "nel/misc/common.h"
#include "nel/misc/path.h"

#include "form.h"
#include "form_loader.h"

using namespace NLMISC;

namespace NLGEORGES
{

// ***************************************************************************
// UForm
// ***************************************************************************

UForm::~UForm ()
{
}

// ***************************************************************************

UFormElm& CForm::getRootNode ()
{
	return Elements;
}

// ***************************************************************************

const UFormElm& CForm::getRootNode () const
{
	return Elements;
}

// ***************************************************************************
// CForm
// ***************************************************************************

CForm::CForm () : Elements (this, NULL, NULL, 0xffffffff)
{
	uint i;
	for (i=0; i<HeldElementCount; i++)
	{
		HeldElements[i] = new CFormElmStruct (this, NULL, NULL, 0xffffffff);
	}
}

// ***************************************************************************

CForm::~CForm ()
{
	uint i;
	for (i=0; i<HeldElementCount; i++)
	{
		delete HeldElements[i];
	}
}

// ***************************************************************************

void CForm::write (xmlDocPtr doc, const char *filename)
{
	// Save the filename
	_Filename = CFile::getFilenameWithoutExtension (filename);

	// Create the first node
	xmlNodePtr node = xmlNewDocNode (doc, NULL, (const xmlChar*)"FORM", NULL);
	xmlDocSetRootElement (doc, node);

	// List of parent
	for (uint parent=0; parent<ParentList.size (); parent++)
	{
		// Parent name not empty ?
		if (!(ParentList[parent].ParentFilename.empty()))
		{
			// Add a parent node
			xmlNodePtr parentNode = xmlNewChild ( node, NULL, (const xmlChar*)"PARENT", NULL );
			xmlSetProp (parentNode, (const xmlChar*)"Filename", (const xmlChar*)ParentList[parent].ParentFilename.c_str());
		}
	}

	// Write elements
	Elements.write (node, this, NULL, true);

	// Write held elements
	uint i;
	for (i=0; i<HeldElementCount; i++)
	{
		HeldElements[i]->write (node, this, NULL, true);
	}

	// Header
	Header.write (node);
}

// ***************************************************************************

void CForm::readParent (const char *parent, CFormLoader &loader)
{
	// Load the parent
	CForm *theParent = (CForm*)loader.loadForm (parent);
	if (theParent != NULL)
	{
		// Set the parent
		if (!insertParent (getParentCount (), parent, theParent))
		{
			// Make an error message
			char tmp[512];
			smprintf (tmp, 512, "Can't set the parent FORM named %s. Check if it is the same form or if it use a differnt formDfn.", parent);

			// Delete the value
			xmlFree ((void*)parent);

			throw EXmlParsingError (tmp);
		}
	}
	else
	{
		// Make an error message
		char tmp[512];
		smprintf (tmp, 512, "Can't load the parent FORM named %s.", parent);

		// Delete the value
		xmlFree ((void*)parent);

		throw EXmlParsingError (tmp);
	}
}

// ***************************************************************************

void CForm::read (xmlNodePtr node, CFormLoader &loader, CFormDfn *dfn, const char *filename)
{
	// Save the filename
	_Filename = CFile::getFilenameWithoutExtension (filename);

	// Reset form
	clean ();

	// Check node name
	if ( ((const char*)node->name == NULL) || (strcmp ((const char*)node->name, "FORM") != 0) )
	{
		// Make an error message
		char tmp[512];
		smprintf (tmp, 512, "Georges FORM XML Syntax error in block line %d, node %s should be FORM", 
			(int)node->content, node->name);
		throw EXmlParsingError (tmp);
	}

	// Get first struct node
	xmlNodePtr child = CIXml::getFirstChildNode (node, "STRUCT");
	if (child == NULL)
	{
		// Make an error message
		char tmp[512];
		smprintf (tmp, 512, "Georges FORM XML Syntax error in block line %d, node %s should have a STRUCT child node", 
			(int)node->content, node->name);
		throw EXmlParsingError (tmp);
	}

	// Read the struct
	Elements.read (child, loader, dfn, this);

	// Get next struct node
	child = CIXml::getNextChildNode (node, "STRUCT");
	uint index = 0;
	while ( (child != NULL) && (index < HeldElementCount))
	{
		HeldElements[index]->read (child, loader, dfn, this);
		index++;
	}
	while (index < HeldElementCount)
	{
		// Build the Form
		HeldElements[index]->build (dfn);
		index++;
	}

	// Get the old parent parameter
	const char *parent = (const char*)xmlGetProp (node, (xmlChar*)"Parent");
	if (parent)
	{
		// Add a parent, xmlFree is done by readParent
		readParent (parent, loader);
	}

	// Read the new parent nodes
	uint parentCount = CIXml::countChildren (node, "PARENT");

	// Reserve some parents
	ParentList.reserve (ParentList.size () + parentCount);

	// Enum children node
	child = CIXml::getFirstChildNode (node, "PARENT");
	while (child)
	{
		parent = (const char*)xmlGetProp (child, (xmlChar*)"Filename");

		// Add a parent, xmlFree is done by readParent
		readParent (parent, loader);

		// Next node <PARENT>
		child = CIXml::getNextChildNode (child, "PARENT");
	}

	// Read the header
	Header.read (node);
}

// ***************************************************************************

const std::string &CForm::getComment () const
{
	return Header.Comments;
}

// ***************************************************************************

bool CForm::insertParent (uint before, const char *filename, CForm *parent)
{
	// Set or reset ?
	nlassert (parent)

	// Must have the same DFN
	if (parent->Elements.FormDfn == Elements.FormDfn)
	{
		// Set members
		std::vector<CParent>::iterator ite = ParentList.insert (ParentList.begin() + before);
		ite->Parent = parent;
		ite->ParentFilename = filename;

		return true;
	}
	else
	{
		nlwarning ("Georges (CForm::insertParent) Can't insert parent form (%s) that has not the same DFN.", filename);
	}

	return false;
}

// ***************************************************************************

void CForm::removeParent (uint parent)
{
	ParentList.erase (ParentList.begin() + parent);
}

// ***************************************************************************

CForm *CForm::getParent (uint parent) const
{
	return ParentList[parent].Parent;
}

// ***************************************************************************

const std::string &CForm::getParentFilename (uint parent) const
{
	return ParentList[parent].ParentFilename;
}

// ***************************************************************************

uint CForm::getParentCount () const
{
	return ParentList.size ();
}

// ***************************************************************************

void CForm::clean ()
{
	clearParents ();
}

// ***************************************************************************

void CForm::clearParents ()
{
	ParentList.clear ();
}

// ***************************************************************************

const std::string &CForm::getFilename () const
{
	return _Filename;
}

} // NLGEORGES

