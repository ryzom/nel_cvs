/** \file entity_id.h
 * This class generate uniq Id for worl entities
 *
 * $Id: entity_id.h,v 1.26 2003/04/28 09:32:52 ledorze Exp $
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

private :
	// ---------------------------------------------------------------------------------
	// instantiated data

	union 
	{
		struct
		{
		/// Id of the service where the entity is.
		uint64	DynamicId   :  8;
		/// Id of the service who created the entity.
		uint64	CreatorId   :  8;
		/// Type of the entity.
		uint64	Type :  8;
		/// Local agent number.
		uint64	Id : 40;
		} DetailedId;

		uint64 FullId;
	};

public :

	// ---------------------------------------------------------------------------------
	// static data

	///The local num service id of the local machin.
	static uint8 ServerId;
	///The maximume of number that we could generate without generat an overtaking exception.
	static const uint64 MaxEntityId;
	/// Unknow CEntityId is similar as an NULL pointer.
	static const CEntityId Unknown;

	// ---------------------------------------------------------------------------------
	// constructors

	///\name Constructor
	//@{

	CEntityId ()
	{
		FullId = 0;
		DetailedId.Type = 127;

		/*
		DynamicId = 0;
		CreatorId = 0;
		Type = 127;
		Id = 0;
		*/
	}

	CEntityId (uint8 type, uint64 id, uint8 creator, uint8 dynamic)
	{
		DetailedId.DynamicId = dynamic;
		DetailedId.CreatorId = creator;
		DetailedId.Type = type;
		DetailedId.Id = id;
	}

	CEntityId (uint8 type, uint64 id)
	{
		DetailedId.Type = type;
		DetailedId.Id = id;
		DetailedId.CreatorId = ServerId;
		DetailedId.DynamicId = ServerId;
	}

	explicit CEntityId (uint64 p)
	{	
		FullId = p;
		/*
		DynamicId = (p & 0xff);
		p >>= 8;
		CreatorId = (p & 0xff);
		p >>= 8;
		Type = (p & 0xff);
		p >>= 8;
		Id = (p);			
		*/
	}

	CEntityId (const CEntityId &a)
	{
		FullId = a.FullId;
		/*
		DynamicId = a.DynamicId;
		CreatorId = a.CreatorId;			
		Type = a.Type;
		Id = a.Id;
		*/
	}

	///fill from read stream.
	CEntityId (NLMISC::IStream &is)
	{
		is.serial(FullId);
		/*
		uint64 p;
		is.serial(p);

		DynamicId = (p & 0xff);
		p >>= 8;
		CreatorId = (p & 0xff);
		p >>= 8;
		Type = (p & 0xff);
		p >>= 8;
		Id = p;
		*/
	}

	explicit CEntityId (const char *str)
	{
		CEntityId ();
		fromString(str);

//	Old version code (doesn't work) i thought it has never been tested :(
// 		char *ident = (char*)str;
//		char *id;
//		char *type;
//		char *creator;
//		char *dyn;
//		id = ident;
//		uint base = 10;
//
////Sameh si le nombre est en hexa alors mettre la base à 16.
//		if(str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
//		{
//			base = 16;
//			str+=2;
//		}
//
//		ident = (char*)str;
//		id = ident;
//
//		while(*ident != ':') if (*ident!=0) ++ident; else {*this=Unknown; return;}		
//		type = ident;
//		while(*ident != ':') if (*ident!=0) ++ident; else {*this=Unknown; return;}		
//		creator = ident;
//		while(*ident != ':') if (*ident!=0) ++ident; else {*this=Unknown; return;}		
//		dyn = ident;	
//
////Sameh conversion en fonction de la base.
//		DetailedId.DynamicId = atoiInt64(dyn, base);
//		DetailedId.CreatorId = atoiInt64(creator, base);
//		DetailedId.Type = atoiInt64(type, base);
//		DetailedId.Id = atoiInt64(id, base);
	}
	//@}	
	

	// ---------------------------------------------------------------------------------
	// accessors

	uint64 getRawId() const
	{
		return FullId;
		/*
		return (uint64)*this;
		*/
	}

	uint64 getShortId() const
	{
		return DetailedId.Id;
	}

	void setShortId( uint64 shortId )
	{
		DetailedId.Id = shortId;
	}

	uint8 getDynamicId() const
	{
		return DetailedId.DynamicId;
	}

	void setDynamicId( uint8 dynId )
	{
		DetailedId.DynamicId = dynId;
	}

	uint8 getCreatorId() const
	{
		return DetailedId.CreatorId;
	}

	void setCreatorId( uint8 creatorId )
	{
		DetailedId.CreatorId = creatorId;
	}

	uint8 getType() const
	{
		return (uint8)DetailedId.Type;
	}

	void setType( uint8 type )
	{
		DetailedId.Type = type;
	}

	uint64 getUniqueId() const
	{
		CEntityId id;
		id.FullId = FullId;
		id.DetailedId.DynamicId = 0;
		return id.FullId;
	}

	bool isUnknownId() const
	{
		return DetailedId.Type == 127;
	}


	// ---------------------------------------------------------------------------------
	// operators

	///\name comparison of two CEntityId.
	//@{
	bool operator == (const CEntityId &a) const
//	virtual bool operator == (const CEntityId &a) const
	{

		CEntityId testId ( FullId ^ a.FullId );
		testId.DetailedId.DynamicId = 0;
		return testId.FullId == 0;

		/*
		return (Id == a.DetailedId.Id && DetailedId.CreatorId == a.DetailedId.CreatorId && DetailedId.Type == a.DetailedId.Type);
		*/
	}

	bool operator < (const CEntityId &a) const
