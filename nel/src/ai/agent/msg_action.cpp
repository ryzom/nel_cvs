/** \file msg_action.cpp
 *
 * $Id: msg_action.cpp,v 1.8 2003/01/13 16:58:59 chafik Exp $
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

#include "nel/ai/agent/msg_action.h"
#include "nel/ai/agent/agent_digital.h"
#include "nel/ai/script/interpret_message_action.h"
//#include "nel/ai/logic/goal.h"
#include "nel/ai/agent/object_type.h"
#include "nel/ai/script/codage.h"
#include "nel/ai/script/object_unknown.h"


namespace NLAIAGENT
{
	CSuccessMsg::CSuccessMsg( std::list<IObjectIA *> &l, NLAISCRIPT::CMessageClass *b):CMessageScript(l,b)
	{
		set(0, new DigitalType(0) );
	}

	CSuccessMsg::CSuccessMsg(NLAISCRIPT::CMessageClass *b):CMessageScript(b)
	{		
		CVectorGroupType *x = new CVectorGroupType(1);		
		setMessageGroup(x);
		set(0, new DigitalType(0) );
	}

	CSuccessMsg::CSuccessMsg(IBasicAgent *agent):
			CMessageScript((NLAISCRIPT::CMessageClass *)NLAISCRIPT::CSuccessMsgClass::IdSuccessMsgClass.getFactory()->getClass())
	{		
		CVectorGroupType *x = new CVectorGroupType(1);
		setMessageGroup(x);		
		set(0, new DigitalType(0) );
 	}

	CSuccessMsg::CSuccessMsg(const CSuccessMsg &m): CMessageScript(m)
	{
	}

	CSuccessMsg::~CSuccessMsg()
	{
		
	}

	const NLAIC::IBasicType *CSuccessMsg::clone() const
	{
		const NLAIC::IBasicType *x;
		x = new CSuccessMsg(*this);
		return x;
	}

	const NLAIC::CIdentType &CSuccessMsg::getType() const
	{
		if ( getCreatorClass() ) 
			return getCreatorClass()->getType();
		else
			return IdSuccessMsg;
	}	

	void CSuccessMsg::getDebugString(std::string &t) const
	{
		std::string txt;
		get()->getDebugString(txt);
		t += NLAIC::stringGetBuild("CSuccessMsg<%s>",txt.c_str());
	}


	tQueue CSuccessMsg::isMember(const IVarName *className,const IVarName *funcName,const IObjectIA &params) const
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

	NLAIAGENT::IObjectIA::CProcessResult CSuccessMsg::runMethodeMember(sint32, sint32, NLAIAGENT::IObjectIA *)
	{
		return IObjectIA::CProcessResult();
	}

	IObjectIA::CProcessResult CSuccessMsg::runMethodeMember(sint32 index, IObjectIA *context)
	{
		IBaseGroupType *arg = (IBaseGroupType *) ( (NLAISCRIPT::CCodeContext *)context )->Param.back();

		switch( index - IMessageBase::getMethodIndexSize() )
		{
		case 0:
			{					
				IObjectIA *param = (IObjectIA *) arg->get();
				arg->popFront();
#ifdef NL_DEBUG
				std::string buffer;
				param->getDebugString( buffer );
#endif
				set(0, param);
			}
			break;
		}
		return IObjectIA::CProcessResult();
	}

	sint32 CSuccessMsg::getBaseMethodCount() const
	{
		return CMessageScript::getBaseMethodCount() + 1;
	}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	CFailureMsg::CFailureMsg( std::list<IObjectIA *> &l, NLAISCRIPT::CMessageClass *b):CMessageScript(l,b)
	{
		set(0, new DigitalType(0) );
	}

	CFailureMsg::CFailureMsg(NLAISCRIPT::CMessageClass *b):CMessageScript(b)
	{		
		CVectorGroupType *x = new CVectorGroupType(1);		
		setMessageGroup(x);		
		set(0, new DigitalType(0) );
	}

	CFailureMsg::CFailureMsg(IBasicAgent *agent):
			CMessageScript((NLAISCRIPT::CMessageClass *)NLAISCRIPT::CFailureMsgClass::IdFailureMsgClass.getFactory()->getClass())
	{		
		CVectorGroupType *x = new CVectorGroupType(1);
		setMessageGroup(x);		
		set(0, new DigitalType(0) );
 	}

	CFailureMsg::CFailureMsg(const CFailureMsg &m): CMessageScript(m)
	{
	}

	CFailureMsg::~CFailureMsg()
	{
		
	}

	const NLAIC::IBasicType *CFailureMsg::clone() const
	{
		const NLAIC::IBasicType *x;
		x = new CFailureMsg(*this);
		return x;
	}

	const NLAIC::CIdentType &CFailureMsg::getType() const
	{
		if ( getCreatorClass() ) 
			return getCreatorClass()->getType();
		else
			return IdFailureMsg;
	}	

	void CFailureMsg::getDebugString(std::string &t) const
	{
		std::string txt;
		get()->getDebugString(txt);
		t += NLAIC::stringGetBuild("CFailureMsg<%s>",txt.c_str());
	}


	tQueue CFailureMsg::isMember(const IVarName *className,const IVarName *funcName,const IObjectIA &params) const
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

	NLAIAGENT::IObjectIA::CProcessResult CFailureMsg::runMethodeMember(sint32, sint32, NLAIAGENT::IObjectIA *)
	{
		return IObjectIA::CProcessResult();
	}

	IObjectIA::CProcessResult CFailureMsg::runMethodeMember(sint32 index, IObjectIA *context)
	{
		IBaseGroupType *arg = (IBaseGroupType *) ( (NLAISCRIPT::CCodeContext *)context )->Param.back();

		switch( index - IMessageBase::getMethodIndexSize() )
		{
		case 0:
			{					
				IObjectIA *param = (IObjectIA *) arg->get()->clone();				
#ifdef NL_DEBUG
				std::string buffer;
				param->getDebugString( buffer );
#endif
				set(0, param);
			}
			break;
		}
		return IObjectIA::CProcessResult();
	}

	sint32 CFailureMsg::getBaseMethodCount() const
	{
		return CMessageScript::getBaseMethodCount() + 1;
	}
}
