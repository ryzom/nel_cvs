/* stream.h
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: stream.h,v 1.10 2000/09/14 10:03:56 berenguier Exp $
 *
 * This File handles IStream 
 */

#ifndef NL_STREAM_H
#define NL_STREAM_H


#include	"nel/misc/types_nl.h"
#include	"nel/misc/class_registry.h"
#include	<utility>
#include	<string>
#include	<map>


namespace	NLMISC
{


// ======================================================================================================
// ======================================================================================================
// Stream System.
// ======================================================================================================
// ======================================================================================================


// ======================================================================================================
/**
 * Stream Exception.
 * \author Lionel Berenguier
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
struct EStream : public Exception
{
	virtual const char	*what() const throw() {return "Stream Error";}
};
struct EOlderStream : public EStream
{
	virtual const char	*what() const throw() {return "The version in stream is older than the class";}
};
struct ENewerStream : public EStream
{
	virtual const char	*what() const throw() {return "The version in stream is newer than the class";}
};


class	IStreamable;

// ======================================================================================================
/**
 * A IO stream interface.
 * This is the base interface for stream objects. Differents kind of streams may be implemented,
 * by specifying serialBuffer() methods.
 *
 * \b Deriver \b Use:
 *
 * The deriver must:
 * - construct object specifying his type, see IStream(). A stream may be setup Input or Output at construction, but cannot
 * change during his life.
 * - specify serialBuffer(), to save or load pack of bytes.
 * - specify serialBit(), to save or load a bit.
 * - call resetPtrTable() when the stream reset itself (e.g.: CIFile::close() )
 *
 * Sample of streams: COutMemoryStream, CInFileStream ...
 *
 * \b Client \b Use:
 *
 * An object which can be serialized, must provide a "void serial(IStream &)" method. In this method, he can use
 * any of the IStream method to help himself like:
 * - serial() with a base type (uint32, string, char...), or even with an object which provide "void serial(IStream &)"
 * - template serial(T0&, T1&, ...) to serialize multiple object/variables in one call (up to 6).
 * - serialCont() and serialMap() to serialize containers.
 * - serialVersion() to check/store a version number of his class.
 * - serialPtr() to use the ptr support of IStream (see serialPtr() for more information)
 * - isReading() to know if he write in the stream, or if he read.
 *
 * The using is very simple as shown in this example:
 *
 * \code
 class A
 {
 public:
	float	x;
	uint32	y;
	Class1	a;		// this class must provide a serial() method too...
	Base	*c,*d;	// Base must derive from IStreamable
	vector<Class2>	tab;

 public:
	void	serial(IStream &f)
	{
		sint	streamver= serialVersion(3);
		serial(x,y,a);
		serialPtr(c);
		if(streamver>=2)
			serialPtr(d);
	}
 };
 \endcode
 * \author Lionel Berenguier
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
class IStream
{
public:
	/**
	 * Set the behavior of IStream regarding input stream that are older/newer than the class. 
	 * If throwOnOlder==true, IStream throws a EOlderStream when needed. 
	 * If throwOnNewer==true, IStream throws a ENewerStream when needed. 
	 *
	 * By default, the behavior is throwOnOlder=false, throwOnNewer=true.
	 * \see serialVersion() getVersionException()
	 */
	static	void	setVersionException(bool throwOnOlder, bool throwOnNewer);
	/**
	 * Get the behavior of IStream regarding input stream that are older/newer than the class. 
	 * \see serialVersion() setVersionException()
	 */
	static	void	getVersionException(bool &throwOnOlder, bool &throwOnNewer);


public:

	/**
	 * Constructor.
	 * You must set needSwap only if your stream need it (a CMemoryStream may not need it).
	 * IStream::IStream() force needSwap=false if \c NL_BIG_ENDIAN defined!
	 * Notice that those behavior can be set at construction only.
	 * \param inputStream is the stream an Input (read) stream?
	 * \param needSwap is the stream need endian swapping?
	 */
	IStream(bool inputStream, bool needSwap);

