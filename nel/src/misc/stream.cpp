/** \file stream.cpp
 * This File handles IStream 
 *
 * $Id: stream.cpp,v 1.15 2001/04/26 15:07:54 vizerie Exp $
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

				// Insert the node.
				_IdMap.insert( ValueIdMap(node, ptr) );

				// Read the object!
				ptr->serial(*this);
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
			// Assume that prt size is an int size
			nlassert(sizeof(uint) == sizeof(void *));
			node= (uint64)((uint)ptr);
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
bool			IStream::seek (sint32 offset, TSeekOrigin origin) throw(EStream)
{
	throw ESeekNotSupported();
	return false;
}
// ======================================================================================================
sint32			IStream::getPos () throw(EStream)
{
	throw ESeekNotSupported();
}

// ======================================================================================================
void			IStream::setInOut(bool inputStream)
{
	_InputStream= inputStream;
}




}

