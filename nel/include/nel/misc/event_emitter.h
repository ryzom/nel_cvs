/** \file event_emitter.h
 * <File description>
 *
 * $Id: event_emitter.h,v 1.1 2000/11/09 16:16:53 coutelas Exp $
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

#ifndef NL_EVENT_EMITTER_H
#define NL_EVENT_EMITTER_H

#include "nel/misc/types_nl.h"


namespace NLMISC {

/*===================================================================*/

class CEventServer;

/**
 * CEventEmitter
 * Send events to the event server
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2000
 */
class CEventEmitter
{
public:
	/** 
	 * sends all events to server
	 * (should call CEventServer method postEvent() ) 
	 * \param server
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	virtual void submitEvents(CEventServer & server) = 0;
		
};

#ifdef NL_OS_WINDOWS
/**
 * CEventEmitterWin32
 * CEventEmitter Win32 implementation
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2000
 */
class CEventEmitterWin32
{
public:
	CEventEmitterWin32 (uint32 hWnd)
	{
		_HWnd=hWnd;
	}

	/** 
	 * sends all events to server
	 * (should call CEventServer method postEvent() ) 
	 * \param server
	 */	
	virtual void submitEvents(CEventServer & server);
private:
	uint32 _HWnd;
};
#endif // NL_OS_WINDOWS

} // NLMISC


#endif // NL_EVENT_EMITTER_H

/* End of event_emitter.h */
