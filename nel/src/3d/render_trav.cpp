/** \file render_trav.cpp
 * <File description>
 *
 * $Id: render_trav.cpp,v 1.4 2000/10/25 13:39:13 lecroart Exp $
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

#include "nel/3d/render_trav.h"
#include "nel/3d/hrc_trav.h"
#include "nel/3d/clip_trav.h"
#include "nel/3d/light_trav.h"
using namespace std;
using namespace NLMISC;


namespace	NL3D
{


// ***************************************************************************
// ***************************************************************************
// CRenderTrav
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CRenderTrav::CRenderTrav()
{
	RenderList.reserve(1024);
}
// ***************************************************************************
IObs		*CRenderTrav::createDefaultObs() const
{
	return new CDefaultRenderObs;
}
// ***************************************************************************
void		CRenderTrav::traverse()
{
	// First traverse the root.
	if(Root)
		Root->traverse(NULL);

	// Then traverse the render list.
	for(sint i=0;i<(sint)RenderList.size();i++)
	{
		RenderList[i]->traverse(NULL);
	}
}
// ***************************************************************************
void		CRenderTrav::clearRenderList()
{
	RenderList.clear();
}
// ***************************************************************************
void		CRenderTrav::addRenderObs(IBaseRenderObs *o)
{
	RenderList.push_back(o);
}



// ***************************************************************************
// ***************************************************************************
// IBaseClipObs
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		IBaseRenderObs::init()
{
	IObs::init();
	nlassert( dynamic_cast<IBaseHrcObs*> (getObs(HrcTravId)) );
	HrcObs= static_cast<IBaseHrcObs*> (getObs(HrcTravId));
	nlassert( dynamic_cast<IBaseClipObs*> (getObs(ClipTravId)) );
	ClipObs= static_cast<IBaseClipObs*> (getObs(ClipTravId));
	nlassert( dynamic_cast<IBaseLightObs*> (getObs(LightTravId)) );
	LightObs= static_cast<IBaseLightObs*> (getObs(LightTravId));
}


}