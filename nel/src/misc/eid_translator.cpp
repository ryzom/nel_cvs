/** \file eid_translator.cpp
 * convert eid into entity name or user name and so on
 *
 * $Id: eid_translator.cpp,v 1.12 2003/08/29 15:34:47 lecroart Exp $
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

#include "nel/misc/types_nl.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/file.h"
#include "nel/misc/command.h"
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
	reit it = RegisteredEntities.find (eid);
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

	for (reit it = RegisteredEntities.begin(); it != RegisteredEntities.end(); it++)
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

bool CEntityIdTranslator::isValidEntityName (const ucstring &entityName)
{
	// 3 char at least
	if (entityName.size() < 3)
		return false;

	for (uint i = 0; i < entityName.size(); i++)
	{
		// only accept name with alphabetic and numeric value [a-zA-Z0-9]
		if (!isalnum (entityName[i]))
		{
			nlinfo ("Bad entity name '%s' (only char and num)", entityName.toString().c_str());
			return false;
		}
	}
	return true;
}

bool CEntityIdTranslator::entityNameExists (const ucstring &entityName)
{
	bool ok = isValidEntityName (entityName);
	
	// if bad name, don't accept it
	if (!ok) return true;

	// Names are stored in case dependant, so we have to test them without case.
	string lowerName = strlwr (entityName.toString());

	for (reit it = RegisteredEntities.begin(); it != RegisteredEntities.end(); it++)
	{
		if (strlwr ((*it).second.EntityName.toString()) == lowerName)
		{
			return true;
		}
	}
	return false;
}

void CEntityIdTranslator::registerEntity (const CEntityId &eid, const ucstring &entityName, sint8 entitySlot, uint32 uid, const string &userName)
{
	if (RegisteredEntities.find (eid) != RegisteredEntities.end ())
	{
		nlwarning ("Can't register EId %s EntityName %s UId %d UserName %s because EId is already in the map", eid.toString().c_str(), entityName.toString().c_str(), uid, userName.c_str());
		return;
	}

	if (entityNameExists(entityName))
	{
		nlwarning ("Can't register EId %s EntityName %s UId %d UserName %s because EntityName is already in the map", eid.toString().c_str(), entityName.toString().c_str(), uid, userName.c_str());
		return;
	}
	
	nlinfo ("Register EId %s EntityName %s UId %d UserName %s", eid.toString().c_str(), entityName.toString().c_str(), uid, userName.c_str());
	RegisteredEntities.insert (make_pair(eid, CEntityIdTranslator::CEntity(entityName, uid, userName, entitySlot)));

	save ();
}

void CEntityIdTranslator::unregisterEntity (const CEntityId &eid)
{
	reit it = RegisteredEntities.find (eid);
	
	if (it == RegisteredEntities.end ())
	{
		nlwarning ("Can't unregister EId %s because EId is not in the map", eid.toString().c_str());
		return;
	}
	
	nlinfo ("Unregister EId %s EntityName %s UId %d UserName %s", eid.toString().c_str(), (*it).second.EntityName.toString().c_str(), (*it).second.UId, (*it).second.UserName.c_str());
	RegisteredEntities.erase (eid);
	
	save ();
}

void CEntityIdTranslator::checkEntity (const CEntityId &eid, const ucstring &entityName, uint32 uid, const string &userName)
{
	map<CEntityId, CEntityIdTranslator::CEntity>::iterator it = RegisteredEntities.find (eid);
	bool needSave = false;
	
	nlinfo ("Checking EId %s EntityName '%s' UId %d UserName '%s'", eid.toString().c_str(), entityName.toString().c_str(), uid, userName.c_str());
	
	if (it == RegisteredEntities.end ())
	{
		nlwarning ("Check failed because EId is not in the CEntityIdTranslator map for EId %s EntityName '%s' UId %d UserName '%s'", eid.toString().c_str(), entityName.toString().c_str(), uid, userName.c_str());
		
		if (entityNameExists(entityName))
		{
			nlwarning ("Check failed because entity name already exist (%s) for EId %s EntityName '%s' UId %d UserName '%s'", getByEntity(entityName).toString().c_str(), eid.toString().c_str(), entityName.toString().c_str(), uid, userName.c_str());
		}
	}
	else
	{
		if ((*it).second.EntityName != entityName)
		{
			nlwarning ("Check failed because entity name not identical (%s) in the CEntityIdTranslator map for EId %s EntityName '%s' UId %d UserName '%s'", (*it).second.EntityName.toString().c_str(), eid.toString().c_str(), entityName.toString().c_str(), uid, userName.c_str());
			if(!entityName.empty())
			{
				(*it).second.EntityName = entityName;
				needSave = true;
			}
		}
		if ((*it).second.UId != uid)
		{
			nlwarning ("Check failed because uid not identical (%d) in the CEntityIdTranslator map for EId %s EntityName '%s' UId %d UserName '%s'", (*it).second.UId, eid.toString().c_str(), entityName.toString().c_str(), uid, userName.c_str());
			if (uid != 0)
			{
				(*it).second.UId = uid;
				needSave = true;
			}
		}
		if ((*it).second.UserName != userName)
		{
			nlwarning ("Check failed because user name not identical (%s) in the CEntityIdTranslator map for EId %s EntityName '%s' UId %d UserName '%s'", (*it).second.UserName.c_str(), eid.toString().c_str(), entityName.toString().c_str(), uid, userName.c_str());
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

void CEntityIdTranslator::load (const string &fileName)
{
	if (fileName.empty())
	{
		nlwarning ("Can't load empty filename for EntityIdTranslator");
		return;
	}

	if (!FileName.empty())
	{
		nlwarning ("Can't load file '%s' for EntityIdTranslator because we already load the file '%s'", fileName.c_str(), FileName.c_str());
		return;
	}
	
	nlinfo ("CEntityIdTranslator: load '%s'", fileName.c_str());

	FileName = fileName;

	CIFile ifile;
	if( ifile.open(FileName) )
	{
		ifile.serialVersion (FileVersion);
		ifile.serialCont (RegisteredEntities);

		ifile.close ();
	}
	else
	{
		nlwarning ("Can't load filename '%s' for EntityIdTranslator", FileName.c_str());
	}
}

void CEntityIdTranslator::save ()
{
	if (FileName.empty())
	{
		nlwarning ("Can't save empty filename for EntityIdTranslator (you forgot to load() it before?)");
		return;
	}

	nlinfo ("CEntityIdTranslator: save");

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
		nlwarning ("Can't save filename '%s' for EntityIdTranslator", FileName.c_str());
	}
}

uint32 CEntityIdTranslator::getUId (const string &userName)
{
	for (reit it = RegisteredEntities.begin(); it != RegisteredEntities.end(); it++)
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
	for (reit it = RegisteredEntities.begin(); it != RegisteredEntities.end(); it++)
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
	reit it = RegisteredEntities.find (eid);
	if (it == RegisteredEntities.end ())
	{
		nlwarning ("%s is not registered in CEntityIdTranslator", eid.toString().c_str());
		entityName = "";
		uid = 0;
		userName = "";
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

void CEntityIdTranslator::setEntityOnline (const CEntityId &eid, bool online)
{
	reit it = RegisteredEntities.find (eid);
	if (it == RegisteredEntities.end ())
	{
		nlwarning ("%s is not registered in CEntityIdTranslator", eid.toString().c_str());
	}
	else
	{
		(*it).second.Online = online;
	}
}

bool CEntityIdTranslator::isEntityOnline (const CEntityId &eid)
{
	reit it = RegisteredEntities.find (eid);
	if (it == RegisteredEntities.end ())
	{
		nlwarning ("%s is not registered in CEntityIdTranslator", eid.toString().c_str());
		return false;
	}
	else
	{
		return (*it).second.Online;
	}
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
			uint32 uid;
			string userName;
			bool online;
			CEntityIdTranslator::getInstance()->getEntityIdInfo (res[i], entityName, entitySlot, uid, userName, online);

			log.displayNL("UId %d UserName '%s' EId %s EntityName '%s' EntitySlot %hd %s", uid, userName.c_str(), res[i].toString().c_str(), entityName.toString().c_str(), (sint16)entitySlot, (online?"Online":"Offline"));
		}

		return true;
	}

	return false;
}

}
