/** \file interpret_object_message.cpp
 *
 * $Id: interpret_object_message.cpp,v 1.13 2001/03/07 11:23:49 chafik Exp $
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
#include "nel/ai/agent/msg_goal.h"
#include "nel/ai/agent/msg_debug.h"
#include "nel/ai/agent/msg_on_change.h"

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
	


//#########################################
//		CGoalMsgClass
//#########################################

	CGoalMsgClass::CGoalMsgClass(const NLAIC::CIdentType &id):CMessageClass(id)
	{
		setBaseObjectInstance((NLAIAGENT::IObjectIA *)NLAIAGENT::CGoalMsg::IdGoalMsg.getFactory()->getClass());		
		registerComponent(NLAIAGENT::CStringVarName("Float"),NLAIAGENT::CStringVarName("ParentState"));
		registerComponent(NLAIAGENT::CStringVarName("GenericAgent"),NLAIAGENT::CStringVarName("Parent"));
		setInheritanceName(NLAIAGENT::CStringVarName("Message"));
	}

	CGoalMsgClass::CGoalMsgClass() : CMessageClass()
	{		
		setBaseObjectInstance((NLAIAGENT::IObjectIA *)NLAIAGENT::CGoalMsg::IdGoalMsg.getFactory()->getClass());		
		registerComponent(NLAIAGENT::CStringVarName("Float"),NLAIAGENT::CStringVarName("ParentState"));
		registerComponent(NLAIAGENT::CStringVarName("GenericAgent"),NLAIAGENT::CStringVarName("Parent"));
		setInheritanceName(NLAIAGENT::CStringVarName("Message"));
	}
	
	const NLAIC::IBasicType *CGoalMsgClass::clone() const
	{
		return new CGoalMsgClass();
	}

	const NLAIC::IBasicType *CGoalMsgClass::newInstance() const
	{
		return new CGoalMsgClass();
	}

	NLAIAGENT::IObjectIA *CGoalMsgClass::buildNewInstance() const
	{
//		return new NLAIAGENT::CGoalMsg( (CMessageClass *) this );
		std::list<NLAIAGENT::IObjectIA *> components;
		createBaseClassComponents( components );

		// Création du message
		NLAIAGENT::IObjectIA *x = new NLAIAGENT::CGoalMsg( components,  (CMessageClass *) this );		
		return x;
	}	
<<<<<<< interpret_object_message.cpp
//#########################################
//		CDebugMsgClass
//#########################################
	CDebugMsgClass::CDebugMsgClass(const NLAIC::CIdentType &id):CMessageClass(id)
	{
		setBaseObjectInstance((NLAIAGENT::IObjectIA *)NLAIAGENT::CGoalMsg::IdGoalMsg.getFactory()->getClass());		
		registerComponent(NLAIAGENT::CStringVarName("Float"),NLAIAGENT::CStringVarName("ParentState"));
		registerComponent(NLAIAGENT::CStringVarName("GenericAgent"),NLAIAGENT::CStringVarName("Parent"));
		setInheritanceName(NLAIAGENT::CStringVarName("Message"));
	}

	CDebugMsgClass::CDebugMsgClass() : CMessageClass()
	{		
		setBaseObjectInstance((NLAIAGENT::IObjectIA *)NLAIAGENT::CGoalMsg::IdGoalMsg.getFactory()->getClass());		
		registerComponent(NLAIAGENT::CStringVarName("Float"),NLAIAGENT::CStringVarName("ParentState"));
		registerComponent(NLAIAGENT::CStringVarName("GenericAgent"),NLAIAGENT::CStringVarName("Parent"));
		setInheritanceName(NLAIAGENT::CStringVarName("Message"));
	}
	
	const NLAIC::IBasicType *CDebugMsgClass::clone() const
	{
		return new CDebugMsgClass();
	}

	const NLAIC::IBasicType *CDebugMsgClass::newInstance() const
	{
		return new CDebugMsgClass();
	}

	NLAIAGENT::IObjectIA *CDebugMsgClass::buildNewInstance() const
	{
//		return new NLAIAGENT::CGoalMsg( (CMessageClass *) this );
		std::list<NLAIAGENT::IObjectIA *> components;
		createBaseClassComponents( components );

		// Création du message
		NLAIAGENT::IObjectIA *x = new NLAIAGENT::CMsgDebug( components,  (CMessageClass *) this );		
		return x;
	}
=======

//#########################################
//		COnChangeMsgClass
//#########################################

	COnChangeMsgClass::COnChangeMsgClass(const NLAIC::CIdentType &id):CMessageClass(id)
	{
		setBaseObjectInstance((NLAIAGENT::IObjectIA *)NLAIAGENT::COnChangeMsg::IdOnChangeMsg.getFactory()->getClass());		
		registerComponent(NLAIAGENT::CStringVarName("Float"),NLAIAGENT::CStringVarName("ParentState"));
		registerComponent(NLAIAGENT::CStringVarName("GenericAgent"),NLAIAGENT::CStringVarName("Parent"));
		setInheritanceName(NLAIAGENT::CStringVarName("Message"));
	}

	COnChangeMsgClass::COnChangeMsgClass() : CMessageClass()
	{		
		setBaseObjectInstance((NLAIAGENT::IObjectIA *)NLAIAGENT::COnChangeMsg::IdOnChangeMsg.getFactory()->getClass());		
		registerComponent(NLAIAGENT::CStringVarName("Float"),NLAIAGENT::CStringVarName("ParentState"));
		registerComponent(NLAIAGENT::CStringVarName("GenericAgent"),NLAIAGENT::CStringVarName("Parent"));
		setInheritanceName(NLAIAGENT::CStringVarName("Message"));
	}
	
	const NLAIC::IBasicType *COnChangeMsgClass::clone() const
	{
		return new COnChangeMsgClass();
	}

	const NLAIC::IBasicType *COnChangeMsgClass::newInstance() const
	{
		return new COnChangeMsgClass();
	}

	NLAIAGENT::IObjectIA *COnChangeMsgClass::buildNewInstance() const
	{
		std::list<NLAIAGENT::IObjectIA *> components;
		createBaseClassComponents( components );

		// Création du message
		NLAIAGENT::IObjectIA *x = new NLAIAGENT::COnChangeMsg( components,  (CMessageClass *) this );		
		return x;
	}	
>>>>>>> 1.12
}
