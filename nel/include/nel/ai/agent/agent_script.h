/** \file agent_script.h
 * class for agent script.
 *
 * $Id: agent_script.h,v 1.53 2002/05/17 13:46:24 chafik Exp $
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

#include "nel/ai/agent/agent.h"
#include "nel/ai/agent/agent_manager.h"
#include "nel/ai/logic/operator.h"
#include "nel/ai/agent/msg_getvalue.h"

////////////////////////////////////////
// Temp to be transfered in CGDAgentScript
/*
#include "nel/ai/logic/factbase.h"
#include "nel/ai/logic/goal.h"

  */
////////////////////////////////////////


namespace NLAISCRIPT 
{
	class CAgentClass;
	class IOpCode;
	class COperandSimpleListOr;
	class COperandSimple;
	class CParam;
}

namespace NLAIAGENT
{
	class IAgentManager;
	struct CKeyAgent;
	/**
		Class for managing script agent.

		* \author Chafik sameh
		* \author Nevrax France
		* \date 2000
	*/
	class CAgentScript : public IAgentManager
	{
	public:		

		///This enum define ident for hard coded method that we have to import its under the script.
		enum  TMethodNumDef {			
			TSend, ///Index of the send method
			TSendComponent, ///Index of the send with continuation method.
			TGetChildTag, ///Index of the getChild method
			TAddChildTag,///Index of the addChild method
			TRemoveChild,///Index of the removeDynamic method
			TFather, ///Get the father of the agent.
			TSelf, ///Get The self pointer.
			TGetName, ///Get the name of the agent in the manager child list.
			TIsInherited,
			TGetClassName, /// Returns the name of the class in the registry
			TRunTellComponent, ///transmit Message to an compoment.
			TRunAskComponent, ///transmit Message to an compoment.
			TRunAskParentNotify, ///the offest of the runAskParentNotify method.
			TRunTellParentNotify, ///the offest of the runTellParentNotify method.			
			TSetStatic,	/// Method to assign a new value to a static component
			TGetValue,	/// Processes the "Ask(GetValueMsg)" msg to obtain a public value of the agent
			TSetValue,	/// Processes the "Tell(SetValueMsg)" msg to connect to the value of a distant object.
//			TGetValueReturn,	/// Processes the "Tell(GetValueMsg)" msg resulting from a "Ask(GetValueMsg)" message sent.
			TDeflautProccessMsg,
			TInitComponent,
			TIsEUU,
			TAddSet,
			TLastM ///The count of export method.
		};

		enum TTypeCheck{
			CheckAll,
			CheckCount,
			DoNotCheck
		};

		///Structure to define the name, id and argument type of hard coded mathod.
		struct CMethodCall
		{			

			CMethodCall(const char *name, int i,IObjectIA *a,TTypeCheck checkArg,int argCount,IObjectIA *r): 
		 			MethodName (name),ArgType(a),ReturnValue(r)
			{
				Index = i;
				CheckArgType = checkArg;
				ArgCount = argCount;
			}

			virtual ~CMethodCall()
			{
				if(ReturnValue) ReturnValue->release();
				if(ArgType != NULL) ArgType->release();

			}
			///Name of the method.
			CStringVarName MethodName;
			///Type of the method argument.
			IObjectIA *ArgType;
			///Return value type.
			IObjectIA *ReturnValue;
			///CheckArg is for force the method argument test. If its true we test juste the name coherence.
			TTypeCheck CheckArgType;
			///Count neaded when the CheckCount it set.
			sint ArgCount;
			///Index of the method in the class.
			sint32 Index;
		};

		static CMethodCall **StaticMethod;

	public:
		static NLAISCRIPT::COperandSimpleListOr *msgType;
		static NLAISCRIPT::COperandSimpleListOr *msgPerf;
		static NLAISCRIPT::CParam *SendParamMessageScript;
		static NLAISCRIPT::CParam *ParamSetStatic;
		static NLAISCRIPT::CParam *SendCompParamMessageScript;
		static NLAISCRIPT::COperandSimple *IdMsgNotifyParentClass;
		static NLAISCRIPT::COperandSimpleListOr *ParamIdGetValueMsg;
		static NLAISCRIPT::CParam *ParamGetValueMsg;
		static NLAISCRIPT::COperandSimpleListOr *ParamIdSetValueMsg;
		static NLAISCRIPT::CParam *ParamSetValueMsg;
		static NLAISCRIPT::COperandSimpleListOr *ParamIdTellComponentMsg;
		static NLAISCRIPT::CParam *ParamTellComponentMsg;
		static NLAISCRIPT::COperandSimpleListOr *ParamIdInitComponentMsg;
		static NLAISCRIPT::CParam *ParamInitComponentMsg;

