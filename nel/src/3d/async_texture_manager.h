/** \file async_texture_manager.h
 * <File description>
 *
 * $Id: async_texture_manager.h,v 1.1 2002/10/10 12:55:44 berenguier Exp $
 */

/* Copyright, 2000-2002 Nevrax Ltd.
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

#ifndef NL_ASYNC_TEXTURE_MANAGER_H
#define NL_ASYNC_TEXTURE_MANAGER_H


#include "nel/misc/types_nl.h"
#include "3d/texture_file.h"
#include <vector>


namespace NL3D 
{


class	CMeshBaseInstance;


// ***************************************************************************
/**
 * <Class description>
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CAsyncTextureManager
{
public:

	/// Constructor
	CAsyncTextureManager();
	~CAsyncTextureManager();

	/** setup the mipMap levels. 
	 *	\baseLevel When the texture is first added, it is loaded skipping the baseLevel
	 *	first mipmap
	 *	\maxLevel During time, furhter mipmap are loaded, according to instance position etc... maxLevel
	 *	tells where to stop. If 0, the texture will finally be entirely uploaded.
	 *	Default is 3,1.
	 */
	void			setupLod(uint baseLevel, uint maxLevel);
	/// Setup max texture upload in driver per update() call.
	void			setupMaxUploadPerFrame(uint maxup);

	/** Add a reference to a texture owned by an instance. Begin Async loading if was not added before.
	 *	ThereFore, only CTextureFile are possible. Note also that the texture is uploaded with mipmap by default, and
	 *	UpLoadFormat is also default (say ITexture::Auto)
	 *
	 *	If the texture file is not a DDS with mipmap, this is an error. But the system doens't fail and
	 *	the file is entirely loaded and uploaded. The problem is that upload is not cut according to maxUpLoadPerFrame, so
	 *	some freeze may occur.
	 */
	uint			addTextureRef(const std::string &textName, CMeshBaseInstance *instance);

	/// release a texture-instance tuple. the texture is released if no more instance use it.
	void			releaseTexture(uint id, CMeshBaseInstance *instance);

	/// tells if a texture is loaded in the driver (ie ready to use)
	bool			isTextureUpLoaded(uint id) const;

	/** update the manager. New loaded texture are uploaded. Instances are updated to know if all their 
	 *	pending textures have been uploaded.
	 */
	void			update(IDriver *pDriver);


// ***************************************************************************
private:

	typedef	std::map<std::string, uint>	TTextureEntryMap;
	typedef	TTextureEntryMap::iterator	ItTextureEntryMap;

	struct	CTextureEntry
	{
		CTextureEntry();

		// The it in the map.
		ItTextureEntryMap					ItMap;
		// the texture currently loaded / uploaded.
		NLMISC::CSmartPtr<CTextureFile>		Texture;
		// true if async loading has ended
		bool								Loaded;
		// true if the texture is loaded in the driver (at least the coarsest level).
		bool								UpLoaded;
		// true if DXTC with mipmap
		bool								CanHaveLOD;
		// If the CanHaveLod, this is the base Size (ie with no extra LOD loaded) this texture takes in VRAM
		uint								BaseSize;
		// list of instances currently using this texture.
		std::vector<CMeshBaseInstance*>		Instances;
	};


	std::vector<CTextureEntry*>			_TextureEntries;
	TTextureEntryMap					_TextureEntryMap;
	std::vector<uint>					_WaitingTextures;

	uint								_BaseLodLevel, _MaxLodLevel;
	uint								_MaxUploadPerFrame;

	// Upload of texture piece by piece.
	CTextureEntry						*_CurrentUploadTexture;
	uint								_CurrentUploadTextureMipMap;
	uint								_CurrentUploadTextureLine;


private:

	static bool		validDXTCMipMap(ITexture *pText);

	// delete the texture and all references in map/array, instance refcount etc...
	void			deleteTexture(uint id);

	// Fill _CurrentUploadTexture with next texture to upload, or set NULL if none
	void			getNextTextureToUpLoad();
	bool			uploadTexturePart(ITexture *pText, IDriver *pDriver, uint &nTotalUpload);


	// Fill _CurrentUploadTexture with next texture to upload

/*	struct	CTextureKey
	{
		// A Ptr on the real texture used.
		CTextureEntry						*TextureEntry;
		// The texture used to load async the file.
		NLMISC::CSmartPtr<CTextureFile>		TextureLoad;
		// the level of this Lod. 0 means full original texture resolution.
		uint8								Level;
		// True if TextureEntry has at least this lod in VRAM
		bool								Loaded;
		// The size that this lod takes in VRAM.
		uint								Size;
	};
	std::vector<CTextureKey*>			_TextureKeys;
*/
};


} // NL3D


#endif // NL_ASYNC_TEXTURE_MANAGER_H

/* End of async_texture_manager.h */
