/** \file base_anim_detail_obs.h
 * <File description>
 *
 * $Id: base_anim_detail_obs.h,v 1.1 2002/06/28 14:21:29 berenguier Exp $
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

#ifndef NL_BASE_ANIM_DETAIL_OBS_H
#define NL_BASE_ANIM_DETAIL_OBS_H


#include "3d/mot.h"


namespace NL3D 
{

class	IBaseHrcObs;
class	IBaseClipObs;

// ***************************************************************************
/**
 * The base interface for AnimDetail traversal.
 * AnimDetail observers MUST derive from IBaseAnimDetailObs.
 * This observer:
 * - leave the notification system to DO NOTHING.
 * - leave traverse() undefined
 *
 * \b DERIVER \b RULES:
 * - implement the notification system (see IObs for details).
 * - implement the traverse() method.
 *
 * \sa CAnimDetailTrav
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class IBaseAnimDetailObs : public IObs
{
public:
	/// Shortcut to observers.
	IBaseHrcObs		*HrcObs;
	IBaseClipObs	*ClipObs;

public:

	/// Constructor.
	IBaseAnimDetailObs()
	{
		HrcObs=NULL;
		ClipObs= NULL;
	}
	/// Build shortcut to Hrc and Clip.
	virtual	void	init();


	/// \name The base doit method.
	//@{
	virtual	void	traverse(IObs *caller) =0;
	//@}


};


// ***************************************************************************
/**
 * The default AnimDetail observer, used by unspecified models.
 * This observer:
 * - leave the notification system to DO NOTHING.
 * - implement the traverse() method to DO NOTHING.
 *
 * \sa IBaseAnimDetailObs
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CDefaultAnimDetailObs : public IBaseAnimDetailObs
{
public:


	/// Constructor.
	CDefaultAnimDetailObs() {}

	/// \name The base doit method.
	//@{
	virtual	void	traverse(IObs *caller)
	{
		// no need to traverseSons. No graph here.
	}
	//@}

};




} // NL3D


#endif // NL_BASE_ANIM_DETAIL_OBS_H

/* End of base_anim_detail_obs.h */