		static NLAISCRIPT::COperandSimpleListOr *IdMsgNotifyParent;
		static NLAISCRIPT::CParam *ParamRunParentNotify;
	private:
		///Type def for the map witch store the name of dynamic agent store in the agent.
		typedef std::multiset<CKeyAgent> tsetDefNameAgent;
		
	protected:			
		///Table of static compoments.
		IObjectIA **_Components;
		///Nomber of static compoment.
		int _NbComponents;
		///Vector for store agent operator.
//		std::vector<NLAILOGIC::IBaseOperator *> _Operators;
		///The manager where the agent is run this manager have the agent script context.
		IAgentManager *_AgentManager;		
		/**
		This is a mail box for script send message. Note that if message have a run message for prossing message, this message is achieve in this mail box, 
		else he is achive in the base class mail box.
		*/
		//IMailBox	*_ScriptMail;
		///Map for store agent added in the dynamic child container.
		tsetDefNameAgent _DynamicAgentName;


	protected:
		///The creator of this instance. This useful for find scripted method entry point.
		NLAISCRIPT::CAgentClass *_AgentClass;
		sint _iComponents;
		
		std::set<std::string> mapSet;

	public:
		static const NLAIC::CIdentType IdAgentScript;

	public:
		///Construct with copy constructor.
		CAgentScript(const CAgentScript &);
		///Build with an knowning manager.
		CAgentScript(IAgentManager *);
		///Build with an knowning manager and a list of static compoment.
		CAgentScript(IAgentManager *, IBasicAgent *, std::list<IObjectIA *> &, NLAISCRIPT::CAgentClass *);

		virtual ~CAgentScript();
		
		/// \name CAgentScript member methods. 
		//@{
		///Method for adding operator.
		virtual void addOperator(NLAILOGIC::IBaseOperator *);
		///Removing operator from th agent.
		virtual void remOperator(NLAILOGIC::IBaseOperator *p);
		/**
		Add an agent to the dynamic agent child.
		Method have an IBaseGroupType argument, this argument must store an CStringType first and an IObjectIA pointer memory next.
		The CStringType is the name of the agent. The IObjectIA pointer memory is the agent.
		*/
		virtual IObjectIA::CProcessResult addDynamicAgent(NLAIAGENT::IBaseGroupType *g);
		/**
		Get agent from the dynamic agent child.
		Method have an IBaseGroupType argument where he must store an CStringType representative of the name of the agent.
		*/
		virtual IObjectIA::CProcessResult getDynamicAgent(NLAIAGENT::IBaseGroupType *g);

		/**		
		Get the instance name of the agent in its manager
		*/
		virtual IObjectIA::CProcessResult getDynamicName(NLAIAGENT::IBaseGroupType *g);

		/**		
		remove an agent from the manager. The rgument g must store an CStringType. This string is the name of the agent to remove.
		The methode return the state of the operation, an digital at 1.0 if the remove is done an digital at 0.0 is the method did'nt find the agent.
		*/
		virtual IObjectIA::CProcessResult removeDynamic(NLAIAGENT::IBaseGroupType *g);

		/**		
		This function process the message CNotifyParentScript for the runAsk.
		*/
		virtual IObjectIA::CProcessResult runAskParentNotify(IBaseGroupType *);
		
		/**
		This function process the message MsgTellCompoment the result is to transmit a Message to an compoment.
		*/
		virtual IObjectIA::CProcessResult runTellComponent(IBaseGroupType *);

		IObjectIA::CProcessResult runAskGetValue(IBaseGroupType *);

		IObjectIA::CProcessResult runTellSetValue(IBaseGroupType *);

		virtual IObjectIA::CProcessResult runInitComponent(IBaseGroupType *);
		virtual IObjectIA::CProcessResult runInitClass(IBaseGroupType *);
		
		/**
		This function process the message CNotifyParentScript for the runTell.
		*/
		virtual IObjectIA::CProcessResult runTellParentNotify(IBaseGroupType *);

		///get the closure correspondent of the method indexed by index in the base class inheritance.
		NLAISCRIPT::IOpCode *getMethode(sint32 inheritance,sint32 index); 
		///get the closure correspondent of the method indexed by index.
		NLAISCRIPT::IOpCode *getMethode(sint32 index);
		///Get the manager of the instance.
		const IAgentManager *getAgentManager() const
		{
			return _AgentManager;
		}

		///Get the creator of this instance.
		const NLAISCRIPT::CAgentClass *getFactoryClass() const
		{
			return _AgentClass;
		}
		///get the mail boxe for scripted message.
		/*const IMailBox *getLocalMailBox() const
		{
			return _ScriptMail;
		}*/		
		virtual sint32 getChildMessageIndex(const IMessageBase *, sint32 );

		///Set the manager for this instance.
		virtual void setAgentManager(IAgentManager *manager);		
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
		virtual bool runChildrenStepByStep();

