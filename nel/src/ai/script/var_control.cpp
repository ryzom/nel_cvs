/** \file var_control.cpp
 *
 * $Id: var_control.cpp,v 1.11 2001/01/19 11:11:45 chafik Exp $
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
#include "nel/ai/script/constraint_stack_component.h"
#include "nel/ai/script/object_unknown.h"
#include "nel/ai/agent/performative.h"

namespace NLAISCRIPT
{
	sint32 CCompilateur::castVariable(const NLAIAGENT::CStringVarName &v,NLAIAGENT::CStringVarName &t)
	{
		NLAIAGENT::IObjectIA *var = getVar(v.getString());
		if(var != NULL)
		{
			try
			{
				NLAIC::CIdentType id(t.getString());
				IOpType * c= new COperandSimple(new NLAIC::CIdentType(id));
				CObjectUnknown *o = (CObjectUnknown *)((CVarPStackParam *)var)->getObject();				
				o->setClassType(c);
				return true;
			}
			catch(NLAIE::IException &)
			{
				char text[4096];
				sprintf(text,"can't find %s object in the class factory",t.getString());
				yyerror(text);
				return false;
			}
			
		}
		else
		{
			yyerror("cast service is reseved to the local variable");
			return false;
		}		
	}
	sint32 CCompilateur::processingVar()
	{
		if(_LasVarStr.size() >= 1)
		{
			NLAIAGENT::IObjectIA *var;
			var = getVar(_LasVarStr.front().data());
			if(_LasVarStr.size() == 1)
			{
				if(var == NULL)
				{
					sint32 i = ((IClassInterpret *)_SelfClass.get())->getStaticMemberIndex(NLAIAGENT::CStringVarName(_LasVarStr.front().data()));
					if(i >= 0)
					{
						_LastFact.Member.clear();
						_LastFact.Member.push_back(i /*+ getCompementShift((IClassInterpret *)_SelfClass.get())*/);
						_LastFact.ValueVar = NULL;						
						_LastFact.VarType = varTypeMember;
						_LastFact.IsUsed = false;						
						try
						{							
							if(_FlotingExpressionType != NULL) delete _FlotingExpressionType;
							_FlotingExpressionType = new COperandSimple(new NLAIC::CIdentType ((_SelfClass.get())->getStaticMember(i)->getType()));
						}
						catch(NLAIE::IException &a)
						{							
							yyerror((char *)a.what());
							return 0;
						}
					}
					else
					{
						if(_TypeList.size())
						{
							IOpType *type = _TypeList.back();
							_TypeList.pop_back();
							_LastFact.Member.clear();
							_LastFact.MemberName.clear();

							if(_FlotingExpressionType != NULL) _FlotingExpressionType->release();


							if(type->satisfied())
							{
								NLAIAGENT::IObjectIA *c = (NLAIAGENT::IObjectIA *)((NLAIC::IClassCFactory *)type->getConstraintTypeOf()->getFactory())->getClass();
								sint32 i = c->getStaticMemberIndex(NLAIAGENT::CStringVarName(_LasVarStr.front().data()));
								if(i >= 0)
								{	
									/*if(((const NLAIC::CTypeOfObject &)*c->getType()) & NLAIC::CTypeOfObject::tAgentInterpret)
									{
										_LastFact.Member.push_back(i + getCompementShift((IClassInterpret *)c));
									}
									else*/
									{
										_LastFact.Member.push_back(i);
									}
									
								}
								else
								{
									type->release();									
									yyerror("attribut is not define");
									return 0;
								}									
								const NLAIAGENT::IObjectIA *o = c->getStaticMember(i);								
								NLAIC::CIdentType *id = new NLAIC::CIdentType (c->getType());
								_FlotingExpressionType = new COperandSimple(id);																
								type->release();
							}
							else
							{
								_LastFact.MemberName = _LasVarStr;
								type->incRef();
								IOpType *constr = (IOpType *)getMethodConstraint(CConstraintStackComp(CConstraintStackComp::stackCall,0,_LastFact.MemberName ,type,0,0));
								if(constr == NULL)
								{
									constr = new CConstraintStackComp(CConstraintStackComp::stackCall,0,_LastFact.MemberName,type,yyLine,yyColone);									
								}
								_FlotingExpressionType = constr;
							}							

							if(_LastFact.TypeStack) _LastFact.TypeStack->release();
							_LastFact.TypeStack = _FlotingExpressionType;
							_LastFact.TypeStack->incRef();
							_LastFact.ValueVar = NULL;	
							_LastFact.VarType = varTypeStackMember;
							_LastFact.IsUsed = false;
							
						}
						else 
						{
							yyerror("variable used but never declared");						
							return 0;
						}
					}
				}
				else
				{
					_LastFact.ValueVar = var;
					_LastFact.VarType = varTypeLocal;
					_LastFact.IsUsed = false;
					_LasVarStr.clear();					
					if(var->getType() == CVarPStackParam::IdVarPStackParam)
					{	
						if(_FlotingExpressionType != NULL) _FlotingExpressionType->release();
						try
						{
							_FlotingExpressionType = new COperandSimple(new NLAIC::CIdentType(((CVarPStackParam *)var)->getVarType()));
						}	
						catch(NLAIE::CExceptionUnReference &)
						{
							_FlotingExpressionType = (IOpType *)((const CObjectUnknown *)((CVarPStackParam *)var)->getObject())->getClassType();
							_FlotingExpressionType->incRef();
						}						
					}
					else
					{													
						if(_FlotingExpressionType != NULL) _FlotingExpressionType->release();
						_FlotingExpressionType = new COperandSimple(new NLAIC::CIdentType(var->getType()));						
					}					
				}
			}
			else
			{

				CConstraintStackComp::OpCodeType opCodeType = CConstraintStackComp::stackCall;
				sint32 posStack = 0;
				var = getVar(_LasVarStr.front().data());
				NLAIAGENT::CStringVarName varName(_LasVarStr.back().data());
				if(var != NULL)
				{
					_LasVarStr.pop_front();
					IOpType *cont = (IOpType *)((const CObjectUnknown *)((CVarPStackParam *)var)->getObject())->getBaseType();
					if(cont->satisfied())
					{
						const NLAIC::CIdentType &id = *cont->getConstraintTypeOf();
						IClassInterpret *cl;

						if(strcmp((const char *)id,((IClassInterpret *)_SelfClass.get())->getClassName()->getString()) == 0)
						{
							cl = (IClassInterpret *)_SelfClass.get();
						}
						else cl = (IClassInterpret *)((CClassInterpretFactory *)id.getFactory())->getClass();
						IOpType *c;												
						if(!isValidateVarName(cl,_LastFact.Member,_LasVarStr,c))
						{
							if(c) c->release();
							char txt[1024*8];
							sprintf(txt,"var '%s' is not define",varName.getString());
							yyerror(txt);
							return false;
						}						
						_FlotingExpressionType = c;
						if(_LastFact.TypeStack) _LastFact.TypeStack->release();
						_LastFact.TypeStack = _FlotingExpressionType;
						_LastFact.TypeStack->incRef();
						_LastFact.ValueVar = var;	
						_LastFact.VarType = varTypeHeapMember;
						_LastFact.IsUsed = false;
						return true;						
					}
					else
					{
						opCodeType = CConstraintStackComp::heapCall;
						posStack = ((CVarPStackParam *)var)->getIndex();
						cont->incRef();
						_TypeList.push_back(cont);
					}
					
				}

				if(!_TypeList.size())
				{				
					_LastFact.Member.clear();
					IOpType *c;
					if(!isValidateVarName(_LastFact.Member,_LasVarStr,c))
					{
						if(c) c->release();
						char txt[1024*8];
						sprintf(txt,"var '%s' is not define",varName.getString());
						yyerror(txt);
						return false;
					}
					_LastFact.ValueVar = NULL;						
					_LastFact.VarType = varTypeMember;
					_LastFact.IsUsed = false;
					if(_FlotingExpressionType != NULL) _FlotingExpressionType->release();
					_FlotingExpressionType = (IOpType *)c;
				}
				else
				{
					IOpType *type = _TypeList.back();
					_TypeList.pop_back();
					_LastFact.Member.clear();
					_LastFact.MemberName.clear();

					if(_FlotingExpressionType != NULL) _FlotingExpressionType->release();
					
					if(type->satisfied())
					{
						IClassInterpret *cl = (IClassInterpret *)type->getConstraintTypeOf()->allocClass();
						IOpType *c;
						if(!isValidateVarName(cl,_LastFact.Member,_LasVarStr,c))
						{
							if(c) c->release();
							type->release();
							cl->release();
							char txt[1024*8];
							sprintf(txt,"var '%s' is not define",varName.getString());
							yyerror(txt);
							return false;
						}						
						_FlotingExpressionType = (IOpType *)c;
						type->release();
						cl->release();
					}
					else
					{
						_LastFact.MemberName = _LasVarStr;
						type->incRef();
						IOpType *constr = (IOpType *)getMethodConstraint(CConstraintStackComp(opCodeType,posStack,_LastFact.MemberName ,type,0,0));
						if(constr == NULL)
						{
							constr = new CConstraintStackComp(opCodeType,posStack,_LastFact.MemberName,type,yyLine,yyColone);									
						}
						else
						{
							constr->incRef();
						}
						_FlotingExpressionType = constr;						
					}					

					if(_LastFact.TypeStack) _LastFact.TypeStack->release();
					_LastFact.TypeStack = _FlotingExpressionType;
					_LastFact.TypeStack->incRef();
					_LastFact.ValueVar = NULL;	
					_LastFact.VarType = varTypeStackMember;
					_LastFact.IsUsed = false;
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
	
	void CCompilateur::setParamVarName()
	{
		
		NLAIC::CIdentType idType = getTypeOfClass(_LastString);
		NLAIAGENT::IObjectIA *i;

		if(_BaseObjectDef)
		{						
			NLAIC::CIdentType idBase = getTypeOfClass(_LastBaseObjectDef);
			COperandSimple *t = new COperandSimple(new NLAIC::CIdentType(idType));
			COperandSimple *b = new COperandSimple(new NLAIC::CIdentType(idBase));			
			_Param.back()->push(t);
			t->incRef();			
			i = new CObjectUnknown(t,b);
		}
		else
		{			
			COperandSimple *c = new COperandSimple(new NLAIC::CIdentType(idType));
			_Param.back()->push(c);
			c->incRef();
			i = new CObjectUnknown(c);//(NLAIAGENT::IObjectIA *)NLAIC::createInstance(id);
		}
				
		NLAIAGENT::IVarName *s = new NLAIAGENT::CStringVarName (LastyyText[1]);
		_Attrib.push_back(pairType(s,i));
				
		i->incRef();
		_Heap[(int)_Heap] = i;
		_Heap ++;
		
	}
	void CCompilateur::pushParamExpression()
	{
		_LastBloc->addCode((new CAddOpCode));
		_Param.back()->push(_ExpressionType);
		if(!_ExpressionType->satisfied())
		{
			_ConstraintType.push_back(_ExpressionType);
			_ExpressionType->incRef();
		}
		_ExpressionType = NULL;
	}

	void CCompilateur::setPerformative(NLAIAGENT::IMessageBase::TPerformatif p)
	{
		if(_LastFact.Value != NULL) 
							_LastFact.Value->release();
		switch(p)
		{
		case NLAIAGENT::IMessageBase::PExec:
			_LastFact.Value = new NLAIAGENT::CPExec();
			break;
		case NLAIAGENT::IMessageBase::PAchieve:
			_LastFact.Value = new NLAIAGENT::CPAchieve();
			break;
		case NLAIAGENT::IMessageBase::PAsk:
			_LastFact.Value = new NLAIAGENT::CPAsk();
			break;
		case NLAIAGENT::IMessageBase::PBreak:
			_LastFact.Value = new NLAIAGENT::CPBreak();
			break;
		case NLAIAGENT::IMessageBase::PTell:
			_LastFact.Value = new NLAIAGENT::CPTell();
			break;
		case NLAIAGENT::IMessageBase::PKill:
			_LastFact.Value = new NLAIAGENT::CPKill();
			break;
		}		
		_LastFact.VarType = varTypeImediate;
		_LastFact.IsUsed = false;		
		if(_FlotingExpressionType != NULL) _FlotingExpressionType->release();
		_FlotingExpressionType = new COperandSimple(new NLAIC::CIdentType(_LastFact.Value->getType()));
	}

	void CCompilateur::setImediateVarNill()
	{
		if(_LastFact.Value != NULL) 
							_LastFact.Value->release();
		
		_LastFact.Value = &NLAIAGENT::DigitalType::NullOperator;
		_LastFact.Value->incRef();
		_LastFact.VarType = varTypeImediate;
		_LastFact.IsUsed = false;		
		if(_FlotingExpressionType != NULL) _FlotingExpressionType->release();
		_FlotingExpressionType = new COperandSimple(new NLAIC::CIdentType(_LastFact.Value->getType()));
	}

	void CCompilateur::setImediateVar()
	{
		if(_LastFact.Value != NULL) 
							_LastFact.Value->release();
		_LastFact.Value = new NLAIAGENT::DDigitalType(LastyyNum);
		_LastFact.VarType = varTypeImediate;
		_LastFact.IsUsed = false;		
		if(_FlotingExpressionType != NULL) _FlotingExpressionType->release();
		_FlotingExpressionType = new COperandSimple(new NLAIC::CIdentType(_LastFact.Value->getType()));
	}

	void CCompilateur::setListVar()
	{
		_LastBloc->addCode(new CLdbOpCode (NLAIAGENT::CGroupType()));								
		NLAIAGENT::CGroupType l;
		if(_FlotingExpressionType != NULL) _FlotingExpressionType->release();
		_FlotingExpressionType = new COperandSimple(new NLAIC::CIdentType(l.getType()));
	}

	void CCompilateur::setChaineVar()
	{
		if(_LastFact.Value != NULL) 
							_LastFact.Value->release();
		_LastFact.Value = new NLAIAGENT::CStringType(NLAIAGENT::CStringVarName(LastyyText[1]));		
		_LastFact.VarType = varTypeImediate;
		_LastFact.IsUsed = false;								
		if(_FlotingExpressionType != NULL) _FlotingExpressionType->release();
		_FlotingExpressionType = new COperandSimple(new NLAIC::CIdentType(_LastFact.Value->getType()));
	}

	void CCompilateur::setStackVar(const NLAIC::CIdentType &type)
	{
		if(_LastFact.Value != NULL) 
							_LastFact.Value->release();
		_LastFact.VarType = varForFunc;
		_LastFact.IsUsed = false;								
		if(_FlotingExpressionType != NULL) _FlotingExpressionType->release();
		_FlotingExpressionType = new COperandSimple(new NLAIC::CIdentType(type) );
	}

	void CCompilateur::setMethodVar()
	{

#ifdef NL_DEBUG
	char mName[1024];
	char pName[1024];
	_LastStringParam.back()->getDebugString(mName);
	_Param.back()->getDebugString(pName);
#endif
		if(_LastFact.Value != NULL) 
							_LastFact.Value->release();
		_LastFact.Value = NULL;		
		_LastFact.VarType = varForFunc;
		_LastFact.IsUsed = false;		

		if(_FlotingExpressionType != NULL)
		{
			_FlotingExpressionType->release();
			_FlotingExpressionType = NULL;
		}
		_LastStringParam.back()->incRef();
		_Param.back()->incRef();
		if(_LastbaseClass != NULL)_LastbaseClass->incRef();
		IConstraint *c = getMethodConstraint(CConstraintMethode((CConstraintMethode::TCallTypeOpCode) _LastTypeCall,_LastPosHeap,_LastbaseClass,_LastStringParam.back(),_Param.back(),0,0));
		if(c == NULL)
		{
			_LastStringParam.back()->incRef();
			_Param.back()->incRef();
			if(_LastbaseClass != NULL)_LastbaseClass->incRef();
			c = new CConstraintMethode((CConstraintMethode::TCallTypeOpCode) _LastTypeCall,_LastPosHeap,_LastbaseClass,_LastStringParam.back(),_Param.back(),yyLine,yyColone);
			_MethodConstraint.push_back(c);			
		}
		c->incRef();		
		IConstraint *cm = new CConstraintChkMethodeType(c,0,0);	
		_FlotingExpressionType = new COperandUnknown(cm);			
		
		_LastStringParam.back()->release();
		_LastStringParam.pop_back();
		_Param.back()->release();
		_Param.pop_back();
#ifdef NL_DEBUG
	if(_LastStringParam.size()) _LastStringParam.back()->getDebugString(mName);
	if(_Param.size())_Param.back()->getDebugString(pName);
		
#endif		
	}

	bool CCompilateur::buildObject()
	{		
		NLAIAGENT::CStringType *s = (NLAIAGENT::CStringType *)_LastStringParam.back()->get();
		if(_LastStringParam.back()->size() == 1)
		{		
			NLAIC::CIdentType id(s->getStr().getString());
			try
			{
				if(_FlotingExpressionType) _FlotingExpressionType->release();
				_FlotingExpressionType = new COperandSimple(new NLAIC::CIdentType (id));				

				NLAIAGENT::IObjectIA *i = (NLAIAGENT::IObjectIA *)NLAIC::createInstance(id);
				
				_LastBloc->addCode( new CLdbOpCode( *i ) );
				_LastBloc->addCode(new CHaltOpCode());
				CBagOfCode *b = _LastBloc->getBagOfCode();
				

				NLAIAGENT::IBaseGroupType *g = new NLAIAGENT::CGroupType();
				g->cpy(NLAIAGENT::CStringType(NLAIAGENT::CStringVarName(_CONSTRUCTOR_)));

				_Param.back()->incRef();					
				_FlotingExpressionType->incRef();
				CConstraintMethode *cm = new CConstraintMethode(CConstraintMethode::newCall,0,_FlotingExpressionType,g,_Param.back(),0,0);
				IConstraint *c = getMethodConstraint(*cm);					
				if(c == NULL)
				{
					c = cm;
					_MethodConstraint.push_back(c);						
				}
				else
				{
					cm->release();
				}
				b->addConstraint(c);
				i->release();				
				_LastStringParam.back()->release();
				_LastStringParam.pop_back();
				_Param.back()->release();
				_Param.pop_back();		
			}
			catch(NLAIE::IException &)
			{

				return false;	
			}
			return true;
		}
		return false;
	}

	void CCompilateur::setTypeExpression()
	{
		if(_ExpressionType)
		{								
			COperandUnknown *gd = new COperandUnknown(_ExpressionType);
			_ExpressionType = gd;
		}
		else
		{
			_ExpressionType = new COperandUnknown(_FlotingExpressionType);
			_FlotingExpressionType = NULL;
		}
	}

	void CCompilateur::setTypeExpression(NLAIC::CTypeOfOperator::TTypeOp op,const char *txtOp)
	{
		if ( _ExpressionType )
		{					
			COperationType *gd = new COperationType;
			gd->setOperand(_ExpressionType);
			gd->setOp(op);
			_ExpressionType = gd;
		}
		else
		{
			_ExpressionType = new COperationType;		
			((COperationType *)_ExpressionType)->setOperand(_FlotingExpressionType);
			((COperationType *)_ExpressionType)->setOp(op);
			_FlotingExpressionType = NULL;
		}
	}
	
	void CCompilateur::setTypeExpressionG()
	{		
		if(_ExpressionType)
		{					
			COperationTypeGD *gd = new COperationTypeGD;
			gd->setOperationG(_ExpressionType);
			_ExpressionType = gd;
		}
		else
		{
			_ExpressionType = new COperationTypeGD;
			((COperationTypeGD *)_ExpressionType)->setOperationG(_FlotingExpressionType);
			_FlotingExpressionType = NULL;
		}
		_ExpressionOp.push_back(_ExpressionType);
		_ExpressionType = NULL;
	}

	void CCompilateur::setTypeExpressionD(NLAIC::CTypeOfOperator::TTypeOp op,const char *txtOp)
	{
		if(_ExpressionType != NULL && _FlotingExpressionType == NULL) _FlotingExpressionType = _ExpressionType;
		//else if(_ExpressionType == NULL && _FlotingExpressionType != NULL) _FlotingExpressionType = _ExpressionType;
		_ExpressionType = _ExpressionOp.back();
		_ExpressionOp.pop_back();

		((COperationTypeGD *)_ExpressionType)->setOperationD(_FlotingExpressionType);
		((COperationTypeGD *)_ExpressionType)->setOp(op);
		_FlotingExpressionType = NULL;
	}
}
