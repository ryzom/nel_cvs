/** \file actor_script.h
 *	
 *	Scripted actors	
 *
 * $Id: actor_script.h,v 1.11 2001/02/28 09:42:02 portier Exp $
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

#ifndef NL_ACTOR_SCRIPT_H_
#define NL_ACTOR_SCRIPT_H_

#include "nel/ai/agent/agent.h"
#include "nel/ai/agent/agent_script.h"
#include "nel/ai/agent/agent_manager.h"
#include "nel/ai/logic/bool_cond.h"
#include "nel/ai/script/type_def.h"
#include "nel/ai/script/interpret_actor.h"
#include "nel/ai/agent/comp_handle.h"

namespace NLAIAGENT
{
	/**
	Class CActorScript
	Scripted actors for the creations of FSMs and other controlers.
	You can think of activation as a set of tokens passed between actors. The number of tokens isn't fixed, which means
	one transition can activate more than one actor.
	An actor has a list of conditions under which he's going to pass activation to other actors.
	A simple state machine is a particular case in which there's always only one token for all actors.
	If a run() is defined, the actor becomes an active agent with an activity at each cycle.
	**/
	class CActorScript  : public CAgentScript
	{
		protected:

			enum c_funcs_id 
			{
				fid_activate,
				fid_onActivate,
				fid_unActivate,
				fid_onUnActivate,
				fid_switch,
				fid_last
			};

			bool _IsActivated;
			sint32	_OnActivateIndex;
			sint32	_OnUnActivateIndex;
			std::vector<CComponentHandle *> _Actors;
			
			/** Transfers activity to another actor.
				The second arg bool must be set to true for this agent to stay active, false otherwise.
			**/
			virtual void switchActor(CActorScript *, bool stay_active = false);
			/** Transfers activity to another actor.
				The second arg bool must be set to true for this agent to stay active, false otherwise.
			**/
			virtual void switchActor(std::vector<CActorScript *> &, bool stay_active = false);
			/** Transfers activity to another actor.
				The second arg bool must be set to true for this agent to stay active, false otherwise.
			**/
			virtual void switchActor(std::vector<CComponentHandle *> &, bool stay_active = false);


		public:
			// Builds and actor with its father
			CActorScript(IAgentManager *, bool activated = false);
			// copy constructor
			CActorScript(const CActorScript &);
			CActorScript(IAgentManager *, IBasicAgent *, std::list<IObjectIA *> &, NLAISCRIPT::CActorClass *);
			virtual ~CActorScript();

			/// Returns true if the actor has a token
			bool isActivated();
			/// Adds a transition, ie a condition and a list of newly activated agents

			/// Activates the actor.
			void activate();
			/// Callback called at the activation of the actor
			virtual void onActivate();

			/// Unactivates the actor.
			void unActivate();
			/// Callback called when the agent is unactivated
			virtual void onUnActivate();
			
			virtual int getBaseMethodCount() const;

			/// Inherited functions
			virtual const NLAIC::IBasicType *clone() const;
			virtual const NLAIC::IBasicType *newInstance() const;
			virtual void getDebugString(char *t) const;
			virtual bool isEqual(const IBasicObjectIA &a) const;
			virtual void processMessages();
			virtual const CProcessResult &run();

//			virtual IObjectIA *run(const IMessageBase &msg);
			virtual	CProcessResult sendMessage(IObjectIA *);
			virtual const NLAIC::CIdentType &getType() const;

			static const NLAIC::CIdentType IdActorScript;

			virtual void save(NLMISC::IStream &os);		
			virtual void load(NLMISC::IStream &is);		

			virtual IObjectIA::CProcessResult runMethodBase(int heritance, int index,IObjectIA *);
			virtual IObjectIA::CProcessResult runMethodBase(int index,IObjectIA *);

			virtual tQueue isMember(const NLAIAGENT::IVarName *, const NLAIAGENT::IVarName *, const IObjectIA &) const;
//			virtual IObjectIA::CProcessResult runMethodeMember(sint32, sint32, IObjectIA *);
//			virtual IObjectIA::CProcessResult runMethodeMember(sint32, IObjectIA *);
			virtual sint32 getMethodIndexSize() const;

			void getFatherComponent(IVarName &);
	};
}
#endif
