/** \file init.cpp
 *
 * $Id: init.cpp,v 1.3 2001/03/28 12:15:14 portier Exp $
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
#include "nel/ai/script/interpret_object_manager.h"
#include "nel/ai/script/interpret_actor.h"
#include "nel/ai/logic/interpret_object_operator.h"
#include "nel/ai/script/libcode.h"
#include "nel/ai/script/test_method.h"
#include "nel/ai/script/type_def.h"
#include "nel/ai/script/object_unknown.h"
#include "nel/ai/script/gd_agent_class.h"	
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
		reinitClass(CGDAgentClass(CGDAgentClass::IdGDAgentClass),CGDAgentClass::IdGDAgentClass);
		reinitClass(COperatorClass(COperatorClass::IdOperatorClass),COperatorClass::IdOperatorClass);
		reinitClass(CFsmClass(CFsmClass::IdFsmClass),CFsmClass::IdFsmClass);
		reinitClass(CSeqFsmClass(CSeqFsmClass::IdSeqFsmClass),CSeqFsmClass::IdSeqFsmClass);
		reinitClass(COperatorClass(COperatorClass::IdOperatorClass),COperatorClass::IdOperatorClass);
		reinitClass(CActorClass(CActorClass::IdActorClass),CActorClass::IdActorClass);
		reinitClass(CMessageClass(CMessageClass::IdMessageClass),CMessageClass::IdMessageClass);
		reinitClass(CMsgNotifyParentClass(CMsgNotifyParentClass::IdMsgNotifyParentClass),CMsgNotifyParentClass::IdMsgNotifyParentClass);
		reinitClass(CGoalMsgClass(CGoalMsgClass::IdGoalMsgClass),CGoalMsgClass::IdGoalMsgClass);
		reinitClass(CSuccessMsgClass(CSuccessMsgClass::IdSuccessMsgClass),CSuccessMsgClass::IdSuccessMsgClass);
		reinitClass(CFailureMsgClass(CFailureMsgClass::IdFailureMsgClass),CFailureMsgClass::IdFailureMsgClass);
		reinitClass(CDebugMsgClass(CDebugMsgClass::IdDebugMsgClass),CDebugMsgClass::IdDebugMsgClass);
		reinitClass(CManagerClass(CManagerClass::IdManagerClass),CManagerClass::IdManagerClass);
		reinitClass(COnChangeMsgClass(COnChangeMsgClass::IdOnChangeMsgClass),COnChangeMsgClass::IdOnChangeMsgClass);

		NLAIAGENT::DigitalType::NullOperator.incRef();

		/*NLAIC::CSelfClassFactory &f = (NLAIC::CSelfClassFactory&)*((CAgentClass::IdAgentClass).getFactory());
		f = NLAISCRIPT::CAgentClass(CAgentClass::IdAgentClass);
		((CAgentClass *)f.getClass())->setClassName(NLAIAGENT::CStringVarName((const char *)CAgentClass::IdAgentClass));*/

		/*NLAIC::CSelfClassFactory &fgda = (NLAIC::CSelfClassFactory&)*((CGDAgentClass::IdGDAgentClass).getFactory());
		fgda = NLAISCRIPT::CGDAgentClass(CGDAgentClass::IdGDAgentClass);
		((CGDAgentClass *)f.getClass())->setClassName(NLAIAGENT::CStringVarName((const char *)CGDAgentClass::IdGDAgentClass));

		NLAIC::CSelfClassFactory &fop = (NLAIC::CSelfClassFactory&)*((COperatorClass::IdOperatorClass).getFactory());
		fop = NLAISCRIPT::COperatorClass(COperatorClass::IdOperatorClass);
		((COperatorClass *)f.getClass())->setClassName(NLAIAGENT::CStringVarName((const char *)COperatorClass::IdOperatorClass));

		NLAIC::CSelfClassFactory &fc = (NLAIC::CSelfClassFactory&)*((CActorClass::IdActorClass).getFactory());
		fc = NLAISCRIPT::CActorClass(CActorClass::IdActorClass);
		((CActorClass *)fc.getClass())->setClassName(NLAIAGENT::CStringVarName((const char *)CActorClass::IdActorClass));

		NLAIC::CSelfClassFactory &fi = (NLAIC::CSelfClassFactory&)*((CMessageClass::IdMessageClass).getFactory());
		fi = CMessageClass(CMessageClass::IdMessageClass);
		((CMessageClass *)fi.getClass())->setClassName(NLAIAGENT::CStringVarName((const char *)CMessageClass::IdMessageClass));

		NLAIC::CSelfClassFactory &fh = (NLAIC::CSelfClassFactory&)*((CMsgNotifyParentClass::IdMsgNotifyParentClass).getFactory());
		fh = CMsgNotifyParentClass(CMsgNotifyParentClass::IdMsgNotifyParentClass);
		((CMsgNotifyParentClass *)fh.getClass())->setClassName(NLAIAGENT::CStringVarName((const char *)CMsgNotifyParentClass::IdMsgNotifyParentClass));

		NLAIC::CSelfClassFactory &fg = (NLAIC::CSelfClassFactory&)*((CGoalMsgClass::IdGoalMsgClass).getFactory());
		fg = CGoalMsgClass(CGoalMsgClass::IdGoalMsgClass);
		((CGoalMsgClass *)fh.getClass())->setClassName(NLAIAGENT::CStringVarName((const char *)CGoalMsgClass::IdGoalMsgClass));

		NLAIC::CSelfClassFactory &fj = (NLAIC::CSelfClassFactory&)*((CDebugMsgClass::IdDebugMsgClass).getFactory());
		fj = CDebugMsgClass(CDebugMsgClass::IdDebugMsgClass);
		((CDebugMsgClass *)fj.getClass())->setClassName(NLAIAGENT::CStringVarName((const char *)CDebugMsgClass::IdDebugMsgClass));

		NLAIC::CSelfClassFactory &fa = (NLAIC::CSelfClassFactory&)*((CManagerClass::IdManagerClass).getFactory());
		fa = CManagerClass(CManagerClass::IdManagerClass);
		((CManagerClass *)fa.getClass())->setClassName(NLAIAGENT::CStringVarName((const char *)CManagerClass::IdManagerClass));

		NLAIC::CSelfClassFactory &fl = (NLAIC::CSelfClassFactory&)*((COnChangeMsgClass::IdOnChangeMsgClass).getFactory());
		fl = COnChangeMsgClass(COnChangeMsgClass::IdOnChangeMsgClass);
		((COnChangeMsgClass *)fa.getClass())->setClassName(NLAIAGENT::CStringVarName((const char *)COnChangeMsgClass::IdOnChangeMsgClass));
		NLAIAGENT::DigitalType::NullOperator.incRef();*/
	}
}
