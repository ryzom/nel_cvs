/** \file interpret_object_message.cpp
 *
 * $Id: interpret_object_message.cpp,v 1.10 2001/02/22 11:07:18 chafik Exp $
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
#include "nel/ai/agent/message_script.h"
#include "nel/ai/script/lexsupport.h"

#include "nel/ai/script/interpret_object_message.h"
#include "nel/ai/agent/msg_notify.h"

namespace NLAISCRIPT
{
	//###############################
	//CMessageClass
	//###############################
	CMessageClass::CMessageClass(const NLAIAGENT::IVarName &n) : CAgentClass(n)
	{
		setBaseMethodCount(((NLAIAGENT::CMessageScript *)(NLAIAGENT::CMessageScript::IdMessageScript.getFactory()->getClass()))->getBaseMethodCount());		
		setBaseObjectInstance(((NLAIAGENT::CMessageScript *)(NLAIAGENT::CMessageScript::IdMessageScript.getFactory()->getClass())));		
	}
	
	CMessageClass::CMessageClass(const NLAIC::CIdentType &id): CAgentClass(id)
	{
		setBaseMethodCount(((NLAIAGENT::CMessageScript *)(NLAIAGENT::CMessageScript::IdMessageScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::CMessageScript *)(NLAIAGENT::CMessageScript::IdMessageScript.getFactory()->getClass())));		
	}

	CMessageClass::CMessageClass(const NLAIAGENT::IVarName &n, const NLAIAGENT::IVarName &inheritance) : CAgentClass( inheritance )
	{
		setBaseMethodCount(((NLAIAGENT::CMessageScript *)(NLAIAGENT::CMessageScript::IdMessageScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::CMessageScript *)(NLAIAGENT::CMessageScript::IdMessageScript.getFactory()->getClass())));		
	}

	CMessageClass::CMessageClass(const CMessageClass &c) : CAgentClass( c )
	{
		setBaseMethodCount(((NLAIAGENT::CMessageScript *)(NLAIAGENT::CMessageScript::IdMessageScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::CMessageScript *)(NLAIAGENT::CMessageScript::IdMessageScript.getFactory()->getClass())));		
	}	

	CMessageClass::CMessageClass()
	{
		setBaseMethodCount(((NLAIAGENT::CMessageScript *)(NLAIAGENT::CMessageScript::IdMessageScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::CMessageScript *)(NLAIAGENT::CMessageScript::IdMessageScript.getFactory()->getClass())));
	}


	const NLAIC::IBasicType *CMessageClass::clone() const
	{
		NLAIC::IBasicType *clone = new CMessageClass(*this);
		return clone;
	}

	const NLAIC::IBasicType *CMessageClass::newInstance() const
	{
		NLAIC::IBasicType *instance = new CMessageClass();
		return instance;
	}

	void CMessageClass::getDebugString(char *t) const
	{
	}

	NLAIAGENT::IObjectIA *CMessageClass::buildNewInstance() const
	{
		// Création des composants statiques
		std::list<NLAIAGENT::IObjectIA *> components;
		createBaseClassComponents( components );

		// Création du message
		NLAIAGENT::CMessageScript *instance = new NLAIAGENT::CMessageScript( components,  (CMessageClass *) this );
		return instance;
	}

	CMessageClass::~CMessageClass()
	{
	}

//#########################################
//		CMsgNotifyParentClass
//#########################################

	CMsgNotifyParentClass::CMsgNotifyParentClass(const NLAIC::CIdentType &id):CMessageClass(id)
	{
		setBaseObjectInstance((NLAIAGENT::IObjectIA *)NLAIAGENT::CNotifyParentScript::IdNotifyParentScript.getFactory()->getClass());		
		registerComponent(NLAIAGENT::CStringVarName("Float"),NLAIAGENT::CStringVarName("ParentState"));
		registerComponent(NLAIAGENT::CStringVarName("GenericAgent"),NLAIAGENT::CStringVarName("Parent"));
		setInheritanceName(NLAIAGENT::CStringVarName("Message"));
	}

	CMsgNotifyParentClass::CMsgNotifyParentClass()
	{		
		setBaseObjectInstance((NLAIAGENT::IObjectIA *)NLAIAGENT::CNotifyParentScript::IdNotifyParentScript.getFactory()->getClass());		
		registerComponent(NLAIAGENT::CStringVarName("Float"),NLAIAGENT::CStringVarName("ParentState"));
		registerComponent(NLAIAGENT::CStringVarName("GenericAgent"),NLAIAGENT::CStringVarName("Parent"));
		setInheritanceName(NLAIAGENT::CStringVarName("Message"));
	}
	
	const NLAIC::IBasicType *CMsgNotifyParentClass::clone() const
	{
		return new CMsgNotifyParentClass();
	}
	const NLAIC::IBasicType *CMsgNotifyParentClass::newInstance() const
	{
		return new CMsgNotifyParentClass();
	}

	NLAIAGENT::IObjectIA *CMsgNotifyParentClass::buildNewInstance() const
	{
		return new NLAIAGENT::CNotifyParentScript( (CMessageClass *) this );
		std::list<NLAIAGENT::IObjectIA *> components;
		createBaseClassComponents( components );

		// Création du message
		return new NLAIAGENT::CNotifyParentScript( components,  (CMessageClass *) this );		
	}	
	
}
