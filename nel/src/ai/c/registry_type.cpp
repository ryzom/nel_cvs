/** \file registry_type.cpp
 *
 * $Id: registry_type.cpp,v 1.8 2003/01/21 11:24:39 chafik Exp $
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
#include <iostream.h>
#include <stdio.h>
#include <stdarg.h>

#include "nel/ai/c/abstract_interface.h" 
#include "nel/ai/e/ai_exception.h" 
#include "nel/ai/c/registry_class.h"
#include "nel/ai/c/registry_type.h"


namespace NLAIC
{
	CIdentType CTypeOfObject::IdTypeOfObject("CTypeOfObject",CSelfClassFactory(CTypeOfObject()),CTypeOfObject(0),CTypeOfOperator(0));
	const CIdentType &CTypeOfObject::getType() const
	{
		return IdTypeOfObject;
	}	

	void CTypeOfObject::getDebugString(char *txt) const
	{		 
		if((uint32)*this)
		{		
			txt[0] = 0;
			const char *opTxt[9] =	{"tNombre","TString","tLogic","tList","tAdresse","tAgent","tExec","tObject","tAgentInterpret"};
			sint32 digi = 1;	
			sint32 last = false;
			for(sint32 i = 0; i< 9; i ++)
			{
				if(*this & digi)
				{
					if(last) strcat(txt," | ");
					strcat(txt,opTxt[i]);
					last = true;
				}
			}
		}
		else
		{
			strcpy(txt,"non definition of object");
		}
	}



	CIdentType CTypeOfOperator::IdTypeOfOperator("CTypeOfOperator",CSelfClassFactory(CTypeOfOperator()),CTypeOfObject(0),CTypeOfOperator(0));
	const CIdentType &CTypeOfOperator::getType() const
	{
		return IdTypeOfOperator;
	}
	
	void CTypeOfOperator::getDebugString(char *txt) const
	{
		if((uint32)*this)
		{		
			txt[0] = 0;
			const char *opTxt[12] =	{"opAdd","opSub","opMul","opDiv","opEq","opInf","opSup","opInfEq","opSupEq","opNot","opDiff","opNeg"};
			sint32 digi = 1;	
			sint32 last = false;
			for(sint32 i = 0; i< 12; i ++)
			{
				if(*this & digi)
				{
					if(last) strcat(txt," | ");
					strcat(txt,opTxt[i]);
					last = true;
				}
			}
		}
		else
		{
			strcpy(txt,"non definition of operation in the object");
		}			
	}
}

