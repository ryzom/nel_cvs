/** \file driver_user.cpp
 * <File description>
 *
 * $Id: driver_user2.cpp,v 1.5 2001/04/18 10:40:22 besson Exp $
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

// ***************************************************************************
// THIS FILE IS DIVIDED IN TWO PARTS BECAUSE IT MAKES VISUAL CRASH.
// fatal error C1076: compiler limit : internal heap limit reached; use /Zm to specify a higher limit
// ***************************************************************************

#include "nel/3d/driver_user.h"
#include "nel/3d/scene_user.h"
#include "nel/3d/text_context_user.h"
#include "nel/3d/tmp/u_driver.h"
#include "nel/3d/dru.h"
#include "nel/3d/scene.h"
#include "nel/3d/texture_user.h"
#include "nel/3d/material_user.h"
#include "nel/3d/tmp/u_camera.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"


using namespace std;
using namespace NLMISC;


namespace NL3D 
{


// ***************************************************************************
// ***************************************************************************
// Component Management.
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
UScene			*CDriverUser::createScene() 
{
	CSceneUser *pSU = new CSceneUser(this);
	pSU->getScene().setShapeBank( &_ShapeBank._ShapeBank );
	return _Scenes.insert(pSU);
}
// ***************************************************************************
void			CDriverUser::deleteScene(UScene	*scene) 
{
	_Scenes.erase((CSceneUser*)scene, "deleteScene(): Bad scene ptr");
}



// ***************************************************************************
UAnimationSet			*CDriverUser::createAnimationSet(const std::string &animationSetFile) 
{
	NLMISC::CIFile	f;
	// throw exception if not found.
	string	path= CPath::lookup(animationSetFile);
	f.open(path);
	return _AnimationSets.insert(new CAnimationSetUser(f));
}
// ***************************************************************************
void			CDriverUser::deleteAnimationSet(UAnimationSet	*animationSet) 
{
	_AnimationSets.erase((CAnimationSetUser*)animationSet, "deleteAnimationSet(): Bad AnimationSet ptr");
}
// ***************************************************************************
UPlayListManager			*CDriverUser::createPlayListManager() 
{
	return _PlayListManagers.insert(new CPlayListManagerUser());
}
// ***************************************************************************
void			CDriverUser::deletePlayListManager(UPlayListManager	*playListManager) 
{
	_PlayListManagers.erase((CPlayListManagerUser*)playListManager, "deletePlayListManager(): Bad PlayListManager ptr");
}



// ***************************************************************************
UTextContext	*CDriverUser::createTextContext(const std::string fontFileName, const std::string fontExFileName) 
{
	return _TextContexts.insert(new CTextContextUser(fontFileName, fontExFileName, this, &_FontManager));
}
// ***************************************************************************
void			CDriverUser::deleteTextContext(UTextContext	*textContext) 
{
	_TextContexts.erase((CTextContextUser*)textContext, "deleteTextContext: Bad TextContext");
}
// ***************************************************************************
void			CDriverUser::setFontManagerMaxMemory(uint maxMem) 
{
	_FontManager.setMaxMemory(maxMem);
}
// ***************************************************************************
std::string		CDriverUser::getFontManagerCacheInformation() const
{
	return _FontManager.getCacheInformation();
}



// ***************************************************************************
UTextureFile	*CDriverUser::createTextureFile(const std::string &file) 
{
	CTextureFileUser	*text= new CTextureFileUser(file);
	_Textures.insert(text);
	return text;
}
// ***************************************************************************
void			CDriverUser::deleteTextureFile(UTextureFile *textfile) 
{
	_Textures.erase(dynamic_cast<CTextureFileUser*>(textfile), "deleteTextureFile: Bad textfile");
}
// ***************************************************************************
UTextureRaw		*CDriverUser::createTextureRaw() 
{
	nlstop; // Not implemented!!

	return NULL;
}
// ***************************************************************************
void			CDriverUser::deleteTextureRaw(UTextureRaw *textraw) 
{
	nlstop; // Not implemented!!

	delete textraw;
}
// ***************************************************************************
UMaterial		*CDriverUser::createMaterial() 
{
	return _Materials.insert(new CMaterialUser);
}
// ***************************************************************************
void			CDriverUser::deleteMaterial(UMaterial *umat) 
{
	_Materials.erase( (CMaterialUser*)umat, "deleteMaterial: Bad material");
}


} // NL3D
