/** \file file.cpp
 *	Set of variables
 *
 * $Id: varset.h,v 1.3 2001/01/08 14:39:59 valignat Exp $
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

#ifndef NL_VARSET_H_
#define NL_VARSET_H_

#include "nel/ai/logic/boolval.h"

namespace NLAILOGIC {

	class IBaseAssert;
	class CValueSet;

	// Logique d'ordre 1
	class CVarSet : public IBaseBoolType {
		protected:
			std::vector<IBaseVar *> _Vars;
			sint32 NbInstanciated;
		public:
			CVarSet();
			CVarSet(const CVarSet &);
			CVarSet(std::vector<IBaseVar *> &);
			~CVarSet();			
			CValueSet *asCValueSet();
			virtual const NLAIC::IBasicType *clone() const;
			virtual const NLAIC::IBasicType *newInstance() const;
			virtual void save(NLMISC::IStream &os);
			virtual void load(NLMISC::IStream &is);
			virtual const NLAIC::CIdentType &getType() const;			
			virtual void getDebugString(char *text) const;
			virtual float truthValue() const;			
			virtual const CProcessResult &run();
			virtual bool isEqual(const CVarSet &a) const;
			bool isEqual(const NLAIAGENT::IBasicObjectIA &a) const;

			virtual std::vector<IBaseVar *> *getVars();
			void addVar(IBaseVar *);
			void removeVar(IBaseVar *);
			virtual CVarSet *unify(CVarSet *);
			virtual CVarSet *unify(CValueSet *);
			bool isUnified();
			std::list<NLAIAGENT::IObjetOp *> *getValues();
			sint32 size();

		public:
			static const NLAIC::CIdentType IdVarSet;

			virtual NLAIAGENT::IObjetOp *operator ! () const;
			virtual NLAIAGENT::IObjetOp *operator != (NLAIAGENT::IObjetOp &a) const;
			virtual bool isTrue() const;

			virtual sint32 undefined();
			virtual IBaseVar *operator[](sint32);
	};

	class CFactPattern   : public CVarSet {
		private:
			IBaseAssert *_Assert;
		public:

			static const NLAIC::CIdentType IdFactPattern ;
			CFactPattern  (const CFactPattern   &);
			CFactPattern  (const CVarSet &);
			CFactPattern  (IBaseAssert *assert = NULL);
			IBaseAssert *getAssert();
			void setAssert(IBaseAssert *);
			virtual CVarSet *unify(CFactPattern   *);
			virtual bool isEqual(const NLAIAGENT::IBasicObjectIA &) const;
			virtual void init(NLAIAGENT::IObjectIA *);

			virtual const NLAIC::IBasicType *clone() const;
			virtual const NLAIC::IBasicType *newInstance() const;
			virtual const NLAIC::CIdentType &getType() const;			
			virtual void getDebugString(char *text) const;
			virtual void propagate();
	};
}
#endif
