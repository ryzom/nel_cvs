/** \file file.cpp
 *	First order logic operators with forward and backward chaining
 *
 * $Id: goal.h,v 1.4 2001/02/28 09:43:29 portier Exp $
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
//#include "nel/ai/agent/agent.h"

namespace NLAILOGIC 
{
	class IBaseOperator;

	class CGoal : public IBaseBoolType
	{
		private:
			NLAIAGENT::IVarName *_Name;
			std::vector<NLAILOGIC::IBaseVar *>	_Vars;
			std::vector<IBaseOperator *>		_Successors;
			std::vector<IBaseOperator *>		_Predecessors;
		public:

			CGoal(const NLAIAGENT::IVarName &);
			CGoal(const CGoal &);
			~CGoal();

			void setVars(std::list<IBaseVar *> &);

			static const NLAIC::CIdentType IdGoal;

			virtual void failure();
			virtual void success();

			virtual void operatorSucces(IBaseOperator *);
			virtual void operatorFailure(IBaseOperator *);

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
	};
}

#endif
