/** \file opcode_lpt.cpp
 *
 * $Id: opcode_lpt.cpp,v 1.1 2001/01/05 10:53:49 chafik Exp $
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
#include "script/compilateur.h"
#include "agent/agent_script.h"

namespace NLIASCRIPT
{
	NLIAAGENT::TProcessStatement CLptMemberOpCode::runOpCode(CCodeContext &context)
	{
		context.Stack ++;
		NLIAAGENT::IObjectIA *a = (NLIAAGENT::IObjectIA *)(context.Self)->getStaticMember(_B);
		context.Stack[(int)context.Stack] = a;
		a->incRef();
		return NLIAAGENT::IObjectIA::ProcessIdle;;
	}

	void CLptMemberOpCode::getDebugResult(char *str,CCodeContext &context) const
	{				
		char txt[1024*8];
		((NLIAAGENT::IObjectIA *)(context.Self))->getStaticMember(_B)->getDebugString(txt);
		sprintf(str,"ldb la reference du membre <%d> '%s' de la class '%s'",_B,txt,(const char *)((NLIAAGENT::IObjectIA *)(context.Self))->getType());		
	}


	NLIAAGENT::TProcessStatement CLptStackMemberiOpCode::runOpCode(CCodeContext &context)
	{
		NLIAAGENT::IObjectIA *a = ((NLIAAGENT::IObjectIA *)context.Stack);
		NLIAAGENT::IObjectIA *obj = a; 
		std::list<sint32>::iterator i = _I.begin();
		sint32 n = _I.size() - 1;
		while(n --)
		{
			a = (NLIAAGENT::IObjectIA *)a->getStaticMember(*i++);
		}
		a = (NLIAAGENT::IObjectIA *)a->getStaticMember(*i);
		context.Stack[(int)context.Stack] = a;
		a->incRef();
		obj->release();
		return NLIAAGENT::processIdle;
	}

	void CLptStackMemberiOpCode::getDebugResult(char *str,CCodeContext &context) const
	{	
		NLIAAGENT::IObjectIA *a = ((NLIAAGENT::IObjectIA *)context.Stack);		
		std::list<sint32>::const_iterator i = _I.begin();
		sint32 n = _I.size() - 1;
		while(n --)
		{
			a = (NLIAAGENT::IObjectIA *)a->getStaticMember(*i++);
		}
		char txt[1024*8];
		a->getStaticMember(*i)->getDebugString(txt);
		sprintf(str,"Ldb la reference du composant membre '%s' sur le heap de la class '%s'",txt,(const char *)a->getType());
	}

	NLIAAGENT::TProcessStatement CLptHeapMemberiOpCode::runOpCode(CCodeContext &context)
	{
		NLIAAGENT::IObjectIA *a = (NLIAAGENT::CAgentScript *)((NLIAAGENT::IObjectIA *)context.Heap[(int)_N]);			
		std::list<sint32>::iterator i = _I.begin();
		sint32 n = _I.size() - 1;
		while(n --)
		{
			a = (NLIAAGENT::IObjectIA *)a->getStaticMember(*i++);
		}
		//a = (NLIAAGENT::IObjectIA *)a->getStaticMember(*i)->clone();
		context.Stack ++;
		context.Stack[(int)context.Stack] = a;
		a->incRef();
		return NLIAAGENT::processIdle;
	}

	void CLptHeapMemberiOpCode::getDebugResult(char *str,CCodeContext &context) const
	{						
		NLIAAGENT::IObjectIA *a = (NLIAAGENT::CAgentScript *)((NLIAAGENT::IObjectIA *)context.Heap[(int)_N]);
		std::list<sint32>::const_iterator i = _I.begin();
		sint32 n = _I.size() - 1;
		while(n --)
		{
			a = (NLIAAGENT::IObjectIA *)a->getStaticMember(*i++);
		}
		char txt[1024*8];
		a->getStaticMember(*i)->getDebugString(txt);
		sprintf(str,"Ldb la reference du composant membre '%s' sur le heap de la class '%s'",txt,(const char *)a->getType());		
	}


	NLIAAGENT::TProcessStatement CLptMemberiOpCode::runOpCode(CCodeContext &context)
	{
		context.Stack ++;
		NLIAAGENT::IObjectIA *obj = (NLIAAGENT::IObjectIA *)context.Self;
		std::list<sint32>::iterator i = _I.begin();
		while(i != _I.end())
		{
			obj = (NLIAAGENT::IObjectIA *)obj->getStaticMember(*i++);
		}
		obj->incRef();
		context.Stack[(int)context.Stack] = (NLIAAGENT::IObjectIA *)obj;
		
		return NLIAAGENT::IObjectIA::ProcessIdle;
	}

	void CLptMemberiOpCode::getDebugResult(char *str,CCodeContext &context) const
	{			
		NLIAAGENT::IObjectIA *obj = (NLIAAGENT::IObjectIA *)context.Self;
		std::list<sint32>::const_iterator i = _I.begin();
		int j;
		while(i != _I.end())
		{
			j = *i++;
			obj = (NLIAAGENT::IObjectIA *)obj->getStaticMember(j);
		}			
					
		sprintf(str,"Lpt la referance du membre <%d> member de la class '%s'",j,(const char *)obj->getType());		
	}

	NLIAAGENT::TProcessStatement CLptRefOpCode::runOpCode(CCodeContext &context)
	{
		context.Stack ++;
		NLIAAGENT::IObjectIA *a = (NLIAAGENT::IObjectIA *)context.Heap[_B]->clone();
		context.Stack[(int)context.Stack] = a;
		a->incRef();
		return NLIAAGENT::IObjectIA::ProcessIdle;
	}

	void CLptRefOpCode::getDebugResult(char *str,CCodeContext &context) const
	{
		char X[1024*8];	
		context.Heap[_B]->getDebugString(X);
		sprintf(str,"Lpt<%d>",_B);		
	}
}