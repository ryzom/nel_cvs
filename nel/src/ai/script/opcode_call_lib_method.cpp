/** \file opcode_call_lib_method.cpp
 *
 * $Id: opcode_call_lib_method.cpp,v 1.1 2001/01/05 10:53:49 chafik Exp $
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

	NLIAAGENT::TProcessStatement CLibMemberMethod::runOpCode(CCodeContext &context)
	{
		NLIAAGENT::IObjectIA *param = context.Stack[(int)context.Stack];

		context.Param.push_back(param);
		param->incRef();

		NLIAAGENT::IObjectIA::CProcessResult r = ((NLIAAGENT::IObjectIA *)context.Self)->runMethodeMember(_Id,param);

		if(r.Result != NULL)
		{
			param->release();
			context.Stack[(int)context.Stack] = r.Result;
		}
		else
		{
			context.Stack --;
		}
		
		context.Param.back()->release();
		context.Param.pop_back();
		return r.ResultState;
	}

	NLIAAGENT::TProcessStatement CLibMemberInheritedMethod::runOpCode(CCodeContext &context)
	{
		NLIAAGENT::IObjectIA *param = context.Stack[(int)context.Stack];
		NLIAAGENT::IObjectIA::CProcessResult r = ((NLIAAGENT::IObjectIA *)context.Self)->runMethodeMember(_Inheritance,_Id,param);

		context.Param.push_back(param);
		param->incRef();
		
		if(r.Result != NULL)
		{
			param->release();
			context.Stack[(int)context.Stack] = r.Result;
		}
		else
		{
			context.Stack --;
		}
		context.Param.back()->release();
		context.Param.pop_back();	
		return r.ResultState;
	}

	NLIAAGENT::TProcessStatement CLibMemberMethodi::runOpCode(CCodeContext &context)
	{
		std::list<sint32>::iterator it = _I.begin();
		NLIAAGENT::IObjectIA *param = context.Stack[(int)context.Stack];
		NLIAAGENT::IObjectIA *obj = (NLIAAGENT::IObjectIA *)context.Self;

		context.Param.push_back(param);
		param->incRef();
		
		while(it != _I.end())
		{
			 obj = (NLIAAGENT::IObjectIA *)obj->getStaticMember(*it++);
		}

		NLIAAGENT::IObjectIA::CProcessResult r;

		if(_Inheritance )  r = obj->runMethodeMember(_Inheritance,_Id,param);
		else r = obj->runMethodeMember(_Id,param);
		
		if(r.Result != NULL)
		{
			param->release();
			context.Stack[(int)context.Stack] = r.Result;
		}
		else
		{
			context.Stack --;
		}
		context.Param.back()->release();
		context.Param.pop_back();
		return r.ResultState;
	}

	NLIAAGENT::TProcessStatement CLibCallMethod::runOpCode(CCodeContext &context)
	{
		NLIAAGENT::IObjectIA *param = context.Stack[(int)context.Stack];
		NLIAAGENT::IObjectIA::CProcessResult r = _Lib->runMethodeMember(_Id,param);

		context.Param.push_back(param);
		param->incRef();

		if(r.Result != NULL)
		{
			param->release();
			context.Stack[(int)context.Stack] = r.Result;
		}
		else
		{
			context.Stack --;
		}
		context.Param.back()->release();
		context.Param.pop_back();
		return r.ResultState;
	}

	NLIAAGENT::TProcessStatement CLibCallInheritedMethod::runOpCode(CCodeContext &context)
	{
		NLIAAGENT::IObjectIA *param = context.Stack[(int)context.Stack];
		NLIAAGENT::IObjectIA::CProcessResult r = _Lib->runMethodeMember(_Inheritance,_Id,param);

		context.Param.push_back(param);
		param->incRef();
		
		if(r.Result != NULL)
		{
			param->release();
			context.Stack[(int)context.Stack] = r.Result;
		}
		else
		{
			context.Stack --;
		}
		context.Param.back()->release();
		context.Param.pop_back();
		return r.ResultState;
	}

	NLIAAGENT::TProcessStatement CLibCallMethodi::runOpCode(CCodeContext &context)
	{
		std::list<sint32>::iterator it = _I.begin();
		NLIAAGENT::IObjectIA *param = context.Stack[(int)context.Stack];
		NLIAAGENT::IObjectIA *obj = _Lib;
		while(it != _I.end())
		{
			 obj = (NLIAAGENT::IObjectIA *)obj->getStaticMember(*it++);
		}

		NLIAAGENT::IObjectIA::CProcessResult r;

		context.Param.push_back(param);
		param->incRef();

		if(_Inheritance )  r = obj->runMethodeMember(_Inheritance,_Id,param);
		else r = obj->runMethodeMember(_Id,param);
		
		if(r.Result != NULL)
		{
			param->release();
			context.Stack[(int)context.Stack] = r.Result;
		}
		else
		{
			context.Stack --;
		}
		context.Param.back()->release();
		context.Param.pop_back();
		return r.ResultState;
	}

	NLIAAGENT::TProcessStatement CLibStackMemberMethod::runOpCode(CCodeContext &context)
	{
		NLIAAGENT::IObjectIA *obj = context.Stack[(int)context.Stack - 1];
		obj->incRef();
		NLIAAGENT::IObjectIA *param = context.Stack[(int)context.Stack];
		param->incRef();
		context.Stack --;
		context.Stack --;
		std::list<sint32>::iterator it = _I.begin();
		while(it != _I.end())
		{
			 obj = (NLIAAGENT::IObjectIA *)obj->getStaticMember(*it++);
		}

		NLIAAGENT::IObjectIA::CProcessResult r;

		context.Param.push_back(param);
		param->incRef();

		if(_H )  r = obj->runMethodeMember(_H,_Id,param);
		else r = obj->runMethodeMember(_Id,param);
		
		param->release();
		if(r.Result != NULL)
		{			
			context.Stack ++;
			context.Stack[(int)context.Stack] = r.Result;
		}

		context.Param.back()->release();
		context.Param.pop_back();
		return r.ResultState;
	}

	NLIAAGENT::TProcessStatement CLibStackNewMemberMethod::runOpCode(CCodeContext &context)
	{
		NLIAAGENT::IObjectIA *obj = context.Stack[(int)context.Stack];
		obj->incRef();
		NLIAAGENT::IObjectIA *param = context.Stack[(int)context.Stack - 1];		
		context.Stack --;
		context.Stack[(int)context.Stack] = obj;
		std::list<sint32>::iterator it = _I.begin();
		while(it != _I.end())
		{
			 obj = (NLIAAGENT::IObjectIA *)obj->getStaticMember(*it++);
		}

		NLIAAGENT::IObjectIA::CProcessResult r;

		context.Param.push_back(param);
		param->incRef();

		if(_H )  r = obj->runMethodeMember(_H,_Id,param);
		else r = obj->runMethodeMember(_Id,param);
		param->release();
		if(r.Result != NULL)
		{			
			context.Stack ++;
			context.Stack[(int)context.Stack] = r.Result;
		}

		context.Param.back()->release();
		context.Param.pop_back();
		
		return r.ResultState;
	}

	NLIAAGENT::TProcessStatement CLibHeapMemberMethod::runOpCode(CCodeContext &context)
	{
		NLIAAGENT::IObjectIA *obj = context.Heap[(int)_Index];		
		NLIAAGENT::IObjectIA *param = context.Stack[(int)context.Stack];

		context.Param.push_back(param);
		param->incRef();
		
		std::list<sint32>::iterator it = _I.begin();
		while(it != _I.end())
		{
			 obj = (NLIAAGENT::IObjectIA *)obj->getStaticMember(*it++);
		}

		NLIAAGENT::IObjectIA::CProcessResult r;

		if(_H )  r = obj->runMethodeMember(_H,_Id,param);
		else r = obj->runMethodeMember(_Id,param);
		
		if(r.Result != NULL)
		{
			param->release();
			context.Stack[(int)context.Stack] = r.Result;
		}
		else
		{
			context.Stack --;
		}

		
		context.Param.back()->release();
		context.Param.pop_back();
		
		return r.ResultState;		
	}	
}
