/** \file file.cpp
 *	Interpret class for operators
 *
 * $Id: interpret_object_operator.h,v 1.4 2001/01/08 14:39:59 valignat Exp $
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

#include "nel/ai/script/interpret_object_agent.h"

namespace NLAISCRIPT
{
	
	class COperatorClass: public CAgentClass
	{
	public:
		static const NLAIC::CIdentType IdOperatorClass;
	public:
		COperatorClass(const NLAIAGENT::IVarName &);
		COperatorClass(const NLAIC::CIdentType &);
		COperatorClass(const NLAIAGENT::IVarName &, const NLAIAGENT::IVarName &);
		COperatorClass(const COperatorClass &);
		COperatorClass();

		const NLAIC::IBasicType *clone() const;
		const NLAIC::IBasicType *newInstance() const;
		void getDebugString(char *t) const;

		virtual NLAIAGENT::IObjectIA *buildNewInstance() const;

		virtual ~COperatorClass();
	};
}
#endif
