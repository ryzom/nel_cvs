/** \file async_file_manager.h
 * <File description>
 *
 * $Id: async_file_manager.h,v 1.1 2001/06/15 16:24:42 corvazier Exp $
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

/**
 * CAsyncFileManager is a class that manage file loading in a seperate thread
 * \author Matthieu Besson
 * \author Nevrax France
 * \date 2001 
 */
class CAsyncFileManager : public NLMISC::CTaskManager
{
public:
	void loadMesh(const std::string& meshName, IShape** ppShp, IDriver *pDriver);
	//////// void LoadZone(std::string& zoneName);

// All the tasks

	class CMeshLoad : public NLMISC::IRunnable
	{
		std::string _meshName;
		IShape **_ppShp;
		IDriver *_pDriver;
	public:
		CMeshLoad(const std::string& meshName, IShape** ppShp, IDriver *pDriver);
		void run(void);
	};

};


} // NL3D


#endif // NL_ASYNC_FILE_MANAGER_H

/* End of async_file_manager.h */
