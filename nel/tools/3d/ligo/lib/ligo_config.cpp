/** \file ligo_config.cpp
 * Ligo config file 
 *
 * $Id: ligo_config.cpp,v 1.2 2001/11/14 15:16:00 corvazier Exp $
 */

/* Copyright, 2000, 2001 Nevrax Ltd.
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

#include "ligo_config.h"

#include <nel/misc/config_file.h>

using namespace NLMISC;

namespace NLLIGO
{

// ***************************************************************************

void CLigoConfig::read (const char *fileName)
{
	// The CF
	CConfigFile cf;

	// Load and parse the file
	cf.load (fileName);

	// Read the parameters
	CConfigFile::CVar &cell_size = cf.getVar ("cell_size");
	CellSize = cell_size.asFloat ();
	CConfigFile::CVar &snap = cf.getVar ("snap");
	Snap = snap.asFloat ();
	CConfigFile::CVar &snapShot = cf.getVar ("zone_snapeshot_res");
	ZoneSnapShotRes = (uint)snapShot.asInt ();
}

// ***************************************************************************

}