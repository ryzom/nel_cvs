/** \file list_manager.cpp
 *
 * $Id: list_manager.cpp,v 1.1 2001/03/01 13:44:01 chafik Exp $
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
#include "nel/ai/agent/agent_object.h"
#include "nel/ai/agent/list_manager.h"

namespace NLAIAGENT
{

/*
	########################
	class IListBasicManager
	########################
*/
	IListBasicManager::IListBasicManager():_List(NULL)
	{
	}
	IListBasicManager::IListBasicManager(IBaseGroupType *l):_List(l)
	{

	}

	IListBasicManager::IListBasicManager(const IListBasicManager &l):_List(l._List)
	{
		l._List->incRef();
	}

	IListBasicManager::~IListBasicManager()
	{
		if(_List != NULL) _List->release();
	}

	void IListBasicManager::save(NLMISC::IStream &os)
	{
		os.serial( (NLAIC::CIdentType &) _List->getType() );
		os.serial( *_List );
	}

	void IListBasicManager::load(NLMISC::IStream &is)
	{
		NLAIC::CIdentTypeAlloc id;
		is.serial( id );
		if(_List != NULL) _List->release();
		_List = (IBaseGroupType *)id.allocClass();
		is .serial( *_List );
	}	

	void IListBasicManager::getDebugString(char *t) const
	{		
		char b[8*1024];
		_List->getDebugString(b);		
		sprintf(t,"IListBasicManager<%4x>: _List:'%s'",this,b);
	}

	IObjectIA &IListBasicManager::operator = (const IObjectIA &b)
	{		
		IListBasicManager &a = (IListBasicManager &)b;
		*_List = *a._List;
		return *this;
	}

	IObjetOp &IListBasicManager::operator += (const IObjetOp &a)
	{
		_List->cpy(a);
		return *this;
	}

	IObjetOp &IListBasicManager::operator -= (const IObjetOp &a)
	{		
		_List->erase(a);
		return *this;
	}
	

	const IObjectIA *IListBasicManager::operator[] (sint32 index) const
	{	
		return (*_List)[index];
	}	

	bool IListBasicManager::isTrue() const
	{
		return _List->isTrue();
	}
	IObjetOp *IListBasicManager::operator ! () const
	{
		return !(*_List);
	}
	void IListBasicManager::push(const IObjectIA *o)
	{
		_List->push(o);
	}

	void IListBasicManager::pushFront(const IObjectIA *o)
	{
		_List->pushFront(o);
	}
	
	void IListBasicManager::cpy(const IObjectIA &o)
	{
		_List->cpy(o);
	}
	const IObjectIA *IListBasicManager::pop()
	{
		return _List->pop();
	}
	const IObjectIA *IListBasicManager::get() const
	{
		return _List->get();
	}
	const IObjectIA *IListBasicManager::popFront()
	{
		return _List->popFront();
	}
	const IObjectIA *IListBasicManager::getFront() const
	{
		return _List->getFront();
	}
	sint32 IListBasicManager::size() const
	{
		return _List->size();
	}

	const IObjectIA *IListBasicManager::find(const IObjectIA &obj) const
	{
		return _List->find(obj);
	}
	void IListBasicManager::eraseFirst(const IObjectIA &obj)
	{
		_List->erase(obj);
	}
	void IListBasicManager::eraseAll(const IObjectIA &obj)
	{
		_List->eraseAll(obj);
	}
	void IListBasicManager::erase(const IObjectIA *o)
	{
		_List->erase(o);
	}
	void IListBasicManager::erase(const IObjectIA &obj)
	{
		_List->erase(obj);
	}
 
	void IListBasicManager::erase(std::list<const IObjectIA *> &l)
	{
		_List->erase(l);
	}
	void IListBasicManager::clear()
	{
		_List->clear();
	}	

	bool IListBasicManager::isEqual(const IBasicObjectIA &b) const	
	{
		IListBasicManager &a = (IListBasicManager &)b;
		return _List->isEqual(*a._List);
	}

	sint32 IListBasicManager::getMethodIndexSize() const
	{
		return IBaseGroupType::getMethodIndexSize() + 0;
	}

	tQueue IListBasicManager::isMember(const IVarName *className,const IVarName *methodName,const IObjectIA &p) const
	{
		return IBaseGroupType::isMember(className,methodName,p);
	}

	IObjectIA::CProcessResult IListBasicManager::runMethodeMember(sint32 h, sint32 m,IObjectIA *p)
	{
		return IBaseGroupType::runMethodeMember(h,m,p);
	}

	IObjectIA::CProcessResult IListBasicManager::runMethodeMember(sint32 m,IObjectIA *p)
	{
		return IBaseGroupType::runMethodeMember(m,p);
	}

/*
	########################
	class IListManager
	########################
*/
	IListManager::IListManager(IBaseGroupType *l):IListBasicManager(l)
	{

	}

	IListManager::IListManager(const IListManager &l):IListBasicManager(l._List)
	{		
	}

	IListManager::~IListManager()
	{	
	}

	bool IListManager::isEqual(const IBasicObjectIA &a) const
	{
		const IListManager &l = (const IListManager &)a;
		return l._List == _List;
	}

	IObjectIA::CProcessResult IListManager::sendMessage(IObjectIA *msg)
	{
		CIteratorContener itr = _List->getIterator();

		while(!itr.isInEnd())
		{
			IObjectIA *obj = (IObjectIA *)(itr++);
			obj->sendMessage(msg);
		}
		return IObjectIA::CProcessResult();
	}

/*
	########################
	class CVectorListManager
	########################
*/
	CVectorListManager::CVectorListManager(sint32 i):IListManager(new CVectorGroupType(i))
	{
	}

	CVectorListManager::CVectorListManager():IListManager(new CVectorGroupType())
	{
	}

	CVectorListManager::CVectorListManager(const CVectorListManager &l):IListManager(l)
	{
	}

	CVectorListManager::~CVectorListManager()
	{
	}

	const NLAIC::CIdentType &CVectorListManager::getType() const
	{
		return IdVectorListManager;
	}

	const NLAIC::IBasicType *CVectorListManager::clone() const
	{
		return new CVectorListManager(*this);
	}

	const NLAIC::IBasicType *CVectorListManager::newInstance() const
	{
		return new CVectorListManager();
	}

	void CVectorListManager::trie()
	{

	}

	const IObjectIA *CVectorListManager::getObject(const CObjectIdent &) const
	{
		return NULL;
	}
}