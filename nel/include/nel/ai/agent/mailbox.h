/** \file mailbox.h
 * class for mailing box.
 *
 * $Id: mailbox.h,v 1.3 2001/01/08 10:47:05 chafik Exp $
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

#include "agent/messagerie.h"
#include "agent/baseia.h"

namespace NLAIAGENT
{	
	class IMailBox:public IConnectIA
	{
	public:
		typedef std::list<const IMessageBase *> tListMessage;
		typedef std::list<IMailBox *> tListMailBox;
		
	public:
		
		IMailBox(const IWordNumRef *parent):IConnectIA(parent)
		{
		}

		IMailBox(const IMailBox &A):IConnectIA(A)
		{
		}
		
		IMailBox (NLMISC::IStream &is):IConnectIA(is)
		{			
		}	

		virtual void save(NLMISC::IStream &os) = 0;	
		virtual void load(NLMISC::IStream &is) = 0;
		
		virtual ~IMailBox()
		{
		}
		
		virtual void addMessage(IMessageBase *msg) = 0;

		virtual void addMailBox(IMailBox *) = 0;
		virtual void removeMailBox(IMailBox *) = 0;

		virtual const IMessageBase &getMessage() = 0;
		virtual void popMessage() = 0;
		virtual sint32	getMessageCount() const = 0;

		virtual std::list<const IMessageBase *> *pumpMessages(IBasicMessageGroup &) = 0;

		virtual void addGroup(IBasicMessageGroup &) = 0;
		virtual void removeGroup(IBasicMessageGroup &) = 0;
		virtual std::list<IBasicMessageGroup *> &getGroups() = 0;

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
	private:
		IMailBox::tListMessage _ListMessage;
		IObjectIA::CProcessResult	_RunState;
		std::list<IBasicMessageGroup *> _Msg_grps;

	public:
		static const NLAIC::CIdentType IdSimpleLocalMailBox;
	public:
		CSimpleLocalMailBox (const IWordNumRef *parent):IMailBox(parent)
		{
		}

		CSimpleLocalMailBox (NLMISC::IStream &is):IMailBox(is)
		{
			load(is);
		}


		CSimpleLocalMailBox (const CSimpleLocalMailBox &a):IMailBox(a)
		{					
			tListMessage::const_reverse_iterator i = a._ListMessage.rbegin();
			while(i != a._ListMessage.rend())
			{
				_ListMessage.push_back(*i++);
				((IMessageBase *)_ListMessage.back())->incRef();
			}
			_RunState = a._RunState;
		}
		
		~CSimpleLocalMailBox()
		{			
			while(_ListMessage.size())
			{
				IMessageBase *m = (IMessageBase *)_ListMessage.back();
				m->release();
				_ListMessage.pop_back();				
			}
		}
		
		virtual void addMessage(IMessageBase *msg)
		{
			_ListMessage.push_back(msg);
			//msg->incRef();
		}

		virtual void addMailBox(IMailBox *)
		{
		}

		virtual void removeMailBox(IMailBox *)
		{
		}

		virtual const IMessageBase &getMessage()
		{
			return *_ListMessage.back();

		}
		
		virtual void popMessage()
		{
			IMessageBase *msg = (IMessageBase *)_ListMessage.back();
			msg->release();
			_ListMessage.pop_back();
		}

		virtual sint32	getMessageCount() const
		{
			return _ListMessage.size();
		}

		virtual std::list<const IMessageBase *> *pumpMessages(IBasicMessageGroup &) 
		{

			return NULL;
		}

		virtual void addGroup(IBasicMessageGroup &)
		{

		}

		virtual void removeGroup(IBasicMessageGroup &)
		{

		}

		virtual std::list<IBasicMessageGroup *> &getGroups()
		{
			return _Msg_grps;
		}

		virtual const CProcessResult &getState() const 
		{
			return _RunState;
		}

		virtual void setState(TProcessStatement state, IObjectIA *result)
		{
			_RunState.ResultState = state;
			_RunState.Result = result;
		}

		const IObjectIA::CProcessResult &run()
		{
			return IObjectIA::ProcessRun;
		}

		virtual bool isEqual(const IBasicObjectIA &a) const
		{
			const IMailBox &m = (const IMailBox &)a;
			return (const IWordNumRef &)m == (const IWordNumRef &)*this;
		}

		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *a = new CSimpleLocalMailBox(*this);
			a->incRef();
			return a;
		}

		virtual const NLAIC::IBasicType *newInstance() const
		{				
			NLAIC::IBasicType *a;
			if(getParent() != NULL) a = new	CSimpleLocalMailBox((const IWordNumRef *)getParent());
			else a = new CSimpleLocalMailBox((const IWordNumRef *)NULL);
			a->incRef();
			return a;
		}

		virtual const NLAIC::CIdentType &getType() const
		{
			return IdSimpleLocalMailBox;
		}

		virtual void getDebugString(char *t) const
		{
			sprintf(t,"class CSimpleLocalMailBox");
		}

		virtual void save(NLMISC::IStream &os)
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

		virtual void load(NLMISC::IStream &is)
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

		virtual void onKill(IConnectIA *)
		{							
		}

		virtual IObjectIA::CProcessResult sendMessage(IMessageBase *)
		{
			return IObjectIA::ProcessRun;
		}

	private:
			virtual IConnectIA &getConnection(IConnectIA &r)
			{								
				addInConnectedList(&r);				
				return *this;
			}			

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
			CVectorMsgContainer _ListMessageIn;
			tListMailBox _ListMailBox;
			std::list<IBasicMessageGroup *> _Msg_grps;
		public:
	
			CLocalMailBox (const IWordNumRef *parent):IMailBox(parent)
			{
			}

			CLocalMailBox (NLMISC::IStream &is):IMailBox(is)
			{
				load(is);
			}


			CLocalMailBox (const CLocalMailBox &A):IMailBox(A),_RunState(A._RunState)
			{						
				tListMailBox::const_reverse_iterator i = A._ListMailBox.rbegin();
				while(i != A._ListMailBox.rend())
				{
					_ListMailBox.push_back(*i++);					
				}	
			}

			virtual ~CLocalMailBox()
			{
			}
						
			virtual const IMessageBase &getMessage()
			{				
				return *_ListMessageIn.getMessage();
			}

			virtual void popMessage()
			{	
				_ListMessageIn.popMessage();
			}

			virtual sint32	getMessageCount() const
			{
				return _ListMessageIn.size();
			}
			
			virtual void sendMessage(const IBasicAgent &,const IBaseGroupType &)
			{			
				//_listMessageOut.push_back((const IMessageBase *)IMessageBase(msg,a).clone());
			}

			virtual IObjectIA::CProcessResult sendMessage(IMessageBase *)
			{
				//_listMessageOut.push_back((const IMessageBase *)msg.clone());
				return IObjectIA::ProcessRun;
			}

			virtual void addMessage(IMessageBase *msg)
			{
				_ListMessageIn.addMessage(msg);
			}
			
			// Ajoute une boite aux lettre dans la liste des boites aux lettres "source" (les bals à consulter).
			virtual void addMailBox(IMailBox *mail)
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
			virtual void removeMailBox(IMailBox *mail)
			{				
				eraseFromList<IMailBox *>(&_ListMailBox,mail);
				removeConnection(*mail);
			}

			virtual void setName(const IVarName &)
			{				
			}

			virtual const NLAIC::IBasicType *clone() const
			{
				NLAIC::IBasicType *a = new CLocalMailBox(*this);
				a->incRef();
				return a;
			}

			virtual const NLAIC::IBasicType *newInstance() const
			{				
				NLAIC::IBasicType *a;
				if(getParent() != NULL) a = new	CLocalMailBox((const IWordNumRef *)getParent());
				else a = new CLocalMailBox((const IWordNumRef *)NULL);
				a->incRef();
				return a;
			}

			virtual const NLAIC::CIdentType &getType() const
			{
				return IdLocalMailBox;
			}

			virtual void onKill(IConnectIA *a)
			{				
				eraseFromList<IMailBox *>(&_ListMailBox,(IMailBox *)a);
			}
			
			virtual bool isEqual(const IBasicObjectIA &a) const
			{
				const IMailBox &m = (const IMailBox &)a;
				return (const IWordNumRef &)m == (const IWordNumRef &)*this;
			}

			virtual void save(NLMISC::IStream &os)
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

			virtual void load(NLMISC::IStream &is)
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

			virtual const CProcessResult &getState() const 
			{
				return _RunState;
			}

			virtual void setState(TProcessStatement state, IObjectIA *result)
			{
				_RunState.ResultState = state;
				_RunState.Result = result;
			}

			virtual std::list<const IMessageBase *> *pumpMessages(IBasicMessageGroup &grp) {
				return _ListMessageIn.pumpMessages(grp);
			}

			virtual const IObjectIA::CProcessResult &run()
			{
				setState(processBuzzy,NULL);	
				tListMailBoxIter j;
				fillMailBox();
				setState(processIdle,NULL);
				return getState();
			}


			virtual void getDebugString(char *t) const
			{
				sprintf(t,"class CLocalMailBox");
			}

			virtual void addGroup(IBasicMessageGroup &grp)
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

			virtual std::list<IBasicMessageGroup *> &getGroups() 
			{
				return _Msg_grps;
			}


			virtual void removeGroup(IBasicMessageGroup &grp)
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

		private:
			
			void fillMailBox()
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
	};
}
#endif
