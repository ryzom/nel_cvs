/** \file agent_script.h
 * class for agent script.
 *
 * $Id: agent_script.h,v 1.1 2001/01/05 10:50:22 chafik Exp $
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

#ifndef NL_AGENTSCRIPT_H
#define NL_AGENTSCRIPT_H

#include "agent/agent.h"
#include "agent/agent_manager.h"
#include "logic/operator.h"

namespace NLIASCRIPT 
{
	class CAgentClass;
	class IOpCode;
}

namespace NLIAAGENT
{
	class IAgentManager;
	
	/**
		Class for managing script agent.

		* \author Chafik sameh
		* \author Nevrax France
		* \date 2000
	*/
	class CAgentScript : public IAgentManager
	{
	public:
		/// \name Static definition for internal method. 
		//@{
		///Offset for the send member method.
		static const int sendTag;
		///Offset for the getChild member method.
		static const int getChildTag;
		///Offset for the addChild member method.
		static const int addChildTag;
		///this int represent the las allocation offset method.
		static const int baseMethodIn;
		//@}

	private:
		///Type def for the map witch store the name of dynamic agent store in the agent.
		typedef std::map<CStringType, std::list<IBasicAgent *>::iterator> tmapDefNameAgent;
		///The pair return by the map.
		typedef std::pair<CStringType, std::list<IBasicAgent *>::iterator> tPairName;
	private:			
		///Table of static compoments.
		IObjectIA **_Components;
		///Nomber of static compoment.
		int _NbComponents;
		///Vector for store agent operator.
		std::vector<NLIALOGIC::IBaseOperator *> _Operators;
		///The manager where the agent is run this manager have the agent script context.
		IAgentManager *_AgentManager;
		/**
		This is a mail box for script send message. Note that if message have a run message for prossing message, this message is achieve in this mail box, 
		else he is achive in the base class mail box.
		*/
		IMailBox	*_ScriptMail;
		///Map for store agent added in the dynamic child container.
		tmapDefNameAgent _DynamicAgentName;

	protected:
		///The creator of this instance. This useful for find scripted method entry point.
		NLIASCRIPT::CAgentClass *_AgentClass;

	public:
		static const NLIAC::CIdentType IdAgentScript;
				

	public:
		///Construct with copy constructor.
		CAgentScript(const CAgentScript &);
		///Build with an knowning manager.
		CAgentScript(IAgentManager *);
		///Build with an knowning manager and a list of static compoment.
		CAgentScript(IAgentManager *, IBasicAgent *, std::list<IObjectIA *> &, NLIASCRIPT::CAgentClass *);

		virtual ~CAgentScript();
		
		/// \name CAgentScript member methods. 
		//@{
		///Method for adding operator.
		virtual void addOperator(NLIALOGIC::IBaseOperator *);
		///Removing operator from th agent.
		virtual void remOperator(NLIALOGIC::IBaseOperator *p);
		/**
		Add an agent to the dynamic agent child.
		Method have an IBaseGroupType argument, this argument must store an CStringType first and an IObjectIA pointer memory next.
		The CStringType is the name of the agent. The IObjectIA pointer memory is the agent.
		*/
		virtual IObjectIA::CProcessResult addDynamicAgent(NLIAAGENT::IBaseGroupType *g);
		/**
		Get agent from the dynamic agent child.
		Method have an IBaseGroupType argument where he must store an CStringType representative of the name of the agent.
		*/
		virtual IObjectIA::CProcessResult getDynamicAgent(NLIAAGENT::IBaseGroupType *g);
		///get the closure correspondent of the method indexed by index in the base class inheritance.
		NLIASCRIPT::IOpCode &getMethode(sint32 inheritance,sint32 index); 
		///get the closure correspondent of the method indexed by index.
		NLIASCRIPT::IOpCode &getMethode(sint32 index);
		///Get the manager of the instance.
		const IAgentManager *getAgentManager() const
		{
			return _AgentManager;
		}

		///Get the creator of this instance.
		const NLIASCRIPT::CAgentClass *getFactoryClass() const
		{
			return _AgentClass;
		}
		///get the mail boxe for scripted message.
		const IMailBox *getLocalMailBox() const
		{
			return _ScriptMail;
		}		

		///Set the manager for this instance.
		void setAgentManager(IAgentManager *manager);		
		//@}

		/// \name IAgentManager member method. 
		//@{
		virtual int getBaseMethodCount() const;
		///Get an valid context for this instance.
		virtual const IObjectIA *getAgentContext() const
		{
			return getAgentManager()->getAgentContext();
		}
		//@}		

		/// \name Base class member method. 
		//@{

		virtual void runChildren();				
		virtual void onKill(IConnectIA *A);		
		virtual void processMessages();		

		virtual bool isEqual(const IBasicObjectIA &a) const;

		IObjectIA::CProcessResult sendMethod(IObjectIA *);
		virtual IObjectIA::CProcessResult runMethodBase(int heritance, int index,IObjectIA *);
		virtual IObjectIA::CProcessResult runMethodBase(int index,IObjectIA *);		

		virtual sint32 getStaticMemberSize() const;
		virtual sint32 getStaticMemberIndex(const IVarName &) const;
		virtual const IObjectIA *getStaticMember(sint32) const;		
		virtual void setStaticMember(sint32,IObjectIA *);

		virtual tQueue isMember(const IVarName *,const NLIAAGENT::IVarName *,const IObjectIA &) const;
		virtual sint32 isClassInheritedFrom(const NLIAAGENT::IVarName &) const;			

		virtual sint32 getMethodIndexSize() const;		
		virtual	IObjectIA::CProcessResult runMethodeMember(sint32 heritance, sint32 index,IObjectIA *);
		virtual	IObjectIA::CProcessResult runMethodeMember(sint32 index,IObjectIA *);		

		virtual	CProcessResult sendMessage(IObjectIA *);

		virtual const IObjectIA::CProcessResult &run();
		//@}

		/// \name NLIAC::IBasicInterface base class method.
		//@{
		virtual void load(NLMISC::IStream &is);
		virtual void save(NLMISC::IStream &os);
		virtual const NLIAC::IBasicType *clone() const;
		virtual const NLIAC::IBasicType *newInstance() const;
		virtual void getDebugString(char *t) const;
		virtual const NLIAC::CIdentType &getType() const;
		//@}
	};
}
#endif
