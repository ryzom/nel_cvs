/** \file base_render_obs.h
 * Base Observers for CRenderTrav. separated for compilation optimisation
 *
 * $Id: base_render_obs.h,v 1.1 2002/06/28 14:21:29 berenguier Exp $
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

#ifndef NL_BASE_RENDER_OBS_H
#define NL_BASE_RENDER_OBS_H


#include "3d/mot.h"


namespace NL3D
{


class	IBaseHrcObs;
class	IBaseClipObs;
class	IBaseLightObs;


// ***************************************************************************
/**
 * The base interface for render observers.
 * Render observers MUST derive from IBaseRenderObs.
 * This observer:
 * - leave the notification system to DO NOTHING.
 * - leave traverse() to undefined.
 * - implement the init() method, to set shortcut to neighbor observers.
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
	 * The observers should not traverseSons(), for speed improvement.
	 */
	virtual	void	traverse(IObs *caller)=0;
};



// ***************************************************************************
/**
 * The default render observer, used by unspecified models.
 * This observer:
 * - leave the notification system to DO NOTHING.
 * - define the traverse() method to DO NOTHING..
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
	}
};



} // NL3D


#endif // NL_BASE_RENDER_OBS_H

/* End of base_render_obs.h */
