/** \file event_listener.h
 * <File description>
 *
 * $Id: event_listener.h,v 1.3 2000/11/10 11:28:25 berenguier Exp $
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

#ifndef NL_EVENT_LISTENER_H
#define NL_EVENT_LISTENER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/events.h"
#include "nel/misc/bit_set.h"


namespace NLMISC {

class CEvent;
class CEventServer;

/**
 * <Class description>
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2000
 */
class CEventListener
{
public:

	/// Constructor
	CEventListener();

	/**
	  * Call back of the listener.
	  * \param event is the event send to the listener
	  */
	virtual void operator ()(const CEvent& event)=0;
};


/**
 * <Class description>
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2000
 */
class CEventListenerAsync: public CEventListener
{
public:

	/// Constructor
	CEventListenerAsync();

	/** 
	  * Register the listener to the server.
	  */
	void addToServer (CEventServer& server);

	/** 
	  * Unregister the listener to the server.
	  */
	void removeFromServer (CEventServer& server);

	/**
	  * Get a key state.
	  * \param key is the key to check.
	  */
	bool isKeyPush (TKey key) const;

private:
	/*
	 * Call back of the listener.
	 * \param event is the event send to the listener
	 */
	/// Internal use
	virtual void operator ()(const CEvent& event);
	CBitSet _KeyArray;
};


} // NLMISC


#endif // NL_EVENT_LISTENER_H

/* End of event_listener.h */