	/// Destructor.
	virtual ~IStream() {}

	/// Is this stream a Read/Input stream?
	bool			isReading() const;


	/**
	 * Template Object serialisation.
	 * \param obj any object providing a "void serial(IStream&)" method.
	 */
    template<class T>
	void			serial(T &obj) throw(EStream)
	{ obj.serial(*this);	}


	/** \name Base type serialisation.
	 * Those method are a specialisation of template method "void serial(T&)".
	 */
	//@{
	void			serial(uint8 &b) throw(EStream);
	void			serial(sint8 &b) throw(EStream);
	void			serial(uint16 &b) throw(EStream);
	void			serial(sint16 &b) throw(EStream);
	void			serial(uint32 &b) throw(EStream);
	void			serial(sint32 &b) throw(EStream);
	void			serial(uint64 &b) throw(EStream);
	void			serial(sint64 &b) throw(EStream);
	void			serial(float &b) throw(EStream);
	void			serial(double &b) throw(EStream);
	void			serial(bool &b) throw(EStream);
	void			serial(char &b) throw(EStream);
	void			serial(std::string &b) throw(EStream);
	void			serial(wchar &b) throw(EStream);
	void			serial(std::wstring &b) throw(EStream);
	//@}

	/** \name Multiple serialisation.
	 * Template for easy multiple serialisation.
	 */
	//@{
	template<class T0,class T1>
	void			serial(T0 &a, T1 &b) throw(EStream)
	{ serial(a); serial(b);}
	template<class T0,class T1,class T2>
	void			serial(T0 &a, T1 &b, T2 &c) throw(EStream)
	{ serial(a); serial(b); serial(c);}
	template<class T0,class T1,class T2,class T3>
	void			serial(T0 &a, T1 &b, T2 &c, T3 &d) throw(EStream)
	{ serial(a); serial(b); serial(c); serial(d);}
	template<class T0,class T1,class T2,class T3,class T4>
	void			serial(T0 &a, T1 &b, T2 &c, T3 &d, T4 &e) throw(EStream)
	{ serial(a); serial(b); serial(c); serial(d); serial(e);}
	template<class T0,class T1,class T2,class T3,class T4,class T5>
	void			serial(T0 &a, T1 &b, T2 &c, T3 &d, T4 &e, T5 &f) throw(EStream)
	{ serial(a); serial(b); serial(c); serial(d); serial(e); serial(f);}
	//@}


	/**
	 * standard STL containers serialisation. Don't work with map<> and multimap<>.
	 * Support up to sint32 length containers.
	 *
	 * the object T must provide:
	 *	\li typedef iterator;		(providing operator++() and operator*())
	 *	\li typedef value_type;		(a base type (uint...), or an object providing "void serial(IStream&)" method.)
	 *	\li void clear();
	 *	\li size_type size() const;
	 *	\li iterator begin();
	 *	\li iterator end();
	 *	\li iterator insert(iterator it, const value_type& x);
	 *
	 * Known Supported containers: vector<>, list<>, deque<>, set<>, multiset<>.
	 * \param cont a STL container (vector<>, set<> ...).
	 */
	template<class T>
	void			serialCont(T &cont) throw(EStream)
	{
		sint32	len;
		if(isReading())
		{
			cont.clear();
			serial(len);
			for(sint i=0;i<len;i++)
			{
				T::value_type	v;
				serial(v);
				cont.insert(cont.end(), v);
			}
		}
		else
		{
			len= cont.size();
			serial(len);
			T::iterator		it= cont.begin();
			for(sint i=0;i<len;i++, it++)
			{
				serial((*it));
			}
		}
	}


