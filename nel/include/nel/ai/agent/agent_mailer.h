/** \file agent_mailer.h
 * Sevral class for mailing message to an agent.
 *
 * $Id: agent_mailer.h,v 1.8 2001/01/25 08:56:23 chafik Exp $
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
#ifndef NL_AGENT_MAILER_H
#define NL_AGENT_MAILER_H

#include "nel/ai/agent/agent.h"

namespace NLAIAGENT
{	
	/**
	This class define an agent that it can assume the local communication role of an ather agent. This agent can be considered as an mail box agent.


	* \author Chafik sameh	 	
	* \author Nevrax France
	* \date 2000	
	*/
	class CLocalAgentMail: public IBasicAgent
	{

	public:
		static const NLAIC::CIdentType LocalAgentMail;
	private:
		IBasicAgent *_HostAgent;

	public:
		CLocalAgentMail():IBasicAgent(NULL),_HostAgent(NULL)
		{
		}
		CLocalAgentMail(IBasicAgent *host);
		virtual ~CLocalAgentMail();

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
			NLAIC::IBasicType *x = new CLocalAgentMail();
			//incRef();
			return x;
		}

		virtual const NLAIC::IBasicType *clone() const
		{
			//if(_HostAgent != NULL) _HostAgent->incRef();
			NLAIC::IBasicType *x = new CLocalAgentMail(_HostAgent);			
			return x;
		}

		virtual void getDebugString(char *t) const
		{
			if(_HostAgent != NULL) 	sprintf(t,"CLocalAgentMail for '%s' agents",(const char *)_HostAgent->getType());
			else sprintf(t,"CLocalAgentMail for 'NILL' agents");
		}
		//@}

		/// \name IObjectIA method.
		//@{
		virtual const CProcessResult &run()
		{
			return IObjectIA::ProcessRun;
		}

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

		virtual void setStaticMember(sint32 i,IObjectIA *o)
		{
			_HostAgent->setStaticMember(i,o);
		}

		virtual sint32 getMethodIndexSize() const
		{
			return _HostAgent->getMethodIndexSize();
		}

		virtual tQueue isMember(const IVarName *h,const IVarName *m,const IObjectIA &p) const;
		
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

		virtual	tQueue canProcessMessage(const IVarName &m)
		{
			return _HostAgent->canProcessMessage(m);
		}
		//@}

		/// \name IBasicObjectIA method.
		//@{
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
		virtual void runChildren();
		virtual void processMessages();			
		virtual IObjectIA::CProcessResult sendMessage(IObjectIA *msg)
		{
			return ((IObjectIA  *)_HostAgent)->sendMessage(msg);
		}
		virtual IObjectIA::CProcessResult sendMessage(IMessageBase *msg, IBasicAgent &receiver)
		{
			return _HostAgent->sendMessage(msg,receiver);
		}
		//@}		

	};
}

#endif