		virtual void onKill(IConnectIA *A);		
		virtual void processMessages();

		virtual void processMessages(IMessageBase *,IObjectIA *);
		virtual void processContinuation(IMessageBase *, IMessageBase *);

		virtual IObjectIA::CProcessResult runActivity();
		virtual bool isEqual(const IBasicObjectIA &a) const;
		virtual bool haveActivity() const;

		
		IObjectIA::CProcessResult sendMethod(IObjectIA *);
		IObjectIA::CProcessResult sendMethodCompoment(IObjectIA *);
		virtual IObjectIA::CProcessResult runMethodBase(int heritance, int index,IObjectIA *);
		virtual IObjectIA::CProcessResult runMethodBase(int index,IObjectIA *);	
		
		virtual	std::string getMethodeMemberDebugString(sint32, sint32) const;

		virtual sint32 getStaticMemberSize() const;
		virtual sint32 getStaticMemberIndex(const IVarName &) const;
		virtual const IObjectIA *getStaticMember(sint32) const;		
		virtual bool setStaticMember(sint32,IObjectIA *);

		virtual tQueue getPrivateMember(const IVarName *,const NLAIAGENT::IVarName *,const IObjectIA &) const;
		virtual tQueue isMember(const IVarName *,const NLAIAGENT::IVarName *,const IObjectIA &) const;
		virtual tQueue isDeflautProccessMsg(const IVarName *,const NLAIAGENT::IVarName *,const IObjectIA &) const;
		virtual sint32 isClassInheritedFrom(const NLAIAGENT::IVarName &) const;			

		virtual sint32 getMethodIndexSize() const;		
		virtual	IObjectIA::CProcessResult runMethodeMember(sint32 heritance, sint32 index,IObjectIA *);
		virtual	IObjectIA::CProcessResult runMethodeMember(sint32 index,IObjectIA *);		

		virtual	CProcessResult sendMessage(IObjectIA *);
		virtual	CProcessResult sendMessage(const IVarName &,IObjectIA *);
		CProcessResult sendMessageToDynmaicChild(const IVarName &,IObjectIA *);

		virtual const IObjectIA::CProcessResult &run();
		virtual const CProcessResult &runStep();
		//@}

		/// \name NLAIC::IBasicInterface base class method.
		//@{
		virtual void load(NLMISC::IStream &is);
		virtual void save(NLMISC::IStream &os);
		virtual const NLAIC::IBasicType *clone() const;
		virtual const NLAIC::IBasicType *newInstance() const;
		virtual void getDebugString(std::string &t) const;
		virtual const NLAIC::CIdentType &getType() const;
		//@}


		///Test the set of the agent.
		virtual bool isa(const std::string &) const;
		///Add a set for the agent.
		virtual void addSet(const std::string &);



	protected:
		virtual	CProcessResult sendBroadCast(IObjectIA *);


	public:
		static void initAgentScript();
		static void releaseAgentScript();


	////////////////////////////////////////////////////////////////////////
	// Temp, to be transfered in CGDAgentScript (Goal Driven Agent)
/*
	protected:
//		NLAILOGIC::CFactBase			_FactBase;
		std::vector<NLAILOGIC::CGoal *>	_GoalStack;

	public:
	*/
		virtual const NLAILOGIC::CFactBase *getFactBase()
		{
			return NULL;
		}

		virtual IObjectIA::CProcessResult runGoalMsg(IBaseGroupType *)
		{
			return IObjectIA::CProcessResult();
		}

		virtual IObjectIA::CProcessResult runCancelGoalMsg(IBaseGroupType *)
		{
			return IObjectIA::CProcessResult();
		}

		virtual IObjectIA::CProcessResult runFactMsg(IBaseGroupType *)
		{
			return IObjectIA::CProcessResult();
		}

		virtual void removeGoal( NLAILOGIC::IGoal *)
		{
		}

		virtual void setTopLevel(NLAIAGENT::CAgentScript *)
		{
		}

	////////////////////////////////////////////////////////////////////////
		const NLAISCRIPT::CAgentClass *getClass() const
		{
			return _AgentClass;
		}

		/// Returns the name of the class in the registry
		virtual const NLAIAGENT::IVarName *getClassName() const;

		virtual NLAILOGIC::CGoal *getTopGoal()
		{
			return NULL;
		}

		virtual const std::vector<NLAILOGIC::CGoal *> *getGoalStack()
		{
			return NULL;
		}


	};

	
	NLAIAGENT::tQueue isTemplateMember(	CAgentScript::CMethodCall **StaticMethod,int count,int shift,
												const NLAIAGENT::IVarName *className,
												const NLAIAGENT::IVarName *methodName,
												const NLAIAGENT::IObjectIA &param);	
}
#endif
