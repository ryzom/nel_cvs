/** \file nel_export_scene.cpp
 * <File description>
 *
 * $Id: nel_export_scene.cpp,v 1.4 2001/06/12 12:30:58 besson Exp $
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

// Scene Exportation

// A scene is made of virtual object instance
// An object instance is 
// - A reference to a mesh (refered by the name of the node)
// - The transformations to get it to the world
// - The parent


#include "std_afx.h"
#include "nel_export.h"
#include "../nel_mesh_lib/export_nel.h"
#include "../nel_patch_lib/rpo.h"
#include "nel/3d/scene_group.h"

#include <vector>

using namespace std;
using namespace NL3D;
using namespace NLMISC;

#define NEL_OBJET_NAME_DATA 1970

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
bool CNelExport::exportInstanceGroup(string filename, vector<INode*>& vectNode, Interface& ip )
{
	// Extract from the node the name, the transformations and the parent
	CInstanceGroup::TInstanceArray aIGArray;
	int i, nNumIG = 0;

	aIGArray.empty ();
	aIGArray.resize (vectNode.size());

	TimeValue tvTime = ip.GetTime();

	// Check integrity of the hierarchy and set the parents
	std::vector<INode*>::iterator it = vectNode.begin();
	for(i=0; i<(sint)vectNode.size(); ++i,++it)
	{
		INode *pNode = *it;

		if( ! RPO::isZone( *pNode, tvTime ) )
		if( CExportNel::isMesh( *pNode, tvTime ) )
		{
		
			INode *pParent = pNode->GetParentNode();

			// Is the pNode has the root node for parent ?
			if( pParent->IsRootNode() == 0 )
			{
				// Look if the parent is in the selection
				std::vector<INode*>::iterator it2 = vectNode.begin();
				for(int j=0; j<(sint)vectNode.size(); ++j,++it2)
				{
					INode *pNode2 = *it2;
					if( pNode2 == pParent )
						break;
				}
				if (j==(sint)vectNode.size())
				{
					// The parent is not selected ! This is considered as an error
					// return ; // No more an error
					aIGArray[nNumIG].nParent = -1;
				}
				else
				{
					aIGArray[nNumIG].nParent = j;
				}
			}
			else
			{
				aIGArray[nNumIG].nParent = -1;
			}
			++nNumIG;
		}
	}
	aIGArray.resize( nNumIG );
	// Build the array of node
	nNumIG = 0;
	it = vectNode.begin();
	for(i=0; i<(sint)vectNode.size(); ++i,++it)
	{
		INode *pNode = *it;

		if( ! RPO::isZone( *pNode, tvTime ) )
		if( CExportNel::isMesh( *pNode, tvTime ) )
		{
			CVector vScaleTemp;
			CQuat qRotTemp;
			CVector vPosTemp;

			// Try to get an APPDATA for the name of the object
			AppDataChunk *ad = pNode->GetAppDataChunk(MAXSCRIPT_UTILITY_CLASS_ID, UTILITY_CLASS_ID, NEL_OBJET_NAME_DATA );
			if (ad&&ad->data)
			{
				// Get the name of the object in the APP data
				aIGArray[nNumIG].Name=(const char*)ad->data;
			}
			else
			{
				// Extract the node name
				aIGArray[nNumIG].Name = pNode->GetName();
			}

			//Get the local transformation matrix
			Matrix3 nodeTM = pNode->GetNodeTM(0);
			INode *pParent = pNode->GetParentNode();
			Matrix3 parentTM = pParent->GetNodeTM(0);
			Matrix3 localTM	= nodeTM*Inverse(parentTM);

			// Extract transformations
			CExportNel::decompMatrix (vScaleTemp, qRotTemp, vPosTemp, localTM);
			aIGArray[nNumIG].Rot   = qRotTemp;
			aIGArray[nNumIG].Pos   = vPosTemp;
			aIGArray[nNumIG].Scale = vScaleTemp;
			++nNumIG;
		}
	}

	if (aIGArray.size())
	{
		CInstanceGroup ig;
		COFile file;
		
		ig.build( aIGArray );

		if (file.open (filename))
		{
			try
			{
				// Serial the skeleton
				ig.serial (file);
				// All is good
			}
			catch (Exception &c)
			{
				// Cannot save the file
				MessageBox (NULL, c.what(), "NeL export", MB_OK|MB_ICONEXCLAMATION);
				return false;
			}
		}
	}
	else
	{
		// No node found with a SWT Modifier
	}
	return true;
}
