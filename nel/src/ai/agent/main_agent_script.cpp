/** \file main_agent_script.cpp
 *
 * $Id: main_agent_script.cpp,v 1.31 2002/08/21 13:58:33 lecroart Exp $
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
#include "nel/ai/agent/agent.h"
#include "nel/ai/agent/main_agent_script.h"
#include "nel/ai/e/ai_exception.h"

namespace NLAIAGENT
{

	const sint StackMemSize = 256;//1024*32;
	CMainAgentScript::CMainAgentScript(const CMainAgentScript &a): IMainAgent(a)
	{
		_Stack = new NLAISCRIPT::CStackPointer(StackMemSize);
		_Heap = new NLAISCRIPT::CStackPointer(StackMemSize);

		_CodeContext = new NLAISCRIPT::CCodeContext(*_Stack,*_Heap,NULL,this,a._CodeContext->InputOutput);
	}

	CMainAgentScript::CMainAgentScript(IAgentManager *main,NLAIC::IIO *io):IMainAgent (main)
	{
		_Stack = new NLAISCRIPT::CStackPointer(StackMemSize);
		_Heap = new NLAISCRIPT::CStackPointer(StackMemSize);
		_CodeContext = new NLAISCRIPT::CCodeContext(*_Stack,*_Heap,NULL,this,io);
	}

	CMainAgentScript::CMainAgentScript(IAgentManager *a,NLAIC::IIO *io, IBasicAgent *b, std::list<IObjectIA *> &v, NLAISCRIPT::CAgentClass *c):IMainAgent(a,b,v,c)
	{
		_Stack = new NLAISCRIPT::CStackPointer(StackMemSize);
		_Heap = new NLAISCRIPT::CStackPointer(StackMemSize);
		_CodeContext = new NLAISCRIPT::CCodeContext(*_Stack,*_Heap,NULL,this,io);
	}
	
	CMainAgentScript::CMainAgentScript(NLAIC::IIO *io):IMainAgent (NULL)
	{		
		_Stack = new NLAISCRIPT::CStackPointer(StackMemSize);
		_Heap = new NLAISCRIPT::CStackPointer(StackMemSize);
		_CodeContext = new NLAISCRIPT::CCodeContext(*_Stack,*_Heap,NULL,this,io);
	}

	CMainAgentScript::~CMainAgentScript()
	{
		Kill();
		_CodeContext->release();
		delete _Stack;
		delete _Heap;
	}

	const IObjectIA *CMainAgentScript::getAgentContext() const
	{
		return _CodeContext;
	}

	int CMainAgentScript::getBaseMethodCount() const
	{
		return CAgentScript::getBaseMethodCount();
	}

	IObjectIA::CProcessResult CMainAgentScript::sendMessage(IObjectIA *m)
	{
		return CAgentScript::sendMessage(m);
	}

	const NLAIC::IBasicType *CMainAgentScript::clone() const
	{
		NLAIC::IBasicType *x = new CMainAgentScript(*this);
		return x;
	}
	
	const NLAIC::IBasicType *CMainAgentScript::newInstance() const
	{
		NLAIC::IBasicType *x = new CMainAgentScript(_CodeContext->InputOutput);
		return x;
	}

	IMessageBase *CMainAgentScript::runExec(const IMessageBase &m)
	{
		if(m.getGroup().getId() == 1)
		{			
			_CodeContext->Self = this;

			CConstIteratorContener i = m.getConstIterator();
			NLAISCRIPT::CCodeBrancheRun *o = (NLAISCRIPT::CCodeBrancheRun *)i++;						
			NLAISCRIPT::CStackPointer stack;
			NLAISCRIPT::CStackPointer heap;
			_CodeContext->InputOutput->incRef();
			NLAISCRIPT::CCodeContext codeContext(stack,heap,NULL,this,_CodeContext->InputOutput);
			codeContext.Code = o;
			(void)o->run(codeContext);
			heap -= (int)heap;
			stack -= (int)stack;
		}
		return NULL;
	}	

	void CMainAgentScript::processMessages()
	{
		/*if(getFactoryClass() != NULL)
		{		
			NLAISCRIPT::CCodeContext &context = (NLAISCRIPT::CCodeContext &)*getAgentManager()->getAgentContext();
			while(getMail()->getMessageCount())
			{
				const IMessageBase &msg = getMail()->getMessage();
				IBaseGroupType *param = new CGroupType();
				param->push(&msg);
				context.Stack ++;
				context.Stack[(int)context.Stack] = param;
				if(msg.getHeritanceIndex())
				{
					runMethodeMember(msg.getHeritanceIndex(),msg.getMethodIndex(),&context);
				}			
			}
		}*/
		try
		{
			CAgentScript::processMessages();
		}
		catch(NLAIE::IException &e)
		{			
			const char *w = e.what();
			_CodeContext->InputOutput->Echo("\n\n%s\n\n",(char *)w);
		}
	}

	const IObjectIA::CProcessResult &CMainAgentScript::run()
	{		
		runChildren();

		//((IMailBox *)getLocalMailBox())->run();
		getMail()->run();
		
		if(getFactoryClass() != NULL && getFactoryClass()->getRunMethod() >= 0) 
		{
			runMethodeMember(getFactoryClass()->getRunMethod(),_CodeContext);
		}				

		processMessages();

		if(haveActivity() && getState().ResultState == processIdle) runActivity();
		
		return getState();
	}

	IObjectIA::CProcessResult CMainAgentScript::addDynamicAgent(IBaseGroupType *g)
	{
		CIteratorContener i = g->getIterator();
		i++;
		IBasicAgent *o = (IBasicAgent *)i++;

		uint b = NLAIC::CTypeOfObject::tInterpret | NLAIC::CTypeOfObject::tAgent;
		const NLAIC::CTypeOfObject &t = o->getType();
		if((t.getValue() & b) == b)
		{
			((CAgentScript *)o)->setAgentManager(this);
		}
		
		return CAgentScript::addDynamicAgent(g);		
	}
}
