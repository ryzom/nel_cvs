/** \file fuzzyrule.h
 *	Fuzzy rules class
 *
 * $Id: fuzzyrule.h,v 1.1 2001/01/05 10:50:23 chafik Exp $
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

#include "fuzzy/fuzzycond.h"
#include "logic/bool_cond.h"

namespace NLIAFUZZY
{
	/**
	* This class used for fuzzy inferences
	*/

	class CFuzzyRule : public NLIALOGIC::CondAnd {
		private:
			std::vector<CFuzzyVar *>	_Vars;
			std::vector<IFuzzySet *>	_Sets;
			char *_Comment;
			float _Threshold;
		public:
			CFuzzyRule(char *comment = NULL);
			CFuzzyRule(const CFuzzyRule &);
			CFuzzyRule(const NLIALOGIC::CondAnd &);
			CFuzzyRule(std::list<NLIALOGIC::IBaseBoolType *> &conds, char *comment, float threshold = 0);
			~CFuzzyRule();
			void addConc(CFuzzyVar *, IFuzzySet *);
			void addCond(CFuzzyVar *, char *);
			void addConc(CFuzzyVar *, char *);

			virtual void save(NLMISC::IStream &os);
			virtual void load(NLMISC::IStream &is);

			virtual void getDebugString(char *) const;

			static const NLIAC::CIdentType IdFuzzyRule;

			const NLIAC::CIdentType &getType() const;

			virtual const NLIAC::IBasicType *clone() const;
			virtual const NLIAC::IBasicType *newInstance() const;
			
			virtual const NLIAAGENT::IObjectIA::CProcessResult &run();
			bool isEqual(const NLIAAGENT::IBasicObjectIA &a) const;
			virtual bool isTrue() const;
			virtual NLIAAGENT::IObjetOp *operator == (NLIAAGENT::IObjetOp &a) const;
			virtual void init(NLIAAGENT::IObjectIA *);
	};
}

#endif
