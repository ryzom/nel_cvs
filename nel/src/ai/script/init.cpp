/** \file init.cpp
 *
 * $Id: init.cpp,v 1.9 2002/08/27 09:25:50 portier Exp $
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
#include "nel/ai/script/compilateur.h"
#include "nel/ai/script/interpret_object_message.h"
#include "nel/ai/script/interpret_message_action.h"
#include "nel/ai/script/interpret_message_getvalue.h"
#include "nel/ai/script/interpret_message_setvalue.h"
#include "nel/ai/script/interpret_message_connect.h"
#include "nel/ai/script/interpret_object_manager.h"
#include "nel/ai/script/interpret_actor.h"
#include "nel/ai/script/goal_path_class.h"
#include "nel/ai/logic/interpret_object_operator.h"
#include "nel/ai/script/libcode.h"
#include "nel/ai/script/test_method.h"
#include "nel/ai/script/type_def.h"
#include "nel/ai/script/object_unknown.h"
#include "nel/ai/script/interpret_fsm.h"



namespace NLAISCRIPT
{		
	void reinitClass(const IClassInterpret &c,const NLAIC::CIdentType &type)
	{
		NLAIC::IClassFactory &f = (NLAIC::IClassFactory&)*(type.getFactory());
		f.setClass(c);
		((IClassInterpret *)f.getClass())->setClassName(NLAIAGENT::CStringVarName((const char *)type));
	}


	void initExternalLib()
	{		
		CCallPrint p;
		CLibTest c;

		reinitClass(CAgentClass(CAgentClass::IdAgentClass), CAgentClass::IdAgentClass);

		reinitClass(COperatorClass(COperatorClass::IdOperatorClass),COperatorClass::IdOperatorClass);
		reinitClass(CFsmClass(CFsmClass::IdFsmClass),CFsmClass::IdFsmClass);
		reinitClass(CSeqFsmClass(CSeqFsmClass::IdSeqFsmClass),CSeqFsmClass::IdSeqFsmClass);
		reinitClass(COperatorClass(COperatorClass::IdOperatorClass),COperatorClass::IdOperatorClass);
		reinitClass(CActorClass(CActorClass::IdActorClass),CActorClass::IdActorClass);
		reinitClass(CGoalPathClass(CGoalPathClass::IdGoalPathClass),CGoalPathClass::IdGoalPathClass);
		reinitClass(CMessageClass(CMessageClass::IdMessageClass),CMessageClass::IdMessageClass);
		reinitClass(CMsgNotifyParentClass(CMsgNotifyParentClass::IdMsgNotifyParentClass),CMsgNotifyParentClass::IdMsgNotifyParentClass);
		reinitClass(CGoalMsgClass(CGoalMsgClass::IdGoalMsgClass),CGoalMsgClass::IdGoalMsgClass);
		reinitClass(CFactMsgClass(CFactMsgClass::IdFactMsgClass),CFactMsgClass::IdFactMsgClass);
		reinitClass(CCancelGoalMsgClass(CCancelGoalMsgClass::IdCancelGoalMsgClass),CCancelGoalMsgClass::IdCancelGoalMsgClass);
		reinitClass(CSuccessMsgClass(CSuccessMsgClass::IdSuccessMsgClass),CSuccessMsgClass::IdSuccessMsgClass);
		reinitClass(CFailureMsgClass(CFailureMsgClass::IdFailureMsgClass),CFailureMsgClass::IdFailureMsgClass);
		reinitClass(CGetValueMsgClass(CGetValueMsgClass::IdGetValueMsgClass),CGetValueMsgClass::IdGetValueMsgClass);
		reinitClass(CSetValueMsgClass(CSetValueMsgClass::IdSetValueMsgClass),CSetValueMsgClass::IdSetValueMsgClass);
		reinitClass(CConnectObjectValueMsgClass(CConnectObjectValueMsgClass::IdConnectObjectValueMsgClass),CConnectObjectValueMsgClass::IdConnectObjectValueMsgClass);
		reinitClass(CDebugMsgClass(CDebugMsgClass::IdDebugMsgClass),CDebugMsgClass::IdDebugMsgClass);
		reinitClass(CManagerClass(CManagerClass::IdManagerClass),CManagerClass::IdManagerClass);
		reinitClass(COnChangeMsgClass(COnChangeMsgClass::IdOnChangeMsgClass),COnChangeMsgClass::IdOnChangeMsgClass);

		NLAIAGENT::DigitalType::NullOperator.incRef();
	}
}
