/** \file ig_lighter.cpp
 * ig_lighter.cpp : Instance lighter
 *
 * $Id: ig_lighter.cpp,v 1.1 2002/02/06 16:57:47 berenguier Exp $
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
#include "nel/misc/config_file.h"
#include "nel/misc/path.h"

#include "3d/instance_lighter.h"
#include "3d/quad_grid.h"
#include "3d/scene_group.h"
#include "3d/shape.h"
#include "3d/transform_shape.h"
#include "3d/register_3d.h"
#include "pacs/global_retriever.h"
#include "pacs/local_retriever.h"
#include "pacs/retriever_bank.h"


using namespace std;
using namespace NLMISC;
using namespace NL3D;
using namespace NLPACS;

// ***************************************************************************
#define BAR_LENGTH 21

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



// My Ig lighter
class CMyIgLighter : public CInstanceLighter
{
public:
	static void	displayProgress(const char *message, float progress)
	{
		// Progress bar
		char msg[512];
		uint	pgId= (uint)(progress*(float)BAR_LENGTH);
		pgId= min(pgId, (uint)(BAR_LENGTH-1));
		sprintf (msg, "\r%s: %s", message, progressbar[pgId]);
		for (uint i=strlen(msg); i<79; i++)
			msg[i]=' ';
		msg[i]=0;
		printf (msg);
		printf ("\r");
	}

protected:
	// Progress bar
	virtual void progress (const char *message, float progress)
	{
		displayProgress(message, progress);
	}
};


// ***************************************************************************
struct	CSurfaceLightingInfo
{
	CRetrieverBank		*RetrieverBank;
	CGlobalRetriever	*GlobalRetriever;
	float				CellSurfaceLightSize;
	float				CellRaytraceDeltaZ; 
	// FileName without the extension
	string				IgFileName;
	string				ColIdentifierPrefix;
	string				ColIdentifierSuffix;

	// Debug.
	bool				BuildDebugSurfaceShape;
	string				DebugSunName;
};


// ***************************************************************************
// OverSample a cell, skipping samples not in Surface.
void	overSampleCell(CIGSurfaceLightBuild::CCellCorner &cell, uint nSampleReq, 
	const CLocalRetriever &localRetriever, CGlobalRetriever &globalRetriever, 
	sint retrieverInstanceId, const ULocalPosition &localPos, float cellSize,
	float cellRaytraceDeltaZ)
{
	uint	sample;

	nlassert(nSampleReq==2 || nSampleReq==4 || nSampleReq==8 || nSampleReq==16);
	nlassert(nSampleReq<=CInstanceLighter::MaxOverSamples);

	// Compute all localPosition according to overSampleGrid.
	//----------------
	float	s2= cellSize/2;
	float	s4= cellSize/4;
	float	s8= cellSize/8;
	ULocalPosition		localSamplePos[CInstanceLighter::MaxOverSamples];

	// copy from localPos. Surface and estimation.
	for(sample= 0; sample<nSampleReq; sample++)
		localSamplePos[sample]= localPos;

		// Special cases.
	if(nSampleReq==2)
	{
		localSamplePos[0].Estimation+= CVector(-s4, -s4, 0);
		localSamplePos[1].Estimation+= CVector(+s4, +s4, 0);
	}
	else if(nSampleReq==4)
	{
		localSamplePos[0].Estimation+= CVector(-s4, -s4, 0);
		localSamplePos[1].Estimation+= CVector(+s4, -s4, 0);
		localSamplePos[2].Estimation+= CVector(-s4, +s4, 0);
		localSamplePos[3].Estimation+= CVector(+s4, +s4, 0);
	}
	else if(nSampleReq==8)
	{
		/*
			For all 16 samples, but skip ones such that we have this pattern (X == sample).
			X . X .
			. X . X
			X . X .
			. X . X
		*/
		for(sample= 0; sample<16; sample++)
		{
			// Compute pos of the sample.
			uint	x= sample%4;
			uint	y= sample/4;
			// Skip to have only 8 samples like above
			if( (x+y) & 1 )
				continue;
			// Start at BL corner (-s2), get middle of sample (+s8) and there is 4 sample by line/column (*s4)
			// NB: index= sample/2 because only 8 samples are computed.
			localSamplePos[sample/2].Estimation.x+= -s2 + s8 + x*s4;
			localSamplePos[sample/2].Estimation.y+= -s2 + s8 + y*s4;
		}
	}
	else if(nSampleReq==16)
	{
		// For all 16 samples
		for(sample= 0; sample<16; sample++)
		{
			// Compute pos of the sample.
			uint	x= sample%4;
			uint	y= sample/4;
			// Start at BL corner (-s2), get middle of sample (+s8) and there is 4 sample by line/column (*s4)
			localSamplePos[sample].Estimation.x+= -s2 + s8 + x*s4;
			localSamplePos[sample].Estimation.y+= -s2 + s8 + y*s4;
		}
	}


	// For each sample, verify if inSurface, and store into samples.
	//----------------
	cell.NumOverSamples= 0;
	for(sample= 0; sample<nSampleReq; sample++)
	{
		bool	snapped;
		localRetriever.snapToInteriorGround(localSamplePos[sample], snapped);

		// If inSurface, add the sample to the list.
		if(snapped)
		{
			// build the globalPosition.
			UGlobalPosition	globalPos;
			globalPos.InstanceId= retrieverInstanceId;
			globalPos.LocalPosition= localSamplePos[sample];
			// Get the result global Position
			CVector		pos= globalRetriever.getGlobalPosition(globalPos);
			pos.z+= cellRaytraceDeltaZ;
			cell.OverSamples[cell.NumOverSamples++]= pos;
		}
	}

}


