/** \file actor_script.h
 *	
 *	Scripted actors	
 *
 * $Id: actor_script.h,v 1.2 2001/01/08 10:47:05 chafik Exp $
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

#include "agent/agent.h"
#include "agent/agent_script.h"
#include "logic/bool_cond.h"
#include "script/type_def.h"
#include "script/interpret_actor.h"

namespace NLAIAGENT
{

	class CComponentHandle : public IObjectIA {
		private:
			CStringVarName	*_CompName;
			const IObjectIA	*_Comp;
			CAgentScript	*_CompFather;

		public:
			CComponentHandle()
			{
				_Comp = NULL;
				_CompFather = NULL;
			}

			~CComponentHandle()
			{
//				if ( _Comp )
//					_Comp->release();
			}


			CComponentHandle(CStringVarName &comp_name, CAgentScript *comp_father, bool get = false)
			{
				_CompName = (CStringVarName *) comp_name.clone();
				_CompFather = comp_father;
				if ( get )
					getComponent();
			}

			void getComponent()
			{
				// Looks in static components
				sint32 comp_id = _CompFather->getStaticMemberIndex( *_CompName );
				if ( comp_id >= 0)
					_Comp = _CompFather->getStaticMember( comp_id );
				else
					_Comp = NULL;

				// Looks in dynamic component
				CGroupType *param = new CGroupType();
				param->push( (IObjectIA *) _CompName );
				IObjectIA::CProcessResult comp = _CompFather->getDynamicAgent(param);
				param->pop();
				delete param;
				if ( comp.Result )
					_Comp = comp.Result;
			}

			const IObjectIA *getValue()
			{
				if ( _Comp )
					return _Comp;
				
				getComponent();
				return _Comp;
			}
	};


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
				fid_unActivate,
				fid_forwardActivity
			};

			bool _IsActivated;
			std::vector<CComponentHandle *> _Actors;
			
			/** Transfers activity to another actor.
				The second arg bool must be set to true for this agent to stay active, false otherwise.
			**/
			virtual void forwardActivity(CActorScript *, bool stay_active = false);
			/** Transfers activity to another actor.
				The second arg bool must be set to true for this agent to stay active, false otherwise.
			**/
			virtual void forwardActivity(std::vector<CActorScript *> &, bool stay_active = false);

		public:
			// Builds and actor with its father
			CActorScript(IAgentManager *, bool activated = false);
			// copy constructor
			CActorScript(const CActorScript &);
			CActorScript(IAgentManager *, IBasicAgent *, std::list<IObjectIA *> &, NLIASCRIPT::CActorClass *);
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
			


			/// Inherited functions
			virtual const NLAIC::IBasicType *clone() const;
			virtual const NLAIC::IBasicType *newInstance() const;
			virtual void getDebugString(char *t) const;
			virtual bool isEqual(const IBasicObjectIA &a) const;
			virtual void processMessages();
			virtual const CProcessResult &run();
			virtual IObjectIA *run(const IMessageBase &msg);
			virtual	CProcessResult sendMessage(IObjectIA *);
			virtual const NLAIC::CIdentType &getType() const;

			static const NLAIC::CIdentType IdActorScript;

			virtual void save(NLMISC::IStream &os);		
			virtual void load(NLMISC::IStream &is);		

			virtual tQueue isMember(const NLAIAGENT::IVarName *, const NLAIAGENT::IVarName *, const IObjectIA &) const;
			virtual IObjectIA::CProcessResult runMethodeMember(sint32, IObjectIA *);
			virtual sint32 getMethodIndexSize() const;

			void getFatherComponent(IVarName &);
	};
}
#endif