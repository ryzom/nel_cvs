/** \file mailbox.h
 * class for mailing box.
 *
 * $Id: mailbox.h,v 1.15 2003/01/21 11:24:25 chafik Exp $
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
#ifndef NL_MAIL_BOX_H
#define NL_MAIL_BOX_H

#include "nel/ai/agent/msg.h"
#include "nel/ai/agent/baseai.h"

namespace NLAIAGENT
{	
	/**
	Abstract class for mail box management.
	*/
	class IMailBox:public IConnectIA
	{
	public:
		typedef std::list<const IMessageBase *> TListMessage;
		typedef std::list<IMailBox *> TListMailBox;
		
	public:
		
		IMailBox(const IWordNumRef *parent);
		IMailBox(const IMailBox &A);
		IMailBox (NLMISC::IStream &is);
		virtual ~IMailBox();

		
		///Redefinition of this method to force user to write it.
		//@{		
		virtual void save(NLMISC::IStream &os) = 0;	
		virtual void load(NLMISC::IStream &is) = 0;		

		//@}

		///Add a agent message to the buffer
		virtual void addMessage(IMessageBase *msg) = 0;
		///Add a new mail box to this mail, all message hapend here will be acheived to the mail box connected.
		virtual void addMailBox(IMailBox *) = 0;
		///Remove a mail box.
		virtual void removeMailBox(IMailBox *) = 0;
		///Get the first message from the buffer.
		virtual const IMessageBase &getMessage() = 0;	
		///Pop the first message from the buffer.
		virtual void popMessage() = 0;
		///get message count.
		virtual sint32	getMessageCount() const = 0;
		///test if buffer is empty.
		virtual bool isEmpty() const = 0;				

		virtual const TListMessage &getMesseageListe() const = 0;		

	};

	inline void IMailBox::save(NLMISC::IStream &os)
	{				
		IConnectIA::save(os);
	}

	inline void IMailBox::load(NLMISC::IStream &is)
	{			
		IConnectIA::load(is);
	}	

	
	class CLocalMailBox:public IMailBox
	{
		public:
			static const NLAIC::CIdentType IdLocalMailBox;
		public:			
			typedef std::list<const IMessageBase *>::iterator TListMessageIter;
			typedef std::list<const IMessageBase *>::const_iterator TListMessageCstIter;
			
			typedef std::list<IMailBox *>::iterator TListMailBoxIter;
			typedef std::list<IMailBox *>::const_iterator TListMailBoxCstIter;

		private:						
			IObjectIA::CProcessResult	_RunState;
			TListMessage _ListMessageIn;
			TListMessage _ListSharedMessage;
			TListMailBox _ListMailBox;
			sint _Size;
			
		public:
	
			CLocalMailBox (const IWordNumRef *parent);
			CLocalMailBox (NLMISC::IStream &is);
			CLocalMailBox (const CLocalMailBox &A);
			virtual ~CLocalMailBox();
			virtual const IMessageBase &getMessage();			
			virtual void popMessage();
			virtual sint32	getMessageCount() const;
			virtual bool isEmpty() const;
			//virtual void sendMessage(const IBasicAgent &,const IBaseGroupType &);
			virtual IObjectIA::CProcessResult sendMessage(IMessageBase *);
			virtual void addMessage(IMessageBase *msg);
			// Ajoute une boite aux lettre dans la liste des boites aux lettres "source" (les bals à consulter).
			virtual void addMailBox(IMailBox *mail);
			// Supprime une boite aux lettre dans la liste des boites aux lettres "source" (les bals à consulter).
			virtual void removeMailBox(IMailBox *mail);
			virtual void setName(const IVarName &);
			virtual const NLAIC::IBasicType *clone() const;
			virtual const NLAIC::IBasicType *newInstance() const;
			virtual const NLAIC::CIdentType &getType() const;
			virtual void onKill(IConnectIA *a);
			virtual bool isEqual(const IBasicObjectIA &a) const;
			virtual void save(NLMISC::IStream &os);
			virtual void load(NLMISC::IStream &is);
			virtual const CProcessResult &getState() const ;
			virtual void setState(TProcessStatement state, IObjectIA *result);			
			virtual const IObjectIA::CProcessResult &run();
			virtual void getDebugString(std::string &t) const;
			virtual const TListMessage &getMesseageListe() const 
			{
				return _ListMessageIn;
			}
			virtual sint size() const
			{
				return _Size;
			}			

	};
	
}
#endif
