/** \file mem_stream.cpp
 * CMemStream class
 *
 * $Id: mem_stream.cpp,v 1.15 2002/05/21 16:41:31 lecroart Exp $
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

#include "stdmisc.h"

#include "nel/misc/mem_stream.h"

namespace NLMISC
{


/*
 * Initialization constructor
 */
/*CMemStream::CMemStream( bool inputStream, bool stringmode, uint32 defaultcapacity ) :
	NLMISC::IStream( inputStream ), _StringMode( stringmode )
{
	_Buffer.resize (std::max( defaultcapacity, (uint32)4 )); // prevent from no allocation
	_BufPos = _Buffer.getPtr();
//	_Buffer.reserve( std::max( defaultcapacity, (uint32)4 ) ); // prevent from no allocation
//	_BufPos = _Buffer.begin();
}
*/

/*
 * Copy constructor
 */
/*CMemStream::CMemStream( const CMemStream& other ) :
	IStream (other)
{
	operator=( other );
}
*/

/*
 * Assignment operator
 */
/*CMemStream& CMemStream::operator=( const CMemStream& other )
{
	IStream::operator= (other);
	_Buffer = other._Buffer;
	_BufPos = _Buffer.getPtr() + other.lengthS();
	_StringMode = other._StringMode;
	return *this;
}
*/

/*
 * serial (inherited from IStream)
 */
void CMemStream::serialBuffer(uint8 *buf, uint len)
{
	// commented for optimum performance
	nlassert (len > 0);
	nlassert (buf != NULL);

	if ( isReading() )
	{
		// Check that we don't read more than there is to read
		if ( lengthS()+len > lengthR() )
		{
			throw EStreamOverflow();
		}
		// Serialize in
		CFastMem::memcpy( buf, _BufPos, len );
		_BufPos += len;
	}
	else
	{
		// Serialize out

		increaseBufferIfNecessary (len);
/*		uint32 oldBufferSize = _Buffer.size();
		if (_BufPos - _Buffer.getPtr() + len > oldBufferSize)
		{
			// need to increase the buffer size
			uint32 pos = _BufPos - _Buffer.getPtr();
			_Buffer.resize(oldBufferSize*2 + len);
			_BufPos = _Buffer.getPtr() + pos;
		}
*/
		CFastMem::memcpy( _BufPos, buf, len );
		_BufPos += len;

/*		_Buffer.resize( size );
		_BufPos = _Buffer.end() - len;
		CFastMem::memcpy( &(*_BufPos), buf, len );
		_BufPos = _Buffer.end();*/
	}
}

/*
 * serialBit (inherited from IStream)
 */
void CMemStream::serialBit(bool &bit)
{
/*	if ( _StringMode )
	{
*/		uint8 u;
		if ( isReading() )
		{
			serial( u );
			bit = (u!=0);
		}
		else
		{
			u = (uint8)bit;
			serial( u );
		}
/*	}
	else
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
	}*/
}


/*
 * seek (inherited from IStream)
 */
bool CMemStream::seek (sint32 offset, TSeekOrigin origin) throw(EStream)
{
	switch (origin)
	{
	case begin:
		if (offset >= (sint)length())
			return false;
		if (offset < 0)
			return false;
		_BufPos=_Buffer.getPtr()+offset;
		break;
	case current:
		if (getPos ()+offset >= (sint)length())
			return false;
		if (getPos ()+offset < 0)
			return false;
		_BufPos+=offset;
		break;
	case end:
		if (offset < -(sint)length())
			return false;
		if (offset > 0)
			return false;
		_BufPos=_BufPos+offset;
		break;
	}
	return true;
}


/*
 * getPos (inherited from IStream)
 */
/*sint32 CMemStream::getPos () throw(EStream)
{
	return _BufPos - _Buffer.getPtr();
//	if (_Buffer.getPtr() == NULL)
//		return 0;
//	else
//		return (sint32)&(_BufPos[0])-(sint32)&(_Buffer[0]);

}
*/

