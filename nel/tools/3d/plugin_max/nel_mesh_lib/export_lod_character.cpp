/** \file export_lod_character.cpp
 * Export from 3dsmax a NeL lod character
 *
 * $Id: export_lod_character.cpp,v 1.2 2002/08/27 12:40:45 corvazier Exp $
 */

/* Copyright, 2002 Nevrax Ltd.
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

#include "stdafx.h"
#include "export_nel.h"
#include "export_appdata.h"

#include "../nel_export/std_afx.h"
#include "../nel_export/nel_export.h"

#include <3d/lod_character_shape.h>

using namespace NLMISC;
using namespace NL3D;

// ***************************************************************************

bool  CExportNel::buildLodCharacter (NL3D::CLodCharacterShapeBuild& lodBuild, INode& node, TimeValue time, const TInodePtrInt *nodeMap)
{
	// Is it a tri object ?
	bool res = false;

	// If skinning, disable skin modifier
	if (nodeMap)
		enableSkinModifier (node, false);
	// a lod character must have skinning
	else
		return false;

	// Get a pointer on the object's node
    Object *obj = node.EvalWorldState(time).obj;

	// Check if there is an object
	if (obj)
	{
		Class_ID  clid = obj->ClassID();

		// Object can be converted in triObject ?
		if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) 
		{ 
			// Get a triobject from the node
			TriObject *tri = (TriObject *) obj->ConvertToType(time, Class_ID(TRIOBJ_CLASS_ID, 0));

			// Note that the TriObject should only be deleted
			// if the pointer to it is not equal to the object
			// pointer that called ConvertToType()
			bool deleteIt=false;
			if (obj != tri) 
				deleteIt = true;

			// Get the node matrix
			Matrix3 nodeMatrixMax;
			CMatrix nodeMatrix;
			getLocalMatrix (nodeMatrixMax, node, time);
			convertMatrix (nodeMatrix, nodeMatrixMax);


			// build the shape
			{					
				// Array of name for the material
				CMaxMeshBaseBuild maxBaseBuild;

				// Fill the build interface of CMesh
				CMeshBase::CMeshBaseBuild buildBaseMesh;
				buildBaseMeshInterface (buildBaseMesh, maxBaseBuild, node, time, nodeMatrix);

				CMesh::CMeshBuild buildMesh;
				buildMeshInterface (*tri, buildMesh, buildBaseMesh, maxBaseBuild, node, time, nodeMap);

				// Build the lodBuild with the meshBuild
				// The mesh must have skinning.
				if( buildMesh.Vertices.size() == buildMesh.SkinWeights.size() )
				{
					// build vertices and skinWeights
					lodBuild.Vertices= buildMesh.Vertices;
					lodBuild.SkinWeights= buildMesh.SkinWeights;

					// build triangles.
					lodBuild.TriangleIndices.resize(buildMesh.Faces.size() * 3);
					for(uint i=0;i<buildMesh.Faces.size();i++)
					{
						lodBuild.TriangleIndices[i*3+0]= buildMesh.Faces[i].Corner[0].Vertex;
						lodBuild.TriangleIndices[i*3+1]= buildMesh.Faces[i].Corner[1].Vertex;
						lodBuild.TriangleIndices[i*3+2]= buildMesh.Faces[i].Corner[2].Vertex;
					}

					// build boneNames
					lodBuild.BonesNames= buildMesh.BonesNames;

					// Ok!!
					res= true;
				}
			}

			// Delete the triObject if we should...
			if (deleteIt)
				delete tri;
		}
	}

	// If skinning, renable skin modifier
	if (nodeMap)
		enableSkinModifier (node, true);

	return res;
}
