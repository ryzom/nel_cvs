/* message.cpp
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: message.cpp,v 1.2 2000/09/14 16:40:53 cado Exp $
 *
 * Implementation of CMessage
 */

#include "nel/net/message.h"


namespace NLNET
{

/*
 * Initialization constructor
 */
CMessage::CMessage( uint32 defaultcapacity, bool inputStream )
: NLMISC::IStream( inputStream, true ),
  _BufPos( _Buffer.begin() )
{
	_Buffer.reserve( defaultcapacity );
}


/*
 * serial (inherited from IStream)
 */
void CMessage::serial(uint8 *buf, uint len) throw(EStreamOverflow)
{
	if ( isReading() )
	{
		// Check that we don't read more than there is to read
		if ( lengthS()+len > lengthR() )
		{
			throw EStreamOverflow();
		}
		// Serialize in
		memcpy( buf, _BufPos, len );
		_BufPos += len;
	}
	else
	{
		// Serialize out
		_Buffer.resize( _Buffer.size() + len );
		memcpy( _BufPos, buf, len );
		_BufPos += len;
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
		memcpy( &thebuf, _BufPos, len );
		_BufPos += len;
		bit = (thebuf!=0);
	}
	else
	{
		thebuf = (uint8)bit;
		// Serialize out
		_Buffer.resize( _Buffer.size() + len );
		memcpy( _BufPos, &thebuf, len );
		_BufPos += len;
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
 * Fills the message buffer
 */
void CMessage::fill( const uint8 *srcbuf, uint32 len )
{
	_Buffer.resize( len );
	_BufPos = _Buffer.begin();
	memcpy( _BufPos, srcbuf, len );
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
	return _BufPos;
}

}