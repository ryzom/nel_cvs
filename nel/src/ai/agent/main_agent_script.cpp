/** \file mai_agent_script.cpp
 *
 * $Id: main_agent_script.cpp,v 1.8 2001/01/17 10:42:55 chafik Exp $
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
#include "nel/ai/script/codage.h"
#include "nel/ai/script/interpret_object_agent.h"
#include "nel/ai/agent/main_agent_script.h"

namespace NLAIAGENT
{
	CMainAgentScript::CMainAgentScript(const CMainAgentScript &a): CAgentScript(a)
	{
		_Stack = new NLAISCRIPT::CStackPointer();
		_Heap = new NLAISCRIPT::CStackPointer();

		_CodeContext = new NLAISCRIPT::CCodeContext(*_Stack,*_Heap,NULL,this,a._CodeContext->InputOutput);
	}

	CMainAgentScript::CMainAgentScript(IAgentManager *main,NLAIC::IIO *io):CAgentScript (main)
	{
		_Stack = new NLAISCRIPT::CStackPointer();
		_Heap = new NLAISCRIPT::CStackPointer();
		_CodeContext = new NLAISCRIPT::CCodeContext(*_Stack,*_Heap,NULL,this,io);
	}
	
	CMainAgentScript::CMainAgentScript(NLAIC::IIO *io):CAgentScript (NULL)
	{		
		_Stack = new NLAISCRIPT::CStackPointer();
		_Heap = new NLAISCRIPT::CStackPointer();
		_CodeContext = new NLAISCRIPT::CCodeContext(*_Stack,*_Heap,NULL,this,io);
	}

	CMainAgentScript::~CMainAgentScript()
	{
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

	IObjectIA *CMainAgentScript::run(const IMessageBase &m) ///throw throw Exc::CExceptionNotImplemented;
	{
		//Super crados.		
		if(m.getGroup().getId() == 1)
		{
			/*NLAIC::IIO *io = _CodeContext->InputOutput;
			_CodeContext->release();
			_CodeContext = new NLAISCRIPT::CCodeContext(*_Stack,*_Heap,NULL,this,io);
			_CodeContext->incRef();*/

			_CodeContext->Self = this;

			CIteratorContener i = m.getIterator();
			NLAISCRIPT::CCodeBrancheRun *o = (NLAISCRIPT::CCodeBrancheRun *)i++;			
			_CodeContext->Code = o;			
//			IObjectIA::CProcessResult r = o->run(*this);
			IObjectIA::CProcessResult r = o->run(*_CodeContext);
			//_CodeContext->init();
			//o->init();
		}
		return NULL;
	}

	void CMainAgentScript::processMessages()
	{
		if(getFactoryClass() != NULL)
		{		
			NLAISCRIPT::CCodeContext &context = (NLAISCRIPT::CCodeContext &)*getAgentManager()->getAgentContext();
			while(getLocalMailBox()->getMessageCount())
			{
				const IMessageBase &msg = ((IMailBox *)getLocalMailBox())->getMessage();
				IBaseGroupType *param = new CGroupType();
				param->push(&msg);
				context.Stack ++;
				context.Stack[(int)context.Stack] = param;
				if(msg.getHeritanceIndex())
				{
					runMethodeMember(msg.getHeritanceIndex(),msg.getMethodIndex(),&context);
				}			
			}
		}
		IAgent::processMessages();
	}

	const IObjectIA::CProcessResult &CMainAgentScript::run()
	{
		setState(processBuzzy,NULL);

		runChildren();

		((IMailBox *)getLocalMailBox())->run();
		getMail()->run();
		
		if(getFactoryClass() != NULL && getFactoryClass()->getRunMethod() >= 0) 
		{
			runMethodeMember(getFactoryClass()->getRunMethod(),_CodeContext);
		}				

		processMessages();

		setState(processIdle,NULL);
		return getState();
	}

	IObjectIA::CProcessResult CMainAgentScript::addDynamicAgent(IBaseGroupType *g)
	{
		CIteratorContener i = g->getIterator();
		i++;
		IBasicAgent *o = (IBasicAgent *)i++;

		if(((const NLAIC::CTypeOfObject &)o->getType()) & NLAIC::CTypeOfObject::tAgentInterpret)
		{
			((CAgentScript *)o)->setAgentManager(this);
		}
		
		return CAgentScript::addDynamicAgent(g);		
	}
}
