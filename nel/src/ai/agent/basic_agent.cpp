/** \file basic_agent.cpp
 *
 * $Id: basic_agent.cpp,v 1.1 2001/01/05 10:53:49 chafik Exp $
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
#include "agent/agent.h"

namespace NLIAAGENT
{

	
	IAgentComposite::IAgentComposite(const IAgentComposite &a): IBasicAgent(a)
	{
		std::list<IBasicAgent *>::const_iterator i = a._AgentList.begin();
		while(i != a._AgentList.end())
		{				
			IBasicAgent *b = ( *i++ );
			b->incRef();	
			addChild( b );
		}
	}

	IAgentComposite::IAgentComposite(IBasicAgent *parent): IBasicAgent(parent != NULL ? (const IWordNumRef *)*parent:NULL)
	{
		if(parent) parent->addChild(this);
	}

	IAgentComposite::IAgentComposite(IBasicAgent *parent,IMailBox	*m): IBasicAgent(parent != NULL ? (const IWordNumRef *)*parent:NULL,m)
	{
		if(parent) parent->addChild(this);
	}

	IAgentComposite::~IAgentComposite()			
	{
		/*std::list<IBasicAgent *>::iterator i = _AgentList.begin();
		while(i != _AgentList.end())
		{			
			IBasicAgent *b = (*i++);
		}*/
		deleteListe();
	}

	// Ajoute un fils à l'agent.
	std::list<IBasicAgent *>::iterator IAgentComposite::addChild(IBasicAgent *p)		
	{			
		p->incRef();
		_AgentList.push_front((IBasicAgent *)p);
		// Donne au fils accès à la boite aux lettres du père
		p->getMail()->addMailBox( this->getMail() );
		return _AgentList.begin();
	}

	void IAgentComposite::cpyChild(const IBasicAgent &p)
	{
		_AgentList.push_front((IBasicAgent *)p.clone());
	}

	void IAgentComposite::removeChild(const IBasicAgent &p)
	{			
		std::list<IBasicAgent *>::iterator i = _AgentList.begin();
		while(i != _AgentList.end())
		{					
			if(*((*i)) == p)
			{
				(*i)->release();
				_AgentList.erase(i);
				// Supprime chez l'ancien fils la boite au lettre du père
				p.getMail()->removeMailBox( this->getMail() );
				return;
			}
			i++;
		}
	}

	void IAgentComposite::removeChild(const IBasicAgent *p)
	{			
		std::list<IBasicAgent *>::iterator i = _AgentList.begin();
		while(i != _AgentList.end())
		{					
			if((*i) == p)
			{
				(*i)->release();
				_AgentList.erase(i);
				// Supprime chez l'ancien fils la boite au lettre du père
				p->getMail()->removeMailBox( this->getMail() );
				return;
			}
			i++;
		}
	}
	void IAgentComposite::deleteListe()
	{
		std::list<IBasicAgent *>::iterator i = _AgentList.begin();
		while(i != _AgentList.end())
		{				
			IBasicAgent *b = (*i++);
			b->setParent(NULL);
			b->release();				
		}
	}

	void IAgentComposite::save(NLMISC::IStream &os)
	{				
		IBasicAgent::save(os);

		sint32 size = _AgentList.size();
		os.serial(size);
		std::list<IBasicAgent *>::const_iterator i = _AgentList.begin();
		while(i != _AgentList.end())
		{
			IBasicAgent &a = *(*i++);
			os.serial( (NLIAC::CIdentType &) a.getType() );
			a.save(os);			
		}
	}

	void IAgentComposite::load(NLMISC::IStream &is)
	{			
		deleteListe();
		IBasicAgent::load(is);
		NLIAC::CIdentTypeAlloc id;
		sint32 i;			
		
		is.serial(i);
		while(i--)
		{
			is.serial( id );
			IBasicAgent &a = *((IBasicAgent *)id.allocClass());
			a.load(is);
			addChild(&a);
		}
	}		
}