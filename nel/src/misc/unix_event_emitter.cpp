/** \file unix_event_emitter.cpp
 * <File description>
 *
 * $Id: unix_event_emitter.cpp,v 1.1 2000/12/19 09:55:14 lecroart Exp $
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

#include "nel/misc/unix_event_emitter.h"

#ifdef NL_OS_UNIX

namespace NLMISC {

CUnixEventEmitter::CUnixEventEmitter ()
{
}

void CUnixEventEmitter::submitEvents(CEventServer & server)
{
	// Window Mode cool.
	// On va empecher de bouger la fenetre.
	while (XPending(dpy))
	{
		XEvent	Event;
		XNextEvent(dpy, &Event);
		if(Event.xany.window==win && Event.type==ConfigureNotify)
		{
			XConfigureEvent	Conf;
			Conf=Event.xconfigure;
			if(Conf.x!=GLX_WinX || Conf.y!=GLX_WinY)
			{
				;
			}
		}
	}
}

void CUnixEventEmitter::processMessage (uint32 hWnd, uint32 msg, uint32 wParam, uint32 lParam, CEventServer *server)
{
	// switch d evenement
}

} // NLMISC

#endif // NL_OS_UNIX
