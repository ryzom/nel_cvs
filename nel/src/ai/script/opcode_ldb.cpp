/** \file opcode_ldb.cpp
 *
 * $Id: opcode_ldb.cpp,v 1.6 2001/01/17 10:32:10 chafik Exp $
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

namespace NLAISCRIPT
{

	NLAIAGENT::TProcessStatement CLdbOpCode::runOpCode(CCodeContext &context)
	{
		context.Stack ++;
		context.Stack[(int)context.Stack] = (NLAIAGENT::IObjectIA *)_B->clone();
		return NLAIAGENT::IObjectIA::ProcessIdle;
	}

	void CLdbOpCode::getDebugResult(char *str,CCodeContext &context) const
	{			
		char X[1024*8];
		_B->getDebugString(X);
		sprintf(str,"ldb %s",X);		
	}
	NLAIAGENT::TProcessStatement CLdbMemberOpCode::runOpCode(CCodeContext &context)
	{
		context.Stack ++;
		context.Stack[(int)context.Stack] = (NLAIAGENT::IObjectIA *)(context.Self)->getStaticMember(_B)->clone();
		return NLAIAGENT::IObjectIA::ProcessIdle;;
	}

	void CLdbMemberOpCode::getDebugResult(char *str,CCodeContext &context) const
	{
					
		sprintf(str,"ldb le composant membre %d de la class '%s'",_B,(const char *)((NLAIAGENT::IObjectIA *)(context.Self))->getType());		
	}


	NLAIAGENT::TProcessStatement CLdbStackMemberiOpCode::runOpCode(CCodeContext &context)
	{
		NLAIAGENT::IObjectIA *a = ((NLAIAGENT::IObjectIA *)context.Stack);
		NLAIAGENT::IObjectIA *obj = a; 
		std::list<sint32>::iterator i = _I.begin();
		sint32 n = _I.size() - 1;
		while(n --)
		{
			a = (NLAIAGENT::IObjectIA *)a->getStaticMember(*i++);
		}
		context.Stack[(int)context.Stack] = (NLAIAGENT::IObjectIA *)a->getStaticMember(*i)->clone();
		obj->release();
		return NLAIAGENT::processIdle;
	}

	void CLdbStackMemberiOpCode::getDebugResult(char *str,CCodeContext &context) const
	{	
		NLAIAGENT::IObjectIA *a = ((NLAIAGENT::IObjectIA *)context.Stack);		
		std::list<sint32>::const_iterator i = _I.begin();
		sint32 n = _I.size() - 1;
		while(n --)
		{
			a = (NLAIAGENT::IObjectIA *)a->getStaticMember(*i++);
		}
		char txt[1024*8];
		a->getStaticMember(*i)->getDebugString(txt);
		sprintf(str,"ldb %s le composan membre sur la pile de la class '%s'",txt,(const char *)a->getType());
	}

	NLAIAGENT::TProcessStatement CLdbHeapMemberiOpCode::runOpCode(CCodeContext &context)
	{
		NLAIAGENT::IObjectIA *a = (NLAIAGENT::IObjectIA *)context.Heap[(int)_N];
		std::list<sint32>::iterator i = _I.begin();
		sint32 n = _I.size() - 1;
		while(n --)
		{
			a = (NLAIAGENT::IObjectIA *)a->getStaticMember(*i++);
		}
		a = (NLAIAGENT::IObjectIA *)a->getStaticMember(*i)->clone();
		context.Stack ++;
		context.Stack[(int)context.Stack] = a;
		return NLAIAGENT::processIdle;
	}

	void CLdbHeapMemberiOpCode::getDebugResult(char *str,CCodeContext &context) const
	{						
		NLAIAGENT::IObjectIA *a = (NLAIAGENT::IObjectIA *)context.Heap[(int)_N];
		std::list<sint32>::const_iterator i = _I.begin();
		sint32 n = _I.size() - 1;
		while(n --)
		{
			a = (NLAIAGENT::IObjectIA *)a->getStaticMember(*i++);
		}
		char txt[1024*8];
		a->getStaticMember(*i)->getDebugString(txt);
		sprintf(str,"ldb %s le composant membre sur le heap de la class '%s'",txt,(const char *)a->getType());		
	}

	NLAIAGENT::TProcessStatement CLdbMemberiOpCode::runOpCode(CCodeContext &context)
	{
		context.Stack ++;
		NLAIAGENT::IObjectIA *obj = (NLAIAGENT::IObjectIA *)context.Self;
		std::list<sint32>::iterator i = _I.begin();
		while(i != _I.end())
		{
			obj = (NLAIAGENT::IObjectIA *)obj->getStaticMember(*i++);
		}
		context.Stack[(int)context.Stack] = (NLAIAGENT::IObjectIA *)obj->clone();
		
		return NLAIAGENT::IObjectIA::ProcessIdle;
	}

	void CLdbMemberiOpCode::getDebugResult(char *str,CCodeContext &context) const
	{			
		NLAIAGENT::IObjectIA *obj = (NLAIAGENT::IObjectIA *)context.Self;
		std::list<sint32>::const_iterator i = _I.begin();
		int j;
		while(i != _I.end())
		{
			j = *i++;
			obj = (NLAIAGENT::IObjectIA *)obj->getStaticMember(j);
		}			
					
		sprintf(str,"ldb le composant membre <%d> member de la class '%s'",j,(const char *)obj->getType());		
	}

	NLAIAGENT::TProcessStatement CLdbRefOpCode::runOpCode(CCodeContext &context)
	{
		context.Stack ++;
		context.Stack[(int)context.Stack] = (NLAIAGENT::IObjectIA *)context.Heap[_B]->clone();
		return NLAIAGENT::IObjectIA::ProcessIdle;;
	}

	void CLdbRefOpCode::getDebugResult(char *str,CCodeContext &context) const
	{
		char X[1024*8];	
		context.Heap[_B]->getDebugString(X);
		sprintf(str,"ldb<%d>",_B);		
	}
}
