/** \file u_shape.h
 * Game interface for managing shape.
 *
 * $Id: u_shape.h,v 1.2 2004/03/23 15:39:21 berenguier Exp $
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

#ifndef NL_U_SHAPE_H
#define NL_U_SHAPE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include <vector>


namespace NL3D 
{

class	UVisualCollisionMesh;

// ***************************************************************************
/**
 * Game interface for managing shape.
 *	NB: unlike old GameInterface, here the UShape is a Proxy: it keep a ptr on a IShape, thus
 *	it can be created or destroyed statically, on the stack etc...
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class UShape
{
public:

	/// Proxy interface
	UShape();
	/// Attach a IShape to this proxy
	void			attach(class IShape *shape);
	/// return true if the proxy is empty() (not attached)
	bool			empty() const {return _Shape==NULL;}
	/// For Advanced usage, get the Shape ptr
	class IShape	*getShapePtr() const {return _Shape;}


	/** Get the mesh under Triangle Form. 
	 *	For now only CMesh and CMeshMultiLod (CMeshGeom only) are supported.
	 *	Warning! return false if the Mesh has already been rendered!! (VertexBuffer resident)
	 *	\param vertices array of vertices
	 *	\param indices triplets of indices to vertices
	 *	\return false if cannot be converted.
	 */
	bool			getMeshTriangles(std::vector<NLMISC::CVector> &vertices, std::vector<uint32> &indices) const;

	/** Return a Visual Collision Mesh if the shape has one (else return empty one)
	 *	For now return someting if Mesh / MeshMultiLod with some lightmap only (supposed big meshs)!
	 */
	void			getVisualCollisionMesh(UVisualCollisionMesh	&colMesh) const;


private:
	class IShape	*_Shape;
};


} // NL3D


#endif // NL_U_SHAPE_H

/* End of u_shape_bank.h */
