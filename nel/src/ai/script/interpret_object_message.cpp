/** \file interpret_object_message.cpp
 *
 * $Id: interpret_object_message.cpp,v 1.1 2001/01/05 10:53:49 chafik Exp $
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
#include "agent/message_script.h"
#include "script/lexsupport.h"

#include "script/interpret_object_message.h"

namespace NLIASCRIPT
{
	CMessageClass::CMessageClass(const NLIAAGENT::IVarName &n) : CAgentClass(n)
	{
		setBaseMethodCount(((NLIAAGENT::CMessageScript *)(NLIAAGENT::CMessageScript::IdMessageScript.getFactory()->getClass()))->getBaseMethodCount());		
		setBaseObjectInstance(((NLIAAGENT::CMessageScript *)(NLIAAGENT::CMessageScript::IdMessageScript.getFactory()->getClass())));		
	}
	
	CMessageClass::CMessageClass(const NLIAC::CIdentType &id): CAgentClass(id)
	{
		setBaseMethodCount(((NLIAAGENT::CMessageScript *)(NLIAAGENT::CMessageScript::IdMessageScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLIAAGENT::CMessageScript *)(NLIAAGENT::CMessageScript::IdMessageScript.getFactory()->getClass())));		
	}

	CMessageClass::CMessageClass(const NLIAAGENT::IVarName &n, const NLIAAGENT::IVarName &inheritance) : CAgentClass( inheritance )
	{
		setBaseMethodCount(((NLIAAGENT::CMessageScript *)(NLIAAGENT::CMessageScript::IdMessageScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLIAAGENT::CMessageScript *)(NLIAAGENT::CMessageScript::IdMessageScript.getFactory()->getClass())));		
	}

	CMessageClass::CMessageClass(const CMessageClass &c) : CAgentClass( c )
	{
		setBaseMethodCount(((NLIAAGENT::CMessageScript *)(NLIAAGENT::CMessageScript::IdMessageScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLIAAGENT::CMessageScript *)(NLIAAGENT::CMessageScript::IdMessageScript.getFactory()->getClass())));		
	}	

	CMessageClass::CMessageClass()
	{
		setBaseMethodCount(((NLIAAGENT::CMessageScript *)(NLIAAGENT::CMessageScript::IdMessageScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLIAAGENT::CMessageScript *)(NLIAAGENT::CMessageScript::IdMessageScript.getFactory()->getClass())));
	}


	const NLIAC::IBasicType *CMessageClass::clone() const
	{
		NLIAC::IBasicType *clone = new CMessageClass(*this);
		clone->incRef();
		return clone;
	}

	const NLIAC::IBasicType *CMessageClass::newInstance() const
	{
		NLIAC::IBasicType *instance = new CMessageClass();
		instance->incRef();
		return instance;
	}

	void CMessageClass::getDebugString(char *t) const
	{
	}

	NLIAAGENT::IObjectIA *CMessageClass::buildNewInstance() const
	{
		// Création des composants statiques
		std::list<NLIAAGENT::IObjectIA *> components;
		createBaseClassComponents( components );

		// Création du message
		NLIAAGENT::CMessageScript *instance = new NLIAAGENT::CMessageScript( components,  (CMessageClass *) this );
		instance->incRef();

		return instance;
	}

	CMessageClass::~CMessageClass()
	{
	}
}
