/** \file interpret_object_list.h
 * Class for define an message class in the interpreter.
 *
 * $Id: interpret_object_message.h,v 1.2 2001/01/05 16:05:57 chafik Exp $
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
#ifndef NL_INTERPRET_OBJECT_MESSAGE_H
#define NL_INTERPRET_OBJECT_MESSAGE_H
#include "script/interpret_object_agent.h"

namespace NLIASCRIPT
{
	class CMessageClass: public CAgentClass
	{
	public:
		static const NLIAC::CIdentType IdMessageClass;
	public:
		CMessageClass(const NLIAAGENT::IVarName &);
		CMessageClass(const NLIAC::CIdentType &);
		CMessageClass(const NLIAAGENT::IVarName &, const NLIAAGENT::IVarName &);
		CMessageClass(const CMessageClass &);
		CMessageClass();

		/// \name CAgentClass method.
		//@{
		virtual const NLIAC::IBasicType *clone() const;
		virtual const NLIAC::IBasicType *newInstance() const;
		virtual void getDebugString(char *t) const;
		virtual NLIAAGENT::IObjectIA *buildNewInstance() const;
		//@}

		virtual ~CMessageClass();
	};
}
#endif
