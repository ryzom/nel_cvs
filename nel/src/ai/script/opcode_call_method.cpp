/** \file opcode_call_method.cpp
 *
 * $Id: opcode_call_method.cpp,v 1.1 2001/01/05 10:53:49 chafik Exp $
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
#include "script/compilateur.h"
#include "agent/agent_script.h"

namespace NLIASCRIPT
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
		NLIAAGENT::IBaseGroupType *param = (NLIAAGENT::IBaseGroupType *)context.Stack[(int)context.Stack];
		NLIAAGENT::CIteratorContener It = param->getIterator();
		
		while(!It.isInEnd())
		{			
			NLIAAGENT::IObjectIA *o = (NLIAAGENT::IObjectIA *)It++;
			o->incRef();
			context.Heap[(int)context.Heap] = o;
			context.Heap ++;
		}				
		int bp = (int)context.Heap;
		CVarPStackParam::_Shift += bp;
		context.Heap.addStack(param->size());
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
		NLIAAGENT::IBaseGroupType *param = (NLIAAGENT::IBaseGroupType *)context.Stack[(int)context.Stack];		
		NLIAAGENT::CIteratorContener It = param->getIterator();
		
		while(!It.isInEnd())
		{			
			NLIAAGENT::IObjectIA *o = (NLIAAGENT::IObjectIA *)It++;
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
		context.Param.back()->release();
		context.Param.pop_back();
		context.ContextDebug.Param.back()->release();
		context.ContextDebug.Param.pop_back();
	}

	//*************************************
	// CCallMethod
	//*************************************
	NLIAAGENT::TProcessStatement CCallMethod::runOpCode(CCodeContext &context)
	{				
		int sp = CVarPStackParam::_Shift;
		saveConstext(context);

		NLIAAGENT::IObjectIA::CProcessResult i;
		if(_Inheritance) i = ((NLIAAGENT::IObjectIA *)context.Self)->runMethodeMember(_Inheritance,_I,&context);
		else i = ((NLIAAGENT::IObjectIA *)context.Self)->runMethodeMember(_I,&context);
		
		loadConstext(context);
		CVarPStackParam::_Shift = sp;
		return NLIAAGENT::processIdle;
	}	

	NLIAAGENT::TProcessStatement CCallMethodi::runOpCode(CCodeContext &context)
	{				
		int sp = CVarPStackParam::_Shift;
		NLIAAGENT::IObjectIA *obj = (NLIAAGENT::IObjectIA *)context.Self;

		std::list<sint32>::iterator it = _N.begin();			
		while(it != _N.end())
		{
			obj = (NLIAAGENT::IObjectIA *)obj->getStaticMember(*it++);
		}

		NLIAAGENT::IObjectIA *selfTmp = (NLIAAGENT::IObjectIA *)context.Self;
		context.Self = obj;
		saveConstext(context);

		NLIAAGENT::IObjectIA::CProcessResult i;
		if(_Inheritance) i = ((NLIAAGENT::IObjectIA *)context.Self)->runMethodeMember(_Inheritance,_I,&context);
		else i = ((NLIAAGENT::IObjectIA *)context.Self)->runMethodeMember(_I,&context);

		context.Self = selfTmp;
		loadConstext(context);
		CVarPStackParam::_Shift = sp;
		return NLIAAGENT::processIdle;
	}	

	NLIAAGENT::TProcessStatement CCallStackNewMethodi::runOpCode(CCodeContext &context)
	{				
		int sp = CVarPStackParam::_Shift;
		NLIAAGENT::IObjectIA *o = (NLIAAGENT::IObjectIA *)context.Stack[(int)context.Stack];
		o->incRef();		
		context.Stack --;		

		std::list<sint32>::iterator it = _N.begin();
		NLIAAGENT::IObjectIA *obj = o;
		while(it != _N.end())
		{
			obj = (NLIAAGENT::IObjectIA *)obj->getStaticMember(*it++);
		}

		NLIAAGENT::IObjectIA *selfTmp = (NLIAAGENT::IObjectIA *)context.Self;
		context.Self = obj;
		saveConstext(context);
		
		NLIAAGENT::IObjectIA::CProcessResult i;
		if(_Inheritance) i = ((NLIAAGENT::IObjectIA *)context.Self)->runMethodeMember(_Inheritance,_I,&context);
		else i = ((NLIAAGENT::IObjectIA *)context.Self)->runMethodeMember(_I,&context);

		context.Self = selfTmp;
		loadConstext(context);
		CVarPStackParam::_Shift = sp;		
		context.Stack ++;
		context.Stack[(int)context.Stack] = o;
		return NLIAAGENT::processIdle;
	}


	NLIAAGENT::TProcessStatement CCallStackMethodi::runOpCode(CCodeContext &context)
	{				
		int sp = CVarPStackParam::_Shift;
		NLIAAGENT::IObjectIA *o = (NLIAAGENT::IObjectIA *)context.Stack[(int)context.Stack - 1];
		o->incRef();
		NLIAAGENT::IObjectIA *os = (NLIAAGENT::IObjectIA *)context.Stack; 
		os->incRef();
		
		context.Stack --;
		context.Stack --;
		context.Stack ++;
		context.Stack[(int)context.Stack] = os;

		std::list<sint32>::iterator it = _N.begin();
		NLIAAGENT::IObjectIA *obj = o;
		while(it != _N.end())
		{
			obj = (NLIAAGENT::IObjectIA *)obj->getStaticMember(*it++);
		}

		NLIAAGENT::IObjectIA *selfTmp = (NLIAAGENT::IObjectIA *)context.Self;
		context.Self = obj;
		saveConstext(context);
		
		NLIAAGENT::IObjectIA::CProcessResult i;
		if(_Inheritance) i = ((NLIAAGENT::IObjectIA *)context.Self)->runMethodeMember(_Inheritance,_I,&context);
		else i = ((NLIAAGENT::IObjectIA *)context.Self)->runMethodeMember(_I,&context);

		context.Self = selfTmp;
		loadConstext(context);
		CVarPStackParam::_Shift = sp;
		o->release();
		return NLIAAGENT::processIdle;
	}			

	NLIAAGENT::TProcessStatement CCallHeapMethodi::runOpCode(CCodeContext &context)
	{				
		int sp = CVarPStackParam::_Shift;
		NLIAAGENT::IObjectIA *obj = (NLIAAGENT::IObjectIA *)context.Heap[(int)_HeapPos];
		NLIAAGENT::IObjectIA *selfTmp = (NLIAAGENT::IObjectIA *)context.Self;

		std::list<sint32>::iterator it = _N.begin();			
		while(it != _N.end())
		{
			obj = (NLIAAGENT::IObjectIA *)obj->getStaticMember(*it++);
		}
		
		context.Self = obj;
		saveConstext(context);

		NLIAAGENT::IObjectIA::CProcessResult i;
		if(_Inheritance) i = ((NLIAAGENT::IObjectIA *)context.Self)->runMethodeMember(_Inheritance,_I,&context);
		else i = ((NLIAAGENT::IObjectIA *)context.Self)->runMethodeMember(_I,&context);

		context.Self = selfTmp;
		loadConstext(context);
		CVarPStackParam::_Shift = sp;
		return NLIAAGENT::processIdle;
	}
}
