/** \file vegetable_manager.h
 * <File description>
 *
 * $Id: vegetable_manager.h,v 1.1 2001/10/31 10:19:40 berenguier Exp $
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

#ifndef NL_VEGETABLE_MANAGER_H
#define NL_VEGETABLE_MANAGER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/matrix.h"
#include "nel/misc/rgba.h"
#include "nel/misc/block_memory.h"
#include "3d/vegetable_clip_block.h"
#include "3d/vegetable_instance_group.h"
#include "3d/vegetable_shape.h"
#include "3d/vegetablevb_allocator.h"
#include "3d/material.h"


namespace NL3D 
{


class	IDriver;


// ***************************************************************************
/**
 * Manager of vegetable. Instance Factory and rendering.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CVegetableManager
{
public:

	/// Constructor
	CVegetableManager();


	/// \name Shape management
	// @{

	/// Load a shape if necessary, and return a shapeId for this shape.
	CVegetableShape				*getVegetableShape(const std::string &shape);

	// @}


	/// \name instance management
	// @{

	/// Create a clipBlock where instance group (ig) will be created.
	CVegetableClipBlock			*createClipBlock();
	/// delete such a clipBlock. all ig must be deleted before.
	void						deleteClipBlock(CVegetableClipBlock *clipBlock);

	/** create an instance group in a clipBlock, where instances will be created.
	 *	Instances will be frustum-clipped by the clipBlock.
	 */
	CVegetableInstanceGroup		*createIg(CVegetableClipBlock *clipBlock);
	/// delete such an ig.
	void						deleteIg(CVegetableInstanceGroup *ig);

	/** add an instance to an ig, enlarging the associated clipBlock
	 *	If the shape is not lighted, then only diffuseColor is used, to setup color per vertex.
	 */
	void						addInstance(CVegetableInstanceGroup *ig, 
		CVegetableShape	*shape, const NLMISC::CMatrix &mat, 
		const NLMISC::CRGBAF &ambientColor, const NLMISC::CRGBAF &diffuseColor, 
		float	bendFactor, float bendPhase);

	// @}


	/// \name render
	// @{

	/// must give a driver to the vegetableManager, before any addInstance().
	void			updateDriver(IDriver *driver);

	/// load a global texture used for all vegetables (lookup into CPath).
	void			loadTexture(const std::string &texName);
	/// setup a global texture used for all vegetables (smartPtr-ized).
	void			loadTexture(ITexture *itex);
	/// setup the directional light
	void			setDirectionalLight(const CVector &light);
	
	/** lock any AGP vertex buffers. Do it wisely (just one time before refine as example).
	 *	You MUST enclose calls to addInstance() (and so CVegetable::generateInstance())
	 *	with lockBuffers() / unlockBuffers().
	 */
	void			lockBuffers();
	/// unlock any AGP vertex buffers
	void			unlockBuffers();

	/** render the manager into a driver, with current viewMatrix/frustum/fog  setuped
	 *	Buffers should be unlocked. 
	 */
	void			render(const std::vector<CPlane> &pyramid, IDriver *driver);

	// @}


	/// \name Wind animation
	// @{

	/** set the Wind for animation.
	 *	All thoses variables may be modified each frame without penalty.
	 *
	 *	\param windDir is the direction of the wind. NB: only XY direction is kept.
	 *	\param windFreq is the frequency for the animation (speed)
	 *	\param windPower is the power of the wind, and is a factor (0..1) of Bend
	 */
	void		setWind(const CVector &windDir, float windFreq, float windPower);

	/** set the Wind animation Time (in seconds)
	 */
	void		setWindAnimationTime(double windTime);

	// @}


// *********************
private:
	NLMISC::CBlockMemory<CVegetableClipBlock>		_ClipBlockMemory;
	NLMISC::CBlockMemory<CVegetableInstanceGroup>	_InstanceGroupMemory;

	// List of ClipBlock not empty. tested for clipping
	CTessList<CVegetableClipBlock>					_ClipBlockList;
	// List of ClipBlock created, with no Ig, so not tested for clipping
	CTessList<CVegetableClipBlock>					_EmptyClipBlockList;


	// Vegetable Shape map.
	typedef	std::map<std::string, CVegetableShape>	TShapeMap;
	typedef	TShapeMap::iterator						ItShapeMap;
	TShapeMap										_ShapeMap;

	// Vertex Buffers for display. One allocator for each renderPass.
	CVegetableVBAllocator							_VBAllocator[NL3D_VEGETABLE_NRDRPASS];

	// Material. Usefull for texture and alphaTest
	CMaterial										_VegetableMaterial;
	// Norm
	CVector											_DirectionalLight;

	// return true if the ith rdrPass is 2Sided.
	static	bool	doubleSidedRdrPass(uint rdrPass);


	/// \name Wind animation
	// @{
	CVector											_WindDirection;
	float											_WindFrequency;
	float											_WindPower;
	double											_WindTime;
	double											_WindPrecRenderTime;
	// updated at each render().
	double											_WindAnimTime;
	// @}

};


} // NL3D


#endif // NL_VEGETABLE_MANAGER_H

/* End of vegetable_manager.h */
