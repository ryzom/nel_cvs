/** \file mem_stream.cpp
 * CMemStream class
 *
 * $Id: mem_stream.cpp,v 1.5 2000/12/07 15:18:42 cado Exp $
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


/*
 * Initialization constructor
 */
CMemStream::CMemStream( bool inputStream, uint32 defaultcapacity ) :
	NLMISC::IStream( inputStream, true )
{
	_Buffer.reserve( defaultcapacity );
	_BufPos = _Buffer.begin();
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
 * serialBit (inherited from IStream)
 */
bool CMemStream::seek (sint32 offset, TSeekOrigin origin) throw(EStream)
{
	switch (origin)
	{
	case begin:
		if (offset >= (sint)_Buffer.size())
			return false;
		if (offset < 0)
			return false;
		_BufPos=_Buffer.begin()+offset;
		break;
	case current:
		if (getPos ()+offset >= (sint)_Buffer.size())
			return false;
		if (getPos ()+offset < 0)
			return false;
		_BufPos+=offset;
		break;
	case end:
		if (offset >= (sint)_Buffer.size())
			return false;
		if (offset < 0)
			return false;
		_BufPos=_Buffer.end()-offset;
		break;
	}
	return true;
}


/*
 * serialBit (inherited from IStream)
 */
sint32 CMemStream::getPos () throw(EStream)
{
	return (sint32)&_BufPos[0]-(sint32)_Buffer[0];
}


/*
 * Clears the message
 */
void CMemStream::clear()
{
	resetPtrTable();
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


/*
 * Transforms the message from input to output or from output to input
 */
void CMemStream::invert()
{
	if ( isReading() )
	{
		// In->Out: We want to write (serialize out) what we have read (serialized in)
		resetPtrTable();
		setInOut( false );
		_BufPos = _Buffer.end();
	}
	else
	{
		// Out->In: We want to read (serialize in) what we have written (serialized out)
		resetPtrTable();
		setInOut( true );
		_BufPos = _Buffer.begin();
	}
}


}

