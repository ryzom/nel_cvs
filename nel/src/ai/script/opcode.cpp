/** \file opcode.cpp
 *
 * $Id: opcode.cpp,v 1.18 2002/03/12 15:52:56 chafik Exp $
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
#include <string>
#include "nel/ai/script/compilateur.h"
#include "nel/ai/agent/agent_script.h"
#include "nel/ai/script/type_def.h"
#include "nel/ai/script/object_load.h"


namespace NLAISCRIPT
{	

	NLAIAGENT::TProcessStatement CNegOpCode::runOpCode(CCodeContext &context)
	{	
		sint k = (int)context.Stack;
		NLAIAGENT::IObjetOp *o = (NLAIAGENT::IObjetOp *)context.Stack[k];
		context.Stack[k] = o->getNeg();
		o->release();
		return NLAIAGENT::IObjectIA::ProcessIdle;
	}

	void CNegOpCode::getDebugResult(std::string &str,CCodeContext &context) const
	{
		std::string X;
		((NLAIAGENT::IObjectIA *)context.Stack)->getDebugString(X);
		str +="-";
		str += X;		
	}


	NLAIAGENT::TProcessStatement CAddOpCode::runOpCode(CCodeContext &context)
	{		
		sint k = (int)context.Stack;
		NLAIAGENT::IObjetOp *a = (NLAIAGENT::IObjetOp *)context.Stack[k - 1];
		NLAIAGENT::IObjetOp *b = (NLAIAGENT::IObjetOp *)context.Stack[k];

		context.Stack[k - 1] = (*a) + (b);
		
		context.Stack--;
		a->release();
		return NLAIAGENT::IObjectIA::ProcessIdle;;
	}

	void CAddOpCode::getDebugResult(std::string &str,CCodeContext &context) const 
	{
		std::string X,Y;
		context.Stack[(int)context.Stack - 1]->getDebugString(X);
		context.Stack[(int)context.Stack]->getDebugString(Y);

		str = X;
		str += " + ";
		str += Y;	
	}

	NLAIAGENT::TProcessStatement CSubOpCode::runOpCode(CCodeContext &context)
	{				
		sint k = (int)context.Stack;
		NLAIAGENT::IObjetOp *a = (NLAIAGENT::IObjetOp *)context.Stack[k - 1];
		NLAIAGENT::IObjetOp *b = (NLAIAGENT::IObjetOp *)context.Stack[k];

		context.Stack[k - 1] = (*a) - (b);
		
		context.Stack--;
		a->release();
		return NLAIAGENT::IObjectIA::ProcessIdle;;
	}
	
	void CSubOpCode::getDebugResult(std::string &str,CCodeContext &context) const 
	{
		std::string X,Y;
		context.Stack[(int)context.Stack - 1]->getDebugString(X);
		context.Stack[(int)context.Stack]->getDebugString(Y);
		
		str = X;
		str += " - ";
		str += Y;
	}

	NLAIAGENT::TProcessStatement CDivOpCode::runOpCode(CCodeContext &context)
	{				
		sint k = (int)context.Stack;
		NLAIAGENT::IObjetOp *a = (NLAIAGENT::IObjetOp *)context.Stack[k - 1];
		NLAIAGENT::IObjetOp *b = (NLAIAGENT::IObjetOp *)context.Stack[k];

		context.Stack[k - 1] = (*a) / (b);
		
		context.Stack--;
		a->release();
		return NLAIAGENT::IObjectIA::ProcessIdle;;
	}

	void CDivOpCode::getDebugResult(std::string &str,CCodeContext &context) const
	{
		std::string X,Y;
		context.Stack[(int)context.Stack - 1]->getDebugString(X);
		context.Stack[(int)context.Stack]->getDebugString(Y);
		
		str = X;
		str += " / ";
		str += Y;
	}


	NLAIAGENT::TProcessStatement CMulOpCode::runOpCode(CCodeContext &context)
	{				
		sint k = (int)context.Stack;
		NLAIAGENT::IObjetOp *a = (NLAIAGENT::IObjetOp *)context.Stack[k - 1];
		NLAIAGENT::IObjetOp *b = (NLAIAGENT::IObjetOp *)context.Stack[k];

		context.Stack[k - 1] = (*a) * (b);
		
		context.Stack--;
		a->release();
		return NLAIAGENT::IObjectIA::ProcessIdle;;
	}

	void CMulOpCode::getDebugResult(std::string &str,CCodeContext &context) const
	{
		std::string X,Y;
		context.Stack[(int)context.Stack - 1]->getDebugString(X);
		context.Stack[(int)context.Stack]->getDebugString(Y);
		
		str = X;
		str += " * ";
		str += Y;
	}

	NLAIAGENT::TProcessStatement CSupOpCode::runOpCode(CCodeContext &context)
	{		
		sint k = (int)context.Stack;
		NLAIAGENT::IObjetOp *a = (NLAIAGENT::IObjetOp *)context.Stack[k - 1];
		NLAIAGENT::IObjetOp *b = (NLAIAGENT::IObjetOp *)context.Stack[k];

		context.Stack[k - 1] = (*a) > (*b);
		
		context.Stack--;
		a->release();

		return NLAIAGENT::IObjectIA::ProcessIdle;;
	}

	void CSupOpCode::getDebugResult(std::string &str,CCodeContext &context) const
	{
		std::string X,Y;
		context.Stack[(int)context.Stack - 1]->getDebugString(X);
		context.Stack[(int)context.Stack]->getDebugString(Y);
		
		str = X;
		str += " > ";
		str += Y;
	}

	

	NLAIAGENT::TProcessStatement CInfOpCode::runOpCode(CCodeContext &context)
	{				
		sint k = (int)context.Stack;
		NLAIAGENT::IObjetOp *a = (NLAIAGENT::IObjetOp *)context.Stack[k - 1];
		NLAIAGENT::IObjetOp *b = (NLAIAGENT::IObjetOp *)context.Stack[k];

		context.Stack[k - 1] = (*a) < (*b);
		
		context.Stack--;
		a->release();

		return NLAIAGENT::IObjectIA::ProcessIdle;;
	}

	void CInfOpCode::getDebugResult(std::string &str,CCodeContext &context) const
	{
		std::string X,Y;
		context.Stack[(int)context.Stack - 1]->getDebugString(X);
		context.Stack[(int)context.Stack]->getDebugString(Y);
		
		str = X;
		str += " < ";
		str += Y;
	}

	NLAIAGENT::TProcessStatement CEqOpCode::runOpCode(CCodeContext &context)
	{				
		sint k = (int)context.Stack;
		NLAIAGENT::IObjetOp *a = (NLAIAGENT::IObjetOp *)context.Stack[k - 1];
		NLAIAGENT::IObjetOp *b = (NLAIAGENT::IObjetOp *)context.Stack[k];

		context.Stack[k - 1] = (*a) == (*b);
		
		context.Stack--;
		a->release();
		return NLAIAGENT::IObjectIA::ProcessIdle;;
	}

	void CEqOpCode::getDebugResult(std::string &str,CCodeContext &context) const
	{
		std::string X,Y;
		context.Stack[(int)context.Stack - 1]->getDebugString(X);
		context.Stack[(int)context.Stack]->getDebugString(Y);
		
		str = X;
		str += " = ";
		str += Y;
	}

	NLAIAGENT::TProcessStatement CSupEqOpCode::runOpCode(CCodeContext &context)
	{				
		sint k = (int)context.Stack;
		NLAIAGENT::IObjetOp *a = (NLAIAGENT::IObjetOp *)context.Stack[k - 1];
		NLAIAGENT::IObjetOp *b = (NLAIAGENT::IObjetOp *)context.Stack[k];

		context.Stack[k - 1] = (*a) >= (*b);
		
		context.Stack--;
		a->release();
		return NLAIAGENT::IObjectIA::ProcessIdle;;
	}

	void CSupEqOpCode::getDebugResult(std::string &str,CCodeContext &context) const
	{
		std::string X,Y;
		context.Stack[(int)context.Stack - 1]->getDebugString(X);
		context.Stack[(int)context.Stack]->getDebugString(Y);
		
		str = X;
		str += " >= ";
		str += Y;
	}

	NLAIAGENT::TProcessStatement CInfEqOpCode::runOpCode(CCodeContext &context)
	{				
		sint k = (int)context.Stack;
		NLAIAGENT::IObjetOp *a = (NLAIAGENT::IObjetOp *)context.Stack[k - 1];
		NLAIAGENT::IObjetOp *b = (NLAIAGENT::IObjetOp *)context.Stack[k];

		context.Stack[k - 1] = (*a) <= (*b);
		
		context.Stack--;
		a->release();
		return NLAIAGENT::IObjectIA::ProcessIdle;;
	}

	void CInfEqOpCode::getDebugResult(std::string &str,CCodeContext &context) const
	{
		std::string X,Y;
		context.Stack[(int)context.Stack - 1]->getDebugString(X);
		context.Stack[(int)context.Stack]->getDebugString(Y);
		
		str = X;
		str += " <= ";
		str += Y;
	}

	NLAIAGENT::TProcessStatement CDiffOpCode::runOpCode(CCodeContext &context)
	{				
		sint k = (int)context.Stack;
		NLAIAGENT::IObjetOp *a = (NLAIAGENT::IObjetOp *)context.Stack[k - 1];
		NLAIAGENT::IObjetOp *b = (NLAIAGENT::IObjetOp *)context.Stack[k];

		context.Stack[k - 1] = (*a) != (*b);
		
		context.Stack--;
		a->release();

		return NLAIAGENT::IObjectIA::ProcessIdle;;
	}

	void CDiffOpCode::getDebugResult(std::string &str,CCodeContext &context) const
	{
		std::string X,Y;
		context.Stack[(int)context.Stack - 1]->getDebugString(X);
		context.Stack[(int)context.Stack]->getDebugString(Y);
		
		str = X;
		str += " != ";
		str += Y;
	}

	NLAIAGENT::TProcessStatement CNotOpCode::runOpCode(CCodeContext &context)
	{		
		
		NLAIAGENT::IObjetOp *op = !*((NLAIAGENT::IObjetOp *)((NLAIAGENT::IObjectIA *)context.Stack));		
		context.Stack[(int)context.Stack]->release();
		context.Stack[(int)context.Stack] = op;
		
		return NLAIAGENT::IObjectIA::ProcessIdle;
	}
	void CNotOpCode::getDebugResult(std::string &str,CCodeContext &context) const
	{
		std::string Y;
		context.Stack[(int)context.Stack]->getDebugString(Y);
		
		str = "!";
		str += Y;
	}

	NLAIAGENT::TProcessStatement CAffMemberiOpCode::runOpCode(CCodeContext &context)
	{				
		NLAIAGENT::IObjectIA *a = ((NLAIAGENT::IObjectIA *)context.Self);
		std::list<sint32>::iterator i = _I.begin();
		sint32 n = _I.size() - 1;
		while(n --)
		{			
			a = (NLAIAGENT::IObjectIA *)a->getStaticMember(*i++);
		}
		NLAIAGENT::IObjectIA *obj = ((NLAIAGENT::IObjectIA *)context.Stack);
		
		if(!a->setStaticMember(*i,obj))
								obj->incRef();
		context.Stack --;
		return NLAIAGENT::processIdle;
	}

	NLAIAGENT::TProcessStatement CAffMemberOpCode::runOpCode(CCodeContext &context)
	{	
		NLAIAGENT::IObjectIA *a = ((NLAIAGENT::IObjectIA *)context.Stack);
		
		if(!((NLAIAGENT::IObjectIA *)context.Self)->setStaticMember(_I,a))
																	a->incRef();
		context.Stack --;
		return NLAIAGENT::processIdle;
	}	

	//*************************************
	// CLocAllocDebug
	//*************************************

	CLocAllocDebug::CLocAllocDebug()
	{
	}		

	CLocAllocDebug::~CLocAllocDebug()
	{
	}

	NLAIAGENT::TProcessStatement CLocAllocDebug::runOpCode(CCodeContext &context)
	{				
		/*NLAIAGENT::iObjectIA *i = (NLAIAGENT::iObjectIA *)NLAIC::getRegistry()->createInstance(_b);
		context.Heap[(sint32)context.Heap] = NULL;*/
		context.Heap ++;
		context.ContextDebug.HeapDebug ++;
		//context.ContextDebug->HeapDebug[context.ContextDebug->HeapDebug]
		return NLAIAGENT::processIdle;
	}

	void CLocAllocDebug::getDebugResult(std::string &str,CCodeContext &context) const
	{					
		str += NLAIC::stringGetBuild("Reservation d'un espace memoir locale pour une variable à l'emplacement %d",(sint32)context.Heap);				
	}

	const NLAIC::IBasicType *CLocAllocDebug::clone() const
	{
		NLAIC::IBasicType *x = new CLocAllocDebug();
		return x;            
	}
	const NLAIC::IBasicType *CLocAllocDebug::newInstance() const 
	{
		return clone();
	}

	const NLAIC::CIdentType &CLocAllocDebug::getType() const
	{
		return IdLocAllocDebug;
	}		

	void CLocAllocDebug::save(NLMISC::IStream &os)
	{	

	}

	void CLocAllocDebug::load(NLMISC::IStream &is) 
	{				
	}

	//*************************************
	// CAffOpCodeDebug
	//*************************************

	CAffOpCodeDebug::CAffOpCodeDebug(int i, const char* varName):
	  _I(i),
	  _VarName(NULL)
	{
		_VarName = new char[strlen(varName)+1];
		strcpy(_VarName, varName);
	}
	
	CAffOpCodeDebug::~CAffOpCodeDebug()
	{
		delete[] _VarName;
	}
	

	NLAIAGENT::TProcessStatement CAffHeapMemberiOpCode::runOpCode(CCodeContext &context)
	{	
		
		NLAIAGENT::IObjectIA *a = ((NLAIAGENT::IObjectIA *)context.Heap[(int)_N]);			
		std::list<sint32>::iterator i = _I.begin();
		sint32 n = _I.size() - 1;
		while(n --)
		{
			a = (NLAIAGENT::IObjectIA *)a->getStaticMember(*i++);
		}		
		NLAIAGENT::IObjectIA *obj = ((NLAIAGENT::IObjectIA *)context.Stack);
		
		if(!a->setStaticMember(*i,obj))
									obj->incRef();
		context.Stack --;			
		return NLAIAGENT::processIdle;
	}

	void CAffHeapMemberiOpCode::getDebugResult(std::string &str,CCodeContext &context) const
	{	
		std::string txt;
		std::string txtClass;
		((NLAIAGENT::IObjectIA *)context.Stack)->getDebugString(txt);
		((NLAIAGENT::IObjectIA *)context.Heap[(int)_N])->getDebugString(txtClass);
		NLAIAGENT::IObjectIA *a = ((NLAIAGENT::IObjectIA *)context.Heap[(int)_N]);			
		std::list<sint32>::const_iterator i = _I.begin();
		sint32 n = _I.size();
		str += NLAIC::stringGetBuild("Affecte le membre sur le heap : %s ",txtClass.c_str());
		while(n --)
		{
			str += NLAIC::stringGetBuild("%d",*i++);
			if(n) str += "->";
		}		
		str += " a la valeur: ";
		str += txt;
	}

	NLAIAGENT::TProcessStatement CAffOpCodeDebug::runOpCode(CCodeContext &context)
	{	
		/*NLAIAGENT::iObjectIA *i = (NLAIAGENT::iObjectIA *)NLAIC::getRegistry()->createInstance(_b);
		context.Heap[(sint32)context.Heap] = i;
		context.Heap ++;*/

		NLAIAGENT::IObjectIA *i = (NLAIAGENT::IObjectIA *)context.Stack;
		NLAIAGENT::CStringVarName vn(_VarName);
		NLAIAGENT::IObjectIA *iD = (NLAIAGENT::IObjectIA *) new NLAIAGENT::CStringType(vn);
		i->incRef();		
		NLAIAGENT::IObjectIA *tmp = context.Heap[(int)_I];
		NLAIAGENT::IObjectIA *tmpD = context.ContextDebug.HeapDebug[(int)_I];
		context.Heap[_I] = i;
		context.ContextDebug.HeapDebug[_I] = iD;
		if(tmp) tmp->release();
		if(tmpD) tmpD->release();
		//context.Heap ++;
		context.Stack --;
		return NLAIAGENT::processIdle;
	}

	void CAffOpCodeDebug::getDebugResult(std::string &str,CCodeContext &context) const
	{
		std::string Y;
		context.Stack[(int)context.Stack]->getDebugString(Y);
		str += NLAIC::stringGetBuild("Affectation<%d> a %s",_I,Y.c_str());
	}

	const NLAIC::IBasicType *CAffOpCodeDebug::clone() const
	{
		NLAIC::IBasicType *x = new CAffOpCodeDebug(_I, _VarName);
		return x;            
	}
	const NLAIC::IBasicType *CAffOpCodeDebug::newInstance() const 
	{
		return clone();
	}

	const NLAIC::CIdentType &CAffOpCodeDebug::getType() const
	{
		return IdAffOpCodeDebug;
	}

	void CAffOpCodeDebug::getDebugString(char *) const{ }

	void CAffOpCodeDebug::save(NLMISC::IStream &os)
	{
		sint32 i = (sint32) _I;
		os.serial( i );
		std::string s(_VarName);
		os.serial(s);		
	}

	void CAffOpCodeDebug::load(NLMISC::IStream &is) 
	{		
		sint32 i;
		is.serial(i);
		_I = i;
		std::string varname;
		is.serial( varname );
		delete _VarName;
		_VarName = new char[varname.length()+1];
		strcpy(_VarName, varname.c_str());
	}

	//*************************************
	// CFreeAllocDebug
	//*************************************

	CFreeAllocDebug::CFreeAllocDebug()
	{
	}
	
	CFreeAllocDebug::~CFreeAllocDebug()
	{
	}
	
	NLAIAGENT::TProcessStatement CFreeAllocDebug::runOpCode(CCodeContext &context)
	{				
		sint32 i = (sint32)context.Heap;	
		i -= context.Heap.popMark();
		context.Heap -= i;

		sint32 j = (sint32)context.ContextDebug.HeapDebug;	
//		j -= context.ContextDebug.HeapDebug.popMark();
		context.ContextDebug.HeapDebug -= j;

		return NLAIAGENT::processIdle;
	}
	void CFreeAllocDebug::getDebugResult(std::string &str,CCodeContext &context) const
	{		
		str += NLAIC::stringGetBuild("libere le bloc de memoire de %d à %d",(sint32)context.Heap.mark(),(sint32)context.Heap);	
	}

	const NLAIC::IBasicType *CFreeAllocDebug::clone() const
	{
		NLAIC::IBasicType *x = new CFreeAllocDebug();
		return x;            
	}
	const NLAIC::IBasicType *CFreeAllocDebug::newInstance() const 
	{
		return clone();
	}

	const NLAIC::CIdentType &CFreeAllocDebug::getType() const
	{
		return IdFreeAllocDebug;
	}

	
	
	void CFreeAllocDebug::save(NLMISC::IStream &os)
	{		
	}

	void CFreeAllocDebug::load(NLMISC::IStream &is) 
	{				
	}

	//*************************************
	// CMarkMsg
	//*************************************
	NLAIAGENT::TProcessStatement CMarkMsg::runOpCode(CCodeContext &context)
	{
		NLAIAGENT::IBaseGroupType *param = (NLAIAGENT::IBaseGroupType *)context.Stack[(int)context.Stack];
		NLAIAGENT::IMessageBase *msg = (NLAIAGENT::IMessageBase *)param->get();
		msg->setMethodIndex(_Heritance,_I);
		return NLAIAGENT::processIdle;
	}

	//*************************************
	// CFindRunMsg
	//*************************************

	CFindRunMsg::CFindRunMsg(NLAIAGENT::IBaseGroupType *methodName,CParam *param,IOpType *baseClass,ILoadObject *objectLoad)
	{
		_Param = param;
		_MethodName = methodName;
		_BaseClass = baseClass;		
		_ObjectLoad = objectLoad;
	}

	CFindRunMsg::CFindRunMsg(const NLAIAGENT::IBaseGroupType &methodName,const CParam &param,const IOpType &baseClass,const ILoadObject &objectLoad)
	{
		_Param = (CParam *)param.clone();
		_MethodName = (NLAIAGENT::IBaseGroupType *)methodName.clone();
		_BaseClass = (IOpType *)baseClass.clone();
		_ObjectLoad = (ILoadObject *)objectLoad.clone();
	}

	CFindRunMsg::~CFindRunMsg()
	{
		_Param->release();
		_MethodName->release();
		_BaseClass->release();
		_ObjectLoad->release();
	}

	NLAIAGENT::TProcessStatement CFindRunMsg::runOpCode(CCodeContext &context)
	{
		const NLAIAGENT::IObjectIA *o = _ObjectLoad->getObject(context);
		NLAIAGENT::IBaseGroupType *param = (NLAIAGENT::IBaseGroupType *)_ObjectLoad->getParam(context);
		NLAIAGENT::CIteratorContener it = param->getIterator();
		it ++;
		NLAIAGENT::IMessageBase *msg = (NLAIAGENT::IMessageBase *)it++;

		const NLAIAGENT::CStringType *name = (const NLAIAGENT::CStringType *)_MethodName->get();

		NLAIAGENT::tQueue a = o->isMember(NULL,&name->getStr(),*_Param);
		
		if(a.size())
		{			
			NLAIAGENT::CIdMethod m = a.top();				
			msg->setMethodIndex(0,m.Index);			
		}

		return NLAIAGENT::processIdle;
	}

	void CFindRunMsg::getDebugResult(std::string &str,CCodeContext &context) const
	{
		std::string m;
		std::string p;
		_MethodName->getDebugString(m);
		_Param->getDebugString(p);
		str += NLAIC::stringGetBuild("find '%s%s' for %s",m.c_str() ,p.c_str(), (const char *)context.Self->getType());
	}

	const NLAIC::IBasicType *CFindRunMsg::clone() const
	{
		_Param->incRef();
		_MethodName->incRef();
		_BaseClass->incRef();
		_ObjectLoad->incRef();
		NLAIC::IBasicType *clone = new CFindRunMsg(_MethodName,_Param,_BaseClass,_ObjectLoad);
		return clone;
	}

	void CFindRunMsg::save(NLMISC::IStream &os)
	{	
		os.serial( (NLAIC::CIdentType &) _ObjectLoad->getType() );
		_ObjectLoad->save(os);
		_BaseClass->serial(os);
		os.serial( (NLAIC::CIdentType &) _Param->getType() );
		_Param->save(os);
		os.serial( (NLAIC::CIdentType &) _MethodName->getType() );
		_MethodName->save(os);
	}

	void CFindRunMsg::load(NLMISC::IStream &is) 
	{
		NLAIC::CIdentTypeAlloc id;
		is.serial( id );
		_ObjectLoad->release();
		_ObjectLoad = (ILoadObject *)id.allocClass();
		_ObjectLoad->load(is);

		_BaseClass->release();
		_BaseClass = (IOpType *)IOpType::loadIOpType(is);
		_Param->release();
		_MethodName->release();		
		is.serial( id );
		_Param = (CParam *)id.allocClass();
		_Param->load(is);
		is.serial( id );
		_MethodName = (NLAIAGENT::IBaseGroupType *)id.allocClass();
		_MethodName->load(is);
	}		

	//*************************************
	// CMsgSetSender
	//*************************************

	NLAIAGENT::TProcessStatement CMsgSetSender::runOpCode(CCodeContext &context)
	{		
		NLAIAGENT::IBaseGroupType *a = (NLAIAGENT::IBaseGroupType *)context.Stack[(int)context.Stack];
		NLAIAGENT::CIteratorContener i = a->getIterator();
		i++;
		if(a->size() == 3) i++;
		NLAIAGENT::IMessageBase *msg = (NLAIAGENT::IMessageBase *)i ++;
		//((NLAIAGENT::IObjectIA *)context.Self)->incRef();
		if(!msg->getProtcetSender()) msg->setSender((NLAIAGENT::IObjectIA *)context.Self);
		else msg->UnsetProtcetSender();

		
		return NLAIAGENT::processIdle;
	}

	void CMsgSetSender::save(NLMISC::IStream &os)
	{
	}

	void CMsgSetSender::load(NLMISC::IStream &is)
	{
	}

	void CMsgSetSender::getDebugResult(std::string &str,CCodeContext &context) const
	{		
		str += "MsgSetSender";
	}

	//*************************************
	// CAddParamNameDebug
	//*************************************

	CAddParamNameDebug::CAddParamNameDebug(const NLAIAGENT::IBaseGroupType &debugAttrib)
	{
		_DebugAttrib = (NLAIAGENT::IBaseGroupType *)debugAttrib.clone();
	}
	
	CAddParamNameDebug::~CAddParamNameDebug()
	{
		_DebugAttrib->release();
	}
	
	NLAIAGENT::TProcessStatement CAddParamNameDebug::runOpCode(CCodeContext &context)
	{
		context.ContextDebug.Param.push_back(_DebugAttrib);
		_DebugAttrib->incRef();
		return NLAIAGENT::processIdle;
	}
	
	void CAddParamNameDebug::getDebugResult(std::string &str,CCodeContext &context) const
	{		
		str = "Build a name liste that match the actual function parameters values.";
	}

	const NLAIC::IBasicType *CAddParamNameDebug::clone() const
	{
		NLAIC::IBasicType *x = new CAddParamNameDebug(*_DebugAttrib);
		return x;            
	}
	
	const NLAIC::IBasicType *CAddParamNameDebug::newInstance() const 
	{
		return clone();
	}

	const NLAIC::CIdentType &CAddParamNameDebug::getType() const
	{
		return IdAddParamNameDebug;
	}
	
	void CAddParamNameDebug::save(NLMISC::IStream &os)
	{		
		os.serial( (NLAIC::CIdentType &) _DebugAttrib->getType() );
		_DebugAttrib->save(os);
	}

	void CAddParamNameDebug::load(NLMISC::IStream &is) 
	{				
		_DebugAttrib->release();
		NLAIC::CIdentTypeAlloc id;
		is.serial( id );
		_DebugAttrib =  (NLAIAGENT::IBaseGroupType *)id.allocClass();
		_DebugAttrib->load(is);
	}	
}
