/** \file scene.h
 * <File description>
 *
 * $Id: scene.h,v 1.16 2001/03/19 14:07:57 berenguier Exp $
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
#include "nel/3d/camera.h"
#include "nel/3d/trav_scene.h"
#include "nel/3d/tmp/viewport.h"
#include "nel/3d/shape.h"
#include "nel/misc/rgba.h"
#include "nel/misc/smart_ptr.h"
#include <map>


namespace NL3D
{


using	NLMISC::CRefPtr;
using	NLMISC::CSmartPtr;

class	CHrcTrav;
class	CClipTrav;
class	CLightTrav;
class	CAnimDetailTrav;
class	CRenderTrav;
class	CDefaultHrcObs;
class	CDefaultClipObs;
class	CDefaultLightObs;
class	CAnimDetailObs;
class	CDefaultRenderObs;
class	CTransform;
class	CTransformShape;
class	IDriver;


// ***************************************************************************
/**
 * A CScene, which own a list of Render Traversals, and a render() method.
 *
 * \b USER \b RULES:
 * - Before creating any CScene, call the cool method CScene::registerBasics(), to register baisc models and observers.
 * - Create a CScene (NB: may be static \c CScene \c scene;).
 * - call first initDefaultRoot() to create / register automatically the 5 basic traversals:
 *		- CHrcTrav
 *		- CClipTrav
 *		- CLightTrav
 *		- CAnimDetailTrav
 *		- CRenderTrav
 * - add others user-specified traversals with addTrav().
 * - initRootModels() to create/setRoot the defaults models roots for the basic traversals:
 *		- CTransform
 *		- CLightGroup
 * - set your driver for this scene with setDriver().
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
 * \b Shape \b System:\n
 * The scene has an instance Managagement:
 *		- IShape  design the object that is instancied (a mesh as example).
 *		- ITransformShape is the instance, which smart_point to a IShape.
 *		- user can add shape manually with addShape(), or remove them with delShape().
 *		- user create instance of a shape with CScene::createInstance(string shapeName);
 *			This create/load auto the shape if needed (searching in CPath, shapename must be a valid file name), 
 *			and then create the instance, with help of IShape::createInstance().
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CScene : public CMOT
{
public:


	/// \name Basic registration.
	//@{
	/// Register Basic models and observers.
	static void		registerBasics();
	//@}


	/// \name Construction / destruction.
	//@{
	/// Constructor.
	CScene();
	/// Destructor. release().
	~CScene();
	/// Create / register the 5 basic traversals:CHrcTrav, CClipTrav, CLightTrav, CAnimDetailTrav, CRenderTravInit.
	void			initDefaultTravs();
	/// Create/setRoot the defaults models roots: a CTransform and a CLightGroup.
	void			initDefaultRoots();
	/// Set the driver to render Traversal.
	void			setDriver(IDriver *drv);
	/// Get the driver of render Traversal.
	IDriver			*getDriver() const;
	/** Add a ITrav or a ITravScene to the scene.
	 * If not a ITravScene (tested with help of dynamic_cast) or if trav->getRenderOrder()==0, The traversal is not added 
	 * to the "render traversal list", else it is. Such a traversal will be traverse() -ed in the order given.
	 * The getRenderOrder() is called only in the addTrav() method (so this is a static information).
	 */
	void			addTrav(ITrav *v);
	/// Release all relative to the scene (Models, traversals...)... Destroy the Basic traversals too.
	void			release();
	//@}


	/// \name Render
	//@{
	/** Render the scene, via the registered ITravScene, from the CurrentCamera view.
	 * NB: no Driver clear buffers (color or ZBuffer) are done....
	 * This call t->traverse() function to registered render traversal following their order given.
	 * \param doHrcPass set it to false to indicate that the CHrcTrav have not to be traversed. UseFull to optimize if 
	 * you know that NONE of your models have moved (a good example is a shoot of the scene from different cameras).
	 */
	void			render(bool	doHrcPass=true);
	//@}


	/// \name Camera/Viewport.
	//@{
	/// Set/Get the current camera/Viewport.
	void			setCam(CCamera *cam) {CurrentCamera= cam;}
	CCamera			*getCam() {return CurrentCamera;}
	void			setViewport(const class CViewport& viewport)
	{
		_Viewport=viewport;
	}
	CViewport		getViewport()
	{
		return _Viewport;
	}
	//@}


	/// \name Instance Mgt.
	//@{
	/// Register manually a shape into the scene. If already here, remplaced.
	void			addShape(const std::string &shapeName, CSmartPtr<IShape> shape);
	/// delete a shape from the scene. It will be really deleted when all instances which points to it will be deleted.
	///  no-op if not exist.
	void			delShape(const std::string &shapeName);
	/// Create a model, instance of the shape "shapename". If not present, try to load "shapename" via the CPath.
	/// If fails, return NULL.
	virtual	CTransformShape	*createInstance(const std::string &shapeName);
	/** Delete an instance via his pointer. This is a synonym for deleteModel().
	 * \see deleteModel()
	 */
	void	deleteInstance(IModel *model) {deleteModel(model);}
	//@}


private:
	typedef			std::map<sint, ITravScene*>	TTravMap;
	TTravMap		RenderTraversals;	// Sorted via their getRenderOrder().

	/// The camera / Viewport.
	CRefPtr<CCamera>	CurrentCamera;
	CViewport		_Viewport;

	/// \name The 5 default traversals, created / linked by CScene::initDefaultTraversals().
	//@{
	CHrcTrav		*HrcTrav;
	CClipTrav		*ClipTrav;
	CLightTrav		*LightTrav;
	CAnimDetailTrav	*AnimDetailTrav;
	CRenderTrav		*RenderTrav;
	//@}

	// The root models (will be deleted by CScene).
	CTransform		*Root;
	// TODO: define the lightgroup model.


	/// \name Shape/Instances.
	//@{
	typedef			CSmartPtr<IShape>	PShape;
	typedef			std::map<std::string, PShape>	TShapeMap;
	TShapeMap		ShapeMap;
	//@}


};


}


#endif // NL_SCENE_H

/* End of scene.h */
