/** \file eid_translator.h
 * convert eid into entity name or user name and so on
 *
 * $Id: eid_translator.h,v 1.16.10.1 2005/01/12 15:21:19 legros Exp $
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
#include <nel/misc/command.h>

namespace	NLMISC
{
	
class CEntityIdTranslator
{
public:

	static CEntityIdTranslator *getInstance ();

	// performs all check on a name ( name validity + uniqueness )
	bool				checkEntityName (const ucstring &entityName);
	/// return true if a name already exists
	bool				entityNameExists(const ucstring &entityName);
	// register an entity in this manager
	void				registerEntity (const CEntityId &eid, const ucstring &entityName, sint8 entitySlot, uint32 uid, const std::string &userName);
	// unregister an entity from this manager
	void				unregisterEntity (const CEntityId &eid);
	// set an association entityName / entityStringId, return true if association has been set
	bool				setEntityNameStringId(const ucstring &entityName, uint32 stringId);
	// get string id for entityId
	uint32				getEntityNameStringId(const CEntityId &eid);
	
	// set an eid to online or not
	void				setEntityOnline (const CEntityId &eid, bool online);	

	// is an entity in online
	bool				isEntityOnline (const CEntityId &eid);

	// check if parameters are coherent with the content of the class, if not, set with the parameters and warn
	void				checkEntity (const CEntityId &eid, const ucstring &entityName, uint32 uid, const std::string &userName);
	
	// the first param is the file where are all entities information, the second is a text file (one line per pattern using * and ?) with invalid entity name
	void				load (const std::string &fileName, const std::string &invalidEntityNamesFilename);

	// you must call this function to save the data into the hard drive
	void				save ();

	// get eid using the entity name
	CEntityId			getByEntity (const ucstring &entityName);

	// get entity name using the eid
	ucstring			getByEntity (const NLMISC::CEntityId &eid);

	void				getEntityIdInfo (const CEntityId &eid, ucstring &entityName, sint8 &entitySlot, uint32 &uid, std::string &userName, bool &online, std::string* additional = NULL);

	// transform a username ucstring into a string that can be compared with registered string
	std::string getRegisterableString( const ucstring & entityName);

	

	/// return a vector of invalid names
	const std::vector<std::string> & getInvalidNames(){ return InvalidEntityNames; }
	
	struct CEntity
	{
		CEntity () :
		EntityNameStringId(0), EntitySlot(-1), UId(~0), Online(false)
		{ }
		
		CEntity (const ucstring &entityName, uint32 uid, const std::string &userName, sint8 entitySlot) :
		EntityName(entityName), EntityNameStringId(0), EntitySlot(entitySlot), UId(uid), UserName(userName), Online(false)
		{ }
		
		ucstring	EntityName;
		uint32		EntityNameStringId;
		sint8		EntitySlot;
		
		uint32		UId;
		std::string UserName;

		bool		Online;

		void serial (NLMISC::IStream &s);
	};

	const std::map<NLMISC::CEntityId, CEntity>	&getRegisteredEntities () { return RegisteredEntities; }
	
	static const uint Version;

	uint FileVersion;

	/**
	 * Callback called when getEntityIdInfo called, so service may add additional info
	 * Format MUST be [InfoName InfoValue]* (e.g. a list of 2 strings, first being name for
	 * the retrieved info, and second being the value of the info
	 */
	typedef std::string	(*TAdditionalInfoCb)(const CEntityId &eid);

	TAdditionalInfoCb	EntityInfoCallback;

private:
	// get all eid for a user using the user name or the user id
	void				getByUser (uint32 uid, std::vector<NLMISC::CEntityId> &res);
	void				getByUser (const std::string &userName, std::vector<NLMISC::CEntityId> &res, bool exact=true);
	
	void				getByEntity (const ucstring &entityName, std::vector<NLMISC::CEntityId> &res, bool exact);

	// return the user id and 0 if not found
	uint32				getUId (const std::string &userName);
	std::string			getUserName (uint32 uid);

	// Returns true if the username is valid.
	// It means that there only alphabetic and numerical character and the name is at least 3 characters long.
	bool isValidEntityName (const ucstring &entityName, NLMISC::CLog *log = NLMISC::InfoLog );

	typedef std::map<NLMISC::CEntityId, CEntity>::iterator reit;

	std::map<NLMISC::CEntityId, CEntity>	RegisteredEntities;

	// Singleton, no ctor access
	CEntityIdTranslator() { EntityInfoCallback = NULL; }

	// Singleton instance
	static CEntityIdTranslator *Instance;

	std::string FileName;

	std::vector<std::string> InvalidEntityNames;

	friend void cbInvalidEntityNamesFilename(const std::string &filename);
	friend struct entityNameValidClass;
	NLMISC_CATEGORISED_COMMAND_FRIEND(nel,findEIdByUser);
	NLMISC_CATEGORISED_COMMAND_FRIEND(nel,findEIdByEntity);
	NLMISC_CATEGORISED_COMMAND_FRIEND(nel,entityNameValid);
	NLMISC_CATEGORISED_COMMAND_FRIEND(nel,playerInfo);
};

}

#endif // NL_EID_TRANSLATOR_H

/* End of eid_translator.h */
