/** \file msg_container.cpp
 *
 * $Id: msg_container.cpp,v 1.10 2003/01/13 16:58:59 chafik Exp $
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
#include "nel/ai/agent/msg_container.h"

namespace NLAIAGENT
{


	CVectorMsgContainer::CVectorMsgContainer()
	{
		_Size = 0;
		_CurrentGrp = 0;
	}

	CVectorMsgContainer::CVectorMsgContainer(const CVectorMsgContainer &cp)
	{
		_Size = cp.size();
		_CurrentGrp = cp._CurrentGrp;
		_MsgIndex = cp._MsgIndex;
		sint32 index = 0;

		std::list<const IMessageBase *>::const_iterator it_msg;

		while ( index != (sint32)cp._Messages.size() )
		{
			_Messages.push_back( std::list<const IMessageBase *>() );		
			it_msg = cp._Messages[index].begin();

			while ( it_msg != cp._Messages[index].end() )
			{
				_Messages[ index ].push_back(  (const IMessageBase *) (*it_msg)->clone() );
				it_msg++;
			}
			index++;
		}
		_MsgIndex = cp._MsgIndex;
	}

	CVectorMsgContainer::~CVectorMsgContainer()
	{
		sint32 index = 0;
		std::list<const IMessageBase *>::const_iterator it_msg;

		while ( index != (sint32)_Messages.size() )	
		{
			if ( ! _Messages[index].empty() )
			{
				it_msg = _Messages[index].begin();

				while ( it_msg != _Messages[index].end() )
				{
					const IMessageBase *m = *it_msg;
					it_msg++;
					delete m;
				}
			}
			index++;
		}

		std::list<const IBasicMessageGroup *>::iterator it_grp = _MsgIndex.begin();
		while ( it_grp != _MsgIndex.end() )
		{
			delete *it_grp;
			it_grp++;
		}
		
	}

	const NLAIC::IBasicType *CVectorMsgContainer::clone() const
	{
		NLAIC::IBasicType *x = new CVectorMsgContainer( *this );
		return x;
	}

	const NLAIC::IBasicType *CVectorMsgContainer::newInstance() const
	{
		NLAIC::IBasicType *x = new CVectorMsgContainer( *this );
		return x;
	}	
	const NLAIC::CIdentType &CVectorMsgContainer::getType() const	
	{
		
		return IdVectorMsgContainer;
	}

	// Adds a message to its index group
	void CVectorMsgContainer::addMessage(IMessageBase *msg)
	{
		/*sint32 index = findIndex( msg->getGroup() );
		if ( index < 0 )
		{
			index = _MsgIndex.size();
			_MsgIndex.push_back( new CMessageGroup( msg->getGroup().getId() )  );
			_Messages.push_back( std::list<const IMessageBase *>() );
		}
#ifdef NL_DEBUG
		sint dbg = _Messages.size();
		dbg = _Messages[ index ].size();
#endif
		_Messages[ index ].push_back( msg );

#ifdef NL_DEBUG	
		dbg = _Messages[ index ].size();
#endif

		_Size++;*/
	}

	const IMessageBase *CVectorMsgContainer::getMessage()
	{
		// S'il reste des messages
		if ( _Size )
		{
			// Si pas de messages dans le groupe courant on passe au suivant
			if ( _Messages[ _CurrentGrp ].empty() )
			{
				while ( _CurrentGrp != (sint32)_Messages.size() && _Messages[ _CurrentGrp ].empty() )
					_CurrentGrp++;
			}
			return _Messages[ _CurrentGrp ].front();
		}
		else 
			return NULL;
	}

	void CVectorMsgContainer::popMessage()
	{
		if ( _Size )
		{
			IMessageBase *m = (IMessageBase *)_Messages[ _CurrentGrp ].front();
			m->release();
			_Messages[ _CurrentGrp ].pop_front();
			_Size--;
			
			if ( _Size == 0 )
				_CurrentGrp = 0;
			else
				if ( _Messages [_CurrentGrp ].empty() )
					_CurrentGrp++;
		}
	}

	std::list<const IMessageBase *> *CVectorMsgContainer::pumpMessages(const IBasicMessageGroup &grp)
	{

		std::list<const IMessageBase *> *msg_list = new std::list<const IMessageBase *>;

		sint32 index = findIndex( grp );

		if (index >= 0 ) 
		{
			std::list<const IMessageBase *>::iterator it_msg = _Messages[index].begin();

			while ( it_msg != _Messages[ index ].end() )
			{
				msg_list->push_back( (const IMessageBase *) (*it_msg)->clone() );
				it_msg++;
			}
		}

		return msg_list;
	}

	sint32 CVectorMsgContainer::findIndex(const IBasicMessageGroup &grp)
	{
		sint32 i = 0;
		std::list<const IBasicMessageGroup *>::iterator it_grp = _MsgIndex.begin();

		while ( it_grp != _MsgIndex.end() )
		{
			if ( grp == *it_grp )
			{
				return i;
			}
			i++;
			it_grp++;
		}
		return -1;
	}

	// TODO: changer en référence sur la liste
	std::list<const IMessageBase *> *CVectorMsgContainer::operator[](const IBasicMessageGroup &grp)
	{
		return pumpMessages( grp );
	}


	void CVectorMsgContainer::save(NLMISC::IStream &os)
	{

		// Sauvegarde de l'index des groupes
		sint32 size = _MsgIndex.size();
		os.serial( size );
		std::list<const IBasicMessageGroup *>::const_iterator it_grp = _MsgIndex.begin();
		while ( it_grp != _MsgIndex.end() )
		{
			os.serial( (NLAIC::CIdentType &) (*it_grp)->getType() );
			((IBasicMessageGroup *)(*it_grp))->save( os );
			it_grp++;
		}


		// Sauvegarde des messages
		size = _Size;
		os.serial( size );
		std::list<const IMessageBase *>::const_iterator it_msg;
		
		if ( _Size )
		{
			sint32 index = 0;
			while ( index != (sint32)_Messages.size() )
			{
				it_msg =_Messages[index].begin();
				while ( it_msg != _Messages[index].end() )
				{
					IMessageBase *msg = (IMessageBase *)*it_msg;
					os.serial( (NLAIC::CIdentType &) msg->getType() );
					msg->save(os);				
					it_msg++;
				}	
				index++;
			}
		}

	}

	void CVectorMsgContainer::load(NLMISC::IStream &is)
	{
		sint32 i;
		NLAIC::CIdentTypeAlloc id;

		// Chargement de l'index des groupes
		sint32 nb_grps;
		is.serial( nb_grps );
		while ( nb_grps-- )
		{
			is.serial( id );
			IBasicMessageGroup *tmp_grp = (IBasicMessageGroup *) id.allocClass();						
			tmp_grp->load( is );
			_MsgIndex.push_back( tmp_grp );
		}

		// Chargement des messages
		is.serial( i );
		if ( i )
		{
			while ( i-- )
			{						
				is.serial( id );
				IMessageBase *msg = (IMessageBase *)id.allocClass();
				msg->load(is);
				addMessage(msg);
			}
		}

	}
}
