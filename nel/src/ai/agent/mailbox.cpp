/** \file mailbox.cpp
 *
 * $Id: mailbox.cpp,v 1.9 2001/01/24 09:54:31 portier Exp $
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

	CSimpleLocalMailBox::CSimpleLocalMailBox(const IWordNumRef *parent) : IMailBox(parent)
	{
	}

	CSimpleLocalMailBox::CSimpleLocalMailBox(NLMISC::IStream &is) : IMailBox(is)
	{
		load(is);
	}


	CSimpleLocalMailBox::CSimpleLocalMailBox(const CSimpleLocalMailBox &a) : IMailBox(a)
	{					
		tListMessage::const_reverse_iterator i = a._ListMessage.rbegin();
		while(i != a._ListMessage.rend())
		{
			_ListMessage.push_back(*i++);
			((IMessageBase *)_ListMessage.back())->incRef();
		}
		_RunState = a._RunState;
	}
	
	CSimpleLocalMailBox::~CSimpleLocalMailBox()
	{			
		while(_ListMessage.size())
		{
			IMessageBase *m = (IMessageBase *)_ListMessage.back();
			m->release();
			_ListMessage.pop_back();				
		}
	}

	void CSimpleLocalMailBox::addMessage(IMessageBase *msg)
	{
		_ListMessage.push_back( msg );
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
		IMessageBase *msg = (IMessageBase *)_ListMessage.back();
		msg->release();
		_ListMessage.pop_back();
	}

	sint32 CSimpleLocalMailBox::getMessageCount() const
	{
		return _ListMessage.size();
	}

	std::list<const IMessageBase *> *CSimpleLocalMailBox::pumpMessages(IBasicMessageGroup &) const
	{
		std::list<const IMessageBase *> *result = new std::list<const IMessageBase *>;

		std::list<const IMessageBase *>::const_iterator i = _ListMessage.begin();
		while(i != _ListMessage.end() )
		{
			result->push_back( *i++ );
		}
		return result;
	}

	void CSimpleLocalMailBox::addGroup(IBasicMessageGroup &)
	{

	}

	void CSimpleLocalMailBox::removeGroup(IBasicMessageGroup &)
	{

	}

	std::list<IBasicMessageGroup *> &CSimpleLocalMailBox::getGroups()
	{
		return _Msg_grps;
	}

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

	void CSimpleLocalMailBox::getDebugString(char *t) const
	{
		sprintf(t,"class CSimpleLocalMailBox");
	}

	void CSimpleLocalMailBox::save(NLMISC::IStream &os)
	{				
		IConnectIA::save(os);
		sint32 size = _ListMessage.size();
		os.serial( size );

		tListMessage::const_iterator i = _ListMessage.begin();
		while(i != _ListMessage.end())
		{
			const IMessageBase &m = *(*i);
			os.serial( (NLAIC::CIdentType &) m.getType() );
			os.serial( (IMessageBase &) m );
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
			}
		}
	}

	void CSimpleLocalMailBox::onKill(IConnectIA *)
	{							
	}

	IObjectIA::CProcessResult CSimpleLocalMailBox::sendMessage(IMessageBase *)
	{
		return IObjectIA::ProcessRun;
	}

	IConnectIA &CSimpleLocalMailBox::getConnection(IConnectIA &r)
	{								
		addInConnectedList(&r);				
		return *this;
	}			

	CLocalMailBox::CLocalMailBox (const IWordNumRef *parent):IMailBox(parent)
	{
	}

	CLocalMailBox::CLocalMailBox(NLMISC::IStream &is):IMailBox(is)
	{
		load(is);
	}


	CLocalMailBox::CLocalMailBox (const CLocalMailBox &A):IMailBox(A),_RunState(A._RunState)
	{						
		tListMailBox::const_reverse_iterator i = A._ListMailBox.rbegin();
		while(i != A._ListMailBox.rend())
		{
			_ListMailBox.push_back(*i++);					
		}	
	}

	CLocalMailBox::~CLocalMailBox()
	{
	}
				
	const IMessageBase &CLocalMailBox::getMessage()
	{				
		return *_ListMessageIn.getMessage();
	}

	void CLocalMailBox::popMessage()
	{	
		_ListMessageIn.popMessage();
	}

	sint32 CLocalMailBox::getMessageCount() const
	{
		return _ListMessageIn.size();
	}
	
	void CLocalMailBox::sendMessage(const IBasicAgent &,const IBaseGroupType &)
	{			
		//_listMessageOut.push_back((const IMessageBase *)IMessageBase(msg,a).clone());
	}

	IObjectIA::CProcessResult CLocalMailBox::sendMessage(IMessageBase *)
	{
		//_listMessageOut.push_back((const IMessageBase *)msg.clone());
		return IObjectIA::ProcessRun;
	}

	void CLocalMailBox::addMessage(IMessageBase *msg)
	{
		_ListMessageIn.addMessage(msg);
	}
	
	// Ajoute une boite aux lettre dans la liste des boites aux lettres "source" (les bals à consulter).
	void CLocalMailBox::addMailBox(IMailBox *mail)
	{
		connect( mail );

		_ListMailBox.push_back((IMailBox *)mail);

		std::list<IBasicMessageGroup *>::const_iterator it_grp = mail->getGroups().begin();
		while ( it_grp != mail->getGroups().end() )
		{
			_Msg_grps.push_back( *it_grp );
			it_grp++;
		}

	}

	// Supprime une boite aux lettre dans la liste des boites aux lettres "source" (les bals à consulter).
	void CLocalMailBox::removeMailBox(IMailBox *mail)
	{				
		eraseFromList<IMailBox *>(&_ListMailBox,mail);
		removeConnection(*mail);
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
			((IWordNumRef *)mail)->save(os);
		}
		
		_ListMessageIn.save(os);
	}

	void CLocalMailBox::load(NLMISC::IStream &is)
	{			
		IMailBox::load(is);
		sint32 i;				
		NLAIC::CIdentTypeAlloc id;			
		is.serial(i);				
		if(i)
		{
			while(i --)
			{
				is.serial(id);						
				IWordNumRef *num = (IWordNumRef *)id.allocClass();
				num->load(is);
				_ListMailBox.push_back((IMailBox *)((const IRefrence *)*num));
				delete num;
			}
		}
		_ListMessageIn.load(is);
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

	std::list<const IMessageBase *> *CLocalMailBox::pumpMessages(IBasicMessageGroup &grp) const
	{
//		return _ListMessageIn.pumpMessages( grp );
		return NULL;
	}

	const IObjectIA::CProcessResult &CLocalMailBox::run()
	{
		setState(processBuzzy,NULL);	
		tListMailBoxIter j;
		fillMailBox();
		setState(processIdle,NULL);
		return getState();
	}

	void CLocalMailBox::getDebugString(char *t) const
	{
		sprintf(t,"class CLocalMailBox");
	}

	void CLocalMailBox::addGroup(IBasicMessageGroup &grp)
	{
		std::list<IBasicMessageGroup *>::iterator it_grp = _Msg_grps.begin();
		while ( it_grp != _Msg_grps.end() )
		{
			if ( ( **it_grp ) == grp )
				return;
			it_grp++;
		}
		_Msg_grps.push_back(&grp);
	}

	std::list<IBasicMessageGroup *> &CLocalMailBox::getGroups() 
	{
		return _Msg_grps;
	}


	void CLocalMailBox::removeGroup(IBasicMessageGroup &grp)
	{
		std::list<IBasicMessageGroup *>::iterator it_grp = _Msg_grps.begin();
		while ( it_grp != _Msg_grps.end() )
		{
			if ( *it_grp == &grp ) 
			{
				_Msg_grps.erase( it_grp );
				return;
			}
			it_grp ++;
		}
	}

	void CLocalMailBox::fillMailBox()
	{
		// Récupère les messages dans chaque boite aux lettres "source"...
		tListMailBoxIter i_mbox = _ListMailBox.begin();

		while ( i_mbox != _ListMailBox.end() )
		{					
			// ... pour chaque groupe.
			std::list<IBasicMessageGroup *>::iterator it_grp = _Msg_grps.begin();
			while ( it_grp != _Msg_grps.end() )
			{
				std::list<const IMessageBase *> *msg_lst = (*i_mbox)->pumpMessages( **it_grp );

				if ( msg_lst->size() )
				{
					while ( !msg_lst->empty() )
					{
						IMessageBase *m = (IMessageBase *) msg_lst->front();
						_ListMessageIn.addMessage(m);
						m->release();
						msg_lst->pop_front();
					}
				}
				it_grp++;

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
		while(_ListMessage.size())
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
		NLAIAGENT::IRefrence *father =  getParent();
		if ( father )
		{
			father = father->getParent();

#ifdef _DEBUG
				const char *dbg_this_type = 	(const char *) getType();
				const char *dbg_father_type = 	(const char *) getParent()->getType();
#endif
			
			if ( father != NULL )	
			{
				const NLAIC::CIdentType &f_type = father->getType();

#ifdef _DEBUG
				const char *dbg_ffather_type = 	(const char *) father->getType();
#endif


				if( ((const NLAIC::CTypeOfObject &) f_type) & NLAIC::CTypeOfObject::tAgentInterpret )	// Le père est il un agent scripté?
				{
					const CScriptMailBox *father_mail = (const CScriptMailBox *) ( (NLAIAGENT::CAgentScript *) father )->getLocalMailBox();

					if ( father_mail != NULL )
					{
						CMessageGroup my_grp(0);
						std::list<const IMessageBase *> *f_mail = father_mail->pumpMessages( my_grp );
						while ( f_mail->size() )
						{
							const IMessageBase *translated_msg = translateMsg( f_mail->front() );
							if ( translated_msg )
								_ListMessage.push_back( translated_msg );
							f_mail->pop_front();
						}
						delete f_mail;
					}
				}
			}
		}
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