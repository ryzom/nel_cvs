/** \file hrc_trav.h
 * <File description>
 *
 * $Id: hrc_trav.h,v 1.6 2000/12/08 10:34:46 berenguier Exp $
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

#ifndef NL_HRC_TRAV_H
#define NL_HRC_TRAV_H

#include "nel/3d/trav_scene.h"
#include "nel/misc/matrix.h"
#include "nel/misc/debug.h"


namespace	NL3D
{


using NLMISC::CVector;
using NLMISC::CPlane;
using NLMISC::CMatrix;


class IBaseHrcObs;


// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		HrcTravId=NLMISC::CClassId(0x5ad56382, 0x2a711530);


// ***************************************************************************
/**
 * The hierarchy traversal.
 * Hierarchy observers MUST derive from IBaseHrcObs.
 *
 * NB: see CScene for 3d conventions (orthonormal basis...)
 * \sa CScene IBaseHrcObs
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CHrcTrav : public ITravScene
{
public:
	/// The visibility flag. In the root case, Herit means Show.
	enum	TVisibility
	{
		Show,		// The model is shown in hte hierarchy
		Hide,		// The model is hidden in hte hierarchy
		Herit		// The model herit the visibilty from his father
	};

public:

	/// Constructor
	CHrcTrav()
	{
		CurrentDate=0;
	}


	/// \name ITrav/ITravScene Implementation.
	//@{
	IObs				*createDefaultObs() const;
	NLMISC::CClassId	getClassId() const {return HrcTravId;}
	sint				getRenderOrder() const {return 1000;}
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

	sint64		CurrentDate;	// The current date of the traversal, usefull for matrix update.
};



// ***************************************************************************
/**
 * The base interface for hierarchy traversal.
 * Hierarchy observers MUST derive from IBaseHrcObs.
 * This observer:
 * - leave the notification system to DO NOTHING.
 * - define his traverse() method.
 *
 * \b DERIVER \b RULES:
 * - implement the notification system (see IObs for details). The clean() method should call
 * updateLocal() so the node know it is updated.
 * - possibly modify/extend the traverse() method.
 *
 * \sa CHrcTrav
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class IBaseHrcObs : public IObs
{
public:


	/// \name IN variables.
	//@{
	CMatrix		LocalMatrix;
	CHrcTrav::TVisibility	LocalVis;	// The visibility state of the node.
	sint64		LocalDate;				// The update date of the LocalMatrix.
	//@}


	/// \name OUT variables.
	//@{
	CMatrix		WorldMatrix;
	bool		WorldVis;			// Is the node visible? (enabled?)
	sint64		WorldDate;			// The update date of the WorldMatrix.
	//@}


public:
	

	/// Constructor.
	IBaseHrcObs()
	{
		LocalVis= CHrcTrav::Herit; LocalMatrix.identity(); LocalDate=0;
		WorldVis= true; WorldMatrix.identity(); WorldDate=0;
	}

	
	/// \name Utility methods.
	//@{
	/// Update the world state according to the parent world state and the local states.
	void	updateWorld(IBaseHrcObs *caller)
	{
		if(caller)
		{
			if(LocalDate>WorldDate || caller->WorldDate>WorldDate)
			{
				// Must recompute the world matrix.
				WorldMatrix= caller->WorldMatrix * LocalMatrix;
				WorldDate= static_cast<CHrcTrav*>(Trav)->CurrentDate;
			}
			switch(LocalVis)
			{
				case CHrcTrav::Herit: WorldVis= caller->WorldVis; break;
				case CHrcTrav::Hide: WorldVis= false; break;
				case CHrcTrav::Show: WorldVis= true; break;
			}
		}
		else
		{
			// Root case.
			if(LocalDate>WorldDate)
			{
				// The world matrix is the local matrix, since we are root.
				WorldMatrix= LocalMatrix;
				WorldDate= static_cast<CHrcTrav*>(Trav)->CurrentDate;
			}
			switch(LocalVis)
			{
				// In the root case, Herit means Show.
				case CHrcTrav::Herit: WorldVis= true; break;
				case CHrcTrav::Hide: WorldVis= false; break;
				case CHrcTrav::Show: WorldVis= true; break;
			}
		}
	}
	/// clean() should call updateLocal() to notify the local modification of the matrix.
	void	updateLocal()
	{
		LocalDate= static_cast<CHrcTrav*>(Trav)->CurrentDate;
	}
	//@}


	/// \name The base doit method.
	//@{
	/// The base behavior is to update() the observer, updateWorld() states, and traverseSons().
	virtual	void	traverse(IObs *caller)
	{
		// update the observer according to the model.
		update();
		// Recompute the matrix, according to caller matrix mode, and local matrix.
		nlassert(!caller || dynamic_cast<IBaseHrcObs*>(caller));
		updateWorld(static_cast<IBaseHrcObs*>(caller));
		// DoIt the sons.
		traverseSons();
	}
	//@}


};


// ***************************************************************************
/**
 * The default hierarchy observer, used by unspecified models.
 * This observer:
 * - leave the notification system to DO NOTHING.
 * - leave the traverse() method as IBaseHrcObs.
 *
 * \sa IBaseHrcObs
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CDefaultHrcObs : public IBaseHrcObs
{
public:


	/// Constructor.
	CDefaultHrcObs() {}


};


}


#endif // NL_HRC_TRAV_H

/* End of hrc_trav.h */
