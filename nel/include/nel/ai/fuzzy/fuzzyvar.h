/** \file fuzzyvar.h
 *	Dexcribes a fuzzy variable with its fuzzy sets and defuzzyfication method
 *
 * $Id: fuzzyvar.h,v 1.5 2003/01/21 11:24:25 chafik Exp $
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

#ifndef NL_FUZZY_VAR_H
#define NL_FUZZY_VAR_H

#include "nel/ai/fuzzy/fuzzyset.h"
#include "nel/ai/logic/var.h"
#include "nel/ai/fuzzy/fuzzyfact.h"

namespace NLAIFUZZY
{

	class CSimpleFuzzyCond;

	class CFuzzyVar : public NLAILOGIC::IBaseVar {
		private:
			double _Value;
			double _Min;
			double _Max;
			char *_Name;
			std::vector<IFuzzySet *> _sets;				// Sous ensembles flous
			std::vector<NLAIAGENT::CStringVarName *> _qual;		// Qualificateurs linguistiques associés	
			std::vector<CFuzzyFact *> _facts;

		public:
			// Constructors
			// Parmaters: name, val min, val max, initial value
			CFuzzyVar(const NLAIAGENT::IVarName &, double, double, NLAIAGENT::IObjetOp *);
			CFuzzyVar(const NLAIAGENT::IVarName &, double, double, double value = 0 );
			CFuzzyVar(const CFuzzyVar &);
			~CFuzzyVar();

			// Adds a fuzzy set to the var
			void addSet(IFuzzySet *, char *);
			IFuzzySet *getSet(char *);

			std::list<CFuzzyFact *> *fuzzify(double val);
			virtual double unfuzify();
			void addFact(CFuzzyFact *);
			void addFact(char *,double);
			bool isIn(IFuzzySet *);
			float membership(IFuzzySet *);

			CSimpleFuzzyCond *getCond(char *);

			virtual void setValue(NLAIAGENT::IObjetOp *obj);
			virtual void setValue(float);
			virtual NLAIAGENT::IObjetOp *getValue() const;
			virtual const NLAIC::IBasicType *clone() const;
			virtual const NLAIC::IBasicType *newInstance() const;
			virtual void save(NLMISC::IStream &os);
			virtual void load(NLMISC::IStream &is);
			virtual void getDebugString(std::string &) const;
			virtual const NLAIAGENT::IObjectIA::CProcessResult &run();
			bool isEqual(const CFuzzyVar &a) const;
			bool isEqual(const NLAIAGENT::IBasicObjectIA &a) const;
			virtual bool isTrue() const;
			const NLAIC::CIdentType &getType() const;
			virtual bool operator==(NLAILOGIC::IBaseVar *var);
			bool unify(NLAILOGIC::IBaseVar *, bool assign = false);
			bool unify(NLAIAGENT::IObjetOp *, bool assign = false);

			virtual NLAIAGENT::IObjetOp *operator == (NLAIAGENT::IObjetOp &a) const;
			
			static const NLAIC::CIdentType IdFuzzyVar;

			sint32 getMethodIndexSize() const;
			virtual NLAIAGENT::TQueue isMember(const NLAIAGENT::IVarName *,const NLAIAGENT::IVarName *,const NLAIAGENT::IObjectIA &) const;
			virtual NLAIAGENT::IObjectIA::CProcessResult runMethodeMember(sint32, NLAIAGENT::IObjectIA *);
			virtual NLAIAGENT::IObjectIA::CProcessResult runMethodeMember(sint32, sint32, NLAIAGENT::IObjectIA *);

			virtual void init(NLAIAGENT::IObjectIA *);
	};
}

#endif
