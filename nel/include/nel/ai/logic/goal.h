/** \file goal.h
 *	First order logic operators with forward and backward chaining
 *
 * $Id: goal.h,v 1.22 2002/04/30 15:11:17 portier Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU NLIACeral Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * NLIACeral Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef NL_GOAL_
#define NL_GOAL_

#include "nel/ai/logic/boolval.h"
#include "nel/ai/agent/agent.h"


namespace NLAIAGENT
{
	class COperatorScript;
}

namespace NLAILOGIC 
{
	class IBaseOperator;


	class IGoal : public IBaseBoolType
	{

		public:
			enum TTypeOfGoal
			{
				achieveOnce,
				achieveForever
			};


		protected:
			NLAIAGENT::IVarName *_Name;
			std::vector<NLAIAGENT::IObjectIA *>		_Args;
			std::vector<NLAIAGENT::IBasicAgent *>	_Predecessors;
			std::vector<NLAIAGENT::IBasicAgent *>	_Successors;

			TTypeOfGoal	_Mode;

			bool _Selected;

			NLAIAGENT::IBasicAgent *_Sender;
			NLAIAGENT::IBasicAgent *_Receiver;

		public:
			IGoal();
			IGoal(const NLAIAGENT::IVarName &, TTypeOfGoal mode = achieveOnce);
			IGoal(const NLAIAGENT::IVarName &, std::list<const NLAIAGENT::IObjectIA *> &,TTypeOfGoal mode = achieveOnce);
			IGoal(const IGoal &);
			virtual ~IGoal();

			const NLAIAGENT::IVarName &getName() const
			{
				return *_Name;
			}

			void setMode(TTypeOfGoal mode)
			{
				_Mode = mode;
			}

			virtual void select()
			{
				_Selected = true;
			}

			virtual void unSelect()
			{
				_Selected = false;
			}

			bool isSelected()
			{
				return _Selected;
			}

			bool isActivable()
			{
				return ( !_Successors.empty() );
			}

			virtual void failure();
			virtual void success();

			virtual void operatorSuccess(NLAIAGENT::IBasicAgent *);
			virtual void operatorFailure(NLAIAGENT::IBasicAgent *);

			bool isExclusive();

	};

	class CGoal : public IGoal
	{
		public:

			CGoal();
			CGoal(const NLAIAGENT::IVarName &, TTypeOfGoal mode = achieveOnce);
			CGoal(const NLAIAGENT::IVarName &, std::list<const NLAIAGENT::IObjectIA *> &,TTypeOfGoal mode = achieveOnce);
			CGoal(const CGoal &);
			virtual ~CGoal();

			void setArgs(std::list<NLAIAGENT::IObjectIA *> &);
			const std::vector<NLAIAGENT::IObjectIA *> &getArgs();

			static const NLAIC::CIdentType IdGoal;

			void addSuccessor(NLAIAGENT::IBasicAgent *);
			void addPredecessor(NLAIAGENT::IBasicAgent *);

//			virtual void failure();
//			virtual void success();

			virtual void operatorSuccess(NLAIAGENT::IBasicAgent *);
			virtual void operatorFailure(NLAIAGENT::IBasicAgent *);

			const std::vector<IBaseOperator *> getOperators();

			const NLAIC::IBasicType *clone() const;
			const NLAIC::IBasicType *newInstance() const;
			void save(NLMISC::IStream &os);
			void load(NLMISC::IStream &is);
			virtual void getDebugString(std::string &) const;
			bool isTrue() const;
			float truthValue() const;
			const IObjectIA::CProcessResult &run();
			bool isEqual(const CGoal &a) const;
			bool isEqual(const NLAIAGENT::IBasicObjectIA &a) const;
			const NLAIC::CIdentType &getType() const;

			virtual NLAIAGENT::tQueue isMember(const NLAIAGENT::IVarName *,const NLAIAGENT::IVarName *,const NLAIAGENT::IObjectIA &) const;
			virtual	NLAIAGENT::IObjectIA::CProcessResult runMethodeMember(sint32, sint32, NLAIAGENT::IObjectIA *);
			virtual	NLAIAGENT::IObjectIA::CProcessResult runMethodeMember(sint32 index, NLAIAGENT::IObjectIA *p);
			sint32 getMethodIndexSize() const;


			virtual bool operator==(const CGoal &);

			void setSender(NLAIAGENT::IBasicAgent *);
			void setReceiver(NLAIAGENT::IBasicAgent *);

			NLAIAGENT::IBasicAgent *getSender();
			NLAIAGENT::IBasicAgent *getReceiver();

			virtual void cancel();
			virtual float priority() const;

/*			virtual void setTopLevel(NLAIAGENT::CAgentScript *);
			const NALAIGENT::CAgentScript *getTOpLevel() const;*/

	};
}

#endif
