/** \file msg_fact.cpp
 *
 * $Id: msg_fact.cpp,v 1.5 2003/01/13 16:58:59 chafik Exp $
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

#include "nel/ai/agent/msg_fact.h"
#include "nel/ai/agent/agent_digital.h"
#include "nel/ai/script/interpret_object_message.h"
#include "nel/ai/agent/object_type.h"
#include "nel/ai/script/codage.h"
#include "nel/ai/script/object_unknown.h"
#include "nel/ai/logic/fact.h"


namespace NLAIAGENT
{
	CFactMsg::CFactMsg( std::list<IObjectIA *> &l, NLAISCRIPT::CMessageClass *b):CMessageScript(l,b)
	{
		set(0, new NLAILOGIC::CFact() );
	}

	CFactMsg::CFactMsg(NLAISCRIPT::CMessageClass *b):CMessageScript(b)
	{		
		CVectorGroupType *x = new CVectorGroupType(1);		
		setMessageGroup(x);		
		set(0, new NLAILOGIC::CFact());
	}

	CFactMsg::CFactMsg(IBasicAgent *agent):
			CMessageScript((NLAISCRIPT::CMessageClass *)NLAISCRIPT::CFactMsgClass::IdFactMsgClass.getFactory()->getClass())
	{		
		CVectorGroupType *x = new CVectorGroupType(1);
		setMessageGroup(x);		
		set(0, new NLAILOGIC::CFact());
 	}

	CFactMsg::CFactMsg(const CFactMsg &m): CMessageScript(m)
	{
	}

	CFactMsg::~CFactMsg()
	{
		
	}

	const NLAIC::IBasicType *CFactMsg::clone() const
	{
		const NLAIC::IBasicType *x;
		x = new CFactMsg(*this);
		return x;
	}

	const NLAIC::CIdentType &CFactMsg::getType() const
	{
		if ( getCreatorClass() ) 
			return getCreatorClass()->getType();
		else
			return IdFactMsg;
	}	

	void CFactMsg::getDebugString(std::string &t) const
	{		
		CMessageScript::getDebugString(t);
		t +=">";
	}


	tQueue CFactMsg::isMember(const IVarName *className,const IVarName *funcName,const IObjectIA &params) const
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

	NLAIAGENT::IObjectIA::CProcessResult CFactMsg::runMethodeMember(sint32, sint32, NLAIAGENT::IObjectIA *)
	{
		return IObjectIA::CProcessResult();
	}

	IObjectIA::CProcessResult CFactMsg::runMethodeMember(sint32 index, IObjectIA *context)
	{
		IBaseGroupType *param = (IBaseGroupType *) ( (NLAISCRIPT::CCodeContext *)context )->Param.back();

		switch(index - IMessageBase::getMethodIndexSize())
		{
		case 0:
			{					
				NLAILOGIC::CFact *fact = (NLAILOGIC::CFact *) param->get();
				param->popFront();
#ifdef NL_DEBUG
				std::string buffer;
				fact->getDebugString( buffer );
#endif
				set(0, fact);
			}
			break;
		}
		return IObjectIA::CProcessResult();
	}

	sint32 CFactMsg::getBaseMethodCount() const
	{
		return CMessageScript::getBaseMethodCount() + 1;
	}
}
