/** \file message.h
 * From memory serialization implementation of IStream with typed system (look at stream.h)
 *
 * $Id: message.h,v 1.29 2001/08/27 10:34:31 lecroart Exp $
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


/* Choose between plain text messages and binary messages
 * using the macro MESSAGES_PLAIN_TEXT
 * Warning: if you choose plain_text, all the interdependant programs
 * of your system must have the same encoding.
 */
#define MESSAGES_PLAIN_TEXT


#include <sstream>

#ifdef MESSAGES_PLAIN_TEXT
#include "nel/misc/string_stream.h"
#else
#include "nel/misc/mem_stream.h"
#endif

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
#ifdef MESSAGES_PLAIN_TEXT
class CMessage : public NLMISC::CStringStream
#else
class CMessage : public NLMISC::CMemStream
#endif
{
public:

	CMessage (NLMISC::CStringIdArray &sida, const std::string &name = "", bool inputStream = false, uint32 defaultCapacity = 0) :
#ifdef MESSAGES_PLAIN_TEXT
	    CStringStream( inputStream, defaultCapacity ),
#else
		CMemStream (inputStream, defaultCapacity),
#endif
		_TypeSet (false), _SIDA (&sida), _HeaderSize(0xFFFFFFFF)
	{
		if (!name.empty())
			setType (name);
	}

	CMessage (const std::string &name = "", bool inputStream = false, uint32 defaultCapacity = 0) :
#ifdef MESSAGES_PLAIN_TEXT
	    CStringStream( inputStream, defaultCapacity ),
#else
		CMemStream (inputStream, defaultCapacity),
#endif
		_TypeSet (false), _SIDA (NULL), _HeaderSize(0xFFFFFFFF)
	{
		if (!name.empty())
			setType (name);
	}

#ifdef MESSAGES_PLAIN_TEXT
	CMessage( NLMISC::CStringStream &memstr) :
#else
	CMessage (NLMISC::CMemStream &memstr) :
#endif
	_HeaderSize(0xFFFFFFFF)
	{
		fill (memstr.buffer (), memstr.length ());
		uint8 LongFormat;
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
	CMessage (const CMessage &other)
	{
		operator= (other);
	}

	/// Assignment operator
	CMessage &operator= (const CMessage &other)
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
	void setType (NLMISC::CStringIdArray::TStringId id)
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

			uint8 LongFormat = false;

			// debug features, we number all packet to be sure that they are all sent and received
			// \todo remove this debug feature when ok
			// this value will be fill after in the callback function
			uint32 zeroValue = 1234;
			serial (zeroValue);

			serial (LongFormat);
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
	void setType (const std::string &name)
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

			// debug features, we number all packet to be sure that they are all sent and received
			// \todo remove this debug feature when ok
			// this value will be fill after in the callback function
			uint32 zeroValue = 1234;
			serial (zeroValue);

			if (id == -1)
			{
				uint8 LongFormat = true;
				serial (LongFormat);
				serial ((std::string&)name);
			}
			else
			{
				uint8 LongFormat = false;
				serial (LongFormat);
				serial (id);

				_Id = id;
				TypeHasAnId = true;
			}
			_HeaderSize = getPos ();
		}

		_TypeSet = true;
	}

	/// Returns the size, in byte of the header that contains the type name of the message or the type number
	uint32 getHeaderSize ()
	{
		nlassert (!isReading ());
		nlassert (_HeaderSize != 0xFFFFFFFF);
		return _HeaderSize;
	}
	
	// The message was filled with an CMemStream, Now, we'll get the message type on this buffer
	void readType ()
	{
		nlassert (isReading ());

		// debug features, we number all packet to be sure that they are all sent and received
		// \todo remove this debug feature when ok

		// we remove the message from the message
		seek (5, begin);

//		uint32 zeroValue;
//		serial (zeroValue);

		uint8 LongFormat;
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

	// Returns true if the message type was already set
	bool typeIsSet () const
	{
		return _TypeSet;
	}

	// Clear the message. With this function, you can reuse a message to create another message
	void clear ()
	{
		CMemStream::clear ();
		_TypeSet = false;
	}

	/// Returns the type name in string if available. Be sure that the message have the name of the message type
	std::string getName () const { nlassert (_TypeSet && TypeHasAName); return _Name; }
	
	/// Returns the type id of this message is available.
	NLMISC::CStringIdArray::TStringId getId () const { nlassert (_TypeSet && TypeHasAnId); return _Id; }

	/** Returns a readable string to display it to the screen. It's only for debugging purpose!
	 * Don't use it for anything else than to debugging, the string format could change in the futur
	 */
	std::string toString () const
	{
		nlassert (_TypeSet);
		std::stringstream s;
		if (TypeHasAName && TypeHasAnId) s << "('" << _Name << "'," << _Id << ")";
		else if (TypeHasAName) s << "('" << _Name << "'," << _SIDA->getId (_Name, true) << ")";
		else if (TypeHasAnId) s << "('" << _SIDA->getString (_Id) << "'," << _Id << "')";
		return s.str();
	}


	bool TypeHasAnId;
	bool TypeHasAName;

private:
	bool _TypeSet;
	NLMISC::CStringIdArray *_SIDA;
	
	std::string	_Name;
	NLMISC::CStringIdArray::TStringId _Id;
	
	// Size of the header (that contains the name type or number type)
	uint32 _HeaderSize;

///////////////////////////////////////
/*
	
	/// Constructor
	CMessage( std::string name="", bool inputStream=false, uint32 defaultcapacity=0 );

	/// Copy constructor
	CMessage( const CMessage& other );

	/// Assignment operator
	CMessage&		operator=( const CMessage& other );

	/// Returns if the message type is a number (and not a string)
	bool			typeIsNumber() const
	{
		return _TypeIsNumber;
	}

	/// Returns message type code. Valid if typeIsNumber()
	uint16			typeAsNumber() const
	{
		return _MsgType;
	}

	/// Returns message name. Valid if !typeIsNumber()
	const std::string&	typeAsString() const
	{
		return _MsgName;
	}

	/// Sets the message type as a number (in range 0..32767)
	void			setType( const TTypeNum msgtype )
	{
		_MsgType = msgtype;
		_TypeIsNumber = true;
	}

	/// Sets the message type as a string
	void			setType( const std::string& msgname )
	{
		_MsgName = msgname;
		_TypeIsNumber = false;
	}

	/// Returns the maximum message total size (header + payload)
	static uint32	maxLength()
	{
		return CMessage::_MaxLength;
	}

	/// Returns the maximum message header size
	static uint32	maxHeaderLength()
	{
		return CMessage::_MaxHeaderLength;
	}

	/// Returns the maximum message payload size
	static uint32	maxPayloadLength()
	{
		return maxLength() - maxHeaderLength();
	}

	/// Returns the maximum message name length
	static uint32	maxNameLength()
	{
		return maxHeaderLength() - sizeof(sint16) - sizeof(uint32);
	}

	/// Clears the message
	void			clear();

private:

	static uint32		_MaxLength;
	static uint32		_MaxHeaderLength;

	TTypeNum			_MsgType;
	std::string			_MsgName;
	bool				_TypeIsNumber;
*/
};

}

#endif // NL_MESSAGE_H

/* End of message.h */
