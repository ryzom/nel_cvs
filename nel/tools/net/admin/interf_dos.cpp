/** \file interf_dos.cpp
 * 
 *
 * $Id: interf_dos.cpp,v 1.3 2002/03/25 10:19:13 lecroart Exp $
 *
 *
 */

/* Copyright, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX NeL Network Services.
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

#ifdef INTERF_DOS

#include "nel/misc/types_nl.h"

#include <string>
#include <conio.h>

#include "nel/misc/debug.h"
#include "nel/misc/log.h"
#include "nel/misc/displayer.h"
#include "nel/misc/command.h"

#include "nel/net/net_manager.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

static CLog logstdout;
static CStdDisplayer dispstdout;

static void printLine(string line)
{
	printf("\r> %s \b", line.c_str());
}

static string command;

void interfAddAS (CAdminService *as) {}
void interfAddAES (CAdminService *as, CAdminExecutorService *aes) {}
void interfAddService (CAdminExecutorService *aes, CService *s) {}

void initInterf ()
{
	logstdout.addDisplayer (&dispstdout);

	/// \todo ace: virer ca pour pas que ca connecte automatiquement
	ICommand::execute ("connect localhost", logstdout);
	
	printLine(command);
}

void runInterf ()
{
	bool end = false;
	while (!end)
	{
		while (kbhit())
		{
			int c = getch();
			switch (c)
			{
			case  8: if (command.size()>0) command.resize (command.size()-1); printLine(command); break;
			case 27: return; break;
			case 13: printf("\n"); nlinfo("execute command: %s", command.c_str()); ICommand::execute(command, logstdout); command = ""; printLine(command); break;
			default: command += c; printLine(command); break;
			}
		}

		CNetManager::update();
	}
}

#endif // INTERF_DOS
