/** \file shadow_poly_receiver.h
 * <File description>
 *
 * $Id: shadow_poly_receiver.h,v 1.4 2004/03/19 10:11:36 corvazier Exp $
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

#ifndef NL_SHADOW_POLY_RECEIVER_H
#define NL_SHADOW_POLY_RECEIVER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/triangle.h"
#include "3d/quad_grid.h"
#include "3d/vertex_buffer.h"
#include "3d/index_buffer.h"


namespace NL3D {

class	IDriver;
class	CMaterial;
class	CShadowMap;


// ***************************************************************************
#define		NL3D_SPR_NUM_CLIP_PLANE			7
#define		NL3D_SPR_NUM_CLIP_PLANE_SHIFT	(1<<NL3D_SPR_NUM_CLIP_PLANE)
#define		NL3D_SPR_NUM_CLIP_PLANE_MASK	(NL3D_SPR_NUM_CLIP_PLANE_SHIFT-1)
#define		NL3D_SPR_MAX_REF_COUNT			255


// ***************************************************************************
/**
 *  A class used to append/remove triangles that will be rendered for ShadowMap
 *	Additionally it can be used also for Camera collision for instance.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2003
 */
class CShadowPolyReceiver
{
public:

	/// Constructor
	CShadowPolyReceiver(uint quadGridSize=32, float quadGridCellSize= 4.f);

	/// Append a triangle to the poly receiver.
	uint			addTriangle(const NLMISC::CTriangle &tri);
	/// remove a triangle from the poly receiver.
	void			removeTriangle(uint id);

	/** clip, and render with a shadow map. Matrix setup should be OK.
	 *	\param vertDelta (for landscape). add this value from vertices before rendering.
	 */
	void			render(IDriver *drv, CMaterial &shadowMat, const CShadowMap *shadowMap, const CVector &casterPos, const CVector &vertDelta);

	/** Use the triangles added for camera 3rd person collision
	 *	return a [0,1] value. 0 => collision at start. 1 => no collision.
	 *	\param radius is the radius of the 'cylinder'
	 *	\param cone if true, the object tested is a cone (radius goes to end)
	*/
	float			getCameraCollision(const CVector &start, const CVector &end, float radius, bool cone);

// ************
private:

	// Vertices.
	class	CVectorId : public CVector
	{
	public:
		uint8			RefCount;
		uint8			Flags;
		sint16			VBIdx;

		CVectorId() {RefCount=0;}
		CVectorId(const CVector &v) {(*(CVector*)this)= v; RefCount=0;}
	};
	std::vector<CVectorId>				_Vertices;
	std::vector<uint>					_FreeVertices;
	typedef	std::map<CVector, uint>		TVertexMap;
	TVertexMap							_VertexMap;

	// Triangles
	struct	CTriangleId
	{
		uint	Vertex[3];
	};
	typedef CQuadGrid<CTriangleId>			TTriangleGrid;
	TTriangleGrid							_TriangleGrid;
	std::vector<TTriangleGrid::CIterator>	_Triangles;
	std::vector<uint>						_FreeTriangles;

	// Render
	// TODO_SHADOW: optim: VBHard.
	CVertexBuffer						_VB;
	CIndexBuffer						_RenderTriangles;


	// Vertex Mgt.
	// Allocate a vertex. RefCount init to 0. _VertexMap modified.
	uint				allocateVertex(const CVector &v);
	// Release a vertex, freeing him if no more unused => _VertexMap modified.
	void				releaseVertex(uint id);
	// increment the Count of the ith vertex;
	void				incVertexRefCount(uint id);
};


} // NL3D


#endif // NL_SHADOW_POLY_RECEIVER_H

/* End of shadow_poly_receiver.h */
