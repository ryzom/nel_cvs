/** \file nel_export_collision.cpp
 * 
 *
 * $Id: nel_export_collision.cpp,v 1.1 2001/08/08 09:04:46 legros Exp $
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

using namespace NLMISC;
using namespace NLPACS;

// --------------------------------------------------

bool CNelExport::exportCollision (const char *sPath, std::vector<INode *> &nodes, Interface& ip, TimeValue time, CExportNelOptions &opt)
{
	// Result to return
	bool bRet=false;

	// Eval the objects a time
	uint	i;

	for (i=0; i<nodes.size(); ++i)
	{
		ObjectState os = nodes[i]->EvalWorldState(time);
		if (!os.obj)
			return bRet;
	}

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

