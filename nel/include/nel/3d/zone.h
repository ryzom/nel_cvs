/** \file zone.h
 * <File description>
 *
 * $Id: zone.h,v 1.13 2000/12/13 12:54:07 berenguier Exp $
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

#ifndef NL_ZONE_H
#define NL_ZONE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/stream.h"
#include "nel/misc/debug.h"
#include "nel/3d/tessellation.h"
#include "nel/3d/patch.h"
#include "nel/3d/bezier_patch.h"
#include <vector>
#include <map>


namespace NL3D 
{


class CZone;
class CLandscape;


#define	NL3D_NOISE_MAX	10

// ***************************************************************************
typedef	std::map<uint16, CZone*>			TZoneMap;
typedef	std::map<uint16, CZone*>::iterator	ItZoneMap;


// ***************************************************************************
/**
 * The struct for connectivity of zone vertices. 
 */
struct	CBorderVertex
{
	// The index of vertex in the current zone to bind.
	uint16			CurrentVertex;
	// The neighbor zone Id.
	uint16			NeighborZoneId;
	// The index of vertex in the neighbor zone to bind to CurrentVertex.
	uint16			NeighborVertex;

	void			serial(NLMISC::IStream &f);
};


// ***************************************************************************
/**
 * The struct for building a patch. 
 * NB: Different from the one which is stored.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
struct	CPatchInfo
{
public:

	/** A bind Info on a edge of a patch.
	 * Entry 0 is only usefull for Bind One/One+
	 * Entry 1 is only usefull for Bind One/Two+
	 * Entry 2/3 is only usefull for Bind One/Four.
	 */
	struct	CBindInfo
	{
		/** The number of patchs on this edge. 0,1, 2 or 4.  0 means no neigbor on this edge. \b 5 is a special code, 
		 * which means the same thing than NPatchs==1, but "I am one of the little patch connected to the bigger neigbor".
		 * Like when NPatchs==1, ZoneId, Next[0] and Edge[0] must be valid.
		 */
		uint8			NPatchs;

		/// The neighbor zone of all neigbor patch. Often the same zone as the patch (but on zone border).
		uint16			ZoneId;
		/// The neighbor patch i.
		uint16			Next[4];
		/// On which edge of Nexti we are binded.
		uint8			Edge[4];

	public:
		void			serial(NLMISC::IStream &f);
		CBindInfo() {NPatchs=0;}
	};

	
public:
	/// \name Patch geometry.
	// @{
	/// The patch coordinates.
	CBezierPatch	Patch;
	/// Tile Order for the patch.
	uint8			OrderS, OrderT;
	/// The Base Size*bumpiness of the patch (/2 at each subdivide). Set to 0, if you want CZone to compute it for you.
	float			ErrorSize;
	/// The base corner vertices indices in the current zone. Used for patch connectivity.
	uint16			BaseVertices[4];
	// @}


	/// \name Patch texture.
	// @{
	/** The Tiles for this patch. There must be OrderS*OrderT tiles.
	 * They are stored in line first order, from S=0 to 1, and T=0 to 1.
	 */
	std::vector<CTileElement>	Tiles;
	// @}


	/// \name Patch Binding.
	// @{
	CBindInfo		BindEdges[4];
	// @}


public:
	CPatchInfo()
	{
		ErrorSize= 0;
	}
};


