/** \file class_registry.cpp
 * This File handles CClassRegistry.
 *
 * $Id: class_registry.cpp,v 1.7 2002/03/11 10:15:36 chafik Exp $
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

#include "stdmisc.h"

#include "nel/misc/class_registry.h"

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

