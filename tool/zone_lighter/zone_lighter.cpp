/** \file zone_lighter.cpp
 * zone_lighter.cpp : Very simple zone lighter
 *
 * $Id: zone_lighter.cpp,v 1.1 2001/01/11 16:02:14 corvazier Exp $
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

#include "nel/misc/stream.h"
#include "nel/misc/file.h"
#include "nel/misc/vector.h"
#include "nel/3d/zone.h"

#include <stdio.h>

using namespace NLMISC;
using namespace NL3D;

int main(int argc, char* argv[])
{
	// Good number of args ?
	if (argc<6)
	{
		// Help message
		printf ("zone_lighter [zonein.zone] [zoneout.zone] [SunDirectionX] [SunDirectionY] [SunDirectionZ]\n");
	}
	else
	{
		// Ok, read the zone
		CIFile inputFile;

		// Open it for reading
		if (inputFile.open (argv[1]))
		{
			// The sun position
			CVector lightDir ((float)atof (argv[3]), (float)atof (argv[4]), (float)atof (argv[5]));
			lightDir.normalize();

			// Load the zone
			try
			{
				// The zone
				CZone zone;

				// Load
				zone.serial (inputFile);

				// Retrieve patch info
				std::vector<CPatchInfo> patchs;
				std::vector<CBorderVertex> borderVertices;
				zone.retrieve(patchs, borderVertices);

				// Light each patch
				for (uint patch=0; patch<patchs.size(); patch++)
				{
					// Num of tiles
					uint numS=patchs[patch].OrderS+1;
					uint numT=patchs[patch].OrderT+1;

					// For all tiles
					for (uint t=0; t<numT; t++)
					for (uint s=0; s<numS; s++)
					{
						// offset on the patch. (center off the lumel)
						float fS=((float)s)/(float)numS;
						float fT=((float)t)/(float)numT;

						// Get the position on the patch
						CVector pos=patchs[patch].Patch.eval (fS, fT);

						// Get the normal on the patch
						CVector normal=patchs[patch].Patch.evalNormal (fS, fT);

						// Simple lighting
						sint nLum=(sint)(255.f*((lightDir*normal)+1.f)/2.f);
						clamp (nLum, 0, 255);

						// Assign
						patchs[patch].TileColors[t*numS+s].Shade=nLum;
					}
				}

				// Build the zone
				zone.build (zone.getZoneId(), patchs, borderVertices);

				// Save the zone
				COFile outputFile;

				// Open it
				if (outputFile.open (argv[2]))
				{
					// Save the new zone
					try
					{
						// Save it
						zone.serial (outputFile);
					}
					catch (Exception& except)
					{
						// Error message
						fprintf (stderr, "Error writing %s: %s\n", argv[2], except);
					}
				}
				else
				{
					// Error can't open the file
					fprintf (stderr, "Can't open %s for writing\n", argv[1]);
				}
			}
			catch (Exception& except)
			{
				// Error message
				fprintf (stderr, "Error reading %s: %s\n", argv[1], except);
			}
		}
		else
		{
			// Error can't open the file
			fprintf (stderr, "Can't open %s for reading\n", argv[1]);
		}

	}
	
	// exit.
	return 0;
}