// ***************************************************************************
/**
 * A landscape zone.
 * There is 2 ways for building a zone:
 *	- use build(). (then you can use serial to save the zone, don't need to compile() the zone).
 *	- use serial() for loading this zone.
 *
 * Before a zone may be rendered, it must be compile()-ed, to compile and bind patch, to make vertices etc...
 *
 * NB: you must call release() before deleting a compiled zone. (else assert in destruction).
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CZone
{
public:
	// The stored patch structure for compile() - ation.
	struct	CPatchConnect
	{
		// NB: same meanings than in CPatchInfo.
		uint8			OrderS, OrderT;
		float			ErrorSize;
		uint16			BaseVertices[4];
		CPatchInfo::CBindInfo		BindEdges[4];

	public:
		void			serial(NLMISC::IStream &f);
	};

public:

	/// Constructor
	CZone();
	/// Destructor
	~CZone();


	/** Build a zone.
	 * This method do:
	 *	- compress the patchs coordinates.
	 *	- build the patchs of the zone, but doesn't compile() them.
	 *
	 * NB: cannot build on a compiled zone. must release the zone before....
	 *
	 * \param zoneId the Unique ID of this zone.
	 * \param patchs the PatchInfo of this zone.
	 * \param borderVertices vertices connectivity for this zone. NB: borderVertices must contains the connectivity 
	 *		across zones. It is VERY IMPORTANT to setup zone corner connectivity too. A "corner borderVertex" may appear 
	 *		3 times here. One for each other zone of the corner.
	 */
	void			build(uint16 zoneId, const std::vector<CPatchInfo> &patchs, const std::vector<CBorderVertex> &borderVertices);


	/** Build a copy of a zone.
	 * This method do a copy of zone (should be builded but maybe not compiled).
	 *
	 * NB: cannot build on a compiled zone. must release the zone before....
	 */
	void			build(const CZone &zone);


	/** Retrieve zone patchinfo.
	 * This method uncompress the patchs coordinates and all info into the patch info/borderVertices.
	 * Warning!!! Due to compression, data won't be the same as those given in build().
	 *
	 * \param patchs the PatchInfo of this zone.
	 * \param borderVertices vertices connectivity for this zone.
	 */
	void			retrieve(std::vector<CPatchInfo> &patchs, std::vector<CBorderVertex> &borderVertices);


	/** Compile a zone. Make it usable for clip()/refine()/render().
	 * This method do:
	 *	- attach this to loadedZones.
	 *	- create/link the base vertices (internal..), according to present neigbor zones.
	 *	- compile() the patchs.
	 *	- bind() the patchs.
	 *	- rebindBorder() on neighbor zones.
	 *
	 * A zone must keep a pointer on a landscape, for texture management.
	 * NB: assert if already compiled.
	 * assert if zone already exist in loadedZones.
	 */
	void			compile(CLandscape *landscape, TZoneMap &loadedZones);

	/** Release a zone.
	 * This method do:
	 *	- detach this zone to loadedZones.
	 *	- destroy/unlink the base vertices (internal..), according to present neigbor zones.
	 *	- unbind() the patchs.
	 *	- release() the patchs.
	 *	- rebindBorder() on neighbor zones.
	 *
	 * NB: no-op if not compiled.
	 */
	void			release(TZoneMap &loadedZones);


	/** Load/save a zone.
	 * Save work even if zone is not compiled, but load must be done on a not compiled zone...
	 */
	void			serial(NLMISC::IStream &f);


	/** 
	 * Update and refresh a patch texture.
	 * Usefull for Tile edition. Even if patch is in tile mode, it is refreshed...
	 * \param numPatch the index of patch in this zone which will receive his new texture. assert if bad id.
	 * \param tiles the patch texture. assert if not of good size (OrderS*OrderT).
	 */
	void			changePatchTexture(sint numPatch, const std::vector<CTileElement> &tiles);

	/** 
	 * Get a patch texture.
	 * Return the tile array.
	 * \param numPatch the index of patch in this zone which will get his texture. assert if bad id.
	 * \param 
	 * \return The tiles the patch texture. The size should be OrderS*OrderT.
	 * \see getPatch()
	 */
	const std::vector<CTileElement> &getPatchTexture(sint numPatch) const;


	// NB: for all those function, CTessFace static rendering context must be setup.
	/// Clip a zone. To know if must be rendered etc... A zone is IN if in BACK of at least one plane of the pyramid.
	void			clip(const std::vector<CPlane>	&pyramid);
	/// Refine a zone (if needed).
	void			refine();
	/// PreRender a zone (if needed).
	void			preRender();
	/// Render pass (if needed).
	void			renderFar0();
	void			renderFar1();
	void			renderTile(sint pass);


	// Accessors.
	const CVector	&getPatchBias() const {return PatchBias;}
	float			getPatchScale() const {return PatchScale;}
	bool			compiled() const {return Compiled;}
	uint16			getZoneId() const {return ZoneId;}
	sint			getNumPatchs() const {return Patchs.size();}
	// Return the Bounding Box of the zone.
	const CAABBoxExt	&getZoneBB() const {return ZoneBB;}

	/** 
	 * Get a read only patch pointer.
	 *
	 * \param patch the index of patch to get.
	 * \return A patch pointer in read only.
	 */
	const CPatch	*getPatch(sint patch) const {nlassert(patch>=0 && patch<(sint)Patchs.size()); return &(Patchs[patch]);}

	/** 
	 * Get a read only patch connect pointer.
	 *
	 * \param patch the index of patch to get.
	 * \return A patch pointer in read only.
	 */
	const CPatchConnect	*getPatchConnect(sint patch) const 
		{nlassert(patch>=0 && patch<(sint)Patchs.size()); return &(PatchConnects[patch]);}


