/** \file main.cpp
 * Display info on many NEL files. ig, zone etc...
 *
 * $Id: main.cpp,v 1.4 2003/05/22 14:14:27 corvazier Exp $
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
#include "3d/skeleton_shape.h"
#include "3d/register_3d.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "3d/mesh.h"
#include "3d/mesh_mrm.h"
#include "3d/mesh_multi_lod.h"


using	namespace std;
using	namespace NLMISC;
using	namespace NL3D;


// ***************************************************************************
void	displayGeom(FILE *logStream, const CMeshGeom &geom)
{
	uint	i,j;
	uint	numFaces=0;
	for(i=0;i<geom.getNbMatrixBlock();i++)
	{
		for(j=0;j<geom.getNbRdrPass(i);j++)
		{
			numFaces+= geom.getRdrPassPrimitiveBlock(i,j).getNumTri();
		}
	}
	fprintf(logStream, "  Standard Mesh %s\n", geom.isSkinned()?"Skinned":"" );
	fprintf(logStream, "  NumFaces: %d\n", numFaces );
	fprintf(logStream, "  NumVertices: %d\n", geom.getVertexBuffer().getNumVertices() );
}

void	displayMRMGeom(FILE *logStream, const CMeshMRMGeom &geom)
{
	uint	i,j;
	uint	numFaces=0;
	uint	numFacesLodMax=0;
	for(i=0;i<geom.getNbLod();i++)
	{
		for(j=0;j<geom.getNbRdrPass(i);j++)
		{
			uint	nPassFaces= geom.getRdrPassPrimitiveBlock(i,j).getNumTri();
			numFaces+= nPassFaces;
			if(i==geom.getNbLod()-1)
				numFacesLodMax+= nPassFaces;
		}
	}
	fprintf(logStream, "  MRM Mesh %s\n", geom.isSkinned()?"Skinned":"" );
	fprintf(logStream, "  NumFaces(Max Lod): %d\n", numFacesLodMax );
	fprintf(logStream, "  NumFaces(Sum all Lods): %d\n", numFaces );
	fprintf(logStream, "  NumVertices(Sum all Lods): %d\n", geom.getVertexBuffer().getNumVertices() );
}


// ***************************************************************************
/// Dispaly info for file in stdout
void	displayInfoFileInStream(FILE *logStream, const char *fileName, const set<string> &options)
{
	if(fileName==NULL)
		return;

	bool ms = options.find ("-ms") != options.end();
	bool vi = options.find ("-vi") != options.end();
	bool vl = options.find ("-vl") != options.end();

	// Special option.
	if( ms )
	{
		if(strstr(fileName, ".shape"))
		{
			// read the skeleton.
			CIFile	file(fileName);
			CShapeStream	shapeStream;
			file.serial(shapeStream);

			// Test Type
			CMesh			*mesh= dynamic_cast<CMesh*>(shapeStream.getShapePointer());

			// Mesh ??
			if( mesh )
			{
				if( mesh->getMeshGeom().isSkinned() )
				{
					fprintf(logStream, "%s is Skinned, but without MRM!!!\n", fileName);
				}
			}

			// release
			delete shapeStream.getShapePointer();
			shapeStream.setShapePointer(NULL);
		}
	}
	// Std Way.
	else
	{
		// some general info.
		fprintf(logStream, "File: %s\n", fileName);
		fprintf(logStream, "***********\n\n");

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
			fprintf(logStream, "  Num Patchs: %d\n", zone.getNumPatchs() );
			fprintf(logStream, "  Num PointLights: %d\n", zoneInfo.PointLights.size() );
			if (vl)
			{
				fprintf(logStream, "  Lights\n");
				uint k;
				for(k = 0; k < zoneInfo.PointLights.size(); ++k)
				{
					const CPointLightNamed &pl = zoneInfo.PointLights[k];
					fprintf(logStream, "    light group = %d, anim = \"%s\" x = %.1f, y = %.1f, z = %.1f\n", pl.LightGroup, pl.AnimatedLight.c_str(), pl.getPosition().x, pl.getPosition().y, pl.getPosition().z);
				}
			}
		}
		else if(strstr(fileName, ".ig"))
		{
			// read the ig.
			CIFile	file(fileName);
			CInstanceGroup	ig;
			file.serial(ig);

			// display Info on the ig:
			CVector gpos = ig.getGlobalPos();
			fprintf(logStream, "  Global pos : x = %.1f, y = %.1f, z =%.1f\n", gpos.x, gpos.y, gpos.z);
			fprintf(logStream, "  Num Instances: %d\n", ig.getNumInstance() );
			fprintf(logStream, "  Num PointLights: %d\n", ig.getPointLightList().size() );
			fprintf(logStream, "  Realtime sun contribution = %s\n", ig.getRealTimeSunContribution() ? "on" : "off");
			if (vi)
			{
				fprintf(logStream, "  Instances:\n");
				uint k;
				for(k = 0; k < ig._InstancesInfos.size(); ++k)
				{
					fprintf(logStream, "    Instance %s : x = %.1f, y = %.1f, z = %.1f, sx = %.1f, sy = %.1f, sz = %.1f\n", ig._InstancesInfos[k].Name.c_str(), ig._InstancesInfos[k].Pos.x + gpos.x, ig._InstancesInfos[k].Pos.y + gpos.y, ig._InstancesInfos[k].Pos.z + gpos.z, ig._InstancesInfos[k].Scale.x, ig._InstancesInfos[k].Scale.y, ig._InstancesInfos[k].Scale.z);
				}
			}
			if (vl)
			{
				fprintf(logStream, "  Lights:\n");
				uint k;
				for(k = 0; k < ig.getNumPointLights(); ++k)
				{
					const CPointLightNamed &pl = ig.getPointLightNamed(k);
					fprintf(logStream, "    Light group = %d, anim = \"%s\" x = %.1f, y = %.1f, z = %.1f\n", pl.LightGroup, pl.AnimatedLight.c_str(), pl.getPosition().x + gpos.x, pl.getPosition().y + gpos.y, pl.getPosition().z + gpos.z);
				}
			}
		}
		else if(strstr(fileName, ".skel"))
		{
			// read the skeleton.
			CIFile	file(fileName);
			CShapeStream	shapeStream;
			file.serial(shapeStream);
			CSkeletonShape	*skel= dynamic_cast<CSkeletonShape*>(shapeStream.getShapePointer());

			if(skel)
			{
				vector<CBoneBase>	bones;
				skel->retrieve(bones);
				// Display Bone Infos.
				fprintf(logStream, "Num Bones: %d\n", bones.size());
				for(uint i=0; i<bones.size(); i++)
				{
					// get default pos.
					const CAnimatedValueBlendable<CVector>	&posValue= 
						static_cast<const CAnimatedValueBlendable<CVector>	&>(bones[i].DefaultPos.getValue());
					CVector	pos= posValue.Value;

					// get default rotquat.
					const CAnimatedValueBlendable<CQuat>	&rotValue= 
						static_cast<const CAnimatedValueBlendable<CQuat>	&>(bones[i].DefaultRotQuat.getValue());
					CQuat	rotQuat= rotValue.Value;

					// get default scale.
					const CAnimatedValueBlendable<CVector>	&scaleValue= 
						static_cast<const CAnimatedValueBlendable<CVector>	&>(bones[i].DefaultScale.getValue());
					CVector	scale= scaleValue.Value;

					// print info
					fprintf(logStream, "Bone %2d. %s.\n", i, bones[i].Name.c_str());
					fprintf(logStream, "   Pos:      (%2.3f, %2.3f, %2.3f)\n", 
						pos.x, pos.y, pos.z);
					fprintf(logStream, "   RotQuat:  (%2.3f, %2.3f, %2.3f, %2.3f)\n", 
						rotQuat.x, rotQuat.y, rotQuat.z, rotQuat.w);
					fprintf(logStream, "   Scale:    (%2.3f, %2.3f, %2.3f)\n",
						scale.x, scale.y, scale.z);
				}
			}
			else
			{
				fprintf(logStream, "Bad Skel file\n");
			}

			// release
			delete shapeStream.getShapePointer();
			shapeStream.setShapePointer(NULL);
		}
		else if(strstr(fileName, ".shape"))
		{
			// read the shape.
			CIFile	file(fileName);
			CShapeStream	shapeStream;
			file.serial(shapeStream);

			// Test Type
			CMesh			*mesh= dynamic_cast<CMesh*>(shapeStream.getShapePointer());
			CMeshMRM		*meshMRM= dynamic_cast<CMeshMRM*>(shapeStream.getShapePointer());
			CMeshMultiLod	*meshMulti= dynamic_cast<CMeshMultiLod*>(shapeStream.getShapePointer());

			// Mesh ??
			if( mesh )
			{
				displayGeom(logStream, mesh->getMeshGeom());
			}
			// MRM ??
			else if( meshMRM )
			{
				displayMRMGeom(logStream, meshMRM->getMeshGeom());
			}
			// MultiLod??
			else if( meshMulti )
			{
				uint	numSlots= meshMulti->getNumSlotMesh ();
				fprintf(logStream, "  Num Lods: %d\n", numSlots );
				if(numSlots)
				{
					const CMeshGeom		*meshGeom= dynamic_cast<const CMeshGeom*>(&(meshMulti->getMeshGeom(0)));
					const CMeshMRMGeom	*meshMRMGeom= dynamic_cast<const CMeshMRMGeom*>(&(meshMulti->getMeshGeom(0)));
					if( meshGeom )
						displayGeom(logStream, *meshGeom);
					else if( meshMRMGeom )
						displayMRMGeom(logStream, *meshMRMGeom);
				}
			}
			else
			{
				fprintf(logStream, "Unsupported .shape type for display info\n");
			}

			// release
			delete shapeStream.getShapePointer();
			shapeStream.setShapePointer(NULL);
		}
		else
		{
			fprintf(logStream, "unsupported format\n");
		}
	}
}


// ***************************************************************************
// dispaly info for a file.
void		displayInfoFile(FILE *logStream, const char *fileName, const set<string> &options)
{
	// Display on screen.
	displayInfoFileInStream(stdout, fileName,options);
	// Display in log
	if(logStream)
		displayInfoFileInStream(logStream, fileName,options);
}


// ***************************************************************************
/// Dispaly info cmd line
int		main(int argc, const char *argv[])
{
	registerSerial3d();

	if(argc<2)
	{
		puts("Usage: ig_info file.??? [opt]");
		puts("Usage: ig_info directory [opt]");
		puts("    For now, only .ig, .zone, .skel, .shape are supported");
		puts("    Results are displayed too in \"c:/temp/file_info.log\" ");
		puts("    [opt] can get: ");
		puts("    -ms display only a Warning if file is a .shape and is a Mesh, skinned, but without MRM");
		puts("    -vi verbose instance informations");
		puts("    -vl verbose light informations");
		puts("Press any key");
		_getch();
		return -1;
	}

	// Parse options.
	set<string> options;
	int i;
	for (i=2; i<argc; i++)
		options.insert (argv[i]);

	// Open log
	FILE	*logStream;
	logStream= fopen("C:/temp/file_info.log", "wt");


	// parse dir or file ??
	const char *fileName= argv[1];
	if(CFile::isDirectory(fileName))
	{
		// dir all files.
		std::vector<std::string>	listFile;
		CPath::getPathContent (fileName, false, false, true, listFile);

		// For all files.
		for(uint i=0;i<listFile.size();i++)
		{
			displayInfoFile(logStream, listFile[i].c_str(), options);
		}
	}
	else
	{
		displayInfoFile(logStream, fileName, options);
	}


	// close log
	if(logStream)
		fclose(logStream);


	puts("Press any key");
	_getch();
}