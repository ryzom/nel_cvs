/** \file clip_trav.h
 * <File description>
 *
 * $Id: clip_trav.h,v 1.10 2002/06/12 12:26:57 berenguier Exp $
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
class	IBaseHrcObs;
class	IBaseRenderObs;
class	IBaseAnimDetailObs;
class	IBaseLoadBalancingObs;
class	CRenderTrav;
class	CHrcTrav;
class	CLightTrav;
class	CCluster;
class	CInstanceGroup;
class	CCamera;
class	CQuadGridClipManager;
class	CRootModel;


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
 * The purpose of this traversal is to insert in the RenderTraversal the observers which are 
 * said to be not clipped. Some observers may do something else.
 *
 * Observer should use the IBaseHrcObs->clip() method to implement their observer, or completly redefine the traverse() method.
 *
 * \b USER \b RULES: Before using traverse() on a clip traversal, you should:
 *	- setFrustum() the camera shape (focale....)
 *	- setCamMatrix() for the camera transform
 *	- setRenderTrav(), to setup the rendertraversal where observers will be cleared / inserted.
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
	void setRenderTrav (CRenderTrav* trav);
	void setHrcTrav (CHrcTrav* trav);
	void setLightTrav (CLightTrav* trav);
	void setQuadGridClipManager(CQuadGridClipManager *mgr);


	bool fullSearch (std::vector<CCluster*>& result, CInstanceGroup *pIG, CVector& pos);

	/// \name Visible List mgt. Those visible observers are updated each traverse().
	//@{
	uint				numVisibleObs() const {return _VisibleList.size();}
	IBaseClipObs		*getVisibleObs(uint i) const {return _VisibleList[i];}

	// For ClipObservers only. NB: list is cleared at begining of traverse().
	void				addVisibleObs(IBaseClipObs *obs);
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
	CRenderTrav		*RenderTrav;
	CHrcTrav		*HrcTrav;
	CLightTrav		*LightTrav;
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

private:
	friend class	IBaseClipObs;

	std::vector<IBaseClipObs*>	_VisibleList;

	CQuadGridClipManager		*_QuadGridClipManager;
};


// ***************************************************************************
/**
 * The base interface for clip traversal.
 * Clip observers MUST derive from IBaseClipObs.
 * This observer:
 * - leave the notification system to DO NOTHING.
 * - implement the init() method, to set shortcut to neighbor observers.
 *
 * \b DERIVER \b RULES:
 * - implement the notification system (see IObs and IObs() for details).
 * - implement the clip() method.
 * - implement the traverse(), which should call clip() and isRenderable(). see CTransform for an implementation.
 * - possibly modify/extend the graph methods (such as a graph behavior).
 *
 * \sa CClipTrav
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class IBaseClipObs : public IObs
{
public:
	/// Shortcut to observers.
	IBaseHrcObs		*HrcObs;
	IBaseRenderObs	*RenderObs;
	IBaseAnimDetailObs		*AnimDetailObs;
	IBaseLoadBalancingObs	*LoadBalancingObs;

	/** OUT variable (good after traverse()).
	 * set to true is the object is visible (not clipped).
	 */
	bool			Visible;

	enum TClipReason	{FrustumClip=0, DistMaxClip, countClip};

public:
	

	/// Constructor.
	IBaseClipObs()
	{
		HrcObs=NULL;
		RenderObs= NULL;
		Visible=false;
		_IndexInVisibleList= -1;
	}
	// Dtor: remove me from _VisibleList.
	virtual	~IBaseClipObs();

	/// Build shortcut to HrcObs and RenderObs.
	virtual	void	init();



	/** Should return true if object has to be inserted in RenderTrav list.
	 *	eg: a mesh must be inserted in a render list, but not a light, or a NULL transform.
	 */
	virtual	bool	isRenderable() const =0;


	/** Should return true if object is visible (eg in frustum)
	 * \param caller the caller obs (may NULL)
	 */
	virtual	bool	clip(IBaseClipObs *caller)=0;


	/** The base doit method.
	 * The default behavior should be:
	 *	- test if HrcObs->WorldVis is visible.
	 *	- test if the observer is clipped with clip()
	 *	- if visible and not clipped, set \c Visible=true (else false).
	 *	- if Visible==true, and renderable, add it to the RenderTraversal: \c RenderTrav->addRenderObs(RenderObs);
	 *	- always traverseSons(), to clip the sons.
	 */
	virtual	void	traverse(IObs *caller) =0;


	/** This method is called when a father want to clip all his sons (eg: QuadGridClipCluster)
	 *  The default behavior is just to call this method on observer sons
	 *	\param TClipReason is the reason of the forced clip. either a frustrum clip or a DistMax Clip
	 */
	virtual	void	forceClip(TClipReason clipReason);


	/** 
	 * Because the clip traversal is a graph of observer not a hierarchy
	 */
	virtual bool	isTreeNode() {return false;}

private:
	friend class	CClipTrav;

	// The index of the Observer in the _VisibleList; -1 (default) means not in
	sint			_IndexInVisibleList;
};


// ***************************************************************************
/**
 * The default clip observer, used by unspecified models.
 * This observer:
 * - leave the notification system to DO NOTHING.
 * - implement the clip() method to return true.
 * - leave the traverse() method as IBaseClipObs.
 *
 * \sa IBaseClipObs
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CDefaultClipObs : public IBaseClipObs
{
public:

	/// don't render.
	virtual	bool	isRenderable() const {return false;}

	/// Don't clip.
	virtual	bool	clip(IBaseClipObs *caller) {return true;}


	/// just traverseSons().
	virtual	void	traverse(IObs *caller)
	{
		traverseSons();
	}

};



}


#endif // NL_CLIP_TRAV_H

/* End of clip_trav.h */
