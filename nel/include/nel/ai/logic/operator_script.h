/** \file operator_script.h
 *	
 *	Instances of operators
 *
 * $Id: operator_script.h,v 1.30 2002/09/16 13:49:39 portier Exp $
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

#ifndef NL_OPERATOR_SCRIPT_H_
#define NL_OPERATOR_SCRIPT_H_

#include "nel/ai/agent/agent.h"
#include "nel/ai/agent/agent_script.h"
#include "nel/ai/agent/agent_manager.h"
#include "nel/ai/logic/bool_cond.h"
#include "nel/ai/script/type_def.h"
#include "nel/ai/logic/interpret_object_operator.h"
#include "nel/ai/agent/actor_script.h"

/*
namespace NLAILOGIC {
	class CValueSet;
}
*/

namespace NLAIAGENT
{
	class COperatorScript  : public CActorScript
	{
		protected:
			enum c_funcs_id 
			{
				fid_modeachieve,
				fid_modemaintain,
				fid_isPaused,
				fid_getPriority,
				fid_setPriority,
				fid_isActivable,
				fid_background,
				fid_exclusive,
				fid_achieve,
				fid_order,
				fid_last
			};

		private:
			std::vector<NLAIAGENT::IObjectIA *> _VarValues;				// Values of the vars for the instanciated operator
			NLAILOGIC::CFactBase				*_FactBase;				// The father's factbase
			NLAILOGIC::CGoal					*_CurrentGoal;
			sint32								_CyclesBeforeUpdate;	// Number of cycles before the preconditions are checked
			std::list<NLAILOGIC::CGoal *>		_ActivatedGoals;
			bool								_IsActivable;
			bool								_Exclusive;
			bool								_Maintain;
			float								_Priority;
			float								_BasePriority;
		public:
			// Builds and actor with its father
			COperatorScript(IAgentManager *, bool activated = false);
			// copy constructor
			COperatorScript(const COperatorScript &);
			COperatorScript(IAgentManager *, IBasicAgent *, std::list<IObjectIA *> &, NLAISCRIPT::COperatorClass *);
			virtual ~COperatorScript();

			/// Priority of the operator
			float priority() const;
			void calcPriority();

//			virtual int getBaseMethodCount() const;

			/// Inherited functions
			virtual const NLAIC::IBasicType *clone() const;
			virtual const NLAIC::IBasicType *newInstance() const;
			virtual void getDebugString(std::string &) const;
			virtual bool isEqual(const IBasicObjectIA &a) const;
//			virtual void processMessages();
			virtual bool checkActivation();
			virtual const CProcessResult &run();
		 	virtual void processMessages(NLAIAGENT::IMessageBase *msg,NLAIAGENT::IObjectIA *o);

//			virtual IObjectIA *run(const IMessageBase &msg);
//			virtual	CProcessResult sendMessage(IObjectIA *);
			virtual const NLAIC::CIdentType &getType() const;

			static const NLAIC::CIdentType IdOperatorScript;

			virtual void save(NLMISC::IStream &os);		
			virtual void load(NLMISC::IStream &is);		

			virtual IObjectIA::CProcessResult runMethodBase(int heritance, int index,IObjectIA *);
			virtual IObjectIA::CProcessResult runMethodBase(int index,IObjectIA *);

//			virtual tQueue isMember(const NLAIAGENT::IVarName *, const NLAIAGENT::IVarName *, const IObjectIA &) const;

			tQueue getPrivateMember(const IVarName *,const IVarName *,const IObjectIA &) const;
			virtual sint32 getMethodIndexSize() const;
			virtual int getBaseMethodCount() const;

			virtual void onKill(IConnectIA *a);

			void getFatherComponent(IVarName &);

			/// Binds the goal args to the variables of the operator (defined by the "Goal:" field in the script).
			void linkGoalArgs(NLAILOGIC::CGoal *);
	
			/// Selects a goal among the ones the operator can process
			virtual NLAILOGIC::CGoal *selectGoal();

			virtual void cancel();
//			virtual bool isActive();

			virtual void onSuccess( IObjectIA *);
			virtual void onFailure( IObjectIA *);

			// Tries to unify the facts
			NLAILOGIC::CFact *buildFromVars(NLAILOGIC::IBaseAssert *, std::vector<sint32> &, NLAILOGIC::CValueSet *);
			std::list<NLAILOGIC::CValueSet *> *propagate(std::list<NLAILOGIC::CValueSet *> &, NLAILOGIC::CValueSet *, std::vector<sint32> &);
			std::list<NLAILOGIC::CFact *> *propagate(std::list<NLAILOGIC::CFact *> &);
			NLAILOGIC::CValueSet *unifyLiaison( const NLAILOGIC::CValueSet *, NLAILOGIC::CValueSet *, std::vector<sint32> &);

			void lookForGoals();
			bool checkPreconditions();
			void execOnActivate();
			void checkPause();
			bool isExclusive();
			void setPriority(float prio);

			virtual bool checkTrigMsg();

			virtual void order(NLAIAGENT::IBaseGroupType *, NLAILOGIC::IGoal *);
	};
}
#endif
