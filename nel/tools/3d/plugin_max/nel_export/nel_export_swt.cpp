/** \file nel_export_swt.cpp
 * <File description>
 *
 * $Id: nel_export_swt.cpp,v 1.1 2001/04/26 16:37:31 corvazier Exp $
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

// Skeleton Weight Template exportation

#include "std_afx.h"
#include "nel_export.h"
#include "resource.h"
#include "nel/3d/skeleton_weight.h"
#include "nel/misc/file.h"
#include "../nel_mesh_lib/export_nel.h"
//#include <max.h>
//#include <modstack.h>

using namespace NL3D;
using namespace NLMISC;

#define NEL_SWT_CLASS_ID_A 0x9765367
#define NEL_SWT_CLASS_ID_B 0x4b4a356a




bool CNelExport::exportSWT(const char *sPath, std::vector<INode*>& vectNode, Interface& ip)
{
	float rPosValue;
	float rRotValue;
	float rScaleValue;
	CSkeletonWeight::TNodeArray aSWNodes; // Array of Skeleton Weight Node
	int nNumNode = 0;

	aSWNodes.empty();

	// Build the array of node
	std::vector<INode*>::iterator it = vectNode.begin();

	for(int i=0; i<(int)vectNode.size(); ++i,++it)
	{
		// Get the SWT Modifier
		INode *pNode = *it;
		Modifier *pModifier = CExportNel::getModifier (pNode,Class_ID(NEL_SWT_CLASS_ID_A, NEL_SWT_CLASS_ID_B) );
		if (pModifier == NULL) continue;

		// Get the value of the parameters
		CExportNel::getValueByNameUsingParamBlock2( *pModifier, "swtPosValue", (ParamType2)TYPE_FLOAT, &rPosValue, 0);
		CExportNel::getValueByNameUsingParamBlock2( *pModifier, "swtRotValue", (ParamType2)TYPE_FLOAT, &rRotValue, 0);
		CExportNel::getValueByNameUsingParamBlock2( *pModifier, "swtScaleValue", (ParamType2)TYPE_FLOAT, &rScaleValue, 0);

		// Store them in the temporary list
		if (( rPosValue == rRotValue) && ( rPosValue == rScaleValue))
		{
			aSWNodes.resize(nNumNode+1);
			// We can optimize the number of SWT
			aSWNodes[nNumNode].Name = pNode->GetName();
			aSWNodes[nNumNode].Name += ".";
			aSWNodes[nNumNode].Weight = rRotValue;
			++nNumNode;
		}
		else
		{
			aSWNodes.resize(nNumNode+3);
			aSWNodes[nNumNode].Name = pNode->GetName();
			aSWNodes[nNumNode].Name += ".rot";
			aSWNodes[nNumNode].Weight = rRotValue;
			++nNumNode;
			aSWNodes[nNumNode].Name = pNode->GetName();
			aSWNodes[nNumNode].Name += ".pos";
			aSWNodes[nNumNode].Weight = rPosValue;
			++nNumNode;
			aSWNodes[nNumNode].Name = pNode->GetName();
			aSWNodes[nNumNode].Name += ".scale";
			aSWNodes[nNumNode].Weight = rScaleValue;
			++nNumNode;
		}
	}

	if (aSWNodes.size())
	{
		CSkeletonWeight sw;
		COFile file;
		
		sw.build( aSWNodes );

		if (file.open (sPath))
		{
			try
			{							
				// Serial the skeleton
				sw.serial (file);
				// All is good
			}
			catch (...)
			{
			}
		}
	}
	else
	{
		// No node found with a SWT Modifier
	}

	return true;
}