/** \file event_listener.h
 * events listener
 *
 * $Id: event_listener.h,v 1.11 2002/08/21 09:36:01 lecroart Exp $
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
 * Interface for event listener. A listener provides a callback.
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2000
 */
class IEventListener
{
public:

	/// Constructor
	IEventListener();

	/**
	  * Call back of the listener.
	  * \param event is the event send to the listener
	  */
	virtual void operator ()(const CEvent& event)=0;
};


/**
 * CEventListenerAsync
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2000
 */
class CEventListenerAsync: public IEventListener
{
public:

	/// Constructor
	CEventListenerAsync();
	virtual ~CEventListenerAsync() {}

	/** 
	  * Register the listener to the server.
	  */
	void addToServer (CEventServer& server);

	/** 
	  * Unregister the listener to the server.
	  */
	void removeFromServer (CEventServer& server);

	/**
	  * Get a key down instant state.
	  * \param key is the key to check.
	  */
	bool isKeyDown(TKey key) const;

	/**
	  * Get if a the state of key has pushed since the last call of isKeyPushed with release=true.
	  * \param key is the key to check.
	  * \param release if true, the pushed state of the key is released (force to be false).
	  *		It will return to true next time only if key is released and then re-pushed.
	  * \see reset()
	  */
	bool isKeyPushed (TKey key, bool release=true);


	/**
	  * Clear all the Down states to false. Usefull sometimes when you don't bother what have been pushed before.
	  * e.g.: your app listen/test to the key 'A' and 'B' for a certain long period. Then, it test 'C' and 'D' later.
	  * If the user has press (by error) the key 'C' during the first period, this API has record it, and then, at the
	  * second period, isKeyDown(KeyC) will return true the first time the key is tested, unless if you do a 
	  * reset() at the beggining of the second period.
	  * Clear all the pushed states to false too.
	  * \see isKeyDown()
	  */
	void reset ();


protected:
	/*
	 * Call back of the listener.
	 * \param event is the event send to the listener
	 */
	virtual void operator ()(const CEvent& event);
	CBitSet _KeyArray;
	// Must have 2 arrays because of key repetition...
	CBitSet _KeyDownArray, _KeyReleaseArray;
};


} // NLMISC


#endif // NL_EVENT_LISTENER_H

/* End of event_listener.h */

