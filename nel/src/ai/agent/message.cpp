/** \file message.cpp
 *
 * $Id: message.cpp,v 1.2 2001/01/08 10:48:01 chafik Exp $
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
#include "agent/agent.h"
#include "logic/boolval.h"

namespace NLAIAGENT
{

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

	const NLAIC::CIdentType &CMessage::getType() const
	{
		return IdMessage;
	}

	const NLAIC::CIdentType &CMessageVector::getType() const
	{
		return IdMessageVector;
	}
}
