/* stream.h
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: stream.h,v 1.3 2000/09/08 13:05:30 berenguier Exp $
 *
 * This File handles CRegistry and CBaseStream 
 */

#ifndef NL_STREAM_H
#define NL_STREAM_H


#include	"nel/misc/types_nl.h"
#include	<string>
#include	<set>
using namespace std;


namespace	MISC
{


// ======================================================================================================
// ======================================================================================================
// Stream System.
// ======================================================================================================
// ======================================================================================================


// ======================================================================================================
class EStream : public Exception
{
public:
	enum	TError
	{
		Eof,
		OlderVersion,
		NewerVersion,
		RegisteredClass,
		UnregisteredClass
	};

public:
	TError	Error;

public:
	EStream(const TError &e) {Error=e;}
	const	char	*errorString() const
	{
		switch(Error)
		{
			case Eof: 
				return "End of file"; break;
			case OlderVersion: 
				return "Too Older Version"; break;
			case NewerVersion: 
				return "The version read is newer than the reader"; break;
			case RegisteredClass: 
				return "Class already registered"; break;
			case UnregisteredClass: 
				return "Class not registered"; break;
			default:
				return "Unknown Stream Exception"; break;
		};
	}
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
 * by specifying write() / read() methods. Sample of streams: CMemoryStream, CFileStream ...
 */
class IStream
{
public:
	/// Constructor.
	IStream(bool inputStream, bool needSwap);
	virtual ~IStream() {}

	///
	bool			isReading();

	/// template Object serialisation.
	template<class T>
	void			serial(T &obj) throw(EStream);

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
	void			serial(string &b) throw(EStream);
	void			serial(wchar &b) throw(EStream);
	void			serial(wstring &b) throw(EStream);

	/// Template for easy multiple serialisation.
	template<class T0,class T1>
	void			serial(T0 &a, T1 &b) throw(EStream);
	template<class T0,class T1,class T2>
	void			serial(T0 &a, T1 &b, T2 &c) throw(EStream);
	template<class T0,class T1,class T2,class T3>
	void			serial(T0 &a, T1 &b, T2 &c, T3 &d) throw(EStream);
	template<class T0,class T1,class T2,class T3,class T4>
	void			serial(T0 &a, T1 &b, T2 &c, T3 &d, T4 &e) throw(EStream);
	template<class T0,class T1,class T2,class T3,class T4,class T5>
	void			serial(T0 &a, T1 &b, T2 &c, T3 &d, T4 &e, T5 &f) throw(EStream);

	/// Serialize Polymorphic Objet Ptr. Works with NULL pointers.
	void			serialPtr(IStreamable* &ptr);

protected:

	/// Derived stream must call this method if they provide reset()-like methods.
	void			resetPtrTable();

	/// Methods to specify.
	virtual void		write(const uint8 *buf, uint len) throw(EStream)=0;
	virtual void		write(const bool &bit) throw(EStream)=0;

	virtual void		read(uint8 *buf, uint len) throw(EStream)=0;
	virtual void		read(bool &bit) throw(EStream)=0;

private:
	bool	_InputStream;
	bool	_NeedSwap;

	// Ptr registry. We store 64 bit Id, to be compatible with futur 64+ bits pointers.
	set<uint64>	_IdSet;
	set<uint64>::iterator	ItIdSet;
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
class IStreamable
{
public:
	virtual void		serial(IStream	&f) throw(EStream)=0;
	virtual string		getClassName() =0;
};


// ======================================================================================================
/**
 * The Class registry where we can instanciate IStreamable objects from their name.
 * \author Lionel Berenguier
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
class CClassRegistry
{
public:
	static	IStreamable	*create(const string &className) throw(EStream);
	static	void		registerClass(const string &className, IStreamable* (*creator)(), const string &typeidCheck) throw(EStream);


private:
	static	bool		checkObject(IStreamable* obj);
	friend	class		IStream;

	struct	CClassNode
	{
		string			ClassName;
		string			TypeIdCheck;
		IStreamable*	(*Creator)();
		bool	operator<(const CClassNode &n) const
		{
			return ClassName<n.ClassName;
		}
	};
	static	set<CClassNode>		RegistredClasses;
};


// Usefull Macros.
#define	MISC_DECLARE_STREAMABLE(_class_)					\
	virtual string	getClassName() {return #_class_;}		\
	static	void IStreamable	*creator() {return new _class_;}
#define	MISC_REGISTER_CLASS(_class_) MISC::CRegistry::registerClass(#_class_, _class_::creator, typeid(_class_).name());



// ======================================================================================================
// Inline Implementation.
// ======================================================================================================


#include "nel/misc/stream_inline.h"



}	// namespace MISC.


#endif // NL_STREAM_H

/* End of stream.h */
