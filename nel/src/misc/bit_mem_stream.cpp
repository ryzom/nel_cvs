/** \file bit_mem_stream.cpp
 * Bit-oriented memory stream
 *
 * $Id: bit_mem_stream.cpp,v 1.1 2001/10/05 16:23:28 cado Exp $
 */

/* Copyright, 2000, 2001 Nevrax Ltd.
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

#include "nel/misc/bit_mem_stream.h"
#include <string>

using namespace std;


namespace NLMISC {


/*
 * Constructor
 */
CBitMemStream::CBitMemStream( bool inputStream, uint32 defaultcapacity ) :
	CMemStream( inputStream, defaultcapacity ),
	_FreeBits( 8 )
{
}


/*
 * Copy constructor
 */
CBitMemStream::CBitMemStream( const CBitMemStream& other ) :
	CMemStream( other ),
	_FreeBits( other._FreeBits )
{
}


/*
 * Transforms the message from input to output or from output to input
 */
void CBitMemStream::invert()
{
	CMemStream::invert();
	_FreeBits = 8;
}


/*
 * Serialize one bit
 */
/*void	CBitMemStream::serialBit( bool& bit )
{
	nlerror ( "Adapt to new bitpos" );
	// _BufPos/_BitPos are pointing the next bit to read/write
	
	if ( isReading() )
	{
		// Input

		// Check that we don't read more than there is to read
		if ( (_BitPos == 0) && (lengthS() > lengthR()) )
		{
			throw EStreamOverflow();
		}

		// Get bit
		uint8 b = *_BufPos;
		bit = (b >> _BitPos) & 1;

		// Increment position
		if ( _BitPos == 7 )
		{
			++_BufPos;
			_BitPos = 0;
		}
		else
		{
			++_BitPos;
		}
	}
	else
	{
		// Output
		if ( _BitPos == 0 )
		{
			// Add one new byte
			_Buffer.resize( _Buffer.size() + 1 );
			_BufPos = _Buffer.end() - 1;
		}

		// Set bit
		*_BufPos |= (((uint8)bit) << _BitPos);

		// Increment position
		_BitPos = (_BitPos + 1) % 8;
	}
}*/


void displayByteBits( uint8 b, uint nbits, sint beginpos )
{
	string s1, s2;
	sint i;
	for ( i=nbits-1; i!=-1; --i )
	{
		s1 += ( (b >> i) & 1 ) ? "1" : "0";
	}
	nlinfo( "%s", s1.c_str() );
	for ( i=nbits; i>beginpos+1; --i )
	{
		s2 += " ";
	}
	s2 += "^";
	nlinfo( "%s beginpos=%u", s2.c_str(), beginpos );
}


void displayDwordBits( uint32 b, uint nbits, sint beginpos )
{
	string s1, s2;
	sint i;
	for ( i=nbits-1; i!=-1; --i )
	{
		s1 += ( (b >> i) & 1 ) ? "1" : "0";
	}
	nlinfo( "%s", s1.c_str() );
	for ( i=nbits; i>beginpos+1; --i )
	{
		s2 += " ";
	}
	s2 += "^";
	nlinfo( "%s beginpos=%u", s2.c_str(), beginpos );
}


/*
 * Serialize only the nbits lower bits of value (when reading, please initialize to zero your variables)
 */
