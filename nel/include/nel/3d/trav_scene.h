/* trav_scene.h
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: trav_scene.h,v 1.3 2000/10/17 16:51:17 berenguier Exp $
 *
 * <Replace this by a description of the file>
 */

#ifndef NL_TRAV_SCENE_H
#define NL_TRAV_SCENE_H


#include "nel/3d/mot.h"
#include "nel/misc/matrix.h"


namespace	NL3D
{


using NLMISC::CVector;
using NLMISC::CPlane;
using NLMISC::CMatrix;


// ***************************************************************************
/**
 * A Traversal which may be renderable in a CScene.
 * Such a traversal provide just a method to get his Render order, and a standard traverse() method, which compute
 * the traversal.
 *
 * \b DERIVERS \b RULES:
 * - Implement createDefaultObs()
 * - Implement getClassId()
 * - Implement getRenderOrder()
 * - Implement traverse()
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class ITravScene : public ITrav
{
public:
	/** Return the order of the traversal.
	 * This is usefull for CScene::render() method. It indicate when the traversal is called.
	 * Order for basic traversal are:
	 * - CHrcTrav:	  1000
	 * - CClipTrav:	  2000
	 * - CLightTrav:  3000
	 * - CRenderTrav: 4000
	 */
	virtual	sint	getRenderOrder() const =0;

	/** Execute the traversal.
	 * CScene::render() will call this method, to execute the traversal.
	 */
	virtual	void	traverse() =0;
};


// ***************************************************************************
/**
 * A ITravScene traversal, with camera setup  (common to CRenderTrav and CClipTrav).
 *
 */
class ITravCameraScene : public ITravScene
{
public:
	/** \name FOR OBSERVERS ONLY.  (Read only)
	 * Those variables are valid only in traverse().
	 */
	//@{
	// NB: znear and zfar are >0 (if perspective).
	float				Left, Right, Bottom, Top, Near, Far;
	bool				Perspective;
	NLMISC::CMatrix		CamMatrix;		// The camera matrix.
	NLMISC::CMatrix		ViewMatrix;		// ViewMatrix= CamMatrix.inverted();
	NLMISC::CVector		CamPos;			// The camera position in world space.
	NLMISC::CVector		CamLook;		// The Y direction of the camera in world space.
	//@}


public:
	/// Setup the camera mode as a perspective/ortho camera. NB: znear and zfar must be >0 (if perspective).
	void		setFrustum(float left, float right, float bottom, float top, float znear, float zfar, bool perspective= true)
	{
		Left= left;
		Right= right;
		Bottom=	bottom;
		Top= top;
		Near= znear;
		Far= zfar;
		Perspective= perspective;
	}
	/// Setup the camera mode as a perspective/ortho camera. NB: znear and zfar must be >0 (if perspective).
	void		setFrustum(float width, float height, float znear, float zfar, bool perspective= true)
	{
		setFrustum(-width/2, width/2, -height/2, height/2, znear, zfar, perspective);
	}
	/// Setup the camera matrix (a translation/rotation matrix).
	void		setCamMatrix(const NLMISC::CMatrix	&camMatrix)
	{
		CamMatrix= camMatrix;
	}


	/// Constructor.
	ITravCameraScene()
	{
		setFrustum(1.0f, 1.0f, 0.01f, 1.0f);
		CamMatrix.identity();
		ViewMatrix.identity();
		CamPos= NLMISC::CVector::Null;
		CamLook= NLMISC::CVector::Null;
	}


protected:

	/// update the dependent information.
	void	update()
	{
		ViewMatrix= CamMatrix.inverted();
		CamPos= CamMatrix.getPos();
		CamLook= CamMatrix.mulVector(NLMISC::CVector::J);
	}


};


}


#endif // NL_TRAV_SCENE_H

/* End of trav_scene.h */
