/** \file transform_shape.h
 * <File description>
 *
 * $Id: transform_shape.h,v 1.2 2001/06/27 15:23:53 corvazier Exp $
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

#ifndef NL_TRANSFORM_SHAPE_H
#define NL_TRANSFORM_SHAPE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/matrix.h"
#include "3d/transform.h"
#include "3d/shape.h"
#include "3d/render_trav.h"
#include <vector>


namespace NL3D 
{


using NLMISC::CSmartPtr;
using NLMISC::CPlane;


class	CTransformShapeClipObs;
class	CTransformShapeRenderObs;


// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		TransformShapeId=NLMISC::CClassId(0x1e6115e6, 0x63502517);


// ***************************************************************************
/**
 * A transform which "is an instance of"/"point to" a IShape.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CTransformShape : public CTransform
{
public:
	/// Call at the begining of the program, to register the model, and the basic observers.
	static	void	registerBasic();

public:
	/// The shape, the object instancied.
	CSmartPtr<IShape>		Shape;

	/// \name Load balancing methods
	// @{

	/** get an approximation of the number of triangles this instance will render for a fixed distance.
	  *
	  * \param distance is the distance of the shape from the eye.
	  * \return the approximate number of triangles this instance will render at this distance. This
	  * number can be a float. The function MUST be decreasing or constant with the distance but don't 
	  * have to be continus.
	  */
	virtual float			getNumTriangles (float distance);

	// @}

protected:
	/// Constructor
	CTransformShape() {}
	/// Destructor
	virtual ~CTransformShape() {}

private:
	static IModel	*creator() {return new CTransformShape;}
	friend class	CTransformShapeClipObs;
	friend class	CTransformShapeRdrObs;
};



// ***************************************************************************
// ***************************************************************************
// Observers implementation.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
/**
 * This observer:
 * - leave the notification system to DO NOTHING.
 * - implement the clip() method to return Shape->clip(), and renderable==true.
 * - leave the traverse() method as CTransformClipObs.
 *
 * \sa CClipTrav CTransformClipObs
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	CTransformShapeClipObs : public CTransformClipObs
{
public:

	/// clip the shape, and set renderable.
	virtual	bool	clip(IBaseClipObs *caller);
	virtual	bool	isRenderable() const {return true;}


	static IObs	*creator() {return new CTransformShapeClipObs;}
};


// ***************************************************************************
/**
 * This observer:
 * - leave the notification system to DO NOTHING.
 * - implement the traverse() method.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	CTransformShapeRenderObs : public IBaseRenderObs
{
public:

	/// render the instance and Don't traverseSons().
	virtual	void	traverse(IObs *caller);
	
	static IObs	*creator() {return new CTransformShapeRenderObs;}
};


} // NL3D


#endif // NL_TRANSFORM_SHAPE_H

/* End of transform_shape.h */
