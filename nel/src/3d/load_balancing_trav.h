/** \file load_balancing_trav.h
 * The LoadBalancing traversal.
 *
 * $Id: load_balancing_trav.h,v 1.2 2001/08/23 10:13:13 berenguier Exp $
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

#ifndef NL_LOAD_BALANCING_TRAV_H
#define NL_LOAD_BALANCING_TRAV_H

#include "nel/misc/types_nl.h"
#include "nel/misc/matrix.h"
#include "nel/misc/plane.h"
#include "3d/trav_scene.h"


namespace NL3D 
{


using NLMISC::CVector;
using NLMISC::CPlane;
using NLMISC::CMatrix;


class IBaseLoadBalancingObs;
class IBaseHrcObs;
class IBaseClipObs;
class CClipTrav;


// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		LoadBalancingTravId=NLMISC::CClassId(0x7181548, 0x36ad3c10);



// ***************************************************************************
/**
 * The LoadBalancing traversal. It needs a camera setup (see ITravCameraScene).
 * LoadBalancing observers MUST derive from IBaseLoadBalancingObs.
 *
 * NB: see CScene for 3d conventions (orthonormal basis...)
 * \sa CScene IBaseLoadBalancingObs
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CLoadBalancingTrav : public ITravCameraScene
{
public:

	/// Constructor
	CLoadBalancingTrav();


	/// \name ITrav/ITravScene Implementation.
	//@{
	IObs				*createDefaultObs() const;
	NLMISC::CClassId	getClassId() const {return LoadBalancingTravId;}
	/** render after AnimDetailObs.
	 * This order is important to get correct object matrix sticked to skeletons.
	 */
	sint				getRenderOrder() const {return 2300;}
	void				traverse();
	//@}


	void				setClipTrav(CClipTrav *trav) {_ClipTrav= trav;}


	/// \name LoadBalancing mgt.
	//@{

	// see CScene.
	enum			TPolygonBalancingMode {PolygonBalancingOff=0, PolygonBalancingOn, PolygonBalancingClamp, CountPolygonBalancing };
	// The PolygonBalancingMode
	TPolygonBalancingMode	PolygonBalancingMode;


	void				setNbFaceWanted(uint nFaces) {_NbFaceWanted= nFaces;}
	uint				getNbFaceWanted() {return _NbFaceWanted;}

	//@}


public:
	// ONLY FOR OBSERVERS.
	// The number of faces count in Pass0.
	float				NbFacePass0;

	uint				getLoadPass() {return _LoadPass;}

	/** Compute the number of face to be rendered for thismodel, according to the number of faces he want to draw
	 * and to his distance from camera.
	 */
	float				computeModelNbFace(float faceIn, float camDist);


private:
	// The number of face the user want
	uint				_NbFaceWanted;

	// Pass: 0 (compute faceCount from all models) or 1 (setup wanted faceCount).
	uint				_LoadPass;

	// use this ratio into Pass 1 to reduce faces.
	float				_FaceRatio;

	// The loadBalancing balance only visible objects.
	CClipTrav			*_ClipTrav;
	void				traverseVisibilityList();

};


// ***************************************************************************
/**
 * The base interface for LoadBalancing traversal.
 * LoadBalancing observers MUST derive from IBaseLoadBalancingObs.
 * This observer:
 * - leave the notification system to DO NOTHING.
 * - leave traverse() undefined
 *
 * \b DERIVER \b RULES:
 * - implement the notification system (see IObs for details).
 * - implement the traverse() method.
 *
 * \sa CLoadBalancingTrav
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class IBaseLoadBalancingObs : public IObs
{
public:
	/// Shortcut to observers.
	IBaseHrcObs		*HrcObs;
	IBaseClipObs	*ClipObs;

public:

	/// Constructor.
	IBaseLoadBalancingObs()
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
 * The default LoadBalancing observer, used by unspecified models.
 * This observer:
 * - leave the notification system to DO NOTHING.
 * - implement the traverse() method to DO NOTHING
 *
 * \sa IBaseLoadBalancingObs
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CDefaultLoadBalancingObs : public IBaseLoadBalancingObs
{
public:


	/// Constructor.
	CDefaultLoadBalancingObs() {}

	/// \name The base doit method.
	//@{
	virtual	void	traverse(IObs *caller)
	{
		// no need to traverseSons. No graph here.
	}
	//@}

};


} // NL3D


#endif // NL_LOAD_BALANCING_TRAV_H

/* End of load_balancing_trav.h */
