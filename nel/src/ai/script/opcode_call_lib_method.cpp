/** \file opcode_call_lib_method.cpp
 *
 * $Id: opcode_call_lib_method.cpp,v 1.7 2001/06/14 10:23:18 chafik Exp $
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

namespace NLAISCRIPT
{
	NLAIAGENT::TProcessStatement CLibMemberMethod::runOpCode(CCodeContext &context)
	{
		NLAIAGENT::IObjectIA *param = context.Stack[(int)context.Stack];

		context.Param.push_back(param);
		param->incRef();

		NLAIAGENT::IObjectIA::CProcessResult r = ((NLAIAGENT::IObjectIA *)context.Self)->runMethodeMember(_Id,param);

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

	NLAIAGENT::TProcessStatement CLibMemberInheritedMethod::runOpCode(CCodeContext &context)
	{
		NLAIAGENT::IObjectIA *param = context.Stack[(int)context.Stack];
		NLAIAGENT::IObjectIA::CProcessResult r = ((NLAIAGENT::IObjectIA *)context.Self)->runMethodeMember(_Inheritance,_Id,param);

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

	NLAIAGENT::TProcessStatement CLibMemberMethodi::runOpCode(CCodeContext &context)
	{
		std::list<sint32>::iterator it = _I.begin();
		NLAIAGENT::IObjectIA *param = context.Stack[(int)context.Stack];
		NLAIAGENT::IObjectIA *obj = (NLAIAGENT::IObjectIA *)context.Self;

		context.Param.push_back(param);
		param->incRef();
		
		while(it != _I.end())
		{
			 obj = (NLAIAGENT::IObjectIA *)obj->getStaticMember(*it++);
		}

		NLAIAGENT::IObjectIA::CProcessResult r;

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

	NLAIAGENT::TProcessStatement CLibCallMethod::runOpCode(CCodeContext &context)
	{
		NLAIAGENT::IObjectIA *param = context.Stack[(int)context.Stack];
		NLAIAGENT::IObjectIA::CProcessResult r = _Lib->runMethodeMember(_Id,param);

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

	NLAIAGENT::TProcessStatement CLibCallInheritedMethod::runOpCode(CCodeContext &context)
	{
		NLAIAGENT::IObjectIA *param = context.Stack[(int)context.Stack];
		NLAIAGENT::IObjectIA::CProcessResult r = _Lib->runMethodeMember(_Inheritance,_Id,param);

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

	NLAIAGENT::TProcessStatement CLibCallMethodi::runOpCode(CCodeContext &context)
	{
		std::list<sint32>::iterator it = _I.begin();
		NLAIAGENT::IObjectIA *param = context.Stack[(int)context.Stack];
		NLAIAGENT::IObjectIA *obj = _Lib;
		while(it != _I.end())
		{
			 obj = (NLAIAGENT::IObjectIA *)obj->getStaticMember(*it++);
		}

		NLAIAGENT::IObjectIA::CProcessResult r;

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

	NLAIAGENT::TProcessStatement CLibStackMemberMethod::runOpCode(CCodeContext &context)
	{
		NLAIAGENT::IObjectIA *obj = context.Stack[(int)context.Stack - 1];
		obj->incRef();
		NLAIAGENT::IObjectIA *param = context.Stack[(int)context.Stack];
		param->incRef();
		context.Stack --;
		context.Stack --;
		std::list<sint32>::iterator it = _I.begin();
		while(it != _I.end())
		{
			 obj = (NLAIAGENT::IObjectIA *)obj->getStaticMember(*it++);
		}

		NLAIAGENT::IObjectIA::CProcessResult r;

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

	NLAIAGENT::TProcessStatement CLibStackNewMemberMethod::runOpCode(CCodeContext &context)
	{
		NLAIAGENT::IObjectIA *obj = context.Stack[(int)context.Stack];
		obj->incRef();
		NLAIAGENT::IObjectIA *param = context.Stack[(int)context.Stack - 1];		
		context.Stack --;
		context.Stack[(int)context.Stack] = obj;
		std::list<sint32>::iterator it = _I.begin();
		while(it != _I.end())
		{
			 obj = (NLAIAGENT::IObjectIA *)obj->getStaticMember(*it++);
		}

		NLAIAGENT::IObjectIA::CProcessResult r;

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

	NLAIAGENT::TProcessStatement CLibHeapMemberMethod::runOpCode(CCodeContext &context)
	{
		NLAIAGENT::IObjectIA *obj = context.Heap[(int)_Index];		
		NLAIAGENT::IObjectIA *param = context.Stack[(int)context.Stack];

		context.Param.push_back(param);
		param->incRef();

#ifdef NL_DEBUG
		int db_size = context.Param.size();
#endif
		
		std::list<sint32>::iterator it = _I.begin();
		while(it != _I.end())
		{
			 obj = (NLAIAGENT::IObjectIA *)obj->getStaticMember(*it++);
		}

		NLAIAGENT::IObjectIA::CProcessResult r;

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

#ifdef NL_DEBUG
		db_size = context.Param.size();
		if(!db_size) throw;
#endif
		NLAIAGENT::IObjectIA *p = context.Param.back();
		context.Param.pop_back();
		p->release();
		
		return r.ResultState;		
	}	
}