void	CBitMemStream::serial( uint32& value, uint nbits )
{
	nlassert( (nbits <= 32) && (nbits != 0) );

	if ( isReading() )
	{
		// Check that we don't read more than there is to read
		if ( lengthS()+(nbits+8-_FreeBits)/8 > lengthR())
		{
			throw EStreamOverflow();
		}
	
		// Clear high-order bits after _FreeBits
		uint8 v = *_BufPos; // & ((1 << _FreeBits) - 1);

		if ( nbits > _FreeBits )
		{
			nldebug( "Reading byte from %u free bits (%u remaining bits)", _FreeBits, nbits );
			value |= (v << (nbits-_FreeBits));
			++_BufPos;
			uint readbits = _FreeBits;
			displayByteBits( *_BufPos, 8, readbits-1 );
			_FreeBits = 8;
			serial( value, nbits - readbits );
		}
		else
		{
			nldebug( "Reading last byte from %u free bits (%u remaining bits)", _FreeBits, nbits );
			value |= (v >> (_FreeBits-nbits));
			if ( _FreeBits == nbits )
			{
				_FreeBits = 8;
				++_BufPos;
			}
			else
			{
				_FreeBits -= nbits;
			}
		}
	}
	else
	{
		// Resize if necessary
		if ( _FreeBits == 8 )
		{
			_Buffer.push_back();
			_BufPos = _Buffer.end() - 1;
			*_BufPos = 0;
		}

		// Clear high-order bits after nbits
		displayDwordBits( value, 32, nbits-1 );
		uint32 mask = (-1 >> (32-nbits)); // (-1 << nbits) - 1 would not work because shl's arg ranges frmo 0 to 31
		uint32 v = value & mask;

		// Set
		if ( nbits > _FreeBits )
		{
			// Longer than the room in the current byte
			nldebug( "Writing byte into %u free bits (%u remaining bits)", _FreeBits, nbits );
			displayDwordBits( value, 32, nbits-1 );
			*_BufPos |= (v >> (nbits - _FreeBits));
			uint filledbits = _FreeBits;
			displayByteBits( *_BufPos, 8, filledbits-1 );
			_FreeBits = 8;
			serial( v, nbits - filledbits );
		}
		else
		{
			// Shorter or equal
			nldebug( "Writing last byte into %u free bits (%u remaining bits)", _FreeBits, nbits );
			displayByteBits( *_BufPos, 8, 7 );
			*_BufPos |= (v << (_FreeBits-nbits));
			displayByteBits( *_BufPos, 8, _FreeBits-1 );
			_FreeBits = ((_FreeBits-1 - nbits) % 8) + 1; // ((uint)-1) % 8 equals 7
		}

	}

#if 0
	if ( isReading() )
	{
		// Input
		uint totalmodbits = _BitPos + nbits;

		// Check that we don't read more than there is to read
		if ( lengthS()+totalmodbits/8 > lengthR())
		{
			throw EStreamOverflow();
		}

		// Get first part of value (32-_BitPos low-order bits)
		bool extended = false;
		if ( lengthS()+32 > lengthR() )
		{
			// First, ensure we can read 32 bits without hitting the vector's boundary
			_Buffer.resize( _Buffer.size() + sizeof(uint32) );
			extended = true;
		}
		uint32 v;
		uint32 *p = (uint32*)&*_BufPos;
		v = (*p >> _BitPos);
		_BufPos += totalmodbits/8;

		// Get the remaining _BitPos high-order bits
		if ( totalmodbits > 32 )
		{
			v |= (*_BufPos<<(sizeof(uint32)-_BitPos));
		}
		_BitPos = totalmodbits % 8;

		if ( extended )
		{
			// Set the original size back
			_Buffer.resize( _Buffer.size() - sizeof(uint32) );
		}

		value = v;
	}
	else
	{
		// Output
		uint totalmodbits = _BitPos + nbits;
		uint32 bufinitsize = _Buffer.size();

		// 1. Add 4 new bytes (even if nbits is lower than 32, we will reduce the size later, in 4.)
		if ( (_BitPos == 0) || (totalmodbits > 8) )
		{
			_Buffer.resize( bufinitsize + sizeof(uint32) );
			_BufPos = _Buffer.end() - sizeof(uint32);
		}

		// 2. Set first part of value (32-_BitPos low-order bits)
		/* Diagram (-=Unchanged M=Modified, low->high):
		 *
		 * _BufPos            _BufPos+1          _BufPos+2          _BufPos+3          _BufPos+4
		 *           _BitPos
		 * - - - - - M M M    M M M M M M M M    M M M M M M M M    M M M M M M M M
		 */
		uint32 *p = (uint32*)&*_BufPos;
		*p |= (value << _BitPos);

		// 3. Set the remaining _BitPos high-order bits from value to _BufPos+4 low-order bits
		/*
		* _BufPos            _BufPos+1          _BufPos+2          _BufPos+3          _BufPos+4
		*           _BitPos
		*                                                                             M M M - - - - -
		*/
		if ( totalmodbits > 32 )
		{
			_BufPos += sizeof(uint32);
			*_BufPos |= ( value >> (sizeof(uint32)-_BitPos) );
		}

		// 4. Resize to only nbits instead of 32
		_Buffer.resize( (sint)bufinitsize + totalmodbits/8 );
		_BitPos = totalmodbits % 8;
		_BufPos = _Buffer.end() - (uint32)(_BitPos != 0); // set to _Buffer.end() if (_BitPos==0)
	}
#endif
}


/*
 * Serialize a bit vector
 */
/*void	CBitMemStream::serial( const std::vector<bool>& bitvect )
{

}*/



} // NLMISC
