/** \file coarse_mesh_manager.h
 * Management of coarse meshes.
 *
 * $Id: coarse_mesh_manager.h,v 1.8 2002/04/12 12:04:58 lecroart Exp $
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

#include "3d/transform.h"
#include "3d/material.h"
#include "3d/primitive_block.h"
#include "3d/vertex_buffer.h"

namespace NL3D 
{

// ***************************************************************************

#define NL3D_COARSEMESH_VERTEXBUFFER_GRANULARITY_SHIFT	3
#define NL3D_COARSEMESH_VERTEXBUFFER_GRANULARITY_MASK	(NL3D_COARSEMESH_VERTEXBUFFER_GRANULARITY-1)
#define NL3D_COARSEMESH_VERTEXBUFFER_GRANULARITY		(1<<NL3D_COARSEMESH_VERTEXBUFFER_GRANULARITY_SHIFT)
#define NL3D_COARSEMESH_VERTEXBUFFER_RESERVE			10
#define NL3D_COARSEMESH_VERTEX_FORMAT					(CVertexBuffer::PositionFlag|CVertexBuffer::TexCoord0Flag)
#define NL3D_COARSEMESH_PRIMITIVE_BLOCK_SIZE			100

// ***************************************************************************

class CMeshGeom;
class CTransformShape;
class CTextureFile;

// ***************************************************************************

const NLMISC::CClassId		CoarseMeshManagerId=NLMISC::CClassId(0x77554f87, 0x5bb373d8);

// ***************************************************************************

/**
 * Management of coarse meshes.
 *
 * This container will render meshes with very low polygon count efficiently.
 *
 * Coarse meshes are merged in render passes. They are inserted in the render pass depending there
 * number of vertices, with a granularity (NL3D_COARSEMESH_VERTEXBUFFER_GRANULARITY).
 *
 * If NL3D_COARSEMESH_VERTEXBUFFER_GRANULARITY is 8, all meshes with 1 to 7 vertices will be rendered
 * at the same time using the same vertex buffer, the same material and the same matrix.
 * Then, meshes with 8 to 15 vertices will be rendered at the same time. etc..
 *
 * Vertices are softly transformed in world space at the "setMatrixMesh" call.
 *
 * All coarse meshes must use a common vertex format. It is a pos + normal + UV + vertex color vertex format.
 * (NL3D_COARSEMESH_VERTEX_FORMAT)
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
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CCoarseMeshManager : public CTransform
{
public:

	enum
	{
		CantAddCoarseMesh = 0xffffffff
	};

	/// Constructor
	CCoarseMeshManager ();

	/// Set texture file to use with this coarse mesh
	void setTextureFile (const char* file);

	/**
	  * Add a coarse mesh in the manager. If an error occured, it returns CantAddCoarseMesh.
	  * Error can be too much vertex, wrong vertex format.
	  */
	uint64 addMesh (const CMeshGeom& geom);

	/**
	  * Remove a coarse mesh in the manager
	  */
	void removeMesh (uint64 id);

	/**
	  * Set the matrix of a mesh
	  */
	void setMatrixMesh (uint64 id, const CMeshGeom& geom, const CMatrix& matrix);

	/**
	  * Set the global color of the coarse meshes
	  */
	void setColor (NLMISC::CRGBA color);

	/**
	  * Render the container
	  */
	void render (IDriver *drv);

	/**
	  * Register class id.
	  */
	static void		registerBasic();
