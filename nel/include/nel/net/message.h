/** \file message.h
 * From memory serialization implementation of IStream with typed system (look at stream.h)
 *
 * $Id: message.h,v 1.34 2002/08/20 11:47:39 cado Exp $
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

#include <sstream>

#include "nel/misc/mem_stream.h"

#include "nel/misc/string_id_array.h"

#include <vector>

namespace NLNET
{


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

	/// Returns the size, in byte of the header that contains the type name of the message or the type number
	uint32 getHeaderSize ();
	
	// The message was filled with an CMemStream, Now, we'll get the message type on this buffer
	void readType ();

	// Returns true if the message type was already set
	bool typeIsSet () const;

	// Transforms the message from input to output or from output to input
	void invert()
	{
		CMemStream::invert();

		if ( isReading() )
		{
			// Write -> Read: skip the header 
			_TypeSet = false;
			readType();
		}
		// For Read -> Write, please use clear()
	}

	// Clear the message. With this function, you can reuse a message to create another message
	void clear ();

	/// Returns the type name in string if available. Be sure that the message have the name of the message type
	std::string getName () const;
	
	/// Returns the type id of this message is available.
	NLMISC::CStringIdArray::TStringId getId () const;

	/** Returns a readable string to display it to the screen. It's only for debugging purpose!
	 * Don't use it for anything else than to debugging, the string format could change in the futur
	 */
	std::string toString () const;

	/// Set default stream mode
	static void	setDefaultStringMode( bool stringmode ) { _DefaultStringMode = stringmode; }

	bool TypeHasAnId;
	bool TypeHasAName;

protected:

	/// Utility method
	void		init( const std::string &name, TStreamFormat streamformat );

private:
	bool								_TypeSet;
	NLMISC::CStringIdArray				*_SIDA;
	
	std::string							_Name;
	NLMISC::CStringIdArray::TStringId	_Id;
	
	// Size of the header (that contains the name type or number type)
	uint32								_HeaderSize;

	// Default stream format
	static bool							_DefaultStringMode;
};

}

#endif // NL_MESSAGE_H

/* End of message.h */
