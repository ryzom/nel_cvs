/** \file fuzzytype.h
 *	Class used to intergrate fuzzy conditions into boolean complex conditions
 *
 * $Id: fuzzytype.h,v 1.3 2001/01/08 14:39:59 valignat Exp $
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

#ifndef NL_FUZZYTYPE_H
#define NL_FUZZYTYPE_H

#include "nel/ai/logic/boolval.h"

namespace NLAIFUZZY {

	// Valeur floue
	class FuzzyType : public NLAILOGIC::IBaseBoolType {
		private:
			float _Value;
			float _Threshold;
		public:
			
			virtual const NLAIC::IBasicType *clone() const;
			virtual const NLAIC::IBasicType *newInstance() const;
			virtual const NLAIC::CIdentType &getType() const;			
			virtual void save(NLMISC::IStream &os);
			virtual void load(NLMISC::IStream &is);
			virtual void getDebugString(char *text) const;
			virtual float truthValue() const;
			virtual bool isTrue() const;
			virtual const NLAIAGENT::IVarName * getName() const;
			virtual void setName(const NLAIAGENT::IVarName &n);
			virtual const CProcessResult &run();
			virtual bool isEqual(const FuzzyType &a) const;
			bool isEqual(const NLAIAGENT::IBasicObjectIA &a) const;
			virtual std::vector<NLAILOGIC::IBaseVar *> *getVars();

			static const NLAIC::CIdentType IdFuzzyType;

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
	};
}

#endif
