/** \file light_trav.h
 * <File description>
 *
 * $Id: light_trav.h,v 1.6 2001/03/16 16:48:35 berenguier Exp $
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

#ifndef NL_LIGHT_TRAV_H
#define NL_LIGHT_TRAV_H

#include "nel/3d/trav_scene.h"


namespace	NL3D
{


using NLMISC::CVector;
using NLMISC::CPlane;
using NLMISC::CMatrix;

	
class	IBaseHrcObs;
class	IBaseClipObs;


// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		LightTravId= NLMISC::CClassId(0x328f500a, 0x57600db9);


// ***************************************************************************
/**
 * The light traversal.
 * The purpose of this traversal is to compute lighting on lightable objects.
 *
 * TODO.
 *
 * NB: see CScene for 3d conventions (orthonormal basis...)
 * \sa CScene IBaseLightObs
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CLightTrav : public ITravScene
{
public:

	/// Constructor
	CLightTrav();

	// ITrav/ITravScene Implementation.
	IObs				*createDefaultObs() const;
	NLMISC::CClassId	getClassId() const {return LightTravId;}
	sint				getRenderOrder() const {return 3000;}
	void				traverse()
	{
		if(Root)
			Root->traverse(NULL);
	}

};



// ***************************************************************************
/**
 * The base interface for clip traversal.
 * Clip observers MUST derive from IBaseLightObs.
 * This observer:
 * - leave the notification system to DO NOTHING.
 * - leave the traverse() method as undefined.
 * - implement the init() method, to set shortcut to neighbor observers.
 *
 * \b DERIVER \b RULES:
 * - implement the notification system (see IObs and IObs() for details).
 * - implement the traverse() method.
 * - possibly modify/extend the graph methods (such as a graph behavior).
 *
 * \sa CLightTrav
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class IBaseLightObs : public IObs
{
public:
	/// Shortcut to observers.
	IBaseHrcObs		*HrcObs;
	IBaseClipObs	*ClipObs;
	

public:

	/// Constructor.
	IBaseLightObs()
	{
		HrcObs=NULL;
		ClipObs= NULL;
	}
	/// Build shortcut to HrcObs and RenderObs.
	virtual	void	init();
};


// ***************************************************************************
/**
 * The base interface for clip traversal.
 * Clip observers MUST derive from IBaseLightObs.
 * This observer:
 * - leave the notification system to DO NOTHING.
 * - implement the traverse() method to DO NOTHING (but traverseSons()).
 *
 * \sa CLightTrav
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CDefaultLightObs : public IBaseLightObs
{
public:
	void	traverse(IObs * caller)
	{
		traverseSons();
	}
};


}


#endif // NL_LIGHT_TRAV_H

/* End of light_trav.h */
