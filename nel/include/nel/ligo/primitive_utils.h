/** \file primitive_utils.h
 * Utility to work with primitives tree
 *
 *	Some of this utilities use the concept of 'predicate' applyed to 
 *	primitive node.
 *	As defined in the stl, a predicate is some form of functor that
 *	receive a parameter, apply some test on it, then return the result
 *	of the test.
 *	In this case, predicate functor receive a IPrimitive pointer and 
 *	return a boolean.
 *	The utilities come with 4 predefined predicate :
 *		- primitive class property test
 *		- primitive name property test
 *		- primitive class and name property test
 *		- primitive specific property value test.
 *
 *	You can define you own predicate. To do so, just create a class or struct
 *	with any data member/constructor you need. The unique requirement is that
 *	your class declare a method
 *		bool operator() (const IPrimitive *) const
 *
 *	You can also 'combine' the predefined predicate by using the 
 *	class CPrimitiveSetFilter that refine a set of primitive
 *	by applying another predicate.
 *
 *	Note that this work is inspired in some way by the functionality found
 *	in XPath for xml.
 *
 *	Boris.
 *
 * $Id: primitive_utils.h,v 1.1 2004/05/11 17:33:25 boucher Exp $
 */

/* Copyright, 2000-2002 Nevrax Ltd.
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




#ifndef PRIMITIVE_UTIL_H
#define PRIMITIVE_UTIL_H

#include "nel/ligo/primitive.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/file.h"
#include <vector>
#include <string>

namespace NLLIGO
{

typedef std::vector<IPrimitive*>	TPrimitiveSet;

/** Default predicate for primitive enumerator.
 *	This predicate test the class name of each primitive against a 
 *	given class name.
 */
struct TPrimitiveClassPredicate : public std::unary_function<IPrimitive*, bool>
{
	TPrimitiveClassPredicate(const std::string &className)
		:	ClassName(className)
	{}

	bool operator () (IPrimitive *prim)
	{
		std::string *s;
		if (prim->getPropertyByName("class", s) && *s == ClassName)
			return true;
		return false;
	}

	/// The primitive class name to check
	const std::string		ClassName;
};

/** Predicate for primitive enumerator.
 *	This predicate test the name of each primitive against a 
 *	given name.
 */
struct TPrimitiveNamePredicate
{
	TPrimitiveNamePredicate(const std::string &name)
		:	Name(name)
	{}

	bool operator () (IPrimitive *prim)
	{
		std::string *s;
		if (prim->getPropertyByName("name", s) && *s == Name)
			return true;
		return false;
	}

	/// The primitive name to check
	const std::string		Name;
};

/** Predicate for primitive enumerator.
 *	This predicate test the class name and name of the primitive.
 */
struct TPrimitiveClassAndNamePredicate
{
	TPrimitiveClassAndNamePredicate(const std::string &className, const std::string &name)
		:	ClassName(className),
		Name(name)
	{}

	bool operator () (IPrimitive *prim)
	{
		std::string *s;
		if (prim->getPropertyByName("class", s) && *s == ClassName)
		{
			if (prim->getPropertyByName("name", s) && *s == Name)
				return true;
		}
		return false;
	}

	/// The primitive class name to check
	const std::string		ClassName;
	/// The primitive name to check
	const std::string		Name;
};

/** Predicate for primitive enumerator.
 *	This predicate test the string value of a given property
 */
struct TPrimitivePropertyPredicate
{
	TPrimitivePropertyPredicate(const std::string &propName, const std::string &value)
		:	PropName(propName), PropValue(value)
	{}

	bool operator () (IPrimitive *prim)
	{
		std::string *s;
		if (prim->getPropertyByName(PropName.c_str(), s) && *s == PropValue)
			return true;
		return false;
	}

	/// The property name
	const std::string		PropName;
	/// The property value
	const std::string		PropValue;
};

/** The primitive enumerator class is used to iterate over primitive node that
 *	match a given predicate.
 *	The primitive are tested recursively starting at an arbitrary node in the
 *	primitive tree.
 *	Application code just need to call getNetMatch until it return NULL indicating
 *	there is no more node that match the predicate.
 */
template <class Pred = TPrimitiveClassPredicate>
class CPrimitiveEnumerator
{
public:
	/** Construct a primitive enumerator.
	 *	startPrim is the primitive where the enumeration start. Even if the startPrimitive
	 *	is not at the root of the primitive tree, the enumerator will not 
	 *	try to parse the parent of startPrim.
	 *	predicate is the functor predicate. Each node is tested against the bool operator()(IPrimitive *)
	 *	method of the predicate. If the predicate return true, then the node is returned by getNextMatch.
	 */
	CPrimitiveEnumerator(IPrimitive *startPrim, Pred &predicate)
		: _StartPrim(startPrim), 
		_Predicate(predicate), 
		_CurrentPrim(NULL)
	{
	}

