/** \file mem_stream.h
 * From memory serialization implementation of IStream using ASCII format (look at stream.h)
 *
 * $Id: mem_stream.h,v 1.24 2002/09/10 09:24:49 cado Exp $
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

#ifndef NL_MEM_STREAM_H
#define NL_MEM_STREAM_H

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include "nel/misc/object_vector.h"
#include "nel/misc/fast_mem.h"

namespace NLMISC
{

/// Exception class for CMemStream
struct EMemStream : public NLMISC::EStream
{
	EMemStream( const std::string& str ) : EStream( str ) {}
};

/// This exception is raised when someone tries to serialize in more than there is.
struct EStreamOverflow : public EMemStream
{
	EStreamOverflow() : EMemStream( "Stream Overflow Error" ) {}
};

/*
/// Vector of uint8
typedef std::vector<uint8> CVector8;

/// Iterator on CVector8
typedef CVector8::iterator It8;
*/


/**
 * MemStream memory stream (see also NLNET::CMessage).
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CMemStream : public NLMISC::IStream
{
public:

	/// Initialization constructor
	CMemStream( bool inputStream=false, bool stringmode=false, uint32 defaultcapacity=0 ) :
		NLMISC::IStream( inputStream ), _StringMode( stringmode )
	{
		_DefaultCapacity = std::max( defaultcapacity, (uint32)16 ); // prevent from no allocation
		_Buffer.resize (_DefaultCapacity);
		_BufPos = _Buffer.getPtr();
	}

	/// Copy constructor
	CMemStream( const CMemStream& other ) :
		IStream (other)
	{
		operator=( other );
	}

	/// Assignment operator
	CMemStream&		operator=( const CMemStream& other )
	{
		IStream::operator= (other);
		_Buffer = other._Buffer;
		_BufPos = _Buffer.getPtr() + other.lengthS();
		_StringMode = other._StringMode;
		_DefaultCapacity = other._DefaultCapacity;
		return *this;
	}

	/// Set string mode
	void			setStringMode( bool stringmode ) { _StringMode = stringmode; }

	/// Return string mode
	bool			stringMode() const { return _StringMode; }

	/// Method inherited from IStream
	virtual void	serialBuffer(uint8 *buf, uint len);

	/// Method inherited from IStream
	virtual void	serialBit(bool &bit);

	/** 
	 * Moves the stream pointer to a specified location.
	 * 
	 * NB: If the stream doesn't support the seek fonctionnality, it throws ESeekNotSupported.
	 * Default implementation: 
	 * { throw ESeekNotSupported; }
	 * \param offset is the wanted offset from the origin.
	 * \param origin is the origin of the seek
	 * \return true if seek sucessfull.
	 * \see ESeekNotSupported SeekOrigin getPos
	 */
	virtual bool	seek (sint32 offset, TSeekOrigin origin) throw(EStream);

	/** 
	 * Get the location of the stream pointer.
	 * 
	 * NB: If the stream doesn't support the seek fonctionnality, it throws ESeekNotSupported.
	 * Default implementation: 
	 * { throw ESeekNotSupported; }
	 * \param offset is the wanted offset from the origin.
	 * \param origin is the origin of the seek
	 * \return the new offset regarding from the origin.
	 * \see ESeekNotSupported SeekOrigin seek
	 */
	virtual sint32	getPos () throw(EStream)
	{
		return _BufPos - _Buffer.getPtr();
	}

	/// Clears the message
	virtual void	clear()
	{
		resetPtrTable();
		_Buffer.clear();
		if (!isReading())
		{
			_Buffer.resize (_DefaultCapacity);
		}
		_BufPos = _Buffer.getPtr();
	}

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
			return lengthS();
		}
	}

	/** Returns a pointer to the message buffer (read only)
	 * Returns NULL if the buffer is empty
	 */
	const uint8		*buffer() const
	{
		return _Buffer.getPtr();
/*		if ( _Buffer.empty() )
		{
			return NULL;
		}
		else
		{
			return &(*_Buffer.begin());
		}*/
	}

