/** \file actor.h
 *	
 *
 * $Id: actor.h,v 1.7 2003/01/21 11:24:24 chafik Exp $
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

#ifndef NL_ACTOR_H_
#define NL_ACTOR_H_

#include "nel/ai/agent/agent.h"
#include "nel/ai/agent/agent_script.h"
#include "nel/ai/logic/bool_cond.h"
#include "nel/ai/script/type_def.h"

namespace NLAIAGENT
{
	class CActor;

	/**
	Class Actor
	You can think of activation as a set of tokens passed between actors. The number of tokens isn't fixed, which means
	one transition can activate more than one actor.
	An actor has a list of conditions under which he's going to pass activation to other actors.
	A simple state machine is a particular case in which there's always only one token for all actors.
	If a run() is defined, the actor becomes an active agent with an activity at each cycle.
	**/
	class CActor	 : public IAgent
	{
		protected:

			enum c_funcs_id 
			{
				fid_activate,
				fid_unActivate,
				fid_forwardActivity
			};

			bool _IsActivated;
//			std::vector<CTransition *> _Transitions;
			
			/** Transfers activity to another actor.
				The second arg bool must be set to true for this agent to stay active, false otherwise.
			**/
			virtual void forwardActivity(CActor *, bool stay_active = false);
			/** Transfers activity to another actor.
				The second arg bool must be set to true for this agent to stay active, false otherwise.
			**/
			virtual void forwardActivity(std::vector<CActor *> &, bool stay_active = false);


		public:
			/// Default constructor
			CActor();
			// Builds and actor with its father
			CActor(IAgent *, bool activated = false);
			// copy constructor
			CActor(const CActor &);
			~CActor();

			/// Returns true if the actor has a token
			bool isActivated();
			/// Adds a transition, ie a condition and a list of newly activated agents
			void addTransition(NLAILOGIC::IBaseCond *, std::vector<CActor *> &, bool);
//			void addTransition(CTransition *);

			/// Activates the actor.
			void activate();
			/// Callback called at the activation of the actor
			virtual void onActivate();

			/// Unactivates the actor.
			void unActivate();
			/// Callback called when the agent is unactivated
			virtual void onUnActivate();

			/// \name Base class member method.			
			//@{
			virtual const NLAIC::IBasicType *clone() const;
			virtual const NLAIC::IBasicType *newInstance() const;
			virtual void getDebugString(std::string &t) const;
			virtual bool isEqual(const IBasicObjectIA &a) const;
			virtual void processMessages();
			virtual const CProcessResult &run();			
			virtual const NLAIC::CIdentType &getType() const;

			static const NLAIC::CIdentType IdActor;
					
			virtual void save(NLMISC::IStream &os);		
			virtual void load(NLMISC::IStream &is);		

			virtual TQueue isMember(const NLAIAGENT::IVarName *, const NLAIAGENT::IVarName *, const IObjectIA &) const;
			virtual IObjectIA::CProcessResult runMethodeMember(sint32, IObjectIA *);
			virtual sint32 getMethodIndexSize() const;
			//@}
	};
}
#endif