private:

	/**
	  * Build a manager id
	  */
	static uint64 buildId (uint32 renderPassId, uint32 renderPassMeshId)
	{
		return ((uint64)renderPassId|(((uint64)renderPassMeshId)<<32));
	}

	/**
	  * Get the render pass id
	  */
	static uint32 getRenderPassId (uint64 id)
	{
		return (uint32)(id&0xFFFFFFFF);
	}

	/**
	  * Get the render pass mesh id
	  */
	static uint32 getRenderPassMeshId (uint64 id)
	{
		return (uint32)(id>>32);
	}

	class CRenderPass
	{
	public:
		enum
		{
			CantAddMesh = 0xffffffff
		};

		/**
		  * Init
		  */
		void init (uint blockSize);

		/**
		  * Add a coarse mesh in the render pass
		  */
		uint32 addMesh (const CMeshGeom& geom);

		/**
		  * Get a tri count of a mesh.
		  */
		uint	getTriCount (const CMeshGeom& geom);

		/**
		  * Remove a coarse mesh in the manager
		  */
		void removeMesh (uint32 id);

		/**
		  * Set the matrix of a mesh
		  */
		void setMatrixMesh (uint32 id, const CMeshGeom& geom, const CMatrix& matrix);

		/**
		  * Render the container
		  */
		void render (IDriver *drv, CMaterial& mat);
	private:

		/**
		  * Build a mesh id
		  */
		static uint32 buildId (uint16 primitiveBlockId, uint16 vertexBufferId)
		{
			return ((uint32)primitiveBlockId|(((uint32)vertexBufferId)<<16));
		}

		/**
		  * Get the primitive block id
		  */
		static uint16 getPrimitiveblockId (uint32 id)
		{
			return (uint16)id;
		}

		/**
		  * Get the vertexbuffer id
		  */
		static uint16 getVertexBufferId (uint32 id)
		{
			return (uint16)(id>>16);
		}

		/**
		  * Primitive block info
		  */
		class CPrimitiveBlockInfo
		{
		public:
			enum
			{
				Failed = 0,
				Success = 1,
			};

			/**
			  * Add a coarse mesh in the primitive block. Return CPrimitiveBlockInfo::Failed is not enought space.
			  */
			uint	addMesh (uint16 vertexBufferId, const CMeshGeom& geom, uint32 firstVertexIndex, uint triCount);

			/**
			  * Add a coarse mesh from the primitive block.
			  */
			void removeMesh (uint16 vertexBufferId);

			/// The mesh info in the block
			class CMeshInfo
			{
			public:
				/// Ctor
				CMeshInfo (uint offset, uint length, uint16 vertexBufferId)
				{
					Offset=offset;
					Length=length;
					VertexBufferId=vertexBufferId;
				}

				/// Mesh first triangle offset
				uint	Offset;

				/// Mesh triangle list length
				uint	Length;

				/// Vertex buffer Id
				uint16	VertexBufferId;
			};

			/// Init
			void init (uint size);

			/// The primitive block
			CPrimitiveBlock					PrimitiveBlock;

			/// The mesh id list
			std::list< CMeshInfo >			MeshIdList;
		};

		uint								VBlockSize;
		CVertexBuffer						VBuffer;
		std::list< uint16 >					FreeVBlock;
		std::list< CPrimitiveBlockInfo >	PrimitiveBlockInfoList;
	};

	// Indexes allocation
	typedef std::map< uint, CRenderPass >	TRenderingPassMap;
	TRenderingPassMap						_RenderPass;

	// The unique texture used by all the coarse object inserted in the container.
	CSmartPtr<CTextureFile>					_Texture;

	// The unique material used by all the coarse object inserted in the container.
	CMaterial								_Material;

	static IModel	*creator() {return new CCoarseMeshManager;}
};

// ***************************************************************************

/**
 * This observer:
 * - leave the notification system to DO NOTHING.
 * - implement the traverse() method.
 */
class	CCoarseMeshManagerRenderObs : public CTransformRenderObs
{
public:

	/// render the instance and Don't traverseSons().
	virtual	void	traverse(IObs *caller);
	
	static IObs	*creator() {return new CCoarseMeshManagerRenderObs;}
};

/**
 * This observer:
 * - return true at isRenderable.
 * - return true at clip.
 */
class	CCoarseMeshClipObs : public CTransformClipObs
{
public:

	/// From CTransformClipObs
	// @{
	virtual	bool	isRenderable() const;
	virtual	bool	clip(IBaseClipObs *caller);
	// @}

	// The creator.
	static IObs	*creator() {return new CCoarseMeshClipObs;}
};

} // NL3D


#endif // NL_COARSE_MESH_MANAGER_H

/* End of coarse_mesh_manager.h */
