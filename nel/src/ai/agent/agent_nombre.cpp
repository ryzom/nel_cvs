/** \file agent_nombre.cpp
 *
 * $Id: agent_nombre.cpp,v 1.7 2003/01/21 11:24:39 chafik Exp $
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
#include "nel/ai/agent/agent_nombre.h"
#include "nel/ai/agent/object_type.h"
#include "nel/ai/script/interpret_methodes.h"
#include "nel/ai/agent/agent_method_def.h"

namespace NLAIAGENT
{	

	INombreDefine::CMethodCall INombreDefine::StaticMethod[] = 
	{
		INombreDefine::CMethodCall(_CONSTRUCTOR_,TConst)		
	};
	
	sint32 INombreDefine::getMethodIndexSize() const
	{
		return IObjetOp::getMethodIndexSize() + TLastM;
	}
	
	TQueue INombreDefine::isMember(const IVarName *className,const IVarName *methodName,const IObjectIA &p) const
	{
		TQueue a;
		NLAISCRIPT::CParam methodParam;
		
		if(className == NULL)
		{
			for(int i = 0; i < TLastM; i++)
			{
				if(*methodName == INombreDefine::StaticMethod[i].MethodName)
				{					
					CObjectType *c = new CObjectType(new NLAIC::CIdentType(NLAIC::CIdentType::VoidType));					
					a.push(CIdMethod(INombreDefine::StaticMethod[i].Index + IObjetOp::getMethodIndexSize(),0.0,NULL,c));					
					break;
				}
			}
		}
		return a;
	}
	
	sint32 INombreDefine::isClassInheritedFrom(const IVarName &) const
	{
		return -1;
	}
}
