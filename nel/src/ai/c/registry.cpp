/** \file registry.cpp
 *
 * $Id: registry.cpp,v 1.3 2001/01/08 10:51:02 chafik Exp $
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
#include "c/abstract_interface.h" 
#include "e/ia_exception.h" 
#include "c/registry_class.h"
#include "c/registry_type.h"

namespace NLAIC
{
	CRegistry::CRegistryClass::CRegistryClass(const CIdentType &id,const IClassCFactory &factory,sint32 i):NameClass(*(new CIdentType(id))),FactoryClass ((const IClassCFactory *)factory.clone()),Index(i)
	{	
	}

	CRegistry::CRegistryClass::~CRegistryClass()
	{
		delete (IClassCFactory *)FactoryClass;
		delete (CIdentType *)&NameClass;
	}

	CRegistry::CRegistry(): _MapRegistry(*(new tMapRegistry))
	{
	}

	CRegistry::~CRegistry()
	{
		for(sint32 i = 0; i < (sint32) _TableRegistry.size(); i++) 
		{
			if ( _TableRegistry[i] )	
			{
				delete _TableRegistry[i];
			}
		}
		delete &_MapRegistry;
	}

	sint32 CRegistry::registerClass(const CIdentType &ident,const IClassCFactory &FactoryClass)
	{
		// Recherche un emplacement vide
		/*for(sint32 idx = 0; idx < (sint32) _TableRegistry.size(); idx++) 
		{
			if ( _TableRegistry[ idx ] == NULL )
			{
				CRegistryClass *t = new CRegistryClass( ident, FactoryClass, idx );
				_TableRegistry[ idx ] = t;
				_MapRegistry.insert( tMapRegistry::value_type(t->NameClass, t) );
				return idx;
			}
		}*/

		// Sinon met à la fin de la table
		CRegistryClass *t = new CRegistryClass(ident, FactoryClass, _TableRegistry.size() );
		_MapRegistry.insert( tMapRegistry::value_type(ident, t) );
		_TableRegistry.push_back( t );		
		return _TableRegistry.size() - 1;
	}

	// Supprime une classe de la registry
	void CRegistry::unRegisterClass(const CIdentType &ident)
	{			
		sint32 i;
		try
		{
			i = getNumIdent(ident);
		}
		catch(NLAIE::IException &e)
		{				
			throw e;
		}
		delete _TableRegistry[i]->FactoryClass;
		delete _TableRegistry[i];
		_TableRegistry[i] = NULL;
	}

	// Returns the registry index of a class from its identifier
	const sint32 CRegistry::getNumIdent(const CIdentType &ident)
	{
		CRegistryClass *t;
		tMapRegistry::iterator Itr = _MapRegistry.find(ident);
		if(Itr != _MapRegistry.end()) t = (*Itr).second;
		else 
		{
			throw NLAIE::CExceptionUnRegisterClassError((char *)((const char *)ident));
		}
		return t->Index;			
	}

	// Returns the registry index of a class from its class name in the registry
	const sint32 CRegistry::getNumIdent(const char *class_name)
	{
		for ( sint32 i = 0; i < (sint32) _TableRegistry.size(); i++ )
		{
			if ( !strcmp( (const char *)_TableRegistry[i]->NameClass, class_name ) )
				return i;
		}
		throw NLAIE::CExceptionUnRegisterClassError((char *)class_name);
		return -1;
	}

	// Returns the identifier of a class from its registry index
	const CIdentType &CRegistry::getIdent(const sint32 index)
	{
		return _TableRegistry[index]->NameClass;
	}

	// Returns the identifier of a class from its name
	const CIdentType &CRegistry::getIdent(const char *class_name)
	{
		for ( sint32 i = 0; i < (sint32) _TableRegistry.size(); i++ )
		{
			if ( !strcmp( (const char *)_TableRegistry[i]->NameClass, class_name ) )
				return _TableRegistry[ i ]->NameClass;
		}
		throw NLAIE::CExceptionUnRegisterClassError((char *)class_name );
	}

	// Creates a new instance from a class using its identifier
	IBasicInterface *CRegistry::createInstance(const CIdentType &ident)
	{
		return createInstance(getNumIdent(ident));
	}

	// Creates a new instance from a class using its class name in the registry
	IBasicInterface *CRegistry::createInstance(const char *class_name)
	{
		IBasicInterface *instance = (IBasicInterface *)_TableRegistry[ getNumIdent(class_name) ]->FactoryClass->createInstance();
		return instance;
	}

	// Creates a new instance from a class using its registry index		
	IBasicInterface *CRegistry::createInstance(const sint32 i)
	{
		IBasicInterface *instance = (IBasicInterface *)_TableRegistry[i]->FactoryClass->createInstance();
		return instance;
	}

	// Creates a new instance from a class using its identifier
	const IClassCFactory *CRegistry::getFactory(const CIdentType &ident)
	{
		CRegistryClass *t;
		tMapRegistry::iterator Itr = _MapRegistry.find(ident);
		if(Itr != _MapRegistry.end()) t = (*Itr).second;
		else 
		{
			throw NLAIE::CExceptionUnRegisterClassError((char *)((const char *)ident));
		}
		return t->FactoryClass;
	}

	// Creates a new instance from a class using its class name in the registry
	const IClassCFactory *CRegistry::getFactory(const char *class_name)
	{
		try
		{				
			CIdentType id(class_name);
			return id.getFactory();
		}
		catch (NLAIE::IException &err)
		{
			throw NLAIE::CExceptionContainer(err);
		}
		return NULL;
	}

	// Creates a new instance from a class using its registry index
	const IClassCFactory *CRegistry::getFactory(sint32 i)
	{
		return _TableRegistry[i]->FactoryClass;
	}

	const CIdentType& CRegistry::operator [](sint32 i) const
	{
		return _TableRegistry[i]->NameClass;
	}

	sint32 CRegistry::size() const
	{
		return _TableRegistry.size();
	}
}