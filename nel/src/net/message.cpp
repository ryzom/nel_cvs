/* message.cpp
 *
 * Copyright, 2000 Nevrax Ltd.
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

/*
 * $Id: message.cpp,v 1.6 2000/09/25 11:14:23 cado Exp $
 *
 * Implementation of CMessage
 */

#include "nel/net/message.h"

namespace NLNET
{


uint32	CMessage::_MaxLength = 65536;
uint32	CMessage::_MaxHeaderLength = 64;


/*
 * Initialization constructor
 */
CMessage::CMessage( bool inputStream, uint32 defaultcapacity ) :
	NLMISC::IStream( inputStream, true ),
	_MsgType( 0 )
{
	_Buffer.reserve( defaultcapacity );
	_BufPos = _Buffer.begin();
}


/*
 * Copy constructor
 */
CMessage::CMessage( const CMessage& other ) :
	NLMISC::IStream( other.isReading(), true )
{
	operator=( other );
}


/*
 * Assignment operator
 */
CMessage& CMessage::operator=( const CMessage& other )
{
	_Buffer = other._Buffer;
	_BufPos = _Buffer.begin() + other.lengthS();
	_MsgType = other._MsgType;
	_MsgName = other._MsgName;
	return *this;
}


/*
 * serial (inherited from IStream)
 */
void CMessage::serialBuffer(uint8 *buf, uint len) throw(EStreamOverflow)
{
	if ( isReading() )
	{
		// Check that we don't read more than there is to read
		if ( lengthS()+len > lengthR() )
		{
			_asm int 3
			//throw EStreamOverflow();
		}
		// Serialize in
		memcpy( buf, &(*_BufPos), len );
		_BufPos += len;
	}
	else
	{
		// Serialize out
		_Buffer.resize( _Buffer.size() + len );
		_BufPos = _Buffer.end() - len;
		memcpy( &(*_BufPos), buf, len );
		_BufPos = _Buffer.end();
	}
}



/*
 * serialBit (inherited from IStream)
 */
void CMessage::serialBit(bool &bit) throw(EStreamOverflow)
{
	uint len = sizeof(uint8);
	uint8 thebuf;

	if ( isReading() )
	{
		// Check that we don't read more than there is to read
		if ( lengthS()+len > lengthR() )
		{
			throw EStreamOverflow();
		}
		// Serialize in
		memcpy( &thebuf, &(*_BufPos), len );
		_BufPos += len;
		bit = (thebuf!=0);
	}
	else
	{
		thebuf = (uint8)bit;
		// Serialize out
		_Buffer.resize( _Buffer.size() + len );
		_BufPos = _Buffer.end() - len;
		memcpy( &(*_BufPos), &thebuf, len );
		_BufPos = _Buffer.end();
	}
}


/*
 * Clears the message
 */
void CMessage::clear()
{
	_Buffer.clear();
	_BufPos = _Buffer.begin();
}


/*
 * Returns message type code or length of message name
 */
uint16 CMessage::encodedMsgType() const
{
	if ( msgName() != "" )
	{
		return msgName().length() | 0x8000; // bit15 is 1 when msgtype is msgnamelen
	}
	else
	{
		return msgType();
	}
}


/*
 * Returns an output message with header encoded in the payload buffer
 */
CMessage CMessage::encode() const
{
	CMessage alldata( false, length()+CMessage::maxHeaderLength() );

	// 1. Write message type
	sint16 msgtype = encodedMsgType();
	alldata.serial( msgtype );

	// 2. Write message name (optional)
	if ( msgName().length() != 0 )
	{
		alldata.serial( msgName() );
	}

	// 3. Write message size
	uint32 msgsize = length();
	alldata.serial( msgsize );

	// 4. Write message payload
	alldata.serialBuffer( const_cast<uint8*>(buffer()), length() );

	return alldata;
}


/* Sets the message using an encoded input message.
 * @param alldata An input message in which the header is in the payload buffer
 */
void CMessage::decode( CMessage& alldata )
{
	// 1. Read message type
	sint16 msgtype;
	alldata.serial( msgtype );

	// 2. Read message name (optional)
	uint16 msgnamelen = 0;
	char *msgname = NULL;
	if ( CMessage::decodeLenInMsgType( msgtype, &msgnamelen ) )
	{
		msgname = new char[msgnamelen+1];
		alldata.serialBuffer( (uint8*)msgname, msgnamelen );
		msgname[msgnamelen] = '\0';
	}
	setHeader( msgtype, std::string( msgname!=NULL ? msgname : "" ) );
	if ( msgname != NULL )
	{
		delete [] msgname;
	}

	// 3. Read message payload size
	uint32 msgsize;
	alldata.serial( msgsize );

	// 4. Read payload buffer
	alldata.serialBuffer( bufferToFill( msgsize ), msgsize );
}


/*
 * Returns true if msgtype contains the length of a message name, and, if so, puts it into msgnamelen
 */
bool CMessage::decodeLenInMsgType( uint16 msgtype, uint16 *msgnamelen )
{
	if ( msgtype < 0 )
	{
		*msgnamelen = msgtype & 0x7FFF; // msgtype when bit 15 is 1 is length of msgname
		return true;
	}
	return false;
}


/*
 * Fills the message buffer
 */
void CMessage::fill( const uint8 *srcbuf, uint32 len )
{
	_Buffer.resize( len );
	_BufPos = _Buffer.begin();
	memcpy( &(*_BufPos), srcbuf, len );
}


/*
 * Sets the message header values
 */
void CMessage::setHeader( sint16 msgtype, const std::string& msgname )
{
	_MsgType = msgtype;
	_MsgName = msgname;
}


/** EXPERIMENTAL: Returns a pointer to the message buffer for filling by an external function (use at your own risk,
 * you MUST fill the number of bytes you specify in "msgsize").
 * This method prevents from doing one useless buffer copy, using fill().
 */
uint8 *CMessage::bufferToFill( uint32 msgsize )
{
	// Same as fill() but the memcpy is done by an external function
	_Buffer.resize( msgsize );
	_BufPos = _Buffer.begin();
	return &(*_BufPos);
}


}