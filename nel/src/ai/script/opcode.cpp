/** \file opcode.cpp
 *
 * $Id: opcode.cpp,v 1.1 2001/01/05 10:53:49 chafik Exp $
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
#include "script/compilateur.h"
#include "agent/agent_script.h"
#include "script/type_def.h"
#include "script/object_load.h"


namespace NLIASCRIPT
{	

	NLIAAGENT::TProcessStatement CAffMemberiOpCode::runOpCode(CCodeContext &context)
	{				
		NLIAAGENT::IObjectIA *a = ((NLIAAGENT::IObjectIA *)context.Self);
		std::list<sint32>::iterator i = _I.begin();
		sint32 n = _I.size() - 1;
		while(n --)
		{			
			a = (NLIAAGENT::IObjectIA *)a->getStaticMember(*i++);
		}
		NLIAAGENT::IObjectIA *obj = ((NLIAAGENT::IObjectIA *)context.Stack);
		obj->incRef();
		a->setStaticMember(*i,obj);
		context.Stack --;
		return NLIAAGENT::processIdle;
	}

	NLIAAGENT::TProcessStatement CAffMemberOpCode::runOpCode(CCodeContext &context)
	{	
		NLIAAGENT::IObjectIA *a = ((NLIAAGENT::IObjectIA *)context.Stack);
		a->incRef();
		((NLIAAGENT::IObjectIA *)context.Self)->setStaticMember(_I,a);
		context.Stack --;
		return NLIAAGENT::processIdle;
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

	NLIAAGENT::TProcessStatement CLocAllocDebug::runOpCode(CCodeContext &context)
	{				
		/*NLIAAGENT::iObjectIA *i = (NLIAAGENT::iObjectIA *)NLIAC::getRegistry()->createInstance(_b);
		context.Heap[(sint32)context.Heap] = NULL;*/
		context.Heap ++;
		context.ContextDebug.HeapDebug ++;
		//context.ContextDebug->HeapDebug[context.ContextDebug->HeapDebug]
		return NLIAAGENT::processIdle;
	}

	void CLocAllocDebug::getDebugResult(char *str,CCodeContext &context) const
	{					
		sprintf(str,"Reservation d'un espace memoir locale pour une variable à l'emplacement %d",(sint32)context.Heap);				
	}

	const NLIAC::IBasicType *CLocAllocDebug::clone() const
	{
		NLIAC::IBasicType *x = new CLocAllocDebug();
		x->incRef();         
		return x;            
	}
	const NLIAC::IBasicType *CLocAllocDebug::newInstance() const 
	{
		return clone();
	}

	const NLIAC::CIdentType &CLocAllocDebug::getType() const
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
	

	NLIAAGENT::TProcessStatement CAffHeapMemberiOpCode::runOpCode(CCodeContext &context)
	{	
		
		NLIAAGENT::IObjectIA *a = ((NLIAAGENT::IObjectIA *)context.Heap[(int)_N]);			
		std::list<sint32>::iterator i = _I.begin();
		sint32 n = _I.size() - 1;
		while(n --)
		{
			a = (NLIAAGENT::IObjectIA *)a->getStaticMember(*i++);
		}		
		NLIAAGENT::IObjectIA *obj = ((NLIAAGENT::IObjectIA *)context.Stack);
		obj->incRef();
		a->setStaticMember(*i,obj);			
		context.Stack --;			
		return NLIAAGENT::processIdle;
	}

	void CAffHeapMemberiOpCode::getDebugResult(char *str,CCodeContext &context) const
	{	
		char txt[1024*8];
		char txtClass[1024*8];
		((NLIAAGENT::IObjectIA *)context.Stack)->getDebugString(txt);
		((NLIAAGENT::IObjectIA *)context.Heap[(int)_N])->getDebugString(txtClass);
		NLIAAGENT::IObjectIA *a = ((NLIAAGENT::IObjectIA *)context.Heap[(int)_N]);			
		std::list<sint32>::const_iterator i = _I.begin();
		sint32 n = _I.size();
		sprintf(str,"Affecte le membre sur le heap : %s ",txtClass);
		while(n --)
		{
			sprintf(&str[strlen(str)],"%d",*i++);
			if(n) strcat(str,"->");
		}		
		strcat(str," a la valeur: ");
		strcat(str,txt);
	}

	NLIAAGENT::TProcessStatement CAffOpCodeDebug::runOpCode(CCodeContext &context)
	{	
		/*NLIAAGENT::iObjectIA *i = (NLIAAGENT::iObjectIA *)NLIAC::getRegistry()->createInstance(_b);
		context.Heap[(sint32)context.Heap] = i;
		context.Heap ++;*/

		NLIAAGENT::IObjectIA *i = (NLIAAGENT::IObjectIA *)context.Stack;
		NLIAAGENT::CStringVarName vn(_VarName);
		NLIAAGENT::IObjectIA *iD = (NLIAAGENT::IObjectIA *) new NLIAAGENT::CStringType(vn);
		i->incRef();
		iD->incRef();
		NLIAAGENT::IObjectIA *tmp = context.Heap[(int)_I];
		NLIAAGENT::IObjectIA *tmpD = context.ContextDebug.HeapDebug[(int)_I];
		context.Heap[_I] = i;
		context.ContextDebug.HeapDebug[_I] = iD;
		if(tmp) tmp->release();
		if(tmpD) tmpD->release();
		//context.Heap ++;
		context.Stack --;
		return NLIAAGENT::processIdle;
	}

	void CAffOpCodeDebug::getDebugResult(char *str,CCodeContext &context) const
	{
		char Y[1024*8];					
		context.Stack[(int)context.Stack]->getDebugString(Y);
		sprintf(str,"Affectation<%d> a %s",_I,Y);	
	}

	const NLIAC::IBasicType *CAffOpCodeDebug::clone() const
	{
		NLIAC::IBasicType *x = new CAffOpCodeDebug(_I, _VarName);
		x->incRef();         
		return x;            
	}
	const NLIAC::IBasicType *CAffOpCodeDebug::newInstance() const 
	{
		return clone();
	}

	const NLIAC::CIdentType &CAffOpCodeDebug::getType() const
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
	
	NLIAAGENT::TProcessStatement CFreeAllocDebug::runOpCode(CCodeContext &context)
	{				
		sint32 i = (sint32)context.Heap;	
		i -= context.Heap.popMark();
		context.Heap -= i;

		sint32 j = (sint32)context.ContextDebug.HeapDebug;	
//		j -= context.ContextDebug.HeapDebug.popMark();
		context.ContextDebug.HeapDebug -= j;

		return NLIAAGENT::processIdle;
	}
	void CFreeAllocDebug::getDebugResult(char *str,CCodeContext &context) const
	{		
		sprintf(str,"libere le bloc de memoire de %d à %d",(sint32)context.Heap.mark(),(sint32)context.Heap);	
	}

	const NLIAC::IBasicType *CFreeAllocDebug::clone() const
	{
		NLIAC::IBasicType *x = new CFreeAllocDebug();
		x->incRef();         
		return x;            
	}
	const NLIAC::IBasicType *CFreeAllocDebug::newInstance() const 
	{
		return clone();
	}

	const NLIAC::CIdentType &CFreeAllocDebug::getType() const
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
	NLIAAGENT::TProcessStatement CMarkMsg::runOpCode(CCodeContext &context)
	{
		NLIAAGENT::IBaseGroupType *param = (NLIAAGENT::IBaseGroupType *)context.Stack[(int)context.Stack];
		NLIAAGENT::IMessageBase *msg = (NLIAAGENT::IMessageBase *)param->get();
		msg->setMethodIndex(_Heritance,_I);
		return NLIAAGENT::processIdle;
	}

	//*************************************
	// CFindRunMsg
	//*************************************

	CFindRunMsg::CFindRunMsg(NLIAAGENT::IBaseGroupType *methodName,CParam *param,IOpType *baseClass,ILoadObject *objectLoad)
	{
		_Param = param;
		_MethodName = methodName;
		_BaseClass = baseClass;		
		_ObjectLoad = objectLoad;
	}

	CFindRunMsg::CFindRunMsg(const NLIAAGENT::IBaseGroupType &methodName,const CParam &param,const IOpType &baseClass,const ILoadObject &objectLoad)
	{
		_Param = (CParam *)param.clone();
		_MethodName = (NLIAAGENT::IBaseGroupType *)methodName.clone();
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
	NLIAAGENT::TProcessStatement CFindRunMsg::runOpCode(CCodeContext &context)
	{
		const NLIAAGENT::IObjectIA *o = _ObjectLoad->getObject(context);
		NLIAAGENT::IBaseGroupType *param = (NLIAAGENT::IBaseGroupType *)_ObjectLoad->getParam(context);
		NLIAAGENT::IMessageBase *msg = (NLIAAGENT::IMessageBase *)param->get();

		const NLIAAGENT::CStringType *name = (const NLIAAGENT::CStringType *)_MethodName->get();

		NLIAAGENT::tQueue a = o->isMember(NULL,&name->getStr(),*_Param);

		if(a.size())
		{
			NLIAAGENT::CIdMethod m = a.top();
			msg->setMethodIndex(0,m.Index);
		}

		return NLIAAGENT::processIdle;
	}
	void CFindRunMsg::getDebugResult(char *str,CCodeContext &context) const
	{
		char m[1024*4];
		char p[1024*4];
		_MethodName->getDebugString(m);
		_Param->getDebugString(p);
		sprintf(str,"find '%s%s' for %s",m,p, (const char *)context.Self->getType());
	}

	const NLIAC::IBasicType *CFindRunMsg::clone() const
	{
		_Param->incRef();
		_MethodName->incRef();
		_BaseClass->incRef();
		_ObjectLoad->incRef();
		NLIAC::IBasicType *clone = new CFindRunMsg(_MethodName,_Param,_BaseClass,_ObjectLoad);
		clone->incRef();
		return clone;
	}

	void CFindRunMsg::save(NLMISC::IStream &os)
	{	
		os.serial( (NLIAC::CIdentType &) _ObjectLoad->getType() );
		_ObjectLoad->save(os);
		_BaseClass->serial(os);
		os.serial( (NLIAC::CIdentType &) _Param->getType() );
		_Param->save(os);
		os.serial( (NLIAC::CIdentType &) _MethodName->getType() );
		_MethodName->save(os);
	}

	void CFindRunMsg::load(NLMISC::IStream &is) 
	{
		NLIAC::CIdentTypeAlloc id;
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
		_MethodName = (NLIAAGENT::IBaseGroupType *)id.allocClass();
		_MethodName->load(is);
	}		

	//*************************************
	// CAddParamNameDebug
	//*************************************

	CAddParamNameDebug::CAddParamNameDebug(const NLIAAGENT::IBaseGroupType &debugAttrib)
	{
		_DebugAttrib = (NLIAAGENT::IBaseGroupType *)debugAttrib.clone();
	}
	
	CAddParamNameDebug::~CAddParamNameDebug()
	{
		_DebugAttrib->release();
	}
	
	NLIAAGENT::TProcessStatement CAddParamNameDebug::runOpCode(CCodeContext &context)
	{
		context.ContextDebug.Param.push_back(_DebugAttrib);
		_DebugAttrib->incRef();
		return NLIAAGENT::processIdle;
	}
	
	void CAddParamNameDebug::getDebugResult(char *str,CCodeContext &context) const
	{		
		sprintf(str,"Build a name liste that match the actual function parameters values.");
	}

	const NLIAC::IBasicType *CAddParamNameDebug::clone() const
	{
		NLIAC::IBasicType *x = new CAddParamNameDebug(*_DebugAttrib);
		x->incRef();
		return x;            
	}
	
	const NLIAC::IBasicType *CAddParamNameDebug::newInstance() const 
	{
		return clone();
	}

	const NLIAC::CIdentType &CAddParamNameDebug::getType() const
	{
		return IdAddParamNameDebug;
	}
	
	void CAddParamNameDebug::save(NLMISC::IStream &os)
	{		
		os.serial( (NLIAC::CIdentType &) _DebugAttrib->getType() );
		_DebugAttrib->save(os);
	}

	void CAddParamNameDebug::load(NLMISC::IStream &is) 
	{				
		_DebugAttrib->release();
		NLIAC::CIdentTypeAlloc id;
		is.serial( id );
		_DebugAttrib =  (NLIAAGENT::IBaseGroupType *)id.allocClass();
		_DebugAttrib->load(is);
	}
	
}
