/** \file event_emitter.cpp
 * <File description>
 *
 * $Id: event_emitter.cpp,v 1.5 2000/11/10 11:04:55 corvazier Exp $
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
	MSG	msg;
	while ( PeekMessage(&msg,NULL,0,0,PM_REMOVE) )
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (msg.hwnd==(HWND)_HWnd)
		{
			switch (msg.message)
			{
				case WM_KEYDOWN:
					server.postEvent (new CEventKeyDown ((TKey)msg.wParam));
				break;
				case WM_KEYUP:
					server.postEvent (new CEventKeyUp ((TKey)msg.wParam));
				break;
				case WM_CHAR:
					server.postEvent (new CEventChar ((ucchar)msg.wParam));
				/*	
				case WM_MBUTTONDOWN:
					server.postEvent (new CEventMouseDown (GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam));
				case WM_MBUTTONUP:
					server.postEvent (new CEventMouseUp (GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam));
				*/	
				break;
			}
		}
	}
}

#endif // NL_OS_WINDOWS

} // NLMISC
