/** \file eid_translator.cpp
 * convert eid into entity name or user name and so on
 *
 * $Id: eid_translator.cpp,v 1.23 2004/04/05 10:05:30 lecroart Exp $
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

//
// Includes
//

#include "stdmisc.h"

#include <string>
#include <vector>
#include <map>

#include "nel/misc/algo.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/command.h"
#include "nel/misc/types_nl.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/eid_translator.h"

using namespace std;

namespace NLMISC {

//
// Variables
//

CEntityIdTranslator *CEntityIdTranslator::Instance = NULL;

// don't forget to increment the number when you change the file format
const uint CEntityIdTranslator::Version = 1;

//
// Functions
//

void CEntityIdTranslator::CEntity::serial (NLMISC::IStream &s)
{
	s.serial (EntityName);
	
	if (CEntityIdTranslator::getInstance()->FileVersion >= 1)
		s.serial (EntitySlot);
	else
	{
		if(s.isReading())
		{
			EntitySlot = -1;
		}
		else
		{
			sint8 slot = -1;
			s.serial (slot);
		}
	}
	s.serial (UId);
	s.serial (UserName);
}

CEntityIdTranslator *CEntityIdTranslator::getInstance ()
{
	if(Instance == NULL)
	{
		Instance = new CEntityIdTranslator;
	}
	return Instance;
}

void CEntityIdTranslator::getByUser (uint32 uid, vector<CEntityId> &res)
{
	for (reit it = RegisteredEntities.begin(); it != RegisteredEntities.end(); it++)
	{
		if ((*it).second.UId == uid)
		{
			res.push_back((*it).first);
		}
	}
}

void CEntityIdTranslator::getByUser (const string &userName, vector<CEntityId> &res, bool exact)
{
	string lowerName = strlwr (userName);
	
	for (reit it = RegisteredEntities.begin(); it != RegisteredEntities.end(); it++)
	{
		if (exact)
		{
			if (strlwr((*it).second.UserName) == lowerName)
			{
				res.push_back((*it).first);
			}
		}
		else
		{
			if (strlwr((*it).second.UserName).find(lowerName) != string::npos)
			{
				res.push_back((*it).first);
			}
		}
	}
}

ucstring CEntityIdTranslator::getByEntity (const CEntityId &eid)
{
	// we have to remove the crea and dyna because it can changed dynamically and will not be found in the storage array
	CEntityId reid(eid);
	reid.setCreatorId(0);
	reid.setDynamicId(0);

	reit it = RegisteredEntities.find (reid);
	if (it == RegisteredEntities.end ())
	{
		return ucstring("");
	}
	else
	{
		return (*it).second.EntityName;
	}
}

CEntityId CEntityIdTranslator::getByEntity (const ucstring &entityName)
{
	vector<CEntityId> res;
	getByEntity (entityName, res, true);
	if (res.empty())
		return CEntityId::Unknown;
	else
		return res[0];
}

void CEntityIdTranslator::getByEntity (const ucstring &entityName, vector<CEntityId> &res, bool exact)
{
	string lowerName = strlwr (entityName.toString());

	for (reit it = RegisteredEntities.begin(); it != RegisteredEntities.end(); ++it)
	{
		if (exact)
		{
			if (strlwr((*it).second.EntityName.toString()) == lowerName)
			{
				res.push_back((*it).first);
			}
		}
		else
		{
			if (strlwr((*it).second.EntityName.toString()).find(lowerName) != string::npos)
			{
				res.push_back((*it).first);
			}
		}
	}
}

bool CEntityIdTranslator::isValidEntityName (const ucstring &entityName,CLog *log, bool acceptBlanks)
{
	// 3 char at least
	if (entityName.size() < 3)
	{
		log->displayNL("Bad entity name '%s' (less than 3 char)", entityName.toString().c_str());
		return false;
	}

	if ( acceptBlanks )
	{
		// no blanks at the beginning or at the end
		if ( entityName[0] ==(uint16)0x20 )
		{
			log->displayNL("Bad entity name '%s' : start with a blank", entityName.toString().c_str());
			return false;
		}
		if ( entityName[entityName.size() - 1] ==(uint16)0x20 )
		{
			log->displayNL("Bad entity name '%s' : end with a blank", entityName.toString().c_str());
			return false;
		}
		bool previousBlank = false;
		for (uint i = 0; i < entityName.size(); i++)
		{
			if( entityName[i] == (uint16)0x20 )
			{
				// don't accept consecutive blanks
				if ( previousBlank )
				{
					log->displayNL("Bad entity name '%s' consecutive blanks are not allowed", entityName.toString().c_str());
					return false;
				}
				previousBlank = true;
			}
			else
			{
				// accept name with alphabetic and numeric value [a-zA-Z0-9]
				if (!isalnum (entityName[i]))
				{
					log->displayNL("Bad entity name '%s' (only char and num)", entityName.toString().c_str());
					return false;
				}
				previousBlank = false;
			}
		}
	}
	else for (uint i = 0; i < entityName.size(); i++)
	{
		// only accept name with alphabetic and numeric value [a-zA-Z0-9]
		if (!isalnum (entityName[i]))
		{
			log->displayNL("Bad entity name '%s' (only char and num)", entityName.toString().c_str());
			return false;
		}
	}

	// now check with the invalid name list
	string en = getRegisterableString( entityName, acceptBlanks);

	for (uint i = 0; i < InvalidEntityNames.size(); i++)
	{
		if(testWildCard(en, InvalidEntityNames[i]))
		{
			log->displayNL("Bad entity name '%s' (match the invalid entity name pattern '%s')", entityName.toString().c_str(), InvalidEntityNames[i].c_str());
			return false;
		}
	}

	return true;
}

bool CEntityIdTranslator::entityNameExists (const ucstring &entityName, bool acceptBlanks )
{
	// if bad name, don't accept it
	if (!isValidEntityName (entityName,NLMISC::InfoLog,acceptBlanks)) return true;

	// Names are stored in case dependant, so we have to test them without case.
	string registerable = getRegisterableString (entityName);

	for (reit it = RegisteredEntities.begin(); it != RegisteredEntities.end(); it++)
	{
		if (getRegisterableString ((*it).second.EntityName) == registerable)
		{
			return true;
		}
	}
	return false;
}

void CEntityIdTranslator::registerEntity (const CEntityId &eid, const ucstring &entityName, sint8 entitySlot, uint32 uid, const string &userName)
{
	// we have to remove the crea and dyna because it can changed dynamically and will not be found in the storage array
	CEntityId reid(eid);
	reid.setCreatorId(0);
	reid.setDynamicId(0);

	if (RegisteredEntities.find (reid) != RegisteredEntities.end ())
	{
		nlwarning ("EIT: Can't register EId %s EntityName %s UId %d UserName %s because EId is already in the map", reid.toString().c_str(), entityName.toString().c_str(), uid, userName.c_str());
		return;
	}

	if (entityNameExists(entityName))
	{
		nlwarning ("EIT: Can't register EId %s EntityName %s UId %d UserName %s because EntityName is already in the map", reid.toString().c_str(), entityName.toString().c_str(), uid, userName.c_str());
		return;
	}
	
	nlinfo ("EIT: Register EId %s EntityName %s UId %d UserName %s", reid.toString().c_str(), entityName.toString().c_str(), uid, userName.c_str());
	RegisteredEntities.insert (make_pair(reid, CEntityIdTranslator::CEntity(entityName, uid, userName, entitySlot)));

	save ();
}

void CEntityIdTranslator::unregisterEntity (const CEntityId &eid)
{
	// we have to remove the crea and dyna because it can changed dynamically and will not be found in the storage array
	CEntityId reid(eid);
	reid.setCreatorId(0);
	reid.setDynamicId(0);

	reit it = RegisteredEntities.find (reid);
	
	if (it == RegisteredEntities.end ())
	{
		nlwarning ("EIT: Can't unregister EId %s because EId is not in the map", reid.toString().c_str());
		return;
	}
	
	nlinfo ("EIT: Unregister EId %s EntityName %s UId %d UserName %s", reid.toString().c_str(), (*it).second.EntityName.toString().c_str(), (*it).second.UId, (*it).second.UserName.c_str());
	RegisteredEntities.erase (reid);
	
	save ();
}

void CEntityIdTranslator::checkEntity (const CEntityId &eid, const ucstring &entityName, uint32 uid, const string &userName)
{
	// we have to remove the crea and dyna because it can changed dynamically and will not be found in the storage array
	CEntityId reid(eid);
	reid.setCreatorId(0);
	reid.setDynamicId(0);

	map<CEntityId, CEntityIdTranslator::CEntity>::iterator it = RegisteredEntities.find (reid);
	bool needSave = false;
	
	nlinfo ("EIT: Checking EId %s EntityName '%s' UId %d UserName '%s'", reid.toString().c_str(), entityName.toString().c_str(), uid, userName.c_str());
	
	if (it == RegisteredEntities.end ())
	{
		nlwarning ("EIT: Check failed because EId is not in the CEntityIdTranslator map for EId %s EntityName '%s' UId %d UserName '%s'", reid.toString().c_str(), entityName.toString().c_str(), uid, userName.c_str());
		
		if (entityNameExists(entityName))
		{
			nlwarning ("EIT: Check failed because entity name already exist (%s) for EId %s EntityName '%s' UId %d UserName '%s'", getByEntity(entityName).toString().c_str(), reid.toString().c_str(), entityName.toString().c_str(), uid, userName.c_str());
		}
	}
	else
	{
		if ((*it).second.EntityName != entityName)
		{
			nlwarning ("EIT: Check failed because entity name not identical (%s) in the CEntityIdTranslator map for EId %s EntityName '%s' UId %d UserName '%s'", (*it).second.EntityName.toString().c_str(), reid.toString().c_str(), entityName.toString().c_str(), uid, userName.c_str());
			if(!entityName.empty())
			{
				(*it).second.EntityName = entityName;
				needSave = true;
			}
		}
		if ((*it).second.UId != uid)
		{
			nlwarning ("EIT: Check failed because uid not identical (%d) in the CEntityIdTranslator map for EId %s EntityName '%s' UId %d UserName '%s'", (*it).second.UId, reid.toString().c_str(), entityName.toString().c_str(), uid, userName.c_str());
			if (uid != 0)
			{
				(*it).second.UId = uid;
				needSave = true;
			}
		}
		if ((*it).second.UserName != userName)
		{
			nlwarning ("EIT: Check failed because user name not identical (%s) in the CEntityIdTranslator map for EId %s EntityName '%s' UId %d UserName '%s'", (*it).second.UserName.c_str(), reid.toString().c_str(), entityName.toString().c_str(), uid, userName.c_str());
			if(!userName.empty())
			{
				(*it).second.UserName = userName;
				needSave = true;
			}
		}
	}
	
	if (needSave)
		save ();
}

// this callback is call when the file is changed
void cbInvalidEntityNamesFilename(const std::string &invalidEntityNamesFilename)
{
	CEntityIdTranslator::getInstance()->InvalidEntityNames.clear ();

	string fn = CPath::lookup(invalidEntityNamesFilename, false);

	if (fn.empty())
	{
		nlwarning ("EIT: Can't load filename '%s' for invalid entity names filename (not found)", invalidEntityNamesFilename.c_str());
		return;
	}

	FILE *fp = fopen (fn.c_str(), "r");
	if (fp == NULL)
	{
		nlwarning ("EIT: Can't load filename '%s' for invalid entity names filename", fn.c_str());
		return;
	}

	while (true)
	{
		char str[512];
		fgets(str, 511, fp);
		if(feof(fp))
			break;
		if (strlen(str) > 0)
		{
			str[strlen(str)-1] = '\0';
			CEntityIdTranslator::getInstance()->InvalidEntityNames.push_back(str);
		}
	}
	
	fclose (fp);
}

void CEntityIdTranslator::load (const string &fileName, const string &invalidEntityNamesFilename)
{
	if (fileName.empty())
	{
		nlwarning ("EIT: Can't load empty filename for EntityIdTranslator");
		return;
	}

	if (!FileName.empty())
	{
		nlwarning ("EIT: Can't load file '%s' for EntityIdTranslator because we already load the file '%s'", fileName.c_str(), FileName.c_str());
		return;
	}
	
	nlinfo ("EIT: CEntityIdTranslator: load '%s'", fileName.c_str());

	FileName = fileName;

	if(CFile::fileExists(FileName))
	{
		CIFile ifile;
		if( ifile.open(FileName) )
		{
			FileVersion = Version;
			ifile.serialVersion (FileVersion);
			ifile.serialCont (RegisteredEntities);
			
			ifile.close ();
		}
		else
		{
			nlwarning ("EIT: Can't load filename '%s' for EntityIdTranslator", FileName.c_str());
		}
	}

	cbInvalidEntityNamesFilename (invalidEntityNamesFilename);

	NLMISC::CFile::addFileChangeCallback (invalidEntityNamesFilename, cbInvalidEntityNamesFilename);
}

void CEntityIdTranslator::save ()
{
	if (FileName.empty())
	{
		nlwarning ("EIT: Can't save empty filename for EntityIdTranslator (you forgot to load() it before?)");
		return;
	}

	nlinfo ("EIT: CEntityIdTranslator: save");

	COFile ofile;
	if( ofile.open(FileName) )
	{
		ofile.serialVersion (Version);
		FileVersion = Version;
		ofile.serialCont (RegisteredEntities);

		ofile.close ();
	}
	else
	{
		nlwarning ("EIT: Can't save filename '%s' for EntityIdTranslator", FileName.c_str());
	}
}

uint32 CEntityIdTranslator::getUId (const string &userName)
{
	const reit itEnd = RegisteredEntities.end();
	for (reit it = RegisteredEntities.begin(); it != itEnd ; ++it)
	{
		if ((*it).second.UserName == userName)
		{
			return (*it).second.UId;
		}
	}
	return 0;
}

string CEntityIdTranslator::getUserName (uint32 uid)
{
	const reit itEnd = RegisteredEntities.end();
	for (reit it = RegisteredEntities.begin(); it != itEnd ; ++it)
	{
		if ((*it).second.UId == uid)
		{
			return (*it).second.UserName;
		}
	}
	return 0;
}

void CEntityIdTranslator::getEntityIdInfo (const CEntityId &eid, ucstring &entityName, sint8 &entitySlot, uint32 &uid, string &userName, bool &online)
{
	// we have to remove the crea and dyna because it can changed dynamically and will not be found in the storage array
	CEntityId reid(eid);
	reid.setCreatorId(0);
	reid.setDynamicId(0);

	reit it = RegisteredEntities.find (reid);
	if (it == RegisteredEntities.end ())
	{
		nlwarning ("EIT: %s is not registered in CEntityIdTranslator", reid.toString().c_str());
		entityName = "";
		entitySlot = -1;
		uid = ~0;
		userName = "";
		online = false;
	}
	else
	{
		entityName = (*it).second.EntityName;
		entitySlot = (*it).second.EntitySlot;
		uid = (*it).second.UId;
		userName = (*it).second.UserName;
		online = (*it).second.Online;
	}
}

bool CEntityIdTranslator::setEntityNameStringId(const ucstring &entityName, uint32 stringId)
{
	const reit itEnd = RegisteredEntities.end();
	for (reit it = RegisteredEntities.begin(); it != itEnd ; ++it)
	{
		if ((*it).second.EntityName == entityName)
		{
			(*it).second.EntityNameStringId = stringId;
			return true;
		}
	}

	return false;
}

uint32 CEntityIdTranslator::getEntityNameStringId(const CEntityId &eid)
{
	// we have to remove the crea and dyna because it can changed dynamically and will not be found in the storage array
	CEntityId reid(eid);
	reid.setCreatorId(0);
	reid.setDynamicId(0);
	
	const reit it = RegisteredEntities.find (reid);
	if (it == RegisteredEntities.end ())
	{
		return 0;
	}
	else
	{
		return (*it).second.EntityNameStringId;
	}
}

void CEntityIdTranslator::setEntityOnline (const CEntityId &eid, bool online)
{
	// we have to remove the crea and dyna because it can changed dynamically and will not be found in the storage array
	CEntityId reid(eid);
	reid.setCreatorId(0);
	reid.setDynamicId(0);

	reit it = RegisteredEntities.find (reid);
	if (it == RegisteredEntities.end ())
	{
		nlwarning ("EIT: %s is not registered in CEntityIdTranslator", reid.toString().c_str());
	}
	else
	{
		(*it).second.Online = online;
	}
}

bool CEntityIdTranslator::isEntityOnline (const CEntityId &eid)
{
	// we have to remove the crea and dyna because it can changed dynamically and will not be found in the storage array
	CEntityId reid(eid);
	reid.setCreatorId(0);
	reid.setDynamicId(0);

	reit it = RegisteredEntities.find (reid);
	if (it == RegisteredEntities.end ())
	{
		nlwarning ("EIT: %s is not registered in CEntityIdTranslator", reid.toString().c_str());
		return false;
	}
	else
	{
		return (*it).second.Online;
	}
}

std::string CEntityIdTranslator::getRegisterableString( const ucstring & entityName,bool removeBlanks )
{
	string ret = strlwr( entityName.toString() );
	uint pos = ret.find( 0x20 );
	while( pos != string::npos )
	{
		ret.erase( pos,1 );
		pos = ret.find( 0x20 );
	}
	return ret;
}


NLMISC_COMMAND(findEIdByUser,"Find entity ids using the user name","<username>|<uid>")
{
	if (args.size () != 1)
		return false;

	vector<CEntityId> res;

	string userName = args[0];
	uint32 uid = atoi (userName.c_str());

	if (uid != 0)
	{
		CEntityIdTranslator::getInstance()->getByUser(uid, res);
		userName = CEntityIdTranslator::getInstance()->getUserName(uid);
	}
	else
	{
		CEntityIdTranslator::getInstance()->getByUser(userName, res);
		CEntityIdTranslator::getInstance()->getUId(userName);
	}
	
	log.displayNL("User Name '%s' (uid=%d) has %d entities:", userName.c_str(), uid, res.size());
	for (uint i = 0 ; i < res.size(); i++)
	{
		log.displayNL(">  %s %s", res[i].toString().c_str(), CEntityIdTranslator::getInstance()->getByEntity (res[i]).c_str());
	}
	
	return true;
}

NLMISC_COMMAND(findEIdByEntity,"Find entity id using the entity name","<entityname>|<eid>")
{
	if (args.size () != 1)
		return false;
	
	CEntityId eid (args[0].c_str());

	if (eid == CEntityId::Unknown)
	{
		eid = CEntityIdTranslator::getInstance()->getByEntity(args[0]);
	}

	if (eid == CEntityId::Unknown)
	{
		log.displayNL("'%s' is not an eid or an entity name", args[0].c_str());
		return false;
	}

	ucstring entityName;
	sint8 entitySlot;
	uint32 uid;
	string userName;
	bool online;

	CEntityIdTranslator::getInstance()->getEntityIdInfo(eid, entityName, entitySlot, uid, userName, online);

	log.displayNL("UId %d UserName '%s' EId %s EntityName '%s' EntitySlot %hd %s", uid, userName.c_str(), eid.toString().c_str(), entityName.toString().c_str(), (sint16)entitySlot, (online?"Online":"Offline"));
	
	return true;
}

NLMISC_COMMAND(entityNameValid,"Tell if an entity name is valid or not using CEntityIdTranslator validation rulez","<entityname>")
{
	if (args.size () != 1) return false;

	if(!CEntityIdTranslator::getInstance()->isValidEntityName(args[0], &log))
	{
		log.displayNL("Entity name '%s' is not valid", args[0].c_str());
	}
	else
	{
		if (CEntityIdTranslator::getInstance()->entityNameExists(args[0]))
		{
			log.displayNL("Entity name '%s' is already used by another player", args[0].c_str());
		}
		else
		{
			log.displayNL("Entity name '%s' is available", args[0].c_str());
		}
	}

	return true;
}

NLMISC_COMMAND(playerInfo,"Get informations about a player or all players in CEntityIdTranslator","[<entityname>|<eid>|<username>|<uid>]")
{
	if (args.size () == 0)
	{
		const map<CEntityId, CEntityIdTranslator::CEntity>	&res = CEntityIdTranslator::getInstance()->getRegisteredEntities ();
		log.displayNL("%d result(s) for 'all players informations'", res.size());
		for (map<CEntityId, CEntityIdTranslator::CEntity>::const_iterator it = res.begin(); it != res.end(); it++)
		{
			log.displayNL("UId %d UserName '%s' EId %s EntityName '%s' EntitySlot %hd %s", (*it).second.UId, (*it).second.UserName.c_str(), (*it).first.toString().c_str(), (*it).second.EntityName.toString().c_str(), (sint16)((*it).second.EntitySlot), ((*it).second.Online?"Online":"Offline"));
		}

		return true;
	}
	else if (args.size () == 1)
	{
		vector<CEntityId> res;

		CEntityId eid (args[0].c_str());
		uint32 uid = atoi (args[0].c_str());

		if (eid != CEntityId::Unknown)
		{
			// we have to remove the crea and dyna because it can changed dynamically and will not be found in the storage array
			eid.setCreatorId(0);
			eid.setDynamicId(0);
			
			res.push_back(eid);
		}
		else if (uid != 0)
		{
			// the parameter is an uid
			CEntityIdTranslator::getInstance()->getByUser (uid, res);
		}
		else
		{
			CEntityIdTranslator::getInstance()->getByUser (args[0], res, false);
			
			CEntityIdTranslator::getInstance()->getByEntity (args[0], res, false);
		}
		
		log.displayNL("%d result(s) for '%s'", res.size(), args[0].c_str());
		for (uint i = 0; i < res.size(); i++)
		{
			ucstring entityName;
			sint8 entitySlot;
			uint32 uid2;
			string userName;
			bool online;
			CEntityIdTranslator::getInstance()->getEntityIdInfo (res[i], entityName, entitySlot, uid2, userName, online);

			log.displayNL("UId %d UserName '%s' EId %s EntityName '%s' EntitySlot %hd %s", uid2, userName.c_str(), res[i].toString().c_str(), entityName.toString().c_str(), (sint16)entitySlot, (online?"Online":"Offline"));
		}

		return true;
	}

	return false;
}

}