// Private part.
private:
/*********************************/
	// A smartptrisable vertex.
	struct	CTessBaseVertex : public NLMISC::CRefCount
	{
		CTessVertex		Vert;
	};

	// Zone vertices.
	typedef	NLMISC::CSmartPtr<CTessBaseVertex>	PBaseVertex;
	typedef	std::vector<PBaseVertex>			TBaseVerticesVec;


private:
	// The lanscape which own this zone. Usefull for texture management.
	// Filled at compilation only.
	CLandscape		*Landscape;

	// Misc.
	uint16			ZoneId;
	bool			Compiled;
	CAABBoxExt		ZoneBB;
	CVector			PatchBias;
	float			PatchScale;

	// The number of vertices she access (maybe on border).
	sint32				NumVertices;
	// The smartptr on zone vertices.
	TBaseVerticesVec	BaseVertices;
	// The list of border vertices.
	std::vector<CBorderVertex>	BorderVertices;
	// NB: No problem on corners, since zones are compile()-ed with knowledge of neighbors.

	// The patchs.
	std::vector<CPatch>			Patchs;
	std::vector<CPatchConnect>	PatchConnects;

	
private:
	friend	class CTessFace;
	// Should do this, for texture mgt.
	friend	class CPatch;

	// Local info for CTessFace tiles. CZone must setup them at the begining at refine()/render().
	// Should we compute the error metric part for tile?? Stored by Zone. By patch, it is Too slow, regarding
	// the cost of computing this errormetric in CTessFace::updateErrorMetric().
	bool			ComputeTileErrorMetric;
	// REMIND: can't have any patch/zone global, since a propagated split()/updateErrorMetric() can arise.

	std::vector<CPlane>	CurrentPyramid;
	sint			ClipResult;

private:
	/**
	 * Force border patchs (those who don't bind to current zone) to re bind() them, using new neighborood.
	 * no-op if zone is not compiled.
	 */
	void			rebindBorder(TZoneMap &loadedZones);

	PBaseVertex		getBaseVertex(sint vert) const {return BaseVertices[vert];}
	CPatch			*getPatch(sint patch) {nlassert(patch>=0 && patch<(sint)Patchs.size()); return &(Patchs[patch]);}
	static CPatch	*getZonePatch(TZoneMap &loadedZones, sint zoneId, sint patch);
	// Bind the patch with ones which are loaded...
	static void		unbindAndMakeBindInfo(TZoneMap &loadedZones, CPatch &pa, CPatchConnect &pc, CPatch::CBindInfo	edges[4]);
	static void		unbindPatch(TZoneMap &loadedZones, CPatch &pa, CPatchConnect &pc);
	static void		bindPatch(TZoneMap &loadedZones, CPatch &pa, CPatchConnect &pc);
	// Is the patch on a border of this zone???
	bool			patchOnBorder(const CPatchConnect &pc) const;
};


} // NL3D


#endif // NL_ZONE_H

/* End of zone.h */
