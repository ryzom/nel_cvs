/** \file bit_mem_stream.cpp
 * Bit-oriented memory stream
 *
 * $Id: bit_mem_stream.cpp,v 1.27 2003/04/02 15:37:01 cado Exp $
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
#include "nel/misc/bit_set.h"

#ifdef LOG_ALL_TRAFFIC
#include "nel/misc/command.h"
#endif

using namespace std;


namespace NLMISC {

#ifdef LOG_ALL_TRAFFIC
bool VerboseAllTraffic = false;
#endif


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


#ifdef LOG_ALL_TRAFFIC

void	CBitMemStream::_serialAndLog( const char *argstr, uint32& value, uint nbits )
{
	sint32 bitpos = getPosInBit();
	serial( value, nbits );
	if ( VerboseAllTraffic )
		nldebug( "TRAFFIC/%p/%s: %s: %u bits at bitpos %d (%u)", this, isReading()?"I":"O", argstr, nbits, bitpos, value );
}

void	CBitMemStream::_serialAndLog( const char *argstr, uint64& value, uint nbits )
{
	sint32 bitpos = getPosInBit();
	serial( value, nbits );
	if ( VerboseAllTraffic )
		nldebug( "TRAFFIC/%p/%s: %s: %u bits at bitpos %d (%u)", this, isReading()?"I":"O", argstr, nbits, bitpos, value );
}

void	CBitMemStream::_serialBitAndLog( const char *argstr, bool& bit )
{
	sint32 bitpos = getPosInBit();
	serialBit( bit );
	if ( VerboseAllTraffic )
		nldebug( "TRAFFIC/%p/%s: %s: 1 bit at bitpos %d (%hu)", this, isReading()?"I":"O", argstr, bitpos, (uint16)bit );
}

NLMISC_COMMAND( verboseAllTraffic, "Verbose the all-traffic logs", "" )
{
	if(args.size()>1)
		return false;

	if(args.size()==1)
	{
		if(args[0]==string("on")||args[0]==string("ON")||args[0]==string("true")||args[0]==string("TRUE")||args[0]==string("1"))
			VerboseAllTraffic=true;
		else if(args[0]==string("off")||args[0]==string("OFF")||args[0]==string("false")||args[0]==string("FALSE")||args[0]==string("0"))
			VerboseAllTraffic=false;
	}

	nlinfo("verboseAllTraffic is %s",VerboseAllTraffic?"ON":"OFF");
	return true;
}

#endif


/*
 * Serialize only the nbits lower bits of value
 */
