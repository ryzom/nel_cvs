/* stream.h
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: stream.h,v 1.7 2000/09/13 08:56:17 berenguier Exp $
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
 * \author Lionel Berenguier
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 * This is the base interface for stream objects. Differents kind of streams may be implemented,
 * by specifying serialBuffer(uint8*, len) methods. Sample of streams: COutMemoryStream, CInFileStream ...
 */
class IStream
{
public:
	/// Set the behavior of IStream regarding input stream that are older/newer than the class (see serialVersion()). 
	/// If set to true, IStream throws a EStream::Older/Newer/Version when needed. By default, the behavior is
	/// throwOnOlder=false, throwOnNewer=true.
	static	void	setVersionException(bool throwOnOlder, bool throwOnNewer);
	static	void	getVersionException(bool &throwOnOlder, bool &throwOnNewer);


public:
	/// Constructor. Notice that those setup MUST be set at contruction.
	IStream(bool inputStream, bool needSwap);
	virtual ~IStream() {}

	/// Is this stream a Read/Input stream?
	bool			isReading();


	/// template Object serialisation.
	template<class T>
	void			serial(T &obj) throw(EStream)
	{ obj.serial(*this);	}

	/// Base type serialisation.
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

	/// Template for easy multiple serialisation.
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


	/// STL pair<> support.
	template<class T0, class T1>
	void			serial(std::pair<T0, T1> &p) throw(EStream)
	{ serial(p.first); serial(p.second);}


	/// Serialize standard STL containers.
	/// Support up to sint32 length containers.
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


	/// Serialize Polymorphic Objet Ptr. Works with NULL pointers.
	template<class T>
	void			serialPtr(T* &ptr) throw(ERegistry, EStream)
	{ IStreamable *p=ptr; serialPtr(p); ptr= static_cast<T*>(p);}
	void			serialPtr(IStreamable* &ptr) throw(ERegistry, EStream);


	/// Serialize a version number. NB: Version Number is read/store as a uint8, or uint32 if too bigger..
	void			serialVersion(uint &streamVersion, uint currentVersion) throw(EStream);



protected:

	/// If Derived stream provide reset()-like methods, they must call this method in their reset() methods.
	void				resetPtrTable();

	/// Methods to specify.
	virtual void		serialBuffer(uint8 *buf, uint len) throw(EStream)=0;
	virtual void		serialBit(bool &bit) throw(EStream)=0;

private:
	bool	_InputStream;
	bool	_NeedSwap;
	static	bool	_ThrowOnOlder;
	static	bool	_ThrowOnNewer;

	// Ptr registry. We store 64 bit Id, to be compatible with futur 64+ bits pointers.
	std::map<uint64, IStreamable*>				_IdMap;
	typedef std::map<uint64, IStreamable*>::iterator	ItIdMap;
	typedef std::map<uint64, IStreamable*>::value_type	ValueIdMap;
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



// Inline Implementation.
#include "nel/misc/stream_inline.h"


}	// namespace NLMISC.


#endif // NL_STREAM_H

/* End of stream.h */
