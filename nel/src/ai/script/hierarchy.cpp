/** \file hierarchy.cpp
 *
 * $Id: hierarchy.cpp,v 1.10 2002/08/21 13:58:33 lecroart Exp $
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
#include "nel/ai/script/constraint.h"

namespace NLAISCRIPT
{	
	const IClassInterpret *CCompilateur::getInheritanceRange(sint32 &h,const IClassInterpret *classType,NLAIAGENT::IBaseGroupType &listName) const
	{		

		NLAIAGENT::CIteratorContener it = listName.getIterator();
		sint32 i = listName.size() - 1;
		const IClassInterpret *bassClass = NULL;		
		while(i --)
		{
			const NLAIAGENT::IVarName &className = ((const NLAIAGENT::CStringType *)it++)->getStr();			
			bool find = false;
			for(sint32 k = classType->sizeVTable() - 1; k >= 0 ;k--)
			{
				bassClass = classType->getInheritance(k);
				h = k;
				if(*bassClass->getClassName() == className)
				{
					/*h  = -1;
					return NULL;*/
					find = true;
					break;
				}
				
			}
			if(!find)
			{
				h  = -1;
				return NULL;
			}
		}		
		return bassClass;
	}

	const IClassInterpret *CCompilateur::getInheritanceRange(sint32 &h,const IClassInterpret *classType,const NLAIAGENT::IVarName &className ) const
	{				
		const IClassInterpret *bassClass = NULL;
		bool find = false;
		for(sint32 k = classType->sizeVTable() - 1; k > 0 ;k--)
		{
			bassClass = classType->getInheritance(k);
			h = k;
			if(*bassClass->getClassName() == className)
			{
				/*h  = -1;
				return NULL;*/
				find = true;
				return bassClass;
			}
			
		}
		if(!find)
		{
			h  = -1;
			return NULL;
		}
		
		return NULL;
	}

	/*sint32 CCompilateur::getCompementShift(IClassInterpret *classType)
	{
		sint32 i = 0;		
		classType = (IClassInterpret *)classType->getBaseClass();
		while(classType != NULL)
		{
			i += classType->getStaticMemberSize();
			classType = (IClassInterpret *)classType->getBaseClass();
		}
		return 0;
	}*/

	const NLAIAGENT::IObjectIA *CCompilateur::getValidateHierarchyBase(std::list<sint32> &ref,sint32 &h,std::list<NLAISCRIPT::CStringType> &listName)
	{
		return getValidateHierarchyBase((IClassInterpret *)_SelfClass.get(),ref,h,listName);
	}

	const NLAIAGENT::IObjectIA *CCompilateur::getValidateHierarchyBase(const NLAIAGENT::IObjectIA *base,std::list<sint32> &ref,sint32 &h,std::list<NLAISCRIPT::CStringType> &listName)
	{		
		std::list<NLAISCRIPT::CStringType> classHName;
		std::list<NLAISCRIPT::CStringType> varHName;
		const NLAIAGENT::IObjectIA *baseClass = base;		

		ref.clear();
		
		h = 0;
		NLAIAGENT::CStringVarName baseClassName("");
		
		while(listName.size())
		{
			NLAIAGENT::CStringVarName lastClassName(listName.back().data());
			listName.pop_back();			
			try
			{
				NLAIC::CIdentType id = getTypeOfClass(lastClassName);				
				if(baseClass->isClassInheritedFrom(NLAIAGENT::CStringVarName(lastClassName.getString()))>=0)
				{
					classHName.push_front(lastClassName.getString());
				}							
				else
				{
					/*char text[4096*4];
					sprintf(text,"you try to access to '%s' and this class is not define in the hierarchy of '%s' class",lastClassName.getString(),(const char *)baseClass->getType());
					yyerror(text);*/
					return NULL;
				}
			}
			catch(NLAIE::IException &)
			{
				while(classHName.size())
				{
					varHName.push_front(classHName.front());
					classHName.pop_front();
				}
				varHName.push_front(lastClassName.getString());					
			}			
		}

		if(classHName.size())
		{
			std::list<NLAISCRIPT::CStringType>::iterator i = classHName.begin();			
			while(i != classHName.end())
			{
				NLAIAGENT::CStringVarName str( NLAIAGENT::CStringVarName((*i++).data()));
				try
				{
					NLAIC::CIdentType id(str.getString());
					const NLAIAGENT::IObjectIA *b = (const NLAIAGENT::IObjectIA *)(id.getFactory())->getClass();					
					
					if((h = baseClass->isClassInheritedFrom(str)) >= 0)
					{						
						baseClass = b;
					}
				}				
				catch(NLAIE::IException &)
				{
					std::string text;
					text = NLAIC::stringGetBuild("you try to access to '%s' and this class is not define in the hierarchy of '%s' class",str.getString(),(const char *)baseClass->getType());
					yyerror((char *)text.c_str());
					return NULL;
				}
			}	

		}				
		if(varHName.size())
		{
			sint32 i;
			NLAIAGENT::CStringVarName oldLastClassName("");

			while(varHName.size())
			{
			
				NLAIAGENT::CStringVarName lastClassName(varHName.front().data());
				varHName.pop_front();
				
				i = baseClass->getStaticMemberIndex(lastClassName);
				if(i>=0)
				{														
					const NLAIAGENT::IObjectIA *c = baseClass->getStaticMember(i);
					/*if((const NLAIC::CTypeOfObject &)baseClass->getType() & NLAIC::CTypeOfObject::tAgentInterpret) 
					{
						i += getCompementShift((IClassInterpret *)baseClass);
					}*/
					ref.push_back(i);					
					h = 0;					
					if(c != NULL)
					{
						baseClass = c;
					}
					else
					{
						std::string text;
						text = NLAIC::stringGetBuild("you try to access to an indefined object in the '%s' class",(const char *)baseClass->getType());
						yyerror((char *)text.c_str());
						return NULL;

					}
										
				}
				else
				{
					if((const NLAIC::CTypeOfObject &)baseClass->getType() & NLAIC::CTypeOfObject::tInterpret) 
					{
						baseClass = getInheritanceRange(h,(const IClassInterpret *)baseClass,lastClassName);	
					}
					else
					{
						h = baseClass->isClassInheritedFrom(lastClassName);
					}
					
					if(baseClass == NULL)
					{
						std::string text;
						text = NLAIC::stringGetBuild("you try to access to '%s' and this class is not define in the hierarchy of '%s' class",lastClassName.getString(),oldLastClassName.getString());
						yyerror((char *)text.c_str());
						return NULL;
					}					
				}				
			}			
		}
		return baseClass;
	}
}
