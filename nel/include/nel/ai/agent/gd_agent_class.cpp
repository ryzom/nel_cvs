/** \file gd_agent_class.cpp
 *
 * $Id: gd_agent_class.cpp,v 1.1 2001/02/28 09:42:37 portier Exp $
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
#include "nel/ai/c/registry_class.h"
#include "nel/ai/script/lexsupport.h"
#include "nel/ai/agent/gd_agent_script.h"

#include "nel/ai/script/gd_agent_class.h"

namespace NLAISCRIPT
{
	CGDAgentClass::CGDAgentClass(const NLAIAGENT::IVarName &n) : CAgentClass(n)
	{
		setBaseMethodCount(((NLAIAGENT::CGDAgentScript *)(NLAIAGENT::CGDAgentScript::IdGDAgentScript.getFactory()->getClass()))->getBaseMethodCount());		
		setBaseObjectInstance(((NLAIAGENT::CGDAgentScript *)(NLAIAGENT::CGDAgentScript::IdGDAgentScript.getFactory()->getClass())));		
	}
	
	CGDAgentClass::CGDAgentClass(const NLAIC::CIdentType &id): CAgentClass(id)
	{
		setBaseMethodCount(((NLAIAGENT::CGDAgentScript *)(NLAIAGENT::CGDAgentScript::IdGDAgentScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::CGDAgentScript *)(NLAIAGENT::CGDAgentScript::IdGDAgentScript.getFactory()->getClass())));		
	}

	CGDAgentClass::CGDAgentClass(const NLAIAGENT::IVarName &n, const NLAIAGENT::IVarName &inheritance) : CAgentClass( inheritance )
	{
		setBaseMethodCount(((NLAIAGENT::CGDAgentScript *)(NLAIAGENT::CGDAgentScript::IdGDAgentScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::CGDAgentScript *)(NLAIAGENT::CGDAgentScript::IdGDAgentScript.getFactory()->getClass())));		
	}

	CGDAgentClass::CGDAgentClass(const CGDAgentClass &c) : CAgentClass( c )
	{
		setBaseMethodCount(((NLAIAGENT::CGDAgentScript *)(NLAIAGENT::CGDAgentScript::IdGDAgentScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::CGDAgentScript *)(NLAIAGENT::CGDAgentScript::IdGDAgentScript.getFactory()->getClass())));		
	}	

	CGDAgentClass::CGDAgentClass()
	{
		setBaseMethodCount(((NLAIAGENT::CGDAgentScript *)(NLAIAGENT::CGDAgentScript::IdGDAgentScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::CGDAgentScript *)(NLAIAGENT::CGDAgentScript::IdGDAgentScript.getFactory()->getClass())));
	}


	const NLAIC::IBasicType *CGDAgentClass::clone() const
	{
		NLAIC::IBasicType *clone = new CGDAgentClass(*this);
		return clone;
	}

	const NLAIC::IBasicType *CGDAgentClass::newInstance() const
	{
		NLAIC::IBasicType *instance = new CGDAgentClass();
		return instance;
	}
/*
	void CGDAgentClass::getDebugString(char *t) const
	{
		strcpy(
	}
*/
	NLAIAGENT::IObjectIA *CGDAgentClass::buildNewInstance() const
	{
		// Création des composants statiques
		std::list<NLAIAGENT::IObjectIA *> components;
		createBaseClassComponents( components );

		// Création du message
		NLAIAGENT::CGDAgentScript *instance = new NLAIAGENT::CGDAgentScript( NULL, NULL ,components,  (CGDAgentClass *) this );
		return instance;
	}

	CGDAgentClass::~CGDAgentClass()
	{
	}
}	// NLAISCRIPT
