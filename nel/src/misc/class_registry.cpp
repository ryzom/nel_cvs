/* class_registry.cpp
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: class_registry.cpp,v 1.2 2000/09/14 16:40:53 cado Exp $
 *
 * <Replace this by a description of the file>
 */

#include "nel/misc/class_registry.h"
#include "nel/misc/assert.h"
#include <typeinfo>
using namespace std;


namespace NLMISC
{


// ======================================================================================================
set<CClassRegistry::CClassNode>		CClassRegistry::RegistredClasses;

	
// ======================================================================================================
IClassable	*CClassRegistry::create(const string &className)  throw(ERegistry)
{
	set<CClassNode>::iterator	it;
	CClassNode	node;

	node.ClassName= className;
	it=RegistredClasses.find(node);
	if(it==RegistredClasses.end())
		return NULL;
	else
	{
		IClassable	*ptr;
		ptr=it->Creator();
		assert(CClassRegistry::checkObject(ptr));
		return ptr;
	}

}

// ======================================================================================================
void		CClassRegistry::registerClass(const string &className, IClassable* (*creator)(), const string &typeidCheck)  throw(ERegistry)
{
	CClassNode	node;
	node.ClassName= className;
	node.Creator=creator;
	node.TypeIdCheck= typeidCheck;
	if(!RegistredClasses.insert(node).second)
	{
		assert(false);
		throw ERegisteredClass();
	}
}

// ======================================================================================================
bool		CClassRegistry::checkObject(IClassable* obj)
{
	set<CClassNode>::iterator	it;
	CClassNode	node;
	node.ClassName= obj->getClassName();

	it=RegistredClasses.find(node);
	if(it==RegistredClasses.end())
		return false;
	node= *it;

	if( node.TypeIdCheck != string(typeid(*obj).name()) )
		return false;

	return true;
}



}