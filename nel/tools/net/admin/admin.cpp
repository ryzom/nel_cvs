/** \file admin.cpp
 * 
 *
 * $Id: admin.cpp,v 1.1 2001/05/02 12:36:31 lecroart Exp $
 *
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX D.T.C. SYSTEM.
 * NEVRAX D.T.C. SYSTEM is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * NEVRAX D.T.C. SYSTEM is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with NEVRAX D.T.C. SYSTEM; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include <conio.h>
#include <io.h>
#include <process.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <string>
#include <list>

#include "nel/misc/debug.h"
#include "nel/misc/config_file.h"
#include "nel/misc/thread.h"
#include "nel/misc/command.h"
#include "nel/misc/log.h"
#include "nel/misc/displayer.h"

#include "nel/net/service.h"
#include "nel/net/net_manager.h"

 
using namespace std;
using namespace NLMISC;
using namespace NLNET;


struct CService
{
	CService() : ASId(NextASId++) { }
	TSockId	ASSockId;
	uint32	ASId;
	uint32	AESId;
	uint32	SId;
	string	ShortName;
	string	LongName;

private:

	static	uint32 NextASId;
};

uint32 CService::NextASId = 0;

list<CService> Services;
typedef list<CService>::iterator sit;

sit find (uint32 asid, uint32 aesid, uint32 sid)
{
	sit it;
	for (it = Services.begin(); it != Services.end(); it++)
	{
		if ((*it).ASId == asid && (*it).AESId == aesid && (*it).SId == sid) break;
	}
	return it;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////// CONNECTION TO THE AS ////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void cbServiceList (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	uint32 size;
	msgin.serial(size);
	for (uint32 i = 0; i < size; i++)
	{
		CService c;
		c.ASSockId = from;
		msgin.serial(c.AESId);
		msgin.serial(c.SId);
		msgin.serial(c.ShortName);
		msgin.serial(c.LongName);
		Services.push_back(c);
	}
}


TCallbackItem ASCallbackArray[] =
{
	{ "SL", cbServiceList},
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// COMMANDS ////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

NLMISC_COMMAND (connect, "connect to the AS", "<ip>")
{
	if(args.size() != 1) return false;

	CNetManager::addClient (args[0], args[0]+":49995");
	CNetManager::addCallbackArray ("AS", ASCallbackArray, sizeof(ASCallbackArray)/sizeof(ASCallbackArray[0]));

	return true;
}

NLMISC_COMMAND (start, "start a service", "<>")
{
	if(args.size() != 1 && args.size() != 2) return false;

	CMessage msgout (CNetManager::getSIDA("AES"), "ESC");
	msgout.serial (const_cast<string &>(args[1]));

	uint8 background = 0;

	if (args.size() == 2)
	{
		if (args[1] == "&")
			uint8 background = 1;
		else
			return false;
	}

//	msgout.serial (background);
//	CNetManager::send ("AES", msgout);
	return true;
}

NLMISC_COMMAND (stop, "stop a service", "<> <>")
{
	if(args.size() != 2) return false;

	CMessage msgout (CNetManager::getSIDA("AES"), "SS");
	msgout.serial (const_cast<string &>(args[2]));
	CNetManager::send ("AES", msgout);
	return true;
}

NLMISC_COMMAND (list, "display list of service", "")
{
	if(args.size() != 0) return false;
	
	nlinfo("There are %d service(s)", Services.size());
	sit it;
	for (it = Services.begin(); it != Services.end(); it++)
	{
		nlinfo(" %s %d %d %d %s %s", (*it).ASSockId->asString().c_str(), (*it).ASId, (*it).AESId, (*it).SId, (*it).ShortName.c_str(), (*it).LongName.c_str());
	}
	nlinfo("End of the list");
	
	return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////// IMPLEMENTATION //////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void printLine(string line)
{
	printf("\r> %s \b", line.c_str());
}

CLog logstdout;
CStdDisplayer dispstdout;

int main (int argc, char **argv)
{
	logstdout.addDisplayer (&dispstdout);

	nlinfo("Admin client for NeL Shard administration ("__DATE__" "__TIME__")\n");

	string command;
	printLine(command);

	bool end = false;
	while (!end)
	{
		if (kbhit())
		{
			int c = getch();
			switch (c)
			{
			case  8: if (command.size()>0) command.resize (command.size()-1); printLine(command); break;
			case 27: end = true; break;
			case 13: printf("\n"); nlinfo("execute command: %s", command.c_str()); ICommand::execute(command, logstdout); command = ""; printLine(command); break;
			default: command += c; printLine(command); break;
			}
		}

		CNetManager::update();
	}

	return EXIT_SUCCESS;
}
