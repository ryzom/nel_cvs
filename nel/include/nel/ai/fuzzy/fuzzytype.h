/** \file fuzzytype.h
 *	Class used to intergrate fuzzy conditions into boolean complex conditions
 *
<<<<<<< fuzzytype.h
 * $Id: fuzzytype.h,v 1.1 2001/01/05 10:50:23 chafik Exp $
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

#include "logic/boolval.h"

namespace NLIAFUZZY {

	// Valeur floue
	class FuzzyType : public NLIALOGIC::IBaseBoolType {
		private:
			float _Value;
			float _Threshold;
		public:
			
			virtual const NLIAC::IBasicType *clone() const;
			virtual const NLIAC::IBasicType *newInstance() const;
			virtual const NLIAC::CIdentType &getType() const;			
			virtual void save(NLMISC::IStream &os);
			virtual void load(NLMISC::IStream &is);
			virtual void getDebugString(char *text) const;
			virtual float truthValue() const;
			virtual bool isTrue() const;
			virtual const NLIAAGENT::IVarName * getName() const;
			virtual void setName(const NLIAAGENT::IVarName &n);
			virtual const CProcessResult &run();
			virtual bool isEqual(const FuzzyType &a) const;
			bool isEqual(const NLIAAGENT::IBasicObjectIA &a) const;
			virtual std::vector<NLIALOGIC::IBaseVar *> *getVars();

			static const NLIAC::CIdentType IdFuzzyType;

			virtual NLIAAGENT::IObjetOp &operator += (const NLIAAGENT::IObjetOp &a);
			virtual NLIAAGENT::IObjetOp &operator -= (const NLIAAGENT::IObjetOp &a);
			virtual NLIAAGENT::IObjetOp &operator *= (const NLIAAGENT::IObjetOp &a);

			virtual NLIAAGENT::IObjetOp *operator < (NLIAAGENT::IObjetOp &a) const;
			virtual NLIAAGENT::IObjetOp *operator > (NLIAAGENT::IObjetOp &a) const;
			virtual NLIAAGENT::IObjetOp *operator <= (NLIAAGENT::IObjetOp &a) const;	
			virtual NLIAAGENT::IObjetOp *operator >= (NLIAAGENT::IObjetOp &a) const;
			virtual NLIAAGENT::IObjetOp *operator ! () const;
			virtual NLIAAGENT::IObjetOp *operator != (NLIAAGENT::IObjetOp &a) const;
			virtual NLIAAGENT::IObjetOp *operator == (NLIAAGENT::IObjetOp &a) const;
	};
}

#endif
