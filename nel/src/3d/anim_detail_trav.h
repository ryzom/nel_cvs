/** \file anim_detail_trav.h
 * <File description>
 *
 * $Id: anim_detail_trav.h,v 1.6 2002/06/28 14:21:29 berenguier Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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

#ifndef NL_ANIM_DETAIL_TRAV_H
#define NL_ANIM_DETAIL_TRAV_H

#include "nel/misc/types_nl.h"
#include "nel/misc/matrix.h"
#include "nel/misc/plane.h"
#include "3d/trav_scene.h"


namespace NL3D
{


using NLMISC::CVector;
using NLMISC::CPlane;
using NLMISC::CMatrix;


class	IBaseAnimDetailObs;
class	CClipTrav;
class	CTransformAnimDetailObs;

// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		AnimDetailTravId=NLMISC::CClassId(0x373f6772, 0x3f562fa3);



// ***************************************************************************
/**
 * The AnimDetail traversal.
 * AnimDetail observers MUST derive from IBaseAnimDetailObs.
 * There is no AnimDetail graph. traverse() use the clipTrav VisibilityList to traverse all observers.
 *
 * NB: see CScene for 3d conventions (orthonormal basis...)
 * \sa CScene IBaseAnimDetailObs
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CAnimDetailTrav : public ITravScene
{
public:

	/// Constructor
	CAnimDetailTrav();


	/// \name ITrav/ITravScene Implementation.
	//@{
	IObs				*createDefaultObs() const;
	NLMISC::CClassId	getClassId() const {return AnimDetailTravId;}
	/** render after Clip and before light.
	 * This order is important for possibles lights sticked to bones of skeletons.
	 */
	sint				getRenderOrder() const {return 2200;}
	void				traverse();
	//@}


public:
	// ONLY FOR OBSERVERS.

	sint64		CurrentDate;	// The current date of the traversal, usefull for evaldetail just one time..


	// For clipTrav. cleared at beginning of CClipTrav::traverse
	void				clearVisibleList();

	// For ClipObservers only. NB: list is cleared at begining of traverse(). NB: only CTransform are supported
	void				addVisibleObs(CTransformAnimDetailObs *obs);


// ********************
private:
	/// traverse the observer recusrively, followin Hrc hierarchy
	void				traverseHrcRecurs(IBaseAnimDetailObs *adObs);

	// traverse list of model visible and usefull to animDetail.
	std::vector<CTransformAnimDetailObs*>	_VisibleList;

};


} // NL3D


#endif // NL_ANIM_DETAIL_TRAV_H

/* End of anim_detail_trav.h */
