/** \file event_listener.cpp
 * <File description>
 *
 * $Id: event_listener.cpp,v 1.2 2000/11/10 11:04:55 corvazier Exp $
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
CEventListener::CEventListener()
{
}

// ***************************************************************************
// ***************************************************************************
// CEventListenerAsynch
// ***************************************************************************
// ***************************************************************************

// ***************************************************************************
CEventListenerAsynch::CEventListenerAsynch()
{
	_KeyArray.resize (KeyCount);
}
// ***************************************************************************
void CEventListenerAsynch::addToServer (CEventServer& server)
{
	server.addListener (EventKeyUpId, this);
	server.addListener (EventKeyDownId, this);
}
// ***************************************************************************
void CEventListenerAsynch::removeFromServer (CEventServer& server)
{
	server.removeListener (EventKeyUpId, this);
	server.removeListener (EventKeyDownId, this);
}
// ***************************************************************************
bool CEventListenerAsynch::isKeyPush (TKey key) const
{
	return _KeyArray.get(key);
}
// ***************************************************************************
void CEventListenerAsynch::operator ()(const CEvent& event)
{
	// Key down ?
	if (event==EventKeyDownId)
	{
		CEventKeyDown *pEvent=(CEventKeyDown*)&event;
		_KeyArray.set (pEvent->Key);
	}
	// Key up ?
	if (event==EventKeyUpId)
	{
		CEventKeyUp *pEvent=(CEventKeyUp*)&event;
		_KeyArray.clear (pEvent->Key);
	}
}


} // NLMISC
