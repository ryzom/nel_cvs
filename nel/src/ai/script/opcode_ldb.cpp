/** \file opcode_ldb.cpp
 *
 * $Id: opcode_ldb.cpp,v 1.15 2002/08/21 13:58:33 lecroart Exp $
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
#include "nel/ai/agent/agent_script.h"
#include "nel/ai/script/interpret_object_agent.h"
#include "nel/ai/script/interpret_object_message.h"

namespace NLAISCRIPT
{
	CLdbOpCode::CLdbOpCode(const NLAIAGENT::IObjectIA &b)
	{
	  _B = (NLAIAGENT::IObjectIA *)b.clone();

	}

	NLAIAGENT::TProcessStatement CLdbOpCode::runOpCode(CCodeContext &context)
	{
		context.Stack ++;
		context.Stack[(int)context.Stack] = (NLAIAGENT::IObjectIA *)_B->clone();
		return NLAIAGENT::IObjectIA::ProcessIdle;
	}

	void CLdbOpCode::getDebugResult(std::string &str,CCodeContext &context) const
	{			
		std::string X;
		_B->getDebugString(X);
		str += "ldb ";
		str += X;		
	}
	NLAIAGENT::TProcessStatement CLdbMemberOpCode::runOpCode(CCodeContext &context)
	{
		context.Stack ++;

		NLAIAGENT::IObjectIA *t = (NLAIAGENT::IObjectIA *)(context.Self)->getStaticMember(_B);//->clone();
		t->incRef();
		context.Stack[(int)context.Stack] = t;
		return NLAIAGENT::IObjectIA::ProcessIdle;;
	}

	void CLdbMemberOpCode::getDebugResult(std::string &str,CCodeContext &context) const
	{		
		const NLAIAGENT::IObjectIA *r = (context.Self);
		std::string name = "????";
		std::string className = "????";

		if(((const NLAIC::CTypeOfObject &)r->getType()) & NLAIC::CTypeOfObject::tInterpret)
		{
			if(((const NLAIC::CTypeOfObject &)r->getType()) & NLAIC::CTypeOfObject::tAgent)
			{			
				name = ((NLAIAGENT::CAgentScript *)r)->getClass()->getComponentName(_B);
			}
			else
			if(((const NLAIC::CTypeOfObject &)r->getType()) & NLAIC::CTypeOfObject::tMessage)
			{			
				name = ((NLAIAGENT::CMessageScript *)r)->getCreatorClass()->getComponentName(_B);
			}
		}
		className = (const char *)r->getType();
		r = r->getStaticMember(_B);
					
		str += NLAIC::stringGetBuild("ldb le composant membre %d named: '%s' de la class '%s'",_B, name.c_str(),className.c_str());
	}


	NLAIAGENT::TProcessStatement CLdbStackMemberiOpCode::runOpCode(CCodeContext &context)
	{
		NLAIAGENT::IObjectIA *a = ((NLAIAGENT::IObjectIA *)context.Stack);
		NLAIAGENT::IObjectIA *obj = a; 
		std::list<sint32>::iterator i = _I.begin();
		sint32 n = _I.size() - 1;
		
		while(n --)
		{			
			a = (NLAIAGENT::IObjectIA *)a->getStaticMember(*i++);
		}
		//context.Stack[(int)context.Stack] = (NLAIAGENT::IObjectIA *)a->getStaticMember(*i)->clone();
		a = (NLAIAGENT::IObjectIA *)a->getStaticMember(*i);
		a->incRef();
		context.Stack[(int)context.Stack] = a;
		obj->release();
		return NLAIAGENT::processIdle;
	}

	void CLdbStackMemberiOpCode::getDebugResult(std::string &str,CCodeContext &context) const
	{	
		NLAIAGENT::IObjectIA *a = ((NLAIAGENT::IObjectIA *)context.Stack);		
		std::list<sint32>::const_iterator i = _I.begin();
		sint32 n = _I.size() - 1;
		std::string name = "????";
		std::string className = "????";
		while(n --)
		{
			if(((const NLAIC::CTypeOfObject &)a->getType()) & NLAIC::CTypeOfObject::tInterpret)
			{
				if(((const NLAIC::CTypeOfObject &)a->getType()) & NLAIC::CTypeOfObject::tAgent)
				{			
					name = ((NLAIAGENT::CAgentScript *)a)->getClass()->getComponentName(*i);
				}
				else
				if(((const NLAIC::CTypeOfObject &)a->getType()) & NLAIC::CTypeOfObject::tMessage)
				{			
					name = ((NLAIAGENT::CMessageScript *)a)->getCreatorClass()->getComponentName(*i);
				}
			}
			className = (const char *)a->getType();
			a = (NLAIAGENT::IObjectIA *)a->getStaticMember(*i++);
		}
		std::string txt;
		a->getStaticMember(*i)->getDebugString(txt);
		str += NLAIC::stringGetBuild("ldb %s le composon '%s' membre sur la pile de la class '%s'",txt.c_str(),name.c_str(),className.c_str());
	}

	NLAIAGENT::TProcessStatement CLdbHeapMemberiOpCode::runOpCode(CCodeContext &context)
	{
		NLAIAGENT::IObjectIA *a = (NLAIAGENT::IObjectIA *)context.Heap[(int)_N];

#ifdef NL_DEBUG
		std::string text;
		text = NLAIC::stringGetBuild("ldb le composant membre sur le heap de la class '%s'",(const char *)a->getType());		
#endif
		std::list<sint32>::iterator i = _I.begin();		
		sint32 n = _I.size() - 1;
		while(n --)
		{
			
			a = (NLAIAGENT::IObjectIA *)a->getStaticMember(*i++);
		}
		a = (NLAIAGENT::IObjectIA *)a->getStaticMember(*i);//->clone();
		context.Stack ++;
		a->incRef();
		context.Stack[(int)context.Stack] = a;
		return NLAIAGENT::processIdle;
	}

	void CLdbHeapMemberiOpCode::getDebugResult(std::string &str,CCodeContext &context) const
	{						
		NLAIAGENT::IObjectIA *a = (NLAIAGENT::IObjectIA *)context.Heap[(int)_N];
#ifdef NL_DEBUG
		std::string text;
		text = NLAIC::stringGetBuild("ldb le composant membre sur le heap de la class '%s'",(const char *)a->getType());		
#endif
		
		std::string name = "????";
		std::string className = "????";
		std::list<sint32>::const_iterator i = _I.begin();
		while(i != _I.end())
		{
			if(((const NLAIC::CTypeOfObject &)a->getType()) & NLAIC::CTypeOfObject::tInterpret)
			{			
				if(((const NLAIC::CTypeOfObject &)a->getType()) & NLAIC::CTypeOfObject::tAgent)
				{			
					name = ((NLAIAGENT::CAgentScript *)a)->getClass()->getComponentName(*i);
				}
				else
				if(((const NLAIC::CTypeOfObject &)a->getType()) & NLAIC::CTypeOfObject::tMessage)
				{			
					name = ((NLAIAGENT::CMessageScript *)a)->getCreatorClass()->getComponentName(*i);
				}
			}
			className = (const char *)a->getType();
			a = (NLAIAGENT::IObjectIA *)a->getStaticMember(*i++);
		}

		std::string txt;
		a->getDebugString(txt);
		str += NLAIC::stringGetBuild("ldb %s le composon '%s' membre sur le heap de la class '%s'",txt.c_str(),name.c_str(),className.c_str());		
	}

	NLAIAGENT::TProcessStatement CLdbMemberiOpCode::runOpCode(CCodeContext &context)
	{
		context.Stack ++;
		NLAIAGENT::IObjectIA *obj = (NLAIAGENT::IObjectIA *)context.Self;
		std::list<sint32>::iterator i = _I.begin();
		while(i != _I.end())
		{
			obj = (NLAIAGENT::IObjectIA *)obj->getStaticMember(*i++);
		}
		context.Stack[(int)context.Stack] = (NLAIAGENT::IObjectIA *)obj;//->clone();
		obj->incRef();
		
		return NLAIAGENT::IObjectIA::ProcessIdle;
	}

	void CLdbMemberiOpCode::getDebugResult(std::string &str,CCodeContext &context) const
	{			
		NLAIAGENT::IObjectIA *obj = (NLAIAGENT::IObjectIA *)context.Self;
		std::list<sint32>::const_iterator i = _I.begin();
		int j=0;
		std::string className, name = "????";

		while(i != _I.end())
		{
			j = *i++;
			if(((const NLAIC::CTypeOfObject &)obj->getType()) & NLAIC::CTypeOfObject::tInterpret)
			{			
				if(((const NLAIC::CTypeOfObject &)obj->getType()) & NLAIC::CTypeOfObject::tAgent)
				{			
					name = ((NLAIAGENT::CAgentScript *)obj)->getClass()->getComponentName(j);
				}
				else
				if(((const NLAIC::CTypeOfObject &)obj->getType()) & NLAIC::CTypeOfObject::tMessage)
				{			
					name = ((NLAIAGENT::CMessageScript *)obj)->getCreatorClass()->getComponentName(j);
				}
			}
			className = (const char *)obj->getType();
			obj = (NLAIAGENT::IObjectIA *)obj->getStaticMember(j);
		}			
					
		str += NLAIC::stringGetBuild("ldb le composon '%s' membre <%d> member de la class '%s'",name.c_str(),j,className.c_str());		
	}

	NLAIAGENT::TProcessStatement CLdbRefOpCode::runOpCode(CCodeContext &context)
	{
		NLAIAGENT::IObjectIA *o = (NLAIAGENT::IObjectIA *)context.Heap[_B];//->clone();
		o->incRef();
		context.Stack ++;
		context.Stack[(int)context.Stack] = o;
		return NLAIAGENT::IObjectIA::ProcessIdle;
	}

	void CLdbRefOpCode::getDebugResult(std::string &str,CCodeContext &context) const
	{
		std::string X;
		context.Heap[_B]->getDebugString(X);
		str += NLAIC::stringGetBuild("ldb<%d>: %s",_B,X.c_str());
	}
}
