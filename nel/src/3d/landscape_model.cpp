/** \file landscape_model.cpp
 * <File description>
 *
 * $Id: landscape_model.cpp,v 1.3 2000/12/06 14:32:39 berenguier Exp $
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

#include "nel/3d/landscape_model.h"
#include "nel/3d/landscape.h"
#include <vector>
using namespace std;
using namespace NLMISC;


namespace NL3D 
{



// ***************************************************************************
void	CLandscapeModel::registerBasic()
{
	CMOT::registerModel(LandscapeModelId, TransformId, CLandscapeModel::creator);
	CMOT::registerObs(ClipTravId, LandscapeModelId, CLandscapeClipObs::creator);
	CMOT::registerObs(RenderTravId, LandscapeModelId, CLandscapeRenderObs::creator);
}

// ***************************************************************************
bool	CLandscapeClipObs::clip(IBaseClipObs *caller, bool &renderable)
{
	renderable= true;
	
	CLandscapeModel		*landModel= (CLandscapeModel*)Model;
	CClipTrav		*trav= (CClipTrav*)Trav;
	vector<CPlane>	&pyramid= trav->WorldPyramid;
	// We are sure that pyramid has normalized plane normals.
	landModel->Landscape.clip(trav->CamPos, pyramid);

	// Well, always visible....
	return true;
}

// ***************************************************************************
void	CLandscapeRenderObs::traverse(IObs *caller)
{
	CLandscapeModel		*landModel= (CLandscapeModel*)Model;

	CRenderTrav		*trav= (CRenderTrav*)Trav;

	CMatrix		m;
	m.identity();
	trav->getDriver()->setupModelMatrix(m);

	// First, refine.
	landModel->Landscape.refine(trav->CamPos);
	// then render.
	landModel->Landscape.render(trav->getDriver(), trav->CamPos);
}



} // NL3D
