/** \file nel_export_collision.cpp
 * 
 *
 * $Id: nel_export_collision.cpp,v 1.2 2001/11/29 14:22:23 legros Exp $
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

#include "std_afx.h"
#include "nel_export.h"
#include "nel/misc/file.h"

#include "pacs/collision_mesh_build.h"

#include "../nel_mesh_lib/export_nel.h"
#include "../nel_mesh_lib/export_lod.h"

using namespace NLMISC;
using namespace NLPACS;

// --------------------------------------------------

bool CNelExport::exportCollision (const char *sPath, std::vector<INode *> &nodes, Interface& ip, TimeValue time, CExportNelOptions &opt)
{
	// Result to return
	bool bRet=false;

	// Eval the objects a time
	uint	i, j;

	for (i=0; i<nodes.size(); ++i)
	{
		ObjectState os = nodes[i]->EvalWorldState(time);
		if (!os.obj)
			return bRet;
	}

	std::vector<std::pair<std::string, std::vector<INode *> > >	igs;
	for (i=0; i<nodes.size(); ++i)
	{
		std::string	ig = CExportNel::getScriptAppData(nodes[i], NEL3D_APPDATA_IGNAME, "");
		if (ig == "")
			ig = "unknown_ig";

		for (j=0; j<igs.size() && ig!=igs[j].first; ++j)
			;
		if (j == igs.size())
		{
			igs.push_back();
			igs[j].first = ig;
		}

		igs[j].second.push_back(nodes[i]);
	}

//	ULONG SelectDir(HWND Parent, char* Title, char* Path);

	std::string	path = std::string(sPath);
	if (path.size() == 0 || path[path.size()-1] != '\\' && path[path.size()-1] != '/')
		path.insert(path.end(), '/');

	for (i=0; i<igs.size(); ++i)
	{
		std::string				igname = igs[i].first;
		std::vector<INode *>	&ignodes = igs[i].second;
		std::string				filename = path+igname+".cmb";
		// Object exist ?
		CCollisionMeshBuild	*pCmb = CExportNel::createCollisionMeshBuild(ignodes, time);

		// Conversion success ?
		if (pCmb)
		{
			// Open a file
			COFile file;
			if (file.open (filename))
			{
				try
				{
					// Serialise the collision mesh build
					file.serial(*pCmb);

					// All is good
					bRet=true;
				}
				catch (...)
				{
				}
			}

			// Delete the pointer
			delete pCmb;
		}
	}

/*
	// Object exist ?
	CCollisionMeshBuild	*pCmb = CExportNel::createCollisionMeshBuild(nodes, time);

	// Conversion success ?
	if (pCmb)
	{
		// Open a file
		COFile file;
		if (file.open (sPath))
		{
			try
			{
				// Serialise the collision mesh build
				file.serial(*pCmb);

				// All is good
				bRet=true;
			}
			catch (...)
			{
			}
		}

		// Delete the pointer
		delete pCmb;
	}
*/
	return bRet;
}

/*
bool CNelExport::exportCollision (const char *sPath, INode& node, Interface& ip, TimeValue time, CExportNelOptions &opt)
{
	// Result to return
	bool bRet=false;

	// Eval the object a time
	ObjectState os = node.EvalWorldState(time);

	// Object exist ?
	if (os.obj)
	{
		CCollisionMeshBuild	*pCmb = CExportNel::createCollisionMeshBuild(node, time);

		// Conversion success ?
		if (pCmb)
		{
			// Open a file
			COFile file;
			if (file.open (sPath))
			{
				try
				{
					// Serialise the collision mesh build
					file.serial(*pCmb);

					// All is good
					bRet=true;
				}
				catch (...)
				{
				}
			}

			// Delete the pointer
			delete pCmb;
		}
	}
	return bRet;
}
*/
// --------------------------------------------------