// ***************************************************************************
// Light an instance Group.
void	lightIg(const CInstanceGroup &igIn, CInstanceGroup &igOut, CInstanceLighter::CLightDesc &lightDesc, 
	CSurfaceLightingInfo &slInfo)
{
	CMyIgLighter		instLighter;
	sint				i;


	// Setup.
	//=======
	// Init
	instLighter.init();

	// Add obstacles.
	std::vector<CInstanceLighter::CTriangle>	obstacles;
	// only if Shadowing On.
	if(lightDesc.Shadow)
	{
		// Map of shape to load
		std::map<string, IShape*> shapeMap;

		// For all instances of igIn.
		for(i=0; i<(sint)igIn.getNumInstance();i++)
		{
			// progress
			CMyIgLighter::displayProgress("Loading Shapes obstacles", float(i)/igIn.getNumInstance());

			// Skip it??
			if(igIn.getInstance(i).DontCastShadow)
				continue;

			// Get the instance shape name
			string name= lightDesc.ShapePath + igIn.getShapeName(i);

			// Add a .shape at the end ?
			if (name.find('.') == std::string::npos)
				name += ".shape";

			// Find the shape in the bank
			std::map<string, IShape*>::iterator iteMap=shapeMap.find (name);
			if (iteMap==shapeMap.end())
			{
				// Input file
				CIFile inputFile;

				if (inputFile.open (name))
				{
					// Load it
					CShapeStream stream;
					stream.serial (inputFile);

					// Get the pointer
					iteMap=shapeMap.insert (std::map<string, IShape*>::value_type (name, stream.getShapePointer ())).first;
				}
				else
				{
					// Error
					nlwarning ("WARNING can't load shape %s\n", name.c_str());
				}
			}

			if(iteMap!=shapeMap.end())
			{
				CMatrix		matInst;
				matInst.setPos(igIn.getInstancePos(i));
				matInst.setRot(igIn.getInstanceRot(i));
				matInst.scale(igIn.getInstanceScale(i));
				// Add triangles of this shape
				CInstanceLighter::addTriangles(*iteMap->second, matInst, obstacles, i);
			}

		}

		// Clean Up shapes.
		//-----------
		std::map<string, IShape*>::iterator iteMap;
		iteMap= shapeMap.begin();
		while(iteMap!= shapeMap.end())
		{
			// delte shape
			delete	iteMap->second;
			// delete entry in map
			shapeMap.erase(iteMap);
			// next
			iteMap= shapeMap.begin();
		}
	}

	// Add pointLights of the IG.
	for(i=0; i<(sint)igIn.getPointLightList().size();i++)
	{
		instLighter.addStaticPointLight( igIn.getPointLightList()[i] );
	}


	// Setup a CIGSurfaceLightBuild if needed.
	//=======
	CIGSurfaceLightBuild	*igSurfaceLightBuild= NULL;
	CGlobalRetriever		*globalRetriever= slInfo.GlobalRetriever;
	CRetrieverBank			*retrieverBank= slInfo.RetrieverBank;
	float	cellSurfaceLightSize= slInfo.CellSurfaceLightSize;
	if(retrieverBank && globalRetriever)
	{
		igSurfaceLightBuild= new CIGSurfaceLightBuild;
		igSurfaceLightBuild->CellSize= cellSurfaceLightSize;
		// col Identifier.
		string	colIdent= slInfo.ColIdentifierPrefix + slInfo.IgFileName + slInfo.ColIdentifierSuffix;

		// For any retreiverInstance with this identifier.
		//----------------
		uint	numInstances= globalRetriever->getInstances().size();
		for(uint instanceId=0; instanceId<numInstances; instanceId++)
		{
			const CRetrieverInstance	&instance= globalRetriever->getInstance(instanceId);
			// If this instance is an interior
			if ( instance.getType() == CLocalRetriever::Interior )
			{
				uint					localRetrieverId= instance.getRetrieverId();
				const CLocalRetriever	&localRetriever= retrieverBank->getRetriever(localRetrieverId);
				// get the identifer of this localRetriever
				string	retIdent= localRetriever.getIdentifier();

				// Match the ident??
				if( retIdent.find(colIdent)!=string::npos )
				{
					// check CRetrieverLightGrid not already present
					CIGSurfaceLightBuild::ItRetrieverGridMap	itRgm;
					itRgm= igSurfaceLightBuild->RetrieverGridMap.find(retIdent);
					nlassert( itRgm == igSurfaceLightBuild->RetrieverGridMap.end() );

					// Append CRetrieverLightGrid.
					itRgm= igSurfaceLightBuild->RetrieverGridMap.insert(
						make_pair(retIdent, CIGSurfaceLightBuild::CRetrieverLightGrid() ) ).first;
					CIGSurfaceLightBuild::CRetrieverLightGrid	&rlg= itRgm->second;

					// Resize Grids.
					uint	numSurfaces= localRetriever.getSurfaces().size();
					rlg.Grids.resize( numSurfaces );

					// Compute the bbox for all surfaces. (NB: local to the localRetriever).
					vector<CAABBox>		surfaceBBoxes;
					localRetriever.buildInteriorSurfaceBBoxes(surfaceBBoxes);

					// For each surface, compute it.
					for(uint surfaceId=0; surfaceId<numSurfaces; surfaceId++)
					{
						// Progress.
						char	stmp[256];
						sprintf(stmp, "Sample surfaces of %s", retIdent.c_str());
						CMyIgLighter::displayProgress(stmp, surfaceId / float(numSurfaces));

						// Compute surface and size of the grid.
						CIGSurfaceLightBuild::CSurface		&surfDst= rlg.Grids[surfaceId];

						// Snap Origin on cellSize
						surfDst.Origin= surfaceBBoxes[surfaceId].getMin();
						surfDst.Origin.x= floorf(surfDst.Origin.x/cellSurfaceLightSize) * cellSurfaceLightSize;
						surfDst.Origin.y= floorf(surfDst.Origin.y/cellSurfaceLightSize) * cellSurfaceLightSize;

						// Snap Width / Height on cellSize.
						float	sizex= surfaceBBoxes[surfaceId].getMax().x - surfDst.Origin.x;
						float	sizey= surfaceBBoxes[surfaceId].getMax().y - surfDst.Origin.y;
						surfDst.Width= (uint)floorf(sizex/cellSurfaceLightSize) + 2;
						surfDst.Height= (uint)floorf(sizey/cellSurfaceLightSize) + 2;
						// Get Zcenter.
						float	zCenter= surfaceBBoxes[surfaceId].getCenter().z;

						// Allocate elements.
						surfDst.Cells.resize(surfDst.Width * surfDst.Height);

						// For all elements
						for(sint yCell=0; yCell<(sint)surfDst.Height; yCell++)
						{
							for(sint xCell=0; xCell<(sint)surfDst.Width; xCell++)
							{
								// compute pos of the cell.
								ULocalPosition	localPos;
								localPos.Estimation.x= surfDst.Origin.x + xCell*cellSurfaceLightSize;
								localPos.Estimation.y= surfDst.Origin.y + yCell*cellSurfaceLightSize;
								localPos.Estimation.z= zCenter;

								// snap the pos to the surface.
								localPos.Surface= surfaceId;
								bool	snapped;
								localRetriever.snapToInteriorGround(localPos, snapped);

								// if snapped then this point is IN the surface.
								CIGSurfaceLightBuild::CCellCorner	&cell= 
									surfDst.Cells[yCell * surfDst.Width + xCell];
								cell.InSurface= snapped;

								// If ok, retrieve the global (ie world) position
								if(snapped)
								{
									// build a valid globalPosition.
									UGlobalPosition	globalPos;
									globalPos.InstanceId= instanceId;
									globalPos.LocalPosition= localPos;
									// retrieve from globalRetriever.
									cell.CenterPos= globalRetriever->getGlobalPosition(globalPos);
									// Add a delta to simulate entity center
									cell.CenterPos.z+= slInfo.CellRaytraceDeltaZ;

									// OverSample
									if(lightDesc.OverSampling==0)
									{
										// No OverSample, just add CenterPos to the samples.
										cell.NumOverSamples= 1;
										cell.OverSamples[0]= cell.CenterPos;
									}
									else
									{
										// OverSample.
										overSampleCell(cell, lightDesc.OverSampling, localRetriever, 
											*globalRetriever, instanceId, localPos, cellSurfaceLightSize, 
											slInfo.CellRaytraceDeltaZ);
										// it is possible that no samples lies in surfaces (small surface).
										// In this case, just copy CenterPos into samples.
										if(cell.NumOverSamples==0)
										{
											cell.NumOverSamples= 1;
											cell.OverSamples[0]= cell.CenterPos;
										}
									}
								}
								else
								{
									// For debug mesh only, get an approximate pos.
									cell.CenterPos= localPos.Estimation + instance.getOrigin();
									cell.CenterPos.z+= slInfo.CellRaytraceDeltaZ;
								}

								// Init cell defaults
								cell.Dilated= false;
								cell.SunContribution= 0;
							}
						}
					}
					
				}
			}
		}

	}


	// Run.
	//=======
	instLighter.light(igIn, igOut, lightDesc, obstacles, NULL, igSurfaceLightBuild);

	// Output a debug mesh??
	if(igSurfaceLightBuild && slInfo.BuildDebugSurfaceShape)
	{
		// compute
		CMesh::CMeshBuild			meshBuild;
		CMeshBase::CMeshBaseBuild	meshBaseBuild;
		CVector	deltaPos= CVector::Null;
		deltaPos.z= - slInfo.CellRaytraceDeltaZ + 0.1f;
		igSurfaceLightBuild->buildSunDebugMesh(meshBuild, meshBaseBuild, deltaPos);

		// build
		CMesh	mesh;
		mesh.build(meshBaseBuild, meshBuild);

		// Save.
		CShapeStream	shapeStream;
		shapeStream.setShapePointer(&mesh);
		COFile		file(slInfo.DebugSunName);
		shapeStream.serial(file);
	}


	// Clean.
	//=======
	if(igSurfaceLightBuild)
		delete igSurfaceLightBuild;
}


