/** \file scene.h
 * A 3d scene, manage model instantiation, tranversals etc..
 *
 * $Id: scene.h,v 1.30 2002/07/08 12:59:27 vizerie Exp $
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


#include "3d/mot.h"
#include "3d/camera.h"
#include "3d/shape.h"
#include "3d/animated_lightmap.h"
#include "3d/play_list_manager.h"
#include "3d/quad_grid_clip_manager.h"
#include "3d/particle_system_manager.h"
#include "3d/animation_set.h"



#include "nel/3d/viewport.h"

#include "nel/misc/rgba.h"
#include "nel/misc/smart_ptr.h"

#include <map>
#include <list>

/// This namespace contains all 3D class
namespace NL3D
{


using	NLMISC::CRefPtr;
using	NLMISC::CSmartPtr;

class	CHrcTrav;
class	CClipTrav;
class	CLightTrav;
class	CAnimDetailTrav;
class	CLoadBalancingTrav;
class	CRenderTrav;
class	CDefaultHrcObs;
class	CDefaultClipObs;
class	CDefaultLightObs;
class	CAnimDetailObs;
class	CDefaultRenderObs;
class	CTransform;
class	CTransformShape;
class	IDriver;
class	CShapeBank;
class	CCoarseMeshManager;
class	CInstanceGroup;
class	CSkipModel;
class	CLodCharacterManager;


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
 *		- CLoadBalancingTrav
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
 *		- user can add shape manually in the scene CShapeBank with CShapeBank::add (), or remove them with CShapeBank::release ().
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
	/// Create / register the 5 basic traversals:CHrcTrav, CClipTrav, CLightTrav, CAnimDetailTrav, CLoadBalancingTrav, CRenderTravInit.
	void			initDefaultTravs();
	/// Create/setRoot the defaults models roots: a CTransform and a CLightGroup.
	void			initDefaultRoots();
	/// Create the world instance group (with only one cluster)
	void			initGlobalnstanceGroup();
	/// Create coarse meshes manager.
	void			initCoarseMeshManager ();
	/// init QuadGridClipManager
	void			initQuadGridClipManager ();
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
	

	/** Release all relative to the scene (Models, traversals...)... Destroy the Basic traversals too.
	 *	The Lod Character Manager is reset() ed, but not deleted (at dtor only).
	 */
	virtual void			release();
	//@}


	/// \name Render
	//@{
	/** Render the scene, via the registered ITravScene, from the CurrentCamera view.
	 *  This also update waiting instance that are loaded asynchronously (by calling updateWaitingInstances)
	 * NB: no Driver clear buffers (color or ZBuffer) are done....
	 * This call t->traverse() function to registered render traversal following their order given.
	 * \param doHrcPass set it to false to indicate that the CHrcTrav have not to be traversed. UseFull to optimize if 
	 * you know that NONE of your models have moved (a good example is a shoot of the scene from different cameras).
	 */
	void			render(bool	doHrcPass=true);
	//@}

	/** Update instances that are loaded asynchronously
	  * \param systemTimeEllapsed : the time between 2 calls to updateWaitingInstances, in seconds
	  */
	void			updateWaitingInstances(double systemTimeEllapsed);


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
	/// Set the shape bank
	void			setShapeBank(CShapeBank*pShapeBank);

	/// Get the shape bank
	CShapeBank   *getShapeBank(void) { return _ShapeBank ; }

	/// Get the shape bank (const version)
	const CShapeBank   *getShapeBank(void) const { return _ShapeBank ; }

	/** Create a model, instance of the shape "shapename". If not present, try to load "shapename" via the CPath.
	 * If fails, return NULL.
	 */
	virtual	CTransformShape	*createInstance(const std::string &shapeName);

	/** Create an instance, if the shape is not present, load the shape asynchronously. The instance is really
	 * created when we process it in the rendering.
	 */
	void			createInstanceAsync(const std::string &shapeName, CTransformShape **pInstance);

	/** Delete an instance via his pointer. An instance is an entity which reference a shape.
	 */
	void			deleteInstance(CTransformShape*model);
	//@}

	CInstanceGroup *getGlobalInstanceGroup() { return _GlobalInstanceGroup;	}

	/// \name Light Mgt.
	//@{
	void	loadLightmapAutoAnim (const std::string &);
	void	setAutoAnim (CAnimation *pAnim);
	void	animate (TGlobalAnimationTime atTime);
	/// Add an IG for auto PointLight Factor animation. called by CInstanceGroup::addToScene()
	void	addInstanceGroupForLightAnimation(CInstanceGroup *ig);
	/// Remove an IG for auto PointLight Factor animation. called by CInstanceGroup::removeFromScene()
	void	removeInstanceGroupForLightAnimation(CInstanceGroup *ig);
	// TODO:
	// TurnOff/TurnOn
	// Animate
	//CLight* addStaticLight(CLight*); // Generate Light Map
	//void addDynamicLight(CLight*); // ??? Vertex lighting
	//@}



	/// get the current time of the scene, in second. It start from 0 (at the firt call of animate)
	TGlobalAnimationTime getCurrentTime(void) const { return _CurrentTime ; }

	/// get the ellapsed time (in second) between the last 2 calls of animate.
	TAnimationTime		getEllapsedTime(void) const { return _EllapsedTime ; }

	/** System time is a time that always run (independent from the animation time that run only on animate()) 
	 *	It is updated at beginning of render()
	 */
	double				getCurrentSystemTime() const {return _GlobalSystemTime;}

	/// get the ellapsed time (in second) between the last 2 calls of render().
	double				getEllapsedSystemTime() const { return _DeltaSystemTimeBetweenRender;}

	/// get the number of time render has been called
	uint64				getNumRender() const { return _NumRender; }


	/// \name LoadBalancing mgt.
	//@{

	/** Setup the number of faces max you want. For backward compatibility only, this is same as
	 *	setGroupLoadMaxPolygon("Global", nFaces);
	 */
	void					setLoadMaxPolygon(uint nFaces);
	/** Get the number of faces max you asked. For backward compatibility only, this is same as
	 *	getGroupLoadMaxPolygon("Global", nFaces);
	 */
	uint					getLoadMaxPolygon();
	/** Get the last face count asked from the instances before reduction.
	 *	It gets the sum of All groups.
	 */
	float					getNbFaceAsked () const;


	/** Set the number of faces wanted for a LoadBlancingGroup.
	 *	The Group is created if did not exist.
	 */
	void					setGroupLoadMaxPolygon(const std::string &group, uint nFaces);
	/** Get the number of faces wanted for a LoadBlancingGroup.
	 *	The Group is created if did not exist.
	 */
	uint					getGroupLoadMaxPolygon(const std::string &group);
	/** Get the last face count asked from the instances before reduction. only for the given group
	 *	return 0 if the Group does not exist.
	 */
	float					getGroupNbFaceAsked (const std::string &group) const;


	/** The mode of polygon balancing.
	 * PolygonBalancingOff => Models will be rendered with the number of faces they want to render.
	 * PolygonBalancingOn  => Models will be rendered with the number of faces the LoadBalancing want.
	 * PolygonBalancingClamp => Same as PolygonBalancingOn, but factor <= 1, ie models won't be rendered
	 *	with more face they want to render.
	 */
	enum			TPolygonBalancingMode {PolygonBalancingOff=0, PolygonBalancingOn, PolygonBalancingClamp, CountPolygonBalancing};

	/// Set the PolygonBalancingMode
	void			setPolygonBalancingMode(TPolygonBalancingMode polBalMode);
	/// Get the PolygonBalancingMode
	TPolygonBalancingMode	getPolygonBalancingMode() const;

	//@}

	/// \name Coarse meshes managers.
	//@{
	CCoarseMeshManager		*getStaticCoarseMeshManager ()
	{
		return _StaticCoarseMeshManager;
	}
	CCoarseMeshManager		*getDynamicCoarseMeshManager ()
	{
		return _DynamicCoarseMeshManager;
	}

	/**
	 *	Each coarseMesh lighting will be updated every "period" frame. clamped to 1,255
	 */
	void					setCoarseMeshLightingUpdate(uint8 period);

	/// see setCoarseMeshLightingUpdate()
	uint8					getCoarseMeshLightingUpdate() const {return _CoarseMeshLightingUpdate;}

	/// Get the LodCharacterManager. Never NULL.
	CLodCharacterManager	*getLodCharacterManager () const
	{
		return _LodCharacterManager;
	}

	//@}


	/// \name transparent Layer mgt
	//@{
		/** Set the order or rendering for transparent objects.
		  * In real case, with direct order, we have:
		  * - Underwater is rendered.
		  * - Water is rendered.
		  * - Objects above water are rendered.
		  */
		void  setLayersRenderingOrder(bool directOrder = true);
		bool  getLayersRenderingOrder() const;
	//@}


	/// \name Sun Light mgt
	//@{

	/** Enable Scene Lighting system. For backward compatibility, false by default.
	 *	If false, all objects will take last driver 's light setup
	 */
	void			enableLightingSystem(bool enable);

	/// see enableLightingSystem
	bool			isLightingSystemEnabled() const {return _LightingSystemEnabled;}

	/// set the global Ambient used for the scene. Default to (50, 50, 50).
	void			setAmbientGlobal(NLMISC::CRGBA ambient);
	/// set the Ambient of the Sun used for the scene.
	void			setSunAmbient(NLMISC::CRGBA ambient);
	/// set the Diffuse of the Sun used for the scene.
	void			setSunDiffuse(NLMISC::CRGBA diffuse);
	/// set the Specular of the Sun used for the scene.
	void			setSunSpecular(NLMISC::CRGBA specular);
	/// set the Direction of the Sun used for the scene.
	void			setSunDirection(const NLMISC::CVector &direction);


	/// get the global Ambient used for the scene. Default to (50, 50, 50).
	NLMISC::CRGBA	getAmbientGlobal() const;
	/// get the Ambient of the Sun used for the scene.
	NLMISC::CRGBA	getSunAmbient() const;
	/// get the Diffuse of the Sun used for the scene.
	NLMISC::CRGBA	getSunDiffuse() const;
	/// get the Specular of the Sun used for the scene.
	NLMISC::CRGBA	getSunSpecular() const;
	/// get the Direction of the Sun used for the scene.
	NLMISC::CVector	getSunDirection() const;


	/** setup the max number of point light that can influence a model. NB: clamped by NL3D_MAX_LIGHT_CONTRIBUTION
	 *	Default is 3.
	 *	NB: the sun contribution is not taken into account
	 */
	void		setMaxLightContribution(uint nlights);
	/// \see setMaxLightContribution()
	uint		getMaxLightContribution() const;

	/** Advanced. When a model is influenced by more light than allowed, or when it reach the limits 
	 *	of the light (attenuationEnd), the light can be darkened according to some threshold.
	 *	The resultLightColor begin to fade when distModelToLight== attEnd- threshold*(attEnd-attBegin).
	 *	when distModelToLight== 0, resultLightColor==Black.
	 *	By default, this value is 0.1f. Setting higher values will smooth transition but will 
	 *	generally darken the global effects of lights.
	 *	NB: clamp(value, 0, 1);
	 */
	void		setLightTransitionThreshold(float lightTransitionThreshold);
	/// \see getLightTransitionThreshold()
	float		getLightTransitionThreshold() const;


	//@}


	/// \name Weather mgt
	//@{

	/// Set the current windPower for all the scene. 0-1.
	void			setGlobalWindPower(float gwp);
	/// get the current windPower
	float			getGlobalWindPower() const {return _GlobalWindPower;}

	/// Set the current windDirection for all the scene. dir.z set to 0 and vector normalized.
	void			setGlobalWindDirection(const CVector &gwd);
	/// get the current windDirection
	const CVector	&getGlobalWindDirection() const {return _GlobalWindDirection;}

	//@}


	/// \name Trav accessor. Use it with caution. (used for mesh rendering)
	//@{
	CHrcTrav			*getHrcTrav() const {return HrcTrav;}
	CClipTrav			*getClipTrav() const {return ClipTrav;}
	CLightTrav			*getLightTrav() const {return LightTrav;}
	CAnimDetailTrav		*getAnimDetailTrav() const {return AnimDetailTrav;}
	CLoadBalancingTrav	*getLoadBalancingTrav() const {return LoadBalancingTrav;}
	CRenderTrav			*getRenderTrav() const {return RenderTrav;}

	// Link models to this one to disable their use in the wanted traversal
	CSkipModel			*getSkipModelRoot() const {return SkipModelRoot;}

	//@}


	/// Get a ref. to the particle system manager. You shouldn't call this (has methods for private processing)
	CParticleSystemManager &getParticleSystemManager();


	/// set the automatic animation set used by this scene. It is stored as a smart pointer
	void				setAutomaticAnimationSet(CAnimationSet *as) { _AutomaticAnimationSet = as; }

	/// Get a reference to the set of automatic animations
	CAnimationSet		*getAutomaticAnimationSet() const { return _AutomaticAnimationSet; }	

private:
	typedef			std::map<sint, ITravScene*>	TTravMap;
	TTravMap		RenderTraversals;	// Sorted via their getRenderOrder().

	/// The camera / Viewport.
	CRefPtr<CCamera>	CurrentCamera;
	CViewport		_Viewport;

	// the current time
	TGlobalAnimationTime  _CurrentTime ;
	
	// the real time
	TGlobalAnimationTime  _RealTime ;

	// true when its the first call of animate
	bool _FirstAnimateCall ;

	// the ellapsed time
	TAnimationTime  _EllapsedTime ;

	// System time is a time that always run (independent from the animation time that run only on animate()) 
	double	_DeltaSystemTimeBetweenRender;
	double	_GlobalSystemTime;
	uint64  _NumRender; // the number of time render has been called


	/// \name The 5 default traversals, created / linked by CScene::initDefaultTraversals().
	//@{
	CHrcTrav		*HrcTrav;
	CClipTrav		*ClipTrav;
	CLightTrav		*LightTrav;
	CAnimDetailTrav	*AnimDetailTrav;
	CLoadBalancingTrav	*LoadBalancingTrav;
	CRenderTrav		*RenderTrav;
	//@}

	// The root models (will be deleted by CScene).
	CTransform		*Root;
	CSkipModel		*SkipModelRoot;
	// This model is used to clip any model which has a Skeleton ancestor
	CRootModel		*SonsOfAncestorSkeletonModelGroup;
	// This model is used for LightTrav to know its dynamic pointLights
	CRootModel		*LightModelRoot;


	// The Ligths automatic movements
	CAnimationSet	_LightmapAnimations;
	std::set<CAnimatedLightmap*> _AnimatedLightmap;
	CPlayListManager _LMAnimsAuto;
	// List of InstanceGroup to animate PointLightFactor.
	typedef std::set<CInstanceGroup*>	TAnimatedIgSet;
	typedef TAnimatedIgSet::iterator	ItAnimatedIgSet;
	TAnimatedIgSet	_AnimatedIgSet;

	/// \name Shape/Instances.
	//@{
	CShapeBank		*_ShapeBank;

	typedef std::multimap<std::string,CTransformShape**> TWaitingInstancesMMap;
	TWaitingInstancesMMap _WaitingInstances;

	CInstanceGroup* _GlobalInstanceGroup;
	//@}

	/// \name Coarse meshes managers.
	//@{
	CRefPtr<CCoarseMeshManager>	_StaticCoarseMeshManager;
	CRefPtr<CCoarseMeshManager>	_DynamicCoarseMeshManager;
	// CLodCharacterManager is not a model. created at ctor.
	CLodCharacterManager		*_LodCharacterManager;
	//@}

	/// \name Clip features
	//@{
	CQuadGridClipManager		_QuadGridClipManager;
	//@}


	// Lighting.
	bool						_LightingSystemEnabled;

	/// set setCoarseMeshLightingUpdate
	uint8						_CoarseMeshLightingUpdate;

	/// \name Particle systems specific
	//@{		
		CParticleSystemManager	_ParticleSystemManager;
	//@}


	/// \name Weather mgt
	//@{
	float						_GlobalWindPower;
	CVector						_GlobalWindDirection;
	//@}
	

	// A set of automatic animation
	NLMISC::CSmartPtr<CAnimationSet>			_AutomaticAnimationSet;	

};


}


#endif // NL_SCENE_H

/* End of scene.h */
