/** \file msg_goal.cpp
 *
 * $Id: msg_goal.cpp,v 1.12 2003/01/13 16:58:59 chafik Exp $
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

#include "nel/ai/agent/msg_goal.h"
#include "nel/ai/agent/agent_digital.h"
#include "nel/ai/script/interpret_object_message.h"
#include "nel/ai/logic/goal.h"
#include "nel/ai/agent/object_type.h"
#include "nel/ai/script/codage.h"
#include "nel/ai/script/object_unknown.h"


namespace NLAIAGENT
{
	CGoalMsg::CGoalMsg( std::list<IObjectIA *> &l, NLAISCRIPT::CMessageClass *b):CMessageScript(l,b)
	{
		set(0, new NLAILOGIC::CGoal());
	}

	CGoalMsg::CGoalMsg(NLAISCRIPT::CMessageClass *b):CMessageScript(b)
	{		
		CVectorGroupType *x = new CVectorGroupType(1);		
		setMessageGroup(x);
		set(0, new NLAILOGIC::CGoal());
	}

	CGoalMsg::CGoalMsg(IBasicAgent *agent):
			CMessageScript((NLAISCRIPT::CMessageClass *)NLAISCRIPT::CGoalMsgClass::IdGoalMsgClass.getFactory()->getClass())
	{		
		CVectorGroupType *x = new CVectorGroupType(1);
		setMessageGroup(x);		
		set(0, new NLAILOGIC::CGoal());
 	}

	CGoalMsg::CGoalMsg(const CGoalMsg &m): CMessageScript(m)
	{
	}

	CGoalMsg::~CGoalMsg()
	{
		
	}

	const NLAIC::IBasicType *CGoalMsg::clone() const
	{
		const NLAIC::IBasicType *x;
		x = new CGoalMsg(*this);
		return x;
	}

	const NLAIC::CIdentType &CGoalMsg::getType() const
	{
		if ( getCreatorClass() ) 
			return getCreatorClass()->getType();
		else
			return IdGoalMsg;
	}	

	void CGoalMsg::getDebugString(std::string &t) const
	{
		
		t += "CGoalMsg<false,NULL>";
	}


	tQueue CGoalMsg::isMember(const IVarName *className,const IVarName *funcName,const IObjectIA &params) const
	{

		tQueue r;

		if(className == NULL)
		{
			if( (*funcName) == CStringVarName( "Constructor" ) )
			{					
				r.push( CIdMethod( IMessageBase::getMethodIndexSize(), 0.0, NULL, new NLAISCRIPT::CObjectUnknown(new NLAISCRIPT::COperandVoid) ) );			
			}
		}

		if ( r.empty() )
			return CMessageScript::isMember(className, funcName, params);
		else
			return r;
	}

	NLAIAGENT::IObjectIA::CProcessResult CGoalMsg::runMethodeMember(sint32, sint32, NLAIAGENT::IObjectIA *)
	{
		return IObjectIA::CProcessResult();
	}

	IObjectIA::CProcessResult CGoalMsg::runMethodeMember(sint32 index, IObjectIA *context)
	{
		IBaseGroupType *param = (IBaseGroupType *) ( (NLAISCRIPT::CCodeContext *)context )->Param.back();

		switch(index - IMessageBase::getMethodIndexSize())
		{
		case 0:
			{					
				NLAILOGIC::CGoal *goal = (NLAILOGIC::CGoal *) param->getFront();
				param->popFront();
#ifdef NL_DEBUG
				std::string buffer;
				goal->getDebugString( buffer );
#endif
				set(0, goal);
			}
			break;
		}
		return IObjectIA::CProcessResult();
	}

	sint32 CGoalMsg::getBaseMethodCount() const
	{
		return CMessageScript::getBaseMethodCount() + 1;
	}


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////


	CCancelGoalMsg::CCancelGoalMsg( std::list<IObjectIA *> &l, NLAISCRIPT::CMessageClass *b):CMessageScript(l,b)
	{
		set(0, new NLAILOGIC::CGoal());
	}

	CCancelGoalMsg::CCancelGoalMsg(NLAISCRIPT::CMessageClass *b):CMessageScript(b)
	{		
		CVectorGroupType *x = new CVectorGroupType(1);		
		setMessageGroup(x);		
		set(0, new NLAILOGIC::CGoal());
	}

	CCancelGoalMsg::CCancelGoalMsg(IBasicAgent *agent):
			CMessageScript((NLAISCRIPT::CMessageClass *)NLAISCRIPT::CCancelGoalMsgClass::IdCancelGoalMsgClass.getFactory()->getClass())
	{		
		CVectorGroupType *x = new CVectorGroupType(1);
		setMessageGroup(x);		
		set(0, new NLAILOGIC::CGoal());
 	}

	CCancelGoalMsg::CCancelGoalMsg(const CCancelGoalMsg &m): CMessageScript(m)
	{
	}

	CCancelGoalMsg::~CCancelGoalMsg()
	{
		
	}

	const NLAIC::IBasicType *CCancelGoalMsg::clone() const
	{
		const NLAIC::IBasicType *x;
		x = new CCancelGoalMsg(*this);
		return x;
	}

	const NLAIC::CIdentType &CCancelGoalMsg::getType() const
	{
		if ( getCreatorClass() ) 
			return getCreatorClass()->getType();
		else
			return IdCancelGoalMsg;
	}	

	void CCancelGoalMsg::getDebugString(std::string &t) const
	{

			t += "CCancelGoalMsg<false,NULL>";
	}


	tQueue CCancelGoalMsg::isMember(const IVarName *className,const IVarName *funcName,const IObjectIA &params) const
	{

		tQueue r;

		if(className == NULL)
		{
			if( (*funcName) == CStringVarName( "Constructor" ) )
			{					
				r.push( CIdMethod( IMessageBase::getMethodIndexSize(), 0.0, NULL, new NLAISCRIPT::CObjectUnknown(new NLAISCRIPT::COperandVoid) ) );			
			}
		}

		if ( r.empty() )
			return CMessageScript::isMember(className, funcName, params);
		else
			return r;
	}

	NLAIAGENT::IObjectIA::CProcessResult CCancelGoalMsg::runMethodeMember(sint32, sint32, NLAIAGENT::IObjectIA *)
	{
		return IObjectIA::CProcessResult();
	}

	IObjectIA::CProcessResult CCancelGoalMsg::runMethodeMember(sint32 index, IObjectIA *context)
	{
		IBaseGroupType *param = (IBaseGroupType *) ( (NLAISCRIPT::CCodeContext *)context )->Param.back();

		switch(index - IMessageBase::getMethodIndexSize())
		{
		case 0:
			{					
				NLAILOGIC::CGoal *goal = (NLAILOGIC::CGoal *) param->getFront()->clone();
				param->popFront();
#ifdef NL_DEBUG
				std::string buffer;
				goal->getDebugString( buffer );
#endif
				set(0, goal);
			}
			break;
		}
		return IObjectIA::CProcessResult();
	}

	sint32 CCancelGoalMsg::getBaseMethodCount() const
	{
		return CMessageScript::getBaseMethodCount() + 1;
	}
}
