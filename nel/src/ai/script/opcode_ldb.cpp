/** \file opcode_ldb.cpp
 *
 * $Id: opcode_ldb.cpp,v 1.1 2001/01/05 10:53:49 chafik Exp $
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

	NLIAAGENT::TProcessStatement CLdbOpCode::runOpCode(CCodeContext &context)
	{
		context.Stack ++;
		context.Stack[(int)context.Stack] = (NLIAAGENT::IObjectIA *)_B->clone();
		return NLIAAGENT::IObjectIA::ProcessIdle;
	}

	void CLdbOpCode::getDebugResult(char *str,CCodeContext &context) const
	{			
		char X[1024*8];
		_B->getDebugString(X);
		sprintf(str,"ldb %s",X);		
	}
	NLIAAGENT::TProcessStatement CLdbMemberOpCode::runOpCode(CCodeContext &context)
	{
		context.Stack ++;
		context.Stack[(int)context.Stack] = (NLIAAGENT::IObjectIA *)(context.Self)->getStaticMember(_B)->clone();
		return NLIAAGENT::IObjectIA::ProcessIdle;;
	}

	void CLdbMemberOpCode::getDebugResult(char *str,CCodeContext &context) const
	{
					
		sprintf(str,"ldb le composant membre %d de la class '%s'",_B,(const char *)((NLIAAGENT::IObjectIA *)(context.Self))->getType());		
	}


	NLIAAGENT::TProcessStatement CLdbStackMemberiOpCode::runOpCode(CCodeContext &context)
	{
		NLIAAGENT::IObjectIA *a = ((NLIAAGENT::IObjectIA *)context.Stack);
		NLIAAGENT::IObjectIA *obj = a; 
		std::list<sint32>::iterator i = _I.begin();
		sint32 n = _I.size() - 1;
		while(n --)
		{
			a = (NLIAAGENT::IObjectIA *)a->getStaticMember(*i++);
		}
		context.Stack[(int)context.Stack] = (NLIAAGENT::IObjectIA *)a->getStaticMember(*i)->clone();
		obj->release();
		return NLIAAGENT::processIdle;
	}

	void CLdbStackMemberiOpCode::getDebugResult(char *str,CCodeContext &context) const
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
		sprintf(str,"ldb %s le composan membre sur la pile de la class '%s'",txt,(const char *)a->getType());
	}

	NLIAAGENT::TProcessStatement CLdbHeapMemberiOpCode::runOpCode(CCodeContext &context)
	{
		NLIAAGENT::IObjectIA *a = (NLIAAGENT::IObjectIA *)context.Heap[(int)_N];
		std::list<sint32>::iterator i = _I.begin();
		sint32 n = _I.size() - 1;
		while(n --)
		{
			a = (NLIAAGENT::IObjectIA *)a->getStaticMember(*i++);
		}
		a = (NLIAAGENT::IObjectIA *)a->getStaticMember(*i)->clone();
		context.Stack ++;
		context.Stack[(int)context.Stack] = a;
		return NLIAAGENT::processIdle;
	}

	void CLdbHeapMemberiOpCode::getDebugResult(char *str,CCodeContext &context) const
	{						
		NLIAAGENT::IObjectIA *a = (NLIAAGENT::IObjectIA *)context.Heap[(int)_N];
		std::list<sint32>::const_iterator i = _I.begin();
		sint32 n = _I.size() - 1;
		while(n --)
		{
			a = (NLIAAGENT::IObjectIA *)a->getStaticMember(*i++);
		}
		char txt[1024*8];
		a->getStaticMember(*i)->getDebugString(txt);
		sprintf(str,"ldb %s le composant membre sur le heap de la class '%s'",txt,(const char *)a->getType());		
	}

	NLIAAGENT::TProcessStatement CLdbMemberiOpCode::runOpCode(CCodeContext &context)
	{
		context.Stack ++;
		NLIAAGENT::IObjectIA *obj = (NLIAAGENT::IObjectIA *)context.Self;
		std::list<sint32>::iterator i = _I.begin();
		while(i != _I.end())
		{
			obj = (NLIAAGENT::IObjectIA *)obj->getStaticMember(*i++);
		}
		//obj->incRef();
		context.Stack[(int)context.Stack] = (NLIAAGENT::IObjectIA *)obj->clone();
		
		return NLIAAGENT::IObjectIA::ProcessIdle;
	}

	void CLdbMemberiOpCode::getDebugResult(char *str,CCodeContext &context) const
	{			
		NLIAAGENT::IObjectIA *obj = (NLIAAGENT::IObjectIA *)context.Self;
		std::list<sint32>::const_iterator i = _I.begin();
		int j;
		while(i != _I.end())
		{
			j = *i++;
			obj = (NLIAAGENT::IObjectIA *)obj->getStaticMember(j);
		}			
					
		sprintf(str,"ldb le composant membre <%d> member de la class '%s'",j,(const char *)obj->getType());		
	}

	NLIAAGENT::TProcessStatement CLdbRefOpCode::runOpCode(CCodeContext &context)
	{
		context.Stack ++;
		context.Stack[(int)context.Stack] = (NLIAAGENT::IObjectIA *)context.Heap[_B]->clone();
		return NLIAAGENT::IObjectIA::ProcessIdle;;
	}

	void CLdbRefOpCode::getDebugResult(char *str,CCodeContext &context) const
	{
		char X[1024*8];	
		context.Heap[_B]->getDebugString(X);
		sprintf(str,"ldb<%d>",_B);		
	}
}