/** \file message.cpp
 * CMessage class
 *
 * $Id: message.cpp,v 1.21 2002/07/10 17:08:24 lecroart Exp $
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

#include "stdnet.h"

#include "nel/net/message.h"

/*#ifdef MESSAGES_PLAIN_TEXT
#pragma message( "CMessage: compiling messages as plain text" )
#else
#pragma message( "CMessage: compiling messages as binary" )
#endif*/

namespace NLNET
{

bool CMessage::_DefaultStringMode = false;


#define FormatLong 1
#define FormatShort 0


CMessage::CMessage (NLMISC::CStringIdArray &sida, const std::string &name, bool inputStream, TStreamFormat streamformat, uint32 defaultCapacity) :
	NLMISC::CMemStream (inputStream, false, defaultCapacity),
	_TypeSet (false), _SIDA (&sida), _HeaderSize(0xFFFFFFFF)
{
	init( name, streamformat );
}

CMessage::CMessage (const std::string &name, bool inputStream, TStreamFormat streamformat, uint32 defaultCapacity) :
	NLMISC::CMemStream (inputStream, false, defaultCapacity),
	_TypeSet (false), _SIDA (NULL), _HeaderSize(0xFFFFFFFF)
{
	init( name, streamformat );
}


/*
 * Utility method
 */
void CMessage::init( const std::string &name, TStreamFormat streamformat )
{
	if ( streamformat == UseDefault )
	{
		setStringMode( _DefaultStringMode );
	}
	else
	{
		setStringMode( streamformat == String );
	}

	if (!name.empty())
		setType (name);
}


CMessage::CMessage (NLMISC::CMemStream &memstr) :
	_HeaderSize(0xFFFFFFFF)
{
	fill (memstr.buffer (), memstr.length ());
	uint8 LongFormat=2;
	serial (LongFormat);

	if (LongFormat)
	{
		std::string name;
		serial (name);
		setType (name);
	}
	else
	{
		NLMISC::CStringIdArray::TStringId id;
		serial (id);
		setType (id);
	}
}


/// Copy constructor
CMessage::CMessage (const CMessage &other)
{
	operator= (other);
}

/// Assignment operator
CMessage &CMessage::operator= (const CMessage &other)
{
	CMemStream::operator= (other);
	_TypeSet = other._TypeSet;
	_SIDA = other._SIDA;
	TypeHasAnId = other.TypeHasAnId;
	TypeHasAName = other.TypeHasAName;
	_Name = other._Name;
	_Id = other._Id;
	_HeaderSize = other._HeaderSize;
	return *this;

}

/// Sets the message type as a number (in range 0..32767) and put it in the buffer if we are in writing mode
void CMessage::setType (NLMISC::CStringIdArray::TStringId id)
{
	// PATCH: the id system is not available
	nlstop;

	// check if we already do a setType ()
	nlassert (!_TypeSet);
	// don't accept negative value
	nlassert (id >= 0 && id < pow(2, sizeof (NLMISC::CStringIdArray::TStringId)*8));

	_Id = id;
	TypeHasAnId = true;
	TypeHasAName = false;

	if (!isReading ())
	{
		// check if they don't already serial some stuffs
		nlassert (length () == 0);

		uint8 format = FormatLong | (_StringMode << 1);
		nlinfo( "OUT format = %hu", (uint16)format );

		// Force binary mode for header
		bool msgmode = _StringMode;
		_StringMode = false;

		// debug features, we number all packet to be sure that they are all sent and received
		// \todo remove this debug feature when ok
		// this value will be fill after in the callback function
		uint32 zeroValue = 123;
		serial (zeroValue);

		serial (format);

		// End of binary header
		_StringMode = msgmode;

		serial (id);
		_HeaderSize = getPos ();
	}
	else
	{
		// we set the id, now, we try to set the name if available in the sida
		if (_SIDA != NULL)
		{
			_Name = _SIDA->getString (id);
			TypeHasAName = true;
		}
	}

	_TypeSet = true;
}

/// Sets the message type as a string and put it in the buffer if we are in writing mode
void CMessage::setType (const std::string &name)
{
	// check if we already do a setType ()
	nlassert (!_TypeSet);
	// don't accept empty string
	nlassert (!name.empty ());

	_Name = name;
	TypeHasAnId = false;
	TypeHasAName = true;

	if (!isReading ())
	{
		// check if they don't already serial some stuffs
		nlassert (length () == 0);

		// if we can send the id instead of the string, "just do it" (c)nike!
		//NLMISC::CStringIdArray::TStringId id = _SIDA->getId (name);

		// PATCH: always send in full text
		NLMISC::CStringIdArray::TStringId id = -1;

		// Force binary mode for header
		bool msgmode = _StringMode;
		_StringMode = false;

		// debug features, we number all packet to be sure that they are all sent and received
		// \todo remove this debug feature when ok
		// this value will be fill after in the callback function
		uint32 zeroValue = 123;
		serial (zeroValue);

		if (id == -1)
		{
			uint8 format = FormatLong | (msgmode << 1);
			//nldebug( "OUT format = %hu", (uint16)format );
			serial (format);

			// End of binary header
			_StringMode = msgmode;

			serial ((std::string&)name);
		}
		else
		{
			uint8 format = FormatShort | (msgmode << 1);
			//nldebug( "OUT format = %hu", (uint16)format );
			serial (format);

			// End of binary header
			_StringMode = msgmode;

			serial (id);

			_Id = id;
			TypeHasAnId = true;
		}
		_HeaderSize = getPos ();
	}

	_TypeSet = true;
}

/// Returns the size, in byte of the header that contains the type name of the message or the type number
uint32 CMessage::getHeaderSize ()
{
	nlassert (!isReading ());
	nlassert (_HeaderSize != 0xFFFFFFFF);
	return _HeaderSize;
}

// The message was filled with an CMemStream, Now, we'll get the message type on this buffer
void CMessage::readType ()
{
	nlassert (isReading ());

	// debug features, we number all packet to be sure that they are all sent and received
	// \todo remove this debug feature when ok

	// we remove the message from the message
	const uint HeaderSize = 4;
	seek (HeaderSize, begin);
//		uint32 zeroValue;
//		serial (zeroValue);

	// Force binary mode for header
	_StringMode = false;

	uint8 format;
	serial (format);
	//nldebug( "IN format = %hu", (uint16)format );
	bool LongFormat = (format & 1);

	// Set mode for the following of the buffer
	_StringMode = (format >> 1) & 1;
	if (LongFormat)
	{
		std::string name;
		serial (name);
		setType (name);
	}
	else
	{
		NLMISC::CStringIdArray::TStringId id;
		serial (id);
		setType (id);
	}
}

// Returns true if the message type was already set
bool CMessage::typeIsSet () const
{
	return _TypeSet;
}

// Clear the message. With this function, you can reuse a message to create another message
void CMessage::clear ()
{
	CMemStream::clear ();
	_TypeSet = false;
}

/// Returns the type name in string if available. Be sure that the message have the name of the message type
std::string CMessage::getName () const
{
	nlassert (_TypeSet && TypeHasAName);
	return _Name;
}

/// Returns the type id of this message is available.
NLMISC::CStringIdArray::TStringId CMessage::getId () const
{
	nlassert (_TypeSet && TypeHasAnId);
	return _Id;
}

/** Returns a readable string to display it to the screen. It's only for debugging purpose!
 * Don't use it for anything else than to debugging, the string format could change in the futur
 */
std::string CMessage::toString () const
{
	nlassert (_TypeSet);
	std::stringstream s;
	if (TypeHasAName && TypeHasAnId) s << "('" << _Name << "'," << _Id << ")";
	else if (TypeHasAName) s << "('" << _Name << "'," << _SIDA->getId (_Name, true) << ")";
	else if (TypeHasAnId) s << "('" << _SIDA->getString (_Id) << "'," << _Id << "')";
	return s.str();
}



}
