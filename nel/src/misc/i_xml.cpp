/** \file i_xml.cpp
 * Input xml stream
 *
 * $Id: i_xml.cpp,v 1.13.2.2 2003/05/16 17:46:10 lecroart Exp $
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

#include "stdmisc.h"

#include "nel/misc/i_xml.h"

#ifndef NL_DONT_USE_EXTERNAL_CODE

// Include from libxml2
#include <libxml/xmlerror.h>

using namespace std;

#define NLMISC_READ_BUFFER_SIZE 1024

namespace NLMISC 
{

// *********************************************************

const char SEPARATOR = ' ';

// ***************************************************************************

#define readnumber(dest,thetype,digits,convfunc) \
	string number_as_string; \
	serialSeparatedBufferIn( number_as_string ); \
	dest = (thetype)convfunc( number_as_string.c_str() );

// ***************************************************************************

inline void CIXml::flushContentString ()
{
	// String size
	_ContentString.erase ();

	// Reset
	_ContentStringIndex = 0;
}

// ***************************************************************************

CIXml::CIXml () : IStream (true /* Input mode */)
{
	// Not initialized
	_Parser = NULL;
	_CurrentElement = NULL;
	_CurrentNode = NULL;
	_PushBegin = false;
	_AttribPresent = false;
	_ErrorString = "";
}

// ***************************************************************************

CIXml::~CIXml ()
{
	// Release
	release ();
}

// ***************************************************************************

void CIXml::release ()
{
	// Release the parser
	if (_Parser)
	{
		// Free it
		xmlFreeDoc (_Parser->myDoc);
		xmlFreeParserCtxt (_Parser);
		// xmlCleanupParser (); Crash..
		_Parser = NULL;
	}

	// Not initialized
	_Parser = NULL;
	_CurrentElement = NULL;
	_CurrentNode = NULL;
	_PushBegin = false;
	_AttribPresent = false;
	_ErrorString = "";
}

// ***************************************************************************

void xmlGenericErrorFuncRead (void *ctx, const char *msg, ...)
{
	// Get the error string
	string str;
	NLMISC_CONVERT_VARGS (str, msg, NLMISC::MaxCStringSize);
	((CIXml*)ctx)->_ErrorString += str;
}

// ***************************************************************************

bool CIXml::init (IStream &stream)
{
	// Release
	release ();

	// Input stream ?
	if (stream.isReading())
	{
		// Set XML mode
		setXMLMode (true);

		// Get current position
		sint32 pos = stream.getPos ();

		// Go to end
		bool seekGood = stream.seek (0, end);
		nlassert (seekGood);

		// Get input stream length
		sint32 length = stream.getPos () - pos;

		// Go to start
		stream.seek (pos, begin);

		// The read buffer
        char buffer[NLMISC_READ_BUFFER_SIZE];

		// Fill the buffer
		stream.serialBuffer ((uint8*)buffer, 4);
		length -= 4;

		// Set error handler
		_ErrorString = "";
		xmlSetGenericErrorFunc	(this, xmlGenericErrorFuncRead);

		// Ask to get debug info
		xmlLineNumbersDefault(1);

		// The parser context
        _Parser = xmlCreatePushParserCtxt(NULL, NULL, buffer, 4, NULL);
		nlassert (_Parser);

		// For all the file
        while (length>=NLMISC_READ_BUFFER_SIZE)
		{
			// Fill the buffer
			stream.serialBuffer ((uint8*)buffer, NLMISC_READ_BUFFER_SIZE);

			// Read a buffer
            int res = xmlParseChunk(_Parser, buffer, NLMISC_READ_BUFFER_SIZE, 0);

			// Error code ?
			if (res)
			{
				throw EXmlParsingError (_ErrorString);
			}

			// Length left
			length -= NLMISC_READ_BUFFER_SIZE;
        }

		// Fill the buffer
		stream.serialBuffer ((uint8*)buffer, length);

		// Parse the last buffer
		int res = xmlParseChunk(_Parser, buffer, length, 1);

		// Error code ?
		if (res)
		{
			throw EXmlParsingError (_ErrorString);
		}

		// Ok
		return true;
	}
	else
	{
		nlwarning ("The stream is not an input stream.");
	}

	// Error
	return false;
}

