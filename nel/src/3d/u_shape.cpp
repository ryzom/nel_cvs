/** \file u_shape.cpp
 * <File description>
 *
 * $Id: u_shape.cpp,v 1.2 2004/03/19 10:11:36 corvazier Exp $
 */

/* Copyright, 2000-2003 Nevrax Ltd.
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


#include "std3d.h"
#include "nel/3d/u_shape.h"
#include "3d/mesh.h"
#include "3d/mesh_multi_lod.h"


using namespace std;
using namespace NLMISC;


namespace NL3D 
{


// ***************************************************************************
UShape::UShape() : _Shape(NULL)
{
}

// ***************************************************************************
void		UShape::attach(class IShape *shape)
{
	_Shape= shape;
}

// ***************************************************************************
bool		UShape::getMeshTriangles(std::vector<NLMISC::CVector> &vertices, std::vector<uint32> &indices) const
{
	if(!_Shape)
		return false;

	// **** try to get a CMeshGeom
	CMesh				*mesh= dynamic_cast<CMesh*>(_Shape);
	CMeshMultiLod		*meshMulti= dynamic_cast<CMeshMultiLod*>(_Shape);
	const CMeshGeom		*meshGeom= NULL;
	if(mesh)
	{
		meshGeom= &mesh->getMeshGeom();
	}
	else if(meshMulti)
	{
		// get the first (bigger) meshGeom
		if(meshMulti->getNumSlotMesh())
		{
			meshGeom= dynamic_cast<const CMeshGeom*>(&meshMulti->getMeshGeom(0));
		}
	}

	if(!meshGeom)
		return false;

	// **** Build the vertices and indices
	uint	i;
	vertices.clear();
	indices.clear();

	// build vertices
	const CVertexBuffer	&vb= meshGeom->getVertexBuffer();
	vertices.resize(vb.getNumVertices());
	{
		CVertexBufferRead vba;
		vb.lock (vba);
		const uint8	*pVert= (const uint8*)vba.getVertexCoordPointer(0);
		uint		vSize= vb.getVertexSize();
		for(i=0;i<vertices.size();i++)
		{
			vertices[i]= *(const CVector*)pVert;
			pVert+= vSize;
		}
	}

	// count numTris
	uint	numTris= 0;
	for(i=0;i<meshGeom->getNbMatrixBlock();i++)
	{
		for(uint rp=0;rp<meshGeom->getNbRdrPass(i);rp++)
		{
			numTris+= meshGeom->getRdrPassPrimitiveBlock(i, rp).getNumIndexes()/3;
		}
	}
	indices.resize(numTris*3);

	// build indices
	uint	triIdx= 0;
	for(i=0;i<meshGeom->getNbMatrixBlock();i++)
	{
		for(uint rp=0;rp<meshGeom->getNbRdrPass(i);rp++)
		{
			const CIndexBuffer	&pb= meshGeom->getRdrPassPrimitiveBlock(i, rp);
			CIndexBufferRead iba;
			pb.lock (iba);
			// copy
			memcpy(&indices[triIdx*3], iba.getPtr(), pb.getNumIndexes()*sizeof(uint32));
			// next
			triIdx+= pb.getNumIndexes()/3;
		}
	}
	
	// ok!
	return true;
}


// ***************************************************************************
bool			UShape::cameraCollisionable() const
{
	/* For now do it easy, take only lightmapped (ie big) objects. An object that is lightmapped
		should have some lightInfos
	*/
	CMeshBase			*mesh= dynamic_cast<CMeshBase*>(_Shape);
	if(mesh)
	{
		return !mesh->_LightInfos.empty();
	}
	else
		return false;
}


} // NL3D
