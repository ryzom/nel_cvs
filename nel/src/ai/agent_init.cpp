/** \file agent_init.cpp
 * <File description>
 *
 * $Id: agent_init.cpp,v 1.15 2003/01/23 15:41:08 chafik Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX D.T.C. SYSTEM.
 * NEVRAX D.T.C. SYSTEM is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX D.T.C. SYSTEM is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX D.T.C. SYSTEM; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "nel/ai/script/compilateur.h"
#include "nel/ai/agent/agent.h"
#include "nel/ai/agent/agent_script.h"
#include "nel/ai/agent/message_script.h"
#include "nel/ai/logic/operator_script.h"
#include "nel/ai/logic/interpret_object_operator.h"
#include "nel/ai/script/interpret_object_manager.h"
#include "nel/ai/script/interpret_object_message.h"
#include "nel/ai/script/interpret_message_action.h"
#include "nel/ai/agent/main_agent_script.h"
#include "nel/ai/agent/agent_digital.h"
#include "nel/ai/agent/msg_group.h"
#include "nel/ai/agent/agent_local_mailer.h"
#include "nel/ai/agent/agent_proxy_mailer.h"
#include "nel/ai/logic/logic.h"
#include "nel/ai/fuzzy/fuzzy.h"
#include "nel/ai/fuzzy/fuzzy_script.h"
#include "nel/ai/agent/actor.h"
#include "nel/ai/agent/actor_script.h"
#include "nel/ai/script/interpret_actor.h"
#include "nel/ai/agent/performative.h"
#include "nel/ai/agent/object_ident.h"


#include "nel/ai/c/registry_class.h"
#include "nel/ai/agent/msg_notify.h"
#include "nel/ai/agent/msg_goal.h"
#include "nel/ai/agent/msg_fact.h"
#include "nel/ai/agent/msg_action.h"
#include "nel/ai/agent/msg_on_change.h"
#include "nel/ai/character/character.h"

using namespace NLAIAGENT;
using namespace NLAICHARACTER;

void staticInitAgent()
{
	CObjectIdent::IdObjectIdent = new NLAIC::CIdentType("ObjectIdent",NLAIC::CSelfClassFactory(CObjectIdent("0:0:0")),
														NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),
														NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));

	const IAgent staticAgent(NULL);
	CLocWordNumRef::IdLocWordNumRef = new NLAIC::CIdentType("LocWordNumRef",NLAIC::CSelfClassFactory((const NLAIC::IBasicInterface &)staticAgent),
																			NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),
																			NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));

	IAgent::IdAgent = new NLAIC::CIdentType("GenericAgent", NLAIC::CSelfClassFactory((const NLAIC::IBasicInterface &)staticAgent), 
															NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tAgent),
															NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));
	
	
}

void registerLibClass()
{	
}

void staticReleaseLibClass()
{	
	delete IAgent::IdAgent;
	delete CObjectIdent::IdObjectIdent;
	delete CLocWordNumRef::IdLocWordNumRef;	
}

namespace NLAIE
{		
	
	CExceptionContainer::CExceptionContainer(const CExceptionContainer &e):IException(e)
	{		
	}
	
	CExceptionContainer::CExceptionContainer(const std::string &name):IException(name)
	{
	}
	const IException *CExceptionContainer::clone() const
	{
		return new CExceptionContainer(*this);
	}

	const IException *CExceptionIndexError::clone() const
	{
		return new CExceptionIndexError();
	}
	const IException *CExceptionIndexHandeledError::clone() const
	{
		return new CExceptionIndexHandeledError();
	}


	CExceptionUnRegisterClassError::CExceptionUnRegisterClassError(const CExceptionUnRegisterClassError &e):IException(e)
	{		
	}

	CExceptionUnRegisterClassError::CExceptionUnRegisterClassError(const std::string &name):IException(name)
	{		
	}
	const IException *CExceptionUnRegisterClassError::clone() const
	{
		return new CExceptionUnRegisterClassError(*this);
	}

	const IException *CExceptionObjectNotFoundError::clone() const
	{
		return new CExceptionObjectNotFoundError(*this);
	}

	CExceptionNotImplemented::CExceptionNotImplemented(const CExceptionNotImplemented &e):IException(e)
	{	
	}
		
	CExceptionNotImplemented::CExceptionNotImplemented(const std::string &name):IException(name)
	{		
	}

	const IException *CExceptionNotImplemented::clone() const
	{
		return new CExceptionNotImplemented(*this);
	}


	CExceptionUnReference::CExceptionUnReference(const CExceptionUnReference &e):IException(e)
	{		
	}

	CExceptionUnReference::CExceptionUnReference(const std::string &text):IException(text)
	{		
	}

	CExceptionUnReference::CExceptionUnReference(char *text):IException((const char *)text)
	{	
	}

	const IException *CExceptionUnReference::clone() const
	{
		return new CExceptionUnReference(*this);
	}

	const IException *CExceptionAllReadyExist::clone() const
	{
		return new CExceptionAllReadyExist(*this);
	}
}
