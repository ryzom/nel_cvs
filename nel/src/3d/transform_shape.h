/** \file transform_shape.h
 * <File description>
 *
 * $Id: transform_shape.h,v 1.11 2002/03/29 13:13:45 berenguier Exp $
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
#include "3d/load_balancing_trav.h"
#include <vector>


namespace NL3D 
{


using NLMISC::CSmartPtr;
using NLMISC::CPlane;


class	CTransformShapeClipObs;
class	CTransformShapeRenderObs;
class	CTransformShapeLoadBalancingObs;


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

#ifdef NL_DEBUG
	std::string NameForDebug; // \todo traptemp
#endif


	/** Get the untransformed AABBox of the mesh. NULL (gtSize()==0) if no mesh.	 
	 */
	virtual void					getAABBox(NLMISC::CAABBox &bbox) const;


	/// \name Load balancing methods
	// @{

	/** get an approximation of the number of triangles this instance want render for a fixed distance.
	  *
	  * \param distance is the distance of the shape from the eye.
	  * \return the approximate number of triangles this instance will render at this distance. This
	  * number can be a float. The function MUST be decreasing or constant with the distance but don't 
	  * have to be continus.
	  */
	virtual float			getNumTriangles (float distance);

	/** get an approximation of the number of triangles this instance should render.
	 * This method is valid only for IShape classes (in render()), after LoadBalancing traversal is performed.
	 * NB: It is not guaranted that this instance will render those number of triangles.
	 */
	float					getNumTrianglesAfterLoadBalancing() {return _NumTrianglesAfterLoadBalancing;}

	// @}


	/// Derived from CTranform. By default a shape is considered big for lightable if it uses localAttenuation
	virtual bool		isBigLightable() const;


protected:
	/// Constructor
	CTransformShape() {_NumTrianglesAfterLoadBalancing= 100;}
	/// Destructor
	virtual ~CTransformShape() {}

private:
	static IModel	*creator() {return new CTransformShape;}
	friend class	CTransformShapeClipObs;
	friend class	CTransformShapeRenderObs;
	friend class	CTransformShapeLoadBalancingObs;

	float			_NumTrianglesAfterLoadBalancing;


	// return the contribution of lights (for redner Observer).
	CLightContribution	&getLightContribution() {return _LightContribution;}

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
class	CTransformShapeRenderObs : public CTransformRenderObs
{
public:

	/// setup lighting if lightable, render the instance and Don't traverseSons().
	virtual	void	traverse(IObs *caller);
	
	static IObs	*creator() {return new CTransformShapeRenderObs;}
};


// ***************************************************************************
/**
 * This observer:
 * - leave the notification system to DO NOTHING.
 * - implement the traverse method.
 *
 * \sa CHrcTrav IBaseHrcObs
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	CTransformShapeLoadBalancingObs : public IBaseLoadBalancingObs
{
public:

	/** this do all the good things:
	 *	- LoadBalancing: get the position of the transform (or the skeleton), and use it as center.
	 *	- traverseSons().
	 */
	virtual	void	traverse(IObs *caller);

	static IObs	*creator() {return new CTransformShapeLoadBalancingObs;}


protected:

	// The distance of the model to the camera computed in Pass0.
	float		_ModelDist;

	// The number of face computed in Pass0.
	float		_FaceCount;


	void		traversePass0();
	void		traversePass1();

};


} // NL3D


#endif // NL_TRANSFORM_SHAPE_H

/* End of transform_shape.h */
