/** \file event_listener.cpp
 * <File description>
 *
 * $Id: event_listener.cpp,v 1.10 2000/12/13 15:01:58 corvazier Exp $
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

#include "nel/misc/event_listener.h"
#include "nel/misc/event_server.h"
#include "nel/misc/events.h"


namespace NLMISC {


/*
 * Constructor
 */
IEventListener::IEventListener()
{
}

// ***************************************************************************
// ***************************************************************************
// CEventListenerAsync
// ***************************************************************************
// ***************************************************************************

// ***************************************************************************
CEventListenerAsync::CEventListenerAsync()
{
	_KeyArray.resize (KeyCount);
	_KeyDownArray.resize (KeyCount);
	_KeyReleaseArray.resize (KeyCount);
	reset ();
}
// ***************************************************************************
void CEventListenerAsync::addToServer (CEventServer& server)
{
	server.addListener (EventKeyUpId, this);
	server.addListener (EventKeyDownId, this);
	server.addListener (EventSetFocusId, this);
}
// ***************************************************************************
void CEventListenerAsync::removeFromServer (CEventServer& server)
{
	server.removeListener (EventKeyUpId, this);
	server.removeListener (EventKeyDownId, this);
	server.removeListener (EventSetFocusId, this);
}
// ***************************************************************************
bool CEventListenerAsync::isKeyDown (TKey key) const
{
	return _KeyArray.get(key);
}

// ***************************************************************************
bool CEventListenerAsync::isKeyPushed (TKey key, bool release)
{
	bool	ret= _KeyDownArray.get(key) && !(_KeyReleaseArray.get(key));
	if(ret && release)
	{
		_KeyReleaseArray.set(key, true);
	}
	return ret;
}

// ***************************************************************************
void CEventListenerAsync::operator ()(const CEvent& event)
{
	// Key down ?
	if (event==EventKeyDownId)
	{
		CEventKeyDown *pEvent=(CEventKeyDown*)&event;
		_KeyArray.set (pEvent->Key);
		_KeyDownArray.set (pEvent->Key);
	}
	// Key up ?
	if (event==EventKeyUpId)
	{
		CEventKeyUp *pEvent=(CEventKeyUp*)&event;
		_KeyArray.clear (pEvent->Key);
		// Do not "raise up" the key, until someone has get the state of this key.
		if(_KeyReleaseArray.get(pEvent->Key))
		{
			_KeyDownArray.clear (pEvent->Key);
			_KeyReleaseArray.clear (pEvent->Key);
		}
	}
	// Activate false ?
	if (event==EventSetFocusId)
	{
		CEventSetFocus *pEvent=(CEventSetFocus *)&event;
		if (!pEvent->Get)
		{
			// Disactive all keys
			_KeyArray.clearAll ();
			_KeyDownArray.clearAll ();
			_KeyReleaseArray.clearAll ();
		}
	}
}


// ***************************************************************************
void CEventListenerAsync::reset ()
{
	_KeyArray.clearAll ();
	_KeyDownArray.clearAll ();
	_KeyReleaseArray.clearAll ();
}


} // NLMISC
