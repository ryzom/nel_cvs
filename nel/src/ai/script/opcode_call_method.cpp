/** \file opcode_call_method.cpp
 *
 * $Id: opcode_call_method.cpp,v 1.9 2003/01/24 15:51:45 chafik Exp $
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
#include "nel/ai/agent/message_script.h"
#include "nel/ai/script/interpret_object_agent.h"
#include "nel/ai/script/interpret_object_message.h"

namespace NLAISCRIPT
{
	//*************************************
	// CMethodContext
	//*************************************
	const IMethodContext *CMethodContext::clone() const
	{
		return new CMethodContext();
	}
	
	void CMethodContext::saveConstext(CCodeContext &context)
	{
		NLAIAGENT::IBaseGroupType *param = (NLAIAGENT::IBaseGroupType *)context.Stack[(int)context.Stack];
		NLAIAGENT::CIteratorContener It = param->getIterator();
		uint z = 0;
		while(!It.isInEnd())
		{			
			NLAIAGENT::IObjectIA *o = (NLAIAGENT::IObjectIA *)It++;
			o->incRef();
			context.Heap[(int)context.Heap] = o;
			context.Heap ++;
			z ++;
		}				
		int bp = (int)context.Heap;
		CVarPStackParam::_Shift += bp;
		context.Heap.addStack(z);
		context.Heap.setShift((int)context.Heap);
		context.Param.push_back(param);
		param->incRef();
		context.Stack --;
		
	}

	void CMethodContext::loadConstext(CCodeContext &context)
	{
		context.Heap.restoreShift();
		context.Heap.restoreStack();
		context.Param.back()->release();
		context.Param.pop_back();
	}

	//*************************************
	// CMethodContextDebug
	//*************************************
	const IMethodContext *CMethodContextDebug::clone() const
	{
		return new CMethodContextDebug();
	}
	
	void CMethodContextDebug::saveConstext(CCodeContext &context)
	{
		NLAIAGENT::IBaseGroupType *param = (NLAIAGENT::IBaseGroupType *)context.Stack[(int)context.Stack];		
		NLAIAGENT::CIteratorContener It = param->getIterator();
		
		while(!It.isInEnd())
		{			
			NLAIAGENT::IObjectIA *o = (NLAIAGENT::IObjectIA *)It++;
			o->incRef();
			context.Heap[(int)context.Heap] = o;
			context.Heap ++;
		}				
		int bp = (int)context.Heap;
		CVarPStackParam::_Shift += bp;
		context.Heap.addStack(param->size());
		int sh = (int)context.Heap;
		context.Heap.setShift(sh);
		context.ContextDebug.HeapDebug.setShift(sh);
		context.Param.push_back(param);
		param->incRef();
		context.Stack --;
	}

	void CMethodContextDebug::loadConstext(CCodeContext &context)
	{
		context.Heap.restoreShift();
		context.Heap.restoreStack();
		context.ContextDebug.HeapDebug.restoreShift();
		NLAIAGENT::IObjectIA *obj = context.Param.back();
		obj->release();
		context.Param.pop_back();		
	}

	//*************************************
	// CCallMethod
	//*************************************

	void CCallMethod::getDebugResult(std::string &str,CCodeContext &context) const
	{		
		const NLAIAGENT::IObjectIA *r = (context.Self);
		std::string name = "????";
		if(((const NLAIC::CTypeOfObject &)r->getType()) & NLAIC::CTypeOfObject::tInterpret)
		{
			if(((const NLAIC::CTypeOfObject &)r->getType()) & NLAIC::CTypeOfObject::tAgent)
			{
				name = r->getMethodeMemberDebugString(_Inheritance,_I);
			}
			else
			if(((const NLAIC::CTypeOfObject &)r->getType()) & NLAIC::CTypeOfObject::tMessage)
			{		
				if(((NLAIAGENT::CMessageScript *)r)->getCreatorClass() != NULL)
				{
					((NLAIAGENT::CMessageScript *)r)->getCreatorClass()->getBrancheCode(_Inheritance,_I).getName().getDebugString(name);
					((NLAIAGENT::CMessageScript *)r)->getCreatorClass()->getBrancheCode(_Inheritance,_I).getParam().getDebugString(name);
				}
			}
		}
		str = NLAIC::stringGetBuild("Method: '%s.%s'", (const char *)(context.Self)->getType(),name.c_str());
	}

	NLAIAGENT::TProcessStatement CCallMethod::runOpCode(CCodeContext &context)
	{				
		int sp = CVarPStackParam::_Shift;
		saveConstext(context);

		NLAIAGENT::IObjectIA::CProcessResult i;

		if(_Inheritance) i = ((NLAIAGENT::IObjectIA *)context.Self)->runMethodeMember(_Inheritance,_I,&context);
		else i = ((NLAIAGENT::IObjectIA *)context.Self)->runMethodeMember(_I,&context);
		
		loadConstext(context);
		CVarPStackParam::_Shift = sp;
		return NLAIAGENT::processIdle;
	}	

	void CCallMethodi::getDebugResult(std::string &str,CCodeContext &context) const
	{
		NLAIAGENT::IObjectIA *obj = (NLAIAGENT::IObjectIA *)context.Self;
		std::list<sint32>::const_iterator it = _N.begin();
		std::string name = "????";

		while(it != _N.end())
		{
			obj = (NLAIAGENT::IObjectIA *)obj->getStaticMember(*it++);
		}

		//str = NLAIC::stringGetBuild("CallMethodi %d de la class '%s'",_I, (const char *)obj->getType());
		if(((const NLAIC::CTypeOfObject &)obj->getType()) & NLAIC::CTypeOfObject::tInterpret)
		{
			if(((const NLAIC::CTypeOfObject &)obj->getType()) & NLAIC::CTypeOfObject::tAgent)
			{
				/*sint i = _I - ((NLAIAGENT::CAgentScript *)obj)->getBaseMethodCount();
				if(i >= 0)
				{
					((NLAIAGENT::CAgentScript *)obj)->getClass()->getBrancheCode(_Inheritance, i).getName().getDebugString(name);
					((NLAIAGENT::CAgentScript *)obj)->getClass()->getBrancheCode(_Inheritance,i).getParam().getDebugString(name);
				}*/
				name = obj->getMethodeMemberDebugString(_Inheritance,_I);
			}
			else
			if(((const NLAIC::CTypeOfObject &)obj->getType()) & NLAIC::CTypeOfObject::tMessage)
			{				
				((NLAIAGENT::CMessageScript *)obj)->getCreatorClass()->getBrancheCode(_Inheritance,_I).getName().getDebugString(name);
				((NLAIAGENT::CMessageScript *)obj)->getCreatorClass()->getBrancheCode(_Inheritance,_I).getParam().getDebugString(name);
			}
		}
		str = NLAIC::stringGetBuild("Method: '%s.%s'", (const char *)(context.Self)->getType(),name.c_str());
	}

	NLAIAGENT::TProcessStatement CCallMethodi::runOpCode(CCodeContext &context)
	{				
		int sp = CVarPStackParam::_Shift;
		NLAIAGENT::IObjectIA *obj = (NLAIAGENT::IObjectIA *)context.Self;

		std::list<sint32>::iterator it = _N.begin();			
		while(it != _N.end())
		{
			obj = (NLAIAGENT::IObjectIA *)obj->getStaticMember(*it++);
		}

		NLAIAGENT::IObjectIA *selfTmp = (NLAIAGENT::IObjectIA *)context.Self;
		context.Self = obj;
		saveConstext(context);

		NLAIAGENT::IObjectIA::CProcessResult i;
		if(_Inheritance) i = ((NLAIAGENT::IObjectIA *)context.Self)->runMethodeMember(_Inheritance,_I,&context);
		else i = ((NLAIAGENT::IObjectIA *)context.Self)->runMethodeMember(_I,&context);

		context.Self = selfTmp;
		loadConstext(context);
		CVarPStackParam::_Shift = sp;
		return NLAIAGENT::processIdle;
	}	

	NLAIAGENT::TProcessStatement CCallStackNewMethodi::runOpCode(CCodeContext &context)
	{				
		int sp = CVarPStackParam::_Shift;
		NLAIAGENT::IObjectIA *o = (NLAIAGENT::IObjectIA *)context.Stack[(int)context.Stack];
		o->incRef();		
		context.Stack --;		

		std::list<sint32>::iterator it = _N.begin();
		NLAIAGENT::IObjectIA *obj = o;
		while(it != _N.end())
		{
			obj = (NLAIAGENT::IObjectIA *)obj->getStaticMember(*it++);
		}

		NLAIAGENT::IObjectIA *selfTmp = (NLAIAGENT::IObjectIA *)context.Self;
		context.Self = obj;
		saveConstext(context);
		
		NLAIAGENT::IObjectIA::CProcessResult i;
		if ( _Inheritance ) 
			i = ((NLAIAGENT::IObjectIA *)context.Self)->runMethodeMember(_Inheritance,_I,&context);
		else 
			i = ((NLAIAGENT::IObjectIA *)context.Self)->runMethodeMember(_I,&context);

		context.Self = selfTmp;
		loadConstext(context);
		CVarPStackParam::_Shift = sp;		
		context.Stack ++;
		context.Stack[(int)context.Stack] = o;
		return NLAIAGENT::processIdle;
	}

	void CCallStackMethodi::getDebugResult(std::string &str,CCodeContext &context) const
	{		
		NLAIAGENT::IObjectIA *obj = (NLAIAGENT::IObjectIA *)context.Stack[(int)context.Stack - 1];
		std::list<sint32>::const_iterator it = _N.begin();
		std::string name = "????";

		while(it != _N.end())
		{
			obj = (NLAIAGENT::IObjectIA *)obj->getStaticMember(*it++);
		}		
		if(((const NLAIC::CTypeOfObject &)obj->getType()) & NLAIC::CTypeOfObject::tInterpret)
		{
			if(((const NLAIC::CTypeOfObject &)obj->getType()) & NLAIC::CTypeOfObject::tAgent)
			{
				/*sint i = _I - ((NLAIAGENT::CAgentScript *)obj)->getBaseMethodCount();
				if(i >= 0)
				{
					((NLAIAGENT::CAgentScript *)obj)->getClass()->getBrancheCode(_Inheritance, i).getName().getDebugString(name);
					((NLAIAGENT::CAgentScript *)obj)->getClass()->getBrancheCode(_Inheritance,i).getParam().getDebugString(name);
				}*/
				name = obj->getMethodeMemberDebugString(_Inheritance,_I);
			}
			else
			if(((const NLAIC::CTypeOfObject &)obj->getType()) & NLAIC::CTypeOfObject::tMessage)
			{				
				((NLAIAGENT::CMessageScript *)obj)->getCreatorClass()->getBrancheCode(_Inheritance,_I).getName().getDebugString(name);
				((NLAIAGENT::CMessageScript *)obj)->getCreatorClass()->getBrancheCode(_Inheritance,_I).getParam().getDebugString(name);
			}
		}
		str = NLAIC::stringGetBuild("Method: '%s.%s'", (const char *)(context.Self)->getType(),name.c_str());
	}

	NLAIAGENT::TProcessStatement CCallStackMethodi::runOpCode(CCodeContext &context)
	{				
		int sp = CVarPStackParam::_Shift;
		NLAIAGENT::IObjectIA *o = (NLAIAGENT::IObjectIA *)context.Stack[(int)context.Stack - 1];
		o->incRef();
		NLAIAGENT::IObjectIA *os = (NLAIAGENT::IObjectIA *)context.Stack; 
		os->incRef();
		
		context.Stack --;
		context.Stack --;
		context.Stack ++;
		context.Stack[(int)context.Stack] = os;

		std::list<sint32>::iterator it = _N.begin();
		NLAIAGENT::IObjectIA *obj = o;
		while(it != _N.end())
		{
			obj = (NLAIAGENT::IObjectIA *)obj->getStaticMember(*it++);
		}

		NLAIAGENT::IObjectIA *selfTmp = (NLAIAGENT::IObjectIA *)context.Self;
		context.Self = obj;
		saveConstext(context);
		
		NLAIAGENT::IObjectIA::CProcessResult i;
		if(_Inheritance) i = ((NLAIAGENT::IObjectIA *)context.Self)->runMethodeMember(_Inheritance,_I,&context);
		else i = ((NLAIAGENT::IObjectIA *)context.Self)->runMethodeMember(_I,&context);

		context.Self = selfTmp;
		loadConstext(context);
		CVarPStackParam::_Shift = sp;
		o->release();
		return NLAIAGENT::processIdle;
	}			


	void CCallHeapMethodi::getDebugResult(std::string &str,CCodeContext &context) const
	{		
		NLAIAGENT::IObjectIA *obj = (NLAIAGENT::IObjectIA *)context.Heap[(int)_HeapPos];
		std::list<sint32>::const_iterator it = _N.begin();
		std::string name = "????";

		while(it != _N.end())
		{
			obj = (NLAIAGENT::IObjectIA *)obj->getStaticMember(*it++);
		}
		if(((const NLAIC::CTypeOfObject &)obj->getType()) & NLAIC::CTypeOfObject::tInterpret)
		{
			if(((const NLAIC::CTypeOfObject &)obj->getType()) & NLAIC::CTypeOfObject::tAgent)
			{
				/*sint i = _I - ((NLAIAGENT::CAgentScript *)obj)->getBaseMethodCount();
				if(i >= 0)
				{
					((NLAIAGENT::CAgentScript *)obj)->getClass()->getBrancheCode(_Inheritance, i).getName().getDebugString(name);
					((NLAIAGENT::CAgentScript *)obj)->getClass()->getBrancheCode(_Inheritance,i).getParam().getDebugString(name);
				}*/
				name = obj->getMethodeMemberDebugString(_Inheritance,_I);
			}
			else
			if(((const NLAIC::CTypeOfObject &)obj->getType()) & NLAIC::CTypeOfObject::tMessage)
			{				
				((NLAIAGENT::CMessageScript *)obj)->getCreatorClass()->getBrancheCode(_Inheritance,_I).getName().getDebugString(name);
				((NLAIAGENT::CMessageScript *)obj)->getCreatorClass()->getBrancheCode(_Inheritance,_I).getParam().getDebugString(name);
			}
		}
		str = NLAIC::stringGetBuild("Method: '%s.%s'", (const char *)(context.Self)->getType(),name.c_str());
	}


	NLAIAGENT::TProcessStatement CCallHeapMethodi::runOpCode(CCodeContext &context)
	{				
		int sp = CVarPStackParam::_Shift;
		NLAIAGENT::IObjectIA *obj = (NLAIAGENT::IObjectIA *)context.Heap[(int)_HeapPos];
		NLAIAGENT::IObjectIA *selfTmp = (NLAIAGENT::IObjectIA *)context.Self;

		std::list<sint32>::iterator it = _N.begin();			
		while(it != _N.end())
		{
			obj = (NLAIAGENT::IObjectIA *)obj->getStaticMember(*it++);
		}
		
		context.Self = obj;
		saveConstext(context);

		NLAIAGENT::IObjectIA::CProcessResult i;
		if(_Inheritance) i = ((NLAIAGENT::IObjectIA *)context.Self)->runMethodeMember(_Inheritance,_I,&context);
		else i = ((NLAIAGENT::IObjectIA *)context.Self)->runMethodeMember(_I,&context);

		context.Self = selfTmp;
		loadConstext(context);
		CVarPStackParam::_Shift = sp;
		return NLAIAGENT::processIdle;
	}
}
