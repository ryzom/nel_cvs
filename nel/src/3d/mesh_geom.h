/** \file mesh_geom.h
 * <File description>
 *
 * $Id: mesh_geom.h,v 1.3 2001/07/05 09:38:49 besson Exp $
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

#ifndef NL_MESH_GEOM_H
#define NL_MESH_GEOM_H


#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include "nel/misc/plane.h"


namespace NL3D 
{


class IDriver;
class CTransformShape;
using NLMISC::CPlane;


// ***************************************************************************
/**
 * Interface for MeshGeom.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class IMeshGeom : public NLMISC::IStreamable
{
public:

	/// Constructor
	IMeshGeom() {}
	/// dtor
	virtual ~IMeshGeom() {}


	/** clip this meshGeom with a pyramid.
	 * the pyramid is given in object space so the shape do not need to know the matrix of the object.
	 * \param pyramid the clipping polytope, planes MUST be normalized.
	 * \return true if the object is visible, false otherwise. The default behavior is to return true (never clipped).
	 */
	virtual bool	clip(const std::vector<CPlane>	&pyramid) {return true;}

	/** render() this meshGeom in a driver, with the specified TransformShape instance information.
	 */
	virtual void	render(IDriver *drv, CTransformShape *trans, bool passOpaque) =0;

	/// \name Load balancing methods
	// @{

	/** get an approximation of the number of triangles this instance will render for a fixed distance.
	  *
	  * \param distance is the distance of the shape from the eye.
	  * \return the approximate number of triangles this instance will render at this distance. This
	  * number can be a float. The function MUST be decreasing or constant with the distance but don't 
	  * have to be continus.
	  */
	virtual float	getNumTriangles (float distance) =0;

	// @}
};


} // NL3D


#endif // NL_MESH_GEOM_H

/* End of mesh_geom.h */
