/** \file i_xml.h
 * Input xml stream
 *
 * $Id: i_xml.h,v 1.6 2003/03/10 18:13:33 corvazier Exp $
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

#ifndef NL_I_XML_H
#define NL_I_XML_H

//#define NL_DONT_USE_EXTERNAL_CODE
#undef NL_DONT_USE_EXTERNAL_CODE

#ifndef NL_DONT_USE_EXTERNAL_CODE

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"

// Include from libxml2
#include <libxml/parser.h>

namespace NLMISC {


struct EXmlParsingError : public EStream
{
	EXmlParsingError ( const std::string& str ) : EStream( str ) {};
};

/**
 * Input xml stream
 *
 * This class is an xml formated input stream.
 *
 * This stream use an internal stream to input source xml code.
 * Use setStream to initialise it.
 \code
	// Check exceptions
	try
	{
		// File stream
		CIFile file;

		// open the file
		file.open ("input.xml");

		// XMl stream
		CIXml input;

		// Init, read all the input file...
		if (input.init (file))
		{
			// Serial the class
			myClass.serial (input);
		}

		// Close the file
		file.close ();
	}
	catch (Exception &e)
	{
	}
 \encode
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CIXml : public IStream
{
	friend void xmlGenericErrorFuncRead (void *ctx, const char *msg, ...);
public:

	/** Default ctor
	  */
	CIXml ();

	/** Dtor. Call release().
	  */
	virtual ~CIXml ();

	/** Stream initialisation. The stream must be an input stream.
	  * init() will load the XML tree. So init() can raise read error exceptions.
	  *
	  * \param stream is the stream the class will use to input xml code.
	  * this pointer is not held by the class. This stream must support end seek functions (as files).
	  * \return true if init success, false if stream is not an input stream.
	  */
	bool			init (IStream &stream);

	/** Release the resources used by the stream.
	  */
	void			release ();

	/** Get the root node pointer 
	  */
	xmlNodePtr		getRootNode () const;

	/** Get the first child node pointer named childName. NULL if no node named childName. 
	  */
	static xmlNodePtr getFirstChildNode (xmlNodePtr parent, const char *childName);

	/** Get the next child node pointer name childName, brother of previous. NULL if no node named childName. 
	  */
	static xmlNodePtr getNextChildNode (xmlNodePtr last, const char *childName);

	/** Get the first child node pointer of type. NULL if no node of type. 
	  */
	static xmlNodePtr getFirstChildNode (xmlNodePtr parent, xmlElementType type);

	/** Get the next child node pointer of type. NULL if no node of type. 
	  */
	static xmlNodePtr getNextChildNode (xmlNodePtr last, xmlElementType type);

	/** Count number of sub node named with a given name for a given node.
	  */
	static uint		countChildren (xmlNodePtr node, const char *childName);

	/** Count number of sub node of type for a given node.
	  */
	static uint		countChildren (xmlNodePtr node, xmlElementType type);
	
	/** 
	  * Read a property string
	  *
	  * Returns true and the result if the property has been found, else false.
	  */
	static bool		getPropertyString (std::string &result, xmlNodePtr node, const char *property);
	
	/** 
	  * Read the content of the node as a string
	  *
	  * Returns true and the result if some text has been found, else false.
	  */
	static bool		getContentString (std::string &result, xmlNodePtr node);

private:

	/// From IStream
	virtual void	serial(uint8 &b);
	virtual void	serial(sint8 &b);
	virtual void	serial(uint16 &b);
	virtual void	serial(sint16 &b);
	virtual void	serial(uint32 &b);
	virtual void	serial(sint32 &b);
	virtual void	serial(uint64 &b);
	virtual void	serial(sint64 &b);
	virtual void	serial(float &b);
	virtual void	serial(double &b);
	virtual void	serial(bool &b);
#ifndef NL_OS_CYGWIN
	virtual void	serial(char &b);
#endif
	virtual void	serial(std::string &b);
	virtual void	serial(ucstring &b);
	virtual void	serialBuffer(uint8 *buf, uint len);
	virtual void	serialBit(bool &bit);

	virtual bool	xmlPushBeginInternal (const char *nodeName);
	virtual bool	xmlPushEndInternal ();
	virtual bool	xmlPopInternal ();
	virtual bool	xmlSetAttribInternal (const char *attribName);
	virtual bool	xmlBreakLineInternal ();
	virtual bool	xmlCommentInternal (const char *comment);

	// Internal functions
	void			serialSeparatedBufferIn ( std::string &value, bool checkSeparator = true );
	inline void		flushContentString ();

	// Push has began
	bool			_PushBegin;

	// Attribute present
	bool			_AttribPresent;

	// Attribute name
	std::string		_AttribName;

	// Current libxml node
	xmlNodePtr		_CurrentNode;

	// Current libxml header node opened
	xmlNodePtr		_CurrentElement;

	// Parser pointer
	xmlParserCtxtPtr	_Parser;

	// Current node text
	std::string		_ContentString;

	// Current index in the node string
	uint			_ContentStringIndex;

	// Error message
	std::string		_ErrorString;
};


} // NLMISC

#endif // NL_DONT_USE_EXTERNAL_CODE

#endif // NL_I_XML_H

/* End of o_xml.h */
