/** \file interpret_object_manager.cpp
 *
 * $Id: interpret_object_manager.cpp,v 1.2 2001/01/08 10:48:01 chafik Exp $
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
#include "c/registry_class.h"
#include "agent/agent_script.h"
#include "agent/agent_manager.h"
#include "script/compilateur.h"
#include "script/interpret_object_manager.h"
#include "agent/main_agent_script.h"

namespace NLIASCRIPT
{
	CManagerClass::CManagerClass(const NLAIAGENT::IVarName &n) : CAgentClass(n)
	{
		setBaseMethodCount(((NLAIAGENT::CMainAgentScript *)(NLAIAGENT::CMainAgentScript::IdMainAgentScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::CMainAgentScript *)(NLAIAGENT::CMainAgentScript::IdMainAgentScript.getFactory()->getClass())));
	}
	
	CManagerClass::CManagerClass(const NLAIC::CIdentType &id): CAgentClass(id)
	{
		setBaseMethodCount(((NLAIAGENT::CMainAgentScript *)(NLAIAGENT::CMainAgentScript::IdMainAgentScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::CMainAgentScript *)(NLAIAGENT::CMainAgentScript::IdMainAgentScript.getFactory()->getClass())));
	}

	CManagerClass::CManagerClass(const NLAIAGENT::IVarName &n, const NLAIAGENT::IVarName &inheritance) : CAgentClass( inheritance )
	{
		setBaseMethodCount(((NLAIAGENT::CMainAgentScript *)(NLAIAGENT::CMainAgentScript::IdMainAgentScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::CMainAgentScript *)(NLAIAGENT::CMainAgentScript::IdMainAgentScript.getFactory()->getClass())));
	}

	CManagerClass::CManagerClass(const CManagerClass &c) : CAgentClass( c )
	{
		setBaseMethodCount(((NLAIAGENT::CMainAgentScript *)(NLAIAGENT::CMainAgentScript::IdMainAgentScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::CMainAgentScript *)(NLAIAGENT::CMainAgentScript::IdMainAgentScript.getFactory()->getClass())));
	}	

	CManagerClass::CManagerClass()
	{
		setBaseMethodCount(((NLAIAGENT::CMainAgentScript *)(NLAIAGENT::CMainAgentScript::IdMainAgentScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::CMainAgentScript *)(NLAIAGENT::CMainAgentScript::IdMainAgentScript.getFactory()->getClass())));
	}


	const NLAIC::IBasicType *CManagerClass::clone() const
	{
		NLAIC::IBasicType *clone = new CManagerClass(*this);
		clone->incRef();
		return clone;
	}

	const NLAIC::IBasicType *CManagerClass::newInstance() const
	{
		NLAIC::IBasicType *instance = new CManagerClass();
		instance->incRef();
		return instance;
	}

	void CManagerClass::getDebugString(char *t) const
	{
	}

	NLAIAGENT::IObjectIA *CManagerClass::buildNewInstance() const
	{
		// Création des composants statiques
		// Création du message
		NLAIAGENT::CMainAgentScript *instance = new NLAIAGENT::CMainAgentScript(NULL);
		instance->incRef();

		return instance;
	}

	CManagerClass::~CManagerClass()
	{
	}
}
