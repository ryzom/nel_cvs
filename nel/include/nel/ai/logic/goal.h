/** \file file.cpp
 *	First order logic operators with forward and backward chaining
 *
 * $Id: goal.h,v 1.9 2001/04/17 09:26:09 portier Exp $
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

namespace NLAILOGIC 
{
	class IBaseOperator;

	class CGoal : public IBaseBoolType
	{
		private:
			NLAIAGENT::IVarName *_Name;
			std::vector<NLAIAGENT::IObjectIA *>	_Args;
			std::vector<NLAIAGENT::IBasicAgent *>	_Predecessors;
			std::vector<NLAIAGENT::IBasicAgent *>	_Successors;

			NLAIAGENT::IBasicAgent *_Sender;
			NLAIAGENT::IBasicAgent *_Receiver;


		public:

			CGoal();
			CGoal(const NLAIAGENT::IVarName &);
			CGoal(const NLAIAGENT::IVarName &, std::list<const NLAIAGENT::IObjectIA *> &);
			CGoal(const CGoal &);
			virtual ~CGoal();

			void setVars(std::list<IBaseVar *> &);

			static const NLAIC::CIdentType IdGoal;

			void addSuccessor(NLAIAGENT::IBasicAgent *);
			void addPredecessor(NLAIAGENT::IBasicAgent *);

			virtual void failure();
			virtual void success();

			virtual void operatorSucces(NLAIAGENT::IBasicAgent *);
			virtual void operatorFailure(NLAIAGENT::IBasicAgent *);

			const std::vector<IBaseOperator *> getOperators();

			const NLAIC::IBasicType *clone() const;
			const NLAIC::IBasicType *newInstance() const;
			void save(NLMISC::IStream &os);
			void load(NLMISC::IStream &is);
			void getDebugString(char *text) const;
			bool isTrue() const;
			float truthValue() const;
			const IObjectIA::CProcessResult &run();
			bool isEqual(const CGoal &a) const;
			bool isEqual(const NLAIAGENT::IBasicObjectIA &a) const;
			std::vector<IBaseVar *> *getVars();
			const NLAIC::CIdentType &getType() const;

			virtual NLAIAGENT::tQueue isMember(const NLAIAGENT::IVarName *,const NLAIAGENT::IVarName *,const NLAIAGENT::IObjectIA &) const;
			virtual	NLAIAGENT::IObjectIA::CProcessResult runMethodeMember(sint32, sint32, NLAIAGENT::IObjectIA *);
			virtual	NLAIAGENT::IObjectIA::CProcessResult runMethodeMember(sint32 index, NLAIAGENT::IObjectIA *p);
			sint32 getMethodIndexSize() const;

			const NLAIAGENT::IVarName &getName() const
			{
				return *_Name;
			}

			virtual bool operator==(const CGoal &);

			void setSender(NLAIAGENT::IBasicAgent *);
			void setReceiver(NLAIAGENT::IBasicAgent *);

			NLAIAGENT::IBasicAgent *getSender();
			NLAIAGENT::IBasicAgent *getReceiver();

			virtual void cancel();
	};
}

#endif
