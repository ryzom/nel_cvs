/** \file u_transform.h
 * <File description>
 *
 * $Id: u_transform.h,v 1.5 2002/01/07 10:23:58 vizerie Exp $
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

#ifndef NL_U_TRANSFORM_H
#define NL_U_TRANSFORM_H

#include "nel/misc/types_nl.h"
#include "nel/3d/u_instance_group.h"
#include "nel/3d/u_transformable.h"


namespace NL3D 
{

using NLMISC::CVector;
using NLMISC::CMatrix;
using NLMISC::CQuat;

// ***************************************************************************
/**
 * Base interface for manipulating Movable Objects in the scene: camera, lights, instances etc...
 * see UTransformable. A UTransform can be set in a hierachy, and can be hidden.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class UTransform : virtual public UTransformable
{
protected:

	/// \name Object
	// @{
	/// Constructor. By default, RotQuat mode.
	UTransform() {}
	virtual	~UTransform() {}
	// @}

public:
	// Enum should be the same than in CHrcTrav.

	/// The visibility flag. In the root case, Herit means Show.
	enum	TVisibility
	{
		Show=0,		// The model is shown in the hierarchy
		Hide,		// The model is hidden in the hierarchy
		Herit,		// The model herit the visibilty from his father

		VisibilityCount
	};


public:


	/// \name Hierarchy manipulation
	// @{
	/** Hierarchy edit. unlink this from oldparent, and make this be a son of newFather.
	 * if this was already a son of newFather, no-op.
	 * \param newFather the new Father. If NULL, the transform will be linked to the root of the hierarchy (Default!).
	 */
	virtual	void			parent(UTransform *newFather)=0;


	virtual void			setClusterSystem (UInstanceGroup *pIG)=0; 
	virtual UInstanceGroup *getClusterSystem ()=0;
// @}


	/// \name visibility
	// @{
	/// Hide the object and his sons.
	virtual	void			hide()=0;
	/// Show the objet and his sons.
	virtual	void			show()=0;
	/// herit the visibility from his father. (default behavior).
	virtual	void			heritVisibility()=0;
	/// Get the local visibility state.
	virtual	TVisibility		getVisibility()=0;
	// @}


	/// \name freezeHRC
	// @{
	/** Optimisation: freeze the HRC so the WorldMatrix computed at next render() will be kept for long, and 
	 *	the model won't either be tested in HRC.
	 *
	 *	NB: the model won't be tested in HRC only if this model is a "root"
	 *	For maximum optimisation, you should freezeHRC() all the models of a hierarchy, from base root to leaves.
	 *
	 *	NB: if the hierarchy of this object must change, or if the object must moves, you must call unfreezeHRC() first,
	 *	and you should do this for all the parents of this model.
	 */
	virtual	void			freezeHRC()=0;


	/**	see freezeHRC().
	 */
	virtual	void			unfreezeHRC()=0;
	// @}


	/** Set the current ordering layer for this transform.
	  * Typically, this is used to sort transparent objects. Isn't used with solid objects.
	  * For now :
	  * Layer 0 is for underwater
	  * Layer 1 is for water surfaces
	  * Layer 2 is for object above water
	  */
	virtual void  setOrderingLayer(uint layer) = 0;

	/// Get the ordering layer
	virtual uint getOrderingLayer() const = 0;

};


} // NL3D


#endif // NL_U_TRANSFORM_H

/* End of u_transform.h */
