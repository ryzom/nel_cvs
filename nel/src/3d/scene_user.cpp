/** \file scene_user.cpp
 * <File description>
 *
 * $Id: scene_user.cpp,v 1.9 2001/08/30 10:07:12 corvazier Exp $
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

#include "3d/scene_user.h"
#include "3d/coarse_mesh_manager.h"

using namespace NLMISC;

namespace NL3D 
{

// ***************************************************************************
UAnimationSet			*CSceneUser::createAnimationSet() 
{
	return new CAnimationSetUser();
}
// ***************************************************************************
UAnimationSet			*CSceneUser::createAnimationSet(const std::string &animationSetFile) 
{
	NLMISC::CIFile	f;
	// throw exception if not found.
	std::string	path= CPath::lookup(animationSetFile);
	f.open(path);
	return _AnimationSets.insert(new CAnimationSetUser(f));
}
// ***************************************************************************
void			CSceneUser::deleteAnimationSet(UAnimationSet	*animationSet) 
{
	_AnimationSets.erase((CAnimationSetUser*)animationSet, "deleteAnimationSet(): Bad AnimationSet ptr");
}
// ***************************************************************************
UPlayListManager			*CSceneUser::createPlayListManager() 
{
	return _PlayListManagers.insert(new CPlayListManagerUser());
}
// ***************************************************************************
void			CSceneUser::deletePlayListManager(UPlayListManager	*playListManager) 
{
	_PlayListManagers.erase((CPlayListManagerUser*)playListManager, "deletePlayListManager(): Bad PlayListManager ptr");
}

// ***************************************************************************

void			CSceneUser::setPolygonBalancingMode(CSceneUser::TPolygonBalancingMode polBalMode)
{
	nlassert( (uint)CScene::CountPolygonBalancing == (uint)CSceneUser::CountPolygonBalancing );
	_Scene.setPolygonBalancingMode((CScene::TPolygonBalancingMode)(uint)(polBalMode));
}

// ***************************************************************************

CSceneUser::TPolygonBalancingMode	CSceneUser::getPolygonBalancingMode() const
{
	nlassert( (uint)CScene::CountPolygonBalancing == (uint)CSceneUser::CountPolygonBalancing );
	return (CSceneUser::TPolygonBalancingMode)(uint)_Scene.getPolygonBalancingMode();
}

// ***************************************************************************

void				CSceneUser::setLoadMaxPolygon(uint nFaces)
{
	_Scene.setLoadMaxPolygon(nFaces);
}

// ***************************************************************************

uint				CSceneUser::getLoadMaxPolygon() const
{
	return _Scene.getLoadMaxPolygon();
}

// ***************************************************************************

float				CSceneUser::getNbFaceAsked () const
{
	return _Scene.getNbFaceAsked ();
}

// ***************************************************************************

void CSceneUser::setStaticCoarseMeshManagerTexture (const char *sPath)
{
	// Get the manager
	CCoarseMeshManager *manager=_Scene.getStaticCoarseMeshManager ();

	// Does it exist ?
	if (manager)
	{
		// Set the texture
		manager->setTextureFile (sPath);
	}
}

// ***************************************************************************

void CSceneUser::setDynamicCoarseMeshManagerTexture (const char *sPath)
{
	// Get the manager
	CCoarseMeshManager *manager=_Scene.getDynamicCoarseMeshManager ();

	// Does it exist ?
	if (manager)
	{
		// Set the texture
		manager->setTextureFile (sPath);
	}
}

// ***************************************************************************

void CSceneUser::setStaticCoarseMeshManagerColor (const CRGBA& color)
{
	// Get the manager
	CCoarseMeshManager *manager=_Scene.getStaticCoarseMeshManager ();

	// Does it exist ?
	if (manager)
	{
		// Set the texture
		manager->setColor (color);
	}
}

// ***************************************************************************

void CSceneUser::setDynamicCoarseMeshManagerColor (const CRGBA& color)
{
	// Get the manager
	CCoarseMeshManager *manager=_Scene.getDynamicCoarseMeshManager ();

	// Does it exist ?
	if (manager)
	{
		// Set the texture
		manager->setColor (color);
	}
}

} // NL3D
