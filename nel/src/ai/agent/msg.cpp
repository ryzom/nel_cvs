/** \file message.cpp
 *
 * $Id: msg.cpp,v 1.6 2001/03/26 14:50:01 chafik Exp $
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
#include "nel/ai/agent/agent.h"
#include "nel/ai/agent/agent_local_mailer.h"
#include "nel/ai/agent/agent_proxy_mailer.h"
#include "nel/ai/logic/boolval.h"
#include "nel/ai/agent/object_type.h"
#include "nel/ai/agent/agent_method_def.h"
#include "nel/ai/agent/agent_digital.h"

namespace NLAIAGENT
{
	const static sint32 _TSender = 0;
	const static sint32 _TReceiver = 1;
	const static sint32 _TContinuation = 2;	
	const static sint32 _TSetContinuation = 3;
	const static sint32 _MLastM = 4;

	IMessageBase::CMethodCall IMessageBase::_Method[] = 
	{
		IMessageBase::CMethodCall(_SENDER_,_TSender),		
		IMessageBase::CMethodCall(_RECEIVER_,_TReceiver),		
		IMessageBase::CMethodCall(_CONTINUATION_,_TContinuation),
		IMessageBase::CMethodCall(_SETCONTINUATION_,_TSetContinuation)
		
	};

	/*IntegerType IMessageBase::IdExec = IntegerType(IMessageBase::PExec);
	IntegerType IMessageBase::IdAchieve = IntegerType(IMessageBase::PAchieve);
	IntegerType IMessageBase::IdAsk = IntegerType(IMessageBase::PAsk);
	IntegerType IMessageBase::IdBreak = IntegerType(IMessageBase::PBreak);
	IntegerType IMessageBase::IdTell = IntegerType(IMessageBase::PTell);
	IntegerType IMessageBase::IdKill = IntegerType(IMessageBase::PKill);*/


	IObjectIA &IMessageBase::operator = (const IObjectIA &a)
	{
		IMessageBase &b = (IMessageBase &)a;
		_Sender = b._Sender;
		if(_Sender) _Sender->incRef();
		_Receiver = b._Receiver;
		if(_Receiver) _Receiver->incRef();
		_Continuation = b._Continuation;
		if(_Continuation) _Continuation->incRef();
		setGroup((IBasicMessageGroup &)b.getGroup());
		*_List = *b._List;
		return *this;
	}

	bool IMessageBase::isEqual(const IBasicObjectIA &a) const
	{
		IMessageBase &b = (IMessageBase &)a;						
		return _Sender->isEqual((const IBasicAgent &)b) && getGroup() == b.getGroup() && IListBasicManager::isEqual(a);
	}	

	void IMessageBase::save(NLMISC::IStream &os)
	{	
		IListBasicManager::save(os);
		if(_Sender != NULL)
		{
			bool t = true;
			os.serial(t);
			IWordNumRef &r = (IWordNumRef&)((const IWordNumRef&)*((IRefrence *)_Sender));
			((CNumericIndex &)r.getNumIdent()).save(os);			
		}
		else
		{
			bool t = false;
			os.serial(t);
		}

		if(_Receiver != NULL)
		{
			bool t = true;
			os.serial(t);
			IWordNumRef &r = (IWordNumRef&)((const IWordNumRef&)*((IRefrence *)_Receiver));
			((CNumericIndex &)r.getNumIdent()).save(os);			
		}
		else
		{
			bool t = false;
			os.serial(t);
		}

		if(_Continuation != NULL)
		{
			bool t = true;
			os.serial(t);
			IWordNumRef &r = (IWordNumRef&)((const IWordNumRef&)*((IRefrence *)_Continuation));
			((CNumericIndex &)r.getNumIdent()).save(os);			
		}
		else
		{
			bool t = false;
			os.serial(t);
		}


		os.serial( (NLAIC::CIdentType &) _MsgGroup->getType() );
		os.serial( *_MsgGroup );		

		sint32 i = _Performatif;
		os.serial(i);
		os.serial(_comeFromC_PLUS);
	}
	
	void IMessageBase::load(NLMISC::IStream &is)
	{
		IListBasicManager::load(is);
		bool t;
		is.serial(t);		
		if(t)
		{
			CNumericIndex r(is);
			IRefrence *ref = CLocWordNumRef::getRef(r);
			if(ref != NULL)
			{
				if(_Sender) _Sender->release();
				_Sender = ref;
				_Sender->incRef();
			}
			else
			{
				if(_Sender) _Sender->release();
				_Sender = new CProxyAgentMail(r.getId());
			}
			
		}
		else
		{
			if(_Sender) _Sender->release();
			_Sender = NULL;
		}
		
		is.serial(t);		
		if(t)
		{
			CNumericIndex r(is);
			IRefrence *ref = CLocWordNumRef::getRef(r);
			if(ref != NULL)
			{
				if(_Receiver) _Receiver->release();
				_Receiver = ref;
				_Receiver->incRef();
			}
			else
			{
				if(_Receiver) _Receiver->release();
				_Receiver = new CProxyAgentMail(r.getId());
			}
			
		}
		else
		{
			if(_Receiver) _Receiver->release();
			_Receiver = NULL;
		}
		is.serial(t);		
		if(t)
		{
			CNumericIndex r(is);
			IRefrence *ref = CLocWordNumRef::getRef(r);
			if(ref != NULL)
			{
				if(_Continuation) _Continuation->release();
				_Continuation = ref;
				_Continuation->incRef();
			}
			else
			{				
				if(_Continuation) _Continuation->release();
				_Continuation = new CProxyAgentMail(r.getId());
			}
			
		}
		else
		{
			if(_Continuation) _Continuation->release();
			_Continuation = NULL;
		}

		NLAIC::CIdentTypeAlloc id;
		is.serial( id );
		if(_MsgGroup) delete _MsgGroup;
		_MsgGroup = (IBasicMessageGroup *)id.allocClass();
		is.serial( (IBasicMessageGroup &)*_MsgGroup );				
		sint32 i = _Performatif;
		is.serial(i);
		_Performatif = (TPerformatif)i;
		is.serial(_comeFromC_PLUS);
	}

	void IMessageBase::getDebugString(char *t) const
	{
		char a[8*1024];
		_Sender->getDebugString(a);	
		char b[8*1024];
		IListBasicManager::getDebugString(b);
		char g[8*1024];
		_MsgGroup->getDebugString(g);
		sprintf(t,"IMessageBase<%d>:\n_sender:'%s' _MsgGroup:'%s' Message:'%s'",this,a,g,b);
	}
	


	sint32 IMessageBase::getMethodIndexSize() const
	{
		return IBaseGroupType::getMethodIndexSize() + _MLastM;
	}	

	tQueue IMessageBase::isMember(const IVarName *className,const IVarName *methodName,const IObjectIA &p) const
	{			
		if(className == NULL)
		{
			tQueue a;
			for(int i = 0; i < _MLastM; i++)
			{
				if( *methodName == IMessageBase::_Method[i].MethodName )
				{					
					CObjectType *c = new CObjectType(new NLAIC::CIdentType(*IAgent::IdAgent));
					a.push( CIdMethod( IMessageBase::_Method[i].Index + IBaseGroupType::getMethodIndexSize(), 0.0, NULL, c) );					
					break;
				}
			}

			if ( a.size() )
				return a;
			else 
				return IBaseGroupType::isMember( className, methodName, p);
		}
		return IListBasicManager::isMember(className,methodName,p);
	}

	IObjectIA::CProcessResult IMessageBase::runMethodeMember(sint32 h, sint32 index,IObjectIA *p)
	{
		return IBaseGroupType::runMethodeMember(h,index,p);
	}

	IObjectIA::CProcessResult IMessageBase::runMethodeMember(sint32 index,IObjectIA *p)
	{
		IBaseGroupType *param = (IBaseGroupType *)p;

		switch(index - IBaseGroupType::getMethodIndexSize())
		{
			case _TSender		:
				{
					IObjectIA::CProcessResult a;		
					if ( _Sender != NULL )
						a.Result = new CLocalAgentMail( (IBasicAgent *) _Sender );				
					else
					{
						a.Result = &DigitalType::NullOperator;
						a.Result->incRef();
					}
					return a;
				}			
				break;

			case _TReceiver		:
				{
					IObjectIA::CProcessResult a;				
					if ( _Receiver != NULL )
						a.Result = new CLocalAgentMail( (IBasicAgent *) _Receiver );				
					else
					{
						a.Result = &DigitalType::NullOperator;
						a.Result->incRef();
					}
					return a;
				}	
				break;

			case _TContinuation	:
				{
					IObjectIA::CProcessResult a;				
					if ( _Continuation != NULL )
						a.Result = new CLocalAgentMail( (IBasicAgent *) _Continuation );				
					else
					{
						a.Result = &DigitalType::NullOperator;
						a.Result->incRef();
					}
					return a;
				}			
				break;
			case _TSetContinuation:
				{
					IObjectIA *o = (IObjectIA *)((IBaseGroupType *)p)->get();
					o->incRef();
					setContinuation(o);
				}
			}
		return IBaseGroupType::runMethodeMember(index,p);
	}


	const NLAIC::CIdentType &CMessageList::getType() const
	{
		return IdMessage;
	}

	const NLAIC::CIdentType &CMessageVector::getType() const
	{
		return IdMessageVector;
	}
}
