/** \file message.h
 * CMessage class
 *
 * $Id: message.h,v 1.17 2001/01/31 15:50:45 cado Exp $
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

#include "nel/misc/mem_stream.h"
#include <vector>

namespace NLNET
{

/// Type of message as a number
typedef sint16 TTypeNum;


/**
 * Message memory stream for network. Can be serialized to/from (see SerialBuffer()). Can be sent or received
 * over a network, using a CSocket or preferably a CMsgSocket object.
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CMessage : public NLMISC::CMemStream
{
public:

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

};

}

#endif // NL_MESSAGE_H

/* End of message.h */
