/** \file file.cpp
 *	First order logic operators with forward and backward chaining
 *
 * $Id: goal_stack.h,v 1.5 2003/01/21 11:24:25 chafik Exp $
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

#ifndef NL_GOALSTACK
#define NL_GOALSTACK

#include "nel/ai/agent/baseai.h"
#include "nel/ai/logic/goal.h"

namespace NLAILOGIC 
{
	class CGoalStack : public NLAIAGENT::IObjectIA
	{
		private:
			std::vector<NLAILOGIC::CGoal *>	_Goals;
			sint32							_MaxGoals;

		public:
			struct greater : public std::binary_function<CGoal *, CGoal *, bool> {
			    bool operator()(const CGoal *x, const CGoal *y) const
				{
					return x->priority() > y->priority();
				}
			};

			static const NLAIC::CIdentType IdGoalStack;
			const NLAIC::CIdentType &getType() const;

		public:
			CGoalStack();
			CGoalStack(const CGoalStack &);
			~CGoalStack();

			virtual IObjectIA::CProcessResult runActivity();
			virtual void addGoal(CGoal *);
			virtual void removeGoal(CGoal *);
			virtual void removeGoal();
			virtual CGoal *getTopGoal();

			virtual const std::vector<CGoal *> &getStack();

			virtual CGoal *operator[](sint32);
			virtual void getDebugString(std::string &) const;

			virtual const NLAIC::IBasicType *clone() const;
			virtual const NLAIC::IBasicType *newInstance() const;
			virtual void save(NLMISC::IStream &os);
			virtual void load(NLMISC::IStream &is);
			virtual bool isTrue() const;
			virtual float truthValue() const;
			virtual const IObjectIA::CProcessResult &run();
			virtual bool isEqual(const CGoal &a) const;
			virtual bool isEqual(const NLAIAGENT::IBasicObjectIA &a) const;

			void setMaxGoals(sint32);

			virtual NLAIAGENT::TQueue isMember(const NLAIAGENT::IVarName *,const NLAIAGENT::IVarName *,const NLAIAGENT::IObjectIA &) const;
			virtual	NLAIAGENT::IObjectIA::CProcessResult runMethodeMember(sint32, sint32, NLAIAGENT::IObjectIA *);
			virtual	NLAIAGENT::IObjectIA::CProcessResult runMethodeMember(sint32 index, NLAIAGENT::IObjectIA *p);
			sint32 getMethodIndexSize() const;

	};
} // NLAILOGIC

#endif // NL_GOALSTACK
