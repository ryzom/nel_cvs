/** \file boolval.h
 *	Base classes for boolean values (conditions, rules...)
 *
 * $Id: boolval.h,v 1.7 2003/02/05 16:05:53 chafik Exp $
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

#ifndef NL_BOOLVAL_H
#define NL_BOOLVAL_H

#include "nel/ai/agent/agent.h"

namespace NLAILOGIC {

	class IBaseVar;

	// Classe de base des valeurs booléennes
	class IBaseBoolType : public NLAIAGENT::IObjetOp {		
		public:
			IBaseBoolType();
			virtual ~IBaseBoolType();

			/// Returns a value in [0,1]
			virtual float truthValue() const = 0;
			virtual const CProcessResult &run() = 0;
	};

	// Valeur booléenne simple
	class CBoolType : public IBaseBoolType {
		private:
			bool _Value;
		public:
			CBoolType(bool);
			CBoolType(const CBoolType &);
			virtual ~CBoolType();
			virtual const NLAIC::IBasicType *clone() const;
			virtual const NLAIC::IBasicType *newInstance() const;
			virtual const NLAIC::CIdentType &getType() const;
			virtual void save(NLMISC::IStream &os);
			virtual void load(NLMISC::IStream &is);
			virtual void getDebugString(std::string &) const;

			virtual float truthValue() const;			
			virtual const CProcessResult &run();
			virtual bool isEqual(const NLAIAGENT::IBasicObjectIA &a) const;
			virtual bool isEqual(const CBoolType &a) const;
			virtual std::vector<IBaseVar *> *getVars();

		public:

			static const NLAIC::CIdentType IdBoolType;

			virtual NLAIAGENT::IObjetOp &operator += (const NLAIAGENT::IObjetOp &a);
			virtual NLAIAGENT::IObjetOp &operator -= (const NLAIAGENT::IObjetOp &a);
			virtual NLAIAGENT::IObjetOp &operator *= (const NLAIAGENT::IObjetOp &a);

			virtual NLAIAGENT::IObjetOp *operator < (NLAIAGENT::IObjetOp &a) const;
			virtual NLAIAGENT::IObjetOp *operator > (NLAIAGENT::IObjetOp &a) const;
			virtual NLAIAGENT::IObjetOp *operator <= (NLAIAGENT::IObjetOp &a) const;	
			virtual NLAIAGENT::IObjetOp *operator >= (NLAIAGENT::IObjetOp &a) const;
			virtual NLAIAGENT::IObjetOp *operator ! () const;
			virtual NLAIAGENT::IObjetOp *operator != (NLAIAGENT::IObjetOp &a) const;
			virtual NLAIAGENT::IObjetOp *operator == (NLAIAGENT::IObjetOp &a) const;


			virtual bool isTrue() const;
	};

	class CBoolTrue : public CBoolType
	{
		public:
			CBoolTrue();
	};

	class CBoolFalse : public CBoolType
	{
		public:
			CBoolFalse();
	};
}
#endif
