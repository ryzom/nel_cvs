/** \file sysmem.cpp
 *
 * $Id: sysmem.cpp,v 1.1 2001/04/18 13:54:25 valignat Exp $
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


#include <string>
#include <fstream>

#include <nel/misc/debug.h>

#include "sysmem.h"

using namespace std;


static const char MEMORY_FILE[] = "/proc/meminfo";


CSysMemory::CSysMemory ( void ) {
}


CSysMemory::~CSysMemory ( void ) {
}


sint CSysMemory::getMemoryUsage ( void )
{
	char   buf[256];

	ifstream memstream( MEMORY_FILE );

	if ( ! memstream )
	{
		nlwarning( "Cannot open \"%s\".", MEMORY_FILE );
		return( -1 );
	}

	string junk;
	uint   memTotal;
	uint   memUsed;

	memstream.getline(buf, 256);

	if ( memstream.eof() )
	{
		return( -1 );
	}

	memstream >> junk >> memTotal >> memUsed;
	memstream.close();

	return( memUsed / (memTotal / 100) );
}


// End of sysmem.cpp