/*
 * Clears the message
 */
/*void CMemStream::clear()
{
	resetPtrTable();
	_Buffer.clear();
	_BufPos = _Buffer.getPtr();
}
*/

void CMemStream::resize (uint32 size)
{
	if (size == length()) return;
	// need to increase the buffer size
	uint32 pos = _BufPos - _Buffer.getPtr();
	_Buffer.resize(size);
	_BufPos = _Buffer.getPtr() + pos;
}

/*
 * Fills the message buffer
 */
/*void CMemStream::fill( const uint8 *srcbuf, uint32 len )
{
	if (len == 0) return;

	_Buffer.resize( len );
	_BufPos = _Buffer.getPtr();
	CFastMem::memcpy( _BufPos, srcbuf, len );
}
*/

/* EXPERIMENTAL: Returns a pointer to the message buffer for filling by an external function (use at your own risk,
 * you MUST fill the number of bytes you specify in "msgsize").
 * This method prevents from doing one useless buffer copy, using fill().
 */
/*uint8 *CMemStream::bufferToFill( uint32 msgsize )
{
	if (msgsize == 0) return NULL;

	// Same as fill() but the memcpy is done by an external function
	_Buffer.resize( msgsize );
	_BufPos = _Buffer.getPtr();
	return _BufPos;
}
*/

/*
 * Transforms the message from input to output or from output to input
 */
/*void CMemStream::invert()
{
	if ( isReading() )
	{
		// In->Out: We want to write (serialize out) what we have read (serialized in)
		resetPtrTable();
		setInOut( false );
//		_BufPos = _Buffer.end();
	}
	else
	{
		// Out->In: We want to read (serialize in) what we have written (serialized out)
		resetPtrTable();
		setInOut( true );
		_BufPos = _Buffer.getPtr();
	}
}
*/



/*** String-specific methods ***/


//const char SEPARATOR = ' ';
//const int SEP_SIZE = 1; // the code is easier to read with that



/*
 * Input: read from the stream until the next separator, and return the number of bytes read. The separator is then skipped.
 */
uint CMemStream::serialSeparatedBufferIn( uint8 *buf, uint len )
{
	nlassert( _StringMode && isReading() );

	// Check that we don't read more than there is to read
	if ( ( _BufPos == _Buffer.getPtr()+_Buffer.size() ) || // we are at the end
		 ( lengthS()+len+SEP_SIZE > lengthR() ) && (_Buffer[_Buffer.size()-1] != SEPARATOR ) ) // we are before the end
	{
		throw EStreamOverflow();
	}
	// Serialize in
	uint32 i = 0;
	while ( (i<len) && (*_BufPos) != SEPARATOR )
	{
		*(buf+i) = *_BufPos;
		i++;
		_BufPos++;
	}
	// Exceeds len
	if ( (*_BufPos) != SEPARATOR )
	{
		throw EStreamOverflow();
	}
	_BufPos += SEP_SIZE;
	return i;
}


/*
 * Output: writes len bytes from buf into the stream
 */
void CMemStream::serialSeparatedBufferOut( uint8 *buf, uint len )
{
	nlassert( _StringMode && (!isReading()) );
	
	// Serialize out
	uint32 oldBufferSize = _Buffer.size();
	if (_BufPos - _Buffer.getPtr() + (len + SEP_SIZE) > oldBufferSize)
	{
		// need to increase the buffer size
		_Buffer.resize(oldBufferSize*2 + len + SEP_SIZE);
		_BufPos = _Buffer.getPtr() + oldBufferSize;
	}

	CFastMem::memcpy( _BufPos, buf, len );
	_BufPos += len;
	*(_BufPos++) = SEPARATOR;

	// Serialize out
/*	_Buffer.resize( _Buffer.size() + len + SEP_SIZE );
	_BufPos = _Buffer.end() - len - SEP_SIZE;
	CFastMem::memcpy( &(*_BufPos), buf, len );
	_Buffer[_Buffer.size()-1] = SEPARATOR;
	_BufPos = _Buffer.end();
*/}

