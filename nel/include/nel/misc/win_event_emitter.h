/** \file win_event_emitter.h
 * <File description>
 *
 * $Id: win_event_emitter.h,v 1.2 2001/02/23 09:08:46 corvazier Exp $
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

#ifndef NL_WIN_EVENT_EMITTER_H
#define NL_WIN_EVENT_EMITTER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/event_emitter.h"

#ifdef NL_OS_WINDOWS

namespace NLMISC {

/**
 * CWinEventEmitter
 * CEventEmitter Windows implementation
 */
class CWinEventEmitter : public IEventEmitter
{
public:
	CWinEventEmitter ()
	{
		_HWnd=NULL;
		resetButtonFlagState ();
	}
	void setHWnd (uint32 hWnd)
	{
		_HWnd=hWnd;
		resetButtonFlagState ();
	}

	/** 
	 * sends all events to server
	 * (should call CEventServer method postEvent() ) 
	 * \param server
	 */	
	virtual void submitEvents(CEventServer & server);
private:
	// Reset button flag state
	void resetButtonFlagState ();

	// Private internal server message
	class CWinEventServer : CEventServer
	{
		friend class CWinEventEmitter;
	public:
		void setServer (CEventServer *server)
		{
			_Server=server;
		}
	private:
		virtual bool pumpEvent(CEvent* event)
		{
			CEventServer::pumpEvent(event);
			_Server->postEvent (event);
			return false;
		}
	private:
		CEventServer *_Server;
	};

public:
	/** Process a win32 message.
	  */
	void processMessage (uint32 hWnd, uint32 msg, uint32 wParam, uint32 lParam, CEventServer *server=NULL);
private:
	CWinEventServer		_InternalServer;
	uint32				_HWnd;
	bool				_CtrlButton;
	bool				_ShiftButton;
	bool				_AltButton;
};

} // NLMISC

#endif // NL_OS_WINDOWS

#endif // NL_WIN_EVENT_EMITTER_H

/* End of win_event_emitter.h */
