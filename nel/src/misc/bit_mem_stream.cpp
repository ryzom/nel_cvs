/** \file bit_mem_stream.cpp
 * Bit-oriented memory stream
 *
 * $Id: bit_mem_stream.cpp,v 1.22 2002/10/30 16:24:49 lecroart Exp $
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

#include "stdmisc.h"

#include "nel/misc/bit_mem_stream.h"

using namespace std;


namespace NLMISC {


/*
 * Constructor
 */
CBitMemStream::CBitMemStream( bool inputStream, uint32 defaultcapacity ) :
	CMemStream( inputStream, false, defaultcapacity ),
	_FreeBits( 8 )
{
	_BufPos--;
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
 * Serialize a buffer
 */
void CBitMemStream::serialBuffer( uint8 *buf, uint len )
{
	uint i;
	uint32 v;
	if ( isReading() )
	{
		for ( i=0; i!=len; ++i )
		{
			serial( v, 8 );
			buf[i] = (uint8)v;
		}
	}
	else
	{
		for ( i=0; i!=len; ++i )
		{
			v = (uint32)buf[i];
			serial( v, 8 );
		}
	}
}


/*
 * Serialize one bit
 */
void CBitMemStream::serialBit( bool& bit )
{
	uint32 ubit=0;
	if ( isReading() )
	{
		serial( ubit, 1 );
		bit = ( ubit!=0 );
	}
	else
	{
		ubit = bit;
		serial( ubit, 1 );
	}
}


//sint32 CBitMemStream::getPosInBit ()


#define displayByteBits(a,b,c)
/*void displayByteBits( uint8 b, uint nbits, sint beginpos )
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
}*/


#define displayDwordBits(a,b,c)
/*void displayDwordBits( uint32 b, uint nbits, sint beginpos )
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
}*/


/*
 * Serialize only the nbits lower bits of value
 */
void	CBitMemStream::serial( uint32& value, uint nbits, bool resetvalue )
{
	//nlassert( (nbits <= 32) && (nbits != 0) );
	if (nbits>32 || nbits==0)
		throw EMemStream (string("trying to serial ")+toString(nbits)+string(" %d bits"));

	if ( isReading() )
	{
		// Check that we don't read more than there is to read
		if (getPosInBit() + nbits > lengthR() * 8)
		{
			throw EStreamOverflow();
		}

		if ( resetvalue )
		{
			value = 0;
		}

		// Clear high-order bits after _FreeBits
		uint8 v = *_BufPos & ((1 << _FreeBits) - 1);

		if ( nbits > _FreeBits )
		{
			//nldebug( "Reading byte %u from %u free bits (%u remaining bits)", lengthS(), _FreeBits, nbits );
			value |= (v << (nbits-_FreeBits));
			++_BufPos;
			uint readbits = _FreeBits;
			displayByteBits( *_BufPos, 8, readbits-1 );
			_FreeBits = 8;
			serial( value, nbits - readbits, false ); // read without resetting value
		}
		else
		{
			//nlinfo( "Reading last byte %u from %u free bits (%u remaining bits)", lengthS(), _FreeBits, nbits );
			//displayByteBits( *_BufPos, 8, _FreeBits-1 );
			value |= (v >> (_FreeBits-nbits));
			displayByteBits( *_BufPos, 8, _FreeBits-1 );
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
		if ( _FreeBits == 8 ) // _FreeBits is from 7 downto 1, then 8
		{
			if (_BufPos == _Buffer.getPtr()+_Buffer.size() - 1)
			{
				// need to resize
				_Buffer.resize(_Buffer.size()*2+1);
				_BufPos = _Buffer.getPtr() + ((_Buffer.size()-1)/2);
			}
			else
			{
				_BufPos++;
			}
//			_Buffer.push_back(0);
//			_BufPos = _Buffer.end() - 1;
			*_BufPos = 0;
		}

		// Clear high-order bits after nbits
		//displayDwordBits( value, 32, nbits-1 );

		//uint32 mask = (-1 >> (32-nbits)); // does not work
		uint32 v;
		if ( nbits != 32 ) // arg of shl/sal/shr/sal ranges from 0 to 31
		{
			uint32 mask = (1 << nbits) - 1;
			v = value & mask;
		}
		else
		{
			v = value;
		}

		// Set
		if ( nbits > _FreeBits )
		{
			// Longer than the room in the current byte
			//nldebug( "Writing byte %u into %u free bits (%u remaining bits)", lengthS(), _FreeBits, nbits );
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
			//nldebug( "Writing last byte %u into %u free bits (%u remaining bits)", lengthS(), _FreeBits, nbits );
			displayByteBits( *_BufPos, 8, 7 );
			*_BufPos |= (v << (_FreeBits-nbits));
			displayByteBits( *_BufPos, 8, _FreeBits-1 );
			_FreeBits = ((_FreeBits-1 - nbits) % 8) + 1; // ((uint)-1) % 8 equals 7
		}

	}

}


/*
 * Serial float
 */
void	CBitMemStream::serial(float &b)
{
	uint32 uf=0;
	if ( isReading() )
	{
		serial( uf );
		b = *(float*)&uf;
	}
	else
	{
		uf = *(uint32*)&b;
		serial( uf );
	}
}


/*
 * Serial string
 */
void	CBitMemStream::serial(std::string &b) 
{
	uint32 len=0;

	// Serialize length
	if ( isReading() )
	{
		serial( len );
		if (len > length()-getPos())
		{
			nlwarning("string maximum length reached, perhaps invalid string size (%d)", len);
			throw NLMISC::EInvalidDataStream();
		}
		b.resize( len );
	}
	else
	{
		len = b.size();
		serial( len );
	}

	// Serialize buffer
	if ( len != 0 )
	{
		serialBuffer( (uint8*)(&*b.begin()), len );
	}
}

/*
 * Serial string
 */
void	CBitMemStream::serial(CBitMemStream &b)
{
	uint32 len=0;

	// Serialize length
	if ( isReading() )
	{
		// fill b with data from this
		serial (len);
		if (len > length()-getPos())
		{
			nlwarning("bitmemstream maximum length reached, perhaps invalid bitmemstream size (%d)", len);
			throw NLMISC::EInvalidDataStream();
		}

		serialBuffer (b.bufferToFill (len), len);
		b.resetBufPos ();
	}
	else
	{
		// fill this with data from b
		len = b.length();

		serial( len );
		serialBuffer( (uint8*) b.buffer (), len );
	}

}

/*
 * Specialisation of serialCont() for vector<bool>
 */
void CBitMemStream::serialCont(std::vector<bool> &cont)
{
	sint32	len=0;
	if(isReading())
	{
		serial(len);
		if (len/8 > (sint32)(length()-getPos()))
		{
			nlwarning("stl container maximum length reached, perhaps invalid container size (%d)", len);
			throw NLMISC::EInvalidDataStream();
		}
		// special version for vector: adjut good size.
		contReset(cont);
		cont.reserve(len);

		for(sint i=0;i<len;i++)
		{
			bool	v;
			serialBit(v);
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
			serialBit( b );
		}
	}
}



} // NLMISC
