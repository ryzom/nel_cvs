/** \file mailbox.h
 * class for mailing box.
 *
 * $Id: mailbox.h,v 1.10 2001/05/22 16:08:01 chafik Exp $
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
	class IMailBox:public IConnectIA
	{
	public:
		typedef std::list<const IMessageBase *> tListMessage;
		typedef std::list<IMailBox *> tListMailBox;
		
	public:
		
		IMailBox(const IWordNumRef *parent);
		IMailBox(const IMailBox &A);
		IMailBox (NLMISC::IStream &is);

		virtual void save(NLMISC::IStream &os) = 0;	
		virtual void load(NLMISC::IStream &is) = 0;
		
		virtual ~IMailBox();

		virtual void addMessage(IMessageBase *msg) = 0;

		virtual void addMailBox(IMailBox *) = 0;
		virtual void removeMailBox(IMailBox *) = 0;

		virtual const IMessageBase &getMessage() = 0;
		virtual void shareMessage() = 0;
		virtual void popMessage() = 0;
		virtual sint32	getMessageCount() const = 0;

		virtual std::list<const IMessageBase *> *pumpMessages(/*IBasicMessageGroup &*/) const= 0;

		/*virtual void addGroup(IBasicMessageGroup &) = 0;
		virtual void removeGroup(IBasicMessageGroup &) = 0;
		virtual std::list<IBasicMessageGroup *> &getGroups() = 0;*/

	};

	inline void IMailBox::save(NLMISC::IStream &os)
	{				
		IConnectIA::save(os);
	}

	inline void IMailBox::load(NLMISC::IStream &is)
	{			
		IConnectIA::load(is);
	}

	class CSimpleLocalMailBox:public IMailBox
	{
	protected:
		IMailBox::tListMessage _ListMessage;
		IObjectIA::CProcessResult	_RunState;
		std::list<IBasicMessageGroup *> _Msg_grps;

	public:
		static const NLAIC::CIdentType IdSimpleLocalMailBox;
	public:
		CSimpleLocalMailBox (const IWordNumRef *parent);
		CSimpleLocalMailBox (NLMISC::IStream &is);
		CSimpleLocalMailBox (const CSimpleLocalMailBox &a);
		virtual ~CSimpleLocalMailBox();
		virtual void addMessage(IMessageBase *msg);
		virtual void addMailBox(IMailBox *);
		virtual void removeMailBox(IMailBox *);
		virtual void shareMessage() {}
		virtual const IMessageBase &getMessage();
		virtual void popMessage();
		virtual sint32	getMessageCount() const;		
		virtual std::list<const IMessageBase *> *pumpMessages(/*IBasicMessageGroup &*/) const;		
		virtual const CProcessResult &getState() const;
		virtual void setState(TProcessStatement state, IObjectIA *result);
		virtual const IObjectIA::CProcessResult &run();
//		virtual void fillMailBox();
		virtual bool isEqual(const IBasicObjectIA &a) const;
		virtual const NLAIC::IBasicType *clone() const;
		virtual const NLAIC::IBasicType *newInstance() const;
		virtual const NLAIC::CIdentType &getType() const;
		virtual void getDebugString(std::string &t) const;
		virtual void save(NLMISC::IStream &os);
		virtual void load(NLMISC::IStream &is);
		virtual void onKill(IConnectIA *);
		virtual IObjectIA::CProcessResult sendMessage(IMessageBase *);
	private:
			virtual IConnectIA &getConnection(IConnectIA &r);
	};

	
	class CLocalMailBox:public IMailBox
	{
		public:
			static const NLAIC::CIdentType IdLocalMailBox;
		public:			
			typedef std::list<const IMessageBase *>::iterator tListMessageIter;
			typedef std::list<const IMessageBase *>::const_iterator tListMessageCstIter;
			
			typedef std::list<IMailBox *>::iterator tListMailBoxIter;
			typedef std::list<IMailBox *>::const_iterator tListMailBoxCstIter;

		private:						
			IObjectIA::CProcessResult	_RunState;
			tListMessage _ListMessageIn;
			tListMessage _ListSharedMessage;
			tListMailBox _ListMailBox;

			
		public:
	
			CLocalMailBox (const IWordNumRef *parent);
			CLocalMailBox (NLMISC::IStream &is);
			CLocalMailBox (const CLocalMailBox &A);
			virtual ~CLocalMailBox();
			virtual const IMessageBase &getMessage();
			virtual void shareMessage();
			virtual void popMessage();
			virtual sint32	getMessageCount() const;
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
			virtual std::list<const IMessageBase *> *pumpMessages(/*IBasicMessageGroup &grp*/) const;
			virtual const IObjectIA::CProcessResult &run();
			virtual void getDebugString(std::string &t) const;
			/*virtual void addGroup(IBasicMessageGroup &grp);
			virtual std::list<IBasicMessageGroup *> &getGroups() ;
			virtual void removeGroup(IBasicMessageGroup &grp);*/
		private:
			void fillMailBox();

	};

	class CScriptMailBox : public CSimpleLocalMailBox
	{
	private:
		sint32 _CIndex;
		const IMessageBase *translateMsg(const IMessageBase *);
	public:
		static const NLAIC::CIdentType IdScriptMailBox;

	public:
			CScriptMailBox (const IWordNumRef *parent);
			CScriptMailBox (NLMISC::IStream &is);
			CScriptMailBox (const CScriptMailBox &A);
			virtual ~CScriptMailBox();
			virtual const IObjectIA::CProcessResult &run();
			virtual void fillMailBox();

			void setIndex(sint32);
			sint32 getIndex();

			virtual const NLAIC::IBasicType *clone() const;
			virtual const NLAIC::IBasicType *newInstance() const;
			virtual const NLAIC::CIdentType &getType() const;

	};	
}
#endif
