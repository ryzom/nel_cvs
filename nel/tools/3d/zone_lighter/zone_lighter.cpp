/** \file zone_lighter.cpp
 * zone_lighter.cpp : Very simple zone lighter
 *
 * $Id: zone_lighter.cpp,v 1.5 2001/03/16 16:17:36 corvazier Exp $
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
#include "nel/misc/time_nl.h"
#include "nel/3d/zone.h"
#include "nel/3d/quad_grid.h"
#include "nel/3d/landscape.h"
#include "nel/misc/triangle.h"
#include "../zone_welder/zone_utility.h"	// load a header file from zone_welder project

#include <stdio.h>
#include <float.h>

using namespace std;
using namespace NLMISC;
using namespace NL3D;

#define BAR_LENGTH 21
#define LAST_LIST_SIZE_MAX 2
#define EPSILON_ST 0.1
#define EPSILON_POS 0.001
#define EPSILON 0.001
#define shadeShadow 0

char *progressbar[BAR_LENGTH]=
{
	"[                    ]",
	"[.                   ]",
	"[..                  ]",
	"[...                 ]",
	"[....                ]",
	"[.....               ]",
	"[......              ]",
	"[.......             ]",
	"[........            ]",
	"[.........           ]",
	"[..........          ]",
	"[...........         ]",
	"[............        ]",
	"[.............       ]",
	"[..............      ]",
	"[...............     ]",
	"[................    ]",
	"[.................   ]",
	"[..................  ]",
	"[................... ]",
	"[....................]"
};

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

class CTriangleShadow
{
public:
	CTriangle	triangle;
	CPlane		plane;
	sint8		nS;
	sint8		nT;
	uint		zone;
	uint		patch;
};

// Init the quadtree
void initQuadtree (CLandscape& landscape, CQuadGrid<CTriangleShadow>& quadGrid)
{
	quadGrid.create (256, 4.f);
}

// Add zone triangle in a quadtree
void addZoneInQuadTree (CLandscape& landscape, CQuadGrid<CTriangleShadow>& quadGrid, uint zoneId, std::vector<CPatchInfo>& patchs, CVector& lightDir)
{
	// Get the zone pointer
	CZone* pZone=landscape.getZone (zoneId);

	// Check the zone exist in the landscape
	nlassert (pZone);

	// Add triangle of this zone in the quadtree
	for (uint patch=0; patch<(uint)pZone->getNumPatchs(); patch++)
	{
		// vector of triangle
		std::vector<CTriangle> faces;

		// Build a list of triangles
		landscape.buildCollideFaces (zoneId, patch, faces);

		// Assume that there is OrderS*OrderT*2 triangles.
		float fOrderS=(float)patchs[patch].OrderS;
		float fOrderT=(float)patchs[patch].OrderT;

		// Add the triangles
		uint s,t,f;
		uint faceId=0;
		for (t=0; t<patchs[patch].OrderT; t++)
		for (s=0; s<patchs[patch].OrderS; s++)
		for (f=0; f<2; f++)
		{
			// Triangle ref
			CTriangle& triangle=faces[faceId];

			// Triangle info
			CTriangleShadow triangleShadow;
			triangleShadow.triangle=triangle;
			triangleShadow.plane.make (triangle.V0, triangle.V1, triangle.V2);

			// Back face of the sun ?
			if (lightDir*triangleShadow.plane.getNormal()>-EPSILON)
			{
				triangleShadow.nS=(uint8)s;
				triangleShadow.nT=(uint8)t;
				triangleShadow.zone=zoneId;
				triangleShadow.patch=patch;
				

				CVector min;
				min.minof (triangle.V0, triangle.V1);
				min.minof (min, triangle.V2);
				min-=CVector ((float)EPSILON_POS, (float)EPSILON_POS, (float)EPSILON_POS);
				CVector max;
				max.maxof (triangle.V0, triangle.V1);
				max.maxof (max, triangle.V2);
				max+=CVector ((float)EPSILON_POS, (float)EPSILON_POS, (float)EPSILON_POS);
				quadGrid.insert (min, max, triangleShadow);
			}

			// Next triangle
			faceId++;
		}
	}
}

bool intersected (const CTriangleShadow& triangleInfo, const CVector& pos, const CVector& lightPos, 
				  uint zoneId, uint patch, sint8 nS, sint8 nT)
{
	// Check the pos is back of the face
	// if (triangleInfo.plane*pos<0.f)
	{
		// Avoid self shadowing...
		if (
			(patch!=triangleInfo.patch)||		// patch number is not the same ?
			(zoneId!=triangleInfo.zone)||		// in the same zone ?
			(nS!=triangleInfo.nS) ||			// not same tile ?
			(nT!=triangleInfo.nT)
		   )
		{
			// hit
			CVector hit;

			// Check if the segment intersect...
			return (triangleInfo.triangle.intersect (pos, lightPos, hit, triangleInfo.plane));
		}
	}
	return false;
}

void loadAdjacentZones (const char* sZoneName, CLandscape& landscape, CQuadGrid<CTriangleShadow>& quadGrid, CVector &lightDir)
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

					// add the main zone
					addZoneInQuadTree (landscape, quadGrid, zone.getZoneId(), patchs, lightDir);
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
				// A landscape
				CLandscape landscape;

				// Quadtree filled of triangles
				CQuadGrid<CTriangleShadow> quadGrid;

				// Change the base
				NLMISC::CMatrix		tmp;
				NLMISC::CVector		I=(fabs(lightDir*CVector(1.f,0,0))>0.99)?CVector(0.f,1.f,0.f):CVector(1.f,0.f,0.f);
				NLMISC::CVector		K=-lightDir;
				NLMISC::CVector		J=K^I;
				J.normalize();
				I=J^K;
				I.normalize();

				tmp.identity();
				tmp.setRot(I,J,K, true);
				CMatrix rayBasis=tmp;
				tmp.invert ();
				quadGrid.changeBase (tmp);

				// The zone
				CZone zone;

				// Load
				zone.serial (inputFile);

				// Retrieve patch info
				std::vector<CPatchInfo> patchs;
				std::vector<CBorderVertex> borderVertices;
				zone.retrieve(patchs, borderVertices);

				// Add the zone
				landscape.addZone (zone);

				// Init it
				initQuadtree (landscape, quadGrid);

				// Load adjacent zones
				loadAdjacentZones (argv[1], landscape, quadGrid, lightDir);

				// *** build the quadtree

				// add the main zone
				addZoneInQuadTree (landscape, quadGrid, zone.getZoneId(), patchs, lightDir);

				// time
				TTime time=CTime::getLocalTime ();

				// Pixels count
				uint compressed=0;
				uint uncompressed=0;

				// Light each patch
				for (uint patch=0; patch<patchs.size(); patch++)
				{
					// Progress bar
					printf ("\rLighting in progress %s", progressbar[patch*BAR_LENGTH/patchs.size()]);
					

					// Patch
					CPatchInfo& pa=patchs[patch];

					// Num of tiles
					uint numS=pa.OrderS*NL_LUMEL_BY_TILE+1;
					uint numT=pa.OrderT*NL_LUMEL_BY_TILE+1;
					uint numTileS=pa.OrderS+1;
					uint numTileT=pa.OrderT+1;

					// Bool
					std::vector<uint8> vectorShade(numS*numT);

					// For all tiles
					uint ss, tt;
					for (tt=0; tt<numTileT-1; tt++)
					for (ss=0; ss<numTileS-1; ss++)
					{
						uint s2, t2;

						// Get the position of the vertices
						float s0=((float)ss)/(float)(numTileS-1);
						float s1=((float)(ss+1))/(float)(numTileS-1);
						float t0=((float)tt)/(float)(numTileT-1);
						float t1=((float)(tt+1))/(float)(numTileT-1);
						CVector pos0=pa.Patch.eval (s0, t0);
						CVector pos1=pa.Patch.eval (s0, t1);
						CVector pos2=pa.Patch.eval (s1, t1);
						CVector pos3=pa.Patch.eval (s1, t0);

						// last pixel
						uint lastS=(ss==numTileS-2)?NL_LUMEL_BY_TILE+1:NL_LUMEL_BY_TILE;
						uint lastT=(tt==numTileT-2)?NL_LUMEL_BY_TILE+1:NL_LUMEL_BY_TILE;

						for (t2=0; t2<lastT; t2++)
						for (s2=0; s2<lastS; s2++)
						{
							// sub coordinate
							uint s=(ss<<2)+s2;
							uint t=(tt<<2)+t2;

							// offset on the patch. (center off the lumel)
							float fS=((float)s)/(float)(numS-1);
							float fT=((float)t)/(float)(numT-1);
							float fS2=((float)s2)/(float)(NL_LUMEL_BY_TILE);
							float fT2=((float)t2)/(float)(NL_LUMEL_BY_TILE);
							fS2=(float)(EPSILON_ST+fS2*(1.f-2.f*EPSILON_ST));
							fT2=(float)(EPSILON_ST+fT2*(1.f-2.f*EPSILON_ST));

							// Get the position on the collid mesh for ray trace
							CVector collidPos;
							if (fS>fT)
								collidPos=pos3+(pos0-pos3)*(1.f-fS2)+(pos2-pos3)*fT2;
							else
								collidPos=pos1+(pos2-pos1)*fS2+(pos0-pos1)*(1.f-fT2);

							// Collid pos + 1 mm
							//collidPos-=lightDir*(float)EPSILON_POS;

							// Get the normal on the patch
							CVector normal=pa.Patch.evalNormal (fS, fT);

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
									if ((pa.BindEdges[border].NPatchs==1)&&(pa.BindEdges[border].ZoneId==zone.getZoneId()))
									{
										// Get the normal of the neighborhood
										CVector normalN=getEdgeNormal (patchs[pa.BindEdges[border].Next[0]].Patch, 
											pa.BindEdges[border].Edge[0], fBorder[border]);

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

							 // Clear the selection of the quad tree
							quadGrid.clearSelection ();

							// Infinite light pos
							CVector lightPos=collidPos-(lightDir*1000.f);

							// Shadow
							bool bShadow=false;

							// Front of the light
							if (lightDir*normalAccu<=EPSILON)
							{
								// Last oply intersected
								static std::list<CTriangleShadow> lastList;

								// Get first selected nodes..
								std::list<CTriangleShadow>::iterator itLast=lastList.begin();
								while (itLast!=lastList.end())
								{
									// Check intersection
									if (intersected (*itLast, collidPos, lightPos, zone.getZoneId(), patch, (sint8)(s>>2), (sint8)(t>>2)))
									{
										// Shadow
										bShadow=true;
										break;
									}

									// Next selected element
									itLast++;
								}

								if (!bShadow)
								{
									// Get first selected nodes..
									// Select an element with the X axis as a 3d ray
									quadGrid.select (collidPos-rayBasis.getI()-rayBasis.getJ(), lightPos+rayBasis.getI()+rayBasis.getJ());

									CQuadGrid<CTriangleShadow>::CIterator it=quadGrid.begin();
									while (it!=quadGrid.end())
									{
										// Check intersection
										if (intersected (*it, collidPos, lightPos, zone.getZoneId(), patch, (sint8)(s>>2), (sint8)(t>>2)))
										{
											// Shadow
											bShadow=true;

											// Add to the visited list
											lastList.push_front (*it);

											// Limit the list
											while (lastList.size()>LAST_LIST_SIZE_MAX)
											{
												std::list<CTriangleShadow>::iterator ite=lastList.end();
												ite--;
												lastList.erase (ite);
											}
											break;
										}

										// Next selected element
										it++;
									}
								}
							}

							// Light
							if (bShadow)
							{
								vectorShade[t*numS+s]=shadeShadow;
							}
							else
							{
								// Shadow
								sint nLum=(sint)(255.f*(-lightDir*normalAccu));
								clamp (nLum, 0, 255);
								vectorShade[t*numS+s]=nLum;
							}
						}
					}

					// Build shading color. One color by tile corner.
					uint s,t;
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
					{
						// For others lumels
						for (s=0; s<numS; s++)
						{
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
				}

				// Build the zone
				zone.build (zone.getZoneId(), patchs, borderVertices);

				// Count memory
				uint shadowMem=0;
				for (patch=0; patch<patchs.size(); patch++)
				{
					const CPatch *pPatch=((const CZone&)zone).getPatch ((sint)patch);
					shadowMem+=pPatch->CompressedLumels.size();
				}

				// Compute time
				printf ("\rCompute time: %d ms, uncompressed lumels: %d %%, shadow mem used: %d bytes                           \r", 
					(uint)(CTime::getLocalTime ()-time), uncompressed*100/(compressed+uncompressed), shadowMem);

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
	
	// exit.
	return 0;
}
