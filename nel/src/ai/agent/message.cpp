/** \file message.cpp
 *
 * $Id: message.cpp,v 1.7 2001/01/23 14:26:24 portier Exp $
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
#include "nel/ai/agent/agent_mailer.h"
#include "nel/ai/logic/boolval.h"
#include "nel/ai/agent/object_type.h"
#include "nel/ai/agent/agent_method_def.h"
#include "nel/ai/agent/agent_digital.h"

namespace NLAIAGENT
{
	const static sint32 _TSender = 0;
	const static sint32 _TReceiver = 1;
	const static sint32 _TContinuation = 2;
	const static sint32 _MLastM = 3;

	IMessageBase::CMethodCall IMessageBase::_Method[] = 
	{
		IMessageBase::CMethodCall(_SENDER_,_TSender),		
		IMessageBase::CMethodCall(_RECEIVER_,_TReceiver),		
		IMessageBase::CMethodCall(_CONTINUATION_,_TContinuation)		
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
		_Sender = (IBasicAgent *)(&((const IBasicAgent &)b));
		setGroup((IBasicMessageGroup &)b.getGroup());
		*_Message = *b._Message;
		return *this;
	}

	bool IMessageBase::isEqual(const IBasicObjectIA &a) const
	{
		IMessageBase &b = (IMessageBase &)a;						
		return _Sender->isEqual((const IBasicAgent &)b) && getGroup() == b.getGroup() && _Message->isEqual((const IBaseGroupType&)b);
	}	

	void IMessageBase::save(NLMISC::IStream &os)
	{	
		
		if(_Sender != NULL)		
		{
			bool t = true;
			os.serial(t);
			IWordNumRef &r = (IWordNumRef&)((const IWordNumRef&)*_Sender);
			char *type = (char *) (const char *) r.getType();
			std::string x = std::string(type);
			os.serial(x);
			r.save(os);
		}
		else
		{
			bool t = false;
			os.serial(t);
		}				
		os.serial( (NLAIC::CIdentType &) _MsgGroup->getType() );
		os.serial( *_MsgGroup );
		os.serial( (NLAIC::CIdentType &) _Message->getType() );
		os.serial( *_Message );

		sint32 i = _Performatif;
		os.serial(i);
		os.serial(_comeFromC_PLUS);
	}
	
	void IMessageBase::load(NLMISC::IStream &is)
	{			
		bool t;
		is.serial(t);
		NLAIC::CIdentTypeAlloc id;
		if(t)
		{			
			is.serial( id );
			IWordNumRef *num = (IWordNumRef *)id.allocClass();
			num->load(is);
			_Sender = (IBasicAgent *)((const IRefrence *)*num);
			delete num;			
		}
		else
		{
			_Sender = NULL;
		}
		is.serial( id );
		if(_MsgGroup) delete _MsgGroup;
		_MsgGroup = (IBasicMessageGroup *)id.allocClass();
		is.serial( (IBasicMessageGroup &)*_MsgGroup );
		is.serial( id );
		if(_Message != NULL) _Message->release();
		_Message = (IBaseGroupType *)id.allocClass();
		is .serial( *_Message );
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
		_Message->getDebugString(b);
		char g[8*1024];
		_MsgGroup->getDebugString(g);
		sprintf(t,"IMessageBase<%d>:\n_sender:'%s' _MsgGroup:'%s' _Message:'%s'",this,a,g,b);
	}

	IObjetOp &IMessageBase::operator += (const IObjetOp &a)
	{
		_Message->cpy(a);
		return *this;
	}

	IObjetOp &IMessageBase::operator -= (const IObjetOp &a)
	{		
		_Message->erase(a);
		return *this;
	}
	

	const IObjectIA *IMessageBase::operator[] (sint32 index) const
	{	
		return (*_Message)[index];
	}	

	const IObjectIA::CProcessResult &IMessageBase::run()
	{
		return _Message->run();
	}

	bool IMessageBase::isTrue() const
	{
		return _Message->isTrue();
	}
	IObjetOp *IMessageBase::operator ! () const
	{
		return !(*_Message);
	}
	void IMessageBase::push(const IObjectIA *o)
	{
		_Message->push(o);
	}

	void IMessageBase::pushFront(const IObjectIA *o)
	{
		_Message->pushFront(o);
	}
	
	void IMessageBase::cpy(const IObjectIA &o)
	{
		_Message->cpy(o);
	}
	const IObjectIA *IMessageBase::pop()
	{
		return _Message->pop();
	}
	const IObjectIA *IMessageBase::get()
	{
		return _Message->get();
	}
	const IObjectIA *IMessageBase::popFront()
	{
		return _Message->popFront();
	}
	const IObjectIA *IMessageBase::getFront()
	{
		return _Message->getFront();
	}
	sint32 IMessageBase::size() const
	{
		return _Message->size();
	}

	const IObjectIA *IMessageBase::find(const IObjectIA &obj) const
	{
		return _Message->find(obj);
	}
	void IMessageBase::eraseFirst(const IObjectIA &obj)
	{
		_Message->erase(obj);
	}
	void IMessageBase::eraseAll(const IObjectIA &obj)
	{
		_Message->eraseAll(obj);
	}
	void IMessageBase::erase(const IObjectIA *o)
	{
		_Message->erase(o);
	}
	void IMessageBase::erase(const IObjectIA &obj)
	{
		_Message->erase(obj);
	}
	void IMessageBase::erase(std::list<const IObjectIA *> &l)
	{
		_Message->erase(l);
	}
	void IMessageBase::clear()
	{
		_Message->clear();
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
					CObjectType *c = new CObjectType(new NLAIC::CIdentType(CLocalAgentMail::LocalAgentMail));
					a.push( CIdMethod( IMessageBase::_Method[i].Index + IBaseGroupType::getMethodIndexSize(), 0.0, NULL, c) );					
					break;
				}
			}

			if ( a.size() )
				return a;
			else 
				return IBaseGroupType::isMember( className, methodName, p);
		}
		return IBaseGroupType::isMember(className,methodName,p);
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
						a.Result = NULL;
					return a;
				}			
				break;

			case _TReceiver		:
				{
					IObjectIA::CProcessResult a;				
					if ( _Receiver != NULL )
						a.Result = new CLocalAgentMail( (IBasicAgent *) _Receiver );				
					else
						a.Result = NULL;
					return a;
				}	
				break;

			case _TContinuation	:
				{
					IObjectIA::CProcessResult a;				
					if ( _Continuation != NULL )
						a.Result = new CLocalAgentMail( (IBasicAgent *) _Continuation );				
					else
						a.Result = NULL;

					return a;
				}			
				break;
			}
		return IBaseGroupType::runMethodeMember(index,p);
	}


	const NLAIC::CIdentType &CMessage::getType() const
	{
		return IdMessage;
	}

	const NLAIC::CIdentType &CMessageVector::getType() const
	{
		return IdMessageVector;
	}
}
