/* debug.h
 *
 * Copyright, 2000 Nevrax Ltd.
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

/*
 * $Id: debug.h,v 1.3 2000/10/06 10:27:36 lecroart Exp $
 *
 * <Replace this by a description of the file>
 */

#ifndef NL_DEBUG_H
#define NL_DEBUG_H

#include "nel/misc/log.h"


namespace NLMISC {

extern CLog ErrorLog;
extern CLog WarningLog;
extern CLog InfoLog;
extern CLog DebugLog;
extern CLog AssertLog;

#define nlerror NLMISC::ErrorLog.setParam( __LINE__, __FILE__ ); NLMISC::ErrorLog.display
#define nlwarning NLMISC::WarningLog.setParam( __LINE__, __FILE__ ); NLMISC::WarningLog.display
#define nlinfo NLMISC::InfoLog.setParam( __LINE__, __FILE__ ); NLMISC::InfoLog.display
#define nldebug NLMISC::DebugLog.setParam( __LINE__, __FILE__ ); NLMISC::DebugLog.display

//NLMISC::AssertLog.setParam( __LINE__, __FILE__ ); NLMISC::AssertLog.display

void NLMISC_InitDebug ();


#include <crtdbg.h>

#define nlassert(f)	\
			do \
			{ \
			if (!(f) && (_CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, NULL, NULL)==1)) \
				_CrtDbgBreak(); \
			} while (0) \

} // NLMISC


#endif // NL_DEBUG_H

/* End of debug.h */
