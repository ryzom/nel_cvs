/** \file base_clip_obs.h
 * <File description>
 *
 * $Id: base_clip_obs.h,v 1.2 2003/03/20 15:01:24 berenguier Exp $
 */

/* Copyright, 2000-2002 Nevrax Ltd.
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

#ifndef NL_BASE_CLIP_OBS_H
#define NL_BASE_CLIP_OBS_H


#include "3d/mot.h"


namespace NL3D 
{


class	IBaseHrcObs;
class	IBaseRenderObs;
class	IBaseAnimDetailObs;
class	IBaseLoadBalancingObs;


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
 * - implement the traverse(), which should call clip(). see CTransform for an implementation.
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
	IBaseAnimDetailObs		*AnimDetailObs;
	IBaseLoadBalancingObs	*LoadBalancingObs;

	/** OUT variable (good after traverse()).
	 * set to true is the object is visible (not clipped).
	 */
	bool			Visible;

	enum TClipReason	{FrustumClip=0, DistMaxClip, countClip};

public:
	

	/// Constructor.
	IBaseClipObs()
	{
		HrcObs=NULL;
		RenderObs= NULL;
		Visible=false;
		_IndexInVisibleList= -1;
	}
	// Dtor: remove me from _VisibleList.
	virtual	~IBaseClipObs();

	/// Build shortcut to HrcObs and RenderObs.
	virtual	void	init();


	/** Should return true if object is visible (eg in frustum)
	 * \param caller the caller obs (may NULL)
	 */
	virtual	bool	clip(IBaseClipObs *caller)=0;


	/** The base doit method.
	 * The default behavior should be:
	 *	- test if HrcObs->WorldVis is visible.
	 *	- test if the observer is clipped with clip()
	 *	- if visible and not clipped, set \c Visible=true (else false).
	 *	- if Visible==true, add it to the post-clip traversals which need it (if renderable, animDetailable etc...)
	 *	- always traverseSons(), to clip the sons.
	 */
	virtual	void	traverse(IObs *caller) =0;


	/** 
	 * Because the clip traversal is a graph of observer not a hierarchy
	 */
	virtual bool	isTreeNode() {return false;}

private:
	friend class	CClipTrav;

	// The index of the Observer in the _VisibleList; -1 (default) means not in
	sint			_IndexInVisibleList;
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

	/// Don't clip.
	virtual	bool	clip(IBaseClipObs *caller) {return true;}


	/// just traverseSons().
	virtual	void	traverse(IObs *caller)
	{
		traverseSons();
	}

};


} // NL3D


#endif // NL_BASE_CLIP_OBS_H

/* End of base_clip_obs.h */
