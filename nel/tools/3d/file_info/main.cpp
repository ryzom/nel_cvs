/** \file main.cpp
 * Display info on many NEL files. ig, zone etc...
 *
 * $Id: main.cpp,v 1.1 2002/05/14 12:49:38 berenguier Exp $
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



#include <conio.h>
#include "3d/scene_group.h"
#include "3d/zone.h"
#include "nel/misc/file.h"


using	namespace std;
using	namespace NLMISC;
using	namespace NL3D;

int		main(int argc, const char *argv[])
{
	if(argc<2)
	{
		puts("Usage: ig_info file.???");
		puts("    For now, only .ig and .zone are supported");
		puts("Press any key");
		_getch();
		return -1;
	}

	const char *fileName= argv[1];

	if(strstr(fileName, ".zone"))
	{
		// read the zone.
		CIFile	file(fileName);
		CZone	zone;
		file.serial(zone);

		// retreive info on Zone
		CZoneInfo	zoneInfo;
		zone.retrieve(zoneInfo);

		// display Info on the zone:
		printf("  Num Patchs: %d\n", zone.getNumPatchs() );
		printf("  Num PointLights: %d\n", zoneInfo.PointLights.size() );
	}
	else if(strstr(fileName, ".ig"))
	{
		// read the ig.
		CIFile	file(fileName);
		CInstanceGroup	ig;
		file.serial(ig);

		// display Info on the ig:
		printf("  Num Instances: %d\n", ig.getNumInstance() );
		printf("  Num PointLights: %d\n", ig.getPointLightList().size() );
	}
	else
	{
		puts("unsupported format");
	}

	puts("Press any key");
	_getch();
}