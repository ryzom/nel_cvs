/** \file datas.cpp
 * 
 *
 * $Id: datas.cpp,v 1.3 2002/03/25 10:19:13 lecroart Exp $
 *
 * \warning the admin client works *only* on Windows because we use kbhit() and getch() functions that are not portable.
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
 
#include "nel/misc/types_nl.h"

#include "nel/net/buf_sock.h"
#include "nel/misc/config_file.h"

#include "datas.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

uint32 CAdminService::NextId = 1;

CConfigFile ConfigFile;

	
TAdminServices AdminServices;


ASIT findAdminService (uint32 asid, bool asrt)
{
	ASIT asit;
	for (asit = AdminServices.begin(); asit != AdminServices.end(); asit++)
		if ((*asit).Id == asid)
			break;
	
	if (asrt)
		nlassert (asit != AdminServices.end());
	return asit;
}

void displayServices ()
{
	for (ASIT asit = AdminServices.begin(); asit != AdminServices.end(); asit++)
	{
		nlinfo ("AS %d %s", (*asit).Id, (*asit).SockId->asString().c_str());
		for (AESIT aesit = (*asit).AdminExecutorServices.begin(); aesit != (*asit).AdminExecutorServices.end(); aesit++)
		{
			nlinfo (" AES %d", (*aesit).Id);
			for (SIT sit = (*aesit).Services.begin(); sit != (*aesit).Services.end(); sit++)
			{
				nlinfo ("  > %d:%d:%d %s %s", (*asit).Id, (*aesit).Id, (*sit).Id, (*sit).ShortName.c_str(), (*sit).LongName.c_str());
			}
		}
	}
}