// ***************************************************************************
int main(int argc, char* argv[])
{
	// Register 3d
	registerSerial3d ();

	// Good number of args ?
	if (argc<4)
	{
		// Help message
		printf ("ig_lighter [directoryIn] [pathOut] [parameter_file] \n");
	}
	else
	{
		try
		{
			string	directoryIn= argv[1];
			string	pathOut= argv[2];
			string	paramFile= argv[3];
			CInstanceLighter::CLightDesc	lighterDesc;
			string	grFile, rbankFile;

			// Verify directoryIn.
			directoryIn= CPath::standardizePath(directoryIn);
			if( !CFile::isDirectory(directoryIn) )
			{
				printf("DirectoryIn %s is not a directory", directoryIn.c_str());
				return -1;
			}
			// Verify pathOut.
			pathOut= CPath::standardizePath(pathOut);
			if( !CFile::isDirectory(pathOut) )
			{
				printf("PathOut %s is not a directory", pathOut.c_str());
				return -1;
			}

			// Load and setup configFile.
			//=================
			CConfigFile parameter;
			// Load and parse the param file
			parameter.load (paramFile);

			// shapes_path
			string shapes_path = parameter.getVar ("shapes_path").asString();
			shapes_path= CPath::standardizePath(shapes_path);
			lighterDesc.ShapePath= shapes_path;

			// Light direction
			CConfigFile::CVar &sun_direction = parameter.getVar ("sun_direction");
			lighterDesc.LightDirection.x=sun_direction.asFloat(0);
			lighterDesc.LightDirection.y=sun_direction.asFloat(1);
			lighterDesc.LightDirection.z=sun_direction.asFloat(2);
			lighterDesc.LightDirection.normalize ();

			// Grid size
			CConfigFile::CVar &quad_grid_size = parameter.getVar ("quad_grid_size");
			lighterDesc.GridSize=quad_grid_size.asInt();

			// Grid size
			CConfigFile::CVar &quad_grid_cell_size = parameter.getVar ("quad_grid_cell_size");
			lighterDesc.GridCellSize=quad_grid_cell_size.asFloat();

			// Shadows enabled ?
			CConfigFile::CVar &shadow = parameter.getVar ("shadow");
			lighterDesc.Shadow=shadow.asInt ()!=0;

			// OverSampling
			CConfigFile::CVar &oversampling = parameter.getVar ("oversampling");
			lighterDesc.OverSampling= oversampling.asInt ();
			// validate value: 0, 2, 4, 8, 16
			lighterDesc.OverSampling= raiseToNextPowerOf2(lighterDesc.OverSampling);
			clamp(lighterDesc.OverSampling, 0U, 16U);
			if(lighterDesc.OverSampling<2)
				lighterDesc.OverSampling= 0;

			// gr
			CConfigFile::CVar &grbank = parameter.getVar ("grbank");
			grFile= grbank.asString ();

			// rbank
			CConfigFile::CVar &rbank = parameter.getVar ("rbank");
			rbankFile= rbank.asString ();

			// CellSurfaceLightSize;
			CConfigFile::CVar &cell_surface_light_size = parameter.getVar ("cell_surface_light_size");
			float cellSurfaceLightSize= cell_surface_light_size.asFloat ();
			if(cellSurfaceLightSize<=0)
				throw Exception("cell_surface_light_size must be > 0");

			// CellRaytraceDeltaZ
			CConfigFile::CVar &cell_raytrace_delta_z = parameter.getVar ("cell_raytrace_delta_z");
			float cellRaytraceDeltaZ= cell_raytrace_delta_z.asFloat ();


			// colIdentifierPrefix
			CConfigFile::CVar &col_identifier_prefix = parameter.getVar ("col_identifier_prefix");
			string colIdentifierPrefix= col_identifier_prefix.asString ();

			// colIdentifierSuffix
			CConfigFile::CVar &col_identifier_suffix = parameter.getVar ("col_identifier_suffix");
			string colIdentifierSuffix= col_identifier_suffix.asString ();

			// colIdentifierSuffix
			CConfigFile::CVar &build_debug_surface_shape = parameter.getVar ("build_debug_surface_shape");
			bool	buildDebugSurfaceShape= build_debug_surface_shape.asInt()!=0;
			

			// try to open gr and rbank
			CRetrieverBank		*retrieverBank= NULL;
			CGlobalRetriever	*globalRetriever= NULL;
			if( grFile!="" && rbankFile!="" )
			{
				CIFile	fin;
				// serial the retrieverBank. Exception if not found.
				fin.open(CPath::lookup(rbankFile));
				retrieverBank= new CRetrieverBank;
				fin.serial(*retrieverBank);
				fin.close();

				// serial the globalRetriever. Exception if not found.
				fin.open(CPath::lookup(grFile));
				globalRetriever= new CGlobalRetriever;
				// set the RetrieverBank before loading
				globalRetriever->setRetrieverBank(retrieverBank);
				fin.serial(*globalRetriever);
				fin.close();

				// And init them.
				globalRetriever->initAll();
			}


			// Scan and load all files .ig in directories
			//=================
			vector<string>				listFile;
			vector<CInstanceGroup*>		listIg;
			vector<string>				listIgFileName;
			CPath::getPathContent(directoryIn, false, false, true, listFile);
			for(uint iFile=0; iFile<listFile.size(); iFile++)
			{
				string	&igFile= listFile[iFile];
				// verify it is a .ig.
				if( CFile::getExtension(igFile) == "ig" )
				{
					// Read the InstanceGroup.
					CInstanceGroup	*ig= new CInstanceGroup;
					CIFile	fin;
					fin.open(CPath::lookup(igFile));
					fin.serial(*ig);

					// add to list.
					listIg.push_back(ig);
					listIgFileName.push_back(CFile::getFilename(igFile));
				}
			}


			// For all ig, light them, and save.
			//=================
			for(uint iIg= 0; iIg<listIg.size(); iIg++)
			{
				// progress
				printf("Processing %s\n", listIgFileName[iIg].c_str());

				CInstanceGroup	igOut;

				// Export a debugSun Name.
				string	debugSunName;
				debugSunName= pathOut + "/" + CFile::getFilenameWithoutExtension(listIgFileName[iIg]) + "_debug_sun_.shape";

				// light the ig.
				CSurfaceLightingInfo	slInfo;
				slInfo.CellSurfaceLightSize= cellSurfaceLightSize;
				slInfo.CellRaytraceDeltaZ= cellRaytraceDeltaZ;
				slInfo.RetrieverBank= retrieverBank;
				slInfo.GlobalRetriever= globalRetriever;
				slInfo.IgFileName= CFile::getFilenameWithoutExtension(listIgFileName[iIg]);
				slInfo.ColIdentifierPrefix= colIdentifierPrefix;
				slInfo.ColIdentifierSuffix= colIdentifierSuffix;
				slInfo.BuildDebugSurfaceShape= buildDebugSurfaceShape;
				slInfo.DebugSunName= debugSunName;
				lightIg(*listIg[iIg], igOut, lighterDesc, slInfo);

				// Save this ig.
				COFile	fout;
				fout.open(pathOut+listIgFileName[iIg]);
				fout.serial(igOut);
				fout.close();

				// skip a line
				printf("\n");
			}

		}
		catch (Exception& except)
		{
			// Error message
			nlwarning ("ERROR %s\n", except.what());
		}
	}


	// Landscape is not deleted, nor the instanceGroups, for faster quit.
	// Must disalbe BlockMemory checks (for pointLights).
	NL3D_BlockMemoryAssertOnPurge= false;


	// exit.
	return 0;
}
