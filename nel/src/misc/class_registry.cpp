/* class_registry.cpp
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: class_registry.cpp,v 1.3 2000/10/09 14:53:44 lecroart Exp $
 *
 * <Replace this by a description of the file>
 */

#include "nel/misc/class_registry.h"
#include "nel/misc/debug.h"
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
		nlassert(CClassRegistry::checkObject(ptr));
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
		nlstop;
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