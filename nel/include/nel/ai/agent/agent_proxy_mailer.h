/** \file agent_proxy_mailer.h
 * Sevral class for mailing message to an agent.
 *
 * $Id: agent_proxy_mailer.h,v 1.18 2003/01/21 11:24:25 chafik Exp $
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
#ifndef NL_AGENT_SERVER_MAILER_H
#define NL_AGENT_SERVER_MAILER_H
#include "nel/ai/agent/agent_script.h"

namespace NLAIAGENT
{	
	class IMainAgent;

	class CProxyAgentMail: public IBasicAgent
	{
	public:
		static const NLAIC::CIdentType IdProxyAgentMail;
	public:
		static IMainAgent *MainAgent;
	public:

		enum  TMethodNumDef {
			TConstructor,
			TLastM
		};		
		
		static CAgentScript::CMethodCall **StaticMethod;

		static void initClass();
		static void releaseClass();

	private:
		CAgentNumber *_AgentRef;
	public:
		CProxyAgentMail();
		CProxyAgentMail(const CAgentNumber &agentRef);
		CProxyAgentMail(const CProxyAgentMail &mailer);
		virtual ~CProxyAgentMail();

		/// \name IBasicInterface method.
		//@{
		virtual void save(NLMISC::IStream &os)
		{
			_AgentRef->save(os);
		}

		virtual void load(NLMISC::IStream &is)
		{
			delete _AgentRef;
			_AgentRef = new CAgentNumber (is);
		}

		virtual const NLAIC::CIdentType &getType() const
		{
			return IdProxyAgentMail;
		}

		virtual const NLAIC::IBasicType *newInstance() const
		{			
			NLAIC::IBasicType *x = new CProxyAgentMail();
			//incRef();
			return x;
		}

		virtual const NLAIC::IBasicType *clone() const
		{
			//if(_HostAgent != NULL) _HostAgent->incRef();
			if(_AgentRef != NULL) return new CProxyAgentMail(*_AgentRef);
			else return new CProxyAgentMail();			
		}

		virtual void getDebugString(std::string &t) const
		{
			if(_AgentRef != NULL)
			{
				std::string text;
				_AgentRef->getDebugString(text);
				t = NLAIC::stringGetBuild("CProxyAgentMail for '%s' agents",text.c_str());
			}
			else t = "CProxyAgentMail 'NILL' agents";
		}
		//@}
		/// \name IObjectIA method.
		//@{
		virtual const CProcessResult &run()
		{
			return IObjectIA::ProcessRun;
		}

		virtual bool isEqual(const IBasicObjectIA &a) const
		{
			const CProxyAgentMail &l = (const CProxyAgentMail &)a;
			if(l._AgentRef != NULL && _AgentRef != NULL)
			{
				return *l._AgentRef == *_AgentRef;
			}
			else
			{
				return false;
			}
		}

		virtual TQueue isMember(const IVarName *h,const IVarName *m,const IObjectIA &p) const;
		virtual	CProcessResult runMethodeMember(sint32 h, sint32 m, IObjectIA *p);
		virtual	CProcessResult runMethodeMember(sint32 m,IObjectIA *p);
		virtual sint32 getMethodIndexSize() const;
		//@}

		/// \name IBasicAgent method.
		//@{
		virtual void onKill(IConnectIA *a);
		virtual std::list<IBasicAgent *>::iterator addChild(IBasicAgent *p);
		virtual void removeChild(const IBasicAgent *p);
		virtual void removeChild(std::list<IBasicAgent *>::iterator &iter);
		virtual void runChildren();
		virtual void processMessages();
		virtual IObjectIA::CProcessResult sendMessage(IObjectIA *msg);
		virtual	IObjectIA::CProcessResult sendMessage(const IVarName &,IObjectIA *);
		virtual IObjectIA::CProcessResult runActivity()
		{
			return ProcessRun;
		}
		virtual bool haveActivity() const
		{
			return false;
		}
		//@}

		const CAgentNumber *getAgentRef() const
		{			
			return _AgentRef;			
		}


	};
}

#endif
