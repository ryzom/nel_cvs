/** \file event_emitter.cpp
 * <File description>
 *
 * $Id: event_emitter.cpp,v 1.9 2000/11/13 13:38:24 corvazier Exp $
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

#include "nel/misc/events.h"
#include "nel/misc/event_emitter.h"
#include "nel/misc/event_server.h"
#include "nel/misc/types_nl.h"


namespace NLMISC {

#ifdef NL_OS_WINDOWS
#include <windows.h>

void CEventEmitterWin32::submitEvents(CEventServer & server)
{
	// Dispatch sended messages
	_InternalServer.setServer (&server);
	_InternalServer.pump ();

	MSG	msg;
	while ( PeekMessage(&msg,NULL,0,0,PM_REMOVE) )
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (msg.hwnd==(HWND)_HWnd)
		{
			//processMessage ((uint32)msg.hwnd, msg.message, msg.wParam, msg.lParam, &server);
		}
	}
}

void CEventEmitterWin32::processMessage (uint32 hWnd, uint32 msg, uint32 wParam, uint32 lParam, CEventServer *server)
{
	if (!server)
		server=&_InternalServer;
	switch (msg)
	{
		case WM_KEYDOWN:
			server->postEvent (new CEventKeyDown ((TKey)wParam, this));
			break;
		case WM_KEYUP:
			server->postEvent (new CEventKeyUp ((TKey)wParam, this));
			break;
		case WM_CHAR:
			server->postEvent (new CEventChar ((ucchar)wParam, this));
			break;
		case WM_ACTIVATE:
			if (WA_INACTIVE==LOWORD(wParam))
				server->postEvent (new CEventActivate (false, this));
			else
				server->postEvent (new CEventActivate (true, this));
			break;
		case WM_KILLFOCUS:
			server->postEvent (new CEventSetFocus (false, this));
			break;
		case WM_SETFOCUS:
			server->postEvent (new CEventSetFocus (true, this));
			break;
	}
}

#endif // NL_OS_WINDOWS

} // NLMISC
