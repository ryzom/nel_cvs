/** \file async_file_manager.cpp
 * <File description>
 *
 * $Id: async_file_manager.cpp,v 1.7 2002/04/18 08:30:00 besson Exp $
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
	delete &getInstance();
	_Singleton = NULL;
}

// ***************************************************************************
	
void CAsyncFileManager::loadMesh(const std::string& meshName, IShape **ppShp, IDriver *pDriver)
{
	addTask (new CMeshLoad(meshName, ppShp, pDriver));
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

	/*int tmp = 4;
	int tmp2 = 0;
	for(int i=0;i<100000000;++i)
	for(int j=0;j<4000;++j)
	{
		for(int k=0;k<1000000;++k)
		{
			tmp *= 5+k*i-j;
			tmp2 = tmp + tmp/200;
		}
		//nlSleep(0);
	}
	++tmp2;
	printf("%d",tmp2);*/

	try
	{
		// Load from file the mesh
		CShapeStream mesh;
		CIFile meshfile( CPath::lookup( _meshName ) );
		meshfile.serial( mesh );
		meshfile.close();
		
		// Call generate on all texture to force loading

		CMesh *pMesh = dynamic_cast<CMesh*>(mesh.getShapePointer());
		// If the driver is not given so do not try to load the textures
		if ((pMesh == NULL) || (_pDriver == NULL))
		{
			delete this;
			return;
		}

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
						// printf("loading texture %s\n", pTextFile->getFileName().c_str());
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
		delete this;
		return;
	}
	delete this;
}


// ***************************************************************************
CAsyncFileManager::CIGLoad::CIGLoad (const std::string &IGName, CInstanceGroup **ppIG)
{
	_IGName = IGName;
	_ppIG = ppIG;
}

// ***************************************************************************
void CAsyncFileManager::CIGLoad::run (void)
{
	/*int tmp = 4;
	int tmp2 = 0;
	for(int i=0;i<100000000;++i)
	for(int j=0;j<4000;++j)
	{
		for(int k=0;k<1000000;++k)
		{
			tmp *= 5+k*i-j;
			tmp2 = tmp + tmp/200;
		}
		//nlSleep(0);
	}
	++tmp2;
	printf("%d",tmp2);*/

	try
	{
		CIFile igfile( CPath::lookup( _IGName ) );
		CInstanceGroup *pIG = new CInstanceGroup();
		pIG->serial (igfile);
		igfile.close();

		*_ppIG = pIG;
	}
	catch(EPathNotFound &)
	{
		delete this;
		return;
	}
	delete this;
}


// ***************************************************************************
CAsyncFileManager::CIGLoadUser::CIGLoadUser (const std::string &IGName, UInstanceGroup **ppIG)
{
	_IGName = IGName;
	_ppIG = ppIG;
}

// ***************************************************************************
void CAsyncFileManager::CIGLoadUser::run (void)
{
	/*int tmp = 4;
	int tmp2 = 0;
	for(int i=0;i<100000000;++i)
	for(int j=0;j<4000;++j)
	{
		for(int k=0;k<1000000;++k)
		{
			tmp *= 5+k*i-j;
			tmp2 = tmp + tmp/200;
		}
		//nlSleep(0);
	}
	++tmp2;
	printf("%d",tmp2);*/

	try
	{
		CInstanceGroupUser *pIG = new CInstanceGroupUser();
		pIG->init (_IGName);

		*_ppIG = pIG;
	}
	catch(EPathNotFound &)
	{
		delete this;
		return;
	}
	delete this;
}

// ***************************************************************************
CAsyncFileManager::CFileLoad::CFileLoad (const std::string& sFileName, uint8 **ppFile)
{
	_FileName = sFileName;
	_ppFile = ppFile;
}

// ***************************************************************************
void CAsyncFileManager::CFileLoad::run (void)
{
/*	HANDLE hFile = CreateFile (_FileName.c_str(),
								GENERIC_READ,
								FILE_SHARE_READ|FILE_SHARE_WRITE,
								NULL,
								OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL,
								NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		uint8 *ptr;
		nlSleep(5);
		DWORD filesize = GetFileSize (hFile, NULL);
		ptr = new uint8[filesize];
		DWORD nbBytesRead;
		nlSleep(5);
		ReadFile (hFile, ptr, filesize, &nbBytesRead, NULL);
		CloseHandle (hFile);

		*_ppFile = ptr;
	}
	else
	{
		*_ppFile = (uint8*)-1;
	}
*/

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
	// The following are tests please do not remove until 


/*	FILE *f = fopen (_FileName.c_str(), "rb");
	if (f != NULL)
	{
		uint8 *ptr;
		fseek (f, 0, SEEK_END);
		long filesize = ftell (f);
		fseek (f, 0, SEEK_SET);
		ptr = new uint8[filesize];
*/
		// Test 0 : Entrelacing reading and code
		/*uint32 i = 0, j;
		while (i < filesize)
		{
			j = i + 100*1024;
			if (j > filesize)
			{
				for (j = 0; j < filesize-i; ++j)
				{
					fread(ptr+i+j, 1, 1, f);
					ptr[i+j] += j+i;
				}
			}
			else
			{
				for (j = 0; j < 100*1024; ++j)
				{
					fread(ptr+i+j, 1, 1, f);
					ptr[i+j] += j+i;
				}
			}

			i += 100*1024;
			nlSleep (20);
		}*/

		// Test 1 : continuous read of the whole file
		/*fread (ptr, filesize, 1, f);
		static	counterTam= 0;
		counterTam+= filesize;
		if(counterTam>100*1024)
		{
			counterTam= 0;
			nlSleep(10);
		}*/

		// Test 2 : read chunk of 200 ko 1000 times at random pos within big file
		//for (uint32 i = 0; i < 1000; ++i)
		//{
		//	uint32 decal = (uint32)(frand(1.0)*(filesize-100*1024));
		//	uint8 *ptr2 = ptr + decal;
		//	fseek (f, decal, SEEK_SET);

		//	for (uint32 j = 0; j < 10*1024; ++j)
		//	{
		//		fread (ptr2+j*10, 10, 1, f);
		//		for (uint32 k = 0; k < 10; ++k)
		//		{
		//			ptr2[j*10+k] += j+k;
		//		}
		//	}
		//	nlSleep(5);

			/*			fread (ptr2, 100*1024, 1, f);


			for (uint32 k = 0; k < 1; ++k)
			{
				for (uint32 j = 0; j < 100*1024; ++j)
				{
					ptr2[j+k*100*1024] += j+k;
				}
				nlSleep (5);
			}*/
//		}
		
		// Test 3 : read chunk of variable size 2000 times at random pos within big file
		/*for (uint32 i = 0; i < 2000; ++i)
		{
			uint32 zesize = 3*1024 + (uint32)(frand(1.0)*(350*1024));
			uint32 decal = (uint32)(frand(1.0)*(filesize-zesize));
			uint8 *ptr2 = ptr + decal;
			fseek (f, decal, SEEK_SET);
			fread (ptr2, zesize, 1, f);

			for (uint32 j = 0; j < zesize; ++j)
			{
				ptr2[j] += j%3;
			}
		}*/
//	fclose (f);

}

CAsyncFileManager::CMultipleFileLoad::CMultipleFileLoad (const std::vector<std::string> &vFileNames, const std::vector<uint8**> &vPtrs)
{
	_FileNames = vFileNames;
	_Ptrs = vPtrs;
}

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



}