//	virtual bool operator < (const CEntityId &a) const
	{
		return getUniqueId() < a.getUniqueId();

		/*
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
		*/
	}

	bool operator > (const CEntityId &a) const
//	virtual bool operator > (const CEntityId &a) const
	{
		return getUniqueId() > a.getUniqueId();

		/*
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
		*/
	}
	//@}

	const CEntityId &operator ++(int)
	{
		if(DetailedId.Id < MaxEntityId)
		{
			DetailedId.Id ++;
		}
		else
		{
			nlerror ("CEntityId looped (max was %"NL_I64"d", MaxEntityId);
		}
		return *this;
	}

	const CEntityId &operator = (const CEntityId &a)
	{
		FullId = a.FullId;
		/*
		DynamicId = a.DynamicId;
		CreatorId = a.CreatorId;
		Type = a.Type;
		Id = a.Id;
		*/
		return *this;
	}

	const CEntityId &operator = (uint64 p)
	{			
		FullId = p;
		/*
		DynamicId = (uint64)(p & 0xff);
		p >>= 8;
		CreatorId = (uint64)(p & 0xff);
		p >>= 8;
		Type = (uint64)(p & 0xff);
		p >>= 8;
		Id = (uint64)(p);
		*/
		return *this;
	}


	// ---------------------------------------------------------------------------------
	// other methods...

	operator uint64 () const
	{
		return FullId;
		/*
		uint64 p = Id;
		p <<= 8;
		p |= (uint64)Type;
		p <<= 8;
		p |= (uint64)CreatorId;
		p <<= 8;
		p |= (uint64)DynamicId;

		return p;
		*/
	}

	void setServiceId (uint8 sid)
	{
		/*
		
		  Daniel says: Who wrote this?! It's horrible!!!
		  you're mixing statics and non-statics indisciminately
		  This needs to be fixed!!!
		
		*/

		DetailedId.DynamicId = sid;
		DetailedId.CreatorId = sid;
		ServerId = sid;
	}


	// ---------------------------------------------------------------------------------
	// loading, saving, serialising...

	/// Save the Id into an output stream.
	void save(NLMISC::IStream &os)
//	virtual void save(NLMISC::IStream &os)
	{
		os.serial(FullId);
		/*
		uint64 p = Id;
		p <<= 8;
		p |= (uint64)Type;
		p <<= 8;
		p |= (uint64)CreatorId;
		p <<= 8;
		p |= (uint64)DynamicId;
		os.serial(p);
		*/
	}

	/// Load the number from an input stream.
	void load(NLMISC::IStream &is)
//	virtual void load(NLMISC::IStream &is)
	{
		is.serial(FullId);
		/*
		uint64 p;
		is.serial(p);

		DynamicId = (uint64)(p & 0xff);
		p >>= 8;
		CreatorId = (uint64)(p & 0xff);
		p >>= 8;
		Type = (uint64)(p & 0xff);
		p >>= 8;
		Id = (uint64)(p);
		*/
	}


	void serial (NLMISC::IStream &f) throw (NLMISC::EStream)
//	virtual void serial (NLMISC::IStream &f) throw (NLMISC::EStream)
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


	// ---------------------------------------------------------------------------------
	// string convertions

	/// return a string in form "(a:b:c:d)" where a,b,c,d are components of entity id.
	std::string toString() const
	{
		std::string id;
		getDebugString (id);
		return "(" + id + ")";
	}

	/// Read from a debug string, use the same format as toString() (id:type:creator:dynamic) in hexadecimal
	void	fromString(const char *str)
//	virtual void	fromString(const char *str)
	{
		uint64		id;
		uint		type;
		uint		creatorId;
		uint		dynamicId;

		if (sscanf(str, "(%"NL_I64"x:%x:%x:%x)", &id, &type, &creatorId, &dynamicId) != 4)
			return;

		DetailedId.Id = id;
		DetailedId.Type = type;
		DetailedId.CreatorId = creatorId;
		DetailedId.DynamicId = dynamicId;
	}
	
	/// Have a debug string.
	void getDebugString(std::string &str) const
//	virtual void getDebugString(std::string &str) const
	{											
		char b[256];
		memset(b,0,255);
		memset(b,'0',19);
		sint n;

		uint64 x = DetailedId.Id;
		char baseTable[] = "0123456789abcdef";
		for(n = 10; n < 19; n ++)
		{
			b[19 - n] = baseTable[(x & 15)];
			x >>= 4;
		}
		b[19 - 9] = ':';

		x = DetailedId.Type;
		for(n = 7; n < 9; n ++)
		{				
			b[19 - n] = baseTable[(x & 15)];
			x >>= 4;
		}
		b[19 - 6] = ':';

		x = DetailedId.CreatorId;
		for(n = 4; n < 6; n ++)
		{				
			b[19 - n] = baseTable[(x & 15)];
			x >>= 4;
		}
		b[19 - 3] = ':';

		x = DetailedId.DynamicId;
		for(n = 1; n < 3; n ++)
		{							
			b[19 - n] = baseTable[(x & 15)];
			x >>= 4;
		}
//Sameh To be sure that the number is in hexa.
		str += "0x" + std::string(b);
	}
/*
	/// \name NLMISC::IStreamable method.
	//@{
	std::string	getClassName ()
//	virtual std::string	getClassName ()
	{
		return std::string ("<CEntityId>");
	}

	//@}
*/

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
