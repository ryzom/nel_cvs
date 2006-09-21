/** \file header.cpp
 * Georges header file class
 *
 * $Id: header.cpp,v 1.7.54.1 2006/09/21 20:50:31 cado Exp $
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

#include "header.h"
#include "nel/misc/thread.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/common.h"

using namespace NLMISC;

namespace NLGEORGES
{

// ***************************************************************************

void warning (bool exception, const char *format, ... );

// ***************************************************************************

CFileHeader::CFileHeader ()
{
	MajorVersion = 0;
	MinorVersion = 0;
	State = Modified;
	Revision = "$R";
	Revision += "evision$";
}

// ***************************************************************************

void CFileHeader::write (xmlNodePtr node, bool georges4CVS) const
{
	// Version for CVS ?
	if (georges4CVS)
	{
		// Georges version system
		xmlSetProp (node, (const xmlChar*)"Revision", (const xmlChar*)Revision.c_str ());
	}
	else
	{
		// Georges version system
		char tmp[512];
		smprintf (tmp, 512, "%d.%d", MajorVersion, MinorVersion);
		xmlSetProp (node, (const xmlChar*)"Version", (const xmlChar*)tmp);
	}

	// State
	if (State == Modified)
		xmlSetProp (node, (const xmlChar*)"State", (const xmlChar*)"modified");
	else
		xmlSetProp (node, (const xmlChar*)"State", (const xmlChar*)"checked");

	// Comments of the form
	if (!Comments.empty ())
	{
		// Create a new node
		xmlNodePtr child = xmlNewChild ( node, NULL, (const xmlChar*)"COMMENTS", NULL);
		xmlNodePtr textNode = xmlNewText ((const xmlChar *)Comments.c_str());
		xmlAddChild (child, textNode);
	}

	// Logs
	if (!Log.empty ())
	{
		// Create a new node
		xmlNodePtr child = xmlNewChild ( node, NULL, (const xmlChar*)"LOG", NULL);
		xmlNodePtr textNode = xmlNewText ((const xmlChar *)Log.c_str());
		xmlAddChild (child, textNode);
	}
}

// ***************************************************************************

void CFileHeader::addLog (const char *log)
{
	time_t t;
	time (&t);
	if (!Log.empty())
		Log += "\n";
	Log += ctime(&t);
	Log.resize (Log.size()-1);
	Log += " (";
	Log += IThread::getCurrentThread ()->getUserName ();
	Log += ") ";
	Log += log;
}

// ***************************************************************************

void CFileHeader::setComments (const char *comments)
{
	Comments = comments;
}

// ***************************************************************************

void CFileHeader::read (xmlNodePtr root)
{
	// Get the version
	const char *value = (const char*)xmlGetProp (root, (xmlChar*)"Version");
	if (value)
	{
		// Read the version
		if (sscanf (value, "%d.%d", &MajorVersion, &MinorVersion) != 2)
		{
			// Delete the value
			xmlFree ((void*)value);

			// Throw exception
			warning (true, "read", "XML Syntax error in TYPE block line %d, the Version argument is invalid.", 
				(ptrdiff_t)root->content);
		}

		// Delete the value
		xmlFree ((void*)value);
	}
	else
	{
		// Set default
		MajorVersion = 0;
		MinorVersion = 0;
	}

	// Get the revision
	value = (const char*)xmlGetProp (root, (xmlChar*)"Revision");
	if (value)
	{
		// Set the value
		Revision = value;

		// Delete the value
		xmlFree ((void*)value);
	}
	else
	{
		// Set default
		Revision = "$R";
		Revision += "evision$";
	}

	// Get the version
	value = (const char*)xmlGetProp (root, (xmlChar*)"State");
	if (value)
	{
		// Read the version
		if (strcmp (value, "modified") == 0)
		{
			State = Modified;
		}
		else if (strcmp (value, "checked") == 0)
		{
			State = Checked;
		}
		else 
		{
			// Delete the value
			xmlFree ((void*)value);

			// Throw exception
			warning (true, "read", "XML Syntax error in TYPE block line %d, the State argument is invalid.", 
				(ptrdiff_t)root->content);
		}

		// Delete the value
		xmlFree ((void*)value);
	}
	else
	{
		// Set default
		State = Modified;
	}

	// Look for the comment node
	Comments = "";
	xmlNodePtr node = CIXml::getFirstChildNode (root, "COMMENTS");
	if (node)
	{
		// Get a text node
		if ((node = CIXml::getFirstChildNode (node, XML_TEXT_NODE)))
		{
			// Get content
			const char *comments = (const char*)xmlNodeGetContent (node);
			if (comments)
			{
				Comments = comments;

				// Delete the value
				xmlFree ((void*)comments);
			}
		}
	}

	// Look for the log node
	Log = "";
	node = CIXml::getFirstChildNode (root, "LOG");
	if (node)
	{
		// Get a text node
		if ((node = CIXml::getFirstChildNode (node, XML_TEXT_NODE)))
		{
			// Get content
			const char *log = (const char*)xmlNodeGetContent (node);
			if (log)
			{
				Log = log;

				// Delete the value
				xmlFree ((void*)log);
			}
		}
	}
}

// ***************************************************************************

const char *CFileHeader::getStateString (TState state)
{
	if (state == Modified)
		return "Modified";
	else
		return "Checked";
}

// ***************************************************************************

void CFileHeader::warning (bool exception, const char *function, const char *format, ... ) const
{
	// Make a buffer string
	va_list args;
	va_start( args, format );
	char buffer[1024];
	vsnprintf( buffer, 1024, format, args );
	va_end( args );

	// Set the warning
	NLGEORGES::warning (exception, "(CFileHeader::%s) : %s", function, buffer);
}

// ***************************************************************************

} // NLGEORGES
