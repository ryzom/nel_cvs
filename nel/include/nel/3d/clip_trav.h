/* clip_trav.h
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: clip_trav.h,v 1.2 2000/10/10 16:12:04 berenguier Exp $
 *
 * <Replace this by a description of the file>
 */

#ifndef NL_CLIP_TRAV_H
#define NL_CLIP_TRAV_H

#include "nel/3d/trav_scene.h"
#include "nel/misc/vector.h"
#include "nel/misc/plane.h"
#include "nel/misc/matrix.h"


namespace	NL3D
{

using namespace	NLMISC;

class	IBaseHrcObs;
class	IBaseRenderObs;
class	CRenderTrav;


// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		ClipTravId=NLMISC::CClassId(0x135208fe, 0x225334fc);


// ***************************************************************************
/**
 * The clip traversal.
 * The purpose of this traversal is to insert in the RenderTraversal the observers which are 
 * said to be not clipped. Some observers may do something else.
 *
 * Observer should use the IBaseHrcObs->clip() method to implement their observer, or completly redefine the traverse() method.
 *
 * \b USER \b RULES: Before using traverse() on a clip traversal, you should:
 *	- setFrustum() the camera shape (focale....)
 *	- setCamMatrix() for the camera transform
 *	- setRenderTrav(), to setup the rendertraversal where observers will be cleared / inserted.
 *
 * NB: see CScene for 3d conventions (orthonormal basis...)
 * \sa CScene IBaseClipObs
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CClipTrav : public ITravCameraScene
{
public:

	/// Constructor
	CClipTrav();

	/// \name ITrav/ITravScene Implementation.
	//@{
	IObs				*createDefaultObs() const;
	NLMISC::CClassId	getClassId() const {return ClipTravId;}
	sint				getRenderOrder() const {return 2000;}
	void				traverse();
	//@}


	/// Setup the render traversal (else traverse() won't work)
	void		setRenderTrav(CRenderTrav	*trav);

public:

	/** \name FOR OBSERVERS ONLY.  (Read only)
	 * Those variables are valid only in traverse().
	 */
	//@{
	/// Vision Pyramid (6 normalized planes) in the view basis.
	std::vector<CPlane>	ViewPyramid;
	/// Vision Pyramid (6 normalized planes) in the world basis.
	std::vector<CPlane>	WorldPyramid;	
	/// Shortcut to the Rdr Traversals (to add the models rdr observers).
	CRenderTrav		*RenderTrav;
	//@}

};


// ***************************************************************************
/**
 * The base interface for clip traversal.
 * Clip observers MUST derive from IBaseClipObs.
 * This observer:
 * - leave the notification system to DO NOTHING.
 * - implement a default traverse() method. See traverse() for more information.
 * - implement the init() method, to set shortcut to neighboor observers.
 *
 * \b DERIVER \b RULES:
 * - implement the notification system (see IObs and IObs() for details).
 * - implement the clip() method.
 * - Possibly re-implement the traverse(). See traverse() for more information. 
 * - possibly modify/extend the graph methods (such as a graph behavior).
 *
 * \sa CClipTrav
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class IBaseClipObs : public IObs
{
public:
	/// Shortcut to observers.
	IBaseHrcObs		*HrcObs;
	IBaseRenderObs	*RenderObs;

	/** OUT variable (good after traverse()).
	 * set to true is the object is visible (not clipped).
	 */
	bool			Visible;

public:
	

	/// Constructor.
	IBaseClipObs()
	{
		HrcObs=NULL;
		RenderObs= NULL;
		Visible=true;
	}
	/// Build shortcut to HrcObs and RenderObs.
	virtual	void	init();


	/** Should return true if object is visible (eg in frustum)
	 * \param caller the caller obs (may NULL)
	 * \param renderable clip() should set to true if the observer has to be inserted in the renderlist (if visible).
	 *	eg: a mesh must be inserted in a render list, but not a light, or a NULL transform.
	 */
	virtual	bool	clip(IBaseClipObs *caller, bool &renderable)=0;


	/** The base doit method.
	 * The default behavior is to:
	 *	- update().
	 *	- test if HrcObs->WorldVis is visible.
	 *	- test if the observer is clipped with clip()
	 *	- if visible and not clipped, set \c Visible=true (else false).
	 *	- if visible, not clipped, and renderable, add it to the RenderTraversal: \c RenderTrav->addRenderObs(RenderObs);
	 *	- if visible and not clipped, traverseSons(), to clip the sons.
	 */
	virtual	void	traverse(IObs *caller);


};


// ***************************************************************************
/**
 * The default clip observer, used by unspecified models.
 * This observer:
 * - leave the notification system to DO NOTHING.
 * - implement the clip() method to return true.
 * - leave the traverse() method as IBaseClipObs.
 *
 * \sa IBaseClipObs
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CDefaultClipObs : public IBaseClipObs
{
public:


	/// Don't clip, but don't render.
	virtual	bool	clip(IBaseClipObs *caller, bool &renderable) {renderable= false; return true;}
};



}


// For Visual 6.0 namespace bug.
namespace MSC_FAKE
{
using namespace	NLMISC;
}


#endif // NL_CLIP_TRAV_H

/* End of clip_trav.h */
