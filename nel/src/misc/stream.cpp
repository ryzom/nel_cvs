/* stream.cpp
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: stream.cpp,v 1.1 2000/09/08 13:06:49 berenguier Exp $
 *
 * <Replace this by a description of the file>
 */

#include "nel/misc/stream.h"
#include "nel/misc/assert.h"


namespace MISC
{


// ======================================================================================================
set<CClassRegistry::CClassNode>		CClassRegistry::RegistredClasses;

	
// ======================================================================================================
IStreamable	*CClassRegistry::create(const string &className)  throw(EStream)
{
	set<CClassNode>::iterator	it;
	CClassNode	node;

	node.ClassName= className;
	it=RegistredClasses.find(node);
	if(it==RegistredClasses.end())
		return NULL;
	else
	{
		IStreamable	*ptr;
		ptr=it->Creator();
		assert(CClassRegistry::checkObject(ptr));
		return ptr;
	}

}

// ======================================================================================================
void		CClassRegistry::registerClass(const string &className, IStreamable* (*creator)(), const string &typeidCheck)  throw(EStream)
{
	CClassNode	node;
	node.ClassName= className;
	node.Creator=creator;
	node.TypeIdCheck= typeidCheck;
	if(!RegistredClasses.insert(node).second)
	{
		assert(false);
		throw EStream(EStream::RegisteredClass);
	}
}

// ======================================================================================================
bool		CClassRegistry::checkObject(IStreamable* obj)
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