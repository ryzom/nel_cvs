/** \file agent.h
 * Sevral class for the definition of agent.
 *
 * $Id: agent.h,v 1.5 2001/01/08 14:39:59 valignat Exp $
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

#ifndef NL_AGENTS_H
#define NL_AGENTS_H

#include "nel/ai/agent/agentexport.h"
#include "nel/ai/agent/baseia.h"
#include "nel/ai/agent/mailbox.h"

namespace NLAIAGENT
{	
	typedef CLocalMailBox tMailBoxLettre;

	/**
	Base class for an agent.Basicly an agent is an IConnectIA that we can communicate with it and an basic agent can assume the operation of an children.

	* \author Chafik sameh	 
	* \author Portier Pierre
	* \author Nevrax France
	* \date 2000	
	*/
	class IBasicAgent : public IConnectIA
	{
	private:
		struct CMethodCall
		{
			CMethodCall(const char *name, int i): MethodName (name)
			{				
				Index = i;
			}
			CStringVarName MethodName;
			sint32 Index;
		};
		static CMethodCall _Method[];

	protected:
		///Internal state of the agent.
		CProcessResult	_RunState;
		///Box letter for communication service.
		IMailBox	*_Mail;

	protected:
		/**
		Copy constructor we protect it because agent need an unique ident, 
		and we don't wont to give this possibility to all programmer because it is dangerous.
		*/
		IBasicAgent(const IBasicAgent &a);

		/**
		Run message is the message processing.
		*/
		virtual IObjectIA *run(const IMessageBase &) throw (NLAIE::CExceptionNotImplemented)
		{
			char text[2048*8];
			sprintf(text,"Function <IObjectIA *run(const IMessageBase &)> note implementaited for the '%s' interface",(const char *)getType());
			throw NLAIE::CExceptionNotImplemented(text);
			return this;	
		}

	public:
		///Contruct agent with an parent.
		IBasicAgent(const IWordNumRef *parent);
		///Contruct agent with an parent and an mail box.
		IBasicAgent(const IWordNumRef *parent,IMailBox	*m);
		virtual ~IBasicAgent();

		///Get the curent state of the agent.
		virtual const CProcessResult &getState() const;
		///Set the state of an agent.
		virtual void setState(TProcessStatement s, IObjectIA *result);

		///Send a message to the agent.
		virtual IObjectIA::CProcessResult sendMessage(IMessageBase *msg);

		///Send message to an receiver agent.
		virtual IObjectIA::CProcessResult sendMessage(IMessageBase *msg, IBasicAgent &receiver);

		///Get the mail box letter.
		IMailBox *getMail() const;

		/// \name Some IBasicInterface method.
		//@{
		virtual void save(NLMISC::IStream &os);
		virtual void load(NLMISC::IStream &is);
		//@}

		/// \name Some IObjectIA method.
		//@{
		virtual sint32 getMethodIndexSize() const;		
		virtual tQueue isMember(const IVarName *,const IVarName *,const IObjectIA &) const;
		virtual	CProcessResult runMethodeMember(sint32, sint32, IObjectIA *);
		virtual	CProcessResult runMethodeMember(sint32 index,IObjectIA *);
		//@}

		/**
		Add a child to the agent. Child is added in a std::list<IBasicAgent *>.
		The method return the range of the child in the list.
		*/
		virtual std::list<IBasicAgent *>::iterator addChild(IBasicAgent *p) = 0;
		/**
		Remove a child from the std::list<IBasicAgent *> child list.
		*/
		virtual void removeChild(const IBasicAgent *p) = 0;

		/**
		Run all child.
		*/
		virtual void runChildren() = 0;

		/**
		Add a message acount in the mail box.
		*/
		virtual void addMsgGroup(IBasicMessageGroup &grp);
		/**
		Remove a message acount from the mail box.
		*/
		virtual void removeMsgGroup(IBasicMessageGroup &grp);

		/**
		The processMessages method, process the loop/Run message.
		*/
		virtual void processMessages() = 0;	

	public:
		/**
		The work of an agent. Typically agent process child message, process its message and run its activity.
		*/
		virtual const CProcessResult &run() = 0;
		
	};	

	/**
	Base class for child working in an agent. This class manage a child list.

	* \author Portier Pierre
	* \author Nevrax France
	* \date 2000	
	*/

	class IAgentComposite:public IBasicAgent
	{		
		private:
			typedef std::list<IBasicAgent *> tBasicList;

			void deleteListe();
	
		protected:		
			tBasicList	_AgentList;
			IAgentComposite(const IAgentComposite &a);

		public:
			///Contruct agent with an parent.
			IAgentComposite(IBasicAgent *parent);
			///Contruct agent with an parent and an mail box.
			IAgentComposite(IBasicAgent *parent,IMailBox *m);

			/// \name Some IBasicAgent method.
			//@{
			virtual std::list<IBasicAgent *>::iterator addChild(IBasicAgent *p);	// Ajoute un fils à l'agent.
			void cpyChild(const IBasicAgent &p);
			void removeChild(const IBasicAgent &p);		
			virtual void removeChild(const IBasicAgent *p);
			//@}

			/// \name Some IBasicInterface method.
			//@{
			virtual void save(NLMISC::IStream &os);
			virtual void load(NLMISC::IStream &is);		
			//@}

			virtual ~IAgentComposite();
	};

	class IAgent:public IAgentComposite
	{	
	public:	
		static const NLAIC::CIdentType IdAgent;
		
	protected:		
		IAgent(const IAgent &a);
		
		/// \name Some IBasicAgent method.
		//@{
		virtual void runChildren();		// Activates the child agents
		virtual IObjectIA *run(const IMessageBase &);
		//@}
	
	public:		
		///Contruct agent with an parent.
		IAgent(IBasicAgent *parent);
		///Contruct agent with an parent and an mail box.
		IAgent(IBasicAgent *parent,IMailBox *m);
		virtual ~IAgent();		
				
		/// \name Some IBasicAgent method.
		//@{
		virtual const NLAIC::IBasicType *clone() const;
		virtual const NLAIC::IBasicType *newInstance() const;
		virtual void getDebugString(char *t) const;
		virtual const NLAIC::CIdentType &getType() const;
		virtual void save(NLMISC::IStream &os);
		virtual void load(NLMISC::IStream &is);		
		//@}

		///Implementation of the isEqual of the IBasicObjectIA 
		virtual bool isEqual(const IBasicObjectIA &a) const;
		/// Called by an agent who's destroyed te remove its references
		virtual void onKill(IConnectIA *A);

		/// \name Some IBasicAgent method.
		//@{
		virtual void processMessages();
		virtual const CProcessResult &run();		
		//@}
	};
}
#endif
