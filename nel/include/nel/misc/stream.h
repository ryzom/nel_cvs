/** \file stream.h
 * serialization interface class
 *
 * $Id: stream.h,v 1.47 2001/09/10 13:21:47 berenguier Exp $
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

#ifndef NL_STREAM_H
#define NL_STREAM_H

#include	"nel/misc/types_nl.h"
#include	"nel/misc/ucstring.h"
#include	"nel/misc/class_registry.h"
#include	<utility>
#include	<string>
#include	<vector>
#include	<deque>
#include	<list>
#include	<set>
#include	<map>

namespace	NLMISC
{


class	IStream;


// ======================================================================================================
// ======================================================================================================
// Stream System.
// ======================================================================================================
// ======================================================================================================

// For Big/little Endian.
#  define NLMISC_BSWAP16(src)	(src) = (((src)>>8)&0xFF) | (((src)&0xFF)<<8)
#  ifdef NL_OS_WINDOWS
#    define NLMISC_BSWAP32(src) _asm mov eax,(src) _asm bswap eax _asm mov (src),eax
#  else
#    define NLMISC_BSWAP32(src) (src) = (((src)>>24)&0xFF) | ((((src)>>16)&0xFF)<<8) | ((((src)>>8)&0xFF)<<16) | (((src)&0xFF)<<24)
#  endif
#  define NLMISC_BSWAP64(src) (src) = (((src)>>56)&0xFF) | ((((src)>>48)&0xFF)<<8) | ((((src)>>40)&0xFF)<<16) | ((((src)>>32)&0xFF)<<24) | ((((src)>>24)&0xFF)<<32) | ((((src)>>16)&0xFF)<<40) | ((((src)>>8)&0xFF)<<48) | (((src)&0xFF)<<56)

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
	EStream() : Exception( "Stream Error" ) {}

	EStream( const std::string& str ) : Exception( str ) {}

	EStream( const IStream &f );

	EStream( const IStream &f, const std::string& str );

	// May Not be Filled...
	std::string	StreamName;
};

struct EOlderStream : public EStream
{
	EOlderStream() : EStream("The version in stream is older than the class" ) {}
	EOlderStream(const IStream &f) : EStream(f, "The version in stream is older than the class" ) {}
};

struct ENewerStream : public EStream
{
	ENewerStream() : EStream("The version in stream is newer than the class" ) {}
	ENewerStream(const IStream &f) : EStream(f, "The version in stream is newer than the class" ) {}
};

struct EInvalidDataStream : public EStream
{
	EInvalidDataStream() : EStream("Invalid data format" ) {}
	EInvalidDataStream(const IStream &f) : EStream(f, "Invalid data format" ) {}
};

struct ESeekNotSupported : public EStream
{
	ESeekNotSupported() : EStream("Seek fonctionnality is not supported" ) {}
	ESeekNotSupported(const IStream &f) : EStream(f, "Seek fonctionnality is not supported" ) {}
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
 * - serialCont() to serialize containers.
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
		sint	streamver= f.serialVersion(3);
		f.serial(x,y,a);
		f.serialPtr(c);
		f.serialCont(tab);
		if(streamver>=2)
			f.serialPtr(d);
	}
 };
 \endcode
 *
 * NB: \b YOU \b CANNOT use serial with a int / uint / sint type, since those type have unspecified length.
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
	 * IStream::IStream() force needSwap=false if \c NL_LITTLE_ENDIAN defined!
	 * Notice that those behavior can be set at construction only.
	 * \param inputStream is the stream an Input (read) stream?
	 * \param needSwap is the stream need endian swapping?
	 */
	IStream(bool inputStream, bool needSwap);

	/// Destructor.
	virtual ~IStream() {}

	/// Copy constructor
	IStream( const IStream& other );

	/// Assignment operator
	IStream&		operator=( const IStream& other );
	
	/// Is this stream a Read/Input stream?
	bool			isReading() const;


	/**
	 * Template Object serialisation.
	 * \param obj any object providing a "void serial(IStream&)" method. The object doesn't have to derive from IStreamable.
	 * 
	 * the VC++ error "error C2228: left of '.serial' must have class/struct/union type" means you don't provide
	 * a serial() method to your object. Or you may have use serial with a int / uint / sint type. REMEMBER YOU CANNOT
	 * do this, since those type have unspecified length.
	 */
    template<class T>
	void			serial(T &obj)  { obj.serial(*this); }


	/** \name Base type serialisation.
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


	/// Template enum serialisation. Serialized as a sint32.
    template<class T>
	void			serialEnum(T &em) 
	{
		sint32	i;
		if(isReading())
		{
			serial(i);
			em = (T)i;
		}
		else
		{
			i = em;
			serial(i);
		}
	}


	/** \name BitField serialisation.
	 * Unlike other serial method, The reading bitfield is returned!! If !this->isReading(), bf is returned.
	 *
	 * MUST use it simply like this:   a= serialBitFieldX(a);		// where X== 8, 16 or 32.
	 *
	 * NB: Performance warning: the data is stored as an uint8, uint16 or uint32, according to the method you use.
	 */
	//@{
	/// Serialisation of bitfield <=8 bits.
	uint8			serialBitField8(uint8  bf);
	/// Serialisation of bitfield <=16 bits.
	uint16			serialBitField16(uint16  bf);
	/// Serialisation of bitfield <=32 bits.
	uint32			serialBitField32(uint32  bf);
	//@}


	/** \name Multiple serialisation.
	 * Template for easy multiple serialisation.
	 */
	//@{
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
	//@}


	/** \name standard STL containers serialisation.
	 * Known Supported containers: vector<>, list<>, deque<>, set<>, multiset<>, map<>, multimap<>
	 * Support up to sint32 length containers.
	 * \see serialContPtr() serialContPolyPtr()
	 */
	template<class T>
	void			serialCont(std::vector<T> &cont) 	{serialVector(cont);}
	template<class T>
	void			serialCont(std::list<T> &cont) 	{serialSTLCont(cont);}
	template<class T>
	void			serialCont(std::deque<T> &cont) 	{serialSTLCont(cont);}
	template<class T>
	void			serialCont(std::set<T> &cont) 		{serialSTLCont(cont);}
	template<class T>
	void			serialCont(std::multiset<T> &cont) 	{serialSTLCont(cont);}
	template<class K, class T>
	void			serialCont(std::map<K, T> &cont) 			{serialMap(cont);}
	template<class K, class T>
	void			serialCont(std::multimap<K, T> &cont) 	{serialMap(cont);}


	/// Specialisation of serialCont() for vector<uint8>
	virtual void			serialCont(std::vector<uint8> &cont) ;
	/// Specialisation of serialCont() for vector<sint8>
	virtual void			serialCont(std::vector<sint8> &cont) ;
	/// Specialisation of serialCont() for vector<bool>
	virtual void			serialCont(std::vector<bool> &cont) ;


	/** \name standard STL containers serialisation. Elements must be pointers on a base type (uint...) or on a 
	 * object providing "void serial(IStream&)" method.
	 * Known Supported containers: vector<>, list<>, deque<>, set<>, multiset<>
	 * Support up to sint32 length containers.
	 * \see serialCont() serialContPolyPtr()
	 */
	template<class T>
	void			serialContPtr(std::vector<T> &cont) 	{serialVectorPtr(cont);}
	template<class T>
	void			serialContPtr(std::list<T> &cont) 	{serialSTLContPtr(cont);}
	template<class T>
	void			serialContPtr(std::deque<T> &cont) 	{serialSTLContPtr(cont);}
	template<class T>
	void			serialContPtr(std::set<T> &cont) 			{serialSTLContPtr(cont);}
	template<class T>
	void			serialContPtr(std::multiset<T> &cont) 	{serialSTLContPtr(cont);}


	/** \name standard STL containers serialisation. Elements must be pointers on a IStreamable object.
	 * Known Supported containers: vector<>, list<>, deque<>, set<>, multiset<>
	 * Support up to sint32 length containers.
	 * \see serialCont() serialContPtr()
	 */
	template<class T>
	void			serialContPolyPtr(std::vector<T> &cont) 	{serialVectorPolyPtr(cont);}
	template<class T>
	void			serialContPolyPtr(std::list<T> &cont) 	{serialSTLContPolyPtr(cont);}
	template<class T>
	void			serialContPolyPtr(std::deque<T> &cont) 	{serialSTLContPolyPtr(cont);}
	template<class T>
	void			serialContPolyPtr(std::set<T> &cont) 			{serialSTLContPolyPtr(cont);}
	template<class T>
	void			serialContPolyPtr(std::multiset<T> &cont) 	{serialSTLContPolyPtr(cont);}


	/** 
	 * Serialize Non Polymorphic Objet Ptr.
	 * Works with NULL pointers. If the same object is found mutliple time in the stream, ONLY ONE instance is written!
	 * NB: The ptr is serialised as a uint64 (64 bit compliant).
	 * \param ptr a pointer on a base type or an object.
	 * \see resetPtrTable()
	 */
	template<class T>
	void			serialPtr(T* &ptr) 
	{
		uint64	node;

		if(isReading())
		{
			serial(node);
			if(node==0)
				ptr=NULL;
			else
			{
				ItIdMap	it;
				it= _IdMap.find(node);

				// Test if object already created/read.
				if( it==_IdMap.end() )
				{
					// Construct object.
					ptr= new T;
					if(ptr==NULL)
						throw EStream();

					// Insert the node.
					_IdMap.insert( ValueIdMap(node, ptr) );

					// Read the object!
					serial(*ptr);
					
				}
				else
					ptr= static_cast<T*>(it->second);
			}
		}
		else
		{
			if(ptr==NULL)
			{
				node= 0;
				serial(node);
			}
			else
			{
				node= (uint64)(uint)ptr;
				serial(node);

				// Test if object already written.
				// If the Id was not yet registered (ie insert works).
				if( _IdMap.insert( ValueIdMap(node, ptr) ).second==true )
				{
					// Write the object!
					serial(*ptr);
				}
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
	void			serialPolyPtr(T* &ptr) 
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
	uint			serialVersion(uint currentVersion) ;


	/** 
	 * Serialize a check value.
	 * An object can stream a check value to check integrity or format of filed or streamed data.
	 * Just call serial check with a const value. Write will serial the value. Read will 
	 * check the value is the same. If it is not, it will throw EInvalidDataStream exception.
	 *
	 * NB: The type of the value must implement an operator == and must be serializable.
	 * \param value the value used to the check.
	 * \see EInvalidDataStream
	 */
	template<class T>
	void			serialCheck(const T& value) 
	{
		if (isReading()) 
		{ 
			T read;
			serial (read); 
			if (read!=value) 
				throw EInvalidDataStream(*this); 
		} 
		else 
		{ 
			serial (const_cast<T&>(value)); 
		}
	}

	/// Seek fonctionnality

	/** 
	 * Parameters for seek().
	 * begin seek from the begining of the stream.
	 * current seek from the current location of the stream pointer.
	 * end seek from the end of the stream.
	 */
	enum TSeekOrigin { begin, current, end };

	/** 
	 * Moves the stream pointer to a specified location.
	 * 
	 * NB: If the stream doesn't support the seek fonctionnality, it throw ESeekNotSupported.
	 * Default implementation: 
	 * { throw ESeekNotSupported; }
	 * \param offset is the wanted offset from the origin.
	 * \param origin is the origin of the seek
	 * \return true if seek sucessfull.
	 * \see ESeekNotSupported SeekOrigin getPos
	 */
	virtual bool		seek (sint32 offset, TSeekOrigin origin) ;


	/** 
	 * Get the location of the stream pointer.
	 * 
	 * NB: If the stream doesn't support the seek fonctionnality, it throw ESeekNotSupported.
	 * Default implementation: 
	 * { throw ESeekNotSupported; }
	 * \param offset is the wanted offset from the origin.
	 * \param origin is the origin of the seek
	 * \return the new offset regarding from the origin.
	 * \see ESeekNotSupported SeekOrigin seek
	 */
	virtual sint32		getPos () ;


	/** Get a name for this stream. maybe a fileName if FileStream.
	 *	Default is to return "".
	 */
	virtual std::string		getStreamName() const;


protected:

	/** 
	 * for Deriver: reset the PtrTable in the stream.
	 * If Derived stream provide reset()-like methods, they must call this method in their reset() methods.
	 * For example, CFile::close() must call it, so it will work correctly with next serialPtr()
	 */
	void				resetPtrTable();

	/**
	 * Change, in live, the state of the inputStream. This could be usefull in certain case.
	 * The deriver which would want to do such a thing must call this method, and implement his own behavior.
	 * In certain case, it should call resetPtrTable() if he want to reset the stream ptr info (maybe always)...
	 */
	void				setInOut(bool inputStream);


public:
	//@{
	/** Method to be specified by the Deriver.
	 * \warning Do not call these methods from outside, unless you really know what you are doing.
	 * Using them instead of serial() can lead to communication problems between different platforms !
	 */
	virtual void		serialBuffer(uint8 *buf, uint len) =0;
	virtual void		serialBit(bool &bit) =0;
	//@}

private:
	bool	_InputStream;
	bool	_NeedSwap;
	static	bool	_ThrowOnOlder;
	static	bool	_ThrowOnNewer;

	// Ptr registry. We store 64 bit Id, to be compatible with futur 64+ bits pointers.
	std::map<uint64, void*>				_IdMap;
	typedef std::map<uint64, void*>::iterator	ItIdMap;
	typedef std::map<uint64, void*>::value_type	ValueIdMap;

	// Ptr serialisation.
	void			serialIStreamable(IStreamable* &ptr) ;



private:
	/**
	 * standard STL containers serialisation. Don't work with map<> and multimap<>.
	 * Support up to sint32 length containers. serialize just len  element of the container.
	 */
	template<class T>
	void			serialSTLContLen(T &cont, sint32 len) 
	{
		typedef typename T::value_type __value_type;
		typedef typename T::iterator __iterator;

		if(isReading())
		{
			for(sint i=0;i<len;i++)
			{
				__value_type v;
				serial(v);
				__iterator it = cont.insert(cont.end(), v);
			}
		}
		else
		{
			__iterator		it= cont.begin();
			for(sint i=0;i<len;i++, it++)
			{
				serial(const_cast<__value_type&>(*it));
			}
		}
	}


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
	void			serialSTLCont(T &cont) 
	{
		sint32	len=0;
		if(isReading())
		{
			serial(len);
			cont.clear();
		}
		else
		{
			len= cont.size();
			serial(len);
		}

		serialSTLContLen(cont, len);
	}

	
protected:

	/**
	 * special version for serializing a vector.
	 * Support up to sint32 length containers.
	 */
	template<class T>
	void			serialVector(T &cont) 
	{
		typedef typename T::value_type __value_type;
		typedef typename T::iterator __iterator;

		sint32	len=0;
		if(isReading())
		{
			serial(len);
			// special version for vector: adjut good size.
			contReset(cont);
			cont.resize (len);

			// Read the vector
			for(sint i=0;i<len;i++)
			{
				serial(cont[i]);
			}
		}
		else
		{
			len= cont.size();
			serial(len);

			// Write the vector
			__iterator		it= cont.begin();
			for(sint i=0;i<len;i++, it++)
			{
				serial(const_cast<__value_type&>(*it));
			}
		}
	}


private:
	/**
	 * standard STL containers serialisation. Don't work with map<> and multimap<>.  Ptr version.
	 * Support up to sint32 length containers. serialize just len  element of the container.
	 */
	template<class T>
	void			serialSTLContLenPtr(T &cont, sint32 len) 
	{
		typedef typename T::value_type __value_type;
		typedef typename T::iterator __iterator;

		if(isReading())
		{
			for(sint i=0;i<len;i++)
			{
				__value_type	v;
				serialPtr(v);
				cont.insert(cont.end(), v);
			}
		}
		else
		{
			__iterator		it= cont.begin();
			for(sint i=0;i<len;i++, it++)
			{
				serialPtr(const_cast<__value_type&>(*it));
			}
		}
	}


	/**
	 * standard STL containers serialisation. Don't work with map<> and multimap<>.  Ptr version.
	 * Support up to sint32 length containers.
	 */
	template<class T>
	void			serialSTLContPtr(T &cont) 
	{
		sint32	len=0;
		if(isReading())
		{
			serial(len);
			cont.clear();
		}
		else
		{
			len= cont.size();
			serial(len);
		}

		serialSTLContLenPtr(cont, len);
	}


	/**
	 * special version for serializing a vector.  Ptr version.
	 * Support up to sint32 length containers.
	 */
	template<class T>
	void			serialVectorPtr(T &cont) 
	{
		typedef typename T::value_type __value_type;
		typedef typename T::iterator __iterator;

		sint32	len=0;
		if(isReading())
		{
			serial(len);
			// special version for vector: adjut good size.
			contReset(cont);
			cont.reserve(len);
		}
		else
		{
			len= cont.size();
			serial(len);
		}

		serialSTLContLenPtr(cont, len);
	}


private:
	/**
	 * standard STL containers serialisation. Don't work with map<> and multimap<>. PolyPtr version
	 * Support up to sint32 length containers. serialize just len  element of the container.
	 */
	template<class T>
	void			serialSTLContLenPolyPtr(T &cont, sint32 len) 
	{
		typedef typename T::value_type __value_type;
		typedef typename T::iterator __iterator;

		if(isReading())
		{
			for(sint i=0;i<len;i++)
			{
				__value_type	v=NULL;
				serialPolyPtr(v);
				cont.insert(cont.end(), v);
			}
		}
		else
		{
			__iterator		it= cont.begin();
			for(sint i=0;i<len;i++, it++)
			{
				serialPolyPtr(const_cast<__value_type&>(*it));
			}
		}
	}


	/**
	 * standard STL containers serialisation. Don't work with map<> and multimap<>. PolyPtr version
	 * Support up to sint32 length containers.
	 */
	template<class T>
	void			serialSTLContPolyPtr(T &cont) 
	{
		sint32	len=0;
		if(isReading())
		{
			serial(len);
			cont.clear();
		}
		else
		{
			len= cont.size();
			serial(len);
		}

		serialSTLContLenPolyPtr(cont, len);
	}


	/**
	 * special version for serializing a vector. PolyPtr version
	 * Support up to sint32 length containers.
	 */
	template<class T>
	void			serialVectorPolyPtr(T &cont) 
	{
		typedef typename T::value_type __value_type;
		typedef typename T::iterator __iterator;

		sint32	len=0;
		if(isReading())
		{
			serial(len);
			// special version for vector: adjut good size.
			contReset(cont);
			cont.reserve(len);
		}
		else
		{
			len= cont.size();
			serial(len);
		}

		serialSTLContLenPolyPtr(cont, len);
	}



private:

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
	void			serialMap(T &cont) 
	{
		typedef typename T::value_type __value_type;
		typedef typename T::key_type __key_type;
		typedef typename T::iterator __iterator;

		sint32	len;
		if(isReading())
		{
			cont.clear();
			serial(len);
			for(sint i=0;i<len;i++)
			{
				__value_type v;
				serial ( const_cast<__key_type&>(v.first) );
				serial (v.second);
				cont.insert(cont.end(), v);
			}
		}
		else
		{
			len= cont.size();
			serial(len);
			__iterator		it= cont.begin();
			for(sint i=0;i<len;i++, it++)
			{
				serial( const_cast<__key_type&>((*it).first) );
				serial((*it).second);
			}
		}
	}

};


// ======================================================================================================
// ======================================================================================================
// Handle for streaming Polymorphic classes.
// ======================================================================================================
// ======================================================================================================


// ======================================================================================================
/**
 * An Object Streamable interface. Any polymorphic class which want to use serial() in a polymorphic way, must derive
 * from this interface.
 * \author Lionel Berenguier
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
class IStreamable : public IClassable
{
public:
	virtual void		serial(IStream	&f) =0;
};


} // NLMISC.


// Inline Implementation.
#include "nel/misc/stream_inline.h"


#endif // NL_STREAM_H

/* End of stream.h */
