/** \file object_viewer_interface.cpp
 *
 * $Id: object_viewer_interface.h,v 1.4 2001/04/26 17:57:41 corvazier Exp $
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
#define OBJECT_VIEWER_VERSION 2

namespace NL3D
{
	class IShape;
	class CAnimation;
	class CLight;
	class CTransformShape;
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
	virtual void initUI ()=0;

	// Go
	virtual void go ()=0;

	// Release the UI
	virtual void releaseUI ()=0;

	// Add a mesh
	virtual NL3D::CTransformShape	*addMesh (NL3D::IShape* pMeshShape, NL3D::IShape* pSkelShape, const char* name, const char *animBaseName)=0;

	// Load a mesh
	virtual bool loadMesh (const char* meshFilename, const char* skeleton)=0;

	// Load a shape
	virtual void resetCamera ()=0;

	// Set single animation
	virtual void setSingleAnimation (NL3D::CAnimation* pAnim, const char* name)=0;

	// Set ambient color
	virtual void setAmbientColor (const NLMISC::CRGBA& color)=0;

	// Set ambient color
	virtual void setLight (unsigned char id, const NL3D::CLight& light)=0;

	// Get instance
	static OBJECT_VIEWER_EXPORT IObjectViewer* getInterface (int version=OBJECT_VIEWER_VERSION);

	// Release instance
	static OBJECT_VIEWER_EXPORT void releaseInterface (IObjectViewer* view);
};

#endif OBJECT_VIEWER_INTERFACE
