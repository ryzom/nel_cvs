/** \file operation.cpp
 * <File description>
 *
 * $Id: operation.cpp,v 1.2 2002/05/27 15:47:21 chafik Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX D.T.C. SYSTEM.
 * NEVRAX D.T.C. SYSTEM is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX D.T.C. SYSTEM is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX D.T.C. SYSTEM; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */
#include "nel/ai/agent/operation.h"
#include "nel/ai/script/object_unknown.h"
#include "nel/ai/script/interpret_object_message.h"


namespace NLAIAGENT
{	
	const NLAIC::CIdentType *CAgentOperation::IdAgentOperation = NULL;
	const NLAIC::CIdentType *CAgentOperation::idMsgOnChangeMsg = NULL;
	const NLAIAGENT::IMessageBase *CAgentOperation::MsgOnChangeMsg = NULL;

	CAgentOperation::CAgentOperation():CAgentScript(NULL), _Op(NULL), _Name(NULL)
	{
	}

	CAgentOperation::CAgentOperation(IObjetOp *o):CAgentScript(NULL), _Op(o), _Name(NULL)
	{

	}

	CAgentOperation::CAgentOperation(const CAgentOperation &a):CAgentScript(a), _Op(a._Op), _Connection(a._Connection), _Name(NULL)
	{
		if(_Op != NULL)
				_Op->incRef();

		if(a._Name != NULL) 
				setName(*a._Name);

		std::list <IConnectIA *>::iterator it = _Connection.begin();
		while(it != _Connection.end())
		{			
			(*it)->connect(this);
			it ++;
		}
	}

	CAgentOperation::~CAgentOperation()
	{
		if(_Op != NULL) 
				_Op->release();

		std::list <IConnectIA *>::iterator it = _Connection.begin();
		while(it != _Connection.end())
		{			
			(*it)->removeConnection(this);
			it ++;
		}
	}

	void CAgentOperation::onKill(IConnectIA *A)
	{
		std::list <IConnectIA *>::iterator it = _Connection.begin();
		while(it != _Connection.end())
		{			
			if((*it) == A)
			{				
				std::list < IConnectIA * >::iterator itTmp = it ++;
				_Connection.erase(itTmp);
				continue;
			}
			it ++;
		}
	}

	IObjectIA::CProcessResult CAgentOperation::runActivity()
	{
		if(changed())
		{
			std::list < IConnectIA * >::iterator it = _Connection.begin();
			while(it != _Connection.end())
			{
				NLAIAGENT::IMessageBase *msg = (NLAIAGENT::IMessageBase *)CAgentOperation::MsgOnChangeMsg->clone();
				msg->push(_Name);
				_Name->incRef();
				msg->push(_Op);
				_Op->incRef();
				(*it)->sendMessage(msg);
				it ++;
			}
			changeIsDone();
		}
		return CAgentScript::runActivity();
	}

	const IObjectIA::CProcessResult &CAgentOperation::run()
	{
		return CAgentScript::run();
	}

	bool CAgentOperation::isEqual(const IBasicObjectIA &a) const
	{
		if(CAgentScript::isEqual((const CAgentScript &)*this))
		{
			if(_Op) 
				return _Op->isEqual((const IObjetOp &)*this);		
		}
		return false;
	}

	void CAgentOperation::getDebugString(std::string &t) const
	{
		t += "CAgentOperation";
	}


	const NLAIC::IBasicType *CAgentOperation::clone() const
	{
		return (const CAgentScript *)new CAgentOperation(*this);

	}

	const NLAIC::IBasicType *CAgentOperation::newInstance() const
	{
		return (const CAgentScript *)new CAgentOperation();
	}


	void CAgentOperation::load(NLMISC::IStream &is)
	{
		
	}

	void CAgentOperation::save(NLMISC::IStream &os)
	{

	}

	IObjetOp &CAgentOperation::neg()
	{	
		return _Op->neg();
	}

	IObjetOp &CAgentOperation::operator += (const IObjetOp &a)
	{
		update();
		return (*_Op) += a;
	}

	IObjetOp &CAgentOperation::operator -= (const IObjetOp &a)
	{
		update();
		return (*_Op) -= a;
	}

	IObjetOp &CAgentOperation::operator *= (const IObjetOp &a)
	{
		update();
		return (*_Op) *= a;
	}

	IObjetOp &CAgentOperation::operator /= (const IObjetOp &a)
	{
		update();
		return (*_Op) /= a;
	}

	bool CAgentOperation::isTrue() const
	{
		if(_Op)
			return _Op->isTrue();
		return false;
	}


	int CAgentOperation::getBaseMethodCount() const
	{
		return CAgentScript::getBaseMethodCount() + CAgentOperation::TLastM;
	}

	sint32 CAgentOperation::getMethodIndexSize() const
	{
		return CAgentScript::getMethodIndexSize() + CAgentOperation::TLastM;
	}

	NLAIAGENT::tQueue CAgentOperation::isMember(const IVarName *className,const IVarName *methodName,const IObjectIA &param) const
	{
#ifdef NL_DEBUG
		const char *dbg_method_name = methodName->getString();
		std::string buffer;
		param.getDebugString(buffer);
#endif

		NLAIAGENT::tQueue r = NLAIAGENT::isTemplateMember(CAgentOperation::StaticMethod,CAgentOperation::TLastM, CAgentScript::getMethodIndexSize(),
															className, methodName, param);
		if(r.size()) return r;
		else return CAgentScript::isMember(className,methodName,param);
	}		

