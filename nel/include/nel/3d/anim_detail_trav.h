/** \file anim_detail_trav.h
 * <File description>
 *
 * $Id: anim_detail_trav.h,v 1.1 2001/03/19 14:07:57 berenguier Exp $
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
#include "nel/3d/trav_scene.h"


namespace NL3D
{


using NLMISC::CVector;
using NLMISC::CPlane;
using NLMISC::CMatrix;


class IBaseAnimDetailObs;


// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		AnimDetailTravId=NLMISC::CClassId(0x45d317a8, 0x5dae3f40);



// ***************************************************************************
/**
 * The AnimDetail traversal.
 * AnimDetail observers MUST derive from IBaseAnimDetailObs.
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
	CAnimDetailTrav()
	{
		CurrentDate=0;
	}


	/// \name ITrav/ITravScene Implementation.
	//@{
	IObs				*createDefaultObs() const;
	NLMISC::CClassId	getClassId() const {return AnimDetailTravId;}
	// render after light and before render.
	sint				getRenderOrder() const {return 3200;}
	void				traverse()
	{
		// Inc the date.
		CurrentDate++;
		// Traverse the graph.
		if(Root)
			Root->traverse(NULL);
	}
	//@}


public:
	// ONLY FOR OBSERVERS.

	sint64		CurrentDate;	// The current date of the traversal, usefull for evaldetail just one time..
};



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
	
	/// Constructor.
	IBaseAnimDetailObs()
	{
	}


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
 * - implement the traverse() method to DO NOTHING, but traverseSons.
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
		traverseSons();
	}
	//@}

};



} // NL3D


#endif // NL_ANIM_DETAIL_TRAV_H

/* End of anim_detail_trav.h */
