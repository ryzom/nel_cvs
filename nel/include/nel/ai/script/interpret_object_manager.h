/** \file interpret_object_list.h
 * Class for define an agent manager.
 *
 * $Id: interpret_object_manager.h,v 1.4 2001/01/08 11:16:29 chafik Exp $
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
#ifndef NL_INTERPRET_OBJECT_MANAGER_H
#define NL_INTERPRET_OBJECT_MANAGER_H
#include "script/interpret_object_agent.h"

namespace NLAISCRIPT
{
	class CManagerClass: public CAgentClass
	{
	public:
		static const NLAIC::CIdentType IdManagerClass;
	public:
		CManagerClass(const NLAIAGENT::IVarName &);
		CManagerClass(const NLAIC::CIdentType &);
		CManagerClass(const NLAIAGENT::IVarName &, const NLAIAGENT::IVarName &);
		CManagerClass(const CManagerClass &);
		CManagerClass();

		/// \name CAgentClass redefinition method.
		//@{
		const NLAIC::IBasicType *clone() const;
		const NLAIC::IBasicType *newInstance() const;
		void getDebugString(char *t) const;

		virtual NLAIAGENT::IObjectIA *buildNewInstance() const;
		//@}

		virtual ~CManagerClass();
	};
}
#endif
