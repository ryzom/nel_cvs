/** \file zone_dependencies.cpp
 * zone_dependencies.cpp : make the zone dependencies file
 *
 * $Id: zone_dependencies.cpp,v 1.7 2002/02/26 17:30:25 corvazier Exp $
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

#include "../zone_welder/zone_utility.h"

#include "3d/zone.h"
#include "3d/quad_grid.h"
#include "3d/scene_group.h"
#include "3d/shape.h"
#include "3d/register_3d.h"




#include "nel/misc/config_file.h"
#include "nel/misc/file.h"
#include "nel/misc/aabbox.h"
#include "nel/misc/path.h"



#include <stdio.h>
#include <map>

using namespace std;
using namespace NLMISC;
using namespace NL3D;

typedef pair<sint, sint> CZoneDependenciesValue;

class CZoneDependencies
{
public:
	// Default Ctor
	CZoneDependencies ()
	{
		// Not loaded
		Loaded=false;
	}

	// Zone coordinates
	sint		X;
	sint		Y;

	// BBox
	CAABBoxExt	BBox;

	// Loaded
	bool							Loaded;

	// List 
	set<CZoneDependenciesValue>		Dependences;
};

class CZoneDescriptorBB
{
public:
	// Zone coordinates
	sint		X;
	sint		Y;

	// BBox
	CAABBoxExt	BBox;
};

/// A map to cache the shapes bbox's
typedef std::map<std::string, NLMISC::CAABBox> TShapeMap;


static bool computeIGBBox(const char *zoneName, NLMISC::CAABBox &result, TShapeMap &shapeMap);


///=========================================================
int main (int argc, char* argv[])
{
	TShapeMap shapeMap;

	// Check number of args
	if (argc<5)
	{
		// Help message
		printf ("zone_dependencies [properties.cfg] [firstZone.zone] [lastzone.zone] [output_dependencies.cfg]\n");
	}
	else
	{
		NL3D::registerSerial3d();
		// Light direction
		CVector lightDirection;

		// Config file handler
		try
		{
			// Read the properties file
			CConfigFile properties;
			
			// Load and parse the properties file
			properties.load (argv[1]);

			// Get the light direction
			CConfigFile::CVar &sun_direction = properties.getVar ("sun_direction");
			lightDirection.set (sun_direction.asFloat(0), sun_direction.asFloat(1), sun_direction.asFloat(2));
			lightDirection.normalize();


			// Get the search pathes
			CConfigFile::CVar &search_pathes = properties.getVar ("search_pathes");
			uint path;
			for (path = 0; path < (uint)search_pathes.size(); path++)
			{
				// Add to search path
				CPath::addSearchPath (search_pathes.asString(path));
			}
/*
			CConfigFile::CVar &ig_path = properties.getVar ("ig_path");
			NLMISC::CPath::addSearchPath(ig_path.asString(), true, true);

			CConfigFile::CVar &shapes_path = properties.getVar ("shapes_path");
			NLMISC::CPath::addSearchPath(shapes_path.asString(), true, true);
*/
			CConfigFile::CVar &compute_dependencies_with_igs = properties.getVar ("compute_dependencies_with_igs");
			bool computeDependenciesWithIgs = compute_dependencies_with_igs.asInt() != 0;			


			// Get the file extension
			string ext=getExt (argv[2]);

			// Get the file directory
			string dir=getDir (argv[2]);

			// Get output extension
			string outExt=getExt (argv[4]);

			// Get output directory
			string outDir=getDir (argv[4]);

			// Get the first and last name
			string firstName=getName (argv[2]);
			string lastName=getName (argv[3]);

			// Get the coordinates
			uint16 firstX, firstY;
			uint16 lastX, lastY;
			if (getZoneCoordByName (firstName.c_str(), firstX, firstY))
			{
				// Last zone
				if (getZoneCoordByName (lastName.c_str(), lastX, lastY))
				{
					// Take care
					if (lastX<firstX)
					{
						uint16 tmp=firstX;
						firstX=lastX;
						lastX=firstX;
					}
					if (lastY<firstY)
					{
						uint16 tmp=firstY;
						firstY=lastY;
						lastY=firstY;
					}

					// Min z
					float minZ=FLT_MAX;

					// Make a quad grid
					CQuadGrid<CZoneDescriptorBB>	quadGrid;
					quadGrid.create (256, 100);

					// The dependencies list
					vector< CZoneDependencies > dependencies;
					dependencies.resize ((lastX-firstX+1)*(lastY-firstY+1));

					// Insert each zone in the quad tree
					sint y, x;
					for (y=firstY; y<=lastY; y++)
					for (x=firstX; x<=lastX; x++)
					{
						// Make a zone file name
						string zoneName;
						getZoneNameByCoord (x, y, zoneName);

						// Open the file
						CIFile file;
						if (file.open (dir+zoneName+ext))
						{
							// The zone
							CZone zone;

							try
							{
								// Serial the zone
								file.serial (zone);

								/// get bbox from the ig of this zone
								NLMISC::CAABBox igBBox;
								bool hasIgBBox = computeDependenciesWithIgs ? computeIGBBox(zoneName.c_str(), igBBox, shapeMap)
																			: false;

								// Create a zone descriptor
								CZoneDescriptorBB zoneDesc;
								zoneDesc.X=x;
								zoneDesc.Y=y;

								if (!hasIgBBox)
								{
									zoneDesc.BBox=zone.getZoneBB();
								}
								else
								{
									NLMISC::CAABBox zoneBox;
									zoneBox.setCenter(zone.getZoneBB().getCenter());
									zoneBox.setHalfSize(zone.getZoneBB().getHalfSize());
									zoneDesc.BBox = CAABBoxExt(CAABBox::computeAABBoxUnion(zoneBox, igBBox));
								}					

								// Insert in the quad grid
								quadGrid.insert (zoneDesc.BBox.getMin(), zoneDesc.BBox.getMax(), zoneDesc);

								// Insert in the dependencies array
								uint index=(x-firstX)+(y-firstY)*(lastX-firstX+1);

								// Loaded
								dependencies[index].Loaded=true;
								dependencies[index].X=x;
								dependencies[index].Y=y;
								dependencies[index].BBox=zoneDesc.BBox;

								// ZMin
								float newZ=zoneDesc.BBox.getMin().z;
								if (newZ<minZ)
									minZ=newZ;
							}
							catch (Exception& e)
							{
								// Error handling
								nlwarning ("ERROR in file %s, %s", (dir+zoneName+ext).c_str(), e.what ());
							}
						}
					}

					// Now select each zone in others and make a depencies list
					for (y=firstY; y<=lastY; y++)
					for (x=firstX; x<=lastX; x++)
					{
						// Index 
						uint index=(x-firstX)+(y-firstY)*(lastX-firstX+1);

						// Loaded ?
						if (dependencies[index].Loaded)
						{
							// Min max vectors
							CVector vMin (dependencies[index].BBox.getMin());
							CVector vMax (dependencies[index].BBox.getMax());

							// Make a corner array 
							CVector corners[4] = 
							{
								CVector (vMin.x, vMin.y, vMax.z), CVector (vMax.x, vMin.y, vMax.z), 
								CVector (vMax.x, vMax.y, vMax.z), CVector (vMin.x, vMax.y, vMax.z)
							};

							// Extended bbox
							CAABBox	bBox=dependencies[index].BBox.getAABBox();

							// For each corner
							uint corner;
							for (corner=0; corner<4; corner++)
							{
								// Target position
								CVector target;
								if (lightDirection.z!=0)
								{
									// Not horizontal target
									target=corners[corner]+(lightDirection*((minZ-corners[corner].z)/lightDirection.z));
								}
								else
								{
									// Horizontal target, select 500 meters around.
									target=(500*lightDirection)+corners[corner];
								}

								// Extend the bbox
								bBox.extend (target);
							}

							// Clear quad tree selection
							quadGrid.clearSelection ();

							// Select
							quadGrid.select (bBox.getMin(), bBox.getMax());

							// Check selection
							CQuadGrid<CZoneDescriptorBB>::CIterator it=quadGrid.begin();
							while (it!=quadGrid.end())
							{
								// Index 
								uint targetIndex=((*it).X-firstX)+((*it).Y-firstY)*(lastX-firstX+1);

								// Not the same
								if (targetIndex!=index)
								{
									// Target min z
									float targetMinZ=dependencies[targetIndex].BBox.getMin().z;
									if (targetMinZ<vMax.z)
									{
										// Min z inf to max z ?
										// Target optimized bbox
										CAABBox	bBoxOptimized=dependencies[index].BBox.getAABBox();

										// For each corner
										for (corner=0; corner<4; corner++)
										{
											// Target position
											CVector target;
											if (lightDirection.z!=0)
											{
												// Not horizontal target
												target=corners[corner]+(lightDirection*((targetMinZ-corners[corner].z)
													/lightDirection.z));
											}
											else
											{
												// Horizontal target, select 500 meters around.
												target=(500*lightDirection)+corners[corner];
											}

											// Extend the bbox
											bBoxOptimized.extend (target);
										}

										// Check it more presisly
										//if ((*it).BBox.intersect (bBoxOptimized))
										if ((*it).BBox.intersect (bBox))
										{
											// Insert in the set
											dependencies[targetIndex].Dependences.insert (CZoneDependenciesValue (x, y));
										}
									}
								}

								// Next selected
								it++;
							}
						}
					}

					// For each zone
					for (y=firstY; y<=lastY; y++)
					for (x=firstX; x<=lastX; x++)
					{
						// Index 
						uint index=(x-firstX)+(y-firstY)*(lastX-firstX+1);

						// Loaded ?
						if (dependencies[index].Loaded)
						{
							// Make a file name
							string outputFileName;
							getZoneNameByCoord(x, y, outputFileName);
							outputFileName=outDir+outputFileName+outExt;

							// Write the dependencies file
							FILE *outputFile;
							if (outputFile=fopen (strlwr (outputFileName).c_str(), "w"))
							{
								// Add a dependency entry
								fprintf (outputFile, "dependencies =\n{\n");

								// Add dependent zones
								set<CZoneDependenciesValue>::iterator ite=dependencies[index].Dependences.begin();
								while (ite!=dependencies[index].Dependences.end())
								{
									// Name of the dependent zone
									std::string zoneName;
									getZoneNameByCoord(ite->first, ite->second, zoneName);

									// Write it
									string message="\t\""+zoneName+"\"";
									fprintf (outputFile, strlwr (message).c_str());

									// Next ite;
									ite++;
									if (ite!=dependencies[index].Dependences.end())
										fprintf (outputFile, ",\n");
								}

								// Close the variable
								fprintf (outputFile, "\n};\n\n");
							}
							else
							{
								nlwarning ("ERROR can't open %s for writing.\n", outputFileName.c_str());
							}

							// Close the file
							fclose (outputFile);
						}
					}

				}
				else
				{
					// Not valid
					nlwarning ("ERROR %s is not a valid zone name.\n", lastName.c_str());
				}
			}
			else
			{
				// Not valid
				nlwarning ("ERROR %s is not a valid zone name.\n", firstName.c_str());
			}
		}
		catch (Exception &ee)
		{
			nlwarning ("ERROR %s\n", ee.what());
		}
	}

	return 0;
}

