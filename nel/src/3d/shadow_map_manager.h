/** \file shadow_map_manager.h
 * <File description>
 *
 * $Id: shadow_map_manager.h,v 1.1 2003/08/07 08:49:13 berenguier Exp $
 */

/* Copyright, 2000-2003 Nevrax Ltd.
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

#ifndef NL_SHADOW_MAP_MANAGER_H
#define NL_SHADOW_MAP_MANAGER_H

#include "nel/misc/types_nl.h"
#include "3d/shadow_map.h"
#include "nel/misc/types_nl.h"
#include "3d/quad_grid.h"
#include "3d/vertex_buffer.h"
#include "3d/material.h"
#include "3d/texture.h"


namespace NL3D {


class	CTransform;


// ***************************************************************************
#define	NL3D_SMM_QUADGRID_SIZE		16
#define	NL3D_SMM_QUADCELL_SIZE		8.0f
#define	NL3D_SMM_MAX_TEXTDEST_SIZE	512


// ***************************************************************************
/**
 * A class to manage the render of shadow map.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2003
 */
class CShadowMapManager
{
public:

	/// Constructor
	CShadowMapManager();
	~CShadowMapManager();

	/// change the QuadGrid size. it reset all the receivers!
	void			setQuadGridSize(uint size, float cellSize);

	/// Add a ShadowCaster that influence the scene this pass.
	void			addShadowCaster(CTransform *model);
	/** From List of ShadowCaster, select a sub - part (so called Loding ShadowMap Casters)
	 *	NB: Beware that this clear the currentlist of shadowMap to generate
	 *	NB: model->setRenderingShadowMap(true); are called for each model selected
	 */
	void			selectShadowMapsToGenerate(CScene *scene);
	/** Add Manually a ShadowCaster that will compute his ShadowMap this pass.
	 *	NB: model->setRenderingShadowMap(true); is called
	 */
	void			addShadowCasterGenerate(CTransform *model);
	/// Add a ShadowRecevier visible in this scene for this pass
	void			addShadowReceiver(CTransform *model);

	/** render ShadowMap with ShadowCasters. Generate ShadowMap only (with the AuxDriver).
	 *	NB: current driver Frustum, ViewMatrix kept their initial state but ModelMatrix not.
	 */
	void			renderGenerate(CScene *scene);

	/** project ShadowMaps onto receivers. NB: renderGenerate() must have been called before.
	 *	NB: current driver Frustum, ViewMatrix kept their initial state but ModelMatrix not.
	 */
	void			renderProject(CScene *scene);

	/// Enable Polygon Smoothing
	void			enableShadowPolySmooth(bool enable) {_PolySmooth= enable;}
	/// get Enable Polygon Smoothing flag
	bool			getEnableShadowPolySmooth() const {return _PolySmooth;}

	/// Get the typical Material for Caster
	CMaterial		&getCasterShadowMaterial() {return _CasterShadowMaterial;}

// ******************
private:

	// The Casters added to this pass. cleared at render
	std::vector<CTransform*>	_ShadowCasters;
	// The Casters added to this pass that will compute their shadowMap this frame. cleared at render
	std::vector<CTransform*>	_GenerateShadowCasters;
	// This is a quadGrid of ShadowReceiver.
	typedef CQuadGrid<CTransform*>	TShadowReceiverGrid;
	TShadowReceiverGrid			_ShadowReceiverGrid;
	uint						_NumShadowReceivers;

	void			computeShadowDirection(CScene *scene, CTransform *sc, CVector &lightDir);

	void			fillBlackBorder(IDriver *drv, uint numPassText, uint numTextW, uint numTextH, uint baseTextureSize);

	void			setBlackQuad(uint index, sint x, sint y, sint w, sint h);

	void			updateBlurTexture(uint w, uint h);
	void			copyScreenToBlurTexture(IDriver *drv, uint numPassText, uint numTextW, uint numTextH, uint baseTextureSize);
	void			applyFakeGaussianBlur(IDriver *drv, uint numPassText, uint numTextW, uint numTextH, uint baseTextureSize);
	void			setBlurQuadFakeGaussian(uint index, sint x, sint y, sint w, sint h);

	bool						_PolySmooth;

	// For clearing
	CVertexBuffer				_FillQuads;
	CMaterial					_FillMaterial;

	// For Blurring
	CSmartPtr<ITexture>			_BlurTexture;
	uint32						_BlurTextureW;
	uint32						_BlurTextureH;
	float						_BlurTextureOOW;
	float						_BlurTextureOOH;
	float						_BlurTextureD05W;
	float						_BlurTextureD05H;
	CMaterial					_BlurMaterial;
	CVertexBuffer				_BlurQuads;

	// For Receiving
	CMaterial					_ReceiveShadowMaterial;
	CMatrix						_XYZToUWVMatrix;
	CMatrix						_XYZToWUVMatrix;
	NLMISC::CSmartPtr<ITexture>	_ClampTexture;

	// For Receiving
	CMaterial					_CasterShadowMaterial;

	void			clearGenerateShadowCasters();
};


} // NL3D


#endif // NL_SHADOW_MAP_MANAGER_H

/* End of shadow_map_manager.h */
