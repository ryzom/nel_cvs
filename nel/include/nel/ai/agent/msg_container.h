/** \file msg_container.h
 * class for container of message in the mail box.
 *
 * $Id: msg_container.h,v 1.3 2001/05/22 16:08:01 chafik Exp $
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

#ifndef NL_IA_MSG_CONTAINER_H
#define NL_IA_MSG_CONTAINER_H

namespace NLAIAGENT {

	class IMessageBase;


	// Classe de base abstraite des containers de messages
	class  IBasicMsgContainer : public NLAIC::IBasicInterface
	{
		protected:
			

		public:
			IBasicMsgContainer() { }
			IBasicMsgContainer(const IBasicMsgContainer &) {}
			virtual ~IBasicMsgContainer() {	}

			virtual void addMessage(IMessageBase *) = 0;							// Ajout d'un message dans le container (il est cloné)
			virtual const IMessageBase *getMessage() = 0;								// Récupère le message courant dans le container
			virtual void popMessage() = 0;											// Retire le message courant du container


			virtual std::list<const IMessageBase *> *pumpMessages(const IBasicMessageGroup &) =0;			// Renvoie une liste de tous les messages d'un groupe

			virtual const sint32 size() const = 0;
			virtual std::list<const IMessageBase *> *operator[](const IBasicMessageGroup &) = 0;	
	};

	// Container de messages contenant un tableau de listes de messages, l'index
	// du tableau étant le groupe du message
	class CVectorMsgContainer : public IBasicMsgContainer
	{
		private:
			std::vector< std::list<const IMessageBase *> > _Messages;
			std::list<const IBasicMessageGroup *> _MsgIndex;
			sint32 _Size;
			sint32 _CurrentGrp;

			sint32 findIndex(const IBasicMessageGroup &grp);

		public:
			static const NLAIC::CIdentType IdVectorMsgContainer;
		public:
			CVectorMsgContainer();
			CVectorMsgContainer(const CVectorMsgContainer &);
			virtual ~CVectorMsgContainer();

			virtual void save(NLMISC::IStream &);
			virtual void load(NLMISC::IStream &);
			virtual const NLAIC::IBasicType *clone() const;
			virtual const NLAIC::IBasicType *newInstance() const;
				
			virtual const NLAIC::CIdentType &getType() const;			

			// TODO
			void getDebugString(std::string &t) const {  }

			virtual void addMessage(IMessageBase *); 
			virtual std::list<const IMessageBase *> *CVectorMsgContainer::pumpMessages(const IBasicMessageGroup &);

//			virtual list<const IMessageBase *> &pumpMessages() {	return _Messages[0]; }
			virtual const IMessageBase *getMessage();
			virtual void popMessage();

			virtual const sint32 size() const	{ return _Size; }
			virtual std::list<const IMessageBase *> *operator[](const IBasicMessageGroup &);
	};	
}
#endif
