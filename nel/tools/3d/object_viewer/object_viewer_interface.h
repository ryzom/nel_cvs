/** \file object_viewer_interface.cpp
 *
 * $Id: object_viewer_interface.h,v 1.14 2002/02/26 17:30:23 corvazier Exp $
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

#ifndef OBJECT_VIEWER_INTERFACE
#define OBJECT_VIEWER_INTERFACE

#ifndef OBJECT_VIEWER_EXPORT
#define OBJECT_VIEWER_EXPORT __declspec( dllimport ) 
#endif // OBJECT_VIEWER_EXPORT

// Increment this version number each time you distribute a new version of the dll.
#define OBJECT_VIEWER_VERSION 3

namespace NL3D
{
	class IShape;
	class CAnimation;
	class CLight;
	class CTransformShape;
	class CSkeletonModel;
	class CWaterPoolManager;
}

namespace NLMISC
{
	class CRGBA;
}

class IObjectViewer
{
public:
	virtual ~IObjectViewer () {};

	// Init the UI
	virtual void initUI (HWND parent=NULL)=0;

	// Go
	virtual void go ()=0;

	// Release the UI
	virtual void releaseUI ()=0;

	// Add a mesh
	virtual NL3D::CTransformShape	*addMesh (NL3D::IShape* pMeshShape, const char* meshName, const char *meshBaseName, NL3D::CSkeletonModel* pSkel, bool createInstance= true) =0;

	// Add a skel
	virtual NL3D::CSkeletonModel	*addSkel (NL3D::IShape* pSkelShape, const char* skelName, const char *skelBaseName) =0;

	// remove all the instance from the scene
	virtual void					 removeAllInstancesFromScene()=0;

	// Load a mesh
	virtual bool loadMesh (std::vector<std::string> &meshFilename, const char* skeleton)=0;

	// Load a shape
	virtual void resetCamera ()=0;

	// Set single animation
	virtual void setSingleAnimation (NL3D::CAnimation* pAnim, const char* name)=0;

	// Set automatic animation
	virtual void setAutoAnimation (NL3D::CAnimation* pAnim)=0;

	// Set ambient color
	virtual void setAmbientColor (const NLMISC::CRGBA& color)=0;

	// Set background color
	virtual void setBackGroundColor (const NLMISC::CRGBA& color)=0;

	// Set ambient color
	virtual void setLight (unsigned char id, const NL3D::CLight& light)=0;

	/** set the water pool manager used by the object viewer. Must be the same than tyhe one of the dll which created the models 
	  * (because the 3d lib is duplicated : one version in the viewer, and one version in the exporter)
	  */
	virtual void setWaterPoolManager(NL3D::CWaterPoolManager &wpm)=0;

	/** Add an InstanceGroup. ptr Will be deleted by objectViewer.
	 */
	virtual void addInstanceGroup(NL3D::CInstanceGroup *ig)=0;

	/** Setup Scene lighting System. Disabled by default
	 */
	virtual void setupSceneLightingSystem(bool enable, const NLMISC::CVector &sunDir, NLMISC::CRGBA sunAmbiant, NLMISC::CRGBA sunDiffuse, NLMISC::CRGBA sunSpecular)=0;


	// Get instance
	static OBJECT_VIEWER_EXPORT IObjectViewer* getInterface (int version=OBJECT_VIEWER_VERSION);

	// Release instance
	static OBJECT_VIEWER_EXPORT void releaseInterface (IObjectViewer* view);
};

#endif OBJECT_VIEWER_INTERFACE
