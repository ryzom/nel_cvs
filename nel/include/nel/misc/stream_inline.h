/* stream_inline.h
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: stream_inline.h,v 1.2 2000/09/12 08:15:38 berenguier Exp $
 *
 * This File Declares inline for stream.h CRegistry and CBaseStream 
 */

#ifndef NL_STREAM_INLINE_H
#define NL_STREAM_INLINE_H

#include "nel/misc/assert.h"



// ======================================================================================================
// ======================================================================================================
// IBasicStream Inline Implementation.
// ======================================================================================================
// ======================================================================================================


// For Big/little Endian.
// NL_BIG_ENDIAN
#  define BSWAP16(src)	(src) = (((src)>>8)&0xFF) | (((src)&0xFF)<<8)
#  ifdef OS_WINDOWS
#    define BSWAP32(src) _asm mov eax,(src) _asm bswap eax _asm mov (src),eax
#  else
#    define BSWAP32(src) (src) = (((src)>>24)&0xFF) | ((((src)>>16)&0xFF)<<8) | ((((src)>>8)&0xFF)<<16) | (((src)&0xFF)<<24)
#  endif
#  define BSWAP64(src) (src) = (((src)>>56)&0xFF) | ((((src)>>48)&0xFF)<<8) | ((((src)>>40)&0xFF)<<16) | ((((src)>>32)&0xFF)<<24) | ((((src)>>24)&0xFF)<<32) | ((((src)>>16)&0xFF)<<40) | ((((src)>>8)&0xFF)<<48) | (((src)&0xFF)<<56)


// ======================================================================================================
// ======================================================================================================
// ======================================================================================================

// ======================================================================================================
inline	IStream::IStream(bool inputStream, bool needSwap)
{
	_InputStream= inputStream;
	_NeedSwap= needSwap;
}


// ======================================================================================================
inline	bool		IStream::isReading()
{
	return _InputStream;
}

// ======================================================================================================
template<class T>
inline	void		IStream::serial(T &obj) throw(EStream)
{
	obj.serial(*this);
}


// ======================================================================================================
// ======================================================================================================
// ======================================================================================================

// ======================================================================================================
inline	void		IStream::serial(uint8 &b) throw(EStream)
{
	serial((uint8 *)&b, 1);
}

// ======================================================================================================
inline	void		IStream::serial(sint8 &b) throw(EStream)
{
	serial((uint8 *)&b, 1);
}

// ======================================================================================================
inline	void		IStream::serial(uint16 &b) throw(EStream)
{
	if(!_NeedSwap)
	{
		serial((uint8 *)&b, 2);
	}
	else
	{
		uint16	v;
		if(isReading())
		{
			serial ((uint8 *)&v, 2);
			BSWAP16(v);
			b=v;
		}
		else
		{
			v=b;
			BSWAP16(v);
			serial((uint8 *)&v, 2);
		}
	}
}

// ======================================================================================================
inline	void		IStream::serial(sint16 &b) throw(EStream)
{
	if(!_NeedSwap)
	{
		serial((uint8 *)&b, 2);
	}
	else
	{
		uint16	v;
		if(isReading())
		{
			serial ((uint8 *)&v, 2);
			BSWAP16(v);
			b=v;
		}
		else
		{
			v=b;
			BSWAP16(v);
			serial((uint8 *)&v, 2);
		}
	}
}

// ======================================================================================================
inline	void		IStream::serial(uint32 &b) throw(EStream)
{
	if(!_NeedSwap)
	{
		serial((uint8 *)&b, 4);
	}
	else
	{
		uint32	v;
		if(isReading())
		{
			serial ((uint8 *)&v, 4);
			BSWAP32(v);
			b=v;
		}
		else
		{
			v=b;
			BSWAP32(v);
			serial((uint8 *)&v, 4);
		}
	}
}

// ======================================================================================================
inline	void		IStream::serial(sint32 &b) throw(EStream)
{
	if(!_NeedSwap)
	{
		serial((uint8 *)&b, 4);
	}
	else
	{
		uint32	v;
		if(isReading())
		{
			serial ((uint8 *)&v, 4);
			BSWAP32(v);
			b=v;
		}
		else
		{
			v=b;
			BSWAP32(v);
			serial((uint8 *)&v, 4);
		}
	}
}

// ======================================================================================================
inline	void		IStream::serial(uint64 &b) throw(EStream)
{
	if(!_NeedSwap)
	{
		serial((uint8 *)&b, 8);
	}
	else
	{
		uint64	v;
		if(isReading())
		{
			serial ((uint8 *)&v, 8);
			BSWAP64(v);
			b=v;
		}
		else
		{
			v=b;
			BSWAP64(v);
			serial((uint8 *)&v, 8);
		}
	}
}

