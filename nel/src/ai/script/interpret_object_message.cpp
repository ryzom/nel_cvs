/** \file interpret_object_message.cpp
 *
 * $Id: interpret_object_message.cpp,v 1.27 2002/05/27 09:23:04 chafik Exp $
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
#include "nel/ai/agent/msg_fact.h"
#include "nel/ai/agent/msg_debug.h"
#include "nel/ai/agent/msg_on_change.h"
#include "nel/ai/c/registry_type.h"

namespace NLAISCRIPT
{
	//###############################
	//CMessageClass
	//###############################
	CMessageClass::CMessageClass(const NLAIAGENT::IVarName &n) : CAgentClass(n)
	{
		if ( _IdType != NULL )
			_IdType->addObjectType( NLAIC::CTypeOfObject::tMessage );
		setBaseMethodCount(((NLAIAGENT::CMessageScript *)(NLAIAGENT::CMessageScript::IdMessageScript.getFactory()->getClass()))->getBaseMethodCount());		
		setBaseObjectInstance(((NLAIAGENT::CMessageScript *)(NLAIAGENT::CMessageScript::IdMessageScript.getFactory()->getClass())));		
	}
	
	CMessageClass::CMessageClass(const NLAIC::CIdentType &id): CAgentClass(id)
	{
		if ( _IdType != NULL )
			_IdType->addObjectType( NLAIC::CTypeOfObject::tMessage );
		setBaseMethodCount(((NLAIAGENT::CMessageScript *)(NLAIAGENT::CMessageScript::IdMessageScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::CMessageScript *)(NLAIAGENT::CMessageScript::IdMessageScript.getFactory()->getClass())));		
	}

	CMessageClass::CMessageClass(const NLAIAGENT::IVarName &n, const NLAIAGENT::IVarName &inheritance) : CAgentClass( inheritance )
	{
		if ( _IdType != NULL )
			_IdType->addObjectType( NLAIC::CTypeOfObject::tMessage );
		setBaseMethodCount(((NLAIAGENT::CMessageScript *)(NLAIAGENT::CMessageScript::IdMessageScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::CMessageScript *)(NLAIAGENT::CMessageScript::IdMessageScript.getFactory()->getClass())));		
	}

	CMessageClass::CMessageClass(const CMessageClass &c) : CAgentClass( c )
	{
		if ( _IdType != NULL )
			_IdType->addObjectType( NLAIC::CTypeOfObject::tMessage );
		setBaseMethodCount(((NLAIAGENT::CMessageScript *)(NLAIAGENT::CMessageScript::IdMessageScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::CMessageScript *)(NLAIAGENT::CMessageScript::IdMessageScript.getFactory()->getClass())));		
	}	

	CMessageClass::CMessageClass()
	{
		if ( _IdType != NULL )
			_IdType->addObjectType( NLAIC::CTypeOfObject::tMessage );
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

	void CMessageClass::getDebugString(std::string &t) const
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

	const NLAIC::CIdentType &CMessageClass::getType() const
	{
		/*if ( _IdType != NULL )
			_IdType->addObjectType( NLAIC::CTypeOfObject::tMessage );*/

		return *_IdType;
	}		

	CMessageClass::~CMessageClass()
	{
	}

