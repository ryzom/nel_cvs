/* clip_trav.cpp
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: clip_trav.cpp,v 1.2 2000/10/09 14:53:44 lecroart Exp $
 *
 * <Replace this by a description of the file>
 */

#include "nel/3d/clip_trav.h"
#include "nel/3d/hrc_trav.h"
#include "nel/3d/render_trav.h"
using namespace std;
using namespace NLMISC;


namespace	NL3D
{


// ***************************************************************************
CClipTrav::CClipTrav() : ViewPyramid(6), WorldPyramid(6)
{
	RenderTrav= NULL;
}

// ***************************************************************************
IObs				*CClipTrav::createDefaultObs() const
{
	return new CDefaultClipObs;
}


// ***************************************************************************
void				CClipTrav::traverse()
{

	// Update Clip infos.
	ITravCameraScene::update();

	// Compute pyramid in view basis.
	CVector		pfoc(0,0,0);
	CVector		lb(Left,  Bottom, Near);
	CVector		lt(Left,  Top,    Near);
	CVector		rb(Right, Bottom, Near);
	CVector		rt(Right, Top,    Near);

	CVector		lbFar(Left,  Bottom, Far);
	CVector		ltFar(Left,  Top,    Far);
	CVector		rtFar(Right, Top,    Far);
	ViewPyramid[0].make(lt, lb, rt);
	ViewPyramid[1].make(pfoc, lt, lb);
	ViewPyramid[2].make(pfoc, rt, lt);
	ViewPyramid[3].make(pfoc, rb, rt);
	ViewPyramid[4].make(pfoc, lb, rb);
	ViewPyramid[5].make(lbFar, ltFar, rtFar);
	
	// Compute pyramid in World basis.
	// The vector transformation M of a plane p is computed as p*M-1.
	// Here, ViewMatrix== CamMatrix-1. Hence the following formula.
	for(sint i=0;i<6;i++)
	{
		WorldPyramid[i]= ViewPyramid[i]*ViewMatrix;
	}


	// Clear the render list.
	RenderTrav->clearRenderList();
	
	// Traverse the graph.
	if(Root)
		Root->traverse(NULL);
}


// ***************************************************************************
void		CClipTrav::setRenderTrav(CRenderTrav	*trav)
{
	RenderTrav= trav;
}


// ***************************************************************************
// ***************************************************************************
// IBaseClipObs
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		IBaseClipObs::init()
{
	IObs::init();
	nlassert( dynamic_cast<IBaseHrcObs*> (getObs(HrcTravId)) );
	HrcObs= static_cast<IBaseHrcObs*> (getObs(HrcTravId));
	nlassert( dynamic_cast<IBaseRenderObs*> (getObs(RenderTravId)) );
	RenderObs= static_cast<IBaseRenderObs*> (getObs(RenderTravId));
}
// ***************************************************************************
void		IBaseClipObs::traverse(IObs *caller)
{
	update();

	nlassert(!caller || dynamic_cast<IBaseClipObs*>(caller));
	bool	renderable;

	Visible= false;
	if(HrcObs->WorldVis && clip(static_cast<IBaseClipObs*>(caller), renderable) )
	{
		Visible= true;

		// Insert the model in the render list.
		if(renderable)
		{
			nlassert(dynamic_cast<CClipTrav*>(Trav));
			static_cast<CClipTrav*>(Trav)->RenderTrav->addRenderObs(RenderObs);
		}
		// DoIt the sons.
		traverseSons();
	}
}


}