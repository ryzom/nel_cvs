/** \file fuzzyrule.h
 *	Fuzzy rules class
 *
 * $Id: fuzzyrule.h,v 1.3 2001/01/08 14:39:59 valignat Exp $
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

 * You should have received a copy of the GNU NLIACeral Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef NL_FUZZY_RULE_H
#define NL_FUZZY_RULE_H

#include "nel/ai/fuzzy/fuzzycond.h"
#include "nel/ai/logic/bool_cond.h"

namespace NLAIFUZZY
{
	/**
	* This class used for fuzzy inferences
	*/

	class CFuzzyRule : public NLAILOGIC::CondAnd {
		private:
			std::vector<CFuzzyVar *>	_Vars;
			std::vector<IFuzzySet *>	_Sets;
			char *_Comment;
			float _Threshold;
		public:
			CFuzzyRule(char *comment = NULL);
			CFuzzyRule(const CFuzzyRule &);
			CFuzzyRule(const NLAILOGIC::CondAnd &);
			CFuzzyRule(std::list<NLAILOGIC::IBaseBoolType *> &conds, char *comment, float threshold = 0);
			~CFuzzyRule();
			void addConc(CFuzzyVar *, IFuzzySet *);
			void addCond(CFuzzyVar *, char *);
			void addConc(CFuzzyVar *, char *);

			virtual void save(NLMISC::IStream &os);
			virtual void load(NLMISC::IStream &is);

			virtual void getDebugString(char *) const;

			static const NLAIC::CIdentType IdFuzzyRule;

			const NLAIC::CIdentType &getType() const;

			virtual const NLAIC::IBasicType *clone() const;
			virtual const NLAIC::IBasicType *newInstance() const;
			
			virtual const NLAIAGENT::IObjectIA::CProcessResult &run();
			bool isEqual(const NLAIAGENT::IBasicObjectIA &a) const;
			virtual bool isTrue() const;
			virtual NLAIAGENT::IObjetOp *operator == (NLAIAGENT::IObjetOp &a) const;
			virtual void init(NLAIAGENT::IObjectIA *);
	};
}

#endif
