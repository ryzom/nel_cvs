/** \file file.cpp
 *	Base variable class for first order and fuzzy logic
 *
 * $Id: var.h,v 1.1 2001/01/05 10:50:23 chafik Exp $
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

#include "agent/agent.h"

namespace NLIALOGIC {

	class IBaseVar : public NLIAAGENT::IObjetOp
	{
		protected:
			NLIAAGENT::IVarName *_Name;

		public:
			IBaseVar(const NLIAAGENT::IVarName &);
			IBaseVar(const char *);

			IBaseVar(const IBaseVar &);
			virtual ~IBaseVar();

			void setName(NLIAAGENT::IVarName &);
			NLIAAGENT::IVarName &getName() const;
			virtual void setValue(NLIAAGENT::IObjetOp*) = 0;
			virtual NLIAAGENT::IObjetOp*getValue() const = 0;		
			virtual bool unify(IBaseVar *,bool assign = false) = 0;
			virtual bool unify(NLIAAGENT::IObjetOp*, bool assign = false) = 0;

			// Ajout du Set dans les variables
			virtual NLIAAGENT::tQueue isMember(const NLIAAGENT::IVarName *, const NLIAAGENT::IVarName *, const NLIAAGENT::IObjectIA &) const;
			virtual	NLIAAGENT::IObjectIA::CProcessResult runMethodeMember(sint32 , NLIAAGENT::IObjectIA *);
			virtual sint32 getMethodIndexSize() const;

	};

	class CVar : public IBaseVar 
	{	
		private:
			NLIAAGENT::IObjetOp*_Value;

		public:
			static const NLIAC::CIdentType IdVar;

		public:
			CVar(const NLIAAGENT::IVarName &,NLIAAGENT::IObjetOp*value = NULL);
			CVar(const char *,NLIAAGENT::IObjetOp*value = NULL);
			CVar(const CVar &cp);
			virtual ~CVar();
			virtual void setValue(NLIAAGENT::IObjetOp*obj);
			virtual NLIAAGENT::IObjetOp*getValue() const;
			virtual const NLIAC::IBasicType *clone() const;
			virtual const NLIAC::IBasicType *newInstance() const;
			virtual void save(NLMISC::IStream &os);
			virtual void load(NLMISC::IStream &is);
			virtual void getDebugString(char *text) const;
			virtual bool isEqual(const CVar &a) const;
			virtual const NLIAAGENT::IObjectIA::CProcessResult &run();
			bool isEqual(const NLIAAGENT::IBasicObjectIA &a) const;
			virtual bool isTrue() const;
			const NLIAC::CIdentType &getType() const;
			virtual bool operator==(IBaseVar *CVar);
			bool unify(IBaseVar *, bool assign = false);
			bool unify(NLIAAGENT::IObjetOp*, bool assign = false);

			virtual NLIAAGENT::IObjetOp*operator == (NLIAAGENT::IObjetOp&a) const;
	};
}
#endif
