/** \file file.cpp
 *	Base classes for simple and composed boolean conditions
 *
 * $Id: bool_cond.h,v 1.2 2001/01/08 10:47:05 chafik Exp $
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

#ifndef NL_BOOL_COND_H
#define NL_BOOL_COND_H

#include "logic/boolval.h"
#include "logic/bool_assert.h"

namespace NLAILOGIC {
	
	class IBaseCond : public IBaseBoolType
	{
		protected:
			std::list<IBaseBoolType *> _Conds;
		public:
			IBaseCond();
			IBaseCond(std::list<IBaseBoolType *> &);
			~IBaseCond();
			virtual void addCond(IBaseBoolType *);
	};
	
	class CondAnd : public IBaseCond {
		private:
			bool _Value;
		public:
			CondAnd();
			CondAnd(std::list<IBaseBoolType *> &);
			virtual float truthValue() const;
			virtual bool isTrue();
	};

	class CondOr : public IBaseCond {
		private:
			bool _Value;
		public:
			CondOr();
			CondOr(std::list<IBaseBoolType *> &);
			virtual float truthValue() const;
			virtual bool isTrue();
	};
}
#endif
