/** \file mailbox.cpp
 *
 * $Id: mailbox.cpp,v 1.32 2003/01/23 15:41:08 chafik Exp $
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
	
	CLocalMailBox::CLocalMailBox (const IWordNumRef *parent):IMailBox(parent),_Size(0)
	{
	}

	CLocalMailBox::CLocalMailBox(NLMISC::IStream &is):IMailBox(is),_Size(0)
	{
		load(is);
	}


	CLocalMailBox::CLocalMailBox (const CLocalMailBox &A):IMailBox(A),_RunState(A._RunState),_Size(0)
	{								
	}

	CLocalMailBox::~CLocalMailBox()
	{
		while(_ListMessageIn.begin() != _ListMessageIn.end())
		{
			IMessageBase *msg = (IMessageBase *)_ListMessageIn.back();
			msg->release();
			_ListMessageIn.pop_back();
		}		

	}

	const IMessageBase &CLocalMailBox::getMessage()
	{				
		return *_ListMessageIn.back();
	}

	void CLocalMailBox::popMessage()
	{			
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
		return _Size == 0;
	}
		
	IObjectIA::CProcessResult CLocalMailBox::sendMessage(IMessageBase *m)
	{
		m->release();		
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

	}

	// Supprime une boite aux lettre dans la liste des boites aux lettres "source" (les bals à consulter).
	void CLocalMailBox::removeMailBox(IMailBox *mail)
	{				
		eraseFromList<IMailBox *>(&_ListMailBox,mail);
		removeConnection(mail);
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

		TListMessageIter msgItr = _ListMessageIn.begin();
		while(msgItr != _ListMessageIn.end())
		{
			IMessageBase *msg = (IMessageBase *)*msgItr;

			if(msg->getSender() == a || msg->getContinuation() == a || msg->getReceiver() == a)
			{
				TListMessageIter iTmp = msgItr++;
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
		TListMailBoxCstIter k = _ListMailBox.begin();
		while(k != _ListMailBox.end())
		{
			IMailBox *mail = *k++;					
			((IWordNumRef *)(const NLAIAGENT::IWordNumRef *)*mail)->save(os);
		}

		size = _ListMessageIn.size();
		os.serial( size );
		TListMessageIter msgItr = _ListMessageIn.begin();
		while(msgItr != _ListMessageIn.end())
		{
			IMessageBase *msg = (IMessageBase *)*msgItr++;
			os.serial( (NLAIC::CIdentType &) (msg->getType()) );
			msg->save(os);
		}
				
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

	const IObjectIA::CProcessResult &CLocalMailBox::run()
	{		
		return getState();
	}

	void CLocalMailBox::getDebugString(std::string &t) const
	{
		t += "class CLocalMailBox parent";	

	}	
}
