/** \file net_log.cpp
 * Class CNetLog (logger for network transfers)
 *
 * $Id: net_log.cpp,v 1.14 2002/11/13 12:06:25 lecroart Exp $
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






/**************************************************************************
********************* THIS CLASS IS DEPRECATED ****************************
**************************************************************************/








#include "stdnet.h"

#include "nel/net/net_log.h"
#include "nel/net/unitime.h"



using namespace std;
using namespace NLMISC;


namespace NLNET {


CNetLog NetLog;


/*
 * Constructor
 */
CNetLog::CNetLog() :
	CLog()
{
}


/*
 * Log an output transfer (send)
 */
void CNetLog::output( const char *srchost, uint8 msgnum,
					  const char *desthost, const char *msgname, uint32 msgsize )
{
/*OLD	char line [1024];
	smprintf( line, 1024, "@@%"NL_I64"d@%s@%hu@%s@%s@%s@%u@", (CUniTime::Sync?CUniTime::getUniTime():(TTime)0),
		srchost, (uint16)msgnum, _ProcessName.c_str(), desthost, msgname, msgsize );

	displayRawNL( line );
	*/
/*	displayRawNL( "@@%"NL_I64"d@%s@%hu@%s@%s@%s@%u@", (CUniTime::Sync?CUniTime::getUniTime():(TTime)0),
		srchost, (uint16)msgnum, _ProcessName.c_str(), desthost, msgname, msgsize );
*/
	displayRawNL( "@@0@%s@%hu@%s@%s@%s@%u@",
		srchost, (uint16)msgnum, (*_ProcessName).c_str(), desthost, msgname, msgsize );
}


/*
 * Log an input transfer (receive)
 */
void CNetLog::input( const char *srchost, uint8 msgnum, const char *desthost )
{
/*OLD	char line [1024];
	smprintf( line, 1024, "##%"NL_I64"d#%s#%hu#%s#%s#", (CUniTime::Sync?CUniTime::getUniTime():(TTime)0),
			  srchost, msgnum, _ProcessName.c_str(), desthost );
	displayRawNL( line );
*/
/*	displayRawNL( "##%"NL_I64"d#%s#%hu#%s#%s#", (CUniTime::Sync?CUniTime::getUniTime():(TTime)0),
		  srchost, msgnum, _ProcessName.c_str(), desthost );
*/
	displayRawNL( "##0#%s#%hu#%s#%s#", 
		  srchost, msgnum, (*_ProcessName).c_str(), desthost );
}


} // NLNET
