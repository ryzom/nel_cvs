/** \file agent_local_mailer.h
 * Sevral class for mailing message to an agent.
 *
 * $Id: agent_local_mailer.h,v 1.8 2003/01/21 11:24:25 chafik Exp $
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
#ifndef NL_AGENT_LOCAL_MAILER_H
#define NL_AGENT_LOCAL_MAILER_H

#include "nel/ai/agent/agent.h"

namespace NLAIAGENT
{	
	/**
	This class define an agent that it can assume the local communication role of an ather agent. This agent can be considered as an mail box agent.


	* \author Chafik sameh	 	
	* \author Nevrax France
	* \date 2000	
	*/
	class CLocalAgentMail: public IAgent
	{

	public:
		static const NLAIC::CIdentType LocalAgentMail;
	private:
		IBasicAgent *_HostAgent; ///host agent where we have to establish communication.

	public:
		CLocalAgentMail():IAgent(NULL),_HostAgent(NULL)
		{
		}
		CLocalAgentMail(IBasicAgent *host);
		virtual ~CLocalAgentMail();

		const IBasicAgent *getHost() const
		{
			return _HostAgent;
		}

		/// \name IBasicInterface method.
		//@{
		virtual void save(NLMISC::IStream &os)
		{
			IBasicAgent::save(os);
			IWordNumRef &r = (IWordNumRef&)*((IConnectIA *)_HostAgent);
			os.serial( (NLAIC::CIdentType &) (r.getType()) );
			r.save(os);
		}
		virtual void load(NLMISC::IStream &is)
		{
			IBasicAgent::load(is);

			NLAIC::CIdentTypeAlloc id;
			is >> id;
			IWordNumRef *num = (IWordNumRef *)id.allocClass();
			num->load(is);
			if(_HostAgent != NULL) _HostAgent->release();
			_HostAgent = (IBasicAgent *)((const IRefrence *)*num);
			delete num;
		}
		virtual const NLAIC::CIdentType &getType() const
		{
			return LocalAgentMail;
		}

		virtual const NLAIC::IBasicType *newInstance() const
		{			
			NLAIC::IBasicType *x = new CLocalAgentMail(_HostAgent);
			//incRef();
			return x;
		}

		virtual const NLAIC::IBasicType *clone() const
		{
			//if(_HostAgent != NULL) _HostAgent->incRef();
			NLAIC::IBasicType *x = new CLocalAgentMail(_HostAgent);			
			return x;
		}

		virtual void getDebugString(std::string &t) const
		{
			if(_HostAgent != NULL) 	t += NLAIC::stringGetBuild("CLocalAgentMail for '%s' agents",(const char *)_HostAgent->getType());
			else t += "CLocalAgentMail for 'NILL' agents";
		}
		//@}

		/// \name IObjectIA method.
		//@{
		///Have no run to do
		virtual const CProcessResult &run()
		{
			return IObjectIA::ProcessRun;
		}

		//@{
		///Member are th host member attribut.
		virtual sint32 getStaticMemberSize() const
		{
			return _HostAgent->getStaticMemberSize();
		}

		virtual sint32 getStaticMemberIndex(const IVarName &m) const
		{
			return _HostAgent->getStaticMemberIndex(m);
		}

		virtual const IObjectIA *getStaticMember(sint32 i) const
		{
			return _HostAgent->getStaticMember(i);
		}

		virtual bool setStaticMember(sint32 i,IObjectIA *o)
		{
			return _HostAgent->setStaticMember(i,o);
		}

		virtual sint32 getMethodIndexSize() const
		{
			return _HostAgent->getMethodIndexSize();
		}
		//@}

		virtual TQueue isMember(const IVarName *h,const IVarName *m,const IObjectIA &p) const;
		
		virtual sint32 isClassInheritedFrom(const IVarName &h) const
		{
			return _HostAgent->isClassInheritedFrom(h);
		}

		virtual	CProcessResult runMethodeMember(sint32 h, sint32 m, IObjectIA *p)
		{
			return _HostAgent->runMethodeMember(h,m,p);
		}

		virtual	CProcessResult runMethodeMember(sint32 m,IObjectIA *p)
		{
			return _HostAgent->runMethodeMember(m,p);
		}		

		virtual	TQueue canProcessMessage(const IVarName &m)
		{
			return _HostAgent->canProcessMessage(m);
		}
		//@}

		/// \name IBasicObjectIA method.
		//@{
		//Note that is equal if the hosts agents is equal.
		virtual bool isEqual(const IBasicObjectIA &a) const
		{
			const CLocalAgentMail &l = (const CLocalAgentMail &)a;
			if(l._HostAgent != NULL && _HostAgent != NULL)
			{
				return _HostAgent->isEqual(*l._HostAgent);
			}
			else
			{
				return false;
			}
		}
		//@}

		/// \name IBasicAgent method.
		//@{
		virtual void onKill(IConnectIA *a);
		virtual std::list<IBasicAgent *>::iterator addChild(IBasicAgent *p)
		{
			return _HostAgent->addChild(p);
		}
		virtual void removeChild(const IBasicAgent *p)
		{
			_HostAgent->removeChild(p);
		}

		virtual void removeChild(std::list<IBasicAgent *>::iterator &iter)
		{
			_HostAgent->removeChild(iter);
		}

		virtual void runChildren();
		virtual void processMessages();			
		virtual IObjectIA::CProcessResult sendMessage(IObjectIA *msg);		
		//virtual IObjectIA::CProcessResult sendMessage(IMessageBase *msg, IBasicAgent &receiver);		

		///Mailer do'nt have own activity.
		virtual IObjectIA::CProcessResult runActivity()
		{
			return ProcessRun;
		}
		virtual bool haveActivity() const
		{
			return false;
		}	
		//@}		

	};
}

#endif