// ***************************************************************************

void CIXml::serialSeparatedBufferIn ( string &value, bool checkSeparator )
{
	nlassert( isReading() );

	// Output stream has been initialized ?
	if ( _Parser )
	{
		// Current node presents ?
		if (_CurrentElement)
		{
			// Write a push attribute ?
			if (_PushBegin)
			{
				// Current attrib is set ?
				if (_AttribPresent)
				{
					// Should have a current element
					nlassert (_CurrentElement);

					// Get the attribute
					xmlChar *attributeValue = xmlGetProp (_CurrentElement, (const xmlChar*)_AttribName.c_str());

					// Attribute is here ?
					if (attributeValue)
					{
						// Copy the value
						value = (const char*)attributeValue;

						// Delete the value
						xmlFree ((void*)attributeValue);
					}
					else
					{
						// Node name must not be NULL
						nlassert (_CurrentElement->name);

						// Make an error message
						char tmp[512];
						smprintf (tmp, 512, "NeL XML Syntax error in block line %d\nAttribute \"%s\" is missing in node \"%s\"", 
							(int)_CurrentElement->content, _AttribName.c_str(), _CurrentElement->name);
						throw EXmlParsingError (tmp);
					}

					// The attribute has been used 
					_AttribPresent = false;
				}
				else
				{
					// * Error, the stream don't use XML streaming properly
					// * You must take care of this in your last serial call:
					// * - Between xmlPushBegin() and xmlPushEnd(), before each serial, you must set the attribute name with xmlSetAttrib.
					// * - Between xmlPushBegin() and xmlPushEnd(), you must serial only basic objects (numbers and strings).
					nlerror ( "Error, the stream don't use XML streaming properly" );
				}
			}
			else
			{
				// Content length
				uint length = _ContentString.length();

				// String empty ?
				if (length==0)
				{
					// Try to open the node
					do
					{
						// If no more node, empty string
						if (_CurrentNode == NULL)
						{
							value = "";
							_ContentStringIndex = 0;
							_ContentString.erase ();
							return;
						}

						// Node with the good name
						if (_CurrentNode->type == XML_TEXT_NODE)
						{
							// Stop searching
							break;
						}
						else
							// Get next
							_CurrentNode = _CurrentNode->next;
					}
					while (_CurrentNode);

					// Not found ?
					if (_CurrentNode != NULL)
					{
						// Read the content
						const char *content = (const char*)xmlNodeGetContent (_CurrentNode);
						if (content)
						{
							_ContentString = content;

							// Delete the value
							xmlFree ((void*)content);
						}
						else
							_ContentString.erase ();

						// Set the current index
						_ContentStringIndex = 0;

						// New length
						length = _ContentString.length();
					}
				}

				// Keyword in the buffer ?
				if (_ContentStringIndex < length)
				{
					// First index
					uint first = _ContentStringIndex;

					// Have to take care of separators ?
					if (checkSeparator)
					{
						// Scan to the end
						while (_ContentStringIndex < length)
						{
							// Not a separator ?
							if ( (_ContentString[_ContentStringIndex]==SEPARATOR) || (_ContentString[_ContentStringIndex]=='\n') )
							{
								_ContentStringIndex++;
								break;
							}

							// Next char
							_ContentStringIndex++;
						}
					}
					else
					{
						// Copy all the string
						_ContentStringIndex = length;
					}

					// Make a string
					value.assign (_ContentString, first, _ContentStringIndex-first);
				}
				else
				{
					// Should have a name
					nlassert (_CurrentElement->name);

					// Make an error message
					char tmp[512];
					smprintf (tmp, 512, "NeL XML Syntax error in block line %d \nMissing keywords in text child node in the node %s", 
						(int)_CurrentElement->content, _CurrentElement->name);
					throw EXmlParsingError (tmp);
				}
			}
		}
		else
		{
			// * Error, no current node present.
			// * Check that your serial is initialy made between a xmlPushBegin and xmlPushEnd calls.
			nlerror ( "Error, the stream don't use XML streaming properly" );
		}
	}
	else
	{
		nlerror ( "Output stream has not been initialized" );
	}
}