	/**
	 * STL map<> and multimap<> serialisation.
	 * Support up to sint32 length containers.
	 *
	 * the object T must provide:
	 *	\li typedef iterator;		(providing operator++() and operator*())
	 *	\li typedef value_type;		(must be a std::pair<>)
	 *	\li typedef key_type;		(must be the type of the key)
	 *	\li void clear();
	 *	\li size_type size() const;
	 *	\li iterator begin();
	 *	\li iterator end();
	 *	\li iterator insert(iterator it, const value_type& x);
	 *
	 * Known Supported containers: map<>, multimap<>.
	 * \param cont a STL map<> or multimap<> container.
	 */
	template<class T>
	void			serialMap(T &cont) throw(EStream)
	{
		sint32	len;
		if(isReading())
		{
			cont.clear();
			serial(len);
			for(sint i=0;i<len;i++)
			{
				T::value_type	v;
				serial( const_cast<T::key_type&>(v.first) );
				serial(v.second);
				cont.insert(cont.end(), v);
			}
		}
		else
		{
			len= cont.size();
			serial(len);
			T::iterator		it= cont.begin();
			for(sint i=0;i<len;i++, it++)
			{
				serial( const_cast<T::key_type&>((*it).first) );
				serial((*it).second);
			}
		}
	}


	/** 
	 * Serialize Polymorphic Objet Ptr.
	 * Works with NULL pointers. If the same object is found mutliple time in the stream, ONLY ONE instance is written!
	 * NB: The ptr is serialised as a uint64 (64 bit compliant).
	 * \param ptr a pointer on a IStreamable object.
	 * \see resetPtrTable()
	 */
	template<class T>
	void			serialPtr(T* &ptr) throw(ERegistry, EStream)
	{ IStreamable *p=ptr; serialIStreamable(p); ptr= static_cast<T*>(p);}


	/** 
	 * Serialize a version number.
	 * Each object should store/read first a version number, using this method.
	 * Then he can use the streamVersion returned to see how he should serialise himself.
	 *
	 * NB: Version Number is read/store as a uint8, or uint32 if too bigger..
	 * \param currentVersion the current version of the class, provided by user.
	 * \return the version of the stream. If the stream is an Output stream, currentVersion is returned.
	 * \see setVersionException() getVersionException()
	 */
	uint			serialVersion(uint currentVersion) throw(EStream);


protected:

	/** 
	 * for Deriver: reset the PtrTable in the stream.
	 * If Derived stream provide reset()-like methods, they must call this method in their reset() methods.
	 * For example, CFile::close() must call it, so it will work correctly with next serialPtr()
	 */
	void				resetPtrTable();

	//@{
	/// Method to be specified by the Deriver.
	virtual void		serialBuffer(uint8 *buf, uint len) throw(EStream)=0;
	virtual void		serialBit(bool &bit) throw(EStream)=0;
	//@}

private:
	bool	_InputStream;
	bool	_NeedSwap;
	static	bool	_ThrowOnOlder;
	static	bool	_ThrowOnNewer;

	// Ptr registry. We store 64 bit Id, to be compatible with futur 64+ bits pointers.
	std::map<uint64, IStreamable*>				_IdMap;
	typedef std::map<uint64, IStreamable*>::iterator	ItIdMap;
	typedef std::map<uint64, IStreamable*>::value_type	ValueIdMap;

	// Ptr serialisation.
	void			serialIStreamable(IStreamable* &ptr) throw(ERegistry, EStream);
};


// ======================================================================================================
// ======================================================================================================
// Handle for streaming Polymorphic classes.
// ======================================================================================================
// ======================================================================================================


// ======================================================================================================
/**
 * An Object Streamable interface.
 * \author Lionel Berenguier
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
class IStreamable : public IClassable
{
public:
	virtual void		serial(IStream	&f) throw(EStream)=0;
};


}	// namespace NLMISC.


// Inline Implementation.
#include "nel/misc/stream_inline.h"


#endif // NL_STREAM_H

/* End of stream.h */
