/* class_registry.h
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: class_registry.h,v 1.3 2000/09/12 17:17:57 berenguier Exp $
 *
 * This File handles CClassRegistry.
 */

#ifndef NL_CLASS_REGISTRY_H
#define NL_CLASS_REGISTRY_H


#include	"nel/misc/types_nl.h"
#include	<string>
#include	<set>


namespace	NLMISC
{


// ======================================================================================================
/**
 * Class Registry Exception.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
struct ERegistry : public Exception
{
	virtual const char	*what() const throw() {return "Registry error";}
};
struct ERegisteredClass : public ERegistry
{
	virtual const char	*what() const throw() {return "Class already registered";}
};
struct EUnregisteredClass : public ERegistry
{
	virtual const char	*what() const throw() {return "Class not registered";}
};


// ======================================================================================================
/**
 * An Object Streamable interface.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class IClassable
{
public:
	virtual std::string		getClassName() =0;
};


// ======================================================================================================
/**
 * The Class registry where we can instanciate IClassable objects from their names.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CClassRegistry
{
public:

	///	Register your class for future Instanciation.
	static	void		registerClass(const std::string &className, IClassable* (*creator)(), const std::string &typeidCheck) throw(ERegistry);

	/// Create an object from his class name.
	static	IClassable	*create(const std::string &className) throw(ERegistry);

	/// check if the object has been correctly registered. Must be used for debug only, and Must compile with RTTI.
	static	bool		checkObject(IClassable* obj);


private:
	struct	CClassNode
	{
		std::string			ClassName;
		std::string			TypeIdCheck;
		IClassable*	(*Creator)();
		bool	operator<(const CClassNode &n) const
		{
			return ClassName<n.ClassName;
		}
	};
	static	std::set<CClassNode>		RegistredClasses;
};


/// Usefull Macros.
#define	NLMISC_DECLARE_CLASS(_class_)					\
	virtual std::string	getClassName() {return #_class_;}		\
	static	IClassable	*creator() {return new _class_;}
#define	NLMISC_REGISTER_CLASS(_class_) NLMISC::CClassRegistry::registerClass(#_class_, _class_::creator, typeid(_class_).name());



}	// namespace NLMISC.


#endif // NL_STREAM_H

/* End of stream.h */
