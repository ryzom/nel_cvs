/* render_trav.h
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: render_trav.h,v 1.2 2000/10/10 16:12:04 berenguier Exp $
 *
 * <Replace this by a description of the file>
 */

#ifndef NL_RENDER_TRAV_H
#define NL_RENDER_TRAV_H

#include "nel/3d/trav_scene.h"
#include <vector>


namespace	NL3D
{

using namespace NLMISC;

class	IBaseRenderObs;
class	IBaseHrcObs;
class	IBaseClipObs;
class	IBaseLightObs;


// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		RenderTravId= NLMISC::CClassId(0x572456ee, 0x3db55f23);


// ***************************************************************************
/**
 * The Render traversal.
 * The purpose of this traversal is to render a list of models. This traversals is tightly linked to the cliptraversal.
 * The clipTraversals insert directly the observers with CRenderTrav::addRenderObs(obs). The traverse() method should 
 * render all the render observer with IDriver.
 *
 * This traversal is an exception since it is not designed to have a graph of observers. But this behavior is permitted,
 * and the root (if any) is traversed.
 *
 * \b USER \b RULES: Before using traverse() on a render traversal, you should:
 *	- setFrustum() the camera shape (focale....)
 *	- setCamMatrix() for the camera transform
 *
 * NB: see CScene for 3d conventions (orthonormal basis...)
 *
 * \sa CScene IBaseRenderObs
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CRenderTrav : public ITravCameraScene
{
public:

	/// Constructor
	CRenderTrav();

	/// \name ITrav/ITravScene Implementation.
	//@{
	IObs				*createDefaultObs() const;
	NLMISC::CClassId	getClassId() const {return RenderTravId;}
	sint				getRenderOrder() const {return 4000;}
	/** First traverse the root (if any), then render the render list.
	 * \warning If an observer exist both in the graph and in the render list, it will be effectively traversed twice.
	 */
	void				traverse();
	//@}

	/// \name RenderList.
	//@{
	/// Clear the list of rendered observers.
	void	clearRenderList();
	/// Add an observer to the list of rendered observers. \b DOESN'T \b CHECK if already inserted.
	void	addRenderObs(IBaseRenderObs *o);
	//@}


private:
	// A grow only list of observers to be rendered.
	// We don't use a ZList, since it may unusefull if we sort primitives by material.
	std::vector<IBaseRenderObs*>	RenderList;

};


// ***************************************************************************
/**
 * The base interface for render observers.
 * Render observers MUST derive from IBaseRenderObs.
 * This observer:
 * - leave the notification system to DO NOTHING.
 * - leave traverse() to undefined.
 * - implement the init() method, to set shortcut to neighboor observers.
 *
 * \b DERIVER \b RULES:
 * - implement the notification system (see IObs and IObs() for details).
 * - implement the traverse(). See traverse() for more information. 
 * - possibly (but obviously not) modify/extend the graph methods (such as a graph behavior).
 *
 * \sa CRenderTrav
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class IBaseRenderObs : public IObs
{
public:
	/// Shortcut to observers.
	IBaseHrcObs		*HrcObs;
	IBaseClipObs	*ClipObs;
	IBaseLightObs	*LightObs;

public:

	/// Constructor.
	IBaseRenderObs()
	{
		HrcObs=NULL;
		ClipObs= NULL;
		LightObs= NULL;
	}
	/// Build shortcut to Hrc, Clip and Light.
	virtual	void	init();


	/** The base render method.
	 * The observers should update().
	 * The observers should not traverseSons(), for speed improvement.
	 */
	virtual	void	traverse(IObs *caller)=0;

};


// ***************************************************************************
/**
 * The default render observer, used by unspecified models.
 * This observer:
 * - leave the notification system to DO NOTHING.
 * - define the traverse() method to DO NOTHING (but update())..
 *
 * \sa IBaseRenderObs
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CDefaultRenderObs : public IBaseRenderObs
{
public:


	/// Do nothing, and don't traverseSons() too.
	virtual	void	traverse(IObs *caller)
	{
		update();
	}
};


// For Visual 6.0 namespace bug.
namespace MSC_FAKE
{
using namespace	NLMISC;
}


}


#endif // NL_RENDER_TRAV_H

/* End of render_trav.h */
