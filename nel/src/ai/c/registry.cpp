/** \file registry.cpp
 *
 * $Id: registry.cpp,v 1.15 2002/10/15 15:03:34 portier Exp $
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
#include "nel/ai/c/abstract_interface.h" 
#include "nel/ai/e/ai_exception.h" 
#include "nel/ai/c/registry_class.h"
#include "nel/ai/c/registry_type.h"

namespace NLAIC
{
	CRegistry::CRegistryClass::CRegistryClass(const CIdentType &id,const IClassFactory &factory,sint32 i):NameClass(*(new CIdentType(id))),FactoryClass ((const IClassFactory *)factory.clone()),Index(i)
	{	
	}

	CRegistry::CRegistryClass::~CRegistryClass()
	{
		delete (IClassFactory *)FactoryClass;
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

	sint32 CRegistry::registerClass(const CIdentType &ident,const IClassFactory &FactoryClass)
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
			std::string text;
			text = NLAIC::stringGetBuild("can't find '%s' in the registry",(char *)((const char *)ident));
			NLAIE::CExceptionUnRegisterClassError e(text.c_str());
			throw e;
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

		std::string text;
		text = NLAIC::stringGetBuild("can't find '%s' in the registry",(char *)class_name);
		NLAIE::CExceptionUnRegisterClassError e(text.c_str());
		throw e;
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
		std::string text;
		text = NLAIC::stringGetBuild("can't find '%s' in the registry",(char *)class_name);
		NLAIE::CExceptionUnRegisterClassError e(text.c_str());
		throw e;
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
	const IClassFactory *CRegistry::getFactory(const CIdentType &ident)
	{
		CRegistryClass *t;
		tMapRegistry::iterator Itr = _MapRegistry.find(ident);
		if(Itr != _MapRegistry.end()) t = (*Itr).second;
		else 
		{			
			std::string text;
			text = NLAIC::stringGetBuild("can't find '%s' in the registry",(char *)((const char *)ident));
			NLAIE::CExceptionUnRegisterClassError e(text.c_str());
			throw e;
		}
		return t->FactoryClass;
	}

	// Creates a new instance from a class using its class name in the registry
	const IClassFactory *CRegistry::getFactory(const char *class_name)
	{
		try
		{				
			CIdentType id(class_name);
			return id.getFactory();
		}
		catch (NLAIE::IException &err)
		{
			throw NLAIE::CExceptionContainer(err.what());
		}
		return NULL;
	}

	// Creates a new instance from a class using its registry index
	const IClassFactory *CRegistry::getFactory(sint32 i)
	{
		return (*(_TableRegistry.begin() + i))->FactoryClass;//_TableRegistry[i]->FactoryClass;
		//return _TableRegistry[i]->FactoryClass;
	}

	const CIdentType& CRegistry::operator [](sint32 i) const
	{	
		return _TableRegistry[i]->NameClass;
	}

	sint32 CRegistry::size() const
	{
		return _TableRegistry.size();
	}

	const bool CRegistry::existsClass(const char *class_name) const
	{
		std::vector<CRegistryClass *>::const_iterator it_rc = _TableRegistry.begin();
		while ( it_rc != _TableRegistry.end() )
		{
			const char *rc_name = (const char *) (*it_rc)->NameClass;
			if ( strcmp( class_name, rc_name ) == 0)
				return true;
			it_rc++;
		}
		return false;
	}
}
