/** \file zone_lighter.cpp
 * zone_lighter.cpp : Very simple zone lighter
 *
 * $Id: zone_lighter.cpp,v 1.2 2001/01/15 15:45:54 corvazier Exp $
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

// Eval the normal of the border of a patch for somewhere on a edge
CVector getEdgeNormal (const CBezierPatch& patch, uint edge, float whereOnEdge)
{
	switch (edge)
	{
	case 0:
		return patch.evalNormal (0.f, whereOnEdge);
	case 1:
		return patch.evalNormal (whereOnEdge, 1.f);
	case 2:
		return patch.evalNormal (1.f, 1.f-whereOnEdge);
	case 3:
		return patch.evalNormal (1.f-whereOnEdge, 0.f);
	default:
		nlassert (0);		// no!
	}
	return CVector::Null;
}

int main(int argc, char* argv[])
{
	// Good number of args ?
	if (argc<7)
	{
		// Help message
		printf ("zone_lighter [zonein.zone] [zoneout.zone] [SunDirectionX] [SunDirectionY] [SunDirectionZ] [NormalThreshold (deg)]\n");
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

			// The normal threshold
			float normalThreshold=(float)cos((float)atof (argv[6])*Pi/180.f);

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
						float fS=((float)s)/(float)(numS-1);
						float fT=((float)t)/(float)(numT-1);

						// Get the position on the patch
						CVector pos=patchs[patch].Patch.eval (fS, fT);

						// Get the normal on the patch
						CVector normal=patchs[patch].Patch.evalNormal (fS, fT);

						// Normal accumulator
						CVector normalAccu=normal;

						// Border values
						const float fBorder[4]={1.f-fT, 1.f-fS, fT, fS};

						// Border ?
						int border;
						for (border=0; border<4; border++)
						{
							// This normal is on this border ?
							if (((border==0)&&(s==0))||
								((border==1)&&(t==(numT-1)))||
								((border==2)&&(s==(numS-1)))||
								((border==3)&&(t==0)))
							{
								// If no bind and the neighboord patch is in the same zone
								if ((patchs[patch].BindEdges[border].NPatchs==1)&&(patchs[patch].BindEdges[border].ZoneId==zone.getZoneId()))
								{
									// Get the normal of the neighborhood
									CVector normalN=getEdgeNormal (patchs[patchs[patch].BindEdges[border].Next[0]].Patch, 
										patchs[patch].BindEdges[border].Edge[0], fBorder[border]);

									// Check normal threshold
									if (normalN*normal>normalThreshold)
									{
										// add this normal
										normalAccu+=normalN;
									}
								}
							}
						}

						// Normalize
						normalAccu.normalize();

						// Simple lighting
						sint nLum=(sint)(255.f*(-lightDir*normalAccu));
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
