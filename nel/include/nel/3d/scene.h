/** \file scene.h
 * <File description>
 *
 * $Id: scene.h,v 1.3 2000/10/25 13:39:13 lecroart Exp $
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

#ifndef NL_SCENE_H
#define NL_SCENE_H


#include "nel/3d/mot.h"
#include "nel/3d/trav_scene.h"


namespace NL3D
{


class	CHrcTrav;
class	CClipTrav;
class	CLightTrav;
class	CRenderTrav;
class	CDefaultHrcObs;
class	CDefaultClipObs;
class	CDefaultLightObs;
class	CDefaultRenderObs;
class	CTransform;
class	CCamera;


// ***************************************************************************
/**
 * A CScene, which own a list of Render Traversals, and a render() method.
 *
 * \b USER \b RULES:
 * - Before creating any CScene, call the cool method CScene::registerBasics(), to register baisc models and observers.
 * - Create a CScene (NB: may be static \c CScene \c scene;).
 * - call first initDefaultRoot() to create / register automatically the 4 basic traversals:
 *		- CHrcTrav
 *		- CClipTrav
 *		- CLightTrav
 *		- CRenderTrav
 * - add others user-specified traversals with addTrav().
 * - initRootModels() to create/setRoot the defaults models roots for the basic traversals:
 *		- CTransform
 *		- CLightGroup
 * - create any other model with createModel() (such as a camera).
 * - Specify a Camera (SmartPtr-ed !!)
 * - render().
 *
 * CScene own those Traversals and those Root and kill them at ~CScene().
 *
 * Coordinate System:  right hand cordinates with: X to the right, Y to the far, Z to the top.
 *
 *           Z ^
 *             |      > Y
 *             |    /
 *             |  /
 *             |/
 *              -------> X
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CScene : public CMOT
{
public:
	/// The camera (SmartPtr-ed !!).
	NLMISC::CSmartPtr<CCamera>	CurrentCamera;


	/// \name The 4 default traversals, created / linked by CScene::initDefaultTraversals().
	//@{
	CHrcTrav	*HrcTrav;
	CClipTrav	*ClipTrav;
	CLightTrav	*LightTrav;
	CRenderTrav	*RenderTrav;
	//@}

	// The root models (will be deleted by CScene).
	CTransform	*Root;
	// TODO: define the lightgroup model.

public:

	/// \name Basic registration.
	//@{
	/// Register Basic models and observers.
	static void	registerBasics();
	//@}

	/// \name Construction / destruction.
	//@{
	/// Constructor.
	CScene();
	/// Destructor. Destroy the Basic traversals and default roots.
	~CScene();
	/// Create / register the 4 basic traversals:CHrcTrav, CClipTrav, CLightTrav, CRenderTravInit.
	void	initDefaultTravs();
	/// Create/setRoot the defaults models roots: a CTransform and a CLightGroup.
	void	initDefaultRoots();
	//@}

	/** Add a ITrav or a ITravScene to the scene.
	 * If not a ITravScene (tested with help of dynamic_cast) or if trav->getRenderOrder()==0, The traversal is not added 
	 * to the "render traversal list", else it is. Such a traversal will be traverse() -ed in the order given.
	 * The getRenderOrder() is called only in the addTrav() method (so this is a static information).
	 */
	void	addTrav(ITrav *v);

	/** Render the scene, via the registered ITravScene, from the CurrentCamera view.
	 * This call t->traverse() function to registered render traversal following their order given.
	 * \param doHrcPass set it to false to indicate that the CHrcTrav have not to be traversed. UseFull to optimize if 
	 * you know that NONE of your models have moved (a good example is a shoot of the scene from different cameras).
	 */
	void	render(bool	doHrcPass=true);


	/// Set the current camera (smartptr-ed!!).
	void	setCam(const NLMISC::CSmartPtr<CCamera>	&cam);

private:
	typedef		std::map<sint, ITravScene*>	TravMap;
	TravMap		RenderTraversals;	// Sorted via their getRenderOrder().

};


}


#endif // NL_SCENE_H

/* End of scene.h */