void	CBitMemStream::serial( uint32& value, uint nbits, bool resetvalue )
{
	//nlassert( (nbits <= 32) && (nbits != 0) );
	if (nbits>32 || nbits==0)
		throw EMemStream (string("trying to serial ")+toString(nbits)+string(" bits"));

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
 * In a output bit stream, serialize nbits bits (no matter their value).
 * Works even if the number of bits to add is larger than 64. See also poke() and pokeBits().
 */
void	CBitMemStream::reserveBits( uint nbits )
{
#ifdef LOG_ALL_TRAFFIC
	if ( VerboseAllTraffic )
		nldebug( "TRAFFIC/%p/%s: Reserving %u bits at bitpos %d", this, isReading()?"I":"O", nbits, getPosInBit() );	
#endif
	uint32 v = 0;
	while ( nbits > 32 )
	{
		serial( v, 32 );
		nbits -= 32;
	}
	serial( v, nbits );
}


/*
 * Helper for poke(), to write a value inside an output stream
 */
void	CBitMemStream::serialPoke( uint32 value, uint nbits )
{
	// Resize if necessary
	if ( _FreeBits == 8 ) // _FreeBits is from 7 downto 1, then 8
	{
		_BufPos++; // increment _BufPos but do not reset (*_BufPos)
	}

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
		serialPoke( v, nbits - filledbits );
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


/* Rewrite the nbbits lowest bits of a value at the specified position bitpos of the current output bit stream.
 * Precondition: bitpos+nbbits <= the current length in bit of the stream.
 */
void	CBitMemStream::poke( uint32 value, uint bitpos, uint nbits )
{
#ifdef NL_DEBUG
	nlassert( (nbits <= 32) && (nbits != 0) );
	nlassert( ! isReading() );
	nlassert( bitpos+nbits <= (uint)getPosInBit() );
#endif

	// Save the current pointers of the stream, and make them point to the required position
	uint savedFreeBits = _FreeBits;
	uint bytepos = bitpos >> 3;
	_FreeBits = 8 - (bitpos - (bytepos << 3));
	uint8 *savedBufPos = _BufPos;
	_BufPos = _Buffer.getPtr() + bytepos;

	// Serial
	serialPoke( value, nbits );

	// Restore the current pointers
	_FreeBits = savedFreeBits;
	_BufPos = savedBufPos;
}


/* Rewrite the bitfield at the specified position bitpos of the current output bit stream.
 * The size of the bitfield is *not* written into stream (unlike serialCont()).
 * Precondition: bitpos+bitfield.size() <= the current length in bit of the stream. See also reserveBits().
 */
void	CBitMemStream::pokeBits( const CBitSet& bitfield, uint bitpos )
{
#ifdef NL_DEBUG
	nlassert( ! isReading() );
	nlassert( bitpos+bitfield.size() <= (uint)getPosInBit() );
#endif

	// Save the current pointers of the stream, and make them point to the required position
	uint savedFreeBits = _FreeBits;
	uint bytepos = bitpos >> 3;
	_FreeBits = 8 - (bitpos - (bytepos << 3));
	uint8 *savedBufPos = _BufPos;
	_BufPos = _Buffer.getPtr() + bytepos;

	// Serial
	const vector<uint32>& uintVec = bitfield.getVector();
	if ( ! uintVec.empty() )
	{
		for ( uint i=0; i<uintVec.size()-1; ++i )
		{
			serialPoke( uintVec[i], 32 );
		}
		serialPoke( uintVec.back(), bitfield.size() % 32 );
	}

	// Restore the current pointers
	_FreeBits = savedFreeBits;
	_BufPos = savedBufPos;
}


/* Read nbits from the input stream to fill the bitfield.
 * It means you have to know its size.
 */
void	CBitMemStream::readBits( NLMISC::CBitSet& bitfield )
{
#ifdef NL_DEBUG
	nlassert( isReading() );
#endif
	uint len = bitfield.size();
	if ( len != 0 )
	{
#ifdef LOG_ALL_TRAFFIC
		if ( VerboseAllTraffic )
			nldebug( "TRAFFIC/%p/%s: Reading %u bits bitfield at bitpos %d", this, isReading()?"I":"O", len, getPosInBit() );
#endif
		uint i = 0;
		uint32 v;
		while ( len > 32 )
		{
			serial( v, 32 );
			bitfield.setUint( v, i );
			len -= 32;
			++i;
		}
		serial( v, len );
		bitfield.setUint( v, i );
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
#ifdef LOG_ALL_TRAFFIC
	sint32 bitpos = getPosInBit();
#endif

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

#ifdef LOG_ALL_TRAFFIC
	if ( VerboseAllTraffic )
		nldebug( "TRAFFIC/%p/%s: String (size 32+%u*8 bits) at bitpos %d", this, isReading()?"I":"O", len, bitpos );
#endif
}


/*
 * Serial string
 */
inline	void		CBitMemStream::serial(ucstring &b) 
{
#ifdef LOG_ALL_TRAFFIC
	sint32 bitpos = getPosInBit();
#endif

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

#ifdef LOG_ALL_TRAFFIC
	if ( VerboseAllTraffic )
		nldebug( "TRAFFIC/%p/%s: Ucstring at bitpos %d", this, isReading()?"I":"O", bitpos );
#endif

}


/*
 * Serial bitmemstream
 */
void	CBitMemStream::serialMemStream(CBitMemStream &b)
{
#ifdef LOG_ALL_TRAFFIC
	sint32 bitpos = getPosInBit();
#endif

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

#ifdef LOG_ALL_TRAFFIC
	if ( VerboseAllTraffic )
		nldebug( "TRAFFIC/%p/%s: Sub-bitmemstring (size 32+%u*8 bits) at bitpos %d", this, isReading()?"I":"O", len, bitpos );
#endif
}

/*
 * Specialisation of serialCont() for vector<bool>
 */
void CBitMemStream::serialCont(std::vector<bool> &cont)
{
#ifdef LOG_ALL_TRAFFIC
	sint32 bitpos = getPosInBit();
#endif

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

#ifdef LOG_ALL_TRAFFIC
	if ( VerboseAllTraffic )
		nldebug( "TRAFFIC/%p/%s: Container (header: 32 bits) at bitpos %d", this, isReading()?"I":"O", bitpos );
#endif

}



} // NLMISC
