/** \file form.cpp
 * Georges form class
 *
 * $Id: form.cpp,v 1.5 2002/05/22 12:09:44 cado Exp $
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

#include "form.h"
#include "form_loader.h"

using namespace NLMISC;

namespace NLGEORGES
{

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

void CForm::write (xmlDocPtr doc) const
{
	// Create the first node
	xmlNodePtr node = xmlNewDocNode (doc, NULL, (const xmlChar*)"FORM", NULL);
	xmlDocSetRootElement (doc, node);

	// Parent filename
	if (!ParentFilename.empty())
		xmlSetProp (node, (const xmlChar*)"Parent", (const xmlChar*)ParentFilename.c_str());

	// Write elements
	Elements.write (node, this, NULL, true);

	// Header
	Header.write (node);
}

// ***************************************************************************

void CForm::read (xmlNodePtr node, CFormLoader &loader, CFormDfn *dfn)
{
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

	// Get the parent
	const char *parent = (const char*)xmlGetProp (node, (xmlChar*)"Parent");
	if (parent)
	{
		// Set the parent
		ParentFilename = parent;

		// Load the parent
		CForm *theParent = (CForm*)loader.loadForm (parent);
		if (theParent != NULL)
		{
			// Set the parent
			if (!setParent (parent, theParent))
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

	// Read the header
	Header.read (node);
}

// ***************************************************************************

bool CForm::setParent (const char *filename, CForm *parent)
{
	if (parent)
	{
		if (parent->Elements.FormDfn == Elements.FormDfn)
		{
			// Set members
			Parent = parent;
			ParentFilename = filename;

			// Set the elements
			if (!Elements.setParent (&parent->Elements))
			{
				nlverify (setParent ("", NULL));
				return false;
			}
			return true;
		}
	}
	else
	{
		ParentFilename = "";
		nlverify (Elements.setParent (NULL));
		return true;
	}

	return false;
}

// ***************************************************************************

CForm *CForm::getParent () const
{
	return Parent;
}

// ***************************************************************************

const std::string &CForm::getParentFilename () const
{
	return ParentFilename;
}

// ***************************************************************************

} // NLGEORGES
