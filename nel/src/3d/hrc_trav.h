/** \file hrc_trav.h
 * <File description>
 *
 * $Id: hrc_trav.h,v 1.6 2002/06/10 09:30:08 berenguier Exp $
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

#include "3d/trav_scene.h"
#include "nel/misc/matrix.h"
#include "nel/misc/debug.h"


namespace	NL3D
{


using NLMISC::CVector;
using NLMISC::CPlane;
using NLMISC::CMatrix;


class	IBaseHrcObs;
class	CSkipModel;
class	IBaseClipObs;
class	IBaseAnimDetailObs;

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
		Show=0,		// The model is shown in hte hierarchy
		Hide,		// The model is hidden in hte hierarchy
		Herit,		// The model herit the visibilty from his father

		VisibilityCount
	};

public:

	/// Constructor
	CHrcTrav()
	{
		// NB: Now, observers update is done before ALL traversals.
		// Hence, we must inc the value before scene rendering. This is equivalent to start with 1, and inc at end of traverse().
		CurrentDate= 1;
	}


	/// \name ITrav/ITravScene Implementation.
	//@{
	IObs				*createDefaultObs() const;
	NLMISC::CClassId	getClassId() const {return HrcTravId;}
	sint				getRenderOrder() const {return 1000;}
	void				traverse();
	//@}


	void		setSkipModelRoot(CSkipModel *m);


public:

	std::vector<IModel*> _MovingObjects;

	// ONLY FOR OBSERVERS.

	sint64		CurrentDate;	// The current date of the traversal, usefull for matrix update.

	CSkipModel		*SkipModelRoot;
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
 * - implement the notification system (see IObs for details). The update() method should update LocalDate.
 * so the node know it is updated.
 * - implement the traverse() method.
 *
 * \sa CHrcTrav
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class IBaseHrcObs : public IObs
{
public:
	/// Some Shortcut to observers.
	IBaseClipObs		*ClipObs;
	IBaseAnimDetailObs	*AnimDetailObs;

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
		WorldVis= true; WorldMatrix.identity(); 
		// Init the WorldDate to -1 so at first pass, LocalDate>WorldDate, and so
		// the model will be processed and so it'll may be inserted in LightingManager (for example)
		WorldDate=-1;
	}
	/// Build shortcut to observers.
	virtual	void	init();

	
	/// \name The base doit method.
	//@{
	/// The base behavior is to update() the observer, updateWorld() states, and traverseSons().
	virtual	void	traverse(IObs *caller) =0;
	//@}


	/// \name Graph methods. USED ONLY BY ITrav*.
	// @{
	/// Call IObs::addParent(), and dirt WorldDate to 0, so the worldMatrix will be recomputed next time.
	virtual	void	addParent(IObs *father);
	// @}

};


// ***************************************************************************
/**
 * The default hierarchy observer, used by unspecified models.
 * This observer:
 * - leave the notification system to DO NOTHING.
 * - traverse() just traverseSons().
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


	/// The default behavior is traverseSons() only.
	virtual	void	traverse(IObs *caller)
	{
		// DoIt the sons.
		traverseSons();
	}


};


}


#endif // NL_HRC_TRAV_H

/* End of hrc_trav.h */
