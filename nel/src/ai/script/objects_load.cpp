/** \file objects_load.cpp
 *
 * $Id: objects_load.cpp,v 1.6 2001/01/19 14:34:54 chafik Exp $
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
#include "nel/ai/script/compilateur.h"
#include "nel/ai/agent/agent_script.h"
#include "nel/ai/script/type_def.h"
#include "nel/ai/script/object_load.h"


namespace NLAISCRIPT
{

	//*************************************
	// CLoadSelfObject
	//*************************************
	const NLAIAGENT::IBaseGroupType *CLoadSelfObject::getParam(CCodeContext &context) const
	{
		return (const NLAIAGENT::IBaseGroupType *)context.Stack[(int)context.Stack];		
	}

	const NLAIAGENT::IObjectIA *CLoadSelfObject::getObject(CCodeContext &context) const
	{
		NLAIAGENT::IObjectIA *a = ((NLAIAGENT::IObjectIA *)context.Self);
		std::list<sint32>::const_iterator i = _I.begin();
		int j;
		while(i != _I.end())
		{
			j = *i++;
			a = (NLAIAGENT::IObjectIA *)a->getStaticMember(j);
		}			
		return a;
	}

	//*************************************
	// CLoadStackObject
	//*************************************

	const NLAIAGENT::IBaseGroupType *CLoadStackObject::getParam(CCodeContext &context) const
	{
		return (NLAIAGENT::IBaseGroupType *)context.Stack[(int)context.Stack];
	}

	const NLAIAGENT::IObjectIA *CLoadStackObject::getObject(CCodeContext &context) const
	{
		NLAIAGENT::IObjectIA *a = (NLAIAGENT::IObjectIA *)context.Stack[(int)context.Stack - 1];
		std::list<sint32>::const_iterator i = _I.begin();
		int j;
		while(i != _I.end())
		{
			j = *i++;
			a = (NLAIAGENT::IObjectIA *)a->getStaticMember(j);
		}
		return a;
	}

	//*************************************
	// CLoadHeapObject
	//*************************************
	const NLAIAGENT::IBaseGroupType *CLoadHeapObject::getParam(CCodeContext &context) const
	{
		return (const NLAIAGENT::IBaseGroupType *)context.Stack[(int)context.Stack];		
	}

	const NLAIAGENT::IObjectIA *CLoadHeapObject::getObject(CCodeContext &context) const
	{
		NLAIAGENT::IObjectIA *a = (NLAIAGENT::IObjectIA *)context.Heap[(int)_N];
		std::list<sint32>::const_iterator i = _I.begin();
		int j;
		while(i != _I.end())
		{
			j = *i++;
			a = (NLAIAGENT::IObjectIA *)a->getStaticMember(j);
		}
		return a;
	}

}
