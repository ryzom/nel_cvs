/** \file coarse_mesh_manager.h
 * Management of coarse meshes.
 *
 * $Id: coarse_mesh_manager.h,v 1.20 2005/02/22 10:19:10 besson Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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

#ifndef NL_COARSE_MESH_MANAGER_H
#define NL_COARSE_MESH_MANAGER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"

#include "transform.h"
#include "material.h"
#include "index_buffer.h"
#include "vertex_buffer.h"

namespace NL3D 
{

// ***************************************************************************

// Number of vertices that can be rendered in one pass
#define NL3D_COARSEMESH_VERTEXBUFFER_SIZE				20000
// Number of trianges that can be rendered in one pass
#define NL3D_COARSEMESH_TRIANGLE_SIZE					10000
// The vertex Format used by the coarseMesh manager
#define NL3D_COARSEMESH_VERTEX_FORMAT_MGR				(CVertexBuffer::PositionFlag|CVertexBuffer::TexCoord0Flag|CVertexBuffer::PrimaryColorFlag)
// The Vertex Format used for export CoarseMesh. MUST NOT HAVE color (important for material coloring/alphaTrans)
#define NL3D_COARSEMESH_VERTEX_FORMAT_EXPORT			(CVertexBuffer::PositionFlag|CVertexBuffer::TexCoord0Flag)

// ***************************************************************************

class CMeshGeom;
class CTransformShape;
class CTextureFile;


// ***************************************************************************

/**
 * Management of coarse meshes.
 *
 * This container will render meshes with very low polygon count efficiently.
 *
 * All coarse meshes must use a common vertex format. It is a pos + UV vertex format.
 * (NL3D_COARSEMESH_VERTEX_FORMAT_EXPORT)
 *
 * Internally the CCoarseMeshManager store meshes with pos + UV + color vertex format, to color instances
 * (NL3D_COARSEMESH_VERTEX_FORMAT_MGR)
 *
 * Coarse meshes must use indexed triangle primitive in a single render pass in a single matrix block.
 *
 * All coarse meshes musts use a single material. It is a simple mapping with alpha test rendering and a common
 * texture.
 *
 * The coarse meshes must have been preprocessed to build the common texture and remap the UV mapping coordinates
 * in the new common texture.
 *
 * The manager must have been setuped with the common texture.
 *
 * \author Cyril 'Hulud' Corvazier, Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CCoarseMeshManager
{
public:

	/// Constructor
	CCoarseMeshManager ();

	/// Set texture file to use with this coarse mesh
	void		setTextureFile (const char* file);

	/**
	  * Add a coarse mesh in the manager. If an error occured, it returns CantAddCoarseMesh.
	  *	\param vBuffer the VertexBuffer pre-transformed / Colored. Size MUST be numVertices*NL3D_COARSEMESH_VERTEX_FORMAT_MGR
	  *	\param indexBuffer containing triangles that will be inserted.
	  *	\return false if the mesh can't be added to this pass BECAUSE OF TOO MANY VERTICES or TOO MANY PRIMITIVES reason
	  *	You may call flushRender(), then restart a block. 
	  *	NB: if numVertices>NL3D_COARSEMESH_VERTEXBUFFER_SIZE or if numTriangles>NL3D_COARSEMESH_TRIANGLE_SIZE, it will always
	  *	NB: the color of vbUffer must already be correct against IDriver::getVertexColorFormat()
	  *	return false
	  */
	bool		addMesh (uint numVertices, const uint8 *vBuffer, uint numTris, const TCoarseMeshIndexType *indexBuffer);

	/**
	  * Render the container
	  */
	void		flushRender (IDriver *drv);

	/**
	  *	Get material of the container. For rendering purpose only.
	  */
	CMaterial	&getMaterial() {return _Material;}

	/// Get the VertexSize of the MGR format
	uint		getVertexSize() const {return _VBuffer.getVertexSize();}
	uint		getUVOff() const {return (uint)_VBuffer.getTexCoordOff(0);}
	uint		getColorOff() const {return (uint)_VBuffer.getColorOff();}

private:

	CVertexBuffer						_VBuffer;
	CIndexBuffer						_Triangles;
	uint								_CurrentNumVertices;
	uint								_CurrentNumTriangles;

	// The unique texture used by all the coarse object inserted in the container.
	CSmartPtr<CTextureFile>				_Texture;

	// The unique material used by all the coarse object inserted in the container.
	CMaterial							_Material;

	// Texture category for profilings
	NLMISC::CSmartPtr<ITexture::CTextureCategory>		_TextureCategory;
	//
	CIndexBufferReadWrite				_IBA;
	CVertexBufferReadWrite				_VBA;	

	struct CMeshInfo
	{
		uint			  NumVertices;
		const uint8		  *VBuffer;
		uint			  NumTris;
		const TCoarseMeshIndexType *IndexBuffer;
	};
	std::vector<CMeshInfo> _Meshs;
};


} // NL3D


#endif // NL_COARSE_MESH_MANAGER_H

/* End of coarse_mesh_manager.h */
