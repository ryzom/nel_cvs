/** \file file.cpp
 *	Set of values
 *
 * $Id: valueset.h,v 1.3 2001/01/08 14:39:59 valignat Exp $
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

#ifndef NL_VALUE_SET_H
#define NL_VALUE_SET_H

#include "nel/ai/agent/agent.h"

namespace NLAILOGIC {

	class IBaseVar;
	class CClause;

	class CValueSet  : public NLAIAGENT::IObjetOp
	{
		protected:
			NLAIAGENT::IObjetOp **_Values;
			sint32 _NbValues;

		public: 
			CValueSet(sint32);
			CValueSet(const CValueSet &);
			CValueSet(std::vector<IBaseVar *> &);
			CValueSet(CValueSet *,std::vector<sint32> &);
			CValueSet(sint32,std::list<NLAIAGENT::IObjetOp *> *,std::vector<sint32> &);
			~CValueSet();
			virtual NLAIAGENT::IObjetOp *getValue(sint32);
			void setValue(sint32, NLAIAGENT::IObjetOp *);
			CValueSet *unify(const CValueSet *) const ;
			CValueSet *unify(std::list<NLAIAGENT::IObjetOp *> *vals, std::vector<sint32> &pos_vals) const;
			CValueSet *unify(CValueSet *vals, std::vector<sint32> &pos_vals) const;
			bool operator==(CValueSet *);
			virtual NLAIAGENT::IObjetOp *operator[](sint32);
			sint32 size();
			sint32 undefined() const;
			std::list<NLAIAGENT::IObjetOp *> *getValues();
			virtual void getDebugString(char *) const;

			virtual const NLAIC::IBasicType *clone() const;
			virtual const NLAIC::IBasicType *newInstance() const;
			virtual const NLAIC::CIdentType &getType() const;
			virtual void save(NLMISC::IStream &os);
			virtual void load(NLMISC::IStream &is);

			static const NLAIC::CIdentType IdValueSet;

			virtual NLAIAGENT::IObjetOp *operator ! () const;
			virtual NLAIAGENT::IObjetOp *operator != (NLAIAGENT::IObjetOp &a) const;
			virtual NLAIAGENT::IObjetOp *operator == (NLAIAGENT::IObjetOp &a) const;

			virtual bool isEqual(const NLAIAGENT::IBasicObjectIA &) const;
			

			virtual bool isTrue() const;

			static const NLAIAGENT::TProcessStatement state;

			virtual const IObjectIA::CProcessResult &run();
			CValueSet *forward(CClause *,std::vector<sint32> &);
	};
}

#endif