// ***************************************************************************

void CIXml::serial(uint8 &b)
{
	// Read the number
	readnumber( b, uint8, 3, atoi );
}

// ***************************************************************************

void CIXml::serial(sint8 &b)
{
	readnumber( b, sint8, 4, atoi );
}

// ***************************************************************************

void CIXml::serial(uint16 &b)
{
	readnumber( b, uint16, 5, atoi );
}

// ***************************************************************************

void CIXml::serial(sint16 &b)
{
	readnumber( b, sint16, 6, atoi );
}

// ***************************************************************************

inline uint32 atoui( const char *ident)
{
	return (uint32) strtoul (ident, NULL, 10);
}

void CIXml::serial(uint32 &b)
{
	readnumber( b, uint32, 10, atoui );
}

// ***************************************************************************

void CIXml::serial(sint32 &b) 
{
	readnumber( b, sint32, 11, atoi );
}

// ***************************************************************************

void CIXml::serial(uint64 &b) 
{
	readnumber( b, uint64, 20, atoiInt64 );
}

// ***************************************************************************

void CIXml::serial(sint64 &b) 
{
	readnumber( b, sint64, 20, atoiInt64 );
}

// ***************************************************************************

void CIXml::serial(float &b) 
{
	readnumber( b, float, 128, atof );
}

// ***************************************************************************

void CIXml::serial(double &b) 
{
	readnumber( b, double, 128, atof );
}

// ***************************************************************************

void CIXml::serial(bool &b)
{
	serialBit(b);
}

// ***************************************************************************

void CIXml::serialBit(bool &bit)
{
	uint8 u;
	serial (u);
	bit = (u!=0);
}

// ***************************************************************************

#ifndef NL_OS_CYGWIN
void CIXml::serial(char &b) 
{
	string toto;
	serialSeparatedBufferIn ( toto );

	// Good value ?
	if (toto.length()!=1)
	{
		// Protect error
		if (_Parser)
		{
			// Should have a name
			nlassert (_CurrentElement->name);

			// Make an error message
			char tmp[512];
			smprintf (tmp, 512, "NeL XML Syntax error in block line %d \nValue is not a char in the node named %s", 
				(int)_CurrentElement->content, _CurrentElement->name);
			throw EXmlParsingError (tmp);
		}
		else
		{
			nlerror ( "Output stream has not been initialized" );
		}
	}
	else
		b=toto[0];
}
#endif // NL_OS_CYGWIN

// ***************************************************************************

void CIXml::serial(std::string &b)
{
	nlassert( isReading() );

	// Attibute ?
	if (_PushBegin)
	{
		// Only serial the string
		serialSeparatedBufferIn ( b, false );
	}
	else
	{
		// Open a string node
		xmlPush ("S");

		// Serial the string
		serialSeparatedBufferIn ( b, false );

		// Close the node
		xmlPop ();
	}
}

// ***************************************************************************

void CIXml::serial(ucstring &b)
{
	nlassert( isReading() );

	/// \todo hulud: handle ucstring to utf-8.

	// Serial a simple string
	string tmp;

	// Serial this string
	serial (tmp);

	// Return a ucstring
	b = tmp;
}

// ***************************************************************************

