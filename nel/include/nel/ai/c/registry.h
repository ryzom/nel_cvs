/** \file registry.h
 * Includes class factory object for register class.
 *
 * $Id: registry.h,v 1.5 2002/10/15 15:03:05 portier Exp $
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

#ifndef NL_REGISTRY_H
#define NL_REGISTRY_H
#include "nel/misc/types_nl.h"

#include <map>
#include <vector>

namespace NLAIC 
{
	class CIdentType;
	class IClassFactory;
	class IBasicInterface;
	
	/**
	CRegistry is a class factory registry. This class share an std::map objects how it containe reference for ordering class. 
	Class factory is order in the map by name, the name is define by a CIdentType and by index. Index is an index in a vector type. 
	That make easy to manage object reference. Not that name is the word reference of an object and index is the locale reference.

	* \author Chafik sameh
	* \author Portier Pierre
	* \author Nevrax France
	* \date 2000
	*/
	class CRegistry
	{		
	private:		
		///Private structure to store data in the std::map.
		struct CRegistryClass
		{			
			const CIdentType &NameClass;
			const IClassFactory *FactoryClass;
			const sint32 Index;
			
			CRegistryClass(const CIdentType &id,const IClassFactory &factory,sint32 i);
			~CRegistryClass();
		};

		typedef std::map<CIdentType ,CRegistryClass *,std::less<CIdentType> > tMapRegistry;

		/// Table for class factory index
		std::vector<CRegistryClass *> _TableRegistry;
		///Map Hows stored class factory instance
		tMapRegistry &_MapRegistry;						

	public:

		CRegistry();

		/// Registers a new class
		sint32 registerClass(const CIdentType &,const IClassFactory &);

		/// Removes a class from the registry
		void unRegisterClass(const CIdentType &);

		/// Returns the registry index of a class from its identifier
		const sint32 getNumIdent(const CIdentType &);

		/// Returns the registry index of a class from its name
		const sint32 getNumIdent(const char *);

		/// Returns the identifier of a class from its registry index
		const CIdentType &getIdent(const sint32);

		/// Returns the identifier of a class from its name
		const CIdentType &getIdent(const char *);

		/// Creates a new instance from a class using its identifier
		IBasicInterface *createInstance(const CIdentType &);

		/// Creates a new instance from a class using its class name in the registry
		IBasicInterface *createInstance(const char *);

		/// Creates a new instance from a class using its registry index		
		IBasicInterface *createInstance(const sint32);

		/// Creates a new instance from a class using its identifier
		const IClassFactory *getFactory(const CIdentType &);

		/// Creates a new instance from a class using its class name in the registry
		const IClassFactory *getFactory(const char *);

		/// Creates a new instance from a class using its registry index
		const IClassFactory *getFactory(sint32);

		///Get CIdentType with an index.
		const CIdentType& operator [](sint32 i) const;

		// Returns true if a class with this name already exists in the registry, false otherwise
		const bool existsClass(const char *) const;

		///Get registry size.
		sint32 size() const;

		~CRegistry();
	};
}



#endif