//#########################################
//		CMsgNotifyParentClass
//#########################################
	CMsgNotifyParentClass::CMsgNotifyParentClass(const NLAIAGENT::IVarName &s):CMessageClass(s)
	{
		setBaseObjectInstance((NLAIAGENT::IObjectIA *)NLAIAGENT::CNotifyParentScript::IdNotifyParentScript.getFactory()->getClass());		
		registerComponent(NLAIAGENT::CStringVarName("Float"),NLAIAGENT::CStringVarName("ParentState"));
		registerComponent(NLAIAGENT::CStringVarName("GenericAgent"),NLAIAGENT::CStringVarName("Parent"));
		setInheritanceName(NLAIAGENT::CStringVarName("Message"));
		setClassName(NLAIAGENT::CStringVarName("CMsgNotifyParentClass"));
		buildVTable();
	}

	CMsgNotifyParentClass::CMsgNotifyParentClass(const NLAIC::CIdentType &id):CMessageClass(id)
	{
		setBaseObjectInstance((NLAIAGENT::IObjectIA *)NLAIAGENT::CNotifyParentScript::IdNotifyParentScript.getFactory()->getClass());		
		registerComponent(NLAIAGENT::CStringVarName("Float"),NLAIAGENT::CStringVarName("ParentState"));
		registerComponent(NLAIAGENT::CStringVarName("GenericAgent"),NLAIAGENT::CStringVarName("Parent"));
		setInheritanceName(NLAIAGENT::CStringVarName("Message"));
		setClassName(NLAIAGENT::CStringVarName("CMsgNotifyParentClass"));
		buildVTable();
	}

	CMsgNotifyParentClass::CMsgNotifyParentClass()
	{		
		setBaseObjectInstance((NLAIAGENT::IObjectIA *)NLAIAGENT::CNotifyParentScript::IdNotifyParentScript.getFactory()->getClass());		
		registerComponent(NLAIAGENT::CStringVarName("Float"),NLAIAGENT::CStringVarName("ParentState"));
		registerComponent(NLAIAGENT::CStringVarName("GenericAgent"),NLAIAGENT::CStringVarName("Parent"));
		setInheritanceName(NLAIAGENT::CStringVarName("Message"));
		setClassName(NLAIAGENT::CStringVarName("CMsgNotifyParentClass"));
		buildVTable();
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
	CGoalMsgClass::CGoalMsgClass(const NLAIAGENT::IVarName &s):CMessageClass(s)
	{
		setBaseObjectInstance((NLAIAGENT::IObjectIA *)NLAIAGENT::CGoalMsg::IdGoalMsg.getFactory()->getClass());		
		registerComponent(NLAIAGENT::CStringVarName("GoalObject"),NLAIAGENT::CStringVarName("Goal"));
		setInheritanceName(NLAIAGENT::CStringVarName("Message"));
		setClassName(NLAIAGENT::CStringVarName("CGoalMsgClass"));
		buildVTable();
	}
	CGoalMsgClass::CGoalMsgClass(const NLAIC::CIdentType &id):CMessageClass(id)
	{
		setBaseObjectInstance((NLAIAGENT::IObjectIA *)NLAIAGENT::CGoalMsg::IdGoalMsg.getFactory()->getClass());		
		registerComponent(NLAIAGENT::CStringVarName("GoalObject"),NLAIAGENT::CStringVarName("Goal"));
		setInheritanceName(NLAIAGENT::CStringVarName("Message"));
		setClassName(NLAIAGENT::CStringVarName("CGoalMsgClass"));
		buildVTable();
	}

	CGoalMsgClass::CGoalMsgClass()
	{		
		setBaseObjectInstance((NLAIAGENT::IObjectIA *)NLAIAGENT::CGoalMsg::IdGoalMsg.getFactory()->getClass());		
		registerComponent(NLAIAGENT::CStringVarName("GoalObject"),NLAIAGENT::CStringVarName("Goal"));
		setInheritanceName(NLAIAGENT::CStringVarName("Message"));
		setClassName(NLAIAGENT::CStringVarName("CGoalMsgClass"));
		buildVTable();
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


//#########################################
//		CCancelGoalMsgClass
//#########################################

	CCancelGoalMsgClass::CCancelGoalMsgClass(const NLAIAGENT::IVarName &s):CMessageClass(s)
	{
		setBaseObjectInstance((NLAIAGENT::IObjectIA *)NLAIAGENT::CCancelGoalMsg::IdCancelGoalMsg.getFactory()->getClass());		
		registerComponent(NLAIAGENT::CStringVarName("GoalObject"),NLAIAGENT::CStringVarName("Goal"));
		setInheritanceName(NLAIAGENT::CStringVarName("Message"));
		setClassName(NLAIAGENT::CStringVarName("CCancelGoalMsgClass"));
		buildVTable();
	}

	CCancelGoalMsgClass::CCancelGoalMsgClass(const NLAIC::CIdentType &id):CMessageClass(id)
	{
		setBaseObjectInstance((NLAIAGENT::IObjectIA *)NLAIAGENT::CCancelGoalMsg::IdCancelGoalMsg.getFactory()->getClass());		
		registerComponent(NLAIAGENT::CStringVarName("GoalObject"),NLAIAGENT::CStringVarName("Goal"));
		setInheritanceName(NLAIAGENT::CStringVarName("Message"));
		setClassName(NLAIAGENT::CStringVarName("CCancelGoalMsgClass"));
		buildVTable();
	}

	CCancelGoalMsgClass::CCancelGoalMsgClass()
	{		
		setBaseObjectInstance((NLAIAGENT::IObjectIA *)NLAIAGENT::CCancelGoalMsg::IdCancelGoalMsg.getFactory()->getClass());		
		registerComponent(NLAIAGENT::CStringVarName("GoalObject"),NLAIAGENT::CStringVarName("Goal"));
		setInheritanceName(NLAIAGENT::CStringVarName("Message"));
		setClassName(NLAIAGENT::CStringVarName("CCancelGoalMsgClass"));
		buildVTable();
	}
	
	const NLAIC::IBasicType *CCancelGoalMsgClass::clone() const
	{
		return new CCancelGoalMsgClass();
	}

	const NLAIC::IBasicType *CCancelGoalMsgClass::newInstance() const
	{
		return new CCancelGoalMsgClass();
	}

	NLAIAGENT::IObjectIA *CCancelGoalMsgClass::buildNewInstance() const
	{
//		return new NLAIAGENT::CGoalMsg( (CMessageClass *) this );
		std::list<NLAIAGENT::IObjectIA *> components;
		createBaseClassComponents( components );

		// Création du message
		NLAIAGENT::IObjectIA *x = new NLAIAGENT::CCancelGoalMsg( components,  (CMessageClass *) this );
		return x;
	}	

//#########################################
//		CFactMsgClass
//#########################################

	CFactMsgClass::CFactMsgClass(const NLAIAGENT::IVarName &s):CMessageClass(s)
	{
		setBaseObjectInstance((NLAIAGENT::IObjectIA *)NLAIAGENT::CFactMsg::IdFactMsg.getFactory()->getClass());		
		registerComponent(NLAIAGENT::CStringVarName("FactObject"),NLAIAGENT::CStringVarName("Fact"));
		setInheritanceName(NLAIAGENT::CStringVarName("Message"));
		setClassName(NLAIAGENT::CStringVarName("CFactMsgClass"));
		buildVTable();
	}

	CFactMsgClass::CFactMsgClass(const NLAIC::CIdentType &id):CMessageClass(id)
	{
		setBaseObjectInstance((NLAIAGENT::IObjectIA *)NLAIAGENT::CFactMsg::IdFactMsg.getFactory()->getClass());		
		registerComponent(NLAIAGENT::CStringVarName("FactObject"),NLAIAGENT::CStringVarName("Fact"));
		setInheritanceName(NLAIAGENT::CStringVarName("Message"));
		setClassName(NLAIAGENT::CStringVarName("CFactMsgClass"));
		buildVTable();
	}

	CFactMsgClass::CFactMsgClass() : CMessageClass()
	{		
		setBaseObjectInstance((NLAIAGENT::IObjectIA *)NLAIAGENT::CGoalMsg::IdGoalMsg.getFactory()->getClass());		
		registerComponent(NLAIAGENT::CStringVarName("FactObject"),NLAIAGENT::CStringVarName("Fact"));
		setInheritanceName(NLAIAGENT::CStringVarName("Message"));
		setClassName(NLAIAGENT::CStringVarName("CFactMsgClass"));
		buildVTable();
	}
	
	const NLAIC::IBasicType *CFactMsgClass::clone() const
	{
		return new CFactMsgClass();
	}

	const NLAIC::IBasicType *CFactMsgClass::newInstance() const
	{
		return new CFactMsgClass();
	}

	NLAIAGENT::IObjectIA *CFactMsgClass::buildNewInstance() const
	{
		std::list<NLAIAGENT::IObjectIA *> components;
		createBaseClassComponents( components );

		NLAIAGENT::IObjectIA *x = new NLAIAGENT::CFactMsg( components,  (CMessageClass *) this );		
		return x;
	}	


//#########################################
//		CDebugMsgClass
//#########################################
	
	CDebugMsgClass::CDebugMsgClass(const NLAIAGENT::IVarName &s):CMessageClass(s)
	{
		setBaseObjectInstance((NLAIAGENT::IObjectIA *)NLAIAGENT::CGoalMsg::IdGoalMsg.getFactory()->getClass());		
		registerComponent(NLAIAGENT::CStringVarName("Float"),NLAIAGENT::CStringVarName("ParentState"));
		registerComponent(NLAIAGENT::CStringVarName("GenericAgent"),NLAIAGENT::CStringVarName("Parent"));
		setInheritanceName(NLAIAGENT::CStringVarName("Message"));
		setClassName(NLAIAGENT::CStringVarName("CDebugMsgClass"));
		buildVTable();
	}
	CDebugMsgClass::CDebugMsgClass(const NLAIC::CIdentType &id):CMessageClass(id)
	{
		setBaseObjectInstance((NLAIAGENT::IObjectIA *)NLAIAGENT::CGoalMsg::IdGoalMsg.getFactory()->getClass());		
		registerComponent(NLAIAGENT::CStringVarName("Float"),NLAIAGENT::CStringVarName("ParentState"));
		registerComponent(NLAIAGENT::CStringVarName("GenericAgent"),NLAIAGENT::CStringVarName("Parent"));
		setInheritanceName(NLAIAGENT::CStringVarName("Message"));
		setClassName(NLAIAGENT::CStringVarName("CDebugMsgClass"));
		buildVTable();
	}

	CDebugMsgClass::CDebugMsgClass() : CMessageClass()
	{		
		setBaseObjectInstance((NLAIAGENT::IObjectIA *)NLAIAGENT::CGoalMsg::IdGoalMsg.getFactory()->getClass());		
		registerComponent(NLAIAGENT::CStringVarName("Float"),NLAIAGENT::CStringVarName("ParentState"));
		registerComponent(NLAIAGENT::CStringVarName("GenericAgent"),NLAIAGENT::CStringVarName("Parent"));
		setInheritanceName(NLAIAGENT::CStringVarName("Message"));
		setClassName(NLAIAGENT::CStringVarName("CDebugMsgClass"));
		buildVTable();
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

//#########################################
//		COnChangeMsgClass
//#########################################

	COnChangeMsgClass::COnChangeMsgClass(const NLAIAGENT::IVarName &s):CMessageClass(s)
	{
		setBaseObjectInstance((NLAIAGENT::IObjectIA *)NLAIAGENT::COnChangeMsg::IdOnChangeMsg.getFactory()->getClass());		
		/*registerComponent(NLAIAGENT::CStringVarName("Float"),NLAIAGENT::CStringVarName("ParentState"));
		registerComponent(NLAIAGENT::CStringVarName("GenericAgent"),NLAIAGENT::CStringVarName("Parent"));*/
		setInheritanceName(NLAIAGENT::CStringVarName("Message"));
	}


	COnChangeMsgClass::COnChangeMsgClass(const NLAIC::CIdentType &id):CMessageClass(id)
	{
		setBaseObjectInstance((NLAIAGENT::IObjectIA *)NLAIAGENT::COnChangeMsg::IdOnChangeMsg.getFactory()->getClass());		
		/*registerComponent(NLAIAGENT::CStringVarName("Float"),NLAIAGENT::CStringVarName("ParentState"));
		registerComponent(NLAIAGENT::CStringVarName("GenericAgent"),NLAIAGENT::CStringVarName("Parent"));*/
		setInheritanceName(NLAIAGENT::CStringVarName("Message"));
	}

	COnChangeMsgClass::COnChangeMsgClass() : CMessageClass()
	{		
		setBaseObjectInstance((NLAIAGENT::IObjectIA *)NLAIAGENT::COnChangeMsg::IdOnChangeMsg.getFactory()->getClass());		
		/*registerComponent(NLAIAGENT::CStringVarName("Float"),NLAIAGENT::CStringVarName("ParentState"));
		registerComponent(NLAIAGENT::CStringVarName("GenericAgent"),NLAIAGENT::CStringVarName("Parent"));*/
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
}
