/** \file async_file_manager.cpp
 * <File description>
 *
 * $Id: async_file_manager.cpp,v 1.11 2002/04/30 13:48:46 besson Exp $
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
#include "3d/scene_group.h"

#include "3d/instance_group_user.h"

#include "nel/misc/file.h"
#include "nel/misc/path.h"


using namespace std;
using namespace NLMISC;

namespace NL3D
{

CAsyncFileManager *CAsyncFileManager::_Singleton = NULL;

// ***************************************************************************

CAsyncFileManager::CAsyncFileManager()
{
}

// ***************************************************************************

CAsyncFileManager &CAsyncFileManager::getInstance()
{
	if (_Singleton == NULL)
	{
		_Singleton = new CAsyncFileManager();
	}
	return *_Singleton;
}

// ***************************************************************************

void CAsyncFileManager::terminate ()
{
	if (_Singleton != NULL)
	{
		delete &getInstance();
		_Singleton = NULL;
	}
}

// ***************************************************************************
	
void CAsyncFileManager::loadMesh(const std::string& meshName, IShape **ppShp, IDriver *pDriver)
{
	addTask (new CMeshLoad(meshName, ppShp, pDriver));
}

// ***************************************************************************

bool CAsyncFileManager::cancelLoadMesh(const std::string& sMeshName)
{
	CSynchronized<list<IRunnable *> >::CAccessor acces(&_TaskQueue);
	list<IRunnable*> &rTaskQueue = acces.value ();
	list<IRunnable*>::iterator it = rTaskQueue.begin();

	while (it != rTaskQueue.end())
	{
		IRunnable *pR = *it;
		CMeshLoad *pML = dynamic_cast<CMeshLoad*>(pR);
		if (pML != NULL)
		{
			if (pML->MeshName == sMeshName)
			{
				// Delete mesh load task
				delete pML;
				rTaskQueue.erase (it);
				return true;
			}
		}
		++it;
	}
	return false;
}

// ***************************************************************************
	
void CAsyncFileManager::loadIG (const std::string& IGName, CInstanceGroup **ppIG)
{
	addTask (new CIGLoad(IGName, ppIG));
}

// ***************************************************************************
	
void CAsyncFileManager::loadIGUser (const std::string& IGName, UInstanceGroup **ppIG)
{
	addTask (new CIGLoadUser(IGName, ppIG));
}

// ***************************************************************************
	
void CAsyncFileManager::loadFile (const std::string& sFileName, uint8 **ppFile)
{
	addTask (new CFileLoad (sFileName, ppFile));
}

// ***************************************************************************

void CAsyncFileManager::loadFiles (const std::vector<std::string> &vFileNames, const std::vector<uint8**> &vPtrs)
{
	addTask (new CMultipleFileLoad (vFileNames, vPtrs));
}

// ***************************************************************************

void CAsyncFileManager::signal (bool *pSgn)
{
	addTask (new CSignal (pSgn));
}

// ***************************************************************************

void CAsyncFileManager::cancelSignal (bool *pSgn)
{
	CSynchronized<list<IRunnable *> >::CAccessor acces(&_TaskQueue);
	list<IRunnable*> &rTaskQueue = acces.value ();
	list<IRunnable*>::iterator it = rTaskQueue.begin();

	while (it != rTaskQueue.end())
	{
		IRunnable *pR = *it;
		CSignal *pS = dynamic_cast<CSignal*>(pR);
		if (pS != NULL)
		{
			if (pS->Sgn == pSgn)
			{
				// Delete signal task
				delete pS;
				rTaskQueue.erase (it);
				return;
			}
		}
		++it;
	}
}

// ***************************************************************************
// TASKS
// ***************************************************************************

// ***************************************************************************
// MeshLoad
// ***************************************************************************

CAsyncFileManager::CMeshLoad::CMeshLoad(const std::string& sMeshName, IShape** ppShp, IDriver *pDriver)
{
	_pDriver = pDriver;
	MeshName = sMeshName;
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

	try
	{
		// Load from file the mesh
		CShapeStream mesh;
		CIFile meshfile;
		meshfile.setAsyncLoading (true);
		meshfile.setCacheFileOnOpen (true);
		meshfile.open (CPath::lookup(MeshName));
		meshfile.serial (mesh);
		meshfile.close ();

		// Is the pointer is invalid return -1
		if (mesh.getShapePointer() == NULL)
		{
			nlwarning ("cant load %s", MeshName);
			*_ppShp = (IShape*)-1;
			delete this;
			return;
		}

		CMeshBase *pMesh = dynamic_cast<CMeshBase *>(mesh.getShapePointer());

		// If the shape is a mesh but the driver is not given or if the shape is not a mesh 
		// so do not try to load the textures

		if ((pMesh == NULL) || ((pMesh != NULL) && (_pDriver == NULL)))
		{
			*_ppShp = mesh.getShapePointer();
			delete this;
			return;
		}
		// Here we are sure that the shape is a mesh and the driver is not null
		// Parse all materials of the mesh
		uint i, j;
		uint nNbMat = pMesh->getNbMaterial();
		ITexture *pText;

		for(i = 0; i < nNbMat; ++i)
		{
			const CMaterial &rMat = pMesh->getMaterial(i);
			// Parse all textures from this material and generate them
			for(j = 0; j < IDRV_MAT_MAXTEXTURES; ++j)
			if (rMat.texturePresent(j))
			{
				pText = rMat.getTexture (j);
				// For all texture that are texture file we have to load them
				CTextureFile *pTextFile = dynamic_cast<CTextureFile*>(pText);
				// Does this texture is a texture file ?
				if(pTextFile != NULL)
				// Yes -> Does the texture is already present in the driver ?
				if( ! _pDriver->isTextureExist(*pTextFile) )
				{
					// No -> So we have perhaps to load it
					TAlreadyPresentTextureSet::iterator aptmIt = AlreadyPresentTextureSet.find (pTextFile->getFileName());
					// Is the texture already loaded ?
					if(aptmIt == AlreadyPresentTextureSet.end())
					{
						// Texture not already present
						// add it
						AlreadyPresentTextureSet.insert (pTextFile->getFileName());
						// And load it (to RAM only (upload in VRAM is done in the shape bank))
						pTextFile->generate();
					}
				}
			}

			// Do the same with lightmaps
			if (rMat.getShader() == CMaterial::LightMap)
			{
				j = 0; pText = rMat.getLightMap (j);
				while (pText != NULL)
				{
					CTextureFile *pTextFile = dynamic_cast<CTextureFile*>(pText);
					// Does this texture is a texture file ?
					if(pTextFile != NULL)
					// Yes -> Does the texture is already present in the driver ?
					if (!_pDriver->isTextureExist(*pTextFile))
					{
						// No -> So we have perhaps to load it
						TAlreadyPresentTextureSet::iterator aptmIt = AlreadyPresentTextureSet.find (pTextFile->getFileName());
						// Is the texture already loaded ?
						if(aptmIt == AlreadyPresentTextureSet.end())
						{
							// Texture not already present -> add it and load it to RAM
							AlreadyPresentTextureSet.insert (pTextFile->getFileName());
							pTextFile->generate();
						}
					}				
					++j; pText = rMat.getLightMap (j);
				}
			}
		}
		// Finally affect the pointer (Trans-Thread operation -> this operation must be atomic)
		*_ppShp = mesh.getShapePointer();
	}
	catch(EPathNotFound &)
	{
		nlwarning ("cant load %s", MeshName);
		*_ppShp = (IShape*)-1;
		delete this;
		return;
	}
	delete this;
}

// ***************************************************************************
// IGLoad
// ***************************************************************************

// ***************************************************************************
CAsyncFileManager::CIGLoad::CIGLoad (const std::string &IGName, CInstanceGroup **ppIG)
{
	_IGName = IGName;
	_ppIG = ppIG;
}

// ***************************************************************************
void CAsyncFileManager::CIGLoad::run (void)
{
	try
	{
		CIFile igfile;
		igfile.setAsyncLoading (true);
		igfile.setCacheFileOnOpen (true);
		igfile.open (CPath::lookup (_IGName));
		CInstanceGroup *pIG = new CInstanceGroup();
		pIG->serial (igfile);
		igfile.close();

		*_ppIG = pIG;
	}
	catch(EPathNotFound &)
	{
		*_ppIG = (CInstanceGroup*)-1;
		delete this;
		return;
	}
	delete this;
}

// ***************************************************************************
// IGLoadUser
// ***************************************************************************

// ***************************************************************************
CAsyncFileManager::CIGLoadUser::CIGLoadUser (const std::string &IGName, UInstanceGroup **ppIG)
{
	_IGName = IGName;
	_ppIG = ppIG;
}

// ***************************************************************************
void CAsyncFileManager::CIGLoadUser::run (void)
{
	try
	{
		CInstanceGroupUser *pIG = new CInstanceGroupUser();
		pIG->init (_IGName);

		*_ppIG = pIG;
	}
	catch(EPathNotFound &)
	{
		*_ppIG = (UInstanceGroup*)-1;
		delete this;
		return;
	}
	delete this;
}

// ***************************************************************************
// FileLoad
// ***************************************************************************

// ***************************************************************************
CAsyncFileManager::CFileLoad::CFileLoad (const std::string& sFileName, uint8 **ppFile)
{
	_FileName = sFileName;
	_ppFile = ppFile;
}

// ***************************************************************************
void CAsyncFileManager::CFileLoad::run (void)
{
	FILE *f = fopen (_FileName.c_str(), "rb");
	if (f != NULL)
	{
		uint8 *ptr;
		fseek (f, 0, SEEK_END);
		long filesize = ftell (f);
		nlSleep(5);
		fseek (f, 0, SEEK_SET);
		ptr = new uint8[filesize];
		fread (ptr, filesize, 1, f);
		fclose (f);

		*_ppFile = ptr;
	}
	else
	{
		*_ppFile = (uint8*)-1;
	}
}

// ***************************************************************************
// MultipleFileLoad
// ***************************************************************************

// ***************************************************************************
CAsyncFileManager::CMultipleFileLoad::CMultipleFileLoad (const std::vector<std::string> &vFileNames, 
														 const std::vector<uint8**> &vPtrs)
{
	_FileNames = vFileNames;
	_Ptrs = vPtrs;
}

// ***************************************************************************
void CAsyncFileManager::CMultipleFileLoad::run (void)
{
	for (uint32 i = 0; i < _FileNames.size(); ++i)
	{
		FILE *f = fopen (_FileNames[i].c_str(), "rb");
		if (f != NULL)
		{
			uint8 *ptr;
			fseek (f, 0, SEEK_END);
			long filesize = ftell (f);
			nlSleep(5);
			fseek (f, 0, SEEK_SET);
			ptr = new uint8[filesize];
			fread (ptr, filesize, 1, f);
			fclose (f);

			*_Ptrs[i] = ptr;
		}
		else
		{
			*_Ptrs[i] = (uint8*)-1;
		}
	}

}

// ***************************************************************************
// Signal
// ***************************************************************************

// ***************************************************************************
CAsyncFileManager::CSignal::CSignal (bool *pSgn)
{
	Sgn = pSgn;
	*Sgn = false;
}

// ***************************************************************************
void CAsyncFileManager::CSignal::run (void)
{
	*Sgn = true;
}


}


