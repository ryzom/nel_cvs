/** \file event_server.cpp
 * <File description>
 *
 * $Id: event_server.cpp,v 1.1 2000/11/09 16:17:43 coutelas Exp $
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

#include "nel/misc/event_server.h"
#include "nel/misc/event_emitter.h"
#include "nel/misc/events.h"


namespace NLMISC {



/*------------------------------------------------------------------*\
							postEvent()
\*------------------------------------------------------------------*/
void CEventServer::postEvent(CEvent * event)
{
	_Events.push_back(event);
}



/*------------------------------------------------------------------*\
							pump()
\*------------------------------------------------------------------*/
void CEventServer::pump()
{
	std::list<CEventEmitter*>::iterator item = _Emitters.begin();
	
	// getting events from emitters
	while(item!=_Emitters.end())
	{
		// ask emitters to submit their events to server
		(*item)->submitEvents(*this);
		item++;
	}

	
	std::list<CEvent*>::iterator itev = _Events.begin();
	
	while(itev!=_Events.end())
	{
		// pump event
		pumpEvent(**itev);
		itev++;
	}
}


/*------------------------------------------------------------------*\
							pumpEvent()
\*------------------------------------------------------------------*/
void CEventServer::pumpEvent(const CEvent& event)
{
	// taking id
	uint64 id = (uint64) event;

	// looking for the first occurence of id
	mapListener::iterator it = _Listeners.find(id);

	// calling every callbacks
	while(it!=_Listeners.end() && (uint64)(*it).first == id)
	{
		(*it).second(event);
	}
}



/*------------------------------------------------------------------*\
							addListener()
\*------------------------------------------------------------------*/
void CEventServer::addListener(CClassId id, 
							   void (*CCallBackListener)(const CEvent&) )
{
	_Listeners.insert( mapListener::value_type(id, CCallBackListener));
}


/*------------------------------------------------------------------*\
							removeListener()
\*------------------------------------------------------------------*/
void CEventServer::removeListener(CClassId id, 
								  void (*CCallBackListener)(const CEvent&) )
{
	// looking for the first occurence of id
	mapListener::iterator it = _Listeners.find(id);

	// looking for occurence with the right callback
	while(it!=_Listeners.end() && (*it).first == id)
	{
		if((*it).second==CCallBackListener)
		{
			// erasing pair
			_Listeners.erase(it);
			return;
		}
		it++;
	}
}


/*------------------------------------------------------------------*\
							addEmitter()
\*------------------------------------------------------------------*/
void CEventServer::addEmitter(CEventEmitter * emitter)
{
	_Emitters.push_back(emitter);
}


/*------------------------------------------------------------------*\
							removeEmitter()
\*------------------------------------------------------------------*/
void CEventServer::removeEmitter(CEventEmitter * emitter)
{
	_Emitters.remove(emitter);
}


} // NLMISC
