/* stream.cpp
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: stream.cpp,v 1.9 2000/10/19 15:21:09 corvazier Exp $
 *
 * <Replace this by a description of the file>
 */

#include "nel/misc/stream.h"
#include "nel/misc/debug.h"
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
	uint64	node=0;

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

				nlassert(CClassRegistry::checkObject(ptr));

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
				nlassert(CClassRegistry::checkObject(ptr));

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
	uint8	b=0;
	uint32	v=0;
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
			b= (uint8)v;
			serial(b);
		}
	}

	return streamVersion;
}


// ======================================================================================================
// ======================================================================================================
// ======================================================================================================


// ======================================================================================================
void			IStream::serialCont(vector<uint8> &cont) throw(EStream)
{
	sint32	len=0;
	if(isReading())
	{
		serial(len);
		cont.resize(len);
		serialBuffer( (uint8*)&(*cont.begin()) ,  len);
	}
	else
	{
		len= cont.size();
		serial(len);
		serialBuffer( (uint8*)&(*cont.begin()) ,  len);
	}
}
// ======================================================================================================
void			IStream::serialCont(vector<sint8> &cont) throw(EStream)
{
	sint32	len=0;
	if(isReading())
	{
		serial(len);
		cont.resize(len);
		serialBuffer( (uint8*)&(*cont.begin()) ,  len);
	}
	else
	{
		len= cont.size();
		serial(len);
		serialBuffer( (uint8*)&(*cont.begin()) ,  len);
	}
}
// ======================================================================================================
void			IStream::serialCont(vector<bool> &cont) throw(EStream)
{
	sint32	len=0;
	vector<uint8>	vec;

	if(isReading())
	{
		serial(len);
		cont.resize(len);

		// read as uint8*.
		sint	lb= (len+7)/8;
		vec.resize(lb);
		serialBuffer( (uint8*)&(*vec.begin()) ,  lb);
		for(sint i=0;i<len;i++)
		{
			uint	bit= (vec[i>>3]>>(i&7)) & 1;
			cont[i]= bit?true:false;
		}
	}
	else
	{
		len= cont.size();
		serial(len);

		// write as uint8*.
		sint	lb= (len+7)/8;
		vec.resize(lb);
		fill_n(vec.begin(), lb, 0);
		for(sint i=0;i<len;i++)
		{
			uint	bit= cont[i]?1:0;
			vec[i>>3]|= bit<<(i&7);
		}
		serialBuffer( (uint8*)&(*vec.begin()) ,  lb);
	}

}
// ======================================================================================================
bool			IStream::seek (sint64 offset, TSeekOrigin origin) throw(EStream)
{
	throw ESeekNotSupported();
	return false;
}
// ======================================================================================================
sint64			IStream::getPos () throw(EStream)
{
	throw ESeekNotSupported();
}


}