// ======================================================================================================
inline	void		IStream::serial(sint64 &b) throw(EStream)
{
	if(!_NeedSwap)
	{
		serial((uint8 *)&b, 8);
	}
	else
	{
		uint64	v;
		if(isReading())
		{
			serial ((uint8 *)&v, 8);
			BSWAP64(v);
			b=v;
		}
		else
		{
			v=b;
			BSWAP64(v);
			serial((uint8 *)&v, 8);
		}
	}
}

// ======================================================================================================
inline	void		IStream::serial(float &b) throw(EStream)
{
	if(!_NeedSwap)
	{
		serial((uint8 *)&b, 4);
	}
	else
	{
		uint32	v;
		if(isReading())
		{
			serial ((uint8 *)&v, 4);
			BSWAP32(v);
			b=*((float*)&v);
		}
		else
		{
			v=*((uint32*)&b);
			BSWAP32(v);
			serial((uint8 *)&v, 4);
		}
	}
}

// ======================================================================================================
inline	void		IStream::serial(double &b) throw(EStream)
{
	if(!_NeedSwap)
	{
		serial((uint8 *)&b, 8);
	}
	else
	{
		uint64	v;
		if(isReading())
		{
			serial ((uint8 *)&v, 8);
			BSWAP64(v);
			b=*((double*)&v);
		}
		else
		{
			v=*((uint64*)&b);
			BSWAP64(v);
			serial((uint8 *)&v, 8);
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
	serial((uint8 *)&b, 8);
}

// ======================================================================================================
inline	void		IStream::serial(std::string &b) throw(EStream)
{
	uint32	len;
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

// ======================================================================================================
inline	void		IStream::serial(std::wstring &b) throw(EStream)
{
	uint32	len;
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


// ======================================================================================================
// ======================================================================================================
// ======================================================================================================


// ======================================================================================================
template<class T0,class T1>
inline	void			IStream::serial(T0 &a, T1 &b) throw(EStream)
{
	serial(a);
	serial(b);
}
// ======================================================================================================
template<class T0,class T1,class T2>
inline	void			IStream::serial(T0 &a, T1 &b, T2 &c) throw(EStream)
{
	serial(a);
	serial(b);
	serial(c);
}
// ======================================================================================================
template<class T0,class T1,class T2,class T3>
inline	void			IStream::serial(T0 &a, T1 &b, T2 &c, T3 &d) throw(EStream)
{
	serial(a);
	serial(b);
	serial(c);
	serial(d);
}
// ======================================================================================================
template<class T0,class T1,class T2,class T3,class T4>
inline	void			IStream::serial(T0 &a, T1 &b, T2 &c, T3 &d, T4 &e) throw(EStream)
{
	serial(a);
	serial(b);
	serial(c);
	serial(d);
	serial(e);
}
// ======================================================================================================
template<class T0,class T1,class T2,class T3,class T4,class T5>
inline	void			IStream::serial(T0 &a, T1 &b, T2 &c, T3 &d, T4 &e, T5 &f) throw(EStream)
{
	serial(a);
	serial(b);
	serial(c);
	serial(d);
	serial(e);
	serial(f);
}


// ======================================================================================================
template<class T0, class T1>
inline	void			IStream::serial(std::pair<T0, T1> &p) throw(EStream)
{
	serial(p.first);
	serial(p.second);
}


// ======================================================================================================
template<class T>
void			IStream::serialCont(T &cont) throw(EStream)
{
	uint32	len;
	uint64	len64;

	// Read the container.
	if(isReading())
	{
		// NB: we use a "uint loop" as possible.
		cont.clear();
		serial(len);
		if(len!=0xFFFFFFFF)
		{
			for(uint i=0;i<len;i++)
			{
				T::value_type	v;
				serial(v);
				cont.insert(cont.end(), v);
			}
		}
		else
		{
			serial(len64);
			for(uint64 i=0;i<len64;i++)
			{
				T::value_type	v;
				serial(v);
				cont.insert(cont.end(), v);
			}
		}
	}

	// Write the container.
	else
	{
		// NB: we use a "uint loop" as possible.
		len64= cont.size();
		if(len64<0xFFFFFFFF)
		{
			len= len64;
			serial(len);
			T::iterator		it= cont.begin();
			for(uint i=0;i<len;i++, it++)
			{
				serial((*it));
			}
		}
		else
		{
			len= 0xFFFFFFFF;
			serial(len);
			serial(len64);
			T::iterator		it= cont.begin();
			for(uint64 i=0;i<len64;i++, it++)
			{
				serial((*it));
			}
		}
	}
}


#endif // NL_STREAM_INLINE_H

/* End of stream_inline.h */
