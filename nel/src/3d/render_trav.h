/** \file render_trav.h
 * <File description>
 *
 * $Id: render_trav.h,v 1.10 2002/06/19 08:42:10 berenguier Exp $
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

#ifndef NL_RENDER_TRAV_H
#define NL_RENDER_TRAV_H

#include "3d/trav_scene.h"
#include "3d/ordering_table.h"
#include "3d/layered_ordering_table.h"
#include "nel/misc/rgba.h"
#include "nel/3d/viewport.h"
#include "3d/light_contribution.h"
#include "3d/light.h"
#include "3d/mesh_block_manager.h"
#include <vector>


namespace	NL3D
{

using NLMISC::CVector;
using NLMISC::CPlane;
using NLMISC::CMatrix;

class	IBaseRenderObs;
class	IBaseHrcObs;
class	IBaseClipObs;
class	IBaseLightObs;
class	IDriver;
class	CMaterial;

class	CTransform;

// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		RenderTravId= NLMISC::CClassId(0x572456ee, 0x3db55f23);


// ***************************************************************************
/**
 * The Render traversal.
 * The purpose of this traversal is to render a list of models. This traversals is tightly linked to the cliptraversal.
 * The clipTraversals insert directly the observers with CRenderTrav::addRenderObs(obs). The traverse() method should 
 * render all the render observer with IDriver.
 *
 * This traversal is an exception since it is not designed to have a graph of observers. But this behavior is permitted,
 * and the root (if any) is traversed.
 *
 * \b USER \b RULES: Before using traverse() on a render traversal, you should:
 *	- setFrustum() the camera shape (focale....)
 *	- setCamMatrix() for the camera transform
 *
 * NB: see CScene for 3d conventions (orthonormal basis...)
 *
 * \sa CScene IBaseRenderObs
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CRenderTrav : public ITravCameraScene
{
public:

	/// Constructor
	CRenderTrav();

	/// \name ITrav/ITravScene Implementation.
	//@{
	IObs				*createDefaultObs() const;
	NLMISC::CClassId	getClassId() const {return RenderTravId;}
	sint				getRenderOrder() const {return 4000;}
	/** First traverse the root (if any), then render the render list.
	 * NB: no Driver clear buffers (color or ZBuffer) are done....
	 * \warning If an observer exist both in the graph and in the render list, it will be effectively traversed twice.
	 */
	void				traverse();
	//@}

	/// \name RenderList.
	//@{
	/// Clear the list of rendered observers.
	void			clearRenderList();
	/// Add an observer to the list of rendered observers. \b DOESN'T \b CHECK if already inserted.
	void			addRenderObs(IBaseRenderObs *o);
	//@}


	void			setDriver(IDriver *drv) {Driver= drv;}
	IDriver			*getDriver() {return Driver;}
	void			setViewport (const CViewport& viewport) 
	{
		_Viewport = viewport;
	}
	CViewport		getViewport () const
	{
		return _Viewport;
	}

	bool isCurrentPassOpaque() { return _CurrentPassOpaque; }

	/** Set the order or rendering for transparent objects.
	  * In real case, with direct order, we have:
	  * - Underwater is rendered.
	  * - Water is rendered.
	  * - Objects above water are rendered.
	  */
	void  setLayersRenderingOrder(bool directOrder = true) { _LayersRenderingOrder = directOrder; }
	bool  getLayersRenderingOrder() const { return _LayersRenderingOrder; }



	/// \name Render Lighting Setup.
	// @{

	// False by default. setuped by CScene
	bool						LightingSystemEnabled;

	// Global ambient. Default is (50,50,50).
	NLMISC::CRGBA				AmbientGlobal;
	// The Sun Setup.
	NLMISC::CRGBA				SunAmbient, SunDiffuse, SunSpecular;
	// set the direction of the sun. dir is normalized.
	void						setSunDirection(const CVector &dir);
	const CVector				getSunDirection() const {return _SunDirection;}

	// @}