void CIXml::serialBuffer(uint8 *buf, uint len)
{
	// Open a node
	xmlPush ("BUFFER");

	// Serialize the buffer
	for (uint i=0; i<len; i++)
	{
		xmlPush ("ELM");

		serial (buf[i]);

		xmlPop ();
	}

	// Close the node
	xmlPop ();
}

// ***************************************************************************

bool CIXml::xmlPushBeginInternal (const char *nodeName)
{
	nlassert( isReading() );

	// Check _Parser
	if ( _Parser )
	{
		// Can make a xmlPushBegin ?
		if ( ! _PushBegin )
		{
			// Current node exist ?
			if (_CurrentNode==NULL)
			{
				// Get the first node
				_CurrentNode = xmlDocGetRootElement (_Parser->myDoc);
				
				// Has a root node ?
				if (_CurrentNode)
				{
					// Node name must not be NULL
					nlassert (_CurrentNode->name);

					// Node element with the good name ?
					if ( (_CurrentNode->type != XML_ELEMENT_NODE) || ( (const char*)_CurrentNode->name != string(nodeName)) )
					{
						// Make an error message
						char tmp[512];
						smprintf (tmp, 512, "NeL XML Syntax error : root node has the wrong name : \"%s\" should have \"%s\"",
							_CurrentNode->name, nodeName);
						throw EXmlParsingError (tmp);
					}
				}
				else
				{
					// Make an error message
					char tmp[512];
					smprintf (tmp, 512, "NeL XML Syntax error : no root node found.");
					throw EXmlParsingError (tmp);
				}
			}

			// Try to open the node
			do
			{
				// Node name must not be NULL
				nlassert (_CurrentNode->name);

				// Node with the good name
				if ( (_CurrentNode->type == XML_ELEMENT_NODE) && ( (const char*)_CurrentNode->name == string(nodeName)) )
				{
					// Save current element
					_CurrentElement = _CurrentNode;

					// Stop searching
					break;
				}
				else
					// Get next
					_CurrentNode = _CurrentNode->next;
			}
			while (_CurrentNode);

			// Not found ?
			if (_CurrentNode == NULL)
			{
				// Make an error message
				char tmp[512];
				smprintf (tmp, 512, "NeL XML Syntax error in block line %d \nCan't open the node named %s in node named %s", 
					(int)_CurrentElement->content, nodeName, _CurrentElement->name);
				throw EXmlParsingError (tmp);
			}
			
			// Get first child
			_CurrentNode = _CurrentNode->children;

			// Push begun
			_PushBegin = true;

			// Flush current string
			flushContentString ();
		}
		else
		{
			nlerror ( "You must close your xmlPushBegin - xmlPushEnd before calling a new xmlPushBegin.");
			return false;
		}
	}
	else
	{
		nlerror ( "Output stream has not been initialized.");
		return false;
	}

	// Ok
	return true;
}

// ***************************************************************************

bool CIXml::xmlPushEndInternal ()
{
	nlassert( isReading() );

	// Check _Parser
	if ( _Parser )
	{
		// Can make a xmlPushEnd ?
		if ( _PushBegin )
		{
			// Push begun
			_PushBegin = false;
		}
		else
		{
			nlerror ( "You must call xmlPushBegin before calling xmlPushEnd.");
			return false;
		}
	}
	else
	{
		nlerror ( "Output stream has not been initialized.");
		return false;
	}

	// Ok
	return true;
}

// ***************************************************************************

bool CIXml::xmlPopInternal ()
{
	nlassert( isReading() );

	// Check _Parser
	if ( _Parser )
	{
		// Not in the push mode ?
		if ( ! _PushBegin )
		{
			// Some content to write ?
			flushContentString ();

			// Get parents
			_CurrentNode = _CurrentElement;
			_CurrentElement = _CurrentElement->parent;
			_CurrentNode = _CurrentNode->next;
		}
		else
		{
			nlerror ( "You must call xmlPop after xmlPushEnd.");
			return false;
		}
	}
	else
	{
		nlerror ( "Output stream has not been initialized.");
		return false;
	}

	// Ok
	return true;
}