/*	/// Returns the message buffer (read only)
	const CVector8&	bufferAsVector() const
	{
		return _Buffer;
	}

	/// Returns the vector for external filling
	CVector8&	bufferAsVector()
	{
		return _Buffer;
	}
*/
	// When you fill the buffer externaly (using bufferAsVector) you have to reset the BufPos calling this method
	void resetBufPos() { _BufPos = _Buffer.getPtr(); }

	/// Fills the message buffer, for reading
	void			fill( const uint8 *srcbuf, uint32 len )
	{
		if (len == 0) return;

		_Buffer.resize( len );
		CFastMem::memcpy( _Buffer.getPtr(), srcbuf, len );
		if (isReading())
		{
			_BufPos = _Buffer.getPtr();
		}
		else
		{
			_BufPos = _Buffer.getPtr() + _Buffer.size();
		}
	}

	void resize (uint32 size);

	/** EXPERIMENTAL: Returns a pointer to the message buffer for filling by an external function (use at your own risk,
	 * you MUST fill the number of bytes you specify in "msgsize").
	 * This method prevents from doing one useless buffer copy, using fill().
	 */
	uint8			*bufferToFill( uint32 msgsize )
	{
		if (msgsize == 0) return NULL;

		// Same as fill() but the memcpy is done by an external function
		_Buffer.resize( msgsize );
		_BufPos = _Buffer.getPtr();
		return _BufPos;
	}

	/// Transforms the message from input to output or from output to input
	virtual void	invert()
	{
		if ( isReading() )
		{
			// In->Out: We want to write (serialize out) what we have read (serialized in)
			resetPtrTable();
			setInOut( false );
			_BufPos = _Buffer.getPtr()+_Buffer.size();
		}
		else
		{
			// Out->In: We want to read (serialize in) what we have written (serialized out)
			resetPtrTable();
			setInOut( true );
			_Buffer.resize (_BufPos - _Buffer.getPtr());
			_BufPos = _Buffer.getPtr();
		}
	}

	/// Force to reset the ptr table
	void			resetPtrTable() { IStream::resetPtrTable() ; }

	/// Increase the buffer size if 'len' can't enter, otherwise, do nothing
#ifdef NL_OS_WINDOWS
	__forceinline
#endif
	void			increaseBufferIfNecessary(uint32 len)
	{
		uint32 oldBufferSize = _Buffer.size();
		if (_BufPos - _Buffer.getPtr() + len > oldBufferSize)
		{
			// need to increase the buffer size
			uint32 pos = _BufPos - _Buffer.getPtr();
			_Buffer.resize(oldBufferSize*2 + len);
			_BufPos = _Buffer.getPtr() + pos;
		}
	}


	template <class T> void fastSerial (T &val)
	{
#ifdef NL_LITTLE_ENDIAN
		if(isReading())
		{
			// Check that we don't read more than there is to read
			// TODO OPTIM we can remove the check if we want to be faster (50ms->43ms for 1 million serial)
			if ( lengthS()+sizeof(T) > lengthR() )
				throw EStreamOverflow();
			// Serialize in
			val = *(T*)_BufPos;
		}
		else
		{
			increaseBufferIfNecessary (sizeof(T));
			*(T*)_BufPos = val;
		}
		_BufPos += sizeof (T);
#else // NL_LITTLE_ENDIAN
		IStream::serial( val );
#endif // NL_LITTLE_ENDIAN
	}

	template <class T>
	void			fastWrite( T& value )
	{
		//nldebug( "MEMSTREAM: Writing %u bits value in %p at pos %u", sizeof(value), this, _BufPos - _Buffer.getPtr() );
		increaseBufferIfNecessary (sizeof(T));
		*(T*)_BufPos = value;
		_BufPos += sizeof (T);
	}

	template <class T>
	void			fastRead( T& value )
	{
		//nldebug( "MEMSTREAM: Reading %u bits value in %p at pos %u", sizeof(value), this, _BufPos - _Buffer.getPtr() );
		// Check that we don't read more than there is to read
		if ( lengthS()+sizeof(value) > lengthR() )
		{
			throw EStreamOverflow();
		}
		// Serialize in
		value = *(T*)_BufPos;
		_BufPos += sizeof(value);
	}


	/// Template serialisation (should take the one from IStream)
    template<class T>
	void			serial(T &obj)							{ obj.serial(*this); }

	template<class T>
	void			serialCont(std::vector<T> &cont) 		{IStream::serialCont(cont);}
	template<class T>
	void			serialCont(std::list<T> &cont) 			{IStream::serialCont(cont);}
	template<class T>
	void			serialCont(std::deque<T> &cont) 		{IStream::serialCont(cont);}
	template<class T>
	void			serialCont(std::set<T> &cont) 			{IStream::serialCont(cont);}
	template<class T>
	void			serialCont(std::multiset<T> &cont) 		{IStream::serialCont(cont);}
	template<class K, class T>
	void			serialCont(std::map<K, T> &cont) 		{IStream::serialCont(cont);}
	template<class K, class T>
	void			serialCont(std::multimap<K, T> &cont) 	{IStream::serialCont(cont);}

	/// Specialisation of serialCont() for vector<uint8>
	virtual void			serialCont(std::vector<uint8> &cont) {IStream::serialCont(cont);} 
	/// Specialisation of serialCont() for vector<sint8>
	virtual void			serialCont(std::vector<sint8> &cont) {IStream::serialCont(cont);} 
	/// Specialisation of serialCont() for vector<bool>
	virtual void			serialCont(std::vector<bool> &cont) {IStream::serialCont(cont);} 



	template<class T0,class T1>
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
	{ serial(a); serial(b); serial(c); serial(d); serial(e); serial(f);}

	/** \name Base types serialisation, redefined for string mode
	 * Those method are a specialisation of template method "void serial(T&)".
	 */
	//@{
	virtual void	serial(uint8 &b) ;
	virtual void	serial(sint8 &b) ;
	virtual void	serial(uint16 &b) ;
	virtual void	serial(sint16 &b) ;
	virtual void	serial(uint32 &b) ;
	virtual void	serial(sint32 &b) ;
	virtual void	serial(uint64 &b) ;
	virtual void	serial(sint64 &b) ;
	virtual void	serial(float &b) ;
	virtual void	serial(double &b) ;
	virtual void	serial(bool &b) ;
