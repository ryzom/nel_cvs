/** \file message.cpp
 * CMessage class
 *
 * $Id: message.cpp,v 1.26 2004/05/07 12:56:22 cado Exp $
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

const char *LockedSubMessageError = "a sub message is forbidden";

#define FormatLong 1
#define FormatShort 0


/*
 * Constructor by name
 */
CMessage::CMessage (const std::string &name, bool inputStream, TStreamFormat streamformat, uint32 defaultCapacity) :
	NLMISC::CMemStream (inputStream, false, defaultCapacity),
	_SubMessagePosR(0), _LengthR(0), _HeaderSize(0xFFFFFFFF), _TypeSet (false)
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


/*
 * Constructor with copy from CMemStream
 */
CMessage::CMessage (NLMISC::CMemStream &memstr) :
	NLMISC::CMemStream( memstr ),
	_SubMessagePosR(0), _LengthR(0), _HeaderSize(0xFFFFFFFF), _TypeSet (false)
{
	sint32 pos = getPos();
	bool reading = isReading();
	if ( reading ) // force input mode to read the type
		readType(); // sets _TypeSet, _HeaderSize and _LengthR
	else
		invert(); // calls readType()
	if ( ! reading )
		invert(); // set ouput mode back if necessary
	seek( pos, begin ); // sets the same position as the one in the memstream
}


/*
 * Copy constructor
 */
CMessage::CMessage (const CMessage &other)
{
	operator= (other);
}

/*
 * Assignment operator
 */
CMessage &CMessage::operator= (const CMessage &other)
{
	nlassertex( (!other.isReading()) || (!other.hasLockedSubMessage()), ("Storing %s", LockedSubMessageError) );
	nlassertex( (!isReading()) || (!hasLockedSubMessage()), ("Assigning %s", LockedSubMessageError) );

	CMemStream::operator= (other);
	_TypeSet = other._TypeSet;
	_Name = other._Name;
	_HeaderSize = other._HeaderSize;
	_SubMessagePosR = other._SubMessagePosR;
	_LengthR = other._LengthR;
	return *this;

}


/**
 * Similar to operator=, but makes the current message contain *only* the locked sub message in msgin
 * or the whole msgin if it is not locked
 *
 * Preconditions:
 * - msgin is an input message (isReading())
 * - The current message is blank (new or reset with clear())
 *
 * Postconditions:
 * - If msgin has been locked using lockSubMessage(), the current message contains only the locked
 *   sub message in msgin, otherwise the current message is exactly msgin
 * - The current message is an input message, it is not locked
 */
void CMessage::assignFromSubMessage( const CMessage& msgin )
{
	nlassert( msgin.isReading() );
	nlassert( ! _TypeSet );
	if ( ! isReading() )
		invert();

	if ( msgin.hasLockedSubMessage() )
	{
		fill( msgin.buffer() + msgin._SubMessagePosR, msgin._LengthR-msgin._SubMessagePosR );
		readType();
	}
	else
	{
		operator=( msgin );
	}
}


/*
 * Sets the message type as a string and put it in the buffer if we are in writing mode
 */
void CMessage::setType (const std::string &name)
{
	// check if we already do a setType ()
	nlassert (!_TypeSet);
	// don't accept empty string
	nlassert (!name.empty ());

	_Name = name;

	if (!isReading ())
	{
		// check if they don't already serial some stuffs
		nlassert (length () == 0);

		// if we can send the id instead of the string, "just do it" (c)nike!
		//NLMISC::CStringIdArray::TStringId id = _SIDA->getId (name);

		// Force binary mode for header
		bool msgmode = _StringMode;
		_StringMode = false;

		// debug features, we number all packet to be sure that they are all sent and received
		// \todo remove this debug feature when ok
		// this value will be fill after in the callback function
		uint32 zeroValue = 123;
		serial (zeroValue);

		uint8 format = FormatLong | (msgmode << 1);
		//nldebug( "OUT format = %hu", (uint16)format );
		serial (format);

		// End of binary header
		_StringMode = msgmode;

		serial ((std::string&)name);

		_HeaderSize = getPos ();
	}

	_TypeSet = true;
}


