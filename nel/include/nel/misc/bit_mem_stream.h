/** \file bit_mem_stream.h
 * Bit-oriented memory stream
 *
 * $Id: bit_mem_stream.h,v 1.23 2003/04/02 15:37:16 cado Exp $
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

#ifndef NL_BIT_MEM_STREAM_H
#define NL_BIT_MEM_STREAM_H

#include "nel/misc/types_nl.h"
#include "nel/misc/mem_stream.h"


namespace NLMISC {


/* In debugging stage, should be defined. In stable stage, undefine it!
 * Works along with the verboseAllTraffic command
 */
#ifndef NL_RELEASE
#define LOG_ALL_TRAFFIC
#endif

#ifdef LOG_ALL_TRAFFIC

extern bool VerboseAllTraffic;

#define serialAndLog1( v ) \
	_serialAndLog( #v, v );

#define serialAndLog2( v, s ) \
	_serialAndLog( #v, v, s );

#define serialBitAndLog( v ) \
	_serialBitAndLog( #v, v );

#define	serialAdaptAndLog( argstr, b, type ) \
	uint32 ub=0; \
	if ( isReading() ) \
	{ \
		_serialAndLog( argstr, ub, sizeof(type)*8 ); \
		b = (type)ub; \
	} \
	else \
	{ \
		ub = (uint32)b; \
		_serialAndLog( argstr, ub, sizeof(type)*8 ); \
	}

#ifdef NL_LITTLE_ENDIAN

#define	serialAdapt64AndLog( argstr, b ) \
	_serialAndLog( argstr, *((uint32*)(&b)), 32 ); \
	_serialAndLog( argstr, *((uint32*)(&b)+1), 32 );

#else

#define	serialAdapt64AndLog( argstr, b ) \
	serialAndLog( argstr, *((uint32*)(&b)+1), 32); \
	serialAndLog( argstr, *((uint32*)(&b)), 32);

#endif


#else
	
#define serialAndLog1 serial
#define serialAndLog2 serial
#define serialBitAndLog serialBit


#endif

#define	serialAdapt( b, type ) \
	uint32 ub=0; \
	if ( isReading() ) \
	{ \
		serial( ub, sizeof(type)*8 ); \
		b = (type)ub; \
	} \
	else \
	{ \
		ub = (uint32)b; \
		serial( ub, sizeof(type)*8 ); \
	}

#ifdef NL_LITTLE_ENDIAN

#define	serialAdapt64( b ) \
	serial( *((uint32*)(&b)), 32); \
	serial( *((uint32*)(&b)+1), 32);

#else

#define	serialAdapt64( b ) \
	serial( *((uint32*)(&b)+1), 32); \
	serial( *((uint32*)(&b)), 32);

#endif


class CBitSet;


/**
 * Bit-oriented memory stream
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CBitMemStream : public CMemStream
{
public:

	/// Constructor
	CBitMemStream( bool inputStream=false, uint32 defaultcapacity=32 );

	/// Copy constructor
	CBitMemStream( const CBitMemStream& other );

	/// Assignment operator
	CBitMemStream&	operator=( const CBitMemStream& other ) { CMemStream::operator=( other ); _FreeBits = other._FreeBits; return *this; }

	/** Returns the length (size) of the message, in bytes.
	 * If isReading(), it is the number of bytes that can be read,
	 * otherwise it is the number of bytes that have been written.
	 */
	virtual uint32	length() const
	{
		if ( isReading() )
		{
			return lengthR();
		}
		else
		{
			if ( _Buffer.getPtr() -  1 == _BufPos )
			{
				return 0;
			}
			else
			{
				return lengthS() + 1;
			}
		}
	}

	/// Transforms the message from input to output or from output to input
	virtual void	invert()
	{
		if (isReading())
		{
			CMemStream::invert();
			_BufPos--;
			_FreeBits = 8;
		}
		else
		{
			_BufPos++;
			CMemStream::invert();
			_FreeBits = 8;
		}
	}

	/// Clears the message
	virtual void	clear()
	{
		CMemStream::clear();
		_FreeBits = 8;
		_BufPos--;
	}

	/// Returns the number of bit from the beginning of the buffer (in bit)
	sint32	getPosInBit()
	{
		if (isReading())
		{
			return getPos() * 8 + (8 - _FreeBits);
		}
		else
		{
			if (_Buffer.getPtr() - 1 == _BufPos)
				return 0;
			else if (_FreeBits == 8)
				return (getPos() + 1) * 8;
			else
				return getPos() * 8 + (8 - _FreeBits);
	/*		if (_Buffer.empty())
				return 0;
			else if (_FreeBits == 8)
				return (getPos() + 1) * 8;
			else
				return getPos() * 8 + (8 - _FreeBits);
	*/	}
	}


	/// Serialize a buffer
	virtual void	serialBuffer(uint8 *buf, uint len);

	/// Serialize one bit
	virtual void	serialBit( bool& bit );

#ifdef LOG_ALL_TRAFFIC
	void			_serialAndLog( const char *argstr, uint32& value, uint nbits );
	void			_serialAndLog( const char *argstr, uint64& value, uint nbits );
	void			_serialBitAndLog( const char *argstr, bool& bit );
#endif

	/** Serialize only the nbits lower bits of value (nbits range: [1..32])
	 * When using this method, always leave resetvalue to true.
	 */
	void			serial( uint32& value, uint nbits, bool resetvalue=true );

	/// Serialize only the nbits lower bits of 64-bit value (nbits range: [1..64])
	void			serial( uint64& value, uint nbits )
	{
		if ( nbits > 32 )
		{
			if ( isReading() )
			{
				// Reset and read MSD
				uint32 msd = 0;
				serial( msd, nbits-32 );
				value = (uint64)msd << 32;
				// Reset and read LSD
				serial( (uint32&)value, 32 );
			}
			else
			{
				// Write MSD
				uint32 msd = (uint32)(value >> 32);
				serial( msd, nbits-32 );
				// Write LSD
				serial( (uint32&)value, 32 );
			}
		}
		else
		{
			if ( isReading() )
			{
				// Reset MSB (=0 is faster than value&=0xFFFFFFFF)
				value = 0;
			}
			// Read or write LSB
			serial( (uint32&)value, nbits );
		}
	}

	/** In a output bit stream, serialize nbits bits (no matter their value).
	 * Works even if the number of bits to add is larger than 64. See also poke() and pokeBits().
	 */
	void			reserveBits( uint nbits );

	/** Rewrite the nbits lowest bits of a value at the specified position bitpos of the current output bit stream.
	 * Precondition: bitpos+nbits <= the current length in bit of the stream. See also reserveBits().
	 */
	void			poke( uint32 value, uint bitpos, uint nbits );

	/** Rewrite the bitfield at the specified position bitpos of the current output bit stream.
	 * The size of the bitfield is *not* written into the stream (unlike serialCont()).
	 * Precondition: bitpos+bitfield.size() <= the current length in bit of the stream. See also reserveBits().
	 */
	void			pokeBits( const NLMISC::CBitSet& bitfield, uint bitpos );

	/** Read bitfield.size() bits from the input stream to fill the bitfield.
	 * It means you have to know the size and to resize the bitfield yourself.
	 */
	void			readBits( NLMISC::CBitSet& bitfield );

	/// Template serialisation (should take the one from IStream)
    template<class T>
	void			serial(T &obj)							{ obj.serial(*this); }

	// CMemStream::serialCont() will call CBitMemStream's virtual serialBuffer()
	template<class T>
	void			serialCont(std::vector<T> &cont) 		{CMemStream::serialCont(cont);}
	template<class T>
	void			serialCont(std::list<T> &cont) 			{CMemStream::serialCont(cont);}
	template<class T>
	void			serialCont(std::deque<T> &cont) 		{CMemStream::serialCont(cont);}
	template<class T>
	void			serialCont(std::set<T> &cont) 			{CMemStream::serialCont(cont);}
	template<class T>
	void			serialCont(std::multiset<T> &cont) 		{CMemStream::serialCont(cont);}
	template<class K, class T>
	void			serialCont(std::map<K, T> &cont) 		{CMemStream::serialCont(cont);}
	template<class K, class T>
	void			serialCont(std::multimap<K, T> &cont) 	{CMemStream::serialCont(cont);}

	/*template<class T0,class T1>
	void			serial(T0 &a, T1 &b) 
	{ serial(a); serial(b);}
	template<class T0,class T1,class T2>
	void			serial(T0 &a, T1 &b, T2 &c) 
	{ serial(a); serial(b); serial(c);}
	template<class T0,class T1,class T2,class T3>
	void			serial(T0 &a, T1 &b, T2 &c, T3 &d) 
	{ serial(a); serial(b); serial(c); serial(d);}
	template<class T0,class T1,class T2,class T3,class T4>
	void			serial(T0 &a, T1 &b, T2 &c, T3 &d, T4 &e) 
	{ serial(a); serial(b); serial(c); serial(d); serial(e);}
	template<class T0,class T1,class T2,class T3,class T4,class T5>
	void			serial(T0 &a, T1 &b, T2 &c, T3 &d, T4 &e, T5 &f) 
	{ serial(a); serial(b); serial(c); serial(d); serial(e); serial(f);}*/

	/** \name Base type serialisation.
	 * Those method are a specialisation of template method "void serial(T&)".
	 */
	//@{


/*
#define	serialAdapt64( b, type ) \
	uint32 ubl=0, ubh=0; \
	if ( isReading() ) \
	{ \
		serial( ubh, sizeof(uint32)*8 ); \
		serial( ubl, sizeof(uint32)*8 ); \
		b = (((type)ubh)<<32)+ubl; \
	} \
	else \
	{ \
		ubh = (uint32)(b>>32); \
		ubl = (uint32)(b); \
		serial( ubh, sizeof(uint32)*8 ); \
		serial( ubl, sizeof(uint32)*8 ); \
	}
*/

#ifdef LOG_ALL_TRAFFIC
	void			_serialAndLog(const char *argstr, uint8 &b) { serialAdaptAndLog( argstr, b, uint8 ); }
	void			_serialAndLog(const char *argstr, sint8 &b) { serialAdaptAndLog( argstr, b, sint8 ); }
	void			_serialAndLog(const char *argstr, uint16 &b) { serialAdaptAndLog( argstr, b, uint16 ); }
	void			_serialAndLog(const char *argstr, sint16 &b) { serialAdaptAndLog( argstr, b, sint16 ); }
	void			_serialAndLog(const char *argstr, uint32 &b) { serialAdaptAndLog( argstr, b, uint32 ); }
	void			_serialAndLog(const char *argstr, sint32 &b) { serialAdaptAndLog( argstr, b, sint32 ); }
	void			_serialAndLog(const char *argstr, uint64 &b) { serialAdapt64AndLog( argstr, b ); }
	void			_serialAndLog(const char *argstr, sint64 &b) { serialAdapt64AndLog( argstr, b ); }
	void			_serialAndLog(const char *argstr, float &b);
	void			_serialAndLog(const char *argstr, double &b) { serialAdapt64AndLog( argstr, b ); }
	void			_serialAndLog(const char *argstr, bool &b) { _serialBitAndLog( argstr, b ); }
#ifndef NL_OS_CYGWIN
	virtual void	_serialAndLog(const char *argstr, char &b) { serialAdaptAndLog( argstr, b, char ); }
#endif
#endif

	virtual void	serial(uint8 &b) { serialAdapt( b, uint8 ); }
	virtual void	serial(sint8 &b) { serialAdapt( b, sint8 ); }
	virtual void	serial(uint16 &b) { serialAdapt( b, uint16 ); }
	virtual void	serial(sint16 &b) { serialAdapt( b, sint16 ); }
	virtual void	serial(uint32 &b) { serialAdapt( b, uint32 ); }
	virtual void	serial(sint32 &b) { serialAdapt( b, sint32 ); }
	virtual void	serial(uint64 &b) { serialAdapt64( b ); }
	virtual void	serial(sint64 &b) { serialAdapt64( b ); }
	virtual void	serial(float &b);
	virtual void	serial(double &b) { serialAdapt64( b ); }
	virtual void	serial(bool &b) { serialBit( b ); }
#ifndef NL_OS_CYGWIN
	virtual void	serial(char &b) { serialAdapt( b, char ); }
#endif

	virtual void	serial(std::string &b);
	virtual void	serial(ucstring &b);

	virtual void	serial(CBitMemStream &b) { serialMemStream(b); }
	virtual void	serialMemStream(CBitMemStream &b);
		

	//@}

	/// Specialisation of serialCont() for vector<uint8>
	virtual void			serialCont(std::vector<uint8> &cont) { serialVector(cont); }
	/// Specialisation of serialCont() for vector<sint8>
	virtual void			serialCont(std::vector<sint8> &cont) { serialVector(cont); }
	/// Specialisation of serialCont() for vector<bool>
	virtual void			serialCont(std::vector<bool> &cont);

protected:

	/// Helper for poke(), to write a value inside an output stream
	void			serialPoke( uint32 value, uint nbits );

	uint			_FreeBits; // From 8 downto 1

};

} // NLMISC


#endif // NL_BIT_MEM_STREAM_H

/* End of bit_mem_stream.h */
