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
 * $Id: message.cpp,v 1.4 2000/09/19 09:14:03 cado Exp $
 *
 * Implementation of CMessage
 */

#include "nel/net/message.h"
#include <algorithm>


namespace NLNET
{

	
/*
 * Initialization constructor
 */
CMessage::CMessage( bool inputStream, uint32 defaultcapacity )
: NLMISC::IStream( inputStream, true ),
  _MsgType( 0 )
{
	_Buffer.reserve( std::max(defaultcapacity,(uint32)32) );
	_BufPos = _Buffer.begin();
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