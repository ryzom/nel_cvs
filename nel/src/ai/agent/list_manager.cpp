/** \file list_manager.cpp
 *
 * $Id: list_manager.cpp,v 1.11 2003/01/21 11:24:39 chafik Exp $
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

	void IListBasicManager::getDebugString(std::string &t) const
	{		
		std::string b;
		_List->getDebugString(b);
		t += NLAIC::stringGetBuild("IListBasicManager<%4x>: _List:'%s'",this,b.c_str());
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
#ifdef NL_DEBUG
		sint s = _List->size();
#endif
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

	TQueue IListBasicManager::isMember(const IVarName *className,const IVarName *methodName,const IObjectIA &p) const
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
		l._List->incRef();
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

		if(itr.isInEnd())
		{
			msg->release();
		}
		else
		{
			while(!itr.isInEnd())
			{
				IObjectIA *obj = (IObjectIA *)(itr++);
				obj->sendMessage(msg);
				if(!itr.isInEnd()) msg->incRef();
			}
		}
		return IObjectIA::CProcessResult();
	}

/*
	########################
	class CVectorGroupManager
	########################
*/
	CVectorGroupManager::CVectorGroupManager(sint32 i):IListManager(new CVectorGroupType(i))
	{
	}

	CVectorGroupManager::CVectorGroupManager():IListManager(new CVectorGroupType())
	{
	}

	CVectorGroupManager::CVectorGroupManager(const CVectorGroupManager &l):IListManager(l)
	{
	}

	CVectorGroupManager::~CVectorGroupManager()
	{
	}

	const NLAIC::CIdentType &CVectorGroupManager::getType() const
	{
		return IdVectorGroupManager;
	}

	const NLAIC::IBasicType *CVectorGroupManager::clone() const
	{
		NLAIC::IBasicType *x = new CVectorGroupManager(*this);
		return x;
	}

	const NLAIC::IBasicType *CVectorGroupManager::newInstance() const
	{
		return new CVectorGroupManager();
	}

	void CVectorGroupManager::trie()
	{
		// TODO : Sort the agent on the ID value.
		throw;
	}

	const IObjectIA *CVectorGroupManager::getObject(const CObjectIdent &) const
	{
		// TODO
		throw;
		return NULL;
	}

/*
	########################
	class CListGroupManager
	########################
*/
	CListGroupManager::CListGroupManager():IListManager(new CGroupType())
	{
	}

	CListGroupManager::CListGroupManager(const CListGroupManager &l):IListManager(l)
	{
	}

	CListGroupManager::~CListGroupManager()
	{
	}

	const NLAIC::CIdentType &CListGroupManager::getType() const
	{
		return IdListGroupManager;
	}

	const NLAIC::IBasicType *CListGroupManager::clone() const
	{
		return new CListGroupManager(*this);
	}

	const NLAIC::IBasicType *CListGroupManager::newInstance() const
	{
		return new CListGroupManager();
	}

	void CListGroupManager::trie()
	{
		// TODO : Sort the agent on the ID value.
		throw;
	}

	const IObjectIA *CListGroupManager::getObject(const CObjectIdent &) const
	{
		// TODO.
		throw;
		return NULL;
	}
}
