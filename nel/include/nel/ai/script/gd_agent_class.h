/** \file gd_agent_class.h
 * Class for define a goals driven agent, using a factbase and a goal stack
 *
 * $Id: gd_agent_class.h,v 1.2 2001/05/22 16:08:01 chafik Exp $
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

#ifndef NL_GD_AGENT_CLASS_H
#define NL_GD_AGENT_CLASS_H

#include "nel/ai/script/interpret_object_agent.h"

namespace NLAISCRIPT
{
	class CGDAgentClass: public CAgentClass
	{
	public:
		static const NLAIC::CIdentType IdGDAgentClass;
	public:
		CGDAgentClass(const NLAIAGENT::IVarName &);
		CGDAgentClass(const NLAIC::CIdentType &);
		CGDAgentClass(const NLAIAGENT::IVarName &, const NLAIAGENT::IVarName &);
		CGDAgentClass(const CGDAgentClass &);
		CGDAgentClass();

		/// \name CAgentClass method.
		//@{
		virtual const NLAIC::IBasicType *clone() const;
		virtual const NLAIC::IBasicType *newInstance() const;
		//virtual void getDebugString(char *t) const;
		virtual NLAIAGENT::IObjectIA *buildNewInstance() const;		
		//@}

		virtual ~CGDAgentClass();
	};

} // NLAISCRIPT
#endif // NL_GD_AGENT_CLASS
