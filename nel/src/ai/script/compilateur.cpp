/** \file compilateur.cpp
 *
 * $Id: compilateur.cpp,v 1.19 2002/06/27 16:58:09 chafik Exp $
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
#include "nel/ai/script/interpret_object_agent.h"
#include "nel/ai/script/type_def.h"
#include "nel/ai/script/object_unknown.h"
#include <queue>
#include <stdarg.h>

namespace NLAISCRIPT
{	
	/*struct BestP
	{
		sint32 i;
		double poid;

		BestP(sint32 n, double d)
		{
			i = n;
			poid = d;
		}

		bool operator < (const BestP &p) const
		{
			return poid < p.poid;
		}

	};*/

	void CCompilateur::onEndClass()
	{
		((NLAIAGENT::IObjectIA *)_SelfClass.pop())->release();
	}

	NLAIC::CIdentType CCompilateur::getTypeOfClass(const NLAIAGENT::IVarName &className)/// throw (NLAIE::IException)
	{
#ifdef NL_DEBUG
		const char *nameDB = className.getString();
#endif
		try
		{				
			NLAIC::CIdentType id(className.getString());
			return id;
		}
		catch (NLAIE::IException &err)
		{				
			IClassInterpret *cl= _SelfClass.find(&className);								
			if(cl == NULL) 
			{
				throw CExceptionHaveNoType(err.what());
			}
			else
			{
				return getTypeOfClass(*cl->getInheritanceName());
			}				
		}
	}

	void CCompilateur::Echo(char *Er,...)
	{
		std::string Temp;
		va_list marker;
		va_start (marker, Er);
		char LaseErrorCodeOrdreInterprete[32*1024];
		vsprintf (LaseErrorCodeOrdreInterprete, Er, marker);
		_Iterface.Echo("%s",LaseErrorCodeOrdreInterprete);
	}


	NLAIAGENT::IObjectIA::CProcessResult CCompilateur::Compile()
	{
		NLAISCRIPT::InitDico();
		int k = yywrap();
		yyLine = yyColone = 1;
		_Error = false;
		
		sint32 i = yyparse();
		while(yylex());
		
		NLAISCRIPT::EraseDico();

		NLAIAGENT::IObjectIA::CProcessResult r;
		if(_Error)
		{
			r.ResultState = NLAIAGENT::processError;
		}
		else r.ResultState = NLAIAGENT::processIdle;
		r.Result = _ResultCompile;
		return r;
	}
	
	NLAIAGENT::CIdMethod CCompilateur::findMethode(sint32 inheritance,NLAIAGENT::CStringType *baseName, const NLAIAGENT::IObjectIA *classType,const NLAIAGENT::IVarName &methodeName,const CParam &param)
	{

		NLAIAGENT::tQueue q;
		if(baseName != NULL && baseName->getStr() == NLAIAGENT::CStringVarName((const char *)CAgentClass::IdAgentClass))
		{
			 q = classType->isMember(&baseName->getStr(),&methodeName,param);
		}
		else
		{
			/*std::string s;
			methodeName.getDebugString(s);			
			param.getDebugString(s);						

			s += "  ";
			classType->getDebugString(s);

			NLAIC::Out("%s\n",s.c_str());*/
			q = classType->isMember(NULL,&methodeName,param);
		}

		if(q.size())
		{
			return q.top();			
		}
		else
		{						
			return NLAIAGENT::CIdMethod(-1,0.0,NULL,NULL);
		}
	}

	CFunctionTag CCompilateur::findMethode(NLAIAGENT::IBaseGroupType &listName,const CParam &param)
	{
		return findMethode((IClassInterpret *)_SelfClass.get(),listName,param);
	}

	CFunctionTag CCompilateur::findMethode(const NLAIAGENT::IObjectIA *classType,NLAIAGENT::IBaseGroupType &listName,const CParam &param)
	{						
		CFunctionTag method;
		if(!listName.size())
		{			
			method.Inheritance = -1;
			method.MethodNum = -1;
			method.MethodName = NULL;
			return method;
		}

		sint32 h = listName.size() - 1;

		NLAIAGENT::CIdMethod r;
		
		if(!h)
		{
			method.Inheritance = 0;//classType->sizeVTable() - 1;
			method.MethodName = &((const NLAIAGENT::CStringType *)listName.get())->getStr();

			r = findMethode(method.Inheritance,NULL,classType, *method.MethodName,param);
			if( r.Index <0)
			{
				method.MethodName = NULL;
				method.Inheritance = -1;
				method.MethodNum = -1;
				
			}
			else
			{
				method.MethodNum = r.Index;
				method.Object = classType;
				method.setTypeObject(r.ReturnType);				
				method.Method = (CMethodeName *)r.Method;				
			}
			
		}
		else
		{	
			NLAIAGENT::CStringType *baseName = (NLAIAGENT::CStringType *)listName.getFront()->clone();			

			if((classType = validateHierarchyMethode(method.Member,method.Inheritance,classType,listName)) != NULL)
			{							
				if(method.Inheritance >= 0)
				{			
					method.MethodName = &((const NLAIAGENT::CStringType *)listName.get())->getStr();

					r = findMethode(method.Inheritance,baseName,classType,*method.MethodName,param);
					if(  r.Index < 0)
					{
						method.MethodName = NULL;
						method.Inheritance = -1;
						method.MethodNum = -1;
					}
					else
					{

						method.Object = classType;
						method.MethodNum = r.Index;
						method.Method = (CMethodeName *)r.Method;
						method.setTypeObject(r.ReturnType);
				
					}
				}
				else
				{
					method.MethodName = NULL;
					method.Inheritance = -1;
					method.MethodNum = -1;
				}
			}
			else
			{
				
				method.MethodName = NULL;
				method.Inheritance = -1;
				method.MethodNum = -1;
				
			}
			baseName->release();
		}	
		
		return method;
	}

	const NLAIAGENT::IObjectIA *CCompilateur::validateHierarchyMethode(std::list<sint32> &listH,sint32 &h,const NLAIAGENT::IObjectIA *classType,NLAIAGENT::IBaseGroupType &listName)
	{
		std::list<NLAISCRIPT::CStringType> listClassName;		
			
		NLAIAGENT::CIteratorContener it = listName.getIterator();						

		while(!it.isInEnd())
		{
			NLAIAGENT::CStringType &name = *((NLAIAGENT::CStringType *)it++);
			listClassName.push_back(name.getStr().getString());			
		}		
		listClassName.pop_back();		
		return getValidateHierarchyBase(classType,listH,h,listClassName);				
	}

	sint32 CCompilateur::runTypeConstraint()
	{
		sint32 n = 1;	
		sint32 did = _ConstraintType.size();
		std::list<IConstraint *>::iterator j,i;		
		while(n != 0)
		{
			i = _ConstraintType.begin();
			n = 0;
			while(i != _ConstraintType.end())
			{
				j = i ++;
				IConstraint *c = *j;
				c->run(*this);
				if( c->satisfied() ) 
				{				
					_ConstraintType.erase(j);
					n++;
					c->release();
				}			
			}		
			did -= n;
		}
		return did;
	}

	sint32 CCompilateur::runMethodConstraint()
	{			


		sint32 n = 1;
		sint32 did = 0;
		while(n)
		{		
			std::list<IConstraint *>::iterator i = _MethodConstraint.begin();
			n = 0;

			while(i != _MethodConstraint.end())
			{			
				IConstraint *c = *i++;				
				if(!c->satisfied())
				{
					c->run(*this);
					if(c->satisfied()) n++;
				}				
			}			
			if(runTypeConstraint()) errorTypeConstraint();
		}
		
		
		std::list<IConstraint *>::iterator j,i = _MethodConstraint.begin();		
		n = 0;
		while(i != _MethodConstraint.end())
		{			
			j = i;
			i ++;
			IConstraint *c = *j;			
			if( c->satisfied() ) 
			{				
				_MethodConstraint.erase(j);
				n++;
				c->release();
			}
		}		
		return n;
	}
	void CCompilateur::errorTypeConstraint()
	{
		std::list<IConstraint *>::iterator j,i = _ConstraintType.begin();
		while(i != _ConstraintType.end())
		{
			j = i ++;
			IConstraint *c = *j;			
			if( !c->satisfied() ) 
			{				
				_ConstraintType.erase(j);
				//yyerror((char *)c->getInfo());
				c->release();				
			}			
		}
	}
	void CCompilateur::errorMethodConstraint()
	{
		char txt[1024*16];

		std::list<IConstraint *>::iterator j,i = _MethodConstraint.begin();		
		while(i != _MethodConstraint.end())
		{
			j = i ++;
			IConstraint *c = *j;			
			c->getError(txt);			
			yyLine = c->getLine();
			yyColone = c->getColone();
			yyerror(txt);
			if(_ConstraintType.begin() != _ConstraintType.end()) _ConstraintType.erase(j);
			c->release();
		}
	}
	
	void CCompilateur::cleanTypeList()
	{
		while(_TypeList.size())
		{
			_TypeList.back()->release();
			_TypeList.pop_back();
		}
	}
	void CCompilateur::clean()
	{
		sint32 i;
		i = _LastBloc1.size();
		while(i --)
		{	
			if(_LastBloc != _LastBloc1.back()) _LastBloc1.back()->release();
			_LastBloc1.pop_back();
		}
		i = _LastBloc2.size();
		while(i --)
		{	
			if(_LastBloc != _LastBloc2.back()) _LastBloc2.back()->release();
			_LastBloc2.pop_back();
		}		

		i = _LastBloc3.size();
		while(i --)
		{	
			if(_LastBloc != _LastBloc3.back()) _LastBloc3.back()->release();
			_LastBloc3.pop_back();
		}		

		//sint32 i = _LastSaveBloc.size();
		i = _LastSaveBloc.size();
		while(i --)
		{	
			IBlock *k = _LastSaveBloc.back();				
			if(_LastBloc != k) k->release();
			_LastSaveBloc.pop_back();
		}
		if(_LastBloc !=NULL) _LastBloc->release();
		_LastBloc = NULL;			
		while(_Attrib.size())
		{	
			_Attrib.back().first->release();
			_Attrib.back().second->release();
			_Attrib.pop_back();
		}				
		
		while(_Param.size())
		{
			_Param.back()->release();
			_Param.pop_back();
		}

		while(_LastStringParam.size())
		{
			_LastStringParam.back()->release();
			_LastStringParam.pop_back();
		}		
		

		if(_ExpressionType != NULL) 
		{
			_ExpressionType->release();
			_ExpressionType = NULL;
		}
		if(_FlotingExpressionType != NULL)
		{
			_FlotingExpressionType->release();
			_FlotingExpressionType = NULL;
		}

		cleanTypeList();		
	}

	void CCompilateur::cleanMethodConstraint()
	{
		while(_MethodConstraint.size())
		{	
			IConstraint *c = _MethodConstraint.back();
			//c->release();
			_MethodConstraint.pop_back();
		}
	}

	void CCompilateur::cleanTypeConstraint()
	{
		while(_ConstraintType.size())
		{	
			_ConstraintType.back()->release();
			_ConstraintType.pop_back();
		}
	}

	IConstraint *CCompilateur::getMethodConstraint(const NLAIAGENT::IBaseGroupType &g,const CParam &p) const
	{
		std::list<IConstraint *>::const_iterator i = _MethodConstraint.begin();
		while(i != _MethodConstraint.end())
		{
			if((*i)->getTypeOfClass() == CConstraintMethode::constraintMethode)
			{
				const CConstraintMethode *o = (const CConstraintMethode *)*i;
				if(o->isEqual(g,p)) return *i;
			}			
			i++;
		}
		return false;
	}

	IConstraint *CCompilateur::getMethodConstraint(const IConstraint&c) const
	{
		std::list<IConstraint *>::const_iterator i = _MethodConstraint.begin();
		while(i != _MethodConstraint.end())
		{			
			const IConstraint *o = *i;
			if(*o == c) return *i;			
			i++;
		}
		return false;
	}
		
	void CCompilateur::setNewLine()
	{
		if (_Debug)
		{
			if (_LastBloc != NULL)
			{
				_LastBloc->setCurrentLine(yyLine);
				_LastBloc->setFirstOpCodeInLine(true);
			}
		}
	}

	void CCompilateur::addOpCode(IOpCode *x)
	{

		//CLdbOpCode *x = new CLdbOpCode (NLAIAGENT::CGroupType());
		_LastBloc->addCode(x);
	}
}
