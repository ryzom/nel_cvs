/** \file character.cpp
 *
 * $Id: character.cpp,v 1.3 2001/03/27 11:58:20 chafik Exp $
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

#include "nel/ai/character/character.h"

namespace NLAICHARACTER
{

/*
##############################
	ICharacter
##############################
*/

	const std::string &ICharacter::getName() const
	{
		throw;
	}

	void setName(const std::string &)
	{
		throw;
	}

	const ICharacter *ICharacter::haveCharacter(const std::string &) const
	{
		throw;
	}

	bool ICharacter::canAddCharacter() const
	{
		throw;
	}

	bool ICharacter::addCharacter(const ICharacter &)
	{
		throw;
	}


/*
##############################
	CCharacterChild
##############################
*/
	const NLAIC::CIdentType *CCharacterChild::IDCharacterChild = NULL;
	CCharacterChild::CCharacterChild(const CCharacterChild &name):ICharacter(name),_Name(name._Name)
	{
	}
	CCharacterChild::~CCharacterChild()
	{
	}

	const ICharacter *CCharacterChild::haveCharacter(const std::string &s) const
	{
		if(s == _Name) return this;
		const ICharacter *c = (const ICharacter *)getParent();	
		if(c == NULL) return NULL;
		else return c->haveCharacter(s);
	}

	bool CCharacterChild::canAddCharacter() const
	{
		return false;
	}
	bool CCharacterChild::addCharacter(const ICharacter &)
	{
		return false;
	}

	void CCharacterChild::save(NLMISC::IStream &os)
	{
		ICharacter::save(os);
		os.serial(_Name);
	}

	void CCharacterChild::load(NLMISC::IStream &is)
	{
		ICharacter::load(is);
		is.serial(_Name);
	}

	void CCharacterChild::getDebugString(char *text) const
	{
	}

	const NLAIC::IBasicType *CCharacterChild::clone() const
	{
		return new CCharacterChild(*this);
	}

	const NLAIC::IBasicType *CCharacterChild::newInstance() const
	{
		return new CCharacterChild(*this);
	}

	bool CCharacterChild::isEqual(const NLAIAGENT::IBasicObjectIA &a) const
	{
		const ICharacter &c = (const ICharacter &)a;
		return c.getName() == _Name;
	}

	const NLAIAGENT::IObjectIA::CProcessResult &CCharacterChild::run()
	{
		return NLAIAGENT::IObjectIA::ProcessRun;
	}

/*
##############################
	CCharacterNoeud
##############################
*/

	const NLAIC::CIdentType *CCharacterNoeud::IDCharacterNoeud = NULL;
	CCharacterNoeud::CCharacterNoeud(const CCharacterNoeud &c):CCharacterChild(c)
	{
		std::list<ICharacter *>::const_iterator i = c._Character.begin();
		while(i != c._Character.end())
		{
			_Character.push_back(*i);
			(*i)->incRef();
			i++;
		}
	}

	const ICharacter *CCharacterNoeud::haveCharacter(const std::string &s) const
	{
		if(s == getName()) return this;
		else
		{
			std::list<ICharacter *>::const_iterator i = _Character.begin();
			while(i != _Character.end())
			{
				const ICharacter *c = (*i++)->haveCharacter(s);
				if(c != NULL) return c;
			}
		}
		return NULL;
	}

	bool CCharacterNoeud::addCharacter(const ICharacter &c)
	{
		_Character.push_back((ICharacter *)c.clone());
		return true;
	}

	bool CCharacterNoeud::canAddCharacter() const
	{
		return true;
	}

	void CCharacterNoeud::save(NLMISC::IStream &os)
	{
		CCharacterChild::save(os);
		std::list<ICharacter *>::const_iterator i = _Character.begin();
		while(i != _Character.end())
		{			
			os.serial( (NLAIC::CIdentType &) (getType()) );
			(*i)->save(os);
			i++;
		}
	}

	void CCharacterNoeud::load(NLMISC::IStream &is)
	{
		CCharacterChild::load(is);
	}

	void CCharacterNoeud::getDebugString(char *text) const
	{
	}

	const NLAIC::IBasicType *CCharacterNoeud::clone() const
	{
		return new CCharacterNoeud(*this);
	}

	const NLAIC::IBasicType *CCharacterNoeud::newInstance() const
	{
		return new CCharacterNoeud(getName());
	}

	bool CCharacterNoeud::isEqual(const NLAIAGENT::IBasicObjectIA &a) const
	{

		const CCharacterNoeud &c = (const CCharacterNoeud &)a;
		if(c._Character.size() != _Character.size()) return false;

		std::list<ICharacter *>::const_iterator i = _Character.begin();
		std::list<ICharacter *>::const_iterator j = c._Character.begin();
		while(i != _Character.end())
		{
			if(!((*i++)->getName() == (*j++)->getName())) return false;			
		}
		return CCharacterChild::isEqual(a);
	}
}
