/** \file sysswap.cpp
 *
 * $Id: sysswap.cpp,v 1.1 2001/04/18 13:54:25 valignat Exp $
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

#include "sysswap.h"

using namespace std;


static const char SWAP_FILE[] = "/proc/meminfo";


CSysSwap::CSysSwap ( void ) {
}


CSysSwap::~CSysSwap ( void ) {
}


sint CSysSwap::getSwapUsage ( void )
{
	char   buf[256];

	ifstream swapstream( SWAP_FILE );

	if ( ! swapstream )
	{
		nlwarning( "Cannot open \"%s\".", SWAP_FILE );
		return( -1 );
	}

	string junk;
	uint   swapTotal;
	uint   swapUsed;

	for (int line = 1; line <= 2 && !swapstream.eof(); line++ )
	{
		swapstream.getline(buf, 256);
	}

	if ( swapstream.eof() )
	{
		return( -1 );
	}

	swapstream >> junk >> swapTotal >> swapUsed;
	swapstream.close();

	return( swapUsed / (swapTotal / 100) );
}


// End of sysswap.cpp
