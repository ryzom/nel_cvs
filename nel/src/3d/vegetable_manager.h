/** \file vegetable_manager.h
 * <File description>
 *
 * $Id: vegetable_manager.h,v 1.6 2001/12/03 09:29:22 berenguier Exp $
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
#include "nel/misc/vector_2f.h"
#include "3d/vegetable_clip_block.h"
#include "3d/vegetable_sort_block.h"
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

	/**	
	 * \param maxVertexVbHardUnlit maximum VertexCount in VBHard for Unlit (or precomputed lighted) vegetables
	 * \param maxVertexVbHardLighted maximum VertexCount in VBHard for Lighted vegetables
	 */
	CVegetableManager(uint maxVertexVbHardUnlit, uint maxVertexVbHardLighted);
	~CVegetableManager();


	/// \name Shape management
	// @{

	/// Load a shape if necessary, and return a shapeId for this shape.
	CVegetableShape				*getVegetableShape(const std::string &shape);

	// @}


	/// \name instance management
	// @{

	/// Create a clipBlock where SortBlock will be created.
	CVegetableClipBlock			*createClipBlock();
	/// delete such a clipBlock. all sortBlocks and so all ig must be deleted before.
	void						deleteClipBlock(CVegetableClipBlock *clipBlock);

	/** Create a SortBlock in a clipBlock where instance group (ig) will be created.
	 *	\param center you must give an approximate center for the sortBlock (for sorting)
	 *	\param radius you must give an approximate radius for the sortBlock (for the system to know when you are IN
	 *	the sortBlock, and then to sort in a better way)
	 */
	CVegetableSortBlock			*createSortBlock(CVegetableClipBlock *clipBlock, const CVector &center, float radius);
	/// delete such a SortBlock. all ig must be deleted before.
	void						deleteSortBlock(CVegetableSortBlock *sortBlock);

	/** create an instance group in a sortBlock, where instances will be created.
	 *	Instances will be frustum-clipped by the clipBlock, and sorted (for the ZSort rdrPass only) by sortBlock.
	 */
	CVegetableInstanceGroup		*createIg(CVegetableSortBlock *sortBlock);
	/** delete such an ig.
	 *	After doing this, you must call igSortBlockOwner->updateSortBlock()
	 *	If the sortBlock has many Igs, you can do it after deleting all your igs.
	 */
	void						deleteIg(CVegetableInstanceGroup *ig);

	/** add an instance to an ig, enlarging the associated clipBlock
	 *	If the shape is not lighted, then only diffuseColor is used, to setup color per vertex.
	 *	Warning! Use OptFastFloor()! So call must be enclosed with a OptFastFloorBegin()/OptFastFloorEnd().
	 *
	 *	Also, buffer must be locked.
	 *
	 *	After adding a bunch of instances to an Ig, you must call igSortBlockOwner->updateSortBlock()
	 *	If the sortBlock has many Igs, you can do it after updating all your igs.
	 */
	void						addInstance(CVegetableInstanceGroup *ig, 
		CVegetableShape	*shape, const NLMISC::CMatrix &mat, 
		const NLMISC::CRGBAF &ambientColor, const NLMISC::CRGBAF &diffuseColor, 
		float	bendFactor, float bendPhase, float blendDistMax);

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
	void			render(const CVector &viewCenter, const CVector &frontVector, const std::vector<CPlane> &pyramid, IDriver *driver);

	// @}


	/// \name Wind animation
	// @{

	/** set the Wind for animation.
	 *	All thoses variables may be modified each frame without penalty.
	 *
	 *	\param windDir is the direction of the wind. NB: only XY direction is kept.
	 *	\param windFreq is the frequency for the animation (speed)
	 *	\param windPower is the power of the wind, and is a factor (0..1) of Bend
	 *	\param windBendMin is a value in (0..1) which indicate how much the vegetables are bended at minimum 
	 *	(for very powerfull wind)
	 */
	void		setWind(const CVector &windDir, float windFreq, float windPower, float windBendMin);

	/** set the Wind animation Time (in seconds)
	 */
	void		setWindAnimationTime(double windTime);

	// @}


// *********************
private:
	NLMISC::CBlockMemory<CVegetableClipBlock>		_ClipBlockMemory;
	NLMISC::CBlockMemory<CVegetableSortBlock>		_SortBlockMemory;
	NLMISC::CBlockMemory<CVegetableInstanceGroup>	_InstanceGroupMemory;

	// List of ClipBlock not empty. tested for clipping
	CTessList<CVegetableClipBlock>					_ClipBlockList;
	// List of ClipBlock created, with no Ig, so not tested for clipping
	CTessList<CVegetableClipBlock>					_EmptyClipBlockList;


	// Vegetable Shape map.
	typedef	std::map<std::string, CVegetableShape>	TShapeMap;
	typedef	TShapeMap::iterator						ItShapeMap;
	TShapeMap										_ShapeMap;


	// Vertex Buffers for display. One allocator for Lighted and Unlit mode.
	CVegetableVBAllocator							_VBHardAllocator[CVegetableVBAllocator::VBTypeCount];
	// The same, but no VBHard.
	CVegetableVBAllocator							_VBSoftAllocator[CVegetableVBAllocator::VBTypeCount];
	// Vertex Program. One VertexProgram for each rdrPass
	CVertexProgram									*_VertexProgram[NL3D_VEGETABLE_NRDRPASS];


	// Material. Usefull for texture and alphaTest
	CMaterial										_VegetableMaterial;
	// Norm
	CVector											_DirectionalLight;

	// return true if the ith rdrPass is 2Sided.
	static	bool	doubleSidedRdrPass(uint rdrPass);


	/// Get the good allocator for the appropriate rdr pass.
	CVegetableVBAllocator	&getVBAllocatorForRdrPassAndVBHardMode(uint rdrPass, uint vbHardMode);


	/// init the ith vertexProgram.
	void					initVertexProgram(uint vpType);


	/** swap the RdrPass type (hard or soft) of the rdrPass of an instance group.
	 *	vertices are allocated in other VBallocator, copied and freed in the old VBallocator.
	 */
	void					swapIgRdrPassHardMode(CVegetableInstanceGroup *, uint rdrPass);


	/// \name Wind animation
	// @{
	CVector											_WindDirection;
	float											_WindFrequency;
	float											_WindPower;
	float											_WindBendMin;
	double											_WindTime;
	double											_WindPrecRenderTime;
	// updated at each render().
	double											_WindAnimTime;

	// Constant LUT.
	float											_CosTable[NL3D_VEGETABLE_VP_LUT_SIZE];
	// computed at each render().
	NLMISC::CVector2f								_WindTable[NL3D_VEGETABLE_VP_LUT_SIZE];


	// @}

};


} // NL3D


#endif // NL_VEGETABLE_MANAGER_H

/* End of vegetable_manager.h */
