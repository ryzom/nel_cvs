/** \file interpret_message_getvalue.cpp
 *
 * $Id: interpret_message_getvalue.cpp,v 1.3 2002/03/11 14:00:53 chafik Exp $
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

#include "nel/ai/agent/msg_getvalue.h"
#include "nel/ai/c/registry_class.h"
#include "nel/ai/agent/message_script.h"
#include "nel/ai/script/lexsupport.h"
#include "nel/ai/script/interpret_message_getvalue.h"


namespace NLAISCRIPT
{
	CGetValueMsgClass::CGetValueMsgClass(const NLAIAGENT::IVarName &s):CMessageClass(s)
	{
		setBaseObjectInstance((NLAIAGENT::IObjectIA *)NLAIAGENT::CGetValueMsg::IdGetValueMsg.getFactory()->getClass());		
		setInheritanceName(NLAIAGENT::CStringVarName("Message"));
		setClassName(NLAIAGENT::CStringVarName("CGetValueMsgClass"));
		buildVTable();
	}

	CGetValueMsgClass::CGetValueMsgClass(const NLAIC::CIdentType &id):CMessageClass(id)
	{
		setBaseObjectInstance((NLAIAGENT::IObjectIA *)NLAIAGENT::CGetValueMsg::IdGetValueMsg.getFactory()->getClass());		
		setInheritanceName(NLAIAGENT::CStringVarName("Message"));
		setClassName(NLAIAGENT::CStringVarName("CGetValueMsgClass"));
		buildVTable();
	}

	CGetValueMsgClass::CGetValueMsgClass() : CMessageClass()
	{		
		setBaseObjectInstance((NLAIAGENT::IObjectIA *)NLAIAGENT::CGetValueMsg::IdGetValueMsg.getFactory()->getClass());
		setInheritanceName(NLAIAGENT::CStringVarName("Message"));
		setClassName(NLAIAGENT::CStringVarName("CGetValueMsgClass"));
		buildVTable();
	}
	
	const NLAIC::IBasicType *CGetValueMsgClass::clone() const
	{
		return new CGetValueMsgClass();
	}

	const NLAIC::IBasicType *CGetValueMsgClass::newInstance() const
	{
		return new CGetValueMsgClass();
	}

	NLAIAGENT::IObjectIA *CGetValueMsgClass::buildNewInstance() const
	{
		std::list<NLAIAGENT::IObjectIA *> components;
		createBaseClassComponents( components );

		NLAIAGENT::IObjectIA *x = new NLAIAGENT::CGetValueMsg( components,  (CMessageClass *) this );		
		return x;
	}	
}	// NLAISCRIPT