/** \file eid_translator.cpp
 * convert eid into entity name or user name and so on
 *
 * $Id: eid_translator.cpp,v 1.2 2003/04/15 08:46:35 lecroart Exp $
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

#include "stdpch.h"

#include <string>
#include <vector>
#include <map>

#include <nel/misc/types_nl.h>
#include <nel/misc/entity_id.h>
#include <nel/misc/file.h>
#include <nel/misc/command.h>

#include "eid_translator.h"

//
// Variables
//

CEntityIdTranslator *CEntityIdTranslator::Instance = NULL;


//
// Functions
//

CEntityIdTranslator *CEntityIdTranslator::getInstance ()
{
	if(Instance == NULL)
	{
		Instance = new CEntityIdTranslator;
	}
	return Instance;
}

void CEntityIdTranslator::getByUser (uint32 uid, std::vector<NLMISC::CEntityId> &res)
{
	for (reit it = RegisteredEntities.begin(); it != RegisteredEntities.end(); it++)
	{
		if ((*it).second.UId == uid)
		{
			res.push_back((*it).first);
		}
	}
}

void CEntityIdTranslator::getByUser (const std::string &userName, std::vector<NLMISC::CEntityId> &res)
{
	for (reit it = RegisteredEntities.begin(); it != RegisteredEntities.end(); it++)
	{
		if ((*it).second.UserName == userName)
		{
			res.push_back((*it).first);
		}
	}
}

std::string CEntityIdTranslator::getByEntity (const NLMISC::CEntityId &eid)
{
	reit it = RegisteredEntities.find (eid);
	if (it == RegisteredEntities.end ())
	{
		return "";
	}
	else
	{
		return (*it).second.EntityName;
	}
}

NLMISC::CEntityId CEntityIdTranslator::getByEntity (const std::string &entityName)
{
	for (reit it = RegisteredEntities.begin(); it != RegisteredEntities.end(); it++)
	{
		if ((*it).second.EntityName == entityName)
		{
			return (*it).first;
		}
	}
	return NLMISC::CEntityId::Unknown;
}

bool CEntityIdTranslator::entityNameExists (const std::string &entityName)
{
	for (reit it = RegisteredEntities.begin(); it != RegisteredEntities.end(); it++)
	{
		if ((*it).second.EntityName == entityName)
		{
			return true;
		}
	}
	return false;
}

void CEntityIdTranslator::registerEntity (const NLMISC::CEntityId &eid, const std::string &entityName, uint32 uid, const std::string &userName)
{
	reit it = RegisteredEntities.find (eid);

	nlassert(it == RegisteredEntities.end ());

	RegisteredEntities.insert (std::make_pair(eid, CEntityIdTranslator::CEntity(entityName, uid, userName)));
	nlinfo ("Registered %s with %s %d %s", eid.toString().c_str(), entityName.c_str(), uid, userName.c_str());

	save ();
}

void CEntityIdTranslator::load (const std::string &fileName)
{
	nlassert (!fileName.empty());
	nlassert (FileName.empty());

	FileName = fileName;

	NLMISC::CIFile ifile;
	if( ifile.open(FileName) )
	{
		ifile.serialCont (RegisteredEntities);

		ifile.close ();
	}
}

void CEntityIdTranslator::save ()
{
	if (FileName.empty())
	{
		nlwarning ("Can't save the eid translator file");
		return;
	}

	NLMISC::COFile ofile;
	if( ofile.open(FileName) )
	{
		ofile.serialCont (RegisteredEntities);

		ofile.close ();
	}
}

uint32 CEntityIdTranslator::getUId (const std::string &userName)
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

std::string CEntityIdTranslator::getUserName (uint32 uid)
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

void CEntityIdTranslator::getEntityIdInfo (const NLMISC::CEntityId &eid, std::string &entityName, uint32 &uid, std::string &userName)
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
		uid = (*it).second.UId;
		userName = (*it).second.UserName;
	}
}

NLMISC_COMMAND(findEIdByUser,"Find entity ids using the user name","<username>|<uid>")
{
	if (args.size () != 1)
		return false;

	std::vector<NLMISC::CEntityId> res;

	std::string userName = args[0];
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
	
	NLMISC::CEntityId eid (args[0].c_str());

	if (eid == NLMISC::CEntityId::Unknown)
	{
		eid = CEntityIdTranslator::getInstance()->getByEntity(args[0]);
	}

	if (eid == NLMISC::CEntityId::Unknown)
	{
		log.displayNL("'%s' is not an eid or an entity name", args[0].c_str());
		return false;
	}

	std::string entityName;
	uint32 uid;
	std::string userName;

	CEntityIdTranslator::getInstance()->getEntityIdInfo(eid, entityName, uid, userName);

	log.displayNL("EId %s EntityName '%s' UId %d UserName '%s'", eid.toString().c_str(), entityName.c_str(), uid, userName.c_str());
	
	return true;
}