	/** Each call the this method will return a primitive pointer that match
	 *	the predicate functor.
	 *	Return NULL when there is no more primitive matching the predicate.
	 */
	IPrimitive *getNextMatch()
	{
		IPrimitive *ret = NULL;
		if (_CurrentPrim == NULL)
		{
			// this is the first call, init the stack an current prim
			_CurrentPrim = _StartPrim;
			_IndexStack.push_back(0);
		}

		// normal tree traversal behavior
		while (_CurrentPrim != NULL)
		{
			while (_CurrentPrim->getNumChildren() > _IndexStack.back())
			{
				// there are still some valid childs
				bool ret = _CurrentPrim->getChild(_CurrentPrim, _IndexStack.back()++);
				nlassert(ret);
				_IndexStack.push_back(0);
			}

			// no more child, test the current node
			if (_Predicate(_CurrentPrim))
			{
				// we got one !
				ret = _CurrentPrim;
			}
			_IndexStack.pop_back();


			if (_CurrentPrim == _StartPrim)
			{
				// we are on the start prim, no more prim to parse
				_CurrentPrim = NULL;
			}
			else
			{
				// back to parent
				_CurrentPrim = _CurrentPrim->getParent();
			}

			// we have a node to return
			if (ret)
				return ret;
		}

		// no more match
		return NULL;
	}


private:

	/// The root primitive for enumeration
	IPrimitive	*_StartPrim;
	/// The predicate functor
	Pred		_Predicate;
	/// The current primitive
	IPrimitive	*_CurrentPrim;
	/// for each recursion level, keep the index of the currently explored child.
	std::vector<uint>	_IndexStack;
};

/** Build a primitive set that match the predicate
 *	This class make use of the CPrimitiveEnumerator class to iterate
 *	onn each valid node and fill the result primitive set.
*/
template <class Pred>
class CPrimitiveSet
{
public:

	void buildSet(IPrimitive *startPrimitive, Pred &predicate, TPrimitiveSet &result)
	{
		CPrimitiveEnumerator<Pred>	enumerator(startPrimitive, predicate);

		IPrimitive *p;
		while ((p = enumerator.getNextMatch()) != NULL)
			result.push_back(p);
	}
};

/** Filter a primitive set against a predicate.
 *	Useful to refine a primitive set with another predicate.
 */
template <class Pred>
class CPrimitiveSetFilter
{
public:

	void filterSet(const std::vector<IPrimitive*> &source, TPrimitiveSet &result)
	{
		Pred	predicate;
		std::vector<IPrimitive*>::const_iterator first(source.begin()), last(source.end());
		for (; first != last; ++first)
		{

			if (predicate(*first))
				result.push_back(*first);
		}
	}
};

/** Utility function that load an xml primitive file into a CPrimitives object.
 *	This function deal with file IO and XML parsing call.
 *	Return false if the loading fail for some reason, true otherwise.
 */
bool loadXmlPrimitiveFile(CPrimitives &primDoc, const std::string &fileName, CLigoConfig &ligoConfig)
{
	try
	{
		NLMISC::CIFile	fileIn(fileName);
		NLMISC::CIXml xmlIn;
		xmlIn.init (fileIn);

		// Read it
		return primDoc.read (xmlIn.getRootNode (), fileName.c_str(), ligoConfig);
		return true;
	}
	catch(NLMISC::Exception e)
	{
		nlwarning("Error reading input file '%s': '%s'", fileName.c_str(), e.what());
		return false;
	}
}

/** Utility function to look for the first child of a primitive node that
 *	match the predicate.
 *	Return NULL if none of the child match the predicate.
 *	There is no way to get the next matching child using this function, 
 *	you must use filterPrimitiveChilds to do this.
 */
template <class Pred>
IPrimitive *getPrimitiveChild(IPrimitive *parent, Pred &predicate)
{
	for (uint i=0; i<parent->getNumChildren(); ++i)
	{
		IPrimitive *child;
		if (parent->getChild(child, i) && predicate(child))
			return child;
	}

	return NULL;
}

/** Utility function that fill a primitive set with all the child nodes
 *	that match the predicate.
 */
template <class Pred>
void filterPrimitiveChilds(IPrimitive *parent, Pred &predicate, TPrimitiveSet &result)
{
	for (uint i=0; i<parent->getNumChildren(); ++i)
	{
		IPrimitive *child;
		if (parent->getChild(child, i) && predicate(child))
			result.push_back(child);
	}
}


} // namespace NLLIGO


#endif // #define PRIMITIVE_UTIL_H