/*
// Input
#define readnumber(dest,thetype,digits,convfunc) \
	char number_as_cstring [digits+1]; \
	uint realdigits = serialSeparatedBufferIn( (uint8*)&number_as_cstring, digits ); \
	number_as_cstring[realdigits] = '\0'; \
	dest = (thetype)convfunc( number_as_cstring );

// Output
#define writenumber(src,format,digits) \
	char number_as_cstring [digits+1]; \
	sprintf( number_as_cstring, format, src ); \
	serialSeparatedBufferOut( (uint8*)&number_as_cstring, strlen(number_as_cstring) );
*/

/*
 * atoihex
 */
/*inline int atoihex( const char* ident )
{
	int number;
	sscanf( ident, "%x", &number );
	return number;
}

inline uint32 atoui( const char *ident)
{
	return (uint32) strtoul (ident, NULL, 10);
}
*/
/*
// ======================================================================================================
void		CMemStream::serial(uint8 &b) 
{
	if ( _StringMode )
	{
		if ( isReading() )
		{
			readnumber( b, uint8, 3, atoi ); // 255
		}
		else
		{
			writenumber( (uint16)b,"%hu", 3 );
		}
	}
	else
	{
		fastWrite (b);
	}
}

// ======================================================================================================
void		CMemStream::serial(sint8 &b) 
{
	if ( _StringMode )
	{
		if ( isReading() )
		{
			readnumber( b, sint8, 4, atoi ); // -128
		}
		else
		{
			writenumber( (sint16)b, "%hd", 4 );
		}
	}
	else
	{
		fastWrite (b);
	}
}

// ======================================================================================================
void		CMemStream::serial(uint16 &b) 
{
	if ( _StringMode )
	{
		// No byte swapping in text mode
		if ( isReading() )
		{
			readnumber( b, uint16, 5, atoi ); // 65535
		}
		else
		{
			writenumber( b, "%hu", 5 );
		}
	}
	else
	{
		fastWrite (b);
	}
}

// ======================================================================================================
void		CMemStream::serial(sint16 &b) 
{
	if ( _StringMode )
	{
		if ( isReading() )
		{
			readnumber( b, sint16, 6, atoi ); // -32768
		}
		else
		{
			writenumber( b, "%hd", 6 );
		}
	}
	else
	{
		fastWrite (b);
	}
}

// ======================================================================================================
void		CMemStream::serial(uint32 &b) 
{
	if ( _StringMode )
	{
		if ( isReading() )
		{
			readnumber( b, uint32, 10, atoui ); // 4294967295
		}
		else
		{
			writenumber( b, "%u", 10 );
		}
	}
	else
	{
		fastWrite (b);
	}
}


// ======================================================================================================
void		CMemStream::serial(sint32 &b) 
{
	if ( _StringMode )
	{
		if ( isReading() )
		{
			readnumber( b, sint32, 11, atoi ); // -2147483648
		}
		else
		{
			writenumber( b, "%d", 11 );
		}
	}
	else
	{
		fastWrite (b);
	}
}

// ======================================================================================================
void		CMemStream::serial(uint64 &b) 
{
	if ( _StringMode )
	{
		if ( isReading() )
		{
			readnumber( b, uint64, 20, atoiInt64 ); // 18446744073709551615
		}
		else
		{
			writenumber( b, "%"NL_I64"u", 20 );
		}
	}
	else
	{
		fastWrite (b);
	}
}

// ======================================================================================================
void		CMemStream::serial(sint64 &b) 
{
	if ( _StringMode )
	{
		if ( isReading() )
		{
			readnumber( b, sint64, 20, atoiInt64 ); // -9223372036854775808
		}
		else
		{
			writenumber( b, "%"NL_I64"d", 20 );
		}
	}
	else
	{
		fastWrite (b);
	}
}

// ======================================================================================================
void		CMemStream::serial(float &b) 
{
	if ( _StringMode )
	{
		if ( isReading() )
		{
			readnumber( b, float, 128, atof ); // ?
		}
		else
		{
			writenumber( (double)b, "%f", 128 );
		}
	}
	else
	{
		fastWrite (b);
	}
}

// ======================================================================================================
void		CMemStream::serial(double &b) 
{
	if ( _StringMode )
	{
		if ( isReading() )
		{
			readnumber( b, double, 128, atof ); //
		}
		else
		{
			writenumber( b, "%f", 128 );
		}
	}
	else
	{
		fastWrite (b);
	}
}

// ======================================================================================================
void		CMemStream::serial(bool &b) 
{
	if ( _StringMode )
	{
		serialBit(b);
	}
	else
	{
#ifdef NL_LITTLE_ENDIAN
		if(isReading())
		{
			IStream::serial( b );
		}
		else
		{
			*(uint8*)_BufPos = b;
			_BufPos += sizeof(uint8);
		}
#else // NL_LITTLE_ENDIAN
		IStream::serial( b );
#endif // NL_LITTLE_ENDIAN
	}
}


#ifndef NL_OS_CYGWIN
// ======================================================================================================
void		CMemStream::serial(char &b) 
{
	if ( _StringMode )
	{
		char buff [2];
		if ( isReading() )
		{
			serialBuffer( (uint8*)buff, 2 );
			b = buff[0];
		}
		else
		{
			buff[0] = b;
			buff[1] = SEPARATOR;
			serialBuffer( (uint8*)buff, 2 );
		}
	}
	else
	{
		fastWrite (b);
	}
}
#endif

// ======================================================================================================
void		CMemStream::serial(std::string &b) 
{
	if ( _StringMode )
	{
		sint32	len=0;
		// Read/Write the length.
		if(isReading())
		{
			serial(len);
			nlassert( len<1000000 ); // limiting string size
			b.resize(len);
		}
		else
		{
			len= b.size();
			serial(len);
		}
		
		// Read/Write the string.
		for(sint i=0;i<len;i++)
			serialBuffer( (uint8*)&(b[i]), sizeof(b[i]) );

		char sep = SEPARATOR;
		serialBuffer( (uint8*)&sep, 1 );
	}
	else
	{
		IStream::serial( b );
	}
}


// ======================================================================================================
void		CMemStream::serial(ucstring &b) 
{
	if ( _StringMode )
	{
		sint32	len=0;
		// Read/Write the length.
		if(isReading())
		{
			serial(len);
			b.resize(len);
		}
		else
		{
			len= b.size();
			serial(len);
		}
		// Read/Write the string.
		for(sint i=0;i<len;i++)
			serialBuffer( (uint8*)&b[i], sizeof( sizeof(b[i]) ) );

		char sep = SEPARATOR;
		serialBuffer( (uint8*)&sep, 1 );
	}
	else
	{
		IStream::serial( b );
	}
}


// Specialisation of serialCont() for vector<bool>
/*void	CMemStream::serialCont(std::vector<bool> &cont)
{
	sint32	len=0;
	if(isReading())
	{
		serial(len);
		// special version for vector: adjut good size.
		contReset(cont);
		cont.reserve(len);

		for(sint i=0;i<len;i++)
		{
			bool	v;
			serial(v);
			cont.insert(cont.end(), v);
		}
	}
	else
	{
		len= cont.size();
		serial(len);

		std::vector<bool>::iterator it= cont.begin();
		for(sint i=0;i<len;i++, it++)
		{
			bool b = *it;
			serial( b );
		}
	}
}*/


/*
 * Serialisation in hexadecimal
 */
/*void	CMemStream::serialHex(uint32 &b)
{
	if ( _StringMode )
	{
		if ( isReading() )
		{
			readnumber( b, uint32, 10, atoihex ); // 4294967295
		}
		else
		{
			writenumber( b, "%x", 10 );
		}
		}
	else
	{
		IStream::serial( b );
	}
}

*/

}

	