///===========================================================================
/** Load and compute the bbox of the models that are located in a given zone
  * \return true if the computed bbox is valid
  */
static bool computeIGBBox(const char *zoneName, NLMISC::CAABBox &result, TShapeMap &shapeMap)
{
	std::string igFileName = zoneName + std::string(".ig");
	std::string pathName = CPath::lookup(igFileName, false, false);

	if (pathName.empty())
	{
		nlwarning("unable to find instance group of zone : %s", zoneName);
		return false;
	}


	/// Load the instance group of this zone
	CIFile igFile;
	if (!igFile.open(pathName))
	{
		nlwarning("unable to open file : %s", pathName.c_str());
		return false;
	}

	NL3D::CInstanceGroup ig;
	try
	{		
		ig.serial(igFile);
	}
	catch (NLMISC::Exception &e)
	{
		nlwarning("Error while reading an instance group file : %s \n reason : %s", pathName.c_str(), e.what());
		return false;
	}

	bool firstBBox = true;	
	NLMISC::CAABBox bbox;

	/// now, compute the union of all bboxs
	for (CInstanceGroup::TInstanceArray::const_iterator it = ig._InstancesInfos.begin(); it != ig._InstancesInfos.end(); ++it)
	{		
		NLMISC::CAABBox currBBox;

		bool validBBox = true; 
		/// get the bbox from file or from map
		if (shapeMap.count(it->Name)) // already loaded ?
		{
			currBBox = shapeMap[it->Name];
		}
		else // must load the shape to get its bbox
		{		
			std::string shapePathName;
			std::string toLoad = it->Name;
			if (getExt(toLoad).empty()) toLoad += ".shape";
			shapePathName = NLMISC::CPath::lookup(toLoad, false, false);

			if (shapePathName.empty())
			{
				nlwarning("Unable to find shape %s", it->Name.c_str());
				validBBox = false;
			}

			if (validBBox)
			{
				CIFile shapeInputFile;
				
				if (shapeInputFile.open (shapePathName.c_str()))
				{					
					NL3D::CShapeStream shapeStream;
					try
					{
						shapeStream.serial (shapeInputFile);
						shapeStream.getShapePointer()->getAABBox(currBBox);
						shapeMap[it->Name] = currBBox;
					}
					
					catch (NLMISC::Exception &e)
					{
						nlwarning("Error while loading shape %s. \n\t Reason : %s ", it->Name.c_str(), e.what());
					}				
				}
				else
				{
					nlwarning("Unable to open shape file %s to get its bbox", it->Name.c_str());
				}
			}
		}

		if (validBBox)
		{
			/// build the model matrix
			NLMISC::CMatrix mat;
			mat.scale(it->Scale);
			NLMISC::CMatrix rotMat;
			rotMat.setRot(it->Rot);
			mat = rotMat * mat;
			mat.setPos(it->Pos);

			/// transform the bbox
			currBBox = NLMISC::CAABBox::transformAABBox(mat, currBBox);
	
		
			if (firstBBox)
			{
				bbox = currBBox;
				firstBBox = false;
			}
			else // add to previous one
			{						
				bbox = NLMISC::CAABBox::computeAABBoxUnion(bbox, currBBox);
			}			
		}		
	}

	return !firstBBox; // found at least one bbox ?
}



