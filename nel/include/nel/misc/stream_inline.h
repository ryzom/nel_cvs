/* stream_inline.h
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: stream_inline.h,v 1.10 2000/10/06 10:27:36 lecroart Exp $
 *
 * This File Declares inline for stream.h CRegistry and CBaseStream 
 */

#ifndef NL_STREAM_INLINE_H
#define NL_STREAM_INLINE_H

#include "nel/misc/assert.h"


namespace	NLMISC
{


// ======================================================================================================
// ======================================================================================================
// IBasicStream Inline Implementation.
// ======================================================================================================
// ======================================================================================================


// ======================================================================================================
// ======================================================================================================
// ======================================================================================================

// ======================================================================================================
inline	IStream::IStream(bool inputStream, bool needSwap)
{
	_InputStream= inputStream;
	_NeedSwap= needSwap;
#ifdef NL_BIG_ENDIAN
	_NeedSwap=false;
#endif
}


// ======================================================================================================
inline	bool		IStream::isReading() const
{
	return _InputStream;
}


// ======================================================================================================
// ======================================================================================================
// ======================================================================================================

// ======================================================================================================
inline	void		IStream::serial(uint8 &b) throw(EStream)
{
	serialBuffer((uint8 *)&b, 1);
}

// ======================================================================================================
inline	void		IStream::serial(sint8 &b) throw(EStream)
{
	serialBuffer((uint8 *)&b, 1);
}

// ======================================================================================================
inline	void		IStream::serial(uint16 &b) throw(EStream)
{
	if(!_NeedSwap)
	{
		serialBuffer((uint8 *)&b, 2);
	}
	else
	{
		uint16	v;
		if(isReading())
		{
			serialBuffer((uint8 *)&v, 2);
			NLMISC_BSWAP16(v);
			b=v;
		}
		else
		{
			v=b;
			NLMISC_BSWAP16(v);
			serialBuffer((uint8 *)&v, 2);
		}
	}
}

// ======================================================================================================
inline	void		IStream::serial(sint16 &b) throw(EStream)
{
	if(!_NeedSwap)
	{
		serialBuffer((uint8 *)&b, 2);
	}
	else
	{
		uint16	v;
		if(isReading())
		{
			serialBuffer((uint8 *)&v, 2);
			NLMISC_BSWAP16(v);
			b=v;
		}
		else
		{
			v=b;
			NLMISC_BSWAP16(v);
			serialBuffer((uint8 *)&v, 2);
		}
	}
}

// ======================================================================================================
inline	void		IStream::serial(uint32 &b) throw(EStream)
{
	if(!_NeedSwap)
	{
		serialBuffer((uint8 *)&b, 4);
	}
	else
	{
		uint32	v;
		if(isReading())
		{
			serialBuffer((uint8 *)&v, 4);
			NLMISC_BSWAP32(v);
			b=v;
		}
		else
		{
			v=b;
			NLMISC_BSWAP32(v);
			serialBuffer((uint8 *)&v, 4);
		}
	}
}

// ======================================================================================================
inline	void		IStream::serial(sint32 &b) throw(EStream)
{
	if(!_NeedSwap)
	{
		serialBuffer((uint8 *)&b, 4);
	}
	else
	{
		uint32	v;
		if(isReading())
		{
			serialBuffer((uint8 *)&v, 4);
			NLMISC_BSWAP32(v);
			b=v;
		}
		else
		{
			v=b;
			NLMISC_BSWAP32(v);
			serialBuffer((uint8 *)&v, 4);
		}
	}
}

// ======================================================================================================
inline	void		IStream::serial(uint64 &b) throw(EStream)
{
	if(!_NeedSwap)
	{
		serialBuffer((uint8 *)&b, 8);
	}
	else
	{
		uint64	v;
		if(isReading())
		{
			serialBuffer((uint8 *)&v, 8);
			NLMISC_BSWAP64(v);
			b=v;
		}
		else
		{
			v=b;
			NLMISC_BSWAP64(v);
			serialBuffer((uint8 *)&v, 8);
		}
	}
}

// ======================================================================================================
inline	void		IStream::serial(sint64 &b) throw(EStream)
{
	if(!_NeedSwap)
	{
		serialBuffer((uint8 *)&b, 8);
	}
	else
	{
		uint64	v;
		if(isReading())
		{
			serialBuffer((uint8 *)&v, 8);
			NLMISC_BSWAP64(v);
			b=v;
		}
		else
		{
			v=b;
			NLMISC_BSWAP64(v);
			serialBuffer((uint8 *)&v, 8);
		}
	}
}

// ======================================================================================================
inline	void		IStream::serial(float &b) throw(EStream)
{
	if(!_NeedSwap)
	{
		serialBuffer((uint8 *)&b, 4);
	}
	else
	{
		uint32	v;
		if(isReading())
		{
			serialBuffer((uint8 *)&v, 4);
			NLMISC_BSWAP32(v);
			b=*((float*)&v);
		}
		else
		{
			v=*((uint32*)&b);
			NLMISC_BSWAP32(v);
			serialBuffer((uint8 *)&v, 4);
		}
	}
}

// ======================================================================================================
inline	void		IStream::serial(double &b) throw(EStream)
{
	if(!_NeedSwap)
	{
		serialBuffer((uint8 *)&b, 8);
	}
	else
	{
		uint64	v;
		if(isReading())
		{
			serialBuffer((uint8 *)&v, 8);
			NLMISC_BSWAP64(v);
			b=*((double*)&v);
		}
		else
		{
			v=*((uint64*)&b);
			NLMISC_BSWAP64(v);
			serialBuffer((uint8 *)&v, 8);
		}
	}
}

// ======================================================================================================
inline	void		IStream::serial(bool &b) throw(EStream)
{
	serialBit(b);
}

// ======================================================================================================
inline	void		IStream::serial(char &b) throw(EStream)
{
	serialBuffer((uint8 *)&b, 1);
}

// ======================================================================================================
inline	void		IStream::serial(std::string &b) throw(EStream)
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
		serial(b[i]);
}

/* ACE: already defined in uint16
// ======================================================================================================
inline	void		IStream::serial(wchar &b) throw(EStream)
{
	uint16	v;
	if(isReading())
	{
		serial(v);
		b=*((wchar*)&v);
	}
	else
	{
		v=*((uint16*)&b);
		serial(v);
	}
}
*/

/* ACE: already defined in uint16
// ======================================================================================================
inline	void		IStream::serial(std::wstring &b) throw(EStream)
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
		serial(b[i]);
}
*/

}


#endif // NL_STREAM_INLINE_H

/* End of stream_inline.h */
