/** \file shape.h
 * <File description>
 *
 * $Id: shape.h,v 1.3 2000/12/13 10:25:22 berenguier Exp $
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

#ifndef NL_SHAPE_H
#define NL_SHAPE_H


#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/matrix.h"
#include "nel/misc/stream.h"


namespace NL3D 
{


using NLMISC::CPlane;

class	CTransformShape;
class	IDriver;
class	CScene;


// ***************************************************************************
/**
 * The basic interface for shapes. A shape is a kind of instanciable mesh.
 * For simplicity, render() and clip() virtual method are provided, so majority of shape could be implemented by just
 * define those methods, and let createInstance() as default. But other complex shapes may be defined, by implement 
 * a compatible model which will comunicate with them.
 *
 * Serialisation of a shape MUST be done with ISTREAM::serialPolyPtr.
 *
 * \b DERIVER \b RULES:
 *		- simple: just implement clip() and render(). The shape will be movable via CTransform.
 *		- complex: if special interaction is needed between the instance and the shape:
 *			- implement a special Model, derived from CTransformShape, adding your instance behavior.
 *			- implement YourShape::createInstance(), so it create this good model.
 *			- implement your own communication system between the model and the shape.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class IShape : public NLMISC::CRefCount, public NLMISC::IStreamable
{
public:

	/// Constructor
	IShape() {}
	/// Dtor.
	virtual ~IShape() {}

	/** create an instance of this shape. The instance may be a CTransformShape, or a specialized version of it.
	 * The default behavior is to createModel() a CTransformShape, and just assign to it the Shape.
	 * \param scene the scene used to createModel().
	 * \return the specialized instance for this shape.
	 */
	virtual	CTransformShape		*createInstance(CScene &scene);

	/** clip this shape with a pyramid.
	 * the pyramid is given in object space so the shape do not need to know the matrix of the object.
	 * \param pyramid the clipping polytope, planes MUST be normalized.
	 * \return true if the object is visible, false otherwise. The default behavior is to return true (never clipped).
	 */
	virtual bool				clip(const std::vector<CPlane>	&pyramid) {return true;}

	/** render() this shape in a driver.
	 * CTransfromShape call this method in the render traversal.
	 */
	virtual void				render(IDriver *drv)=0;

};


} // NL3D


#endif // NL_SHAPE_H

/* End of shape.h */
