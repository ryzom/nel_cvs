/** \file async_file_manager.cpp
 * <File description>
 *
 * $Id: async_file_manager.cpp,v 1.5 2002/02/28 12:59:49 besson Exp $
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

#include "std3d.h"

#include "3d/async_file_manager.h"
#include "3d/shape.h"
#include "3d/mesh.h"
#include "3d/texture_file.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"

using namespace std;
using namespace NLMISC;


namespace NL3D
{

// ***************************************************************************
	
void CAsyncFileManager::loadMesh(const std::string& meshName, IShape** ppShp, IDriver *pDriver)
{
	addTask(new CMeshLoad(meshName, ppShp, pDriver) );
}

// ***************************************************************************
// TASKS
// ***************************************************************************

CAsyncFileManager::CMeshLoad::CMeshLoad(const std::string& meshName, IShape** ppShp, IDriver *pDriver)
{
	_pDriver = pDriver;
	_meshName = meshName;
	_ppShp = ppShp;
}

// ***************************************************************************

void CAsyncFileManager::CMeshLoad::run()
{
	// This set represent the texture already loaded in memory
	// We have to have this set because the driver load the textures only on the 
	// setupTexture, done in CShapeBank::isPresent. This must be done in the main
	// thread because setupTexture upload texture to VRAM.
	typedef set<string> TAlreadyPresentTextureSet;
	TAlreadyPresentTextureSet AlreadyPresentTextureSet;
	
/* // TEST
	int tmp = 1;
	//for(int i=0;i<100000000;++i)
	for(int j=0;j<4000;++j)
	{
		for(int k=0;k<1000000;++k)
		{
			tmp *= 5;
		}
		//nlSleep(0);
	}
*/

	try
	{
		// Load from file the mesh
		CShapeStream mesh;
		CIFile meshfile( CPath::lookup( _meshName ) );
		meshfile.serial( mesh );
		meshfile.close();
		
		// Call generate on all texture to force loading

		CMesh *pMesh = dynamic_cast<CMesh*>(mesh.getShapePointer());
		if( pMesh == NULL )
			return;

		// Parse all materials of the mesh
		uint i, j;
		uint nNbMat = pMesh->getNbMaterial();

		for(i = 0; i < nNbMat; ++i)
		{
			const CMaterial &rMat = pMesh->getMaterial(i);
			// Parse all textures from this material and generate them
			for(j = 0; j < IDRV_MAT_MAXTEXTURES; ++j)
			if( rMat.texturePresent(j) )
			{
				ITexture *pText = rMat.getTexture(j);
				// For all texture that are texture file we have to load them
				CTextureFile *pTextFile = dynamic_cast<CTextureFile*>(pText);
				// Does this texture is a texture file ?
				if(pTextFile != NULL)
				// Yes -> Does the texture is already present in the driver ?
				if( ! _pDriver->isTextureExist(*pTextFile) )
				{
					// No -> So we have perhaps to load it
					TAlreadyPresentTextureSet::iterator aptmIt = AlreadyPresentTextureSet.find(pTextFile->getFileName());
					// Is the texture already loaded ?
					if(aptmIt != AlreadyPresentTextureSet.end())
					{
						// Texture already present
						printf("texture deja presente\n");
					}
					else
					{
						// Texture not already present
						// add it
						AlreadyPresentTextureSet.insert(pTextFile->getFileName());
						// And load it
						printf("chargement de %s\n", pTextFile->getFileName().c_str());
						pTextFile->generate();
					}
				}
			}
		}

		
		// Finally affect the pointer (Trans-Thread operation -> this operation must be atomic)
		*_ppShp = mesh.getShapePointer();
	}
	catch(EPathNotFound &)
	{
		return;
	}
}

}