/*
 * Warning: MUST be of the same size than previous name!
 * Output message only.
 */
void CMessage::changeType (const std::string &name)
{
	sint32 prevPos = getPos();
	seek( sizeof(uint32)+sizeof(uint8), begin );
	serial ((std::string&)name);
	seek( prevPos, begin );
}


/*
 * Returns the size, in byte of the header that contains the type name of the message or the type number
 */
uint32 CMessage::getHeaderSize ()
{
	nlassert (_HeaderSize != 0xFFFFFFFF);
	return _HeaderSize;
}


/*
 * The message was filled with an CMemStream, Now, we'll get the message type on this buffer
 */
void CMessage::readType ()
{
	nlassert (isReading ());

	// debug features, we number all packet to be sure that they are all sent and received
	// \todo remove this debug feature when ok

	// we remove the message from the message
	resetSubMessageInternals();
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
	std::string name;
	serial (name);
	setType (name);
	_HeaderSize = getPos();
}


/*
 * Get the message name (input message only) and advance the current pos
 */
std::string CMessage::readTypeAtCurrentPos()
{
	nlassert( isReading() );

	const uint HeaderSize = 4;
	seek( getPos() + HeaderSize, begin );

	bool sm = _StringMode;
	_StringMode = false;

	uint8 format;
	serial( format );
	bool LongFormat = (format & 1);
	_StringMode = (format >> 1) & 1;
	if ( LongFormat )
	{
		std::string name;
		serial( name );
		_StringMode = sm;
		return name;
	}
	else
		nlerror( "Id not supported" );

	_StringMode = sm;
	return "";
}


// Returns true if the message type was already set
bool CMessage::typeIsSet () const
{
	return _TypeSet;
}

// Clear the message. With this function, you can reuse a message to create another message
void CMessage::clear ()
{
	nlassertex( (!isReading()) || (!hasLockedSubMessage()), ("Clearing %s", LockedSubMessageError) );

	CMemStream::clear ();
	_TypeSet = false;
	_SubMessagePosR = 0;
	_LengthR = 0;
}

/*
 * Returns the type name in string if available. Be sure that the message have the name of the message type
 */
std::string CMessage::getName () const
{
	if ( hasLockedSubMessage() )
	{
		CMessage& notconstMsg = const_cast<CMessage&>(*this);
		sint32 savedPos = notconstMsg.getPos();
		notconstMsg.seek( _SubMessagePosR, begin ); // not really const... but removing the const from getName() would need too many const changes elsewhere
		std::string name = notconstMsg.readTypeAtCurrentPos();
		notconstMsg.seek( savedPos, begin );
		return name;
	}
	else
	{
		nlassert (_TypeSet);
		return _Name;
	}
}

/* Returns a readable string to display it to the screen. It's only for debugging purpose!
 * Don't use it for anything else than to debugging, the string format could change in the futur
 */
std::string CMessage::toString () const
{
	nlassert (_TypeSet);
	return "('" + _Name + ")";
}


/*
 * Return an input stream containing the stream beginning in the message at the specified pos
 */
NLMISC::CMemStream	CMessage::extractStreamFromPos( sint32 pos )
{
	NLMISC::CMemStream msg( true );
	sint32 len = length() - pos;
	memcpy( msg.bufferToFill( len ), buffer() + pos, len );
	return msg;
}


/*
 * Encapsulate/decapsulate another message inside the current message
 */
void	CMessage::serialMessage( CMessage& msg )
{
	if ( isReading() )
	{
		// Init 'msg' with the contents serialised from 'this'
		uint32 len;
		serial( len );
		if ( ! msg.isReading() )
			msg.invert();
		serialBuffer( msg.bufferToFill( len ), len );
		msg.readType();
		msg.invert();
		msg.seek( 0, CMemStream::end );
	}
	else
	{
		// Store into 'this' the contents of 'msg'
		uint32 len = msg.length();
		serial( len );
		serialBuffer( const_cast<uint8*>(msg.buffer()), msg.length() );
	}
}

}
