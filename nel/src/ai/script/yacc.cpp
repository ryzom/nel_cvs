/** \file yacc.cpp
 *
 * $Id: yacc.cpp,v 1.5 2001/01/12 09:52:56 chafik Exp $
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
#include "nel/ai/script/constraint_stack_component.h"
#include "nel/ai/script/constraint_find_run.h"
#include "nel/ai/agent/main_agent_script.h"
#include "nel/ai/script/interpret_object_manager.h"

namespace NLAISCRIPT
{		
	void CCompilateur::interrogationEnd()
	{		
		_VarState -= (sint32)_VarState  - _VarState.popMark();
		_LastBloc1.back()->addCode((new CFreeAlloc));
		if (_Debug)
		{
			_LastBloc1.back()->addCode((new CFreeAllocDebug));
		}
		else
		{
			_LastBloc1.back()->addCode((new CFreeAlloc));
		}
		sint32 i =_LastBloc1.back()->listCode().size();
		_LastBloc = _LastSaveBloc.back();
		_LastBloc->release();
		_LastBloc->addCode(new CJFalseOpCode (i));
		while(i --)
		{									
			_LastBloc->pushCode(_LastBloc1.back()->getFront());
			CBagOfCode *b = _LastBloc1.back()->listCode().front();
			(_LastBloc->getBagOfCode())->addConstraint(b);
			delete b;
			_LastBloc1.back()->listCode().pop_front();
		}
		_LastBloc1.back()->release();
		_LastBloc1.pop_back();
	}

	void CCompilateur::ifInterrogation()
	{
		_LastSaveBloc.push_back(_LastBloc);
		_LastSaveBloc.back()->incRef();
		_LastBloc1.push_back(new IBlock(_Debug));
		_LastBloc1.back()->incRef();
		_LastBloc = _LastBloc1.back();
		_VarState.pushMark();
		_LastBloc->addCode((new CMarkAlloc));
		if(_ExpressionType)
		{
			_ExpressionType->release();
			_ExpressionType = NULL;
		}		
	}

	void CCompilateur::ifInterrogationPoint()
	{
		_VarState -= (sint32)_VarState  - _VarState.popMark();		
		if (_Debug)
		{
			_LastBloc1.back()->addCode((new CFreeAllocDebug));
		}
		else
		{
			_LastBloc1.back()->addCode((new CFreeAlloc));
		}
		_LastBloc2.push_back(new IBlock(_Debug));
		_LastBloc2.back()->incRef();
		_LastBloc = _LastBloc2.back();
		_VarState.pushMark();
		_LastBloc->addCode((new CMarkAlloc));
	}

	void CCompilateur::ifInterrogationEnd()
	{
		_VarState -= (sint32)_VarState  - _VarState.popMark();
		
		if (_Debug)
		{
			_LastBloc2.back()->addCode((new CFreeAllocDebug));
		}
		else
		{
			_LastBloc2.back()->addCode((new CFreeAlloc));
		}
		_LastBloc = _LastSaveBloc.back();
		_LastBloc->release();
		_LastSaveBloc.pop_back();
		sint32 i =_LastBloc1.back()->listCode().size();
		_LastBloc->addCode(new CJFalseOpCode (i + 1));
		while(i --)
		{
			_LastBloc->pushCode(_LastBloc1.back()->getFront());
			CBagOfCode *b = _LastBloc1.back()->listCode().front();
			(_LastBloc->getBagOfCode())->addConstraint(b);
			delete b;
			_LastBloc1.back()->listCode().pop_front();
		}
		i =_LastBloc2.back()->listCode().size();
		_LastBloc->addCode(new CJmpOpCode (i));
		while(i --)
		{
			_LastBloc->pushCode(_LastBloc2.back()->getFront());
			CBagOfCode *b = _LastBloc2.back()->listCode().front();
			(_LastBloc->getBagOfCode())->addConstraint(b);
			delete b;
			_LastBloc2.back()->listCode().pop_front();
		}
		_LastBloc1.back()->release();
		_LastBloc1.pop_back();
		_LastBloc2.back()->release();
		_LastBloc2.pop_back();
	}
	
	sint32 CCompilateur::affectation()
	{
		if(_LasAffectationVarStr.size() >= 1)
		{			
			if(_LasAffectationVarStr.size() == 1)
			{
				_LastAffectation = getVar(_LasAffectationVarStr.front().data());
				if(_LastAffectation == NULL)
				{
					sint32 member = ((IClassInterpret *)_SelfClass.get())->getStaticMemberIndex(NLAIAGENT::CStringVarName(_LasAffectationVarStr.front().data()));
					if(member < 0)
					{
						NLAIAGENT::IObjectIA *i;
						if(_ExpressionType->satisfied())
						{
							IOpType * c= new COperandSimple(new NLAIC::CIdentType(*_ExpressionType->getConstraintTypeOf()));
							c->incRef();
							i = new CObjectUnknown(c);
							i->incRef();
							_ExpressionType->release();
							_ExpressionType = NULL;
						}
						else
						{											
							_ExpressionType->incRef();
							i = new CObjectUnknown(_ExpressionType);
							i->incRef();
							_ConstraintType.push_back(_ExpressionType);
							_ExpressionType = NULL;
						}						
						
						#ifdef NL_DEBUG
						_LastAffectation = new CVarPStackParam((sint32)_VarState,_LasAffectationVarStr.front().data());
						_LastAffectation->incRef();
						#else
						_LastAffectation = new CVarPStackParam((sint32)_VarState);
						_LastAffectation->incRef();
						#endif

						_LastBloc->allocLocVar(_LasAffectationVarStr.front().data(), _LastAffectation);

						_Heap[(int)_Heap] = i;
						_Heap ++;
						_VarState ++;
						if(_LastAffectation)
						{
							if (_Debug)
							{
								_LastBloc->addCode(new CLocAllocDebug());
								_LastBloc->addCode(new CAffOpCodeDebug( ((CVarPStackParam *)_LastAffectation)->getIndex(), _LasAffectationVarStr.front().data()));
							}
							else
							{
								_LastBloc->addCode(new CLocAlloc());
								_LastBloc->addCode(new CAffOpCode( ((CVarPStackParam *)_LastAffectation)->getIndex()));
							}
						}
					}
					else
					{
						
						if(!affectationMember(member)) return false;
					}
				}
				else
				{
					if (_Debug)
					{
						_LastBloc->addCode(new CAffOpCodeDebug( ((CVarPStackParam *)_LastAffectation)->getIndex(), _LasAffectationVarStr.front().data()));
					}
					else
					{
						_LastBloc->addCode(new CAffOpCode( ((CVarPStackParam *)_LastAffectation)->getIndex()));
					}
				}
			}
			else
			{
				_LastAffectation = getVar(_LasAffectationVarStr.front().data());
				if(_LastAffectation == NULL)
				{				
					if(!PrivateError())
					{					
						return false;
					}
					
					if(_ExpressionType->satisfied())
					{
						_ExpressionType->release();
					}
					else
					{
						_ConstraintType.push_back(_ExpressionType);
					}
					_ExpressionType = NULL;
				}
				else
				{					
					_LasAffectationVarStr.pop_front();					
					IOpType *cont = (IOpType *)((const CObjectUnknown *)((CVarPStackParam *)_LastAffectation)->getObject())->getBaseType();
					if(cont->satisfied())
					{
						std::list<sint32> ref;
						IOpType *constr;
						IClassInterpret *cl = (IClassInterpret *)((CClassInterpretFactory *)cont->getConstraintTypeOf()->getFactory())->getClass();
						if(!isValidateVarName(cl,ref,_LasAffectationVarStr,constr))
						{
							if(constr) constr->release();
							return false;		
						}
						COperationTypeGD *dg = new COperationTypeGD();
						dg->setOperationD(constr);
						dg->setOperationG(_ExpressionType);
						dg->setOp(NLAIC::CTypeOfOperator::opAff);
						dg->incRef();
						_ConstraintType.push_back(dg);						
						_ExpressionType = NULL;						
						_LastBloc->addCode(new CAffHeapMemberiOpCode( ref,((CVarPStackParam *)_LastAffectation)->getIndex()));
						

					}
					else
					{
						_LastBloc->addCode(new CHaltOpCode());
						CBagOfCode *b = _LastBloc->getBagOfCode();
						cont->incRef();
						IOpType *c = (IOpType *)getMethodConstraint(CConstraintStackComp(CConstraintStackComp::heapAffectation,
																						((CVarPStackParam *)_LastAffectation)->getIndex(),
																						_LasAffectationVarStr ,cont,0,0));
						if(c == NULL)
						{
							cont->incRef();
							c = new CConstraintStackComp(CConstraintStackComp::heapAffectation,((CVarPStackParam *)_LastAffectation)->getIndex(),
																							_LasAffectationVarStr ,cont,0,0);
							c->incRef();
							_ConstraintType.push_back(c);
						}
						
						c->incRef();
						COperationTypeGD *dg = new COperationTypeGD();
						dg->setOperationD(c);
						dg->setOperationG(_ExpressionType);
						dg->setOp(NLAIC::CTypeOfOperator::opAff);
						dg->incRef();
						_ConstraintType.push_back(dg);						
						_ExpressionType = NULL;						
						b->addConstraint(c);
					}
				}
				
			}									
		}
		else
		{
			yyerror("erreur de definition de variable");
			return 0;
		}				
		return true;
	}

	sint32 CCompilateur::affectationMember(sint32 member)
	{
		return affectationMember((IClassInterpret *)_SelfClass.get(),member);
	}

	sint32 CCompilateur::affectationMember(IClassInterpret *base,sint32 member)
	{		
		try
		{															
			_LastBloc->addCode(new CAffMemberOpCode(member));
			COperandSimple *a = new COperandSimple(new NLAIC::CIdentType(base->getStaticMember(member)->getType()));
			a->incRef();
			COperationTypeGD *gd = new COperationTypeGD();
			gd->setOperationG(a);
			gd->setOperationD(_ExpressionType);
			gd->setOp(NLAIC::CTypeOfOperator::opAff);
			gd->incRef();
			if(gd->satisfied())
			{
				gd->release();
			}
			else
			{				
				_ConstraintType.push_back(gd);	
			}
			_ExpressionType = NULL;
			
		}
		catch(CExceptionHaveNoType a)
		{
			char text[4096];
			sprintf(text,"Referance à un composant non défini ou constituer d'une hiararchie non completement définie");
			yyerror(text);
			return 0;
		}
		return true;
	}

	sint32 CCompilateur::PrivateError()
	{		
		std::list<sint32> ref;
		IOpType *c;
		if(!isValidateVarName(ref,_LasAffectationVarStr,c))
		{
			if(c) c->release();
			return false;			
		}
		if(ref.size() == 1)
		{
			_LastBloc->addCode(new CAffMemberOpCode(ref.back()));			
		}
		else
		{
			_LastBloc->addCode(new CAffMemberiOpCode	(ref));
		}
		
		if(c) c->release();
		return true;
	}	

	sint32 CCompilateur::isValidateVarName(std::list<sint32> &ref,std::list<NLAISCRIPT::CStringType> &listName,IOpType *&type)
	{		
		return isValidateVarName(_SelfClass.get(),ref,listName,type);
	}
	
	sint32 CCompilateur::isValidateVarName(const NLAIAGENT::IObjectIA *base,std::list<sint32> &ref,std::list<NLAISCRIPT::CStringType> &listName,IOpType *&type)
	{		
		NLAIAGENT::CStringVarName varName(listName.back().data());
		sint32 h;
		listName.pop_back();
		type = NULL;		
		const NLAIAGENT::IObjectIA *baseClass = getValidateHierarchyBase(base,ref,h,listName);		

		if(baseClass)
		{		
			sint32 i = baseClass->getStaticMemberIndex(varName);
			if(i>=0)
			{				
				const NLAIAGENT::IObjectIA *c = baseClass->getStaticMember(i);
				/*if((const NLAIC::CTypeOfObject &)baseClass->getType() & NLAIC::CTypeOfObject::tAgentInterpret) 
				{
					i += getCompementShift((IClassInterpret *)baseClass);
				}*/
				if(c != NULL)
				{
					ref.push_back(i);					
					type = new COperandSimple(new NLAIC::CIdentType(c->getType()));
					type->incRef();
					return true;
				}
				else 
				{
					return false;
				}
				
			}					
		}
		return false;		
	}

	sint32 CCompilateur::affectationiMember(const IClassInterpret *baseClass,std::list<NLAISCRIPT::CStringType> &varHName)
	{

		return true;		
	}	

	void CCompilateur::allocExpression(IOpCode *op,bool gd)
	{
		switch(_LastFact.VarType)
		{
			case varTypeImediate:
				if(!_LastFact.IsUsed)
				{
					_LastBloc->addCode(new CLdbOpCode (*_LastFact.Value));
					_LastFact.IsUsed = true;
				}
				if(op) _LastBloc->addCode(op);
				break;
				
			case varTypeLocal:
				if(!_LastFact.IsUsed)
				{												
					if(((NLAIAGENT::IObjectIA	*)_LastFact.ValueVar)->getType() == CVarPStackParam::IdVarPStackParam)
					{
						_LastBloc->addCode(new CLdbRefOpCode ( ((CVarPStackParam *)((NLAIAGENT::IObjectIA *)_LastFact.ValueVar))->getIndex()));
					}
					else 
					{
						_LastBloc->addCode(new CLdbOpCode (*((NLAIAGENT::IObjectIA *)_LastFact.ValueVar)));
					}											
					_LastFact.IsUsed = true;
				}
				if(op) _LastBloc->addCode(op);
				break;
			case varTypeMember:
				if(!_LastFact.IsUsed)
				{
					if(_LastFact.Member.size() == 1)
					{
						_LastBloc->addCode(new CLdbMemberOpCode(_LastFact.Member.back()));			
					}
					else
					{
						_LastBloc->addCode(new CLdbMemberiOpCode(_LastFact.Member));
					}
					
					_LastFact.IsUsed = true;
				}
				if(op) _LastBloc->addCode(op);
				break;

			case varForFunc:
				if(op) _LastBloc->addCode(op);
				break;

			case varTypeHeapMember:
				if(!_LastFact.IsUsed)
				{
					if(_LastFact.Member.size())
					{
						_LastBloc->addCode(new CLdbHeapMemberiOpCode(_LastFact.Member,((CVarPStackParam *)_LastFact.ValueVar)->getIndex()));
					}
					else
					{
						_ConstraintType.push_back(_LastFact.TypeStack);						
						_LastBloc->addCode(new CHaltOpCode());
						CBagOfCode *b = _LastBloc->getBagOfCode();
						b->addConstraint(_LastFact.TypeStack);
						_LastFact.TypeStack = NULL;
					}
					_LastFact.IsUsed = true;
				}
				if(op) _LastBloc->addCode(op);
				break;
				
			case varTypeStackMember:
				if(!_LastFact.IsUsed)
				{
					if(_LastFact.Member.size())
					{
						_LastBloc->addCode(new CLdbStackMemberiOpCode(_LastFact.Member));
					}
					else
					{
						_ConstraintType.push_back(_LastFact.TypeStack);						
						_LastBloc->addCode(new CHaltOpCode());
						CBagOfCode *b = _LastBloc->getBagOfCode();
						b->addConstraint(_LastFact.TypeStack);
						_LastFact.TypeStack = NULL;
					}
					_LastFact.IsUsed = true;
				}
				if(op) _LastBloc->addCode(op);
				break;
		}
	}

	void CCompilateur::nameMethodeProcessing()
	{	
		NLAIAGENT::IBaseGroupType *g = new NLAIAGENT::CGroupType;
		g->incRef();
		_LastStringParam.push_back(g);		
		
		std::list<NLAISCRIPT::CStringType>::iterator i = _LasVarStr.begin();
		while(i != _LasVarStr.end())
		{
			NLAISCRIPT::CStringType &s = *i++;
			g->cpy(NLAIAGENT::CStringType(NLAIAGENT::CStringVarName(s.data())));
		}
	}

	void CCompilateur::RegisterClass()
	{
		IClassInterpret *c = (IClassInterpret *)_SelfClass.get();
		//c->buildVTable();
		const NLAIAGENT::IVarName &className = *c->getClassName();		
#ifdef NL_DEBUG
	className.getString();
	_ConstraintType.size();
#endif
		NLAIC::CIdentType id(className.getString(),(const NLAIC::IClassCFactory &)CClassInterpretFactory(c), 
							NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tAgentInterpret), NLAIC::CTypeOfOperator(0));

		c->setType(new NLAIC::CIdentType(id));
	}

	bool CCompilateur::computContraint()
	{				

		sint32 constraint = getNMethodConstraint();
		if((constraint - runMethodConstraint()))
		{			
			errorMethodConstraint();
			cleanMethodConstraint();
			errorTypeConstraint();
			cleanTypeConstraint();
			return 0;					
		}
		
		/*constraint = _ConstraintType.size() - runTypeConstraint();
		while(_ConstraintType.size())
		{
			delete _ConstraintType.back();
			_ConstraintType.pop_back();
		}*/
		
		cleanMethodConstraint();
		cleanTypeConstraint();			
		return true;
	}

	void CCompilateur::initParam()
	{
		clean();
		_LastBloc = new IBlock(_Debug);
		_LastBloc->incRef();
		_Heap -= (sint32)_Heap;
		CVarPStack::_LocalTableRef = &_Heap[0];
		_VarState.clear();								
		_VarState.pushMark();
		if(!_InLineParse) _LastBloc->addCode((new CMarkAlloc));
		_Param.push_back(new CParam);
		_Param.back()->incRef();
	}

	bool CCompilateur::registerMethod()
	{
		bool isRun =  !strcmp(_MethodName.back().getString(),_RUN_);
		bool isSend =  !strcmp(_MethodName.back().getString(),_SEND_);
		bool runProcces = false;
		if( isRun || isSend )
		{
			CParam p;			
			COperandSimple *x = new COperandSimple(new NLAIC::CIdentType("Message"));
			x->incRef();
			p.push(x);						
			
			if((isRun && _Param.back()->size() > 1)  || (isSend && _Param.back()->size() != 1))
			{
				char txt[1024*8];
				sprintf(txt,"method %s have more than 1 parametre",_MethodName.back().getString());
				yyerror(txt);
				return false;
			}
			else
			if(_Param.back()->size())
			{
				if(p.eval(*_Param.back()) < 0.0)
				{
					char txt[1024*8];
					sprintf(txt,"the method %s have a parametre not derived from Message class",_MethodName.back().getString());
					yyerror(txt);
					return false;
				}
			}
			else
			if(isRun)
			{
				runProcces = true;				
			}
		}
		
		sint32 indexMethod = ((IClassInterpret *)_SelfClass.get())->addBrancheCode(_MethodName.back(),*_Param.back());
		if(runProcces)
		{
			if( ((IClassInterpret *)_SelfClass.get())->getRunMethod() >=0 )
			{
				char txt[1024*8];
				sprintf(txt,"the method Run() have all ready defined");
				yyerror(txt);
				return false;
			}
			((IClassInterpret *)_SelfClass.get())->setRunMethod(indexMethod);
		}
		sint32 i = 0;
		_DecalageHeap = _Attrib.size();
		_Heap.setShift(_DecalageHeap);
		CVarPStackParam::_Shift = _DecalageHeap;

		NLAIAGENT::CStringType* debugStringAttrib;
		NLAIAGENT::CGroupType* debugAttrib;
		if (_Debug)
		{
			debugAttrib = new NLAIAGENT::CGroupType();
			debugAttrib->incRef();
		}

		while(_Attrib.size() != 0)
		{
			i--;
#ifdef NL_DEBUG
			_LastAffectation = new CVarPStackParam(i,_Attrib.back().first->getString());
			_LastAffectation->incRef();
#else
			_LastAffectation = new CVarPStackParam(i);
			_LastAffectation->incRef();
#endif
			// We put the functions atributs in the _LastBloc dico.
			if(!_LastBloc->allocLocVar(_Attrib.back().first->getString(),_LastAffectation))
			{
				char txt[1024*8];
				sprintf(txt,"variable '%s' already defined",_Attrib.back().first->getString());
				yyerror(txt);
				_Heap -= (sint32)_Heap;
				if(_Heap.restoreStackState()) _Heap.restoreStack();
				if(_Heap.restoreShiftState()) _Heap.restoreShift();
				_Heap -= (sint32)_Heap;							
				_LastAffectation->release();
				return 0;
			}
			if (_Debug)
			{
				// We store the function var name;
				debugStringAttrib = new NLAIAGENT::CStringType(*(_Attrib.back().first));
				debugStringAttrib->incRef();
				debugAttrib->pushFront(debugStringAttrib);
			}
			_Attrib.back().first->release();
			_Attrib.back().second->release();
			_Attrib.pop_back();
		}
		if (_Debug)
		{
			_LastBloc->addCode(new CAddParamNameDebug(*debugAttrib));
			debugAttrib->release();
		}
		//_Param.back()->clear();
		_Param.back()->release();
		_Param.pop_back();
		return true;
	}

	
	sint32 CCompilateur::typeOfMethod()
	{				

		const IOpType *a;
		if((a = ((IClassInterpret *)_SelfClass.get())->getBrancheCode().getTypeOfMethode()) == NULL)
		{						
			((IClassInterpret *)_SelfClass.get())->getBrancheCode().setTypeOfMethode(_ExpressionType);
			if(!_ExpressionType->satisfied())
			{
				_ConstraintType.push_back(_ExpressionType);
				_ExpressionType->incRef();
			}
		}
		else
		{			 
			if(a->getTypeOfClass() == IConstraint::operandListType)
			{
				((COperandListType *)a)->add(_ExpressionType);
			}
			else
			{
				COperandListType *c = new COperandListType();
				c->incRef();
				((IOpType *)a)->incRef();
				c->add((IOpType *)a);
				c->add(_ExpressionType);				
				((IClassInterpret *)_SelfClass.get())->getBrancheCode().setTypeOfMethode(c);
				if(!c->satisfied())
				{
					c->incRef();
					_ConstraintType.push_back(c);					
				}
			}			
		}
		_ExpressionType = NULL;
		_IsVoid = false;
		return true;
	}	

	void CCompilateur::callSend()
	{

		//IConstraint *c = getMethodConstraint(CConstraintMethode(CConstraintMethode::normalCall, 0 , _LastbaseClass,_LastStringParam.back(),_Param.back(),0,0));
	}

	void CCompilateur::callFunction()
	{
#ifdef NL_DEBUG
	char mName[1024];
	char pName[1024];
	_LastStringParam.back()->getDebugString(mName);
	_Param.back()->getDebugString(pName);	
	//sint32 i = _TypeList.size();
#endif
		CBagOfCode *sendOp = NULL;
		NLAIAGENT::CStringType *s = (NLAIAGENT::CStringType *)_LastStringParam.back()->get();
		if(	!strcmp(s->getStr().getString(),_SEND_) && _Param.back()->size() == 1)
		{			
			_LastBloc->addCode(new CNopOpCode());
			sendOp = _LastBloc->getBagOfCode();
		}
	
		_LastBloc->addCode(new CNopOpCode());
		CBagOfCode *b = _LastBloc->getBagOfCode();
		_LastbaseClass = NULL;
		_LastPosHeap = 0;
		_LastTypeCall = CConstraintMethode::normalCall;

		IConstraint *c;		
		_LastPosHeap = 0;
		_LastTypeCall = (sint32)CConstraintMethode::normalCall;
		if(_TypeList.size()) 
		{
			_LastbaseClass = _TypeList.back();
			_TypeList.pop_back();
			_LastStringParam.back()->incRef();
			_Param.back()->incRef();
			_LastbaseClass->incRef();
			_LastTypeCall = CConstraintMethode::stackCall;
			c = getMethodConstraint(CConstraintMethode((CConstraintMethode::TCallTypeOpCode)_LastTypeCall,_LastPosHeap,_LastbaseClass,_LastStringParam.back(),_Param.back(),0,0));			
		}
		else
		{
			NLAIAGENT::CStringType *name = (NLAIAGENT::CStringType *)(_LastStringParam.back()->getFront());
			NLAIAGENT::IObjectIA *var = getVar(name->getStr().getString());
			if(var)
			{
				((NLAIAGENT::IObjectIA *)_LastStringParam.back()->popFront())->release();
				_LastTypeCall = CConstraintMethode::heapCall;
				_LastPosHeap = ((CVarPStackParam *)var)->getIndex();
				_LastbaseClass = (IOpType *)((const CObjectUnknown *)((CVarPStackParam *)var)->getObject())->getBaseType();
				_LastStringParam.back()->incRef();
				_Param.back()->incRef();
				_LastbaseClass->incRef();
				c = getMethodConstraint(CConstraintMethode((CConstraintMethode::TCallTypeOpCode)_LastTypeCall,_LastPosHeap,_LastbaseClass,_LastStringParam.back(),_Param.back(),0,0));
				_LastbaseClass->incRef();
			}
			else
			{
				_LastStringParam.back()->incRef();
				_Param.back()->incRef();			
				_LastTypeCall = CConstraintMethode::normalCall;
				c = getMethodConstraint(CConstraintMethode((CConstraintMethode::TCallTypeOpCode)_LastTypeCall,_LastPosHeap,_LastbaseClass,_LastStringParam.back(),_Param.back(),0,0));
			}
		}
		
		if(c == NULL)
		{
			c = new CConstraintMethode((CConstraintMethode::TCallTypeOpCode)_LastTypeCall,_LastPosHeap,_LastbaseClass,_LastStringParam.back(),_Param.back(),yyLine,yyColone);
			_LastStringParam.back()->incRef();
			_Param.back()->incRef();
			_MethodConstraint.push_back(c);
			c->incRef();
		}
		b->addConstraint(c);
		if(sendOp != NULL)
		{
			NLAIAGENT::IBaseGroupType *nameRun = (NLAIAGENT::IBaseGroupType *)_LastStringParam.back()->clone();
			((NLAIAGENT::IObjectIA *)nameRun->pop())->release();
			nameRun->cpy(NLAIAGENT::CStringType ((NLAIAGENT::CStringVarName(_RUN_))));
			nameRun->incRef();
#ifdef NL_DEBUG	
	nameRun->getDebugString(mName);
#endif
			_Param.back()->incRef();
			if(_LastbaseClass) _LastbaseClass->incRef();
			c = getMethodConstraint(CConstraintFindRun((CConstraintMethode::TCallTypeOpCode)_LastTypeCall,0,_LastbaseClass,nameRun,_Param.back(),0,0));
			if(c == NULL)
			{				
				_Param.back()->incRef();
				if(_LastbaseClass) _LastbaseClass->incRef();
				c = new CConstraintFindRun((CConstraintMethode::TCallTypeOpCode)_LastTypeCall,0,_LastbaseClass,nameRun,_Param.back(),yyLine,yyColone);				
				c->incRef();
				_MethodConstraint.push_back(c);
			}
			else
			{
				nameRun->release();
			}
			//c->incRef();			
			sendOp->addConstraint(c);
		}
	}

	CCodeBrancheRun *CCompilateur::getCode(bool isMain)
	{
		if(_LastBloc != NULL && !_LastBloc->isCodeMonted())
		{									
			_VarState.popMark();
			IOpCode *x;
			if (_Debug)
			{
				x = new CFreeAllocDebug();
			}
			else
			{
				x = new CFreeAlloc();
			}
			_LastBloc->addCode(x);
			x = new CHaltOpCode();									
			_LastBloc->addCode(x);

			CCodeBrancheRun* listCode;
			if (_Debug)
			{
				listCode = _LastBloc->getCodeDebug(_SourceFileName);
			}
			else
			{
				listCode = _LastBloc->getCode();
			}

			if(listCode != NULL)
			{
				if(!isMain) ((IClassInterpret *)_SelfClass.get())->getBrancheCode().setCode((IOpCode *)listCode);
				_Heap -= (sint32)_Heap;
				if(_Heap.restoreStackState()) _Heap.restoreStack();
				if(_Heap.restoreShiftState()) _Heap.restoreShift();
				_Heap -= (sint32)_Heap;									
				//listCode->release();
			}								
			_DecalageHeap = 0;
			CVarPStackParam::_Shift = 0;
			
			return listCode;
			
		}
		return NULL;
	}

	void CCompilateur::initMessageManager()
	{
		IClassInterpret *o = (IClassInterpret *)(CManagerClass::IdManagerClass.getFactory())->getClass();
		o->incRef();
		_SelfClass.push(o);		
		initParam();
		setNewLine();
		if (_Debug)
		{
			_LastBloc->addCode(new CAddParamNameDebug(NLAIAGENT::CGroupType()));			
		}
	}

	bool CCompilateur::endMessageManager()
	{
		_ResultCompile = getCode(true);
		bool state = computContraint();
		((NLAIAGENT::IObjectIA *)_SelfClass.pop())->release();
		return state;
	}
}