// ***************************************************************************

bool CIXml::xmlSetAttribInternal (const char *attribName)
{
	nlassert( isReading() );

	// Check _Parser
	if ( _Parser )
	{
		// Can make a xmlPushEnd ?
		if ( _PushBegin )
		{
			// Set attribute name
			_AttribName = attribName;

			// Attribute name is present
			_AttribPresent = true;
		}
		else
		{
			nlerror ( "You must call xmlSetAttrib between xmlPushBegin and xmlPushEnd calls.");
			return false;
		}
	}
	else
	{
		nlerror ( "Output stream has not been initialized.");
		return false;
	}

	// Ok
	return true;
}

// ***************************************************************************

bool CIXml::xmlBreakLineInternal ()
{
	// Ok
	return true;
}

// ***************************************************************************

bool CIXml::xmlCommentInternal (const char *comment)
{
	// Ok
	return true;
}

// ***************************************************************************

xmlNodePtr CIXml::getFirstChildNode (xmlNodePtr parent, const char *childName)
{
	xmlNodePtr child = parent->children;
	while (child)
	{
		if (strcmp ((const char*)child->name, childName) == 0)
			return child;
		child = child->next;
	}
	return NULL;
}

// ***************************************************************************

xmlNodePtr CIXml::getNextChildNode (xmlNodePtr last, const char *childName)
{
	last = last->next;
	while (last)
	{
		if (strcmp ((const char*)last->name, childName) == 0)
			return last;
		last = last->next;
	}
	return NULL;
}

// ***************************************************************************

xmlNodePtr CIXml::getFirstChildNode (xmlNodePtr parent, xmlElementType type)
{
	xmlNodePtr child = parent->children;
	while (child)
	{
		if (child->type == type)
			return child;
		child = child->next;
	}
	return NULL;
}

// ***************************************************************************

xmlNodePtr CIXml::getNextChildNode (xmlNodePtr last, xmlElementType type)
{
	last = last->next;
	while (last)
	{
		if (last->type == type)
			return last;
		last = last->next;
	}
	return NULL;
}

// ***************************************************************************

uint CIXml::countChildren (xmlNodePtr node, const char *childName)
{
	uint count=0;
	xmlNodePtr child = getFirstChildNode (node, childName);
	while (child)
	{
		count++;
		child = getNextChildNode (child, childName);
	}
	return count;
}

// ***************************************************************************

uint CIXml::countChildren (xmlNodePtr node, xmlElementType type)
{
	uint count=0;
	xmlNodePtr child = getFirstChildNode (node, type);
	while (child)
	{
		count++;
		child = getNextChildNode (child, type);
	}
	return count;
}

// ***************************************************************************

xmlNodePtr CIXml::getRootNode () const
{
	if (_Parser)
		if (_Parser->myDoc)
			return xmlDocGetRootElement (_Parser->myDoc);
	return NULL;
}

// ***************************************************************************
	
bool CIXml::getPropertyString (std::string &result, xmlNodePtr node, const char *property)
{
	// Get the value
	const char *value = (const char*)xmlGetProp (node, (xmlChar*)property);
	if (value)
	{
		// Active value
		result = value;

		// Delete the value
		xmlFree ((void*)value);

		// Found
		return true;
	}
	return false;
}

// ***************************************************************************
	
bool CIXml::getContentString (std::string &result, xmlNodePtr node)
{
	const char *valueText = (const char*)xmlNodeGetContent (node);
	if (valueText)
	{
		result = valueText;

		// Delete the value
		xmlFree ((void*)valueText);
	
		// Found
		return true;
	}
	return false;
}

// ***************************************************************************
	
} // NLMISC

#endif // NL_DONT_USE_EXTERNAL_CODE
