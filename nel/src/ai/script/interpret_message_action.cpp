/** \file interpret_message_action.cpp
 *
 * $Id: interpret_message_action.cpp,v 1.6 2001/06/19 16:50:08 chafik Exp $
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
#include "nel/ai/agent/msg_action.h"

namespace NLAISCRIPT
{

//#########################################
//		CSuccessMsgClass
//#########################################

	CSuccessMsgClass::CSuccessMsgClass(const NLAIAGENT::IVarName &s):CMessageClass(s)
	{
		setBaseObjectInstance((NLAIAGENT::IObjectIA *)NLAIAGENT::CSuccessMsg::IdSuccessMsg.getFactory()->getClass());		
		registerComponent(NLAIAGENT::CStringVarName("Float"),NLAIAGENT::CStringVarName("Param"));
		setInheritanceName(NLAIAGENT::CStringVarName("Message"));
		setClassName(NLAIAGENT::CStringVarName("CSuccessMsgClass"));
		buildVTable();
	}

	CSuccessMsgClass::CSuccessMsgClass(const NLAIC::CIdentType &id):CMessageClass(id)
	{
		setBaseObjectInstance((NLAIAGENT::IObjectIA *)NLAIAGENT::CSuccessMsg::IdSuccessMsg.getFactory()->getClass());		
		registerComponent(NLAIAGENT::CStringVarName("Float"),NLAIAGENT::CStringVarName("Param"));
		setInheritanceName(NLAIAGENT::CStringVarName("Message"));
		setClassName(NLAIAGENT::CStringVarName("CSuccessMsgClass"));
		buildVTable();
	}

	CSuccessMsgClass::CSuccessMsgClass() : CMessageClass()
	{		
		setBaseObjectInstance((NLAIAGENT::IObjectIA *)NLAIAGENT::CSuccessMsg::IdSuccessMsg.getFactory()->getClass());		
		registerComponent(NLAIAGENT::CStringVarName("Float"),NLAIAGENT::CStringVarName("Param"));
		setInheritanceName(NLAIAGENT::CStringVarName("Message"));
		setClassName(NLAIAGENT::CStringVarName("CSuccessMsgClass"));
		buildVTable();
	}
	
	const NLAIC::IBasicType *CSuccessMsgClass::clone() const
	{
		return new CSuccessMsgClass();
	}

	const NLAIC::IBasicType *CSuccessMsgClass::newInstance() const
	{
		return new CSuccessMsgClass();
	}

	NLAIAGENT::IObjectIA *CSuccessMsgClass::buildNewInstance() const
	{
		std::list<NLAIAGENT::IObjectIA *> components;
		createBaseClassComponents( components );

		NLAIAGENT::IObjectIA *x = new NLAIAGENT::CSuccessMsg( components,  (CMessageClass *) this );		
		return x;
	}	

//#########################################
//		CFailureMsgClass
//#########################################

	CFailureMsgClass::CFailureMsgClass(const NLAIAGENT::IVarName &s):CMessageClass(s)
	{
		setBaseObjectInstance((NLAIAGENT::IObjectIA *)NLAIAGENT::CFailureMsg::IdFailureMsg.getFactory()->getClass());		
		registerComponent(NLAIAGENT::CStringVarName("Float"),NLAIAGENT::CStringVarName("Param"));
		setInheritanceName(NLAIAGENT::CStringVarName("Message"));
		setClassName(NLAIAGENT::CStringVarName("CFailureMsgClass"));
		buildVTable();
	}

	CFailureMsgClass::CFailureMsgClass(const NLAIC::CIdentType &id):CMessageClass(id)
	{
		setBaseObjectInstance((NLAIAGENT::IObjectIA *)NLAIAGENT::CFailureMsg::IdFailureMsg.getFactory()->getClass());		
		registerComponent(NLAIAGENT::CStringVarName("Float"),NLAIAGENT::CStringVarName("Param"));
		setInheritanceName(NLAIAGENT::CStringVarName("Message"));
		setClassName(NLAIAGENT::CStringVarName("CFailureMsgClass"));
		buildVTable();
	}

	CFailureMsgClass::CFailureMsgClass() : CMessageClass()
	{		
		setBaseObjectInstance((NLAIAGENT::IObjectIA *)NLAIAGENT::CFailureMsg::IdFailureMsg.getFactory()->getClass());		
		registerComponent(NLAIAGENT::CStringVarName("Float"),NLAIAGENT::CStringVarName("Param"));
		setInheritanceName(NLAIAGENT::CStringVarName("Message"));
		setClassName(NLAIAGENT::CStringVarName("CFailureMsgClass"));
		buildVTable();
	}
	
	const NLAIC::IBasicType *CFailureMsgClass::clone() const
	{
		return new CFailureMsgClass();
	}

	const NLAIC::IBasicType *CFailureMsgClass::newInstance() const
	{
		return new CFailureMsgClass();
	}

	NLAIAGENT::IObjectIA *CFailureMsgClass::buildNewInstance() const
	{
		std::list<NLAIAGENT::IObjectIA *> components;
		createBaseClassComponents( components );

		NLAIAGENT::IObjectIA *x = new NLAIAGENT::CFailureMsg( components,  (CMessageClass *) this );		
		return x;
	}	
}
