/** \file interpret_object_manager.cpp
 *
 * $Id: interpret_object_manager.cpp,v 1.1 2001/01/05 10:53:49 chafik Exp $
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
	CManagerClass::CManagerClass(const NLIAAGENT::IVarName &n) : CAgentClass(n)
	{
		setBaseMethodCount(((NLIAAGENT::CMainAgentScript *)(NLIAAGENT::CMainAgentScript::IdMainAgentScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLIAAGENT::CMainAgentScript *)(NLIAAGENT::CMainAgentScript::IdMainAgentScript.getFactory()->getClass())));
	}
	
	CManagerClass::CManagerClass(const NLIAC::CIdentType &id): CAgentClass(id)
	{
		setBaseMethodCount(((NLIAAGENT::CMainAgentScript *)(NLIAAGENT::CMainAgentScript::IdMainAgentScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLIAAGENT::CMainAgentScript *)(NLIAAGENT::CMainAgentScript::IdMainAgentScript.getFactory()->getClass())));
	}

	CManagerClass::CManagerClass(const NLIAAGENT::IVarName &n, const NLIAAGENT::IVarName &inheritance) : CAgentClass( inheritance )
	{
		setBaseMethodCount(((NLIAAGENT::CMainAgentScript *)(NLIAAGENT::CMainAgentScript::IdMainAgentScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLIAAGENT::CMainAgentScript *)(NLIAAGENT::CMainAgentScript::IdMainAgentScript.getFactory()->getClass())));
	}

	CManagerClass::CManagerClass(const CManagerClass &c) : CAgentClass( c )
	{
		setBaseMethodCount(((NLIAAGENT::CMainAgentScript *)(NLIAAGENT::CMainAgentScript::IdMainAgentScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLIAAGENT::CMainAgentScript *)(NLIAAGENT::CMainAgentScript::IdMainAgentScript.getFactory()->getClass())));
	}	

	CManagerClass::CManagerClass()
	{
		setBaseMethodCount(((NLIAAGENT::CMainAgentScript *)(NLIAAGENT::CMainAgentScript::IdMainAgentScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLIAAGENT::CMainAgentScript *)(NLIAAGENT::CMainAgentScript::IdMainAgentScript.getFactory()->getClass())));
	}


	const NLIAC::IBasicType *CManagerClass::clone() const
	{
		NLIAC::IBasicType *clone = new CManagerClass(*this);
		clone->incRef();
		return clone;
	}

	const NLIAC::IBasicType *CManagerClass::newInstance() const
	{
		NLIAC::IBasicType *instance = new CManagerClass();
		instance->incRef();
		return instance;
	}

	void CManagerClass::getDebugString(char *t) const
	{
	}

	NLIAAGENT::IObjectIA *CManagerClass::buildNewInstance() const
	{
		// Création des composants statiques
		// Création du message
		NLIAAGENT::CMainAgentScript *instance = new NLIAAGENT::CMainAgentScript(NULL);
		instance->incRef();

		return instance;
	}

	CManagerClass::~CManagerClass()
	{
	}
}
