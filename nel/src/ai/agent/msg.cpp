/** \file message.cpp
 *
 * $Id: msg.cpp,v 1.23 2003/01/13 16:58:59 chafik Exp $
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

	const NLAIC::IBasicType *CMessageGroup::clone() const 
	{
		NLAIC::IBasicType *x = new CMessageGroup( *this );

#ifdef NL_DEBUG
	static sint kaka = 0;
#endif
		return x;
	}

	const NLAIC::IBasicType *CMessageGroup::newInstance() const 
	{
		NLAIC::IBasicType *x = new CMessageGroup( *this );
		return x;
	}


	const static sint32 _TSender = 0;
	const static sint32 _TReceiver = 1;
	const static sint32 _TXchgReceiverIsSender = 2;
	const static sint32 _TContinuation = 3;	
	const static sint32 _TSetContinuation = 4;
	const static sint32 _TSetSender = 5;
	const static sint32 _TInitProtocol = 6;	
	const static sint32 _TProtcetSender = 7;		
	const static sint32 _MLastM = 8;

	IMessageBase::CMethodCall IMessageBase::_Method[] = 
	{
		IMessageBase::CMethodCall(_SENDER_,_TSender),		
		IMessageBase::CMethodCall(_RECEIVER_,_TReceiver),		
		IMessageBase::CMethodCall(_CONTINUATION_,_TContinuation),
		IMessageBase::CMethodCall(_SETCONTINUATION_,_TSetContinuation),
		IMessageBase::CMethodCall("XChangeReceiverSender",_TXchgReceiverIsSender),
		IMessageBase::CMethodCall("ProtcetSender",_TProtcetSender),
		IMessageBase::CMethodCall("InitProtocol",_TInitProtocol),
		IMessageBase::CMethodCall("SetSender",_TSetSender)		
		
	};

	/*IntegerType IMessageBase::IdExec = IntegerType(IMessageBase::PExec);
	IntegerType IMessageBase::IdAchieve = IntegerType(IMessageBase::PAchieve);
	IntegerType IMessageBase::IdAsk = IntegerType(IMessageBase::PAsk);
	IntegerType IMessageBase::IdBreak = IntegerType(IMessageBase::PBreak);
	IntegerType IMessageBase::IdTell = IntegerType(IMessageBase::PTell);
	IntegerType IMessageBase::IdKill = IntegerType(IMessageBase::PKill);*/

	IMessageBase::IMessageBase():IListBasicManager(),_Sender(NULL)
	{
		_ReservedMethodIndexVar = -1;
		_ReservedHeritanceIndexVar = 0;
		_Receiver = NULL;
		_Continuation = NULL;
		_Performatif = PUndefine;
		_comeFromC_PLUS = true;
		_Dispatch = false;
		_ProtectSender = false;
		_SenderIsVolatile = false;
		_ReceiverIsVolatile = false;		
		_ContinuationIsVolatile = false;
	}
	IMessageBase::IMessageBase(IObjectIA *sender,IBaseGroupType *g):IListBasicManager(g),_Sender(sender)
	{
		_ReservedMethodIndexVar = -1;
		_ReservedHeritanceIndexVar = 0;
		_Receiver = NULL;
		_Continuation = NULL;
		_Performatif = PUndefine;
		_comeFromC_PLUS = true;
		_Dispatch = false;
		_ProtectSender = false;
		_SenderIsVolatile = false;
		_ReceiverIsVolatile = false;		
		_ContinuationIsVolatile = false;
	}

	IMessageBase::IMessageBase(IObjectIA *sender, IBasicMessageGroup &msg_group,IBaseGroupType *g):
							IListBasicManager(g),_Sender(sender)
	{
		_ReservedMethodIndexVar = -1;
		_ReservedHeritanceIndexVar = 0;
		_Receiver = NULL;
		_Continuation = NULL;
		_Performatif = PUndefine;
		_comeFromC_PLUS = true;
		_Dispatch = false;
		_ProtectSender = false;
		_SenderIsVolatile = false;
		_ReceiverIsVolatile = false;		
		_ContinuationIsVolatile = false;
	}

	IMessageBase::IMessageBase(const IMessageBase &m):IListBasicManager(m._List != NULL ? (IBaseGroupType *)m._List->clone(): NULL)
	{
		_Sender = m._Sender;
		_SenderIsVolatile = m._SenderIsVolatile;
		if(_SenderIsVolatile && _Sender != NULL) _Sender->incRef();
		_Receiver = m._Receiver;
		_ReceiverIsVolatile = m._ReceiverIsVolatile;
		if(_ReceiverIsVolatile && _Receiver != NULL) _Receiver->incRef();
		_Continuation = m._Continuation;
		_ContinuationIsVolatile = m._ContinuationIsVolatile;
		if(_ContinuationIsVolatile && _Continuation != NULL) _Continuation->incRef();
		_ReservedMethodIndexVar = m._ReservedMethodIndexVar;
		_ReservedHeritanceIndexVar = m._ReservedHeritanceIndexVar;			
		_Performatif = m._Performatif;
		_comeFromC_PLUS = m._comeFromC_PLUS;
		_Dispatch = m._Dispatch;
		_ProtectSender = m._ProtectSender;
	}

	IMessageBase::~IMessageBase()
	{		
		if(_SenderIsVolatile && _Sender != NULL) _Sender->release();
		if(_ReceiverIsVolatile && _Receiver != NULL) _Receiver->release();
	    if(_ContinuationIsVolatile && _Continuation != NULL) _Continuation->release();
	}	


	IObjectIA &IMessageBase::operator = (const IObjectIA &a)
	{
		IMessageBase &b = (IMessageBase &)a;
		_Sender = b._Sender;
		if(_SenderIsVolatile && _Sender != NULL) _Sender->incRef();
		_Receiver = b._Receiver;
		if(_ReceiverIsVolatile && _Receiver != NULL) _Receiver->incRef();
		_Continuation = b._Continuation;
		if(_ContinuationIsVolatile && _Continuation != NULL) _Continuation->incRef();		
		*_List = *b._List;
		return *this;
	}

	void IMessageBase::setSender(IObjectIA *s, bool v)
	{					
		if(_SenderIsVolatile) _Sender->release();
		_Sender = s;
		_SenderIsVolatile = v;
	}

	void IMessageBase::setReceiver(IObjectIA *r, bool v)
	{	
		if(_ReceiverIsVolatile) _Receiver->release();
		_Receiver = r;
		_ReceiverIsVolatile = v;
	}

	void IMessageBase::setContinuation(IObjectIA *r, bool v)
	{					
		if(_ContinuationIsVolatile) _Continuation->release();
		_Continuation = r;
		_ContinuationIsVolatile = v;
	}

	bool IMessageBase::isEqual(const IBasicObjectIA &a) const
	{
		IMessageBase &b = (IMessageBase &)a;						
		return _Sender->isEqual((const IBasicAgent &)b) && IListBasicManager::isEqual(a);
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
				//if(_Sender) _Sender->release();
				_Sender = ref;
				//_Sender->incRef();
			}
			else
			{
				//if(_Sender) _Sender->release();
				setSender(new CProxyAgentMail(r.getId()) , true);
			}
			
		}
		else
		{			
			_Sender = NULL;
		}
		
		is.serial(t);		
		if(t)
		{
			CNumericIndex r(is);
			IRefrence *ref = CLocWordNumRef::getRef(r);
			if(ref != NULL)
			{
				//if(_Receiver) _Receiver->release();
				_Receiver = ref;
				//_Receiver->incRef();
			}
			else
			{
				//if(_Receiver) _Receiver->release();
				setReceiver(new CProxyAgentMail(r.getId()) , true);
			}
			
		}
		else
		{
			//if(_Receiver) _Receiver->release();
			_Receiver = NULL;
		}
		is.serial(t);		
		if(t)
		{
			CNumericIndex r(is);
			IRefrence *ref = CLocWordNumRef::getRef(r);
			if(ref != NULL)
			{
				//if(_Continuation) _Continuation->release();
				_Continuation = ref;
				//_Continuation->incRef();
			}
			else
			{				
				//if(_Continuation) _Continuation->release();
				setContinuation(new CProxyAgentMail(r.getId()) , true);
			}
			
		}
		else
		{
			//if(_Continuation) _Continuation->release();
			_Continuation = NULL;
		}
		
		sint32 i = _Performatif;
		is.serial(i);
		_Performatif = (TPerformatif)i;
		is.serial(_comeFromC_PLUS);
	}

	void IMessageBase::getDebugString(std::string &t) const
	{
		std::string a;
		if(_Sender != NULL) _Sender->getDebugString(a);	
		else a = "NULL";
		std::string b;
		IListBasicManager::getDebugString(b);		
		t += NLAIC::stringGetBuild("IMessageBase<%d>:\n_sender:'%s' Message:'%s' dispatch:%d",this,a.c_str(),b.c_str(),_Dispatch);
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

	IObjectIA::CProcessResult IMessageBase::sendMessage(IObjectIA *m)
	{
		return _Receiver->sendMessage(m);
	}
	
	IObjectIA::CProcessResult IMessageBase::sendMessage(const IVarName &name,IObjectIA *m)
	{
		return _Receiver->sendMessage(name,m);
	}

	IObjectIA::CProcessResult IMessageBase::runMethodeMember(sint32 h, sint32 index,IObjectIA *p)
	{
		return IBaseGroupType::runMethodeMember(h,index,p);
	}

	IObjectIA::CProcessResult IMessageBase::runMethodeMember(sint32 index,IObjectIA *p)
	{
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

			case _TReceiver	:
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
			
			case _TXchgReceiverIsSender	:
				{
					IObjectIA *t = _Sender;
					_Sender = _Receiver;
					_Receiver = t;
					return IObjectIA::CProcessResult();
				}
			case _TSetContinuation:
				{
					IObjectIA *o = (IObjectIA *)((IBaseGroupType *)p)->get();
					//o->incRef();
					setContinuation(o);
					return IObjectIA::CProcessResult();
				}

			case _TSetSender:
				{
					IObjectIA *obj = (IObjectIA *) ((IBaseGroupType *)p)->get();					
					CLocalAgentMail *o = dynamic_cast<CLocalAgentMail *>(obj);
					if(o != NULL)
						setSender((IObjectIA *)o->getHost());
					else
						setSender((IObjectIA *)obj);

					return IObjectIA::CProcessResult();
				}			

			case _TInitProtocol:
				{
					_ReservedMethodIndexVar = -1;
					_ReservedHeritanceIndexVar = 0;					
					return IObjectIA::CProcessResult();				
				}
				
			case _TProtcetSender:
				{
					setProtcetSender();					
					return IObjectIA::CProcessResult();				
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
