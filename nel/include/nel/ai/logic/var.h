/** \file var.h
 *	Base variable class for first order and fuzzy logic
 *
 * $Id: var.h,v 1.7 2003/01/21 11:24:25 chafik Exp $
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

#ifndef NL_VAR_H
#define NL_VAR_H

#include "nel/ai/agent/agent.h"

namespace NLAILOGIC {

	class IBaseVar : public NLAIAGENT::IObjetOp
	{
		protected:
			NLAIAGENT::IVarName *_Name;

		public:
			IBaseVar(const NLAIAGENT::IVarName &);
			IBaseVar(const char *);

			IBaseVar(const IBaseVar &);
			virtual ~IBaseVar();

			void setName(NLAIAGENT::IVarName &);
			NLAIAGENT::IVarName &getName() const;
			virtual void setValue(NLAIAGENT::IObjetOp*) = 0;
			virtual NLAIAGENT::IObjetOp*getValue() const = 0;		
			virtual bool unify(IBaseVar *,bool assign = false) = 0;
			virtual bool unify(NLAIAGENT::IObjetOp*, bool assign = false) = 0;

			// Ajout du Set dans les variables
			virtual NLAIAGENT::TQueue isMember(const NLAIAGENT::IVarName *, const NLAIAGENT::IVarName *, const NLAIAGENT::IObjectIA &) const;
			virtual	NLAIAGENT::IObjectIA::CProcessResult runMethodeMember(sint32 , NLAIAGENT::IObjectIA *);
			virtual sint32 getMethodIndexSize() const;
	};

	class CVar : public IBaseVar 
	{	
		private:
			NLAIAGENT::IObjetOp*_Value;

		public:
			static const NLAIC::CIdentType IdVar;

		public:
			CVar(const NLAIAGENT::IVarName &,NLAIAGENT::IObjetOp*value = NULL);
			CVar(const char *,NLAIAGENT::IObjetOp*value = NULL);
			CVar(const CVar &cp);
			virtual ~CVar();
			virtual void setValue(NLAIAGENT::IObjetOp*obj);
			virtual NLAIAGENT::IObjetOp *getValue() const;
			virtual const NLAIC::IBasicType *clone() const;
			virtual const NLAIC::IBasicType *newInstance() const;
			virtual void save(NLMISC::IStream &os);
			virtual void load(NLMISC::IStream &is);
			virtual void getDebugString(std::string &) const;
			virtual bool isEqual(const CVar &a) const;
			virtual const NLAIAGENT::IObjectIA::CProcessResult &run();
			bool isEqual(const NLAIAGENT::IBasicObjectIA &a) const;
			virtual bool isTrue() const;
			const NLAIC::CIdentType &getType() const;
			virtual bool operator==(IBaseVar *CVar);
			bool unify(IBaseVar *, bool assign = false);
			bool unify(NLAIAGENT::IObjetOp*, bool assign = false);

			virtual NLAIAGENT::IObjetOp*operator == (NLAIAGENT::IObjetOp &a) const;
	};
}
#endif
