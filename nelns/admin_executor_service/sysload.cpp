/** \file sysload.cpp
 *
 * $Id: sysload.cpp,v 1.1 2001/04/18 13:54:25 valignat Exp $
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


#include <fstream>

#include <nel/misc/debug.h>

#include "sysload.h"

using namespace std;


static const char LOAD_FILE[] = "/proc/loadavg";


CSysLoad::CSysLoad ( void ) {
}


CSysLoad::~CSysLoad ( void ) {
}


double CSysLoad::getLoadInfo ( void )
{
	double   load;
	ifstream loadstream( LOAD_FILE );

	if ( ! loadstream )
	{
		nlwarning( "Cannot open \"%s\".", LOAD_FILE );
		return( -1 );
	}

	loadstream >> load;

	loadstream.close();

	return( load );
}


// End of sysload.cpp
