/** \file msg.h
 * class message.
 *
 * $Id: msg.h,v 1.22 2003/01/23 15:40:56 chafik Exp $
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
		///statu of the sender, if true that indicate that the IObjectIA *_Sender pointer will be destruct on the destructor.
		bool _SenderIsVolatile;
		///witch Agent the message have to be achieve.
		IObjectIA *_Receiver;
		bool _ReceiverIsVolatile;
		///This adresse is when receiver have to inform something at a third agent.
		IObjectIA *_Continuation;
		bool _ContinuationIsVolatile;		
		///This reserved variable represent the offset of the Run(MSG) if the message come from script.
		sint32 _ReservedMethodIndexVar;
		///if the message come from script then this reserved variable represent the offset of the base class which the Run(MSG) is.
		sint32 _ReservedHeritanceIndexVar;
		///This variable define the performatif for agent dialog protocole. 
		TPerformatif _Performatif;
		///This bool allow to know here the message come from: scriptes agent or hard coded agent.
		bool _comeFromC_PLUS;
		///Sender can be change on a multible send/run messages.
		bool _ProtectSender;

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
		IMessageBase();
		IMessageBase(IObjectIA *sender,IBaseGroupType *g);
		IMessageBase(IObjectIA *sender, IBasicMessageGroup &msg_group,IBaseGroupType *g);

		IMessageBase(const IMessageBase &m);

		virtual ~IMessageBase();		

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
		void setSender(IObjectIA *s, bool v = false);
		void setReceiver(IObjectIA *r, bool v = false);
		void setContinuation(IObjectIA *r, bool v = false);		

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

		void setProtcetSender()
		{
			_ProtectSender = true;
		}
		void UnsetProtcetSender()
		{
			_ProtectSender = false;
		}

		bool getProtcetSender() const
		{
			return _ProtectSender;
		}

		//@}		

	public:
		virtual IObjectIA &operator = (const IObjectIA &a);		

		virtual sint32 getMethodIndexSize() const;
		virtual TQueue isMember(const IVarName *className,const IVarName *methodName,const IObjectIA &p) const;
		virtual IObjectIA::CProcessResult runMethodeMember(sint32, sint32,IObjectIA *);
		virtual IObjectIA::CProcessResult runMethodeMember(sint32,IObjectIA *);
		virtual bool isEqual(const IBasicObjectIA &a) const;
		
		virtual void save(NLMISC::IStream &os);
		virtual void load(NLMISC::IStream &is);
		virtual void getDebugString(std::string &t) const;	
		
		virtual	CProcessResult sendMessage(IObjectIA *);
		virtual	CProcessResult sendMessage(const IVarName &,IObjectIA *);
		//@}
	};

	/**		
		CMessage is an IMessageBase based on a CGroupType list.

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