#ifndef NL_OS_CYGWIN
	virtual void	serial(char &b) ;
#endif
	virtual void	serial(std::string &b) ;
	virtual void	serial(ucstring &b) ;
	//@}


	///\name String-specific methods
	//@{

	/// Input: read len bytes at most from the stream until the next separator, and return the number of bytes read. The separator is then skipped.
	uint			serialSeparatedBufferIn( uint8 *buf, uint len );

	/// Output: writes len bytes from buf into the stream
	void			serialSeparatedBufferOut( uint8 *buf, uint len );

	/// Serialisation in hexadecimal
	virtual void	serialHex(uint32 &b);

	//@}

protected:

	/// Returns the serialized length (number of bytes written or read)
	uint32			lengthS() const
	{
		return _BufPos-_Buffer.getPtr();
	}

	/// Returns the "read" message size (number of bytes to read)
	uint32			lengthR() const
	{
//		return _BufPos-_Buffer.getPtr();
		return _Buffer.size();
	}

	CObjectVector<uint8, false> _Buffer;
	uint8 *_BufPos;
	
	//CVector8		_Buffer;
	//It8				_BufPos;
	bool			_StringMode;

	uint32			_DefaultCapacity;
};

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

/*
 * atoihex
 */
inline int atoihex( const char* ident )
{
	int number;
	sscanf( ident, "%x", &number );
	return number;
}

inline uint32 atoui( const char *ident)
{
	return (uint32) strtoul (ident, NULL, 10);
}

const char SEPARATOR = ' ';
const int SEP_SIZE = 1; // the code is easier to read with that

//
// inline serial functions
//


// ======================================================================================================
inline	void		CMemStream::serial(uint8 &b) 
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
		fastSerial (b);
	}
}

// ======================================================================================================
inline	void		CMemStream::serial(sint8 &b) 
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
		fastSerial (b);
	}
}

// ======================================================================================================
inline	void		CMemStream::serial(uint16 &b) 
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
		fastSerial (b);
	}
}

// ======================================================================================================
inline	void		CMemStream::serial(sint16 &b) 
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
		fastSerial (b);
	}
}

// ======================================================================================================
inline	void		CMemStream::serial(uint32 &b) 
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
		fastSerial (b);
	}
}


// ======================================================================================================
inline	void		CMemStream::serial(sint32 &b) 
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
		fastSerial (b);
	}
}

// ======================================================================================================
inline	void		CMemStream::serial(uint64 &b) 
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
		fastSerial (b);
	}
}

// ======================================================================================================
inline	void		CMemStream::serial(sint64 &b) 
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
		fastSerial (b);
	}
}

// ======================================================================================================
inline	void		CMemStream::serial(float &b) 
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
		fastSerial (b);
	}
}

// ======================================================================================================
inline	void		CMemStream::serial(double &b) 
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
		fastSerial (b);
	}
}

// ======================================================================================================
inline	void		CMemStream::serial(bool &b) 
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
			b = (*(uint8*)_BufPos) == 1;
			_BufPos += sizeof(uint8);
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
inline	void		CMemStream::serial(char &b) 
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
		fastSerial (b);
	}
}
#endif

// ======================================================================================================
inline	void		CMemStream::serial(std::string &b) 
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
inline	void		CMemStream::serial(ucstring &b) 
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
/*inline	void	CMemStream::serialCont(std::vector<bool> &cont)
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
inline	void	CMemStream::serialHex(uint32 &b)
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








}

#endif // NL_MEM_STREAM_H

/* End of mem_stream.h */
