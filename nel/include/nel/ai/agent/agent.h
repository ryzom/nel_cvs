/** \file agent.h
 * Sevral class for the definition of agent.
 *
 * $Id: agent.h,v 1.22 2002/11/15 09:20:53 chafik Exp $
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
#include "nel/ai/agent/baseai.h"
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
		virtual IObjectIA *run(const IMessageBase &m);		


		

		/**
		RunExec is to processing reltative to PExec Performatif. its call from the IObjectIA *run(const IMessageBase &m) method.
		*/
		virtual IMessageBase *runExec(const IMessageBase &m)
		{
			std::string debugString;
			std::string text;
			getDebugString(debugString);
			text = "IMessageBase *runExec(";
			text += (const char *)m.getType();
			text += " note implementaited for the '";
			text += (const char *)getType();
			text += "' interface for the instence '";
			text += (const char *)getType();
			text += "' '";
			text += debugString;
			text += "'";
			
			throw NLAIE::CExceptionNotImplemented(text.c_str());
			return NULL;
		}

		/**
		RunExec is to processing reltative to PExec Performatif. its call from the IObjectIA *run(const IMessageBase &m) method.
		*/
		virtual IMessageBase *runEven(const IMessageBase &m)
		{
			std::string debugString;
			std::string text;
			getDebugString(debugString);
			text = "IMessageBase *runEven(";
			text += (const char *)m.getType();
			text += " note implementaited for the '";
			text += (const char *)getType();
			text += "' interface for the instence '";
			text += (const char *)getType();
			text += "' '";
			text += debugString;
			text += "'";

			throw NLAIE::CExceptionNotImplemented(text.c_str());
			return NULL;
		}

		/**
		runAchieve ist to processing reltative to PAchieve Performatif. its call from the IObjectIA *run(const IMessageBase &m) method.
		*/
		virtual IMessageBase *runAchieve(const IMessageBase &m)
		{
			std::string debugString;
			std::string text;
			getDebugString(debugString);
			text = "IMessageBase *runAchieve(";
			text += (const char *)m.getType();
			text += " note implementaited for the '";
			text += (const char *)getType();
			text += "' interface for the instence '";
			text += (const char *)getType();
			text += "' '";
			text += debugString;
			text += "'";
			
			throw NLAIE::CExceptionNotImplemented(text.c_str());
			return NULL;
		}

		/**
		runAsk is to processing reltative to PAsk Performatif. its call from the IObjectIA *run(const IMessageBase &m) method. 
		*/
		virtual IMessageBase *runAsk(const IMessageBase &m);		

		/**
		runTell is to processing reltative to PTell Performatif. its call from the IObjectIA *run(const IMessageBase &m) method. 
		*/
		virtual IMessageBase *runTell(const IMessageBase &m);		

		/**
		runBreak is to processing reltative to PBreak Performatif. its call from the IObjectIA *run(const IMessageBase &m) method. 
		*/
		virtual IMessageBase *runBreak(const IMessageBase &m)
		{
			std::string debugString;
			std::string text;
			getDebugString(debugString);
			text = "IMessageBase *runBreak(";
			text += (const char *)m.getType();
			text += " note implementaited for the '";
			text += (const char *)getType();
			text += "' interface for the instence '";
			text += (const char *)getType();
			text += "' '";
			text += debugString;
			text += "'";
			
			throw NLAIE::CExceptionNotImplemented(text.c_str());
			return NULL;
		}

		/// Called by an agent who's destroyed te remove its references
		virtual void onKill(IConnectIA *A);

		/**
		runKill is to processing reltative to PKill Performatif. its call from the IObjectIA *run(const IMessageBase &m) method. 
		*/
		virtual IMessageBase *runKill(const IMessageBase &m)
		{
			std::string debugString;
			std::string text;
			getDebugString(debugString);
			text = "IMessageBase *runKill(";
			text += (const char *)m.getType();
			text += " note implementaited for the '";
			text += (const char *)getType();
			text += "' interface for the instence '";
			text += (const char *)getType();
			text += "' '";
			text += debugString;
			text += "'";
			
			throw NLAIE::CExceptionNotImplemented(text.c_str());
			return NULL;
		}

		/**
		runError is to processing reltative to PError Performatif. its call from the IObjectIA *run(const IMessageBase &m) method. 
		*/
		virtual IMessageBase *runError(const IMessageBase &m)
		{
			std::string debugString;
			std::string text;
			getDebugString(debugString);
			text = "IMessageBase *runError(";
			text += (const char *)m.getType();
			text += " note implementaited for the '";
			text += (const char *)getType();
			text += "' interface for the instence '";
			text += (const char *)getType();
			text += "' '";
			text += debugString;
			text += "'";
			
			throw NLAIE::CExceptionNotImplemented(text.c_str());
			return NULL;
		}

		/**
		runService is to processing reltative to PService Performatif. its call from the IObjectIA *run(const IMessageBase &m) method. 
		*/
		virtual IMessageBase *runService(const IMessageBase &m)
		{
			std::string debugString;
			std::string text;
			getDebugString(debugString);
			text = "IMessageBase *runService(";
			text += (const char *)m.getType();
			text += " note implementaited for the '";
			text += (const char *)getType();
			text += "' interface for the instence '";
			text += (const char *)getType();
			text += "' '";
			text += debugString;
			text += "'";
			
			throw NLAIE::CExceptionNotImplemented(text.c_str());
			return NULL;
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

		///Run the activity process an agent.
		virtual IObjectIA::CProcessResult runActivity() = 0;		

		///allow to know if the agent have an activity process to run.
		virtual bool haveActivity() const = 0;

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
		Remove a child from the std::list<IBasicAgent *> child list.
		*/
		virtual void removeChild(std::list<IBasicAgent *>::iterator &iter) = 0;

		/**
		Run all child.
		*/
		virtual void runChildren() = 0;		

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
			sint _SizeChild;

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
			virtual void removeChild(std::list<IBasicAgent *>::iterator &iter);
			//@}
			virtual sint getChildCoun() const
			{
				return _SizeChild;
			}

			virtual void onKill(IConnectIA *A);

			/// \name Some IBasicInterface method.
			//@{
			virtual void save(NLMISC::IStream &os);
			virtual void load(NLMISC::IStream &is);		
			//@}

			virtual ~IAgentComposite();
	};

	/**
	Base class for minimum agent working. This class manage a child list, message & run of proccess.
	* \author Portier Pierre
	* \author Nevrax France
	* \date 2000	
	*/
	class IAgent:public IAgentComposite
	{	
	public:	
		static const NLAIC::CIdentType *IdAgent;

	private:
		std::list<IBasicAgent *>::iterator _Iter_Child;
		
	protected:		
		IAgent(const IAgent &a);
		
		/// \name Some IBasicAgent method.
		//@{
		virtual void runChildren();		/// Activates the child agents
		virtual bool runChildrenStepByStep();/// Activates the child agents
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
		virtual void getDebugString(std::string &t) const;
		virtual const NLAIC::CIdentType &getType() const;
		virtual void save(NLMISC::IStream &os);
		virtual void load(NLMISC::IStream &is);		
		//@}

		///Implementation of the isEqual of the IBasicObjectIA 
		virtual bool isEqual(const IBasicObjectIA &a) const;

		/// Call this function to destroy an agent including its referneces in other agents and its childs
		virtual void Kill();

		/// Called by an agent who's destroyed te remove its references
		virtual void onKill(IConnectIA *A);

		virtual bool haveActivity() const
		{
			return false;
		}

		///Mailer do'nt have own activity.
		virtual IObjectIA::CProcessResult runActivity()
		{
			return ProcessRun;
		}		
		/// \name Some IBasicAgent method.
		//@{
		virtual void processMessages();
		virtual const CProcessResult &run();		
		virtual const CProcessResult &runStep();
		//@}
	};
}
#endif
