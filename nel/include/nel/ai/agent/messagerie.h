/** \file messagerie.h
 * class message.
 *
 * $Id: messagerie.h,v 1.8 2001/01/17 10:32:29 chafik Exp $
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
#ifndef NL_MESSAGERIE_H
#define NL_MESSAGERIE_H

#include "nel/ai/agent/agentexport.h"
#include "nel/ai/c/registry_class.h"
#include "nel/ai/agent/agent_object.h"
#include "nel/ai/agent/msg_group.h"
#include "nel/ai/agent/msg_container.h"

namespace NLAIAGENT
{	
	class CIdentType;
	class IBasicAgent;
	

	/**		
		Abstract Base class for message, all message tansited by agent have this class as base class.

		* \author Chafik sameh
		* \author Portier Pierre
		* \author Nevrax France
		* \date 2000
	*/
	class IMessageBase: public IBaseGroupType
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
			PKill
		};

	private:
		///Who send the message.
		IObjectIA *_Sender;
		///witch Agent the message have to be achieve.
		IObjectIA *_Receiver;
		///This adresse is when receiver have to inform something at a third agent.
		IObjectIA *_Continuation;
		///Message is arranged by group.
		IBasicMessageGroup *_MsgGroup;
		///This is the list whos containe the message.
		IBaseGroupType *_Message;
		///This reserved variable represent the offset of the Run(MSG) if the message come from script.
		sint32 _ReservedMethodIndexVar;
		///if the message come from script then this reserved variable represent the offset of the base class which the Run(MSG) is.
		sint32 _ReservedHeritanceIndexVar;
		///This variable define the performatif for agent dialog protocole. 
		TPerformatif _Performatif;
		///This bool allow to know here the message come from: scriptes agent or hard coded agent.
		bool _comeFromC_PLUS;

	protected:
		void setMessageGroup(IBaseGroupType *g)
		{
			_Message = g;
		}

		const IBaseGroupType *getMessageGroup() const
		{
			return _Message;
		}

	public:
		IMessageBase():_Sender(NULL),_MsgGroup(NULL),_Message(NULL)
		{
			_ReservedMethodIndexVar = -1;
			_ReservedHeritanceIndexVar = 0;
			_Receiver = NULL;
			_Continuation = NULL;
			_Performatif = PUndefine;
			_comeFromC_PLUS = true;
		}
		IMessageBase(IObjectIA *sender,IBaseGroupType *g):_Sender(sender),_MsgGroup(NULL),_Message(g)
		{
			_ReservedMethodIndexVar = -1;
			_ReservedHeritanceIndexVar = 0;
			_Receiver = NULL;
			_Continuation = NULL;
			_Performatif = PUndefine;
			_comeFromC_PLUS = true;
		}
		IMessageBase(IObjectIA *sender, IBasicMessageGroup &msg_group,IBaseGroupType *g):
			_Sender(sender),_MsgGroup((IBasicMessageGroup *)msg_group.clone()),_Message(g)
		{
			_ReservedMethodIndexVar = -1;
			_ReservedHeritanceIndexVar = 0;
			_Receiver = NULL;
			_Continuation = NULL;
			_Performatif = PUndefine;
			_comeFromC_PLUS = true;
		}
		IMessageBase(const IMessageBase &m)
		{
			_Sender = m._Sender;
			if(m._MsgGroup) _MsgGroup = (IBasicMessageGroup *)m._MsgGroup->clone();
			else _MsgGroup = NULL;
			_Message = (IBaseGroupType *)m._Message->clone();			
			_ReservedMethodIndexVar = m._ReservedMethodIndexVar;
			_ReservedHeritanceIndexVar = m._ReservedHeritanceIndexVar;
			_Receiver = m._Receiver;
			_Continuation = m._Continuation;			
			_Performatif = m._Performatif;
			_comeFromC_PLUS = m._comeFromC_PLUS;
		}

		virtual ~IMessageBase()
		{
			if(_MsgGroup != NULL) _MsgGroup->release();
			if(_Message != NULL) _Message->release();
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
			_MsgGroup = (IBasicMessageGroup *)grp.clone();
		}

		void setPerformatif(TPerformatif p)
		{
			_Performatif = p;
		}

		const TPerformatif &getPerformatif()
		{
			return _Performatif;
		}

		///\name Set and get agent sender reciver and third
		//@{
		void setSender(IObjectIA *s)
		{			
			_Sender = s;
		}

		void setReceiver(IObjectIA *r)
		{			
			_Receiver = r;
		}

		void setContinuation(IObjectIA *r)
		{			
			_Continuation = r;
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
		

		///Get the list of message contents.
		operator const IBaseGroupType *() const
		{
			return _Message;
		}

	public:
		
		///\name IBaseGroupType methode realize.
		//@{	
		virtual IObjetOp &operator += (const IObjetOp &a);
		virtual IObjetOp &operator -= (const IObjetOp &a);
		virtual IObjectIA &operator = (const IObjectIA &a);

		virtual const IObjectIA *operator[] (sint32) const;

		virtual void set(int i,IObjectIA *o)
		{
			_Message->set(i,o);
		}

		virtual bool isTrue() const;

		virtual IObjetOp *operator ! () const;
		virtual void push(const IObjectIA *o);
		virtual void pushFront(const IObjectIA *o);
		virtual CIteratorContener getIterator() const
		{
			return _Message->getIterator();
		}
		virtual void cpy(const IObjectIA &o);
		virtual const IObjectIA *pop();
		virtual const IObjectIA *get();
		virtual const IObjectIA *popFront();
		virtual const IObjectIA *getFront();
		virtual sint32 size() const;
		virtual const IObjectIA *find(const IObjectIA &obj) const;
		virtual void eraseFirst(const IObjectIA &obj);
		virtual void eraseAll(const IObjectIA &obj); 
		virtual void erase(const IObjectIA *o); 
		virtual void erase(const IObjectIA &obj);
		virtual void erase(std::list<const IObjectIA *> &l);
		virtual void clear();

		virtual const CProcessResult &run();
		virtual	IObjectIA::CProcessResult runMethodeMember(sint32, sint32, IObjectIA *)
		{
			return IObjectIA::CProcessResult();
		}
		virtual	IObjectIA::CProcessResult runMethodeMember(sint32 ,IObjectIA *)
		{
			return IObjectIA::CProcessResult();
		}

		virtual bool isEqual(const IBasicObjectIA &a) const;
		
		virtual void save(NLMISC::IStream &os);
		virtual void load(NLMISC::IStream &is);
		virtual void getDebugString(char *t) const;		
		//@}
	};

	/**		
		AbCMessage is an IMessageBase based on a CGroupType list.

		* \author Chafik sameh
		* \author Portier Pierre
		* \author Nevrax France
		* \date 2000
	*/
	class CMessage: public IMessageBase
	{
	private:
		

	public:
		static const NLAIC::CIdentType IdMessage;
	public:

		CMessage()
		{
			CGroupType *x = new CGroupType();
			setMessageGroup(x);
		}

		CMessage(const CGroupType &m)
		{
			setMessageGroup((CGroupType *)m.clone());
		}

		CMessage(const CGroupType &m,IObjectIA *sender): IMessageBase(sender,(CGroupType *)m.clone())
		{
		}

		CMessage(const CGroupType &m,IObjectIA *sender, IBasicMessageGroup &msg_group):IMessageBase(sender,msg_group,(CGroupType *)m.clone())
		{
		}

		CMessage(const CMessage &m):IMessageBase(m)
		{
		}		
		

		virtual ~CMessage()
		{			
		}

	public:

		///\name IMessageBase methode realize.
		//@{	
		virtual const NLAIC::CIdentType &getType() const;					

		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicInterface *c = (NLAIC::IBasicInterface *)new CMessage(*this);
			return c;
		}		

		virtual const NLAIC::IBasicType *newInstance() const
		{									
			CMessage *instance = new CMessage;
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
