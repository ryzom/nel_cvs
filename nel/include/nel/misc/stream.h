
#ifndef STREAM_H
#define STREAM_H

// ============================================================================
class BaseStream
{
public:

	enum	Type
	{
		In, Out
	};

	BaseStream(Type t) : t(t)

	// ou une fonction open....
	bool			isReading();
	bool			isWriting();


	// template Object serialisation.
	template<class T>
	void			serialize(T &obj)
	{
		obj.serialize(*this);
	}

	// Base type serialisation.
	void			serialize(uint8 &b) throw(EStream);
	void			serialize(sint8 &b) throw(EStream);
	void			serialize(uint16 &b) throw(EStream);
	void			serialize(sint16 &b) throw(EStream);
	void			serialize(uint32 &b) throw(EStream);
	void			serialize(sint32 &b) throw(EStream);
	void			serialize(uint64 &b) throw(EStream);
	void			serialize(sint64 &b) throw(EStream);
	void			serialize(float &b) throw(EStream);
	void			serialize(double &b) throw(EStream);
	void			serialize(bool &b) throw(EStream);
	void			serialize(char &b) throw(EStream);
	void			serialize(string &b) throw(EStream);
	void			serialize(wchar &b) throw(EStream);
	void			serialize(wstring &b) throw(EStream);

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
	virtual void		write(const uint8 *buf, uint len) throw(EStream) =0;
	virtual void		read(uint8 *buf, uint len) throw(EStream) =0;
	virtual void		write(const bool &bit) throw(EStream) =0;
	virtual void		read(bool &bit) throw(EStream) =0;
};


// ============================================================================
class	Serializable
{
public:
	virtual void serialize (BaseStream& buff) = 0;
};

class IStream : virtual public BaseStream
{
}

class OStream : virtual public BaseStream
{
}

class LStream : virtual public OStream
{
}

#endif // STREAM_H
