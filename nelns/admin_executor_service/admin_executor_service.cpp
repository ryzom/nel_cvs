/** \file admin_executor_service.cpp
 * Admin Executor Service (AES)
 *
 * $Id: admin_executor_service.cpp,v 1.1 2001/04/18 13:54:25 valignat Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NeL Network Services
 * NEVRAX NeL Network Services is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * NEVRAX NeL Network Services is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NeL Network Services; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include <nel/misc/debug.h>
#include <nel/misc/log.h>
#include <nel/misc/common.h>
#include <nel/net/service.h>
#include <nel/net/net_displayer.h>

#include "sysload.h"
#include "sysmem.h"
#include "sysswap.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;


static const char UNKNOW_VALUE[] = "<Unknown>";


CLog StatLog(CLog::LOG_STAT);


/// Log Stat data (STT)
void sendData (const char *str, TSenderId from)
{
	StatLog.displayNL( str );
}


/// Log the last minute average load
void cbLoad ( CMessage& message, TSenderId from )
{
	CSysLoad sysload = CSysLoad();
	double   load    = sysload.getLoadInfo();
	string   answer( "LOAD " );

	if ( load < 0 )
	{
		answer.append( UNKNOW_VALUE );
	}
	else
	{
		char str[6];
		smprintf( str, 6, "%.2f", load );
		answer.append( string(str) );
	}
	
	sendData( answer.c_str(), from );
}


/// Log the total memory size and the used memory size (in B)
void cbMemory ( CMessage& message, TSenderId from )
{
	CSysMemory sysmem   = CSysMemory();
	sint       memUsage = sysmem.getMemoryUsage();
	string     answer( "MEM " );

	if ( memUsage < 0 )
	{
		answer.append( UNKNOW_VALUE );
	}
	else
	{
		char str[3];
		smprintf( str, 3, "%d", memUsage );
		answer.append( string(str) );
	}
	
	sendData( answer.c_str(), from );
}


/// Log the swap size and the used swap space (in kB)
void cbSwap ( CMessage& message, TSenderId from )
{
	CSysSwap sysswap   = CSysSwap();
	sint     swapUsage = sysswap.getSwapUsage();
	string   answer( "SWAP " );

	if ( swapUsage < 0 )
	{
		answer.append( UNKNOW_VALUE );
	}
	else
	{
		char str[3];
		smprintf( str, 3, "%d", swapUsage );
		answer.append( string(str) );
	}
	
	sendData( answer.c_str(), from );
}


// Log all the server informations.
void cbSystem ( CMessage& message, TSenderId from )
{
	cbLoad   ( message, from );
	cbMemory ( message, from );
	cbSwap   ( message, from );
}


/**
 * Callback Array
 * Message types:
 *	LOAD:   log Load information
 *	MEMORY: log Memory information
 *	SWAP:   log Swap information
 *	SYSTEM: log the Load, Memory, and Swap informations
 */
TCallbackItem CallbackArray[] =
{
	{ "LOAD",   cbLoad },
	{ "MEMORY", cbMemory },
	{ "SWAP",   cbSwap },

	{ "SYSTEM", cbSystem }
};



/** Admin Executor Service (AES).
 * Log informations (load, memory usage, etc ...) about the server it's
 * running on.
 * These informations are used by the Admin Service (AS) and the Naming
 * Service (NS) to watch the differents servers of the shard.
 */
class CAdminExecutorService : public NLNET::IService
{
public:

	/// Initializes the service
	void init ()
	{
		// Connect to the Log Service
		StatLog.addDisplayer( new NLMISC::CStdDisplayer() );

		NLNET::CNetDisplayer *nd = new NLNET::CNetDisplayer;

		if ( nd->connected() )
		{
			StatLog.addDisplayer( nd );
		}
		else
		{
			nlerror( "Coudn't connect to the Log Service." );
		}
	}

};


NLNET_SERVICE_MAIN( CAdminExecutorService, "AES", 50009 );

// End of admin_executor_service.cpp
