/* render_trav.cpp
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: render_trav.cpp,v 1.1 2000/10/06 16:43:58 berenguier Exp $
 *
 * <Replace this by a description of the file>
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
	for(sint i=0;i<RenderList.size();i++)
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
	assert( dynamic_cast<IBaseHrcObs*> (getObs(HrcTravId)) );
	HrcObs= static_cast<IBaseHrcObs*> (getObs(HrcTravId));
	assert( dynamic_cast<IBaseClipObs*> (getObs(ClipTravId)) );
	ClipObs= static_cast<IBaseClipObs*> (getObs(ClipTravId));
	assert( dynamic_cast<IBaseLightObs*> (getObs(LightTravId)) );
	LightObs= static_cast<IBaseLightObs*> (getObs(LightTravId));
}


}