	NLAIAGENT::IObjectIA::CProcessResult CAgentOperation::runMethodBase(int heritance, int index,NLAIAGENT::IObjectIA *o)
	{		
		return runMethodBase(index,o);
	}

	NLAIAGENT::IObjectIA::CProcessResult CAgentOperation::runMethodBase(int index, IObjectIA *o)
	{
		NLAIAGENT::IObjectIA::CProcessResult r;
		NLAIAGENT::IBaseGroupType *param = (NLAIAGENT::IBaseGroupType *)o;
		switch(index - CAgentScript::getMethodIndexSize())
		{		
		case CAgentOperation::TSetValue:
			{
				IObjetOp *obj = (NLAIAGENT::IObjetOp *)param->get();
				setValue(obj);
				obj->incRef();
			}
			return r;

		case CAgentOperation::TGetValue:
			r.Result = _Op;
			_Op->incRef();
			return r;

		case CAgentOperation::TSetName:
			{
				NLAIAGENT::CStringType s(*(NLAIAGENT::CStringType *)param->get());
				setName(s);			
			}
			return r;

		case CAgentOperation::TGetName:
			r.Result = _Name;
			_Name->release();
			return r;

		case CAgentOperation::TUpdate:
			update();
			return r;

		case CAgentOperation::TIsChange:		
			r.Result = new NLAIAGENT::DDigitalType(changed() ? 1.0 : 0.0);
			return r;

		case CAgentOperation::TConnect:
			{
				IConnectIA *c = (IConnectIA *)param->get();
				connectOnChange(c);
			}
			return r;
		

		default:
			return CAgentScript::runMethodBase(index,o);
		}
	}

	NLAIAGENT::CAgentScript::CMethodCall **CAgentOperation::StaticMethod = NULL;

	void CAgentOperation::initClass()
	{
		CAgentOperation a;
		CAgentOperation::IdAgentOperation = new NLAIC::CIdentType("AgentOperation", NLAIC::CSelfClassFactory( (const NLAIC::IBasicInterface &)((const CAgentScript &)a) ),
																NLAIC::CTypeOfObject::tAgent | NLAIC::CTypeOfObject::tInterpret, NLAIC::CTypeOfOperator::opEq );

		CAgentOperation::idMsgOnChangeMsg = new NLAIC::CIdentType("OnChangeMsg");
		CAgentOperation::MsgOnChangeMsg = (NLAIAGENT::IMessageBase *)CAgentOperation::idMsgOnChangeMsg->allocClass();


		CAgentOperation::StaticMethod = new NLAIAGENT::CAgentScript::CMethodCall *[CAgentOperation::TLastM];

		CAgentOperation::StaticMethod[CAgentOperation::TSetValue] = 
						new CAgentScript::CMethodCall(	"SetValue",
														CAgentOperation::TSetValue,
														NULL,CAgentScript::CheckCount, 1, new NLAISCRIPT::CObjectUnknown(new NLAISCRIPT::COperandVoid));

		CAgentOperation::StaticMethod[CAgentOperation::TGetValue] = 
						new CAgentScript::CMethodCall(	"GetValue",
														CAgentOperation::TGetValue,
														NULL,CAgentScript::CheckCount, 0, new NLAISCRIPT::CObjectUnknown(new NLAISCRIPT::COperandAnyObject));

		CAgentOperation::StaticMethod[CAgentOperation::TSetName] = 
						new CAgentScript::CMethodCall(	"SetName",
														CAgentOperation::TSetName,
														NULL,CAgentScript::CheckCount, 1, new NLAISCRIPT::CObjectUnknown(new NLAISCRIPT::COperandVoid));

		CAgentOperation::StaticMethod[CAgentOperation::TGetName] = 
						new CAgentScript::CMethodCall(	"GetName",
														CAgentOperation::TGetName,
														NULL,CAgentScript::CheckCount, 0, new NLAISCRIPT::CObjectUnknown(new NLAISCRIPT::COperandAnyObject));


		CAgentOperation::StaticMethod[CAgentOperation::TUpdate] = 
						new CAgentScript::CMethodCall(	"Update",
														CAgentOperation::TUpdate,
														NULL,CAgentScript::CheckCount, 0, new NLAISCRIPT::CObjectUnknown(new NLAISCRIPT::COperandVoid));

		CAgentOperation::StaticMethod[CAgentOperation::TIsChange] = 
						new CAgentScript::CMethodCall(	"IsChange",
														CAgentOperation::TIsChange,
														NULL,CAgentScript::CheckCount, 0, new NLAISCRIPT::CObjectUnknown(	
																								new NLAISCRIPT::COperandSimple(
																								new NLAIC::CIdentType(NLAIAGENT::DDigitalType::IdDDigitalType))));
		CAgentOperation::StaticMethod[CAgentOperation::TConnect] = 
						new CAgentScript::CMethodCall(	"Connect",
														CAgentOperation::TConnect,
														NULL,CAgentScript::CheckCount, 1, new NLAISCRIPT::CObjectUnknown(new NLAISCRIPT::COperandVoid));

	}
	void CAgentOperation::releaseClass()
	{
		delete CAgentOperation::IdAgentOperation;
		delete CAgentOperation::idMsgOnChangeMsg;
		delete CAgentOperation::MsgOnChangeMsg;

		sint i;
		for(i = 0; i < CAgentOperation::TLastM; i++ )
		{
			delete CAgentOperation::StaticMethod[i];
		}
		delete CAgentOperation::StaticMethod;
	}
}
