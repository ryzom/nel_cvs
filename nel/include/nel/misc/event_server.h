/** \file event_server.h
 * <File description>
 *
 * $Id: event_server.h,v 1.1 2000/11/09 16:17:03 coutelas Exp $
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

#ifndef NL_EVENT_SERVER_H
#define NL_EVENT_SERVER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/class_id.h"
#include <map>
#include <list>


namespace NLMISC {

class CEventEmitter;
class CEvent;

/*===================================================================*/

typedef std::multimap<CClassId,void (*)(const CEvent& event)> mapListener;


/**
 * CEventServer
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2000
 */
class CEventServer
{
	mapListener _Listeners;
	std::list<CEventEmitter*> _Emitters;
	std::list<CEvent*> _Events;

public:

	/** 
	 * add event to the list
	 * \param event
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	void postEvent(CEvent * event);

	/** 
	 * get call every callbacks associated with event id
	 * \param event
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	void pump();

	/** 
	 * call every callbacks associated with event id
	 * \param event
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	void pumpEvent(const CEvent& event);
	
		
	/** 
	 * Add a callback (associated with an id)
	 * \param id
	 * \param callback
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	void addListener(CClassId id, void (*CCallBackListener)(const CEvent&) );
	
	/** 
	 * Remove a callback
	 * \param id of event's callback
	 * \param the callback to be removed
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	void removeListener(CClassId id, void (*CCallBackListener)(const CEvent&) );

	/** 
	 * Add an Emitter to the server 
	 * \param emitter
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	void addEmitter(CEventEmitter * emitter);
	
	/** 
	 * Remove an Emitter from the server 
	 * \param emitter
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	void removeEmitter(CEventEmitter * emitter);

};


/*===================================================================*/

} // NLMISC


#endif // NL_EVENT_SERVER_H

/* End of event_server.h */
