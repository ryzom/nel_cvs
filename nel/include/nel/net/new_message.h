/** \file new_message.h
 * CNewMessage class
 *
 * $Id: new_message.h,v 1.1 2001/03/02 09:54:30 lecroart Exp $
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

#ifndef NL_NEW_MESSAGE_H
#define NL_NEW_MESSAGE_H

#include "nel/misc/mem_stream.h"
#include <vector>

namespace NLNET
{


/**
 * Message memory stream for network. Can be serialized to/from (see SerialBuffer()). Can be sent or received
 * over a network, using a CSocket or preferably a CMsgSocket object.
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
class CNewMessage : public NLMISC::CMemStream
{
public:

	CNewMessage (const NLMISC::CStringIdArray &sida, const std::string &name = "", bool inputStream = false, uint32 defaultCapacity = 0) :
		CMemStream (inputStream, defaultCapacity), _TypeSet (false), _SIDA (&sida)
	{
		if (!name.empty())
			setType (name);
	}
	
	/// Copy constructor
	CNewMessage (const CNewMessage &other)
	{
		operator= (other);
	}

	/// Assignment operator
	CNewMessage &operator= (const CNewMessage &other)
	{
		CMemStream::operator= (other);
		_TypeSet = other._TypeSet;
		_SIDA= other._SIDA;
		return *this;
	}
	
	/// Sets the message type as a number (in range 0..32767)
	void setType (const TStringId id)
	{
		// check if we already do a setType ()
		nlassert (!typeSet);
		// check if they don't already serial some stuffs
		nlassert (length () == 0);
		// check if setType () is in input mode
		nlassert (!isReading ());
		// don't accept negative value
		nlassert (msgtype >= 0);

		serial (id);

		_TypeSet = true;
	}

	/// Sets the message type as a string
	void setType (const std::string &name)
	{
		// check if we already do a setType ()
		nlassert (!typeSet);
		// check if they don't already serial some stuffs
		nlassert (length () == 0);
		// check if setType () is in input mode
		nlassert (!isReading ());
		// don't accept empty string
		nlassert (!msgname.empty ());

		// if we can send the id instead of the string, just do it (c)nike!
		NLMISC::CStringIdArray::TStringId id = _SIDA->getId (name);
		if (id == -1)
			serial (name);
		else
			serial (id);

		_TypeSet = true;
	}
	
	void clear ()
	{
		CMemStream::clear ();
		_TypeSet = false;
	}

private:
	bool _TypeSet;
	const NLMISC::CStringIdArray *_SIDA;
	
	
	
	
	
	
///////////////////////////////////////
/*
	
	/// Constructor
	CNewMessage( std::string name="", bool inputStream=false, uint32 defaultcapacity=0 );

	/// Copy constructor
	CNewMessage( const CNewMessage& other );

	/// Assignment operator
	CNewMessage&		operator=( const CNewMessage& other );

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
		return CNewMessage::_MaxLength;
	}

	/// Returns the maximum message header size
	static uint32	maxHeaderLength()
	{
		return CNewMessage::_MaxHeaderLength;
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

#endif // NL_NEW_MESSAGE_H

/* End of new_message.h */
