/** \file actor_script.h
 *	
 *	Instances of operators
 *
 * $Id: operator_script.h,v 1.13 2001/05/11 14:11:44 portier Exp $
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

namespace NLAIAGENT
{
	class COperatorScript  : public CActorScript
	{
		private:
			std::vector<NLAIAGENT::IObjectIA *> _VarValues;		// Values of the vars for the instanciated operator
			NLAILOGIC::CFactBase				*_FactBase;		// The father's factbase
			NLAILOGIC::CGoal					*_CurrentGoal;
		public:
			// Builds and actor with its father
			COperatorScript(IAgentManager *, bool activated = false);
			// copy constructor
			COperatorScript(const COperatorScript &);
			COperatorScript(IAgentManager *, IBasicAgent *, std::list<IObjectIA *> &, NLAISCRIPT::COperatorClass *);
			virtual ~COperatorScript();

			/// Priority of the operator
			virtual float priority() const;

//			virtual int getBaseMethodCount() const;

			/// Inherited functions
			virtual const NLAIC::IBasicType *clone() const;
			virtual const NLAIC::IBasicType *newInstance() const;
			virtual void getDebugString(char *t) const;
			virtual bool isEqual(const IBasicObjectIA &a) const;
//			virtual void processMessages();
			virtual const CProcessResult &run();
 
//			virtual IObjectIA *run(const IMessageBase &msg);
//			virtual	CProcessResult sendMessage(IObjectIA *);
			virtual const NLAIC::CIdentType &getType() const;

			static const NLAIC::CIdentType IdOperatorScript;

			virtual void save(NLMISC::IStream &os);		
			virtual void load(NLMISC::IStream &is);		

//			virtual IObjectIA::CProcessResult runMethodBase(int heritance, int index,IObjectIA *);
//			virtual IObjectIA::CProcessResult runMethodBase(int index,IObjectIA *);

//			virtual tQueue isMember(const NLAIAGENT::IVarName *, const NLAIAGENT::IVarName *, const IObjectIA &) const;
//			virtual sint32 getMethodIndexSize() const;

			void getFatherComponent(IVarName &);

//			virtual void setParent(const IWordNumRef *parent);

			/// Binds the goal args to the variables of the operator (defined by the "Goal:" field in the script).
			void instanciateGoalArgs(NLAILOGIC::CGoal *);

			virtual void cancel();
//			virtual bool isActive();
	};
}
#endif
