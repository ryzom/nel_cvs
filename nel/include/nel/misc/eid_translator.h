/** \file eid_translator.h
 * convert eid into entity name or user name and so on
 *
 * $Id: eid_translator.h,v 1.7 2003/09/01 12:22:24 lecroart Exp $
 */

/* Copyright, 2003 Nevrax Ltd.
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

#ifndef	NL_EID_TRANSLATOR_H
#define	NL_EID_TRANSLATOR_H

#include <string>
#include <vector>
#include <map>

#include <nel/misc/types_nl.h>
#include <nel/misc/entity_id.h>
#include <nel/misc/ucstring.h>

namespace	NLMISC
{
	
class CEntityIdTranslator
{
public:

	static CEntityIdTranslator *getInstance ();

	// get all eid for a user using the user name or the user id
	void				getByUser (uint32 uid, std::vector<NLMISC::CEntityId> &res);
	void				getByUser (const std::string &userName, std::vector<NLMISC::CEntityId> &res, bool exact=true);
	
	// get entity name using the eid
	ucstring			getByEntity (const NLMISC::CEntityId &eid);

	// get eid using the entity name
	CEntityId			getByEntity (const ucstring &entityName);
	void				getByEntity (const ucstring &entityName, std::vector<NLMISC::CEntityId> &res, bool exact);
	
	// return true if an entity name already exists
	bool				entityNameExists (const ucstring &entityName);
		
	void				registerEntity (const CEntityId &eid, const ucstring &entityName, sint8 entitySlot, uint32 uid, const std::string &userName);
	void				unregisterEntity (const CEntityId &eid);

	// set an eid to online or not
	void				setEntityOnline (const CEntityId &eid, bool online);

	// is an entity in online
	bool				isEntityOnline (const CEntityId &eid);

	// check if parameters are coherent with the content of the class, if not, set with the parameters and warn
	void				checkEntity (const CEntityId &eid, const ucstring &entityName, uint32 uid, const std::string &userName);
	
	void				load (const std::string &fileName);

	void				getEntityIdInfo (const CEntityId &eid, ucstring &entityName, sint8 &entitySlot, uint32 &uid, std::string &userName, bool &online);

	// return the user id and 0 if not found
	uint32				getUId (const std::string &userName);
	std::string			getUserName (uint32 uid);
	
	struct CEntity
	{
		CEntity () :
		EntitySlot(-1), UId(~0), Online(false)
		{ }
		
		CEntity (const ucstring &entityName, uint32 uid, const std::string &userName, sint8 entitySlot) :
		EntityName(entityName), EntitySlot(entitySlot), UId(uid), UserName(userName), Online(false)
		{ }
		
		ucstring EntityName;
		sint8 EntitySlot;
		
		uint32 UId;
		std::string UserName;

		bool Online;

		void serial (NLMISC::IStream &s);
	};

	const std::map<NLMISC::CEntityId, CEntity>	&getRegisteredEntities () { return RegisteredEntities; }
	
	static const uint Version;

	uint FileVersion;

private:

	typedef std::map<NLMISC::CEntityId, CEntity>::iterator reit;

	std::map<NLMISC::CEntityId, CEntity>	RegisteredEntities;

	// Singleton, no ctor access
	CEntityIdTranslator() { }

	// Singleton instance
	static CEntityIdTranslator *Instance;

	void save ();

	// Returns true if the username is valid.
	// It means that there only alphabetic and numerical character and the name is at least 3 characters long.
	bool CEntityIdTranslator::isValidEntityName (const ucstring &entityName);

	std::string FileName;
};

}

#endif // NL_EID_TRANSLATOR_H

/* End of eid_translator.h */
