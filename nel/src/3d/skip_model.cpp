/** \file skip_model.cpp
 * <File description>
 *
 * $Id: skip_model.cpp,v 1.3 2002/07/08 10:00:09 berenguier Exp $
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

#include "3d/skip_model.h"
#include "3d/hrc_trav.h"


namespace NL3D 
{


// ***************************************************************************
void	CSkipModel::registerBasic()
{
	CMOT::registerModel( SkipModelId, 0, CSkipModel::creator);
	CMOT::registerObs( HrcTravId, SkipModelId, CSkipModelHrcObs::creator );
	CMOT::registerObs( ClipTravId, SkipModelId, CSkipModelClipObs::creator );
	CMOT::registerObs( AnimDetailTravId, SkipModelId, CSkipModelAnimDetailObs::creator );
	CMOT::registerObs( LoadBalancingTravId, SkipModelId, CSkipModelLoadBalancingObs::creator );
	CMOT::registerObs( LightTravId, SkipModelId, CSkipModelLightObs::creator );
	CMOT::registerObs( RenderTravId, SkipModelId, CSkipModelRenderObs::creator );
}


// ***************************************************************************
void	CSkipModelHrcObs::traverse(IObs *caller)
{
	// do nothing.
}
// ***************************************************************************
void	CSkipModelClipObs::traverse(IObs *caller)
{
	// do nothing.
}
// ***************************************************************************
void	CSkipModelAnimDetailObs::traverse(IObs *caller)
{
	// do nothing.
}
// ***************************************************************************
void	CSkipModelLoadBalancingObs::traverse(IObs *caller)
{
	// do nothing.
}
// ***************************************************************************
void	CSkipModelLightObs::traverse(IObs *caller)
{
	// do nothing.
}
// ***************************************************************************
void	CSkipModelRenderObs::traverse(IObs *caller)
{
	// do nothing.
}



} // NL3D
