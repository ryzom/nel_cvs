/** \file messagerie.h
 * class message.
 *
 * $Id: msg.h,v 1.13 2001/06/14 10:23:31 chafik Exp $
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
#ifndef NL_MSG_H
#define NL_MSG_H

#include "nel/ai/agent/agentexport.h"
#include "nel/ai/c/registry_class.h"
#include "nel/ai/agent/agent_object.h"
#include "nel/ai/agent/msg_group.h"
#include "nel/ai/agent/msg_container.h"
#include "nel/ai/agent/list_manager.h"

namespace NLAIAGENT
{	
	class CIdentType;
	class IBasicAgent;
	//class IListBasicManager;
	
	/**		
		Abstract Base class for message, all message tansited by agent have this class as base class.

		* \author Chafik sameh
		* \author Portier Pierre
		* \author Robert Gabriel
		* \author Nevrax France
		* \date 2000
	*/
	
	class IMessageBase: public IListBasicManager
	{
	public:
		enum TPerformatif
		{
			PUndefine,
			PExec,
			PAchieve,
			PAsk,
			PBreak,
			PTell,
			PKill,
			PError,			
			PEven,
			PService
		};

	private:

		struct CMethodCall
		{
			CMethodCall(const char *name, int i): MethodName (name)
			{				
				Index = i;
			}
			CStringVarName MethodName;
			sint32 Index;
		};


		static CMethodCall _Method[];

	private:

		///Who send the message.
		IObjectIA *_Sender;
		///witch Agent the message have to be achieve.
		IObjectIA *_Receiver;
		///This adresse is when receiver have to inform something at a third agent.
		IObjectIA *_Continuation;
		///Message is arranged by group.
		IBasicMessageGroup *_MsgGroup;
		/*
		///This is the list whos containe the message.
		IBaseGroupType *_Message;
		*/
		///This reserved variable represent the offset of the Run(MSG) if the message come from script.
		sint32 _ReservedMethodIndexVar;
		///if the message come from script then this reserved variable represent the offset of the base class which the Run(MSG) is.
		sint32 _ReservedHeritanceIndexVar;
		///This variable define the performatif for agent dialog protocole. 
		TPerformatif _Performatif;
		///This bool allow to know here the message come from: scriptes agent or hard coded agent.
		bool _comeFromC_PLUS;
		///Is true if agent want to dispatch it to its child;
		bool _Dispatch;

	public:
		void setMessageGroup(IBaseGroupType *g)
		{
			if(_List != NULL) _List->release();
			_List = g;
		}

		const IBaseGroupType *getMessageGroup() const
		{
			return _List;
		}

	public:
		IMessageBase():IListBasicManager(),_Sender(NULL),_MsgGroup(NULL)
		{
			_ReservedMethodIndexVar = -1;
			_ReservedHeritanceIndexVar = 0;
			_Receiver = NULL;
			_Continuation = NULL;
			_Performatif = PUndefine;
			_comeFromC_PLUS = true;
			_Dispatch = false;
		}
		IMessageBase(IObjectIA *sender,IBaseGroupType *g):IListBasicManager(g),_Sender(sender),_MsgGroup(NULL)
		{
			_ReservedMethodIndexVar = -1;
			_ReservedHeritanceIndexVar = 0;
			_Receiver = NULL;
			_Continuation = NULL;
			_Performatif = PUndefine;
			_comeFromC_PLUS = true;
			_Dispatch = false;
		}
		IMessageBase(IObjectIA *sender, IBasicMessageGroup &msg_group,IBaseGroupType *g):
			IListBasicManager(g),_Sender(sender),_MsgGroup((IBasicMessageGroup *)msg_group.clone())
		{
			_ReservedMethodIndexVar = -1;
			_ReservedHeritanceIndexVar = 0;
			_Receiver = NULL;
			_Continuation = NULL;
			_Performatif = PUndefine;
			_comeFromC_PLUS = true;
			_Dispatch = false;
		}

		IMessageBase(const IMessageBase &m):IListBasicManager(m._List != NULL ? (IBaseGroupType *)m._List->clone(): NULL)
		{
			_Sender = m._Sender;
			//if(_Sender) _Sender->incRef();
			_Receiver = m._Receiver;
			//if(_Receiver) _Receiver->incRef();
			_Continuation = m._Continuation;
			//if(_Continuation) _Continuation->incRef();

			if(m._MsgGroup) _MsgGroup = (IBasicMessageGroup *)m._MsgGroup->clone();
			else _MsgGroup = NULL;
			//_Message = (IBaseGroupType *)m._Message->clone();			
			_ReservedMethodIndexVar = m._ReservedMethodIndexVar;
			_ReservedHeritanceIndexVar = m._ReservedHeritanceIndexVar;			
			_Performatif = m._Performatif;
			_comeFromC_PLUS = m._comeFromC_PLUS;
			_Dispatch = m._Dispatch;
		}

		virtual ~IMessageBase()
		{
			if(_MsgGroup != NULL) _MsgGroup->release();
			//if(_Message != NULL) _Message->release();
			//if(_Sender != NULL) _Sender->release();
			//if(_Receiver != NULL) _Receiver->release();
			//if(_Continuation != NULL) _Continuation->release();
		}		
		
		///Get the IBasicMessageGroup whiche define the group of the message. 
		const IBasicMessageGroup &getGroup() const/// throw (NLAIE::CExceptionUnReference)
		{
			if(_MsgGroup == NULL) throw NLAIE::CExceptionUnReference("_MsgGroup in the iMessage class is not corectly inistializ");
			return *_MsgGroup;
		}

		///Set the IBasicMessageGroup whiche define the group of the message.
		void setGroup(IBasicMessageGroup &grp)
		{			
			if(_MsgGroup != NULL) _MsgGroup->release();
			_MsgGroup = (IBasicMessageGroup *)grp.clone();
		}

		void setPerformatif(TPerformatif p)
		{
			_Performatif = p;
		}

		const TPerformatif &getPerformatif() const
		{
			return _Performatif;
		}

		///\name Set and get agent sender reciver and third
		//@{
		void setSender(IObjectIA *s)
		{			
			//if(_Sender) _Sender->release();
			_Sender = s;
		}

		void setReceiver(IObjectIA *r)
		{	
			//if(_Receiver) _Receiver->release();
			_Receiver = r;
		}

		void setContinuation(IObjectIA *r)
		{			
			//if(_Continuation) _Continuation->release();
			_Continuation = r;
		}

		void setDispatch(bool state = true)
		{
			_Dispatch = state;
		}

		bool getDispatch() const
		{
			return _Dispatch;
		}

		const IObjectIA *getSender() const
		{			
			return _Sender;
		}

		const IObjectIA *getReceiver() const
		{			
			return _Receiver;
		}

		const IObjectIA *getContinuation() const
		{			
			return _Continuation;
		}
		//@}
		
		///\name Set and get script message method processing.
		//@{
		void setMethodIndex(sint32 h,sint32 n)
		{
			_ReservedMethodIndexVar = n;
			_ReservedHeritanceIndexVar = h;
		}

		sint32 getMethodIndex() const
		{
			return _ReservedMethodIndexVar;
		}

		const sint32  getHeritanceIndex() const
		{
			return _ReservedHeritanceIndexVar;
		}
		//@}		

	public:
		
		///\name IBaseGroupType methode realize.
		//@{	
		/*virtual IObjetOp &operator += (const IObjetOp &a);
		virtual IObjetOp &operator -= (const IObjetOp &a);		

		virtual const IObjectIA *operator[] (sint32) const;

		virtual void set(int i,IObjectIA *o)
		{
			_Message->set(i,o);
		}

		virtual bool isTrue() const;

		virtual IObjetOp *operator ! () const;
		virtual void push(const IObjectIA *o);
		virtual void pushFront(const IObjectIA *o);
		virtual CIteratorContener getIterator()
		{
			return _Message->getIterator();
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
		virtual void clear();*/

		virtual IObjectIA &operator = (const IObjectIA &a);		

		virtual sint32 getMethodIndexSize() const;
		virtual tQueue isMember(const IVarName *className,const IVarName *methodName,const IObjectIA &p) const;
		virtual IObjectIA::CProcessResult runMethodeMember(sint32, sint32,IObjectIA *);
		virtual IObjectIA::CProcessResult runMethodeMember(sint32,IObjectIA *);
		virtual bool isEqual(const IBasicObjectIA &a) const;
		
		virtual void save(NLMISC::IStream &os);
		virtual void load(NLMISC::IStream &is);
		virtual void getDebugString(std::string &t) const;		
		//@}
	};

	/**		
		AbCMessage is an IMessageBase based on a CGroupType list.

		* \author Chafik sameh
		* \author Portier Pierre
		* \author Nevrax France
		* \date 2000
	*/
	class CMessageList: public IMessageBase
	{
	private:
		

	public:
		static const NLAIC::CIdentType IdMessage;
	public:

		CMessageList()
		{
			CGroupType *x = new CGroupType();
			setMessageGroup(x);
		}

		CMessageList(const CGroupType &m)
		{
			setMessageGroup((CGroupType *)m.clone());
		}

		CMessageList(const CGroupType &m,IObjectIA *sender): IMessageBase(sender,(CGroupType *)m.clone())
		{
		}

		CMessageList(const CGroupType &m,IObjectIA *sender, IBasicMessageGroup &msg_group):IMessageBase(sender,msg_group,(CGroupType *)m.clone())
		{
		}

		CMessageList(const CMessageList &m):IMessageBase(m)
		{
		}		
		

		virtual ~CMessageList()
		{			
		}

	public:

		///\name IMessageBase methode realize.
		//@{	
		virtual const NLAIC::CIdentType &getType() const;					

		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicInterface *c = (NLAIC::IBasicInterface *)new CMessageList(*this);
			return c;
		}		

		virtual const NLAIC::IBasicType *newInstance() const
		{									
			CMessageList *instance = new CMessageList;
			return instance;
		}		
		//@}
	
	};

	/**		
		AbCMessage is an IMessageBase based on a CVectorGroupType list.

		* \author Chafik sameh
		* \author Portier Pierre
		* \author Nevrax France
		* \date 2000
	*/
	class CMessageVector: public IMessageBase
	{
	private:
		

	public:
		static const NLAIC::CIdentType IdMessageVector;
	public:

		CMessageVector(int n)
		{
			CVectorGroupType *x = new CVectorGroupType(n);
			setMessageGroup(x);
		}

		CMessageVector()
		{
			CVectorGroupType *x = new CVectorGroupType();
			setMessageGroup(x);
		}

		CMessageVector(const CVectorGroupType &m)
		{
			setMessageGroup((CVectorGroupType *)m.clone());
		}

		CMessageVector(const CVectorGroupType &m,IObjectIA *sender): IMessageBase(sender,(CVectorGroupType *)m.clone())
		{
		}

		CMessageVector(const CVectorGroupType &m,IObjectIA *sender, IBasicMessageGroup &msg_group):
				IMessageBase(sender,msg_group,(CVectorGroupType *)m.clone())
		{
		}

		CMessageVector(const CMessageVector &m):IMessageBase(m)
		{
		}		
		

		virtual ~CMessageVector()
		{			
		}

	public:

		///\name IMessageBase methode realize.
		//@{	

		virtual const NLAIC::CIdentType &getType() const;					

		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicInterface *m = (NLAIC::IBasicInterface *)new CMessageVector(*this);
			return m;
		}		

		virtual const NLAIC::IBasicType *newInstance() const
		{									
			return clone();
		}		
		//@}
	};
}
#endif
