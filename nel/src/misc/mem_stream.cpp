/** \file mem_stream.cpp
 * CMemStream class
 *
 * $Id: mem_stream.cpp,v 1.1 2000/12/05 10:03:57 lecroart Exp $
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

#include "nel/misc/mem_stream.h"

namespace NLMISC
{


uint32	CMemStream::_MaxLength = 65536;
uint32	CMemStream::_MaxHeaderLength = 64;


/*
 * Initialization constructor
 */
CMemStream::CMemStream( std::string name, bool inputStream, uint32 defaultcapacity ) :
	NLMISC::IStream( inputStream, true ),
	_MsgType( 0 )
{
	_Buffer.reserve( defaultcapacity );
	_BufPos = _Buffer.begin();

	if ( name != "" )
	{
		setType( name );
	}
}


/*
 * Copy constructor
 */
CMemStream::CMemStream( const CMemStream& other ) :
	NLMISC::IStream( other.isReading(), true )
{
	operator=( other );
}


/*
 * Assignment operator
 */
CMemStream& CMemStream::operator=( const CMemStream& other )
{
	_Buffer = other._Buffer;
	_BufPos = _Buffer.begin() + other.lengthS();
	_MsgType = other._MsgType;
	_MsgName = other._MsgName;
	_TypeIsNumber = other._TypeIsNumber;
	return *this;
}


/*
 * serial (inherited from IStream)
 */
void CMemStream::serialBuffer(uint8 *buf, uint len) throw(EStreamOverflow)
{
	if ( isReading() )
	{
		// Check that we don't read more than there is to read
		if ( lengthS()+len > lengthR() )
		{
			//_asm int 3
			throw EStreamOverflow();
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
void CMemStream::serialBit(bool &bit) throw(EStreamOverflow)
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
void CMemStream::clear()
{
	_Buffer.clear();
	_BufPos = _Buffer.begin();
}



/*
 * Fills the message buffer
 */
void CMemStream::fill( const uint8 *srcbuf, uint32 len )
{
	if (len == 0) return;

	_Buffer.resize( len );
	_BufPos = _Buffer.begin();
	memcpy( &(*_BufPos), srcbuf, len );
}


/* EXPERIMENTAL: Returns a pointer to the message buffer for filling by an external function (use at your own risk,
 * you MUST fill the number of bytes you specify in "msgsize").
 * This method prevents from doing one useless buffer copy, using fill().
 */
uint8 *CMemStream::bufferToFill( uint32 msgsize )
{
	if (msgsize == 0) return NULL;

	// Same as fill() but the memcpy is done by an external function
	_Buffer.resize( msgsize );
	_BufPos = _Buffer.begin();
	return &(*_BufPos);
}


}
