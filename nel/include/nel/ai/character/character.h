/** \file character.h
 *
 * $Id: character.h,v 1.1 2001/03/14 13:19:25 chafik Exp $
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

#ifndef NL_CHARACTER_H
#define NL_CHARACTER_H

#include "nel/ai/agent/agent.h"

namespace NLAICHARACTER
{	
	class ICharacter: public NLAIAGENT::IRefrence
	{		
	public:		
		ICharacter():IRefrence(){}
		ICharacter(const ICharacter &m):IRefrence(m){}
		virtual ~ICharacter() {}

		virtual const std::string &getName() const = 0
		{
			throw;
		}

		virtual void setName(const std::string &) = 0
		{
			throw;
		}

		virtual const ICharacter *haveCharacter(const std::string &) const = 0
		{
			throw;
		}

		const ICharacter *haveCharacter(const ICharacter &c) const
		{
			return haveCharacter(c.getName());
		}

		virtual bool canAddCharacter() const = 0
		{
			throw;
		}

		virtual bool addCharacter(const ICharacter &) = 0
		{
			throw;
		}
		
	};

	class CCharacterChild: public ICharacter
	{
	private:
		std::string _Name;
	public:
		static const NLAIC::CIdentType IDCharacterChild;

	public:
		CCharacterChild(const std::string &name):_Name(name) {}
		CCharacterChild(const CCharacterChild &name);
		virtual ~CCharacterChild();

		virtual void setName(const std::string &name)
		{
			_Name = name;
		}

		virtual const std::string &getName() const
		{
			return _Name;
		}
		virtual const ICharacter *haveCharacter(const std::string &) const;
		virtual bool addCharacter(const ICharacter &);
		virtual bool canAddCharacter() const;

		virtual void save(NLMISC::IStream &os);
		virtual void load(NLMISC::IStream &is);
		virtual void getDebugString(char *text) const;
		virtual const NLAIC::IBasicType *clone() const;
		virtual const NLAIC::IBasicType *newInstance() const;

		virtual const NLAIC::CIdentType &getType() const
		{
			return IDCharacterChild;
		}
		virtual bool isEqual(const NLAIAGENT::IBasicObjectIA &a) const;
		virtual const NLAIAGENT::IObjectIA::CProcessResult &run();
		
	};

	class CCharacterNoeud: public CCharacterChild
	{
	private:		
		std::list<ICharacter *> _Character;

	public:
		static const NLAIC::CIdentType IDCharacterNoeud;

	public:
		CCharacterNoeud(const std::string &name):CCharacterChild(name) {}
		CCharacterNoeud(const CCharacterNoeud &name);
		virtual ~CCharacterNoeud();

		virtual const ICharacter *haveCharacter(const std::string &) const;
		virtual bool addCharacter(const ICharacter &);
		//virtual void popCharacter(const std::string &);
		virtual bool canAddCharacter() const;

		virtual void save(NLMISC::IStream &os);
		virtual void load(NLMISC::IStream &is);
		virtual void getDebugString(char *text) const;
		virtual const NLAIC::IBasicType *clone() const;
		virtual const NLAIC::IBasicType *newInstance() const;
		virtual const NLAIC::CIdentType &getType() const
		{
			return IDCharacterChild;
		}
		virtual bool isEqual(const NLAIAGENT::IBasicObjectIA &a) const;

	};	
}
#endif
