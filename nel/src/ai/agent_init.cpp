/** \file agent_init.cpp
 * <File description>
 *
 * $Id: agent_init.cpp,v 1.2 2001/03/29 07:52:48 chafik Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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
#include "nel/ai/agent/msg_debug.h"
#include "nel/ai/agent/msg_action.h"
#include "nel/ai/agent/msg_on_change.h"
#include "nel/ai/agent/gd_agent_script.h"
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

	CCharacterNoeud::initClass();
	CCharacterChild::initClass();
}

void registerLibClass()
{

}