/** \file mailbox.cpp
 *
 * $Id: mailbox.cpp,v 1.29 2002/07/23 15:37:47 chafik Exp $
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
#include "nel/ai/agent/agent_script.h"

namespace NLAIAGENT
{

	IMailBox::IMailBox(const IWordNumRef *parent):IConnectIA(parent)
	{
	}

	IMailBox::IMailBox(const IMailBox &c) : IConnectIA(c)
	{
	}

	IMailBox::IMailBox(NLMISC::IStream &is):IConnectIA(is)
	{			
	}	

	IMailBox::~IMailBox()
	{
	}

	CSimpleLocalMailBox::CSimpleLocalMailBox(const IWordNumRef *parent) : IMailBox(parent), _Size(0)
	{
	}

	CSimpleLocalMailBox::CSimpleLocalMailBox(NLMISC::IStream &is) : IMailBox(is), _Size(0)
	{
		load(is);
	}


	CSimpleLocalMailBox::CSimpleLocalMailBox(const CSimpleLocalMailBox &a) : IMailBox(a), _Size(0)
	{					
		tListMessage::const_reverse_iterator i = a._ListMessage.rbegin();
		while(i != a._ListMessage.rend())
		{
			_ListMessage.push_back(*i++);
			((IMessageBase *)_ListMessage.back())->incRef();
			_Size ++;
		}
		_RunState = a._RunState;
	}
	
	CSimpleLocalMailBox::~CSimpleLocalMailBox()
	{			
		while(_ListMessage.begin() != _ListMessage.end())
		{
			IMessageBase *m = (IMessageBase *)_ListMessage.back();
			m->release();
			_ListMessage.pop_back();				
		}
	}

	void CSimpleLocalMailBox::addMessage(IMessageBase *msg)
	{
		_ListMessage.push_front( msg );
		_Size ++;
	}

	void CSimpleLocalMailBox::addMailBox(IMailBox *)
	{
	}

	void CSimpleLocalMailBox::removeMailBox(IMailBox *)
	{
	}

	const IMessageBase &CSimpleLocalMailBox::getMessage()
	{
		return *_ListMessage.back();
	}
	
	void CSimpleLocalMailBox::popMessage()
	{
		((IMessageBase *)_ListMessage.back())->release();		
		_ListMessage.pop_back();
		_Size --;
	}

	sint32 CSimpleLocalMailBox::getMessageCount() const
	{
		return _ListMessage.size();
	}
	bool CSimpleLocalMailBox::isEmpty() const
	{
		return _Size == 0;//_ListMessage.begin() == _ListMessage.end();
	}

	std::list<const IMessageBase *> *CSimpleLocalMailBox::pumpMessages(/*IBasicMessageGroup &*/) const
	{
		std::list<const IMessageBase *> *result = new std::list<const IMessageBase *>;

		std::list<const IMessageBase *>::const_iterator i = _ListMessage.begin();
		while(i != _ListMessage.end() )
		{
			result->push_back( *i++ );
		}
		return result;
	}

	/*void CSimpleLocalMailBox::addGroup(IBasicMessageGroup &)
	{

	}

	void CSimpleLocalMailBox::removeGroup(IBasicMessageGroup &)
	{

	}

	std::list<IBasicMessageGroup *> &CSimpleLocalMailBox::getGroups()
	{
		return _Msg_grps;
	}*/

	const IObjectIA::CProcessResult &CSimpleLocalMailBox::getState() const 
	{
		return _RunState;
	}

	void CSimpleLocalMailBox::setState(TProcessStatement state, IObjectIA *result)
	{
		_RunState.ResultState = state;
		_RunState.Result = result;
	}

	const IObjectIA::CProcessResult &CSimpleLocalMailBox::run()
	{
		return IObjectIA::ProcessRun;
	}

	bool CSimpleLocalMailBox::isEqual(const IBasicObjectIA &a) const
	{
		const IMailBox &m = (const IMailBox &)a;
		return (const IWordNumRef &)m == (const IWordNumRef &)*this;
	}

	const NLAIC::IBasicType *CSimpleLocalMailBox::clone() const
	{
		NLAIC::IBasicType *a = new CSimpleLocalMailBox(*this);
		return a;
	}

	const NLAIC::IBasicType *CSimpleLocalMailBox::newInstance() const
	{				
		NLAIC::IBasicType *a;
		if(getParent() != NULL) 
			a = new	CSimpleLocalMailBox((const IWordNumRef *) getParent());
		else 
			a = new CSimpleLocalMailBox((const IWordNumRef *) NULL);
		return a;
	}

	const NLAIC::CIdentType &CSimpleLocalMailBox::getType() const
	{
		return IdSimpleLocalMailBox;
	}

	void CSimpleLocalMailBox::getDebugString(std::string &t) const
	{
		t += "class CSimpleLocalMailBox";
	}

	void CSimpleLocalMailBox::save(NLMISC::IStream &os)
	{				
		IConnectIA::save(os);
		sint32 size = _ListMessage.size();
		os.serial( size );

		tListMessage::const_iterator i = _ListMessage.begin();
		while(i != _ListMessage.end())
		{
			IMessageBase &m = (IMessageBase &)*(*i);
			os.serial( (NLAIC::CIdentType &) m.getType() );
			//os.serial( (IMessageBase &) m );
			m.save(os);
		}
		
	}

	void CSimpleLocalMailBox::load(NLMISC::IStream &is)
	{			
		IConnectIA::load(is);
		while(_ListMessage.size())
		{
			((IMessageBase *)_ListMessage.back())->release();
			_ListMessage.pop_back();				
		}
		_Size = 0;

		NLAIC::CIdentTypeAlloc id;
		sint32 i;
		is.serial(i);				
		if(i)
		{
			while(i --)
			{
				is.serial(id);
				IMessageBase *m = (IMessageBase *)id.allocClass();
				is.serial( *m );
				_ListMessage.push_back(m);
				_Size ++;
			}
		}
	}

	void CSimpleLocalMailBox::onKill(IConnectIA *)
	{							
	}

	IObjectIA::CProcessResult CSimpleLocalMailBox::sendMessage(IMessageBase *m)
	{
		m->release();
		return IObjectIA::ProcessRun;
	}

	IConnectIA &CSimpleLocalMailBox::getConnection(IConnectIA &r)
	{								
		addInConnectedList(&r);				
		return *this;
	}			

	CLocalMailBox::CLocalMailBox (const IWordNumRef *parent):IMailBox(parent),_Size(0)
	{
	}

	CLocalMailBox::CLocalMailBox(NLMISC::IStream &is):IMailBox(is),_Size(0)
	{
		load(is);
	}


	CLocalMailBox::CLocalMailBox (const CLocalMailBox &A):IMailBox(A),_RunState(A._RunState),_Size(0)
	{						
		/*tListMailBox::const_reverse_iterator i = A._ListMailBox.rbegin();
		while(i != A._ListMailBox.rend())
		{
			connect(*i);
			_ListMailBox.push_back(*i++);
		}*/	
	}

	CLocalMailBox::~CLocalMailBox()
	{
		while(_ListMessageIn.begin() != _ListMessageIn.end())
		{
			IMessageBase *msg = (IMessageBase *)_ListMessageIn.back();
#ifdef NL_DEBUG
			const char *mname = (const char *)msg->getType();
#endif
			msg->release();
			_ListMessageIn.pop_back();
		}

		while(_ListSharedMessage.begin() != _ListSharedMessage.end())
		{
			IMessageBase *msg = (IMessageBase *)_ListSharedMessage.back();
			msg->release();
			_ListSharedMessage.pop_back();
		}

	}
				
	void CLocalMailBox::shareMessage()
	{
		IMessageBase *msg = (IMessageBase *)_ListMessageIn.back();
		msg->incRef();
		_ListSharedMessage.push_back(msg);
	}

	const IMessageBase &CLocalMailBox::getMessage()
	{				
		return *_ListMessageIn.back();
	}

	void CLocalMailBox::popMessage()
	{	

		/*IMessageBase *msg = (IMessageBase *)_ListMessageIn.back();
		_ListMessageIn.pop_back();
		if(msg->getDispatch())
		{
			_ListSharedMessage.push_back(msg);
		}
		else
		{			
			msg->release();
		}*/
		((IMessageBase *)_ListMessageIn.back())->release();
		_ListMessageIn.pop_back();
		_Size --;
	}

	sint32 CLocalMailBox::getMessageCount() const
	{
		return _Size;
	}

	bool CLocalMailBox::isEmpty() const
	{
		return _Size == 0;//_ListMessageIn.begin() == _ListMessageIn.end();
	}
	
	/*void CLocalMailBox::sendMessage(const IBasicAgent &,const IBaseGroupType &)
	{					
	}*/

	IObjectIA::CProcessResult CLocalMailBox::sendMessage(IMessageBase *m)
	{
		m->release();
		//_listMessageOut.push_back((const IMessageBase *)msg.clone());
		return IObjectIA::ProcessRun;
	}

	void CLocalMailBox::addMessage(IMessageBase *msg)
	{
		_ListMessageIn.push_front(msg);
		_Size ++;
	}
	
	// Ajoute une boite aux lettre dans la liste des boites aux lettres "source" (les bals à consulter).
	void CLocalMailBox::addMailBox(IMailBox *mail)
	{
		connect( mail );
		_ListMailBox.push_back((IMailBox *)mail);

		/*std::list<IBasicMessageGroup *>::const_iterator it_grp = mail->getGroups().begin();
		while ( it_grp != mail->getGroups().end() )
		{
			_Msg_grps.push_back( *it_grp );
			it_grp++;
		}*/

	}

	// Supprime une boite aux lettre dans la liste des boites aux lettres "source" (les bals à consulter).
	void CLocalMailBox::removeMailBox(IMailBox *mail)
	{				
		eraseFromList<IMailBox *>(&_ListMailBox,mail);
		removeConnection(mail);
	}

	void CLocalMailBox::setName(const IVarName &)
	{				
	}

	const NLAIC::IBasicType *CLocalMailBox::clone() const
	{
		NLAIC::IBasicType *a = new CLocalMailBox(*this);
		return a;
	}

	const NLAIC::IBasicType *CLocalMailBox::newInstance() const
	{				
		NLAIC::IBasicType *a;
		if(getParent() != NULL) 
			a = new	CLocalMailBox((const IWordNumRef *)getParent());
		else 
			a = new CLocalMailBox((const IWordNumRef *)NULL);
		return a;
	}

	const NLAIC::CIdentType &CLocalMailBox::getType() const
	{
		return IdLocalMailBox;
	}

	void CLocalMailBox::onKill(IConnectIA *a)
	{				
		eraseFromList<IMailBox *>(&_ListMailBox,(IMailBox *)a);

		tListMessageIter msgItr = _ListMessageIn.begin();
		while(msgItr != _ListMessageIn.end())
		{
			IMessageBase *msg = (IMessageBase *)*msgItr;

			if(msg->getSender() == a || msg->getContinuation() == a || msg->getReceiver() == a)
			{
				tListMessageIter iTmp = msgItr++;
				_ListMessageIn.erase(iTmp);
				msg->release();
				_Size --;
			}
			else msgItr++;			
		}

	}
	
	bool CLocalMailBox::isEqual(const IBasicObjectIA &a) const
	{
		const IMailBox &m = (const IMailBox &)a;
		return (const IWordNumRef &)m == (const IWordNumRef &)*this;
	}

	void CLocalMailBox::save(NLMISC::IStream &os)
	{	
		IMailBox::save(os);
		sint32 size = _ListMailBox.size();
		os.serial( size );
		tListMailBoxCstIter k = _ListMailBox.begin();
		while(k != _ListMailBox.end())
		{
			IMailBox *mail = *k++;					
			((IWordNumRef *)(const NLAIAGENT::IWordNumRef *)*mail)->save(os);
		}

		size = _ListMessageIn.size();
		os.serial( size );
		tListMessageIter msgItr = _ListMessageIn.begin();
		while(msgItr != _ListMessageIn.end())
		{
			IMessageBase *msg = (IMessageBase *)*msgItr++;
			os.serial( (NLAIC::CIdentType &) (msg->getType()) );
			msg->save(os);
		}

		size = _ListSharedMessage.size();
		os.serial( size );
		msgItr = _ListSharedMessage.begin();
		while(msgItr != _ListSharedMessage.end())
		{
			IMessageBase *msg = (IMessageBase *)*msgItr++;
			os.serial( (NLAIC::CIdentType &) (msg->getType()) );
			msg->save(os);
		}
		//_ListMessageIn.save(os);
	}

	void CLocalMailBox::load(NLMISC::IStream &is)
	{			
		IMailBox::load(is);
		sint32 i;				
		NLAIC::CIdentTypeAlloc id;			
		is.serial(i);
		_Size = 0;
		if(i)
		{
			while(i --)
			{
				is.serial(id);
				IWordNumRef *num = (IWordNumRef *)id.allocClass();
				num->load(is);
				_ListMailBox.push_back((IMailBox *)((const IRefrence *)*num));
				_Size ++;
				delete num;
			}
		}

		//_ListMessageIn.load(is);
	}					

	const IObjectIA::CProcessResult &CLocalMailBox::getState() const 
	{
		return _RunState;
	}

	void CLocalMailBox::setState(TProcessStatement state, IObjectIA *result)
	{
		_RunState.ResultState = state;
		_RunState.Result = result;
	}

	std::list<const IMessageBase *> *CLocalMailBox::pumpMessages(/*IBasicMessageGroup &grp*/) const
	{
		//return _ListMessageIn.pumpMessages( grp );
		if(_ListSharedMessage.begin() != _ListSharedMessage.end())
		{
			std::list<const IMessageBase *> *l = new tListMessage;
			tListMessageCstIter msgItr = _ListSharedMessage.begin();
			while(msgItr != _ListSharedMessage.end())
			{	
				const IMessageBase *msg = *msgItr++;
				l->push_back((const IMessageBase *)msg->clone());								
			}
			return l;
		}
		return NULL;
	}

	const IObjectIA::CProcessResult &CLocalMailBox::run()
	{		

		/*while(_ListSharedMessage.begin() != _ListSharedMessage.end())
		{
			_ListSharedMessage.back()->release();			
			_ListSharedMessage.pop_back();
		}
		tListMailBoxIter j;
		fillMailBox();*/
		return getState();
	}

	void CLocalMailBox::getDebugString(std::string &t) const
	{
		t += "class CLocalMailBox parent";
		/*if(getParent() != NULL)
		{
			const IObjectIA *o = getParent();
			t += (const char *)o->getType();
			t += ">";
		}
		else
			t += "NULL>";*/

	}

	void CLocalMailBox::fillMailBox()
	{		
		tListMailBoxIter i_mbox = _ListMailBox.begin();

#ifdef NL_DEBUG
		int s = _ListMailBox.size();
#endif

		while ( i_mbox != _ListMailBox.end() )
		{
			IMailBox *box = (*i_mbox);
			std::list<const IMessageBase *> *msg_lst = box->pumpMessages();
			if ( msg_lst != NULL)
			{
				while ( msg_lst->begin() != msg_lst->end())
				{
					IMessageBase *m = (IMessageBase *) msg_lst->front();
					m->incRef();
					_ListMessageIn.push_back(m);
					m->release();
					msg_lst->pop_front();
				}
				delete msg_lst;
			}
			i_mbox++;
		}
	}


	CScriptMailBox::CScriptMailBox(const IWordNumRef *parent) : CSimpleLocalMailBox(parent)
	{
	}

	CScriptMailBox::CScriptMailBox(NLMISC::IStream &is) : CSimpleLocalMailBox(is)
	{
		load(is);
	}

	CScriptMailBox::CScriptMailBox(const CScriptMailBox &a) : CSimpleLocalMailBox(a)
	{					
		tListMessage::const_reverse_iterator i = a._ListMessage.rbegin();
		while(i != a._ListMessage.rend())
		{
			_ListMessage.push_back(*i++);
			((IMessageBase *)_ListMessage.back())->incRef();
		}
		_RunState = a._RunState;
	}
	
	CScriptMailBox::~CScriptMailBox()
	{			
		while(_ListMessage.begin() != _ListMessage.end())
		{
			IMessageBase *m = (IMessageBase *)_ListMessage.back();
			m->release();
			_ListMessage.pop_back();				
		}
	}

	const IObjectIA::CProcessResult &CScriptMailBox::run()
	{
		// Get messages from the father's mailbox
		fillMailBox();
		return IObjectIA::ProcessRun;
	}

	/// Gets messages from its father
	void CScriptMailBox::fillMailBox()
	{		
	}

	void CScriptMailBox::setIndex(sint32 i)
	{
		_CIndex = i;
	}

	sint32 CScriptMailBox::getIndex()
	{
		return _CIndex;
	}

	const IMessageBase *CScriptMailBox::translateMsg(const IMessageBase *msg)
	{
		CAgentScript *father = (CAgentScript *) getParent()->getParent();

		sint32 index = father->getChildMessageIndex( msg, _CIndex );

		if ( index != -1 )
		{
			IMessageBase *translated_msg = (IMessageBase *) msg->clone();
			translated_msg->setMethodIndex( 0, index + ( (CAgentScript *) getParent())->getBaseMethodCount() );
			return translated_msg;
		}
		return NULL;
	}

	const NLAIC::IBasicType *CScriptMailBox::clone() const
	{
		NLAIC::IBasicType *a = new CScriptMailBox(*this);
		return a;
	}

	const NLAIC::IBasicType *CScriptMailBox::newInstance() const
	{
		NLAIC::IBasicType *a;
		if(getParent() != NULL)
			a = new	CScriptMailBox((const IWordNumRef *)getParent());
		else
			a = new CScriptMailBox((const IWordNumRef *)NULL);
		return a;
	}

	const NLAIC::CIdentType &CScriptMailBox::getType() const
	{
		return IdScriptMailBox;
	}
}
