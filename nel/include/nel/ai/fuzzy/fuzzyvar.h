/** \file fuzzyvar.h
 *	Dexcribes a fuzzy variable with its fuzzy sets and defuzzyfication method
 *
 * $Id: fuzzyvar.h,v 1.1 2001/01/05 10:50:23 chafik Exp $
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

#include "fuzzy/fuzzyset.h"
#include "logic/var.h"
#include "fuzzy/fuzzyfact.h"

namespace NLIAFUZZY
{

	class CSimpleFuzzyCond;

	class CFuzzyVar : public NLIALOGIC::IBaseVar {
		private:
			double _Value;
			double _Min;
			double _Max;
			char *_Name;
			std::vector<IFuzzySet *> _sets;				// Sous ensembles flous
			std::vector<NLIAAGENT::CStringVarName *> _qual;		// Qualificateurs linguistiques associés	
			std::vector<CFuzzyFact *> _facts;

		public:
			// Constructors
			// Parmaters: name, val min, val max, initial value
			CFuzzyVar(const NLIAAGENT::IVarName &, double, double, NLIAAGENT::IObjetOp *);
			CFuzzyVar(const NLIAAGENT::IVarName &, double, double, double value = 0 );
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

			virtual void setValue(NLIAAGENT::IObjetOp *obj);
			virtual void setValue(float);
			virtual NLIAAGENT::IObjetOp *getValue() const;
			virtual const NLIAC::IBasicType *clone() const;
			virtual const NLIAC::IBasicType *newInstance() const;
			virtual void save(NLMISC::IStream &os);
			virtual void load(NLMISC::IStream &is);
			virtual void getDebugString(char *txt) const;
			virtual const NLIAAGENT::IObjectIA::CProcessResult &run();
			bool isEqual(const CFuzzyVar &a) const;
			bool isEqual(const NLIAAGENT::IBasicObjectIA &a) const;
			virtual bool isTrue() const;
			const NLIAC::CIdentType &getType() const;
			virtual bool operator==(NLIALOGIC::IBaseVar *var);
			bool unify(NLIALOGIC::IBaseVar *, bool assign = false);
			bool unify(NLIAAGENT::IObjetOp *, bool assign = false);

			virtual NLIAAGENT::IObjetOp *operator == (NLIAAGENT::IObjetOp &a) const;
			
			static const NLIAC::CIdentType IdFuzzyVar;

			sint32 getMethodIndexSize() const;
			virtual NLIAAGENT::tQueue isMember(const NLIAAGENT::IVarName *,const NLIAAGENT::IVarName *,const NLIAAGENT::IObjectIA &) const;
			virtual NLIAAGENT::IObjectIA::CProcessResult runMethodeMember(sint32, NLIAAGENT::IObjectIA *);
			virtual NLIAAGENT::IObjectIA::CProcessResult runMethodeMember(sint32, sint32, NLIAAGENT::IObjectIA *);

			virtual void init(NLIAAGENT::IObjectIA *);
	};
}

#endif
