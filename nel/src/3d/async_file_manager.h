/** \file async_file_manager.h
 * <File description>
 *
 * $Id: async_file_manager.h,v 1.2 2002/04/17 12:09:22 besson Exp $
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

#ifndef NL_ASYNC_FILE_MANAGER_H
#define NL_ASYNC_FILE_MANAGER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/task_manager.h"


namespace NL3D
{

class IShape;
class IDriver;
class CInstanceGroup;
class UInstanceGroup;

/**
 * CAsyncFileManager is a class that manage file loading in a seperate thread
 * \author Matthieu Besson
 * \author Nevrax France
 * \date 2002 
 */
class CAsyncFileManager : public NLMISC::CTaskManager
{

public:

	static CAsyncFileManager &getInstance (); // Must be called instead of constructing the object
	void terminate (); // End all tasks and terminate

	void loadMesh (const std::string &meshName, IShape **ppShp, IDriver *pDriver);
	void loadIG (const std::string &igName, CInstanceGroup **ppIG);
	void loadIGUser (const std::string &igName, UInstanceGroup **ppIG);

	void loadFile (const std::string &fileName, uint8 **pPtr);

private:

	CAsyncFileManager (); // Singleton mode -> access it with the getInstance function

	static CAsyncFileManager *_Singleton;

	// All the tasks
	// -------------
	
	// Load a .shape
	class CMeshLoad : public NLMISC::IRunnable
	{
		std::string _meshName;
		IShape **_ppShp;
		IDriver *_pDriver;
	public:
		CMeshLoad (const std::string &meshName, IShape **ppShp, IDriver *pDriver);
		void run (void);
	};

	// Load a .ig
	class CIGLoad : public NLMISC::IRunnable
	{
		std::string _IGName;
		CInstanceGroup **_ppIG;
	public:
		CIGLoad (const std::string& meshName, CInstanceGroup **ppIG);
		void run (void);
	};

	// Load a .ig User Interface
	class CIGLoadUser : public NLMISC::IRunnable
	{
		std::string _IGName;
		UInstanceGroup **_ppIG;
	public:
		CIGLoadUser (const std::string& meshName, UInstanceGroup **ppIG);
		void run (void);
	};

	// Load a file
	class CFileLoad : public NLMISC::IRunnable
	{
		std::string _FileName;
		uint8 **_ppFile;
	public:
		CFileLoad (const std::string& sFileName, uint8 **ppFile);
		void run (void);
	};

};


} // NL3D


#endif // NL_ASYNC_FILE_MANAGER_H

/* End of async_file_manager.h */
