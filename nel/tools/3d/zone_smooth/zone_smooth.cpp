/** \file zone_smooth.cpp
 * zone_smooth.cpp : Very simple zone smoother after light
 *
 * $Id: zone_smooth.cpp,v 1.1 2001/02/15 09:02:59 corvazier Exp $
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
#include "nel/3d/landscape.h"
#include "nel/3d/triangle.h"
#include "../zone_welder/zone_utility.h"	// load a header file from zone_welder project

#include <stdio.h>
#include <float.h>

using namespace std;
using namespace NLMISC;
using namespace NL3D;

typedef std::map<uint, std::vector<CPatchInfo> > mapPatchInfo;

void loadAdjacentZones (const char* sZoneName, CLandscape& landscape, mapPatchInfo& patchInfo)
{
	// Get the dir
	string dir=getDir (sZoneName);
	string ext=getExt (sZoneName);
	string name=getName (sZoneName);

	// Get zone coordonnate hardcoded in the name
	uint16 x;
	uint16 y;
	if (getZoneCoordByName(name.c_str(), x, y))
	{
		// ok, it's a valid name, get the neighborhood zones
		std::vector<std::string> names;
		getAdjacentZonesName(name, names);

		// for all name in the array
		std::vector<std::string>::iterator ite=names.begin();
		while (ite!=names.end())
		{
			// try to load it
			CIFile file;
			if (file.open (dir+(*ite)+ext))
			{
				// The zone
				CZone zone;

				try
				{
					// Load
					zone.serial (file);

					// Retrieve poly infos
					std::vector<CPatchInfo> patchs;
					std::vector<CBorderVertex> borderVertices;
					zone.retrieve(patchs, borderVertices);

					// Add to landscape
					landscape.addZone (zone);

					// Add to the map
					patchInfo.insert (mapPatchInfo::value_type (zone.getZoneId(), patchs));
				}
				catch (Exception& except)
				{
					// Error message
					fprintf (stderr, "Error reading %s: %s\n", ite->c_str(), except.what());
				}
			}
			ite++;
		}
	}
}

struct lumelDesc
{
	uint Zone;
	uint Patch;
	sint S;
	sint T;
};

bool getNeigborhood (uint zone, uint patch, uint where, uint edge, uint order, const mapPatchInfo& info, uint &zoneDst, uint &patchDst, 
					 sint &sDst, sint &tDst)
{
	// Find the zone in the map
	mapPatchInfo::const_iterator ite=info.find (zone);
	nlassert (ite!=info.end());

	// Get patches
	const std::vector<CPatchInfo>& patchs=ite->second;

	// Zone id destination
	zoneDst=patchs[patch].BindEdges[edge].ZoneId;

	// Get patch voisins neigbor cout
	uint patchCount=patchs[patch].BindEdges[edge].NPatchs;
	if (patchCount==0)
		return false;

	// For patchCount=5
	uint mulFactor=1;
	uint offset=0;
	bool smallBinded=false;

	if (patchCount==5)
	{
		// In fact only one patch
		patchCount=1;
		smallBinded=true;
	}

	// Patch dst
	uint next=(where*patchCount)/order;
	if (where==order)
		next--;
	nlassert (next<patchCount);

	// Get next patch
	patchDst=patchs[patch].BindEdges[edge].Next[next];

	// Get next whereDst
	volatile uint edgeDst=patchs[patch].BindEdges[edge].Edge[next];

	// Get the destination zone
	if (zone!=zoneDst)
	{
		ite=info.find (zoneDst);
		if (ite==info.end())
			return false;
	}
	const std::vector<CPatchInfo>& patchs2=ite->second;

	// Look in the neigborhood patch for offset and mul factor
	if (smallBinded)
	{
		// Check the mulfactor
		uint numPatchsNeigh=patchs2[patchDst].BindEdges[edgeDst].NPatchs;
		switch (numPatchsNeigh)
		{
		case 2:
			mulFactor=2;
			break;
		case 4:
			mulFactor=4;
			break;
		default:
			nlassert (0);		// impossible, neigborhood is a small binded patch on this edge. Must be 2 or 4.
		}

		// Check the binded zone is the good one
		nlassert (patchs2[patchDst].BindEdges[edgeDst].ZoneId==zone);

		// Ok now calc the offset
		for (uint j=0; j<numPatchsNeigh; j++)
			if (patchs2[patchDst].BindEdges[edgeDst].Next[j]==patch)
				break;

		// Check we found our patch. Must be in the neigborhood list
		nlassert (j!=numPatchsNeigh);

		// Compute the offset
		offset=(numPatchsNeigh-j-1)*(order-1)/numPatchsNeigh;
	}

	// Get where
	uint wantedDstOrder=mulFactor*((order-1)>>NL_LUMEL_BY_TILE_SHIFT)/patchCount;

	// What is the new subTile offset
	//uint whereTile=where; //offset+where%(wantedDstOrder<<NL_LUMEL_BY_TILE_SHIFT);
	volatile uint whereTile=offset+where%(wantedDstOrder<<NL_LUMEL_BY_TILE_SHIFT);

	switch (edgeDst)
	{
	case 0:
		if (patchs2[patchDst].OrderT!=wantedDstOrder)
			return false;
		sDst=0;
		tDst=patchs2[patchDst].OrderT*NL_LUMEL_BY_TILE-whereTile;
		break;
	case 1:
		if (patchs2[patchDst].OrderS!=wantedDstOrder)
			return false;
		sDst=patchs2[patchDst].OrderS*NL_LUMEL_BY_TILE-whereTile;
		tDst=patchs2[patchDst].OrderT*NL_LUMEL_BY_TILE;
		break;
	case 2:
		if (patchs2[patchDst].OrderT!=wantedDstOrder)
			return false;
		sDst=patchs2[patchDst].OrderS*NL_LUMEL_BY_TILE;
		tDst=whereTile;
		break;
	case 3:
		if (patchs2[patchDst].OrderS!=wantedDstOrder)
			return false;
		sDst=whereTile;
		tDst=0;
		break;
	default:
		nlassert (0);		// no!
	}
	volatile sint ss=sDst;
	volatile sint tt=tDst;
	clamp (sDst, 1, (patchs2[patchDst].OrderS<<NL_LUMEL_BY_TILE_SHIFT)-1);
	clamp (tDst, 1, (patchs2[patchDst].OrderT<<NL_LUMEL_BY_TILE_SHIFT)-1);

	nlassert (sDst>=0);
	nlassert (tDst>=0);
	nlassert (sDst<(patchs2[patchDst].OrderS<<NL_LUMEL_BY_TILE_SHIFT)+1);
	nlassert (tDst<(patchs2[patchDst].OrderT<<NL_LUMEL_BY_TILE_SHIFT)+1);

	return true;
}

bool getLumel (lumelDesc& dest, sint deltaS, sint deltaT, const mapPatchInfo& info, const CLandscape& landscape)
{
	// check args
	nlassert (((deltaS!=0)&&(deltaT==0))||((deltaT!=0)&&(deltaS==0)));

	// Copy descriptor
	dest.S+=deltaS;
	dest.T+=deltaT;

	// Get the zone pointer
	const CZone *pZone=landscape.getZone ((sint)dest.Zone);
	nlassert (pZone);

	// Get source patch
	const CPatch* pPatch=pZone->getPatch ((sint)dest.Patch);

	// Clip coord
	if (dest.S==-1)
	{
		nlassert (dest.T<(pPatch->getOrderT()<<NL_LUMEL_BY_TILE_SHIFT)+1);
		if (!getNeigborhood (dest.Zone, dest.Patch, dest.T, 0, pPatch->getOrderT()*NL_LUMEL_BY_TILE+1, info, dest.Zone, dest.Patch, 
			dest.S, dest.T))
			return false;
#ifdef NL_DEBUG
		pZone=landscape.getZone ((sint)dest.Zone);
		pPatch=pZone->getPatch ((sint)dest.Patch);
		nlassert (dest.S<(pPatch->getOrderS()<<NL_LUMEL_BY_TILE_SHIFT)+1);
		nlassert (dest.T<(pPatch->getOrderT()<<NL_LUMEL_BY_TILE_SHIFT)+1);
#endif // NL_DEBUG
	}
	if (dest.S==(pPatch->getOrderS()*NL_LUMEL_BY_TILE+1))
	{
		nlassert (dest.T<(pPatch->getOrderT()<<NL_LUMEL_BY_TILE_SHIFT)+1);
		if (!getNeigborhood (dest.Zone, dest.Patch, pPatch->getOrderT()*NL_LUMEL_BY_TILE-dest.T, 2, pPatch->getOrderT()*NL_LUMEL_BY_TILE+1, 
			info, dest.Zone, dest.Patch, dest.S, dest.T))
			return false;
#ifdef NL_DEBUG
		pZone=landscape.getZone ((sint)dest.Zone);
		pPatch=pZone->getPatch ((sint)dest.Patch);
		nlassert (dest.S<(pPatch->getOrderS()<<NL_LUMEL_BY_TILE_SHIFT)+1);
		nlassert (dest.T<(pPatch->getOrderT()<<NL_LUMEL_BY_TILE_SHIFT)+1);
#endif // NL_DEBUG
	}
	if (dest.T==-1)
	{
		nlassert (dest.S<(pPatch->getOrderS()<<NL_LUMEL_BY_TILE_SHIFT)+1);
		if (!getNeigborhood (dest.Zone, dest.Patch, pPatch->getOrderS()*NL_LUMEL_BY_TILE-dest.S, 3, pPatch->getOrderS()*NL_LUMEL_BY_TILE+1, 
			info, dest.Zone, dest.Patch, dest.S, dest.T))
			return false;
#ifdef NL_DEBUG
		pZone=landscape.getZone ((sint)dest.Zone);
		pPatch=pZone->getPatch ((sint)dest.Patch);
		nlassert (dest.S<(pPatch->getOrderS()<<NL_LUMEL_BY_TILE_SHIFT)+1);
		nlassert (dest.T<(pPatch->getOrderT()<<NL_LUMEL_BY_TILE_SHIFT)+1);
#endif // NL_DEBUG
	}
	if (dest.T==(pPatch->getOrderT()*NL_LUMEL_BY_TILE+1))
	{
		nlassert (dest.S<(pPatch->getOrderS()<<NL_LUMEL_BY_TILE_SHIFT)+1);
		if (!getNeigborhood (dest.Zone, dest.Patch, dest.S, 1, pPatch->getOrderS()*NL_LUMEL_BY_TILE+1, info, dest.Zone, dest.Patch, 
			dest.S, dest.T))
			return false;
#ifdef NL_DEBUG
		pZone=landscape.getZone ((sint)dest.Zone);
		pPatch=pZone->getPatch ((sint)dest.Patch);
		nlassert (dest.S<(pPatch->getOrderS()<<NL_LUMEL_BY_TILE_SHIFT)+1);
		nlassert (dest.T<(pPatch->getOrderT()<<NL_LUMEL_BY_TILE_SHIFT)+1);
#endif // NL_DEBUG
	}
	return true;
}

uint8 getShading (const lumelDesc& source, const CLandscape& landscape)
{
	// Get the zone pointer
	const CZone *pZone=landscape.getZone ((sint)source.Zone);
	nlassert (pZone);

	// Get the patch
	CPatch *pPatch=(CPatch*)pZone->getPatch ((sint)source.Patch);

	// Compressed ?
	if (pPatch->UncompressedLumels.size()==0)
	{
		pPatch->UncompressedLumels.resize ((pPatch->getOrderS()*NL_LUMEL_BY_TILE+1)*(pPatch->getOrderT()*NL_LUMEL_BY_TILE+1));
		pPatch->unpackShadowMap (&pPatch->UncompressedLumels[0]);
	}

	// Get the luminosity
	return pPatch->UncompressedLumels[source.T*(pPatch->getOrderS()*NL_LUMEL_BY_TILE+1)+source.S].Shaded;
}

uint8 getBlurPixel (const lumelDesc& source, const CLandscape& landscape, const mapPatchInfo& patchInfo, uint blurSize)
{
	if (blurSize==0)
	{
		return getShading (source, landscape);
	}
	else
	{
		// Accu shading
		uint shadingOriginal=getShading (source, landscape);
		uint shading=shadingOriginal;

		// Second desc
		lumelDesc descDst=source;
		if (getLumel (descDst, +1, 0, patchInfo, landscape))
		{
			shading+=getShading (descDst, landscape)/2;
			lumelDesc descDst2=descDst;
			
			if (getLumel (descDst2, 0, +1, patchInfo, landscape))
			{
				shading+=getShading (descDst2, landscape)/4;
			}
			else
				shading+=shadingOriginal/4;
			descDst2=descDst;
			if (getLumel (descDst2, 0, -1, patchInfo, landscape))
			{
				shading+=getShading (descDst2, landscape)/4;
			}
			else
				shading+=shadingOriginal/4;
		}
		else
			shading+=shadingOriginal/2;
		// Third desc
		descDst=source;
		if (getLumel (descDst, 0, +1, patchInfo, landscape))
		{
			shading+=getShading (descDst, landscape)/2;
		}
		else
			shading+=shadingOriginal/2;

		// Fourth desc
		descDst=source;
		if (getLumel (descDst, -1, 0, patchInfo, landscape))
		{
			shading+=getShading (descDst, landscape)/2;

			lumelDesc descDst2=descDst;
			if (getLumel (descDst2, 0, +1, patchInfo, landscape))
			{
				shading+=getShading (descDst2, landscape)/4;
			}
			else
				shading+=shadingOriginal/4;
			descDst2=descDst;
			if (getLumel (descDst2, 0, -1, patchInfo, landscape))
			{
				shading+=getShading (descDst2, landscape)/4;
			}
			else
				shading+=shadingOriginal/4;
		}
		else
			shading+=shadingOriginal/2;

		// Fifth desc
		descDst=source;
		if (getLumel (descDst, 0, -1, patchInfo, landscape))
		{
			shading+=getShading (descDst, landscape)/2;
		}
		else
			shading+=shadingOriginal/2;

		// Smooth
		shading/=4;

		return shading;
	}
}

int main(int argc, char* argv[])
{
	// return value
	uint shadowMem=0;

	// Good number of args ?
	if (argc<4)
	{
		// Help message
		printf ("zone_smooth [zonein.zone] [zoneout.zone] [blur_size 0 or 1]\n");
	}
	else
	{
		// Blur size
		uint blurSize=atoi (argv[3]);
		if ((blurSize!=0)&&(blurSize!=1))
		{
			// Help message
			printf ("zone_smooth [zonein.zone] [zoneout.zone] [blur_size 0 or 1]\n");
		}
		else
		{
			// Ok, read the zone
			CIFile inputFile;

			// Open it for reading
			if (inputFile.open (argv[1]))
			{
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
					mapPatchInfo patchInfo;
					patchInfo.insert (mapPatchInfo::value_type (zone.getZoneId(), patchs));

					// Pixels count
					uint compressed=0;
					uint uncompressed=0;

					// A landscape
					CLandscape landscape;

					// Add the zone
					landscape.addZone (zone);

					// Load adjacent zones
					loadAdjacentZones (argv[1], landscape, patchInfo);

					for (uint patch=0; patch<patchs.size(); patch++)
					{
						// Lumel desc
						lumelDesc desc;
						desc.Patch=patch;
						desc.Zone=zone.getZoneId();

						// Patch
						CPatchInfo& pa=patchs[patch];

						// Num of lumels
						uint numS=pa.OrderS*NL_LUMEL_BY_TILE+1;
						uint numT=pa.OrderT*NL_LUMEL_BY_TILE+1;
						uint numTileS=pa.OrderS+1;
						uint numTileT=pa.OrderT+1;

						// Bool
						std::vector<uint8> vectorShade(numS*numT);

						// For all lumels
						uint s, t;
						for (t=0; t<numT; t++)
						for (s=0; s<numS; s++)
						{
							// Desc
							desc.S=s;
							desc.T=t;

							// Accu shading
							uint shadingOriginal=getBlurPixel (desc, landscape, patchInfo, blurSize);
							uint shading=shadingOriginal;

							// Second desc
							lumelDesc descDst=desc;
							if (getLumel (descDst, +1, 0, patchInfo, landscape))
							{
								shading+=getBlurPixel (descDst, landscape, patchInfo, blurSize)/2;
								lumelDesc descDst2=descDst;
								
								if (getLumel (descDst2, 0, +1, patchInfo, landscape))
								{
									shading+=getBlurPixel (descDst2, landscape, patchInfo, blurSize)/4;
								}
								else
									shading+=shadingOriginal/4;
								descDst2=descDst;
								if (getLumel (descDst2, 0, -1, patchInfo, landscape))
								{
									shading+=getBlurPixel (descDst2, landscape, patchInfo, blurSize)/4;
								}
								else
									shading+=shadingOriginal/4;
							}
							else
								shading+=shadingOriginal/2;
							// Third desc
							descDst=desc;
							if (getLumel (descDst, 0, +1, patchInfo, landscape))
							{
								shading+=getBlurPixel (descDst, landscape, patchInfo, blurSize)/2;
							}
							else
								shading+=shadingOriginal/2;

							// Fourth desc
							descDst=desc;
							if (getLumel (descDst, -1, 0, patchInfo, landscape))
							{
								shading+=getShading (descDst, landscape)/2;

								lumelDesc descDst2=descDst;
								if (getLumel (descDst2, 0, +1, patchInfo, landscape))
								{
									shading+=getBlurPixel (descDst2, landscape, patchInfo, blurSize)/4;
								}
								else
									shading+=shadingOriginal/4;
								descDst2=descDst;
								if (getLumel (descDst2, 0, -1, patchInfo, landscape))
								{
									shading+=getBlurPixel (descDst2, landscape, patchInfo, blurSize)/4;
								}
								else
									shading+=shadingOriginal/4;
							}
							else
								shading+=shadingOriginal/2;

							// Fifth desc
							descDst=desc;
							if (getLumel (descDst, 0, -1, patchInfo, landscape))
							{
								shading+=getBlurPixel (descDst, landscape, patchInfo, blurSize)/2;
							}
							else
								shading+=shadingOriginal/2;

							// Smooth
							shading/=4;

							// Store shading
							vectorShade[t*numS+s]=(uint8)shading;
						}

						// Build shading color. One color by tile corner.
						for (t=0; t<numTileT; t++)
						for (s=0; s<numTileS; s++)
						{
							// Copy vertex shading
							uint8 shade=vectorShade[(t<<2)*numS+(s<<2)];
							pa.TileColors[t*numTileS+s].Shade=shade;
						}

						// Build shading info: for each lumel, check if it is shadowed. If it is, build it with it's
						// interpolated value and shading value. If not, build it with only it's shading value.
						for (t=0; t<numT; t++)
						for (s=0; s<numS; s++)
						{
							// assume NL_LUMEL_BY_TILE == 4
							nlassert (NL_LUMEL_BY_TILE==4);

							// Coordinates in tiles
							uint tTile=t>>2;
							uint sTile=s>>2;

							// Get the interpolated value
							uint topLeft=(uint)pa.TileColors[tTile*numTileS+sTile].Shade;
							uint topRight=0xff;
							uint bottomLeft=0xff;
							uint bottomRight=0xff;
							if (s<numS-1)
								topRight=(uint)pa.TileColors[tTile*numTileS+sTile+1].Shade;
							if (t<numT-1)
								bottomLeft=(uint)pa.TileColors[(tTile+1)*numTileS+sTile].Shade;
							if ((t<numT-1)&&(s<numS-1))
								bottomRight=(uint)pa.TileColors[(tTile+1)*numTileS+sTile+1].Shade;
							uint8 interpolated=
								(uint8)
								(
									(
										(topLeft*(4-(s&0x3)) + topRight*(s&0x3))*(4-(t&0x3)) +
										(bottomLeft*(4-(s&0x3)) + bottomRight*(s&0x3))*(t&0x3)
									)>>4
								);

							// Vertex shadowed ?
							// Create with interpolated value and shading value
							pa.Lumels[t*numS+s].createUncompressed (interpolated, vectorShade[t*numS+s]);

							// Pixel counter
							if (pa.Lumels[t*numS+s].isShadowed ())
								uncompressed++;
							else
								compressed++;
						}
					}

					// Build the zone
					zone.build (zone.getZoneId(), patchs, borderVertices);

					// Count memory
					for (patch=0; patch<patchs.size(); patch++)
					{
						const CPatch *pPatch=((const CZone&)zone).getPatch ((sint)patch);
						shadowMem+=pPatch->CompressedLumels.size();
					}

					// Compute time
					printf ("Uncompressed lumels: %d %%, shadow mem used: %d bytes\n",
						uncompressed*100/(compressed+uncompressed), shadowMem);

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
							fprintf (stderr, "Error writing %s: %s\n", argv[2], except.what());
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
					fprintf (stderr, "Error reading %s: %s\n", argv[1], except.what());
				}
			}
			else
			{
				// Error can't open the file
				fprintf (stderr, "Can't open %s for reading\n", argv[1]);
			}

		}
	}
	
	// exit.
	return shadowMem;
}
