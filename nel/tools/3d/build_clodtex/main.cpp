/** \file lod_texture_builder.h
 * <File description>
 *
 * $Id: main.cpp,v 1.1 2002/11/08 18:44:23 berenguier Exp $
 */

/* Copyright, 2000-2002 Nevrax Ltd.
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


#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include <math.h>
#include "lod_texture_builder.h"
#include "3d/mesh.h"
#include "3d/mesh_mrm.h"
#include "3d/register_3d.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;


// ***************************************************************************
int main(int argc, char *argv[])
{
	// Filter addSearchPath
	NLMISC::createDebug();
	NLMISC::InfoLog->addNegativeFilter ("adding the path");

	NL3D::registerSerial3d();

	if (argc != 4)
	{
		nlwarning("%s compute a Lod textureInfo to put in a shape", argv[0]);
		nlwarning("usage : %s clod_in shape_in shape_out ", argv[0]);
		exit(-1);
	}

	try
	{
		// Load the clod.
		CLodCharacterShapeBuild		theLod;
		CIFile	fIn;
		if(!fIn.open(argv[1]))
			throw Exception("Can't load %s", argv[1]);
		fIn.serial(theLod);
		fIn.close();

		// Load the shape.
		CSmartPtr<IShape>			theShape;
		if(!fIn.open(argv[2]))
			throw Exception("Can't load %s", argv[2]);
		CShapeStream	ss;
		fIn.serial(ss);
		if(!ss.getShapePointer())
			throw Exception("Can't load %s", argv[2]);
		theShape= ss.getShapePointer();
		fIn.close();

		// init the LodBuilder
		CLodTextureBuilder	lodBuilder;
		lodBuilder.setLod(theLod);

		// compute the texture.
		CLodCharacterTexture	lodTexture;
		CMesh		*mesh= dynamic_cast<CMesh*>((IShape*)theShape);
		CMeshMRM	*meshMRM= dynamic_cast<CMeshMRM*>((IShape*)theShape);
		if(mesh)
			lodBuilder.computeTexture(*mesh, lodTexture);
		else if(meshMRM)
			lodBuilder.computeTexture(*meshMRM, lodTexture);
		else
			throw Exception("The shape %s is not a Mesh/MeshMRM", argv[2]);

		// store in mesh
		if(mesh)
			mesh->setupLodCharacterTexture(lodTexture);
		else
			meshMRM->setupLodCharacterTexture(lodTexture);
		// serial
		COFile	fOut;
		if(!fOut.open(argv[3]))
			throw Exception("Can't open %s for writing", argv[2]);
		ss.setShapePointer(theShape);
		fOut.serial(ss);

		// TestYoyo
		/*CBitmap		dbg;
		dbg.resize(lodTexture.getWidth(), lodTexture.getHeight());
		memcpy(&dbg.getPixels(0)[0], &lodTexture.Texture[0], dbg.getSize()*4);
		COFile	dbgF("testDBG.tga");
		dbg.writeTGA(dbgF, 32);*/
	}
	catch(Exception &e)
	{
		nlwarning("ERROR: %s", e.what());
	}

	return 0;
}