/** \file var_mem.h
 *	Set of variables
 *
 * $Id: var_mem.h,v 1.1 2001/05/31 15:00:19 chafik Exp $
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

#ifndef NL_VAR_MEM_H
#define NL_VAR_MEM_H

#include "nel/ai/agent/volatil_memory.h"

namespace NLAILOGIC 
{

	class CVarMem: public NLAIAGENT::CVolatilMemmory
	{
	public:
		static const NLAIC::CIdentType *IdVarMem;
	public:
		CVarMem(NLAIAGENT::IBaseGroupType *l = new NLAIAGENT::CGroupType) : NLAIAGENT::CVolatilMemmory(l) 
		{
		}

		CVarMem(const CVarMem &l) : NLAIAGENT::CVolatilMemmory(l)
		{
		}

		virtual ~CVarMem() {}

		virtual const NLAIC::CIdentType &getType() const
		{
			return *IdVarMem;
		}

		virtual const NLAIC::IBasicType *newInstance() const
		{			
			NLAIC::IBasicType *x = new CVarMem();		
			return x;
		}

		virtual const NLAIC::IBasicType *clone() const
		{			
			NLAIC::IBasicType *x = new CVarMem(*this);			
			return x;
		}
		

		virtual void sendUpdateMessage(NLAIAGENT::IObjectIA *);

	};
}

#endif