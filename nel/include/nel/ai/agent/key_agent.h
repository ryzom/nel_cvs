/** \file key_agent.h
 * key for the multi set in the CAgentScript class.
 *
 * $Id: key_agent.h,v 1.4 2002/08/21 13:58:17 lecroart Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
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

#ifndef NL_KEY_AGENT_H
#define NL_KEY_AGENT_H
namespace NLAIAGENT
{
	class IObjectIA;

	struct CKeyAgent
	{
		CStringType Name;
		std::list<IBasicAgent *>::iterator Itr;

		CKeyAgent(const CStringType &s, const std::list<IBasicAgent *>::iterator &i): Name(s), Itr(i)
		{
		}

		CKeyAgent(const CKeyAgent &k):Name(k.Name), Itr(k.Itr)
		{
		}

		CKeyAgent(const CStringType &s): Name(s)
		{
		}

		bool operator < (const CKeyAgent &k) const
		{
			return (Name < k.Name);
		}
	};	

	struct CKeyObject
	{
		CStringType Name;
		IObjectIA *Obj;

		CKeyObject(const CStringType &s, IObjectIA *i): Name(s), Obj(i)
		{
			Obj->incRef();
		}

		CKeyObject(const CKeyObject &k):Name(k.Name), Obj(k.Obj)
		{
			Obj->incRef();
		}

		CKeyObject(const CStringType &s): Name(s)
		{
		}

		virtual ~CKeyObject()
		{
			Obj->release();
		}

		const CKeyObject operator = (const CKeyObject &a)
		{
			Obj->release();
			Obj = a.Obj;
			Obj->incRef();
			return *this;
		}

		bool operator < (const CKeyObject &k) const
		{
			return (Name < k.Name);
		}
	};
}
#endif
