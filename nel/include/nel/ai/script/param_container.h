/** \file param_container.h
 * Includes all for compiling a script.
 *
 * $Id: param_container.h,v 1.1 2002/01/30 16:51:49 chafik Exp $
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

#ifndef NL_PARAM_CONTAINER_H
#define NL_PARAM_CONTAINER_H

#include "nel/ai/agent/agent_object.h"

namespace NLAISCRIPT
{
	class CPramContainer: public NLAIAGENT::CGroupType
	{
	public:
		static const NLAIC::CIdentType IdPramContainer;
	public:
		CPramContainer();
		CPramContainer(const CPramContainer &g);

		virtual ~CPramContainer();

		virtual const NLAIC::IBasicType *clone() const;		

		virtual const NLAIC::IBasicType *newInstance() const;		

		virtual const NLAIC::CIdentType &getType() const
		{
			return IdPramContainer;
		}

		virtual NLAIAGENT::IObjetOp &operator += (const NLAIAGENT::IObjetOp &a);
	};
}

#endif
