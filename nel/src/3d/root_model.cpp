/** \file root_model.cpp
 * <File description>
 *
 * $Id: root_model.cpp,v 1.2 2002/02/28 12:59:51 besson Exp $
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

#include "std3d.h"

#include "3d/root_model.h"


namespace NL3D {


// ***************************************************************************
void	CRootModel::registerBasic()
{
	CMOT::registerModel( RootModelId, 0, CRootModel::creator);
	CMOT::registerObs( HrcTravId, RootModelId, CRootModelHrcObs::creator );
	CMOT::registerObs( ClipTravId, RootModelId, CRootModelClipObs::creator );
	CMOT::registerObs( AnimDetailTravId, RootModelId, CRootModelAnimDetailObs::creator );
	CMOT::registerObs( LoadBalancingTravId, RootModelId, CRootModelLoadBalancingObs::creator );
	CMOT::registerObs( LightTravId, RootModelId, CRootModelLightObs::creator );
	CMOT::registerObs( RenderTravId, RootModelId, CRootModelRenderObs::creator );
}


// ***************************************************************************
IObs			*CRootModel::getObs(const NLMISC::CClassId &idTrav)
{
	return IModel::getObs(idTrav);
}


// ***************************************************************************
void	CRootModelHrcObs::traverse(IObs *caller)
{
	traverseSons();
}
// ***************************************************************************
void	CRootModelClipObs::traverse(IObs *caller)
{
	traverseSons();
}
// ***************************************************************************
void	CRootModelAnimDetailObs::traverse(IObs *caller)
{
	traverseSons();
}
// ***************************************************************************
void	CRootModelLoadBalancingObs::traverse(IObs *caller)
{
	traverseSons();
}
// ***************************************************************************
void	CRootModelLightObs::traverse(IObs *caller)
{
	traverseSons();
}
// ***************************************************************************
void	CRootModelRenderObs::traverse(IObs *caller)
{
	traverseSons();
}


} // NL3D
