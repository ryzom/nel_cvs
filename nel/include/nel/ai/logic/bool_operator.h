/** \file bool_operator.h
 *	0 order logic operators base class 
 *
 * $Id: bool_operator.h,v 1.8 2003/02/05 16:05:53 chafik Exp $
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
#ifndef NL_BOOL_OPERATOR_H
#define NL_BOOL_OPERATOR_H

#include "nel/ai/logic/operator.h"
#include "nel/ai/logic/ai_assert.h"

namespace NLAILOGIC {
	
	/**
	This class define an 0 order logic operators class.
	*/
	class CBoolOperator : public IBaseOperator
	{
		private:
			char				       	*_Comment;
			std::vector<bool>			_CondsVal;
			std::vector<bool>			_ConcsVal;
		public:
			CBoolOperator();
			CBoolOperator(char *n);
			CBoolOperator(CBoolOperator &);

			virtual void addPrecondition(IBaseAssert *, bool);
			virtual void addPostcondition(IBaseAssert *, bool);

			void forward( std::vector<bool> &);
			virtual bool isTrue() const;
			std::list<CFact *> *backward(std::list<IBaseAssert *> &, std::list<bool> &);
			virtual bool isValid(CFactBase *);

			static const NLAIC::CIdentType IdBoolOperator;
			virtual const NLAIC::CIdentType &getType() const;

			virtual const NLAIC::IBasicType *clone() const;
			virtual const NLAIC::IBasicType *newInstance() const;
			virtual void save(NLMISC::IStream &os);
			virtual void load(NLMISC::IStream &is);
			virtual void getDebugString(std::string &) const;
			virtual bool isEqual(const CBoolOperator &) const;
			virtual const NLAIAGENT::IObjectIA::CProcessResult &run();
			bool isEqual(const NLAIAGENT::IBasicObjectIA &a) const;
			virtual float truthValue() const;

			virtual std::list<CFact *> *backward(std::list<CFact *> &);
			virtual std::list<CFact *> *forward(std::list<CFact *> &);
			virtual std::list<CFact *> *propagate(std::list<CFact *> &);

			virtual float priority() const;
			virtual void success();
			virtual void failure();
			virtual void success(IBaseOperator *);
			virtual void failure(IBaseOperator *);
	};
}
#endif
