/** \file file.cpp
 *	Interpret class for operators
 *
 * $Id: interpret_object_operator.h,v 1.1 2001/01/05 10:50:23 chafik Exp $
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

#ifndef NL_OPERATOR_CLASS
#define NL_OPERATOR_CLASS

#include "script/interpret_object_agent.h"

namespace NLIASCRIPT
{
	
	class COperatorClass: public CAgentClass
	{
	public:
		static const NLIAC::CIdentType IdOperatorClass;
	public:
		COperatorClass(const NLIAAGENT::IVarName &);
		COperatorClass(const NLIAC::CIdentType &);
		COperatorClass(const NLIAAGENT::IVarName &, const NLIAAGENT::IVarName &);
		COperatorClass(const COperatorClass &);
		COperatorClass();

		const NLIAC::IBasicType *clone() const;
		const NLIAC::IBasicType *newInstance() const;
		void getDebugString(char *t) const;

		virtual NLIAAGENT::IObjectIA *buildNewInstance() const;

		virtual ~COperatorClass();
	};
}
#endif
