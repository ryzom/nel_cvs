/** \file message.h
 * From memory serialization implementation of IStream with typed system (look at stream.h)
 *
 * $Id: message.h,v 1.37 2003/12/29 13:32:53 lecroart Exp $
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

#ifndef NL_MESSAGE_H
#define NL_MESSAGE_H

#include "nel/misc/types_nl.h"

//#include <sstream>

#include "nel/misc/mem_stream.h"

#include "nel/misc/string_id_array.h"

#include <vector>

namespace NLNET
{

extern const char *LockedSubMessageError;


/**
 * Message memory stream for network. Can be serialized to/from (see SerialBuffer()). Can be sent or received
 * over a network, using the NeL network engine.
 * If MESSAGES_PLAIN_TEXT is defined, the messages will be serialized to/from plain text (human-readable),
 * instead of binary.
 * Warning: if you don't give a "sida", the message type will not be associated with id so, it'll not be optimized
 * Warning: THE ASSOCIATION Id<->String SYSTEM FOR THE MESSAGE TYPE IS DISABLED BECAUSE 2 DIFFERENT CLIENTS COULD HAVE
 *          SAME STRING ASSOC AND DIFFERENT ID SO WE HAVE TO PUT ONE SIDA TABLE PER CONNECTION AND NOT PER NET_BASE
 *
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
class CMessage : public NLMISC::CMemStream
{
public:

	enum TStreamFormat { UseDefault, Binary, String };

	CMessage (NLMISC::CStringIdArray &sida, const std::string &name = "", bool inputStream = false, TStreamFormat streamformat = UseDefault, uint32 defaultCapacity = 1000);

	CMessage (const std::string &name = "", bool inputStream = false, TStreamFormat streamformat = UseDefault, uint32 defaultCapacity = 1000);

	CMessage (NLMISC::CMemStream &memstr);

	/// Copy constructor
	CMessage (const CMessage &other);

	/// Assignment operator
	CMessage &operator= (const CMessage &other);
	
	/// Sets the message type as a number (in range 0..32767) and put it in the buffer if we are in writing mode
	void setType (NLMISC::CStringIdArray::TStringId id);

	/// Sets the message type as a string and put it in the buffer if we are in writing mode
	void setType (const std::string &name);

	void changeType (const std::string &name);

	/// Returns the size, in byte of the header that contains the type name of the message or the type number
	uint32 getHeaderSize ();
	
	/** The message was filled with an CMemStream, Now, we'll get the message type on this buffer.
	 * This method updates _LengthR with the actual size of the buffer (it calls resetLengthR()).
	 */
	void readType ();

	/// Get the message name (input message only) and advance the current pos
	std::string readTypeAtCurrentPos();

	// Returns true if the message type was already set
	bool typeIsSet () const;

	/**
	 * Returns the length (size) of the message, in bytes.
	 * If isReading(), it is the number of bytes that can be read,
	 * otherwise it is the number of bytes that have been written.
	 * Overloaded because uses a specific version of lengthR().
	 */
	virtual uint32	length() const
	{
		if ( isReading() )
		{
			return lengthR();
		}
		else
		{
			return lengthS();
		}
	}

	/// Returns the "read" message size (number of bytes to read) (note: see comment about _LengthR)
	uint32			lengthR() const
	{
		return _LengthR;
	}

	/**
	 * Set an input message to look like, from a message callback's scope, as if it began at the current
	 * pos and ended at the current pos + msgSize, and read the header and return the name of the sub message.
	 *
	 * This method provides a way to pass a big message containing a set of sub messages to their message
	 * callback, without copying each sub message to a new message. If you need to perform some actions
	 * that are not allowed with a locked message (see postconditions), use assignFromSubMessage():
	 * the locked sub message in M1 can be copied to a new message M2 with 'M2.assignFromSubMessage( M1 )'.
	 * 
	 * Preconditions:
	 * - The message is an input message (isReading())
	 * - msgEndPos <= length()
	 * - getPos() >= getHeaderSize()
	 *
	 * Postconditions:
	 * - The sub message is ready to be read from
	 * - length() returns the size of the sub message
	 * - getName() return the name of the sub message
	 * - Unless you call unlockSubMessage(), the following actions will assert or raise an exception:
	 *   Serializing more than the sub message size, clear(), operator=() (from/to), invert().
	 */
	std::string			lockSubMessage( uint32 subMsgSize )
	{
		uint32 subMsgBeginPos = getPos();
		uint32 subMsgEndPos = subMsgBeginPos + subMsgSize;
		nlassertex( isReading() && (subMsgEndPos <= lengthR()), ("%s %u %u", isReading()?"R":"W", subMsgEndPos, lengthR()) );
		std::string name = readTypeAtCurrentPos();
		_SubMessagePosR = subMsgBeginPos;
		_LengthR = subMsgEndPos;
		return name;
	}

	/**
	 * Exit from sub message locking, and skip the whole sub message.
	 *
	 * Preconditions:
	 * - The message is an input message (isReading()) and has been locked using lockSubMessage()
	 * - The reading pos is within or at the end of the previous sub message (if any) (see nlassertex)
	 *
	 * Postconditions:
	 * - The current pos is the next byte after the sub message
	 */
	void			unlockSubMessage()
	{
		nlassert( isReading() && hasLockedSubMessage() );
		nlassertex( getPos() <= (sint32)_LengthR, ("The callback for msg %s read more data than there is in the message (pos=%d len=%u)", getName().c_str(), getPos(), _LengthR) );

		uint32 subMsgEndPos = _LengthR;
		resetSubMessageInternals();
		seek( subMsgEndPos, IStream::begin );
	}

	/// Return true if a sub message has been locked
	bool			hasLockedSubMessage() const
	{
		return (_SubMessagePosR != 0);
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
	void			assignFromSubMessage( const CMessage& msgin );

	/**
	 * Transforms the message from input to output or from output to input
	 */
	void invert()
	{
		nlassertex( (!isReading()) || (!hasLockedSubMessage()), ("Inverting %s", LockedSubMessageError) );

		CMemStream::invert();

		if ( isReading() )
		{
			// Write -> Read: skip the header 
			_TypeSet = false;
			if ( _Buffer.size() != 0 )
				readType();
		}
		// For Read -> Write, please use clear()
	}

	// Clear the message. With this function, you can reuse a message to create another message
	void clear ();

	/** Returns the type name in string if available. Be sure that the message have the name of the message type
	 * In a callback driven by message name, getName() does not necessarily return the right name.
	 */
	std::string getName () const;
	
	/// Returns the type id of this message is available.
	NLMISC::CStringIdArray::TStringId getId () const;

	/** Returns a readable string to display it to the screen. It's only for debugging purpose!
	 * Don't use it for anything else than to debugging, the string format could change in the futur
	 */
	std::string toString () const;

	/// Set default stream mode
	static void	setDefaultStringMode( bool stringmode ) { _DefaultStringMode = stringmode; }

	/// Return an input stream containing the stream beginning in the message at the specified pos
	NLMISC::CMemStream	extractStreamFromPos( sint32 pos );

	/// Encapsulate/decapsulate another message inside the current message
	void	serialMessage( CMessage& msg );

	bool TypeHasAnId;
	bool TypeHasAName;

protected:

	/// Utility method
	void		init( const std::string &name, TStreamFormat streamformat );

	/// Utility method
	void		resetSubMessageInternals()
	{
		_SubMessagePosR = 0;
		_LengthR = _Buffer.size();
	}

private:
	NLMISC::CStringIdArray				*_SIDA;
	
	std::string							_Name;
	
	// When sub message lock mode is enabled, beginning position of sub message to read (before header)
	uint32								_SubMessagePosR;

	// Length (can be smaller than _Buffer.size() if limitLength() is used) (updated in reading mode only)
	uint32								_LengthR;

	// Size of the header (that contains the name type or number type)
	uint32								_HeaderSize;

	NLMISC::CStringIdArray::TStringId	_Id;

	bool								_TypeSet;

	// Default stream format
	static bool							_DefaultStringMode;
};

}

#endif // NL_MESSAGE_H

/* End of message.h */
