/** \file entity_id.h
 * This class generate uniq Id for worl entities
 *
 * $Id: entity_id.h,v 1.1 2001/10/19 15:16:31 lecroart Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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

#ifndef NL_ENTITY_ID_H
#define NL_ENTITY_ID_H

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/common.h"
#include "nel/misc/stream.h"

namespace NLMISC {

/**
 * TODO
 * \author Sameh Chafik, Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
struct CEntityId
{
	static uint8 ServerId;
	static const uint64 MaxEntityId;

	/// Id of the service where the entity is.
	uint64	DynamicId   :  8;
	/// Id of the service who created the entity.
	uint64	CreatorId   :  8;
	/// Type of the entity.
	uint64	Type :  8;
	/// Local agent number.
	uint64	Id : 40;

	CEntityId()
	{
		DynamicId = ServerId;
		CreatorId = ServerId;
		Type = 0;
		Id = 0;
	}		

	CEntityId(uint64 id,uint8 creator,uint8 dyn)
	{
		DynamicId = dyn;
		CreatorId = creator;
		Id = id;
	}

	CEntityId(uint64 p)
	{			
		DynamicId = (p & 0xff);
		p >>= 8;
		CreatorId = (p & 0xff);
		p >>= 8;
		Type = (p & 0xff);
		p >>= 8;
		Id = (p);			
	}

	CEntityId(const CEntityId &a)
	{
		DynamicId = a.DynamicId;
		CreatorId = a.CreatorId;			
		Type = a.Type;
		Id = a.Id;
	}

	///fill from read stream.
	CEntityId(NLMISC::IStream &is)
	{
		uint64 p;
		is.serial(p);

		DynamicId = (p & 0xff);
		p >>= 8;
		CreatorId = (p & 0xff);
		p >>= 8;
		Type = (p & 0xff);
		p >>= 8;
		Id = p;
	}

	CEntityId(const char *str)
	{
		char *ident = (char*)str;
		char *id;
		char *type;
		char *creator;
		char *dyn;
		id = ident;

		sint n = 0;
		while(*(ident++) != ':');		
		type = ident;
		while(*(ident++) != ':');		
		creator = ident;
		while(*(ident++) != ':');		
		dyn = ident;	

		DynamicId = atoiInt64(dyn);
		CreatorId = atoiInt64(creator);
		Type = atoiInt64(type);
		Id = atoiInt64(id);
	}

	///\name comparison of two CIndexVariant.
	//@{
	bool operator == (const CEntityId &a) const
	{
		return (Id == a.Id && CreatorId == a.CreatorId);
	}

	bool operator < (const CEntityId &a) const
	{
		if(Id < a.Id)
			return true;
		else if(Id == a.Id)
			return (CreatorId < a.CreatorId);

		return false;
	}

	bool operator > (const CEntityId &a) const
	{
		if(Id > a.Id)
			return true;
		else if(Id == a.Id)
			return (CreatorId > a.CreatorId);

		return false;
	}
	//@}

	const CEntityId &operator ++(int)
	{
		if(Id < MaxEntityId)
		{
			Id ++;
		}
		else
		{
			nlerror ("CEntityId looped (max was %"NL_I64"d", MaxEntityId);
		}
		return *this;
	}

	const CEntityId &operator = (const CEntityId &a)
	{
		DynamicId = a.DynamicId;
		CreatorId = a.CreatorId;
		Type = a.Type;
		Id = a.Id;
		return *this;
	}

	const CEntityId &operator = (uint64 p)
	{			
		DynamicId = (uint64)(p & 0xff);
		p >>= 8;
		CreatorId = (uint64)(p & 0xff);
		p >>= 8;
		Type = (uint64)(p & 0xff);
		p >>= 8;
		Id = (uint64)(p);
		
		return *this;
	}

	operator uint64 () const
	{
		uint64 p = Id;
		p <<= 8;
		p |= (uint64)Type;
		p <<= 8;
		p |= (uint64)CreatorId;
		p <<= 8;
		p |= (uint64)DynamicId;

		return p;
	}

	void setServiceId (uint8 sid)
	{
		DynamicId = sid;
		CreatorId = sid;
		ServerId = sid;
	}

	///saving the nomber in an output stream.
	virtual void save(NLMISC::IStream &os)
	{
		uint64 p = Id;
		p <<= 8;
		p |= (uint64)Type;
		p <<= 8;
		p |= (uint64)CreatorId;
		p <<= 8;
		p |= (uint64)DynamicId;
		os.serial(p);
	}

	///loading the nomber from an input stream.
	virtual void load(NLMISC::IStream &is)
	{
		uint64 p;
		is.serial(p);

		DynamicId = (uint64)(p & 0xff);
		p >>= 8;
		CreatorId = (uint64)(p & 0xff);
		p >>= 8;
		Type = (uint64)(p & 0xff);
		p >>= 8;
		Id = (uint64)(p);
	}

	
	///Have a debug string.
	virtual void getDebugString(std::string &str) const
	{											
		char b[256];
		memset(b,0,255);
		memset(b,'0',19);
		sint n;

		uint64 x = Id;
		char baseTable[] = "0123456789abcdef";
		for(n = 10; n < 19; n ++)
		{
			b[19 - n] = baseTable[(x & 15)];
			x >>= 4;
		}
		b[19 - 9] = ':';

		x = Type;
		for(n = 7; n < 9; n ++)
		{				
			b[19 - n] = baseTable[(x & 15)];
			x >>= 4;
		}
		b[19 - 6] = ':';

		x = CreatorId;
		for(n = 4; n < 6; n ++)
		{				
			b[19 - n] = baseTable[(x & 15)];
			x >>= 4;
		}
		b[19 - 3] = ':';

		x = DynamicId;
		for(n = 1; n < 3; n ++)
		{							
			b[19 - n] = baseTable[(x & 15)];
			x >>= 4;
		}
		str += std::string(b);
	}

	/// \name NLMISC::IStreamable method.
	//@{
	virtual std::string	getClassName()
	{
		return std::string("<CEntityId>");
	}

	virtual void serial(NLMISC::IStream	&f) throw(NLMISC::EStream)
	{
		if(f.isReading())
		{
			load(f);
		}
		else
		{				
			save(f);
		}

	}
	//@}

};	


} // NLMISC

#endif // NL_ENTITY_ID_H

/* End of entity_id.h */
