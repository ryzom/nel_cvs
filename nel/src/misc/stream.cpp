/* stream.cpp
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: stream.cpp,v 1.5 2000/09/14 15:58:40 berenguier Exp $
 *
 * <Replace this by a description of the file>
 */

#include "nel/misc/stream.h"
#include "nel/misc/assert.h"
using namespace std;


namespace NLMISC
{


// ======================================================================================================
// ======================================================================================================
// IStream.
// ======================================================================================================
// ======================================================================================================


// ======================================================================================================
bool	IStream::_ThrowOnOlder=false;
bool	IStream::_ThrowOnNewer=true;


// ======================================================================================================
void	IStream::setVersionException(bool throwOnOlder, bool throwOnNewer)
{
	_ThrowOnOlder=throwOnOlder;
	_ThrowOnNewer=throwOnNewer;
}

// ======================================================================================================
void	IStream::getVersionException(bool &throwOnOlder, bool &throwOnNewer)
{
	throwOnOlder=_ThrowOnOlder;
	throwOnNewer=_ThrowOnNewer;
}


// ======================================================================================================
// ======================================================================================================
// ======================================================================================================


// ======================================================================================================
void			IStream::serialIStreamable(IStreamable* &ptr) throw(ERegistry, EStream)
{
	uint64	node;

	if(isReading())
	{
		serial(node);
		if(node==0)
			ptr=NULL;
		else
		{
			ItIdMap	it;
			it= _IdMap.find(node);

			// Test if object already created/read.
			if( it==_IdMap.end() )
			{
				// Read the class name.
				string	className;
				serial(className);

				// Construct object.
				ptr= dynamic_cast<IStreamable*> (CClassRegistry::create(className));
				if(ptr==NULL)
					throw EUnregisteredClass();

				assert(CClassRegistry::checkObject(ptr));

				// Read the object!
				ptr->serial(*this);

				// Insert the node.
				_IdMap.insert( ValueIdMap(node, ptr) );
			}
			else
				ptr= static_cast<IStreamable*>(it->second);
		}
	}
	else
	{
		if(ptr==NULL)
		{
			node= 0;
			serial(node);
		}
		else
		{
			node= (uint64)ptr;
			serial(node);

			// Test if object already written.
			// If the Id was not yet registered (ie insert works).
			if( _IdMap.insert( ValueIdMap(node, ptr) ).second==true )
			{
				assert(CClassRegistry::checkObject(ptr));

				// Write the class name.
				string	className=ptr->getClassName();
				serial(className);

				// Write the object!
				ptr->serial(*this);
			}
		}
	}

}
// ======================================================================================================
void			IStream::resetPtrTable()
{
	_IdMap.clear();
}


// ======================================================================================================
// ======================================================================================================
// ======================================================================================================


// ======================================================================================================
uint IStream::serialVersion(uint currentVersion) throw(EStream)
{
	uint8	b;
	uint32	v;
	uint	streamVersion;

	if(isReading())
	{
		serial(b);
		if(b==0xFF)
			serial(v);
		else
			v=b;
		streamVersion=v;

		// Exception test.
		if(_ThrowOnOlder && streamVersion < currentVersion)
			throw EOlderStream();
		if(_ThrowOnNewer && streamVersion > currentVersion)
			throw ENewerStream();
	}
	else
	{
		v= streamVersion=currentVersion;
		if(v>=0xFF)
		{
			b=0xFF;
			serial(b);
			serial(v);
		}
		else
		{
			b=v;
			serial(b);
		}
	}

	return streamVersion;
}



}