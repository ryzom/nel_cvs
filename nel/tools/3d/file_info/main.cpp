/** \file main.cpp
 * Display info on many NEL files. ig, zone etc...
 *
 * $Id: main.cpp,v 1.2 2002/05/28 08:35:47 berenguier Exp $
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


using	namespace std;
using	namespace NLMISC;
using	namespace NL3D;


/// Dispaly info for file in stdout
void	displayInfo(FILE *logStream, const char *fileName)
{
	if(fileName==NULL)
		return;

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
	}
	else if(strstr(fileName, ".ig"))
	{
		// read the ig.
		CIFile	file(fileName);
		CInstanceGroup	ig;
		file.serial(ig);

		// display Info on the ig:
		fprintf(logStream, "  Num Instances: %d\n", ig.getNumInstance() );
		fprintf(logStream, "  Num PointLights: %d\n", ig.getPointLightList().size() );
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
	}
	else
	{
		fprintf(logStream, "unsupported format\n");
	}
}


/// Dispaly info cmd line
int		main(int argc, const char *argv[])
{
	registerSerial3d();

	if(argc<2)
	{
		puts("Usage: ig_info file.???");
		puts("    For now, only .ig, .zone, .skel are supported");
		puts("    Results are displayed too in \"c:/temp/file_info.log\" ");
		puts("Press any key");
		_getch();
		return -1;
	}

	const char *fileName= argv[1];

	// Display on screen.
	displayInfo(stdout, fileName);

	// Display in file.
	FILE	*logStream;
	logStream= fopen("C:/temp/file_info.log", "wt");
	if(logStream)
	{
		displayInfo(logStream, fileName);
		fclose(logStream);
	}

	puts("Press any key");
	_getch();
}