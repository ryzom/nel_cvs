/** \file file.cpp
 *	Set of values
 *
 * $Id: valueset.h,v 1.1 2001/01/05 10:50:23 chafik Exp $
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

#include "agent/agent.h"

namespace NLIALOGIC {

	class IBaseVar;
	class CClause;

	class CValueSet  : public NLIAAGENT::IObjetOp
	{
		protected:
			NLIAAGENT::IObjetOp **_Values;
			sint32 _NbValues;

		public: 
			CValueSet(sint32);
			CValueSet(const CValueSet &);
			CValueSet(std::vector<IBaseVar *> &);
			CValueSet(CValueSet *,std::vector<sint32> &);
			CValueSet(sint32,std::list<NLIAAGENT::IObjetOp *> *,std::vector<sint32> &);
			~CValueSet();
			virtual NLIAAGENT::IObjetOp *getValue(sint32);
			void setValue(sint32, NLIAAGENT::IObjetOp *);
			CValueSet *unify(const CValueSet *) const ;
			CValueSet *unify(std::list<NLIAAGENT::IObjetOp *> *vals, std::vector<sint32> &pos_vals) const;
			CValueSet *unify(CValueSet *vals, std::vector<sint32> &pos_vals) const;
			bool operator==(CValueSet *);
			virtual NLIAAGENT::IObjetOp *operator[](sint32);
			sint32 size();
			sint32 undefined() const;
			std::list<NLIAAGENT::IObjetOp *> *getValues();
			virtual void getDebugString(char *) const;

			virtual const NLIAC::IBasicType *clone() const;
			virtual const NLIAC::IBasicType *newInstance() const;
			virtual const NLIAC::CIdentType &getType() const;
			virtual void save(NLMISC::IStream &os);
			virtual void load(NLMISC::IStream &is);

			static const NLIAC::CIdentType IdValueSet;

			virtual NLIAAGENT::IObjetOp *operator ! () const;
			virtual NLIAAGENT::IObjetOp *operator != (NLIAAGENT::IObjetOp &a) const;
			virtual NLIAAGENT::IObjetOp *operator == (NLIAAGENT::IObjetOp &a) const;

			virtual bool isEqual(const NLIAAGENT::IBasicObjectIA &) const;
			

			virtual bool isTrue() const;

			static const NLIAAGENT::TProcessStatement state;

			virtual const IObjectIA::CProcessResult &run();
			CValueSet *forward(CClause *,std::vector<sint32> &);
	};
}

#endif
