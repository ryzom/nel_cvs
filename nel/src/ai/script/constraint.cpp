/** \file constraint.cpp
 *
 * $Id: constraint.cpp,v 1.17 2002/08/21 13:58:33 lecroart Exp $
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
#include "nel/ai/script/type_def.h"
#include "nel/ai/script/object_unknown.h"


namespace NLAISCRIPT
{
	void IConstraint::popConstraint(IConstraint *c)
	{
		std::list<IConstraint *>::iterator it = _ConstraintList.begin();
		while(it != _ConstraintList.end())
		{			
			if(*it == c)
			{
				_ConstraintList.erase(it);
				break;
			}
			it ++ ;
		}		
	}
}


namespace NLAISCRIPT
{	
	void CConstraintMethode::run(CCompilateur &comp)
	{
		if(_Satisfied) return;
#ifdef NL_DEBUG
		std::string mtxt;
		std::string ptxt;
		std::string txt;
		
		_MethodName->getDebugString(mtxt);

		/*if(mtxt == "[Get]")
		{
			throw;
		}*/
		_Param->getDebugString(ptxt);
		txt = mtxt + ptxt;
#endif	
		const NLAIAGENT::IObjectIA *cl = NULL;
		bool isMember = true;
		
		if(_BaseClass)
		{
			if(!_BaseClass->satisfied()) _BaseClass->run(comp);
			if(_BaseClass->satisfied())
			{
				cl = (const NLAIAGENT::IObjectIA *)(_BaseClass->getConstraintTypeOf()->getFactory())->getClass();
				_M = comp.findMethode(cl,*_MethodName,*_Param);
			}
			else
			{	
				std::string txt;
				std::string param;
				std::string Method;
				
				_Param->getDebugString(param);
				_MethodName->getDebugString(Method);
				txt = NLAIC::stringGetBuild("can't find the method '%s%s'",Method.c_str(),param.c_str());
				_Txt = new char [strlen(txt.c_str()) + 1];
				strcpy(_Txt,txt.c_str());
				return;
			}			
		}
		else
		{
			try
			{
				_M = comp.findMethode(*_MethodName,*_Param);
			}
			catch(NLAIE::IException &e)
			{
				std::string txt;
				std::string param;
				std::string Method;
				
				_Param->getDebugString(param);
				_MethodName->getDebugString(Method);
				txt += NLAIC::stringGetBuild("error '%s' when serch '%s%s'",(char *) e.what(),Method.c_str(),param.c_str());
				_Txt = new char [strlen(txt.c_str()) + 1];
				strcpy(_Txt,txt.c_str());
				return;

			}
			if(_M.MethodName == NULL)
			{
				isMember = false;
				NLAIAGENT::CStringType *lib = (NLAIAGENT::CStringType *)_MethodName->getFront();
				try
				{					
					NLAIC::CIdentType id(lib->getStr().getString());
					_M = comp.findMethode((NLAIAGENT::IObjectIA *)id.getFactory()->getClass() ,*_MethodName,*_Param);

				}
				catch(NLAIE::IException &)
				{
					std::string txt;
					std::string param;
					std::string Method;
					
					_Param->getDebugString(param);
					_MethodName->getDebugString(Method);
					txt += NLAIC::stringGetBuild("can't find the method '%s%s'",Method.c_str(),param.c_str());
					_Txt = new char [strlen(txt.c_str()) + 1];
					strcpy(_Txt,txt.c_str());
					
				}
			}
		}		 
		if(_M.MethodName == NULL)
		{			

			std::string txt;
			std::string param;
			std::string Method;
			
			_Param->getDebugString(param);
			_MethodName->getDebugString(Method);
			txt += NLAIC::stringGetBuild("can't find the method '%s%s'",Method.c_str(),param.c_str());
			_Txt = new char [strlen(txt.c_str()) + 1];
			strcpy(_Txt,txt.c_str());
		}
		else
		{
			_Satisfied = true;
			IOpCode *x=0;
						

			if(((const NLAIC::CTypeOfObject &)_M.Object->getType()) & NLAIC::CTypeOfObject::tInterpret)
			{				
				IMethodContext *methodContex;
				if (comp.getDebugMode())
				{
					methodContex = new CMethodContextDebug();
				}
				else
				{
					methodContex = new CMethodContext();
				}
				
				switch(_CallType)
				{
					case normalCall:
						if(!_M.Member.size())
						{
							x = new CCallMethod(methodContex,_M.Inheritance,_M.MethodNum);
						}
						else
						{
							x = new CCallMethodi(methodContex,_M.Inheritance,_M.MethodNum,_M.Member);
						}
						break;
					case stackCall:
						x = new CCallStackMethodi(methodContex,_M.Inheritance,_M.MethodNum,_M.Member);
						break;
					case heapCall:
						x = new CCallHeapMethodi(methodContex,_M.Inheritance,_M.MethodNum,_PosHeap,_M.Member);
						break;
					case newCall:
						x = new CCallStackNewMethodi(methodContex,_M.Inheritance,_M.MethodNum,_M.Member);
						break;
					case searchCall:
						x = NULL;
						delete methodContex;
						break;
				}
			}
			else
			{
				_Satisfied = true;
				switch(_CallType)
				{
				case normalCall:
					if(_M.Member.size())
					{						
						if(!isMember) x =  new CLibCallMethodi(_M.Inheritance,_M.MethodNum ,_M.Member , *_M.Object);
						else
						{
							x =  new CLibMemberMethodi(_M.Inheritance,_M.MethodNum ,_M.Member );
						}
					}
					else
					{
						if(_M.Inheritance)
						{
							if(!isMember) x =  new CLibCallInheritedMethod(_M.Inheritance,_M.MethodNum , *_M.Object);
							else
							{
								x =  new CLibMemberInheritedMethod(_M.Inheritance,_M.MethodNum);
							}
						}
						else
						{
							if(!isMember) x =  new CLibCallMethod(_M.MethodNum ,*_M.Object);
							else
							{								
								x =  new CLibMemberMethod(_M.MethodNum);
							}
						}
					}
					break;
				case stackCall:
					x =  new CLibStackMemberMethod(_M.Inheritance,_M.MethodNum ,_M.Member);
					break;
				case heapCall:
					x =  new CLibHeapMemberMethod(_M.Inheritance,_M.MethodNum ,_M.Member,_PosHeap);
					break;
				case newCall:					
					x =  new CLibStackNewMemberMethod(_M.Inheritance,_M.MethodNum ,_M.Member);					
					break;

				case searchCall:
					x = NULL;
					break;
				}
			}			
			IConstraint *c;			

			if(_M.Method != NULL)
			{
				c = (IConstraint *)_M.Method->getTypeOfMethode();
				setOpCode(comp,x,c,false);
				if(_M.ReturnType != NULL)
				{
					_M.ReturnType->release();
					_M.ReturnType = NULL;
				}				
			}
			else
			{
				if(_M.ReturnType != NULL)
				{
					try
					{
						NLAIC::CIdentType type = _M.ReturnType->getType();
						NLAIC::CIdentType *tmp = new NLAIC::CIdentType (type);
						c = new COperandSimple ( tmp );
						setOpCode(comp,x,c,true);						
					}
					catch(NLAIE::IException &)
					{	
						c = new COperandSimple (new NLAIC::CIdentType (_M.Object->getType()));
						setOpCode(comp,x,c,true);
					}
					_M.ReturnType->release();
					_M.ReturnType = NULL;
				}
				else
				{
					c = new COperandSimple (new NLAIC::CIdentType (_M.Object->getType()));
					setOpCode(comp,x,c,true);
				}				
			}
			
		}		
	}

	void CConstraintMethode::setOpCode(CCompilateur &comp,IOpCode *x,IConstraint *type,bool f)
	{
		if(x != NULL)
		{		
			if(_Code.size())
			{		
				while(_Code.size())
				{			
					std::pair<int, CCodeBrancheRun *> &p = _Code.back();
					IOpCode *op = (*p.second)[p.first];//
/*#ifdef NL_DEBUG
					std::string dbugS;
					p.second->getDebugString(dbugS);
					NLAIC::Out("%s \n\tindex <%d> \n\tRef of IOpCode int setOpCode is %d\n", dbugS.c_str(), p.first, op->getRef());
#endif*/
					op->release();
					(*p.second)[p.first] = x;
					_Code.pop_back();			
					if(_Code.size()) x->incRef();
				}
			}
			else
			{
				x->release();
			}
		}
		_Type = type;
		_Type->run(comp);
		_DelCType = f;
		runConnexcion();
	}

	void CConstraintMethode::getError(char *txt) const
	{			
		strcpy(txt,_Txt);
	}

	bool CConstraintMethode::isEqual (const NLAIAGENT::IBaseGroupType &g,const CParam &p) const
	{
		if(((const NLAIAGENT::IBasicObjectIA &)*_MethodName) == ((const NLAIAGENT::IBasicObjectIA &)g) && *_Param == p) return true;
		return false;
	}

	void CConstraintMethode::run(IConstraint *)
	{
	}	

	const IConstraint *CConstraintMethode::clone() const
	{
		IConstraint *x = new CConstraintMethode(_CallType,_PosHeap,_BaseClass,_MethodName,_Param,_Lin,_Col);
		_BaseClass->incRef();
		_MethodName->incRef();
		_Param->incRef();		
		return x;
	}	
}
