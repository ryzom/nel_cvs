/** \file agent_proxy_mailer.cpp
 *
 * $Id: agent_proxy_mailer.cpp,v 1.5 2001/02/08 17:27:53 chafik Exp $
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

#include "nel/ai/agent/agent_proxy_mailer.h"
#include "nel/ai/agent/agent_method_def.h"
#include "nel/ai/script/type_def.h"
#include "nel/ai/script/object_unknown.h"
#include "nel/ai/agent/agent_object.h"

namespace NLAIAGENT
{
	IMainAgent *CProxyAgentMail::MainAgent = NULL;

	CProxyAgentMail::CMethodCall **CProxyAgentMail::StaticMethod = NULL;

	void CProxyAgentMail::initClass()
	{
		CProxyAgentMail::StaticMethod =  new CProxyAgentMail::CMethodCall *[CProxyAgentMail::TLast];
		CProxyAgentMail::StaticMethod[TConstructor]= new CProxyAgentMail::CMethodCall(
			_CONSTRUCTOR_,					
			CProxyAgentMail::TConstructor,			
			new NLAISCRIPT::CParam(1,new NLAISCRIPT::COperandSimple(new NLAIC::CIdentType(CStringType::IdStringType))),
			CProxyAgentMail::CheckAll,
			1,
			new NLAISCRIPT::CObjectUnknown(new NLAISCRIPT::COperandVoid));
	}

	void CProxyAgentMail::releaseClass()
	{
		sint i;
		for(i = 0; i < CProxyAgentMail::TLast; i ++)
		{
			delete CProxyAgentMail::StaticMethod[i];
		}
		delete CProxyAgentMail::StaticMethod;
	}

	CProxyAgentMail::CProxyAgentMail():IBasicAgent((IWordNumRef *)NULL),_AgentRef(NULL)
	{
	}
	CProxyAgentMail::CProxyAgentMail(const CAgentNumber &agentRef):IBasicAgent((IWordNumRef *)NULL),_AgentRef(new CAgentNumber(agentRef))
	{
	}
	CProxyAgentMail::~CProxyAgentMail()
	{
		if(_AgentRef != NULL) delete _AgentRef;
	}

	void CProxyAgentMail::onKill(IConnectIA *a)
	{
	}

	std::list<IBasicAgent *> listBidon;
	std::list<IBasicAgent *>::iterator CProxyAgentMail::addChild(IBasicAgent *p)
	{
		return listBidon.begin();
	}

	void CProxyAgentMail::removeChild(const IBasicAgent *p)
	{
	}

	void CProxyAgentMail::removeChild(std::list<IBasicAgent *>::iterator &iter)
	{		
	}

	void CProxyAgentMail::runChildren()
	{
	}
	void CProxyAgentMail::processMessages()
	{
	}

	IObjectIA::CProcessResult CProxyAgentMail::sendMessage(IObjectIA *msg)
	{
		return IObjectIA::CProcessResult();
	}

	tQueue CProxyAgentMail::isMember(const IVarName *h,const IVarName *m,const IObjectIA &param) const
	{

		tQueue a;
		sint i;
		CProxyAgentMail::TMethodNumDef index = CProxyAgentMail::TLast;

		if(h == NULL)
		{
			for( i = 0; i< CProxyAgentMail::TLast; i++)
			{
				if(CProxyAgentMail::StaticMethod[i]->MethodName == *m)
				{
					index = (CProxyAgentMail::TMethodNumDef)CProxyAgentMail::StaticMethod[i]->Index;
					switch(CProxyAgentMail::StaticMethod[i]->CheckArgType)
					{
						case CProxyAgentMail::CheckAll:
							{
								double d = ((NLAISCRIPT::CParam &)*CProxyAgentMail::StaticMethod[i]->ArgType).eval((NLAISCRIPT::CParam &)param);
								if(d >= 0.0)
								{								
									tQueue r;
									CProxyAgentMail::StaticMethod[i]->ReturnValue->incRef();
									r.push(CIdMethod(	(getMethodIndexSize() + CProxyAgentMail::StaticMethod[i]->Index),
														0.0,
														NULL,
														CProxyAgentMail::StaticMethod[i]->ReturnValue));
									return r;
								}
							}
							break;
						case CProxyAgentMail::CheckCount:
							{
								if(((NLAISCRIPT::CParam &)param).size() == CProxyAgentMail::StaticMethod[i]->ArgCount)
								{								
									tQueue r;
									CProxyAgentMail::StaticMethod[i]->ReturnValue->incRef();
									r.push(CIdMethod(	(getMethodIndexSize() + CProxyAgentMail::StaticMethod[i]->Index),
														0.0,
														NULL,
														CProxyAgentMail::StaticMethod[i]->ReturnValue ));
									return r;
								}
							}
							break;
						case CProxyAgentMail::DoNotCheck:
							{							
								tQueue r;
								CProxyAgentMail::StaticMethod[i]->ReturnValue->incRef();
								r.push(CIdMethod(	(getMethodIndexSize() + CProxyAgentMail::StaticMethod[i]->Index),
													0.0,
													NULL,
													CProxyAgentMail::StaticMethod[i]->ReturnValue));
								return r;						
							}					
							break;
				
					}
				}
			}
		}

		return tQueue();
	}

	IObjectIA::CProcessResult CProxyAgentMail::runMethodeMember(sint32 h, sint32 m, IObjectIA *p)
	{
		return runMethodeMember(m,p);
	}
	IObjectIA::CProcessResult CProxyAgentMail::runMethodeMember(sint32 m,IObjectIA *p)
	{
		switch(m)
		{
		case CProxyAgentMail::TConstructor:
			{
				CStringType *n = (CStringType *)((IBaseGroupType *)p)->get();
				if(_AgentRef != NULL) delete _AgentRef;
				_AgentRef = new CAgentNumber(n->getStr().getString());
			}
			break;
		}
		return IObjectIA::CProcessResult();
	}
	sint32 CProxyAgentMail::getMethodIndexSize() const
	{
		return 0;
	}
}
