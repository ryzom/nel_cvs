/** \file file.cpp
 *	First order logic condition
 *
 * $Id: clause.h,v 1.1 2001/01/05 10:50:23 chafik Exp $
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

#ifndef NL_CLAUSE_H
#define NL_CLAUSE_H
#include "logic/varset.h"

namespace NLIALOGIC {

	class IBaseBoolType;
	class IBaseAssert;
	class CValueSet;
	class CBoolAssert;

	class CClause : public CVarSet {
		protected:
			std::list<CBoolAssert *>		_SimpleConds;	
			std::vector<IBaseBoolType *>	_Conds;
			std::vector< std::vector<sint32> >	_PosVarsCond;
			std::list< CValueSet *>			_Liaisons;
			std::list< CValueSet *>			_BufLiaisons;
			std::list< CValueSet *>			_Conflits;
			sint32 _NbValid;
			
		protected:
			std::vector<IBaseAssert *>	_Asserts;
			sint32 findAssert(IBaseAssert *);
			bool _FirstOrderTrue;

		public:
			static const NLIAC::CIdentType IdClause;

			CClause();
			CClause(const CClause &);
			CClause(std::list<CFactPattern   *> &);
			~CClause();
			virtual void addCond(CFactPattern   *cond);
			virtual void getDebugString(char *) const;
			virtual CVarSet *unify(CVarSet *);
			CValueSet *unifyLiaison( const CValueSet *, std::list<NLIAAGENT::IObjetOp *> *, std::vector<sint32> &);
			virtual void propagate(std::list<NLIAAGENT::IObjetOp *> *, std::vector<sint32> &);
			virtual void propagate(CFactPattern   *);
			std::list<CClause *> *getInputs();

			virtual const NLIAC::IBasicType *clone() const;
			virtual const NLIAC::IBasicType *newInstance() const;

			virtual NLIAAGENT::IObjetOp *operator ! () const;
			virtual NLIAAGENT::IObjetOp *operator != (NLIAAGENT::IObjetOp &a) const;
			virtual NLIAAGENT::IObjetOp *operator == (NLIAAGENT::IObjetOp &a) const;
			virtual bool isTrue() const;
			virtual float truthValue() const;

			void showConflicts();
			void showBuffer();
			void showLiaisons();

			virtual void addBuffer();
			virtual void addConflicts();

			std::vector<IBaseAssert *> &CClause::getAssert();
			sint32 nbVars();
			void CClause::init(NLIAAGENT::IObjectIA *);
	};
}
#endif
