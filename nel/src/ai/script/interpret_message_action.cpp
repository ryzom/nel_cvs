/** \file interpret_message_action.cpp
 *
 * $Id: interpret_message_action.cpp,v 1.2 2001/03/07 13:51:04 chafik Exp $
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
#include "nel/ai/script/interpret_message_action.h"

namespace NLAISCRIPT
{

//#########################################
//		CMsgSuccessClass
//#########################################

	CMsgSuccessClass::CMsgSuccessClass(const NLAIC::CIdentType &id):CMessageClass(id)
	{
		/*setBaseObjectInstance((NLAIAGENT::IObjectIA *)NLAIAGENT::COnChangeMsg::IdOnChangeMsg.getFactory()->getClass());		
		registerComponent(NLAIAGENT::CStringVarName("Float"),NLAIAGENT::CStringVarName("ParentState"));
		registerComponent(NLAIAGENT::CStringVarName("GenericAgent"),NLAIAGENT::CStringVarName("Parent"));
		setInheritanceName(NLAIAGENT::CStringVarName("Message"));*/
	}

	CMsgSuccessClass::CMsgSuccessClass() : CMessageClass()
	{		
		/*setBaseObjectInstance((NLAIAGENT::IObjectIA *)NLAIAGENT::COnChangeMsg::IdOnChangeMsg.getFactory()->getClass());		
		registerComponent(NLAIAGENT::CStringVarName("Float"),NLAIAGENT::CStringVarName("ParentState"));
		registerComponent(NLAIAGENT::CStringVarName("GenericAgent"),NLAIAGENT::CStringVarName("Parent"));
		setInheritanceName(NLAIAGENT::CStringVarName("Message"));*/
	}
	
	const NLAIC::IBasicType *CMsgSuccessClass::clone() const
	{
		return new CMsgSuccessClass();
	}

	const NLAIC::IBasicType *CMsgSuccessClass::newInstance() const
	{
		return new CMsgSuccessClass();
	}

	NLAIAGENT::IObjectIA *CMsgSuccessClass::buildNewInstance() const
	{
		std::list<NLAIAGENT::IObjectIA *> components;
		createBaseClassComponents( components );

		// Création du message
		//NLAIAGENT::IObjectIA *x = new NLAIAGENT::COnChangeMsg( components,  (CMessageClass *) this );		
		//return x;

		//have to change.
		throw;
		return NULL;
	}	

//#########################################
//		CMsgFailureClass
//#########################################

	CMsgFailureClass::CMsgFailureClass(const NLAIC::CIdentType &id):CMessageClass(id)
	{
		/*setBaseObjectInstance((NLAIAGENT::IObjectIA *)NLAIAGENT::COnChangeMsg::IdOnChangeMsg.getFactory()->getClass());		
		registerComponent(NLAIAGENT::CStringVarName("Float"),NLAIAGENT::CStringVarName("ParentState"));
		registerComponent(NLAIAGENT::CStringVarName("GenericAgent"),NLAIAGENT::CStringVarName("Parent"));
		setInheritanceName(NLAIAGENT::CStringVarName("Message"));*/
	}

	CMsgFailureClass::CMsgFailureClass() : CMessageClass()
	{		
		/*setBaseObjectInstance((NLAIAGENT::IObjectIA *)NLAIAGENT::COnChangeMsg::IdOnChangeMsg.getFactory()->getClass());		
		registerComponent(NLAIAGENT::CStringVarName("Float"),NLAIAGENT::CStringVarName("ParentState"));
		registerComponent(NLAIAGENT::CStringVarName("GenericAgent"),NLAIAGENT::CStringVarName("Parent"));
		setInheritanceName(NLAIAGENT::CStringVarName("Message"));*/
	}
	
	const NLAIC::IBasicType *CMsgFailureClass::clone() const
	{
		return new CMsgFailureClass();
	}

	const NLAIC::IBasicType *CMsgFailureClass::newInstance() const
	{
		return new CMsgFailureClass();
	}

	NLAIAGENT::IObjectIA *CMsgFailureClass::buildNewInstance() const
	{
		std::list<NLAIAGENT::IObjectIA *> components;
		createBaseClassComponents( components );

		// Création du message
		//NLAIAGENT::IObjectIA *x = new NLAIAGENT::COnChangeMsg( components,  (CMessageClass *) this );		
		//return x;
		//have to change.
		throw;
		return NULL;
	}	
}