// ******************
public:

	/// \name Render Lighting Setup. FOR OBSERVERS ONLY.
	// @{

	// Max VP Light setup Infos.
	enum	{MaxVPLight= 4};

	/** reset the lighting setup in the driver (all lights are disabled).
	 *	called at begining of traverse(). Must be called by any model (before and after rendering)
	 *	that wish to use CDriver::setLight() instead of the standard behavior with changeLightSetup()
	 */
	void		resetLightSetup();

	/** setup the driver to the given lightContribution.
	 *	if lightContribution==NULL, then all currently enabled lights are disabled.
	 *	NB: lightContribution is cached, so if same than preceding, no-op.
	 *	cache cleared at each frame with resetLightSetup().
	 *	NB: models which are sticked or skinned on a skeleton have same lightContribution 
	 *	because lightSetup is made on the skeleton only. Hence the interest of this cache.
	 *
	 *	\param useLocalAttenuation if true, use Hardware Attenuation, else use global one 
	 *	(attenuation with AttFactor)
	 */
	void		changeLightSetup(CLightContribution	*lightContribution, bool useLocalAttenuation);


	/** setup the driver VP constants to get info from current LightSetup. 
	 *	Only 0..3 Light + SunLights are supported. The VP do NOT support distance/Spot attenuation
	 *	Also it does not handle World Matrix with non uniform scale correctly since lighting is made in ObjectSpace
	 *	
	 *	\param ctStart the program use ctes from ctStart to ctStart+NumCtes.
	 *	\param supportSpecular asitsounds. PointLights and dirLight are localViewer
	 *	\param invObjectWM the inverse of object matrix: lights are mul by this. Vp compute in object space.	 
	 */
	void		beginVPLightSetup(uint ctStart, bool supportSpecular, const CMatrix &invObjectWM);

	/** change the driver VP LightSetup constants which depends on material.
	 *  \param excludeStrongest This remove the strongest light from the setup. The typical use is to have it computed by using perpixel lighting.
	 */
	void		changeVPLightSetupMaterial(const CMaterial &mat, bool excludeStrongest);


	/** tool to get a VP fragment which compute lighting with following rules:
	 *	IN: 
	 *		- R5  vertex in objectSpace (ie untransformed)
	 *		- R6  normal in objectSpace (ie untransformed)
	 *	OUT: 
	 *		- R6  normal normalized
	 *		- o[COL0] and o[COL1] are setuped. NB: BF0 and BF1 not computed/setuped.
	 *	Scratch:
	 *		- R0, R1, R2, R3, R4
	 *
	 *	For information, constant mapping is (add ctStart):
     *
	 *  == Strongest light included ==
     *
	 *	if !supportSpecular:
	 *		- 0:		AmbientColor.
	 *		- 1..4:		DiffuseColor of 4 lights.
	 *		- 5:		- (directional Light direction) in objectSpace
	 *		- 6..8:		light position (3 pointLihgts) in objectSpace
	 *		- 9:		material Diffuse Alpha copied to output. cte is: {0,0, 1, alphaMat}
	 *		TOTAL: 10 constants used.
	 *	if supportSpecular:
	 *		- 0:		AmbientColor.
	 *		- 1..4:		DiffuseColor of 4 lights.
	 *		- 5..8:		SpecularColor of 4 lights. NB: SpecularColor[5].w get the specular exponent of the material
	 *		- 9:		- (directional Light direction) in objectSpace
	 *		- 10:		material Diffuse Alpha copied to output. cte is: {0,0, 1, alphaMat}
	 *		- 11:		eye position in objectSpace
	 *		- 12..14:	light position (3 pointLihgts) in objectSpace
	 *		TOTAL: 15 constants used.
	 *	 
	 *
	 *	 NB: the number of active light does not change the number of constantes used. But the VP code returned is 
	 *	modified accordingly.
	 *
	 *  \param numActivePoinLights tells how many point light from 0 to 3 this VP must handle. NB: the Sun directionnal is not option
	 *		NB: nlassert(numActiveLights<=MaxVPLight-1).
	 */
	static	std::string		getLightVPFragment(uint numActivePointLights, uint ctStart, bool supportSpecular, bool normalize);

	/** This returns a reference to a driver light, by its index
	  * \see getStrongestLightIndex
	  */
	const CLight  &getDriverLight(sint index) const 
	{ 
		nlassert(index >= 0 && index < NL3D_MAX_LIGHT_CONTRIBUTION+1);
		return _DriverLight[index]; 
	}

	/// return an index to the current strongest settuped light (or -1 if there's none)
	sint		getStrongestLightIndex() const;

	/** Get current color, diffuse and specular of the strongest light in the scene.
	  * These values are modulated by the current material color, so these values are valid only after
	  * changeVPLightSetupMaterial() has been called
	  */
	void		getStrongestLightColors(NLMISC::CRGBA &diffuse, NLMISC::CRGBA &specular);

	/** return the number of VP lights currently activated (sunlight included)
	 *	Value correct after beginVPLightSetup() only
	 */
	uint		getNumVPLights() const {return _VPNumLights;}

	// @}


	/// \name MeshBlock Manager. FOR OBSERVERS AND MESHS ONLY.
	// @{
	/// The manager of meshBlock. Used to add instances.
	CMeshBlockManager		MeshBlockManager;
	// @}

private:
	
	// A grow only list of observers to be rendered.
	std::vector<IBaseRenderObs*>	RenderList;
	// Ordering Table to sort transparent objects
	COrderingTable<IBaseRenderObs>			OrderOpaqueList;
	CLayeredOrderingTable<IBaseRenderObs>	OrderTransparentList;

	IDriver			*Driver;
	CViewport		_Viewport;

	// Temporary for the render
	bool			_CurrentPassOpaque;	
	bool			_LayersRenderingOrder;


	/// \name Render Lighting Setup.
	// @{
	// The last setup.
	CLightContribution			*_CacheLightContribution;
	bool						_LastLocalAttenuation;
	// The number of light enabled
	uint						_NumLightEnabled;

	// More precise setup
	uint						_LastSunFactor;
	NLMISC::CRGBA				_LastSunAmbient;
	CPointLight					*_LastPointLight[NL3D_MAX_LIGHT_CONTRIBUTION];
	uint8						_LastPointLightFactor[NL3D_MAX_LIGHT_CONTRIBUTION];
	bool						_LastPointLightLocalAttenuation[NL3D_MAX_LIGHT_CONTRIBUTION];

	CVector						_SunDirection;

	// driver Lights setuped in changeLightSetup()
	CLight						_DriverLight[NL3D_MAX_LIGHT_CONTRIBUTION+1];

	// index of the strongest light (when used)
	mutable uint				_StrongestLightIndex;
	mutable bool				_StrongestLightTouched;

	// Current ctStart setuped with beginVPLightSetup()
	uint						_VPCurrentCtStart;
	// Current num of VP lights enabled.
	uint						_VPNumLights;
	// Current support of specular
	bool						_VPSupportSpecular;
	// Sum of all ambiant of all lights + ambiantGlobal.
	NLMISC::CRGBAF				_VPFinalAmbient;
	// Diffuse/Spec comp of all light / 255.
	NLMISC::CRGBAF				_VPLightDiffuse[MaxVPLight];
	NLMISC::CRGBAF				_VPLightSpecular[MaxVPLight];

	NLMISC::CRGBA				_StrongestLightDiffuse;
	NLMISC::CRGBA				_StrongestLightSpecular;	

	// @}

};


// ***************************************************************************
/**
 * The base interface for render observers.
 * Render observers MUST derive from IBaseRenderObs.
 * This observer:
 * - leave the notification system to DO NOTHING.
 * - leave traverse() to undefined.
 * - implement the init() method, to set shortcut to neighbor observers.
 *
 * \b DERIVER \b RULES:
 * - implement the notification system (see IObs and IObs() for details).
 * - implement the traverse(). See traverse() for more information. 
 * - possibly (but obviously not) modify/extend the graph methods (such as a graph behavior).
 *
 * \sa CRenderTrav
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class IBaseRenderObs : public IObs
{
public:
	/// Shortcut to observers.
	IBaseHrcObs		*HrcObs;
	IBaseClipObs	*ClipObs;
	IBaseLightObs	*LightObs;

public:

	/// Constructor.
	IBaseRenderObs()
	{
		HrcObs=NULL;
		ClipObs= NULL;
		LightObs= NULL;
	}
	/// Build shortcut to Hrc, Clip and Light.
	virtual	void	init();


	/** The base render method.
	 * The observers should not traverseSons(), for speed improvement.
	 */
	virtual	void	traverse(IObs *caller)=0;

	/**
	 * To avoid dynamic casting in mot fault of yoyo
	 */
	virtual CTransform* getTransformModel() 
	{
		return NULL;
	}
};



// ***************************************************************************
/**
 * The default render observer, used by unspecified models.
 * This observer:
 * - leave the notification system to DO NOTHING.
 * - define the traverse() method to DO NOTHING..
 *
 * \sa IBaseRenderObs
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CDefaultRenderObs : public IBaseRenderObs
{
public:


	/// Do nothing, and don't traverseSons() too.
	virtual	void	traverse(IObs *caller)
	{
	}
};


}


#endif // NL_RENDER_TRAV_H

/* End of render_trav.h */
