/** \file clip_trav.h
 * <File description>
 *
 * $Id: clip_trav.h,v 1.1 2001/06/15 16:24:42 corvazier Exp $
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

#ifndef NL_CLIP_TRAV_H
#define NL_CLIP_TRAV_H

#include "3d/trav_scene.h"
#include "nel/misc/vector.h"
#include "nel/misc/plane.h"
#include "nel/misc/matrix.h"


namespace	NL3D
{

using NLMISC::CVector;
using NLMISC::CPlane;
using NLMISC::CMatrix;


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
 * - implement the init() method, to set shortcut to neighbor observers.
 *
 * \b DERIVER \b RULES:
 * - implement the notification system (see IObs and IObs() for details).
 * - implement the clip() method.
 * - implement the traverse(), which should call clip() and isRenderable(). see CTransform for an implementation.
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



	/** Should return true if object has to be inserted in RenderTrav list.
	 *	eg: a mesh must be inserted in a render list, but not a light, or a NULL transform.
	 */
	virtual	bool	isRenderable() const =0;


	/** Should return true if object is visible (eg in frustum)
	 * \param caller the caller obs (may NULL)
	 */
	virtual	bool	clip(IBaseClipObs *caller)=0;


	/** The base doit method.
	 * The default behavior should be:
	 *	- test if HrcObs->WorldVis is visible.
	 *	- test if the observer is clipped with clip()
	 *	- if visible and not clipped, set \c Visible=true (else false).
	 *	- if Visible==true, and renderable, add it to the RenderTraversal: \c RenderTrav->addRenderObs(RenderObs);
	 *	- always traverseSons(), to clip the sons.
	 */
	virtual	void	traverse(IObs *caller) =0;


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

	/// don't render.
	virtual	bool	isRenderable() const {return false;}

	/// Don't clip.
	virtual	bool	clip(IBaseClipObs *caller) {return true;}


	/// just traverseSons().
	virtual	void	traverse(IObs *caller)
	{
		traverseSons();
	}

};



}


#endif // NL_CLIP_TRAV_H

/* End of clip_trav.h */
