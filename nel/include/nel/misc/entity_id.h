/** \file entity_id.h
 * This class generate uniq Id for worl entities
 *
 * $Id: entity_id.h,v 1.18 2002/03/06 17:47:46 chafik Exp $
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
	///The local num service id of the local machin.
	static uint8 ServerId;
	///The maximume of number that we could generate without generat an overtaking exception.
	static const uint64 MaxEntityId;
	/// Unknow CEntityId is similar as an NULL pointer.
	static const CEntityId Unknown;


	/// Id of the service where the entity is.
	uint64	DynamicId   :  8;
	/// Id of the service who created the entity.
	uint64	CreatorId   :  8;
	/// Type of the entity.
	uint64	Type :  8;
	/// Local agent number.
	uint64	Id : 40;

	///\name Constructor
	//@{

	CEntityId ()
	{
		DynamicId = 0;
		CreatorId = 0;
		Type = 0;
		Id = 0;
	}

	CEntityId (uint8 type, uint64 id, uint8 creator, uint8 dynamic)
	{
		DynamicId = dynamic;
		CreatorId = creator;
		Type = type;
		Id = id;
	}

	CEntityId (uint8 type, uint64 id)
	{
		Type = type;
		Id = id;
		CreatorId = ServerId;
		DynamicId = ServerId;
	}

	explicit CEntityId (uint64 p)
	{			
		DynamicId = (p & 0xff);
		p >>= 8;
		CreatorId = (p & 0xff);
		p >>= 8;
		Type = (p & 0xff);
		p >>= 8;
		Id = (p);			
	}

	CEntityId (const CEntityId &a)
	{
		DynamicId = a.DynamicId;
		CreatorId = a.CreatorId;			
		Type = a.Type;
		Id = a.Id;
	}

	///fill from read stream.
	CEntityId (NLMISC::IStream &is)
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

	explicit CEntityId (const char *str)
	{
		char *ident = (char*)str;
		char *id;
		char *type;
		char *creator;
		char *dyn;
		id = ident;
		uint base = 10;

//Sameh si le nombre est en hexa alors mettre la base à 16.
		if(str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
		{
			base = 16;
			str+=2;
		}

		ident = (char*)str;
		id = ident;

		sint n = 0;
		while(*(ident++) != ':');		
		type = ident;
		while(*(ident++) != ':');		
		creator = ident;
		while(*(ident++) != ':');		
		dyn = ident;	

//Sameh conversion en fonction de la base.
		DynamicId = atoiInt64(dyn, base);
		CreatorId = atoiInt64(creator, base);
		Type = atoiInt64(type, base);
		Id = atoiInt64(id, base);
	}
	//@}	
	


	///\name comparison of two CEntityId.
	//@{
	virtual bool operator == (const CEntityId &a) const
	{
		return (Id == a.Id && CreatorId == a.CreatorId && Type == a.Type);
	}

	virtual bool operator < (const CEntityId &a) const
	{
		if (Type < a.Type)
		{
			return true;
		}
		else if (Type == a.Type)
		{
			if (Id < a.Id)
			{
				return true;
			}
			else if (Id == a.Id)
			{
				return (CreatorId < a.CreatorId);
			}
		}		
		return false;
	}

	virtual bool operator > (const CEntityId &a) const
	{
		if (Type > a.Type)
		{
			return true;
		}
		else if (Type == a.Type)
		{
			if (Id > a.Id)
			{
				return true;
			}
			else if (Id == a.Id)
			{
				return (CreatorId > a.CreatorId);
			}
		}
		// lesser
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

	uint64 getRawId() const
	{
		return (uint64)*this;
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

	/// Save the Id into an output stream.
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

	/// Load the number from an input stream.
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

	/// Have a debug string.
	virtual std::string toString() const
	{
		std::string id;
		getDebugString (id);
		return "(" + id + ")";
	}
	
	/// Have a debug string.
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
//Sameh To be sure that the number is in hexa.
		str += "0x" + std::string(b);
	}

	/// \name NLMISC::IStreamable method.
	//@{
	virtual std::string	getClassName ()
	{
		return std::string ("<CEntityId>");
	}

	virtual void serial (NLMISC::IStream &f) throw (NLMISC::EStream)
	{
		if (f.isReading ())
		{
			load (f);
		}
		else
		{				
			save (f);
		}

	}

	uint8 getType() const
	{
		return (uint8)Type;
	}
	//@}

//	friend std::stringstream &operator << (std::stringstream &__os, const CEntityId &__t);
};	

inline std::stringstream &operator << (std::stringstream &__os, const CEntityId &__t)
{
	__os << __t.toString ();
	return __os;
}

} // NLMISC

#endif // NL_ENTITY_ID_H

/* End of entity_id.h */
