/** \file list_manager.h
 * Sevral class for objects manipulation.
 *
 * $Id: list_manager.h,v 1.10 2003/01/21 11:24:25 chafik Exp $
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
#ifndef NL_LIST_MANAGER_H
#define NL_LIST_MANAGER_H

#include "nel/ai/agent/agent_object.h"

namespace NLAIAGENT
{
	class CObjectIdent;

	class IListBasicManager : public IBaseGroupType
	{
	protected:
		IBaseGroupType *_List;
	public:
		IListBasicManager();
		IListBasicManager(IBaseGroupType *);
		IListBasicManager(const IListBasicManager &);
		virtual ~IListBasicManager();


		///\name IBaseGroupType methode realize.
		//@{	
		virtual IObjetOp &operator += (const IObjetOp &a);
		virtual IObjetOp &operator -= (const IObjetOp &a);
		virtual IObjectIA &operator = (const IObjectIA &a);

		virtual const IObjectIA *operator[] (sint32) const;

		virtual void set(int i,IObjectIA *o)
		{
			_List->set(i,o);
		}

		virtual bool isTrue() const;

		virtual IObjetOp *operator ! () const;
		virtual void push(const IObjectIA *o);
		virtual void pushFront(const IObjectIA *o);
		virtual CIteratorContener getIterator()
		{
			return _List->getIterator();
		}

		virtual CConstIteratorContener getConstIterator() const
		{
			return _List->getConstIterator();
		}
		
		virtual void cpy(const IObjectIA &o);
		virtual const IObjectIA *pop();
		virtual const IObjectIA *get() const;
		virtual const IObjectIA *popFront();
		virtual const IObjectIA *getFront() const;
		virtual sint32 size() const;
		virtual const IObjectIA *find(const IObjectIA &obj) const;
		virtual void eraseFirst(const IObjectIA &obj);
		virtual void eraseAll(const IObjectIA &obj); 
		virtual void erase(const IObjectIA *o); 
		virtual void erase(const IObjectIA &obj);
		virtual void erase(std::list<const IObjectIA *> &l);
		virtual void clear();

		virtual sint32 getMethodIndexSize() const;
		virtual TQueue isMember(const IVarName *className,const IVarName *methodName,const IObjectIA &p) const;
		virtual IObjectIA::CProcessResult runMethodeMember(sint32, sint32,IObjectIA *);
		virtual IObjectIA::CProcessResult runMethodeMember(sint32,IObjectIA *);


		virtual const CProcessResult &run(){return IObjectIA::ProcessRun;}
		virtual bool isEqual(const IBasicObjectIA &a) const;

		virtual void save(NLMISC::IStream &os);
		virtual void load(NLMISC::IStream &is);
		virtual void getDebugString(std::string &t) const;
		//@}

		///Get the list of message contents.
		operator const IBaseGroupType *() const
		{
			return _List;
		}
	};


	class IListManager : public IListBasicManager
	{	
	public:
		IListManager(IBaseGroupType *);
		IListManager(const IListManager &);
			

		virtual bool isEqual(const IBasicObjectIA &a) const;		
		virtual	CProcessResult sendMessage(IObjectIA *);


		virtual void trie() = 0;
		virtual const IObjectIA *getObject(const CObjectIdent &) const = 0;
		
		virtual ~IListManager();

	};

	class CVectorGroupManager: public IListManager
	{
	public:
		static const NLAIC::CIdentType IdVectorGroupManager;

	public:
		CVectorGroupManager(sint32);
		CVectorGroupManager();
		CVectorGroupManager(const CVectorGroupManager &);
		
		virtual const NLAIC::CIdentType &getType() const;		
		virtual const NLAIC::IBasicType *clone() const;
		virtual const NLAIC::IBasicType *newInstance() const;
		
		virtual void trie();
		virtual const IObjectIA *getObject(const CObjectIdent &) const;

		virtual ~CVectorGroupManager();
	};

	class CListGroupManager: public IListManager
	{
	public:
		static const NLAIC::CIdentType IdListGroupManager;

	public:
		CListGroupManager();
		CListGroupManager(const CListGroupManager &);
		
		virtual const NLAIC::CIdentType &getType() const;		
		virtual const NLAIC::IBasicType *clone() const;
		virtual const NLAIC::IBasicType *newInstance() const;
		
		virtual void trie();
		virtual const IObjectIA *getObject(const CObjectIdent &) const;

		virtual ~CListGroupManager();
	};
}
#endif
