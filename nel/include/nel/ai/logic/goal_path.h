/** \file fact.h
 *	First order logic facts
 *
 * $Id: goal_path.h,v 1.6 2003/01/21 11:24:25 chafik Exp $
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

#ifndef NL_GOAL_PATH_H
#define NL_GOAL_PATH_H

#include "nel/ai/agent/actor_script.h"
#include "nel/ai/logic/goal_stack.h"
#include "nel/ai/agent/agent_proxy_mailer.h"

// A CGoalPath is a finite state machine wher each state corresponds to a goal.

namespace NLAILOGIC
{

class CGoalPath : public NLAIAGENT::CActorScript
{
	private:
		std::vector<CGoal *>		_Goals;
		std::vector<int>			_OnSucces;
		std::vector<int>			_OnFailure;
		std::vector<bool>			_JmpNext;
		int							_CurrentState;
		NLAILOGIC::CGoalStack		*_GoalStack;
		NLAIAGENT::CProxyAgentMail	*_Father;
		

	public:
		static const NLAIC::CIdentType IdGoalPath;

	public:
		CGoalPath(NLAIAGENT::IAgentManager *);
		CGoalPath(NLAIAGENT::IAgentManager *, NLAIAGENT::IBasicAgent *, std::list<NLAIAGENT::IObjectIA *> &, NLAISCRIPT::CAgentClass *);

		void setGoalStack(NLAILOGIC::CGoalStack * );
		void addGoal(CGoal *, bool action = true, int on_succes = 0 , int on_failure = 0);

		void setFather( NLAIAGENT::CProxyAgentMail *f)
		{
			_Father = f;
		}

		virtual void onActivate();
		virtual void onUnActivate();


		/// Own success and failure functions
		/// These function tell other operators and goals that might be waiting for 
		/// the execution of this one.
		virtual void success();

		virtual void failure();


		/// These functions can be overloaded to have the actor execute something when paused or restarted
//		virtual void onPause();
//		virtual void onRestart();

		// Script hardcoded functions
		virtual int getBaseMethodCount() const;
		sint32 getMethodIndexSize() const;
		virtual NLAIAGENT::TQueue getPrivateMember(const NLAIAGENT::IVarName *,const NLAIAGENT::IVarName *,const NLAIAGENT::IObjectIA &) const;
		virtual NLAIAGENT::IObjectIA::CProcessResult runMethodBase(int heritance, int index, NLAIAGENT::IObjectIA *);
		virtual NLAIAGENT::IObjectIA::CProcessResult runMethodBase(int index, NLAIAGENT::IObjectIA *);


		virtual const NLAIC::IBasicType *clone() const;
		virtual const NLAIC::IBasicType *newInstance() const;
		virtual const NLAIC::CIdentType &getType() const;
};

} // NLAILOGIC

#endif // NL_GOAL_PATH
