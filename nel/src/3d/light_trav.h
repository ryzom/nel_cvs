/** \file light_trav.h
 * <File description>
 *
 * $Id: light_trav.h,v 1.4 2002/06/28 14:21:29 berenguier Exp $
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

#include "3d/trav_scene.h"
#include "3d/lighting_manager.h"


namespace	NL3D
{


using NLMISC::CVector;
using NLMISC::CPlane;
using NLMISC::CMatrix;

	
class	IBaseLightObs;
class	CTransform;
class	CRootModel;


// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		LightTravId= NLMISC::CClassId(0x328f500a, 0x57600db9);


// ***************************************************************************
/**
 * The light traversal.
 * The purpose of this traversal is to compute lighting on lightable objects.
 *
 * Lightable objects can be CTransform only.
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

	/// ITrav/ITravScene Implementation.
	//@{
	IObs				*createDefaultObs() const;
	NLMISC::CClassId	getClassId() const {return LightTravId;}
	sint				getRenderOrder() const {return 3000;}
	void				traverse();
	//@}


	// False by default. setuped by CScene
	bool						LightingSystemEnabled;


	/// \name LightingList. Filled during clip traversal.
	//@{
	/// Clear the list of lighted observers.
	void			clearLightedList();
	/// Add an observer to the list of lighted observers. \b DOESN'T \b CHECK if already inserted.
	void			addLightedObs(IBaseLightObs *o);
	//@}


	/// The lightingManager, where objects/lights are inserted, and modelContributions are computed
	CLightingManager	LightingManager;


	/// Must init it!
	void	setLightModelRoot(CRootModel *lightModelRoot);


// ********************
public:

	/// \name FOR OBSERVERS ONLY.  (Read only)
	// @{
	// The root of light models.
	CRootModel						*LightModelRoot;
	// @}

private:

	// A grow only list of observers to be lighted.
	std::vector<IBaseLightObs*>		_LightedList;

};



}


#endif // NL_LIGHT_TRAV_H

/* End of light_trav.h */
