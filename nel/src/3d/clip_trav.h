/** \file clip_trav.h
 * <File description>
 *
 * $Id: clip_trav.h,v 1.14 2002/11/14 12:55:01 berenguier Exp $
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

#ifndef NL_CLIP_TRAV_H
#define NL_CLIP_TRAV_H

#include "3d/trav_scene.h"
#include "3d/quad_grid.h"
#include "nel/misc/vector.h"
#include "nel/misc/plane.h"
#include "nel/misc/matrix.h"


namespace	NL3D
{

using NLMISC::CVector;
using NLMISC::CPlane;
using NLMISC::CMatrix;


class	IBaseClipObs;
class	CRenderTrav;
class	CAnimDetailTrav;
class	CLoadBalancingTrav;
class	CHrcTrav;
class	CLightTrav;
class	CCluster;
class	CInstanceGroup;
class	CCamera;
class	CQuadGridClipManager;
class	CRootModel;
class	CTransformClipObs;


// ***************************************************************************
// This is the order of clip planes.
#define	NL3D_CLIP_PLANE_NEAR	0
#define	NL3D_CLIP_PLANE_FAR		1
#define	NL3D_CLIP_PLANE_LEFT	2
#define	NL3D_CLIP_PLANE_TOP		3
#define	NL3D_CLIP_PLANE_RIGHT	4
#define	NL3D_CLIP_PLANE_BOTTOM	5


// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		ClipTravId=NLMISC::CClassId(0x135208fe, 0x225334fc);

// ***************************************************************************
/**
 * The clip traversal.
 * The purpose of this traversal is to insert in the post-clip Traversal the observers which are 
 * said to be not clipped. Some observers may do something else.
 *
 * Observer should use the IBaseHrcObs->clip() method to implement their observer, or completly redefine the traverse() method.
 *
 * \b USER \b RULES: Before using traverse() on a clip traversal, you should:
 *	- setFrustum() the camera shape (focale....)
 *	- setCamMatrix() for the camera transform
 *	- setHrcTrav(),setRenderTrav()...., to setup shortcuts to other traversals
 *
 * NB: see CScene for 3d conventions (orthonormal basis...)
 * \sa CScene IBaseClipObs
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CClipTrav : public ITravCameraScene
{
public:

	/// Constructor
	CClipTrav();
	~CClipTrav();

	/// \name ITrav/ITravScene Implementation.
	//@{
	IObs				*createDefaultObs () const;
	NLMISC::CClassId	getClassId () const {return ClipTravId;}
	sint				getRenderOrder () const {return 2000;}
	void				traverse ();
	//@}

	void registerCluster (CCluster* pCluster);
	void unregisterCluster (CCluster* pCluster);

	/// Setup the render traversal (else traverse() won't work)
	void setHrcTrav (CHrcTrav* trav);
	void setAnimDetailTrav(CAnimDetailTrav *trav);
	void setLoadBalancingTrav(CLoadBalancingTrav *trav);
	void setLightTrav (CLightTrav* trav);
	void setRenderTrav (CRenderTrav* trav);
	void setQuadGridClipManager(CQuadGridClipManager *mgr);


	bool fullSearch (std::vector<CCluster*>& result, CInstanceGroup *pIG, CVector& pos);

	/// \name Visible List mgt. Those visible observers are updated each traverse(). Only support Transform Type obs.
	//@{
	// For ClipObservers only. NB: list is cleared at begining of traverse().
	void				addVisibleObs(CTransformClipObs *obs);
	//@}


	void				setSonsOfAncestorSkeletonModelGroup(CRootModel *m);

public:

	/** \name FOR OBSERVERS ONLY.  (Read only)
	 * Those variables are valid only in traverse().
	 */
	//@{
	/// Vision Pyramid (6 normalized planes) in the view basis.
	std::vector<CPlane>	ViewPyramid;
	/// Vision Pyramid (6 normalized planes) in the world basis. NB: NOT modified by the ClusterSystem.
	std::vector<CPlane>	WorldFrustumPyramid;
	/// Vision Pyramid in the world basis. NB: may be modified by the ClusterSystem.
	std::vector<CPlane>	WorldPyramid;	
	/// Shortcut to the Rdr Traversals (to add the models rdr observers).
	CHrcTrav			*HrcTrav;
	CAnimDetailTrav		*AnimDetailTrav;
	CLoadBalancingTrav	*LoadBalancingTrav;
	CLightTrav			*LightTrav;
	CRenderTrav			*RenderTrav;
	//@}
	sint64 CurrentDate;

	CCluster *RootCluster;
	CCamera *Camera;
	
	CQuadGrid<CCluster*> Accel;

	/** for CQuadGridClipClusterClipObs only. This flag means observers traversed do not need to clip,
	 *	they are sure to be visible.
	 */
	bool				ForceNoFrustumClip;


	// For skeleton stickObject.
	CRootModel		*SonsOfAncestorSkeletonModelGroup;

// **********************
private:
	friend class	IBaseClipObs;

	std::vector<CTransformClipObs*>	_VisibleList;

	CQuadGridClipManager		*_QuadGridClipManager;

	// For skeleton CLod Load balancing
	struct	CSkeletonKey
	{
		uint				Priority;
		CSkeletonModel		*SkeletonModel;

		bool	operator<(const CSkeletonKey &k) const
		{
			return Priority<k.Priority;
		}
	};
	std::vector<CSkeletonKey>	_TmpSortSkeletons;

	void	loadBalanceSkeletonCLod();
};



}


#endif // NL_CLIP_TRAV_H

/* End of clip_trav.h */
