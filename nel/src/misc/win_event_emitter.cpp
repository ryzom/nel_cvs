/** \file win_event_emitter.cpp
 * class CWinEnventEmitter
 *
 * $Id: win_event_emitter.cpp,v 1.1 2000/12/19 09:55:14 lecroart Exp $
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
#include "nel/misc/win_event_emitter.h"
#include "nel/misc/event_server.h"
#include "nel/misc/types_nl.h"

#ifdef NL_OS_WINDOWS
#include <windows.h>

/** 
  * Needed for definition of WM_MOUSEWHEEL. It should be in winuser.h 
  * but not under win98.. strange.. 
  */
#include <zmouse.h>


namespace NLMISC {

/*------------------------------------------------------------------*\
							submitEvents()
\*------------------------------------------------------------------*/
void CWinEventEmitter::submitEvents(CEventServer & server)
{
	MSG	msg;
	while ( PeekMessage(&msg,(HWND)_HWnd,0,0,PM_REMOVE) )
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// Dispatch sended messages
	_InternalServer.setServer (&server);
	_InternalServer.pump ();

}


/*------------------------------------------------------------------*\
							processMessage()
\*------------------------------------------------------------------*/


TMouseButton getMouseButton (uint32 wParam)
{
	TMouseButton button=noButton;
	if (wParam&MK_CONTROL)
		(int&)button|=ctrlButton;
	if (wParam&MK_LBUTTON)
		(int&)button|=leftButton;
	if (wParam&MK_RBUTTON)
		(int&)button|=rightButton;
	if (wParam&MK_MBUTTON)
		(int&)button|=middleButton;
	if (wParam&MK_SHIFT)
		(int&)button|=shiftButton;
	if (GetAsyncKeyState(VK_MENU)&(1<<15))
		(int&)button|=altButton;
 
	return button;
}

void CWinEventEmitter::processMessage (uint32 hWnd, uint32 msg, uint32 wParam, uint32 lParam, CEventServer *server)
{
	if (!server)
		server=&_InternalServer;
	switch (msg)
	{
	case WM_KEYDOWN:
		server->postEvent (new CEventKeyDown ((TKey)wParam, (((int) wParam)&(1<<30))==0, this));
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
	case WM_MOUSEMOVE:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_LBUTTONDBLCLK:
		{
			// MSWindows coordinates to NeL window coordinate
			float fX, fY;
			RECT client;
			GetClientRect ((HWND)hWnd, &client);
			fX=(float)LOWORD(lParam)/(float)(client.right-client.left);
			fY=1.f-(float)HIWORD(lParam)/(float)(client.bottom-client.top);

			// buttons
			TMouseButton button=getMouseButton (wParam);

			// Reswitch
			switch (msg)
			{
			case WM_MOUSEMOVE:
				server->postEvent (new CEventMouseMove (fX, fY, button, this));
				break;

			case WM_RBUTTONDOWN:
				server->postEvent (new CEventMouseDown (fX, fY, (TMouseButton)(rightButton|(button&~(leftButton|middleButton|rightButton))), this));
				break;
			case WM_MBUTTONDOWN:
				server->postEvent (new CEventMouseDown (fX, fY, (TMouseButton)(middleButton|(button&~(leftButton|middleButton|rightButton))), this));
				break;
			case WM_LBUTTONDOWN:
				server->postEvent (new CEventMouseDown (fX, fY, (TMouseButton)(leftButton|(button&~(leftButton|middleButton|rightButton))), this));
				break;

			case WM_RBUTTONUP:
				server->postEvent (new CEventMouseUp (fX, fY, (TMouseButton)(rightButton|(button&~(leftButton|middleButton|rightButton))), this));
				break;
			case WM_MBUTTONUP:
				server->postEvent (new CEventMouseUp (fX, fY, (TMouseButton)(middleButton|(button&~(leftButton|middleButton|rightButton))), this));
				break;
			case WM_LBUTTONUP:
				server->postEvent (new CEventMouseUp (fX, fY, (TMouseButton)(leftButton|(button&~(leftButton|middleButton|rightButton))), this));
				break;

			case WM_RBUTTONDBLCLK:
				server->postEvent (new CEventMouseDblClk (fX, fY, (TMouseButton)(rightButton|(button&~(leftButton|middleButton|rightButton))), this));
				break;
			case WM_MBUTTONDBLCLK:
				server->postEvent (new CEventMouseDblClk (fX, fY, (TMouseButton)(middleButton|(button&~(leftButton|middleButton|rightButton))), this));
				break;
			case WM_LBUTTONDBLCLK:
				server->postEvent (new CEventMouseDblClk (fX, fY, (TMouseButton)(leftButton|(button&~(leftButton|middleButton|rightButton))), this));
				break;
			}
			break;
		}
	case WM_DESTROY:
		server->postEvent (new CEventDestroyWindow (this));
		break;
	case WM_MOUSEWHEEL:
		{
			// MSWindows coordinates to NeL window coordinate
			float fX, fY;
			RECT client;
			GetClientRect ((HWND)hWnd, &client);
			fX=(float)LOWORD(lParam)/(float)(client.right-client.left);
			fY=1.f-(float)HIWORD(lParam)/(float)(client.bottom-client.top);

			// buttons
			TMouseButton button=getMouseButton (LOWORD(wParam));

			server->postEvent (new CEventMouseWheel (fX, fY, button, (short) HIWORD(wParam)>=0, this));
			break;
		}
	}
}

#endif // NL_OS_WINDOWS

} // NLMISC
