
#ifndef NL_STREAM_H
#define NL_STREAM_H

#include "nel/misc/types_nl.h"

// If little endian, we have to swap
#if BIG_ENDIAN
#  define BSWAP16(src,dest)
#  define BSWAP32(src,dest)
#  define BSWAP64(src,dest)
#else
#  define BSWAP16(src,dest)	(dest) = (((src)>>8)&0xFF) | (((src)&0xFF)<<8)
#  ifdef OS_WINDOWS
#    define BSWAP32(src,dest) _asm mov eax,(src) _asm bswap eax _asm mov (dest),eax
#  else
#    define BSWAP32(src,dest) (dest) = (((src)>>24)&0xFF) | ((((src)>>16)&0xFF)<<8) | ((((src)>>8)&0xFF)<<16) | (((src)&0xFF)<<24)
#  endif
#  define BSWAP64(src,dest) (dest) = (((src)>>56)&0xFF) | ((((src)>>48)&0xFF)<<8) | ((((src)>>40)&0xFF)<<16) | ((((src)>>32)&0xFF)<<24) | ((((src)>>24)&0xFF)<<32) | ((((src)>>16)&0xFF)<<40) | ((((src)>>8)&0xFF)<<48) | (((src)&0xFF)<<56)
#endif

class EStream : virtual public Exception
{
};

// ============================================================================
class CBaseStream
{
public:

	enum	Type
	{
		In, Out
	};

	CBaseStream(Type type, bool needSwap)
	{
		_Type = type;
		_needSwap = needSwap;
	}

	// ou une fonction open....
	bool			isReading();
	bool			isWriting();


	// template Object serialisation.
	template<class T>
	void			serialize(T &obj)
	{
		obj.serialize(*this);
	}
/*
	void			serialize(uint &b) throw(EStream)
	{
		uint8	tab[4];
		if(isReading())
		{
			read(tab, 4);
			b = tab[0]<<24 | tab[1]<<16 | tab[2]<<8 | tab[3];
		}
		else
		{
			tab[0] = b>>24; tab[1]=b>>16; tab[2]=b>>8; tab[3]=b;
			write(tab, 4);
		}
	}
*/

	// Base type serialisation.
	void			serialize(uint8 &b) throw(EStream)
	{
		write ((uint8 *)&b, 1);
	}

	void			serialize(sint8 &b) throw(EStream)
	{
		write ((uint8 *)&b, 1);
	}

	void			serialize(uint16 &b) throw(EStream)
	{
		if (_needSwap)
		{
			uint16 v;
			BSWAP16(b,v);
			write ((uint8 *)&v, 2);
		}
		else
		{
			write ((uint8 *)&b, 2);
		}
	}

	void			serialize(sint16 &b) throw(EStream)
	{
		if (_needSwap)
		{
			uint16 v;
			BSWAP16(b,v);
			write ((uint8 *)&v, 2);
		}
		else
		{
			write ((uint8 *)&b, 2);
		}
	}

	void			serialize(uint32 &b) throw(EStream)
	{
		if (_needSwap)
		{
			uint32 v;
			BSWAP32(b,v);
			write ((uint8 *)&v, 4);
		}
		else
		{
			write ((uint8 *)&b, 4);
		}
	}

	void			serialize(sint32 &b) throw(EStream)
	{
		if (_needSwap)
		{
			uint32 v;
			BSWAP32(b,v);
			write ((uint8 *)&v, 4);
		}
		else
		{
			write ((uint8 *)&b, 4);
		}
	}

	void			serialize(uint64 &b) throw(EStream)
	{
		if (_needSwap)
		{
			uint64 v;
			BSWAP64(b,v);
			write ((uint8 *)&v, 8);
		}
		else
		{
			write ((uint8 *)&b, 8);
		}
	}

	void			serialize(sint64 &b) throw(EStream)
	{
		if (_needSwap)
		{
			uint64 v;
			BSWAP64(b,v);
			write ((uint8 *)&v, 8);
		}
		else
		{
			write ((uint8 *)&b, 8);
		}
	}

	void			serialize(float &b) throw(EStream)
	{
		if (_needSwap)
		{
			uint32 v;
			uint32 *b2 = (uint32 *)&b;
			BSWAP32(*b2,v);
			write ((uint8 *)&v, 4);
		}
		else
		{
			write ((uint8 *)&b, 4);
		}
	}

	void			serialize(double &b) throw(EStream)
	{
		if (_needSwap)
		{
			uint64 v;
			uint64 *b2 = (uint64 *)&b;
			BSWAP64(*b2,v);
			write ((uint8 *)&v, 8);
		}
		else
		{
			write ((uint8 *)&b, 8);
		}
	}

	void			serialize(bool &b) throw(EStream)
	{
		uint8 b2 = (b)?1:0;
		write (&b2, 1);
	}

	void			serialize(char &b) throw(EStream)
	{
		write ((uint8 *)&b, 1);
	}

//	void			serialize(string &b) throw(EStream);
//	void			serialize(wchar &b) throw(EStream);
//	void			serialize(wstring &b) throw(EStream);

	// Multiple template serialisation.
	template<class T0,class T1>
	void			serialize(T0 &a, T1 &b) throw(EStream)
	{
		serialize(a);
		serialize(b);
	}
	template<class T0,class T1,class T2>
	void			serialize(T0 &a, T1 &b, T2 &c) throw(EStream)
	{
		serialize(a);
		serialize(b);
		serialize(c);
	}
	template<class T0,class T1,class T2,class T3>
	void			serialize(T0 &a, T1 &b, T2 &c, T3 &d) throw(EStream)
	{
		serialize(a);
		serialize(b);
		serialize(c);
		serialize(d);
	}

protected:

	// Methodes à spécifier.
	virtual void		reset()=0;

	virtual void		write(const uint8 *buf, uint len) throw(EStream)=0;
	virtual void		write(const bool &bit) throw(EStream)=0;

	virtual void		read(uint8 *buf, uint len) throw(EStream)=0;
	virtual void		read(bool &bit) throw(EStream)=0;

private:
	Type _Type;
	bool _needSwap;
};


// ============================================================================
class	CSerializable
{
public:
	virtual void serialize (CBaseStream& buff) = 0;
};

class CIStream : virtual public CBaseStream
{
};

class COStream : virtual public CBaseStream
{
};

class CLStream : virtual public COStream
{
};

#endif // NL_STREAM_H
