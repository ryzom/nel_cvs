/** \file transform_shape.cpp
 * <File description>
 *
 * $Id: transform_shape.cpp,v 1.1 2000/12/08 10:36:52 berenguier Exp $
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

#include "nel/3d/transform_shape.h"
#include "nel/3d/driver.h"


namespace NL3D 
{


// ***************************************************************************
void		CTransformShape::registerBasic()
{
	CMOT::registerModel(TransformShapeId, TransformId, CTransformShape::creator);
	CMOT::registerObs(ClipTravId, TransformShapeId, CTransformShapeClipObs::creator);
	CMOT::registerObs(RenderTravId, TransformShapeId, CTransformShapeRenderObs::creator);
}

// ***************************************************************************
bool	CTransformShapeClipObs::clip(IBaseClipObs *caller, bool &renderable) 
{
	CClipTrav			*trav= (CClipTrav*)Trav;
	CTransformShape		*m= (CTransformShape*)Model;

	renderable= true; 
	if(m->Shape)
	{
		std::vector<CPlane>	pyramid= trav->WorldPyramid;
		// Transform the pyramid in Object space.
		CMatrix		&mat= HrcObs->WorldMatrix;
		for(sint i=0;i<(sint)pyramid.size();i++)
		{
			pyramid[i]= pyramid[i]*mat;
		}
		return m->Shape->clip(pyramid);
	}
	else
		return false;
}


// ***************************************************************************
void	CTransformShapeRenderObs::traverse(IObs *caller)
{
	// Must update() the observer.
	update();

	IBaseRenderObs::traverse(caller);

	CRenderTrav			*trav= (CRenderTrav*)Trav;
	CTransformShape		*m= (CTransformShape*)Model;
	IDriver				*drv= trav->getDriver();

	// Setup the matrix.
	drv->setupModelMatrix(HrcObs->WorldMatrix);

	// render the shape.
	if(m->Shape)
		m->Shape->render(drv);
}

} // NL